#ifndef _RTFDOCUMENTIMPL_HXX_
#define _RTFDOCUMENTIMPL_HXX_

#include <memory>
#include <rtftok/RTFDocument.hxx>

namespace writerfilter {
    namespace rtftok {
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
            private:
                class Impl;
                ::std::auto_ptr<Impl> m_pImpl;
        };
    } // namespace rtftok
} // namespace writerfilter

#endif // _RTFDOCUMENTIMPL_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
