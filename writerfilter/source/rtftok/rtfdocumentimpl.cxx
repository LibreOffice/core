#include <rtfdocumentimpl.hxx>
#include <rtftypes.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>

using rtl::OString;
using rtl::OStringBuffer;
using rtl::OUString;
using rtl::OUStringBuffer;
using rtl::OUStringToOString;

namespace writerfilter {
namespace rtftok {

extern RTFSymbol aRTFControlWords[];
extern int nRTFControlWords;

RTFDocumentImpl::RTFDocumentImpl(uno::Reference<io::XInputStream> const& xInputStream)
    : m_nGroup(0)
{
    OSL_ENSURE(xInputStream.is(), "no input stream");
    if (!xInputStream.is())
        throw uno::RuntimeException();
    m_pInStream = utl::UcbStreamHelper::CreateStream( xInputStream, sal_True );
}

RTFDocumentImpl::~RTFDocumentImpl()
{
}

SvStream& RTFDocumentImpl::Strm()
{
    return *m_pInStream;
}

Stream& RTFDocumentImpl::Mapper()
{
    return *m_pMapperStream;
}

void RTFDocumentImpl::resolve(Stream & rMapper)
{
    m_pMapperStream = &rMapper;
    switch (resolveParse())
    {
        case ERROR_OK:
            OSL_TRACE("%s: finished without errors", OSL_THIS_FUNC);
            break;
        case ERROR_GROUP_UNDER:
            OSL_TRACE("%s: unmatched '}'", OSL_THIS_FUNC);
            break;
        case ERROR_GROUP_OVER:
            OSL_TRACE("%s: unmatched '{'", OSL_THIS_FUNC);
            break;
        case ERROR_EOF:
            OSL_TRACE("%s: unexpected end of file", OSL_THIS_FUNC);
            break;
    }
}

int RTFDocumentImpl::resolveChars(char ch)
{
    OSL_TRACE("%s start", OSL_THIS_FUNC);
    OStringBuffer aBuf;

    while(!Strm().IsEof() && ch != '{' && ch != '}' && ch != '\\')
    {
        if (ch != 0x0d && ch != 0x0a)
            aBuf.append(ch);
        Strm() >> ch;
    }
    if (!Strm().IsEof())
        Strm().SeekRel(-1);
    OString aStr = aBuf.makeStringAndClear();
    OSL_TRACE("%s: collected '%s'", OSL_THIS_FUNC, aStr.getStr());

    // TODO encoding handling
    OUString aOUStr(OStringToOUString(aStr, RTL_TEXTENCODING_UTF8));

    Mapper().utext(reinterpret_cast<sal_uInt8 const*>(aOUStr.getStr()), aOUStr.getLength());

    return 0;
}

int RTFDocumentImpl::dispatchKeyword(OString& rKeyword, bool bParam, int nParam)
{
    if (m_aState.nDestinationState == DESTINATION_SKIP)
        return 0;
    OSL_TRACE("%s: keyword '\\%s' with param? %d param val: '%d'", OSL_THIS_FUNC,
            rKeyword.getStr(), (bParam ? 1 : 0), (bParam ? nParam : 0));
    int i;
    for (i = 0; i < nRTFControlWords; i++)
    {
        if (!strcmp(rKeyword.getStr(), aRTFControlWords[i].sKeyword))
            break;
    }
    if (i == nRTFControlWords)
    {
        OSL_TRACE("%s: unknown keyword '\\%s'", OSL_THIS_FUNC, rKeyword.getStr());
        // TODO when an unknown keyword starts with \*, then it's a destination
        return 0;
    }

    switch (aRTFControlWords[i].nControlType)
    {
        case CONTROL_FLAG:
            OSL_TRACE("%s: TODO handle flag '%s'", OSL_THIS_FUNC, rKeyword.getStr());
            break;
        case CONTROL_DESTINATION:
            OSL_TRACE("%s: TODO handle destination '%s'", OSL_THIS_FUNC, rKeyword.getStr());
            // Make sure we skip destinations till we don't handle them
            m_aState.nDestinationState = DESTINATION_SKIP;
            break;
        case CONTROL_SYMBOL:
            OSL_TRACE("%s: TODO handle symbol '%s'", OSL_THIS_FUNC, rKeyword.getStr());
            break;
        case CONTROL_TOGGLE:
            OSL_TRACE("%s: TODO handle toggle '%s'", OSL_THIS_FUNC, rKeyword.getStr());
            break;
        case CONTROL_VALUE:
            OSL_TRACE("%s: TODO handle value '%s'", OSL_THIS_FUNC, rKeyword.getStr());
            break;
    }

    return 0;
}

int RTFDocumentImpl::resolveKeyword()
{
    OSL_TRACE("%s", OSL_THIS_FUNC);
    char ch;
    OStringBuffer aBuf;
    bool bNeg = false;
    bool bParam = false;
    int nParam = 0;

    Strm() >> ch;
    if (Strm().IsEof())
        return ERROR_EOF;

    if (!isalpha(ch))
    {
        aBuf.append(ch);
        OString aKeyword = aBuf.makeStringAndClear();
        // control symbols aren't followed by a space, so we can return here
        // without doing any SeekRel()
        return dispatchKeyword(aKeyword, bParam, nParam);
    }
    while(isalpha(ch))
    {
        aBuf.append(ch);
        Strm() >> ch;
    }

    if (ch == '-')
    {
        // in case we'll have a parameter, that will be negative
        bNeg = true;
        Strm() >> ch;
        if (Strm().IsEof())
            return ERROR_EOF;
    }
    if (isdigit(ch))
    {
        OStringBuffer aParameter;

        // we have a parameter
        bParam = true;
        while(isdigit(ch))
        {
            aParameter.append(ch);
            Strm() >> ch;
        }
        nParam = aParameter.makeStringAndClear().toInt32();
        if (bNeg)
            nParam = -nParam;
    }
    if (ch != ' ')
        Strm().SeekRel(-1);
    OString aKeyword = aBuf.makeStringAndClear();
    return dispatchKeyword(aKeyword, bParam, nParam);
}

int RTFDocumentImpl::pushState()
{
    OSL_TRACE("%s before push: %d", OSL_THIS_FUNC, m_nGroup);

    // TODO save character, paragraph properties etc

    m_nGroup++;

    return 0;
}

int RTFDocumentImpl::popState()
{
    OSL_TRACE("%s before pop: %d", OSL_THIS_FUNC, m_nGroup);

    // TODO restore character, paragraph properties etc

    m_nGroup--;

    return 0;
}

int RTFDocumentImpl::resolveParse()
{
    OSL_TRACE("%s", OSL_THIS_FUNC);
    char ch;
    int ret;

    while ((Strm() >> ch, !Strm().IsEof()))
    {
        OSL_TRACE("%s: parsing character '%c'", OSL_THIS_FUNC, ch);

        if (m_nGroup < 0)
            return ERROR_GROUP_UNDER;
        if (m_aState.nInternalState == INTERNAL_BIN)
        {
            OSL_TRACE("%s: TODO, binary internal state", OSL_THIS_FUNC);
        }
        else
        {
            switch (ch)
            {
                case '{':
                    if ((ret = pushState()))
                        return ret;
                    break;
                case '}':
                    if ((ret = popState()))
                        return ret;
                    break;
                case '\\':
                    if ((ret = resolveKeyword()))
                        return ret;
                    break;
                case 0x0d:
                case 0x0a:
                    break; // ignore these
                default:
                    if (m_aState.nInternalState == INTERNAL_NORMAL)
                    {
                        if ((ret = resolveChars(ch)))
                            return ret;
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
        return ERROR_GROUP_UNDER;
    else if (m_nGroup > 0)
        return ERROR_GROUP_OVER;
    return 0;
}

::std::string RTFDocumentImpl::getType() const
{
    return "RTFDocumentImpl";
}

} // namespace rtftok
} // namespace writerfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
