#ifndef _RTFTYPES_HXX_
#define _RTFTYPES_HXX_

#include <rtfcontrolwords.hxx>

namespace writerfilter {
    namespace rtftok {
        /// Respresents an RTF Control Word
        typedef struct
        {
            const char *sKeyword;
            int nControlType;
            RTFKeyword nIndex;
        } RTFSymbol;
        /// RTF legacy charsets
        typedef struct
        {
            int charset;
            int codepage;
        } RTFEncoding;
    } // namespace rtftok
} // namespace writerfilter

#endif // _RTFTYPES_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
