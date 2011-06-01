#include <rtfdocumentimpl.hxx>
#include <rtftypes.hxx>
#include <rtfcontrolwords.hxx>
#include <rtfvalue.hxx>
#include <rtfsprm.hxx>
#include <rtfreferenceproperties.hxx>
#include <doctok/sprmids.hxx> // NS_sprm
// TODO: get rid of rtf and ooxml namespaces
#include <doctok/resourceids.hxx> // NS_rtf
#include <ooxml/resourceids.hxx> // NS_ooxml
#include <unotools/ucbstreamhelper.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/tencinfo.h>

using rtl::OString;
using rtl::OStringBuffer;
using rtl::OUString;
using rtl::OUStringBuffer;
using rtl::OUStringToOString;

namespace writerfilter {
namespace rtftok {

RTFDocumentImpl::RTFDocumentImpl(uno::Reference<io::XInputStream> const& xInputStream)
    : m_nGroup(0),
    m_aDefaultState(),
    m_bSkipUnknown(false),
    m_pCurrentKeyword(0),
    m_aFontEncodings(),
    m_aColorTable(),
    m_bFirstRun(true)
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

sal_uInt32 RTFDocumentImpl::getColorTable(sal_uInt32 nIndex)
{
    if (nIndex < m_aColorTable.size())
        return m_aColorTable[nIndex];
    return 0;
}

sal_uInt32 RTFDocumentImpl::getEncodingTable(sal_uInt32 nFontIndex)
{
    if (nFontIndex < m_aFontEncodings.size())
        return m_aFontEncodings[nFontIndex];
    return 0;
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

    if (m_aStates.top().nDestinationState == DESTINATION_NORMAL)
        text(aOUStr);
    else if (m_aStates.top().nDestinationState == DESTINATION_FONTENTRY)
    {
        // this is a font name, drop the ; at the end if it's there
        if (aOUStr.endsWithAsciiL(";", 1))
        {
            aOUStr = aOUStr.copy(0, aOUStr.getLength() - 1);
        }
        RTFValue::Pointer_t pValue(new RTFValue(aOUStr));
        m_aStates.top().aAttributes[NS_rtf::LN_XSZFFN] = pValue;
    }
    else if (m_aStates.top().nDestinationState == DESTINATION_COLORTABLE)
    {
        // we hit a ';' at the end of each color entry
        sal_uInt32 color = m_aStates.top().aCurrentColor.nRed | ( m_aStates.top().aCurrentColor.nGreen << 8)
            | (m_aStates.top().aCurrentColor.nBlue << 16);
        m_aColorTable.push_back(color);
        // set components back to zero
        m_aStates.top().aCurrentColor = RTFColorTableEntry();
    }
    else if (m_aStates.top().nDestinationState == DESTINATION_STYLEENTRY)
    {
        // this is a style name, drop the ; at the end if it's there
        if (aOUStr.endsWithAsciiL(";", 1))
        {
            aOUStr = aOUStr.copy(0, aOUStr.getLength() - 1);
        }
        RTFValue::Pointer_t pValue(new RTFValue(aOUStr));
        m_aStates.top().aAttributes[NS_rtf::LN_XSTZNAME1] = pValue;
    }

    return 0;
}

void RTFDocumentImpl::text(OUString& rString)
{
    writerfilter::Reference<Properties>::Pointer_t const pProperties(
            new RTFReferenceProperties(m_aStates.top().aAttributes, m_aStates.top().aSprms)
            );

    if (m_bFirstRun)
    {
        // start initial paragraph after the optional font/color/stylesheet tables
        Mapper().startSectionGroup();
        Mapper().startParagraphGroup();
        Mapper().props(pProperties);
        m_bFirstRun = false;
    }

    Mapper().startCharacterGroup();
    Mapper().props(pProperties);
    Mapper().utext(reinterpret_cast<sal_uInt8 const*>(rString.getStr()), rString.getLength());
    Mapper().endCharacterGroup();
}

int RTFDocumentImpl::dispatchDestination(RTFKeyword nKeyword)
{
    switch (nKeyword)
    {
        case RTF_RTF:
            break;
        case RTF_FONTTBL:
            m_aStates.top().nDestinationState = DESTINATION_FONTTABLE;
            break;
        case RTF_COLORTBL:
            m_aStates.top().nDestinationState = DESTINATION_COLORTABLE;
            break;
        case RTF_STYLESHEET:
            m_aStates.top().nDestinationState = DESTINATION_STYLESHEET;
            break;
        default:
            OSL_TRACE("%s: TODO handle destination '%s'", OSL_THIS_FUNC, m_pCurrentKeyword->getStr());
            // Make sure we skip destinations till we don't handle them
            m_aStates.top().nDestinationState = DESTINATION_SKIP;
            break;
    }

    return 0;
}

int RTFDocumentImpl::dispatchFlag(RTFKeyword nKeyword)
{
    switch (nKeyword)
    {
        case RTF_FNIL:
        case RTF_FROMAN:
        case RTF_FSWISS:
        case RTF_FMODERN:
        case RTF_FSCRIPT:
        case RTF_FDECOR:
        case RTF_FTECH:
        case RTF_FBIDI:
            // TODO ooxml:CT_Font_family seems to be ignored by the domain mapper
            break;
        case RTF_ANSI:
            m_aStates.top().nCurrentEncoding = RTL_TEXTENCODING_MS_1252;
            break;
        case RTF_PARD:
            m_aStates.top() = m_aDefaultState;
            break;
        default:
            OSL_TRACE("%s: TODO handle flag '%s'", OSL_THIS_FUNC, m_pCurrentKeyword->getStr());
            break;
    }
    return 0;
}

int RTFDocumentImpl::dispatchValue(RTFKeyword nKeyword, int nParam)
{
    switch (nKeyword)
    {
        case RTF_F:
            if (m_aStates.top().nDestinationState == DESTINATION_FONTENTRY)
                m_aStates.top().nCurrentFontIndex = nParam;
            else
            {
                RTFValue::Pointer_t pValue(new RTFValue(nParam));
                m_aStates.top().aSprms[NS_sprm::LN_CRgFtc0] = pValue;
                m_aStates.top().nCurrentEncoding = getEncodingTable(nParam);
            }
            break;
        case RTF_AF:
            {
                RTFValue::Pointer_t pValue(new RTFValue(nParam));
                m_aStates.top().aSprms[NS_sprm::LN_CRgFtc1] = pValue;
            }
            break;
        case RTF_FS:
            {
                RTFValue::Pointer_t pValue(new RTFValue(nParam));
                m_aStates.top().aSprms[NS_sprm::LN_CHps] = pValue;
            }
            break;
        case RTF_AFS:
            {
                RTFValue::Pointer_t pValue(new RTFValue(nParam));
                m_aStates.top().aSprms[NS_sprm::LN_CHpsBi] = pValue;
            }
            break;
        case RTF_FPRQ:
            {
                RTFValue::Pointer_t pValue(new RTFValue(nParam));
                m_aStates.top().aSprms[NS_rtf::LN_PRQ] = pValue;
            }
            break;
        case RTF_RED:
            m_aStates.top().aCurrentColor.nRed = nParam;
            break;
        case RTF_GREEN:
            m_aStates.top().aCurrentColor.nGreen = nParam;
            break;
        case RTF_BLUE:
            m_aStates.top().aCurrentColor.nBlue = nParam;
            break;
        case RTF_FCHARSET:
            {
                // we always send text to the domain mapper in OUString, so no
                // need to send encoding info
                int i;
                for (i = 0; i < nRTFEncodings; i++)
                {
                    if (aRTFEncodings[i].charset == nParam)
                        break;
                }
                if (i == nRTFEncodings)
                    // not found
                    return 0;
                m_aFontEncodings[m_aStates.top().nCurrentFontIndex] = rtl_getTextEncodingFromWindowsCodePage(aRTFEncodings[i].charset);
            }
            break;
        case RTF_CF:
            {
                RTFValue::Pointer_t pValue(new RTFValue(getColorTable(nParam)));
                // NS_sprm::LN_CIco won't work, that would be an index in a static table
                m_aStates.top().aAttributes[NS_ooxml::LN_CT_Color_val] = pValue;
            }
            break;
        case RTF_S:
            if (m_aStates.top().nDestinationState == DESTINATION_STYLEENTRY)
            {
                m_aStates.top().nCurrentStyleIndex = nParam;
                RTFValue::Pointer_t pValue(new RTFValue(nParam));
                m_aStates.top().aAttributes[NS_rtf::LN_ISTD] = pValue;
            }
            else
            {
                RTFValue::Pointer_t pValue(new RTFValue(nParam));
                m_aStates.top().aAttributes[NS_rtf::LN_ISTD] = pValue;
            }
            break;
        case RTF_SBASEDON:
            {
                RTFValue::Pointer_t pValue(new RTFValue(nParam));
                m_aStates.top().aAttributes[NS_rtf::LN_ISTDBASE] = pValue;
            }
            break;
        case RTF_SNEXT:
            {
                RTFValue::Pointer_t pValue(new RTFValue(nParam));
                m_aStates.top().aAttributes[NS_rtf::LN_ISTDNEXT] = pValue;
            }
            break;
        case RTF_DEFF:
            {
                RTFValue::Pointer_t pValue(new RTFValue(nParam));
                m_aDefaultState.aSprms[NS_sprm::LN_CRgFtc0] = pValue;
            }
            break;
        case RTF_DEFLANG:
            {
                RTFValue::Pointer_t pValue(new RTFValue(nParam));
                m_aDefaultState.aSprms[NS_sprm::LN_CRgLid0] = pValue;
            }
            break;
        case RTF_ADEFLANG:
            {
                RTFValue::Pointer_t pValue(new RTFValue(nParam));
                m_aDefaultState.aSprms[NS_sprm::LN_CLidBi] = pValue;
            }
            break;
        default:
            OSL_TRACE("%s: TODO handle value '%s'", OSL_THIS_FUNC, m_pCurrentKeyword->getStr());
            break;
    }
    return 0;
}

int RTFDocumentImpl::dispatchToggle(RTFKeyword nKeyword, bool bParam, int nParam)
{
    int nSprm = 0;

    switch (nKeyword)
    {
        case RTF_B:
            nSprm = NS_sprm::LN_CFBold;
            break;
        case RTF_AB:
            nSprm = NS_sprm::LN_CFBoldBi;
            break;
        case RTF_I:
            nSprm = NS_sprm::LN_CFItalic;
            break;
        case RTF_UL:
            nSprm = NS_sprm::LN_CKul;
            break;
        case RTF_OUTL:
            nSprm = NS_sprm::LN_CFOutline;
            break;
        default:
            OSL_TRACE("%s: TODO handle toggle '%s'", OSL_THIS_FUNC, m_pCurrentKeyword->getStr());
            break;
    }
    if (nSprm > 0)
    {
        RTFValue::Pointer_t pValue(new RTFValue(!bParam || nParam != 0));
        m_aStates.top().aSprms[nSprm] = pValue;
    }

    return 0;
}

int RTFDocumentImpl::dispatchKeyword(OString& rKeyword, bool bParam, int nParam)
{
    if (m_aStates.top().nDestinationState == DESTINATION_SKIP)
        return 0;
    OSL_TRACE("%s: keyword '\\%s' with param? %d param val: '%d'", OSL_THIS_FUNC,
            rKeyword.getStr(), (bParam ? 1 : 0), (bParam ? nParam : 0));
    int i, ret;
    for (i = 0; i < nRTFControlWords; i++)
    {
        if (!strcmp(rKeyword.getStr(), aRTFControlWords[i].sKeyword))
            break;
    }
    if (i == nRTFControlWords)
    {
        OSL_TRACE("%s: unknown keyword '\\%s'", OSL_THIS_FUNC, rKeyword.getStr());
        if (m_bSkipUnknown)
        {
            m_aStates.top().nDestinationState = DESTINATION_SKIP;
            m_bSkipUnknown = false;
        }
        return 0;
    }

    // known keyword, shoult not be skipped, even if starts with \*
    m_bSkipUnknown = false;

    m_pCurrentKeyword = &rKeyword;
    switch (aRTFControlWords[i].nControlType)
    {
        case CONTROL_FLAG:
            // flags ignore any parameter by definition
            if ((ret = dispatchFlag(aRTFControlWords[i].nIndex)))
                return ret;
            break;
        case CONTROL_DESTINATION:
            // same for destinations
            if ((ret = dispatchDestination(aRTFControlWords[i].nIndex)))
                return ret;
            break;
        case CONTROL_SYMBOL:
            switch (aRTFControlWords[i].nIndex)
            {
                case RTF_IGNORE:
                    m_bSkipUnknown = true;
                    break;
                case RTF_PAR:
                    {
                        // end previous paragraph
                        OUString aStr = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\x0d"));
                        text(aStr);
                        Mapper().endParagraphGroup();
                        // start new one
                        Mapper().startParagraphGroup();
                        writerfilter::Reference<Properties>::Pointer_t const pProperties(
                                new RTFReferenceProperties(m_aStates.top().aAttributes, m_aStates.top().aSprms)
                                );
                        Mapper().props(pProperties);
                    }
                    break;
                default:
                    OSL_TRACE("%s: TODO handle symbol '%s'", OSL_THIS_FUNC, rKeyword.getStr());
                    break;
            }
            break;
        case CONTROL_TOGGLE:
            if ((ret = dispatchToggle(aRTFControlWords[i].nIndex, bParam, nParam)))
                return ret;
            break;
        case CONTROL_VALUE:
            // values require a parameter by definition
            if (bParam && (ret = dispatchValue(aRTFControlWords[i].nIndex, nParam)))
                return ret;
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

    RTFParserState aState;
    if (!m_aStates.empty())
    {
        aState = m_aStates.top();
    }
    m_aStates.push(aState);

    m_nGroup++;

    if (m_aStates.top().nDestinationState == DESTINATION_FONTTABLE)
    {
        m_aStates.top().nDestinationState = DESTINATION_FONTENTRY;
    }
    else if (m_aStates.top().nDestinationState == DESTINATION_STYLESHEET)
    {
        m_aStates.top().nDestinationState = DESTINATION_STYLEENTRY;
    }

    return 0;
}

int RTFDocumentImpl::popState()
{
    OSL_TRACE("%s before pop: m_nGroup %d, dest state: %d", OSL_THIS_FUNC, m_nGroup, m_aStates.top().nDestinationState);

    RTFReferenceTable::Entry_t aEntry;
    bool bFontEntryEnd = false;
    bool bStyleEntryEnd = false;

    if (m_aStates.top().nDestinationState == DESTINATION_FONTTABLE)
    {
        writerfilter::Reference<Table>::Pointer_t const pTable(new RTFReferenceTable(m_aStates.top().aFontTableEntries));
        Mapper().table(NS_rtf::LN_FONTTABLE, pTable);
    }
    else if (m_aStates.top().nDestinationState == DESTINATION_STYLESHEET)
    {
        writerfilter::Reference<Table>::Pointer_t const pTable(new RTFReferenceTable(m_aStates.top().aStyleTableEntries));
        Mapper().table(NS_rtf::LN_STYLESHEET, pTable);
    }
    else if (m_aStates.top().nDestinationState == DESTINATION_FONTENTRY)
    {
        bFontEntryEnd = true;
        writerfilter::Reference<Properties>::Pointer_t const pProp(
                new RTFReferenceProperties(m_aStates.top().aAttributes, m_aStates.top().aSprms)
                );
        aEntry.first = m_aStates.top().nCurrentFontIndex;
        aEntry.second = pProp;
    }
    else if (m_aStates.top().nDestinationState == DESTINATION_STYLEENTRY)
    {
        bStyleEntryEnd = true;
        writerfilter::Reference<Properties>::Pointer_t const pProp(
                new RTFReferenceProperties(m_aStates.top().aAttributes, m_aStates.top().aSprms)
                );
        aEntry.first = m_aStates.top().nCurrentStyleIndex;
        aEntry.second = pProp;
    }

    m_aStates.pop();

    m_nGroup--;

    if (bFontEntryEnd)
        m_aStates.top().aFontTableEntries[aEntry.first] = aEntry.second;
    else if (bStyleEntryEnd)
        m_aStates.top().aStyleTableEntries[aEntry.first] = aEntry.second;

    if (!m_aStates.empty())
        OSL_TRACE("%s after pop: m_nGroup %d, dest state: %d", OSL_THIS_FUNC, m_nGroup, m_aStates.top().nDestinationState);
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
        if (!m_aStates.empty() && m_aStates.top().nInternalState == INTERNAL_BIN)
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
                    if (m_aStates.top().nInternalState == INTERNAL_NORMAL)
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

RTFParserState::RTFParserState()
    : nInternalState(INTERNAL_NORMAL),
    nDestinationState(DESTINATION_NORMAL),
    aSprms(),
    aAttributes(),
    aFontTableEntries(),
    nCurrentFontIndex(0),
    aCurrentColor(),
    aStyleTableEntries(),
    nCurrentStyleIndex(0),
    nCurrentEncoding(0)
{
}

RTFColorTableEntry::RTFColorTableEntry()
    : nRed(0),
    nGreen(0),
    nBlue(0)
{
}

} // namespace rtftok
} // namespace writerfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
