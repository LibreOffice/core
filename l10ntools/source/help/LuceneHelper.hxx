#ifndef LUCENEHELPER_HXX
#define LUCENEHELPER_HXX

#include <CLucene/StdHeader.h>
#include <CLucene.h>

#include <rtl/ustring.hxx>
#include <vector>

std::vector<TCHAR> OUStringToTCHARVec(rtl::OUString const &rStr);
rtl::OUString TCHARArrayToOUString(TCHAR const *str);

#endif
