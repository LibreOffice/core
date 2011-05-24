#include <rtfdocumentimpl.hxx>

namespace writerfilter {
namespace rtftok {

class RTFDocumentImpl::Impl
{
    friend class RTFDocumentImpl;

    uno::Reference<io::XInputStream> m_xInputStream;

    explicit Impl(uno::Reference<io::XInputStream> const& xInputStream)
        : m_xInputStream(xInputStream)
    {
        OSL_ENSURE(xInputStream.is(), "no input stream");
        if (!xInputStream.is())
            throw uno::RuntimeException();
    }
};

RTFDocumentImpl::RTFDocumentImpl(uno::Reference<io::XInputStream> const& xInputStream)
    : m_pImpl( new Impl(xInputStream) )
{
}

RTFDocumentImpl::~RTFDocumentImpl()
{
}

void RTFDocumentImpl::resolve(Stream & /*rStream*/)
{
    OSL_TRACE("%s", OSL_THIS_FUNC);
}

::std::string RTFDocumentImpl::getType() const
{
    return "RTFDocumentImpl";
}

} // namespace rtftok
} // namespace writerfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
