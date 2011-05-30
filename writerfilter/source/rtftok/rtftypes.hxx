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
        extern RTFSymbol aRTFControlWords[];
        extern int nRTFControlWords;

        /// RTF legacy charsets
        typedef struct
        {
            int charset;
            int codepage;
        } RTFEncoding;
        extern RTFEncoding aRTFEncodingss[];
        extern int nRTFEncodings;
    } // namespace rtftok
} // namespace writerfilter

#endif // _RTFTYPES_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
