// See README.md for information and build instructions.

#include <ctime>
#include <fstream>
#include <google/protobuf/util/time_util.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/gzip_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <iostream>
#include <string>

#include "addressbook.pb.h"

using namespace std;

using google::protobuf::util::TimeUtil;
using namespace google::protobuf;

// This function fills in a Person message based on user input.
void PromptForAddress(tutorial::Person* person) {
  cout << "Enter person ID number: ";
  int id;
  cin >> id;
  person->set_id(id);
  cin.ignore(256, '\n');

  cout << "Enter name: ";
  getline(cin, *person->mutable_name());

  cout << "Enter email address (blank for none): ";
  string email;
  getline(cin, email);
  if (!email.empty()) {
    person->set_email(email);
  }

  while (true) {
    cout << "Enter a phone number (or leave blank to finish): ";
    string number;
    getline(cin, number);
    if (number.empty()) {
      break;
    }

    tutorial::Person::PhoneNumber* phone_number = person->add_phones();
    phone_number->set_number(number);

    cout << "Is this a mobile, home, or work phone? ";
    string type;
    getline(cin, type);
    if (type == "mobile") {
      phone_number->set_type(tutorial::Person::MOBILE);
    } else if (type == "home") {
      phone_number->set_type(tutorial::Person::HOME);
    } else if (type == "work") {
      phone_number->set_type(tutorial::Person::WORK);
    } else {
      cout << "Unknown phone type.  Using default." << endl;
    }
  }
  *person->mutable_last_updated() = TimeUtil::SecondsToTimestamp(time(NULL));
}

// Main function:  Reads the entire address book from a file,
//   adds one person based on user input, then writes it back out to the same
//   file.
int main(int argc, char* argv[]) {
  // Verify that the version of the library that we linked against is
  // compatible with the version of the headers we compiled against.
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  if (argc != 2) {
    cerr << "Usage:  " << argv[0] << " ADDRESS_BOOK_FILE" << endl;
    return -1;
  }

  tutorial::AddressBook address_book;

  {
    io::IstreamInputStream* wrappedFileStream;
    io::GzipInputStream* gzipStream;
 
    // Read the existing address book.
    std::ifstream fileStream(argv[1], std::ios::in | std::ios::binary);
    if (!fileStream)
        cout << argv[1] << ": File not found.  Creating a new file." << endl;
    else {
        wrappedFileStream = new io::IstreamInputStream(&fileStream);
        gzipStream = new io::GzipInputStream(wrappedFileStream);
        io::CodedInputStream codedStream(gzipStream);
        address_book.ParseFromCodedStream(&codedStream);
    }
    delete gzipStream;
    delete wrappedFileStream;
    fileStream.close();
  }

  // Add an address.
  PromptForAddress(address_book.add_people());

  {
    io::OstreamOutputStream* wrappedFileStream;
    io::GzipOutputStream* gzipStream;

    std::ofstream fileStream(argv[1], std::ios::out | std::ios::binary | std::ios::trunc);
    wrappedFileStream = new io::OstreamOutputStream(&fileStream);
    gzipStream = new io::GzipOutputStream(wrappedFileStream);
    {
        io::CodedOutputStream codedStream(gzipStream);
        address_book.SerializeToCodedStream(&codedStream);
    }
    delete gzipStream;
    delete wrappedFileStream;
    fileStream.close();
  }

  // Optional:  Delete all global objects allocated by libprotobuf.
  google::protobuf::ShutdownProtobufLibrary();

  return 0;
}
