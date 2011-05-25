#include <rtfdocumentimpl.hxx>
#include <unotools/ucbstreamhelper.hxx>

namespace writerfilter {
namespace rtftok {

RTFDocumentImpl::RTFDocumentImpl(uno::Reference<io::XInputStream> const& xInputStream)
    : m_nGroup(0),
    m_nInternalState(INTERNAL_NORMAL)
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

    while (!Strm().IsEof())
    {
        Strm() >> ch;
        OSL_TRACE("%s: parsing character '%c'", OSL_THIS_FUNC, ch);

        if (m_nGroup < 0)
        {
            OSL_TRACE("%s: unmatched '}'", OSL_THIS_FUNC);
            return;
        }
        if (m_nInternalState == INTERNAL_BIN)
        {
            OSL_TRACE("%s: TODO, binary internal state", OSL_THIS_FUNC);
        }
        else
        {
            switch (ch)
            {
                case '{':
                    OSL_TRACE("%s: TODO, push rtf state", OSL_THIS_FUNC);
                    break;
                case '}':
                    OSL_TRACE("%s: TODO, pop rtf state", OSL_THIS_FUNC);
                    break;
                case '\\':
                    OSL_TRACE("%s: TODO, parse rtf keyword", OSL_THIS_FUNC);
                    break;
                case 0x0d:
                case 0x0a:
                    break; // ignore these
                default:
                    if (m_nInternalState == INTERNAL_NORMAL)
                    {
                        OSL_TRACE("%s: TODO, parse character", OSL_THIS_FUNC);
                    }
                    else
                    {
                        OSL_TRACE("%s: TODO, hex internal state", OSL_THIS_FUNC);
                    }
                    break;
            }
        }
    }

    if (m_nGroup < 0)
        OSL_TRACE("%s: unmatched '}'", OSL_THIS_FUNC);
    else if (m_nGroup > 0)
        OSL_TRACE("%s: unmatched '{'", OSL_THIS_FUNC);
}

::std::string RTFDocumentImpl::getType() const
{
    return "RTFDocumentImpl";
}

} // namespace rtftok
} // namespace writerfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
