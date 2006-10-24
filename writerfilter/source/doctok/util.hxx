#ifndef INCLUDED_UTIL_HXX
#define INCLUDED_UTIL_HXX

#include <string>
#include <iostream>

namespace doctok {
using namespace ::std;

/**
   Assertion

   @bTest       if false the assertion is raised
*/
void util_assert(bool bTest);

/**
   Print string to ostream.

   Printable characters are passed without change. Non-printable
   characters are replaced by '.'.

   @param o      ostream for output
   @param str    string to print
 */
void printBytes(ostream & o, const string & str);
}

#endif // INCLUDED_UTIL_HXX
