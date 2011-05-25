#include <rtfdocumentimpl.hxx>
#include <unotools/ucbstreamhelper.hxx>

namespace writerfilter {
namespace rtftok {

RTFDocumentImpl::RTFDocumentImpl(uno::Reference<io::XInputStream> const& xInputStream)
{
    OSL_ENSURE(xInputStream.is(), "no input stream");
    if (!xInputStream.is())
        throw uno::RuntimeException();
    m_pStream = utl::UcbStreamHelper::CreateStream( xInputStream, sal_True );
}

RTFDocumentImpl::~RTFDocumentImpl()
{
}

SvStream& RTFDocumentImpl::Strm()
{
    return *m_pStream;
}

void RTFDocumentImpl::resolve(Stream & /*rStream*/)
{
    OSL_TRACE("%s", OSL_THIS_FUNC);
    char ch;

    while (!Strm().IsEof()) {
        Strm() >> ch;
        OSL_TRACE("%c", ch);
    }
}

::std::string RTFDocumentImpl::getType() const
{
    return "RTFDocumentImpl";
}

} // namespace rtftok
} // namespace writerfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
