#ifndef _RTFDOCUMENTIMPL_HXX_
#define _RTFDOCUMENTIMPL_HXX_

#include <memory>
#include <rtftok/RTFDocument.hxx>

class SvStream;

namespace writerfilter {
    namespace rtftok {
        enum RTFInternalState
        {
            INTERNAL_NORMAL,
            INTERNAL_BIN,
            INTERNAL_HEX
        };

        enum RTFErrors
        {
            ERROR_OK,
            ERROR_GROUP_UNDER,
            ERROR_GROUP_OVER,
            ERROR_EOF
        };

        /// Implementation of the RTFDocument interface.
        class RTFDocumentImpl
            : public RTFDocument
        {
            public:
                RTFDocumentImpl(
                        ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >
                        const& xInputStream);
                virtual ~RTFDocumentImpl();
                virtual void resolve(Stream & rHandler);
                virtual ::std::string getType() const;

                SvStream& Strm();
                Stream& Mapper();
            private:
                int resolveParse();
                int resolveKeyword();
                int resolveChars(char ch);

                SvStream* m_pInStream;
                Stream* m_pMapperStream;
                int m_nGroup;
                RTFInternalState m_nInternalState;
        };
    } // namespace rtftok
} // namespace writerfilter

#endif // _RTFDOCUMENTIMPL_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
