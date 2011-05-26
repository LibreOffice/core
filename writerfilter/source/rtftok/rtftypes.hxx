#ifndef _RTFTYPES_HXX_
#define _RTFTYPES_HXX_

namespace writerfilter {
    namespace rtftok {
        typedef struct
        {
            const char *sKeyword;
            int nControlType;
        } RTFSymbol;
    } // namespace rtftok
} // namespace writerfilter

#endif // _RTFTYPES_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
