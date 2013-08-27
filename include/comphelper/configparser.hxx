#ifndef XML_CONFIGPARSING_HXX
#define XML_CONFIGPARSING_HXX
#include "rtl/ustring.hxx"
#include <vector>
#include "comphelper/comphelperdllapi.h"

namespace comphelper
{
    class COMPHELPER_DLLPUBLIC Xml_ConfigParsing
    {
        public:
            Xml_ConfigParsing();
            const static std::vector<OUString>& InItExpandedFilePath(const rtl::OUString&);
    };
}
#endif