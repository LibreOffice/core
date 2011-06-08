#include <rtfdocumentimpl.hxx>
#include <rtftypes.hxx>
#include <rtfcontrolwords.hxx>
#include <rtfvalue.hxx>
#include <rtfsprm.hxx>
#include <rtfreferenceproperties.hxx>
// TODO having 3 separate namespaces for the same purpose is... ugly.
#include <doctok/sprmids.hxx> // NS_sprm
#include <doctok/resourceids.hxx> // NS_rtf
#include <ooxml/resourceids.hxx> // NS_ooxml
#include <unotools/ucbstreamhelper.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/tencinfo.h>
#include <svl/lngmisc.hxx>
#include <editeng/borderline.hxx>

using std::make_pair;
using rtl::OString;
using rtl::OStringBuffer;
using rtl::OUString;
using rtl::OUStringBuffer;
using rtl::OUStringToOString;

namespace writerfilter {
namespace rtftok {

std::multimap<Id, RTFValue::Pointer_t>& lcl_getTabsTab(std::stack<RTFParserState>& aStates)
{
    // insert the tabs sprm if necessary
    std::multimap<Id, RTFValue::Pointer_t>::iterator itTabs = aStates.top().aSprms.find(NS_ooxml::LN_CT_PPrBase_tabs);
    if (itTabs == aStates.top().aSprms.end())
    {
        std::multimap<Id, RTFValue::Pointer_t> aTabsAttributes;
        std::multimap<Id, RTFValue::Pointer_t> aTabsSprms;
        RTFValue::Pointer_t pTabsValue(new RTFValue(aTabsAttributes, aTabsSprms));
        aStates.top().aSprms.insert(make_pair(NS_ooxml::LN_CT_PPrBase_tabs, pTabsValue));
        itTabs = aStates.top().aSprms.find(NS_ooxml::LN_CT_PPrBase_tabs);
    }
    std::multimap<Id, RTFValue::Pointer_t>& rSprms = itTabs->second->getSprms();

    // insert the tab sprm if necessary
    std::multimap<Id, RTFValue::Pointer_t>::iterator itTab = rSprms.find(NS_ooxml::LN_CT_Tabs_tab);
    if (itTab == rSprms.end())
    {
        std::multimap<Id, RTFValue::Pointer_t> aTabAttributes;
        RTFValue::Pointer_t pTabValue(new RTFValue(aTabAttributes));
        rSprms.insert(make_pair(NS_ooxml::LN_CT_Tabs_tab, pTabValue));
        itTab = rSprms.find(NS_ooxml::LN_CT_Tabs_tab);
    }
    return itTab->second->getAttributes();
}

std::multimap<Id, RTFValue::Pointer_t>& lcl_getNumPr(std::stack<RTFParserState>& aStates)
{
    // insert the numpr sprm if necessary
    std::multimap<Id, RTFValue::Pointer_t>::iterator it = aStates.top().aSprms.find(NS_ooxml::LN_CT_PPrBase_numPr);
    if (it == aStates.top().aSprms.end())
    {
        std::multimap<Id, RTFValue::Pointer_t> aAttributes;
        std::multimap<Id, RTFValue::Pointer_t> aSprms;
        RTFValue::Pointer_t pValue(new RTFValue(aAttributes, aSprms));
        aStates.top().aSprms.insert(make_pair(NS_ooxml::LN_CT_PPrBase_numPr, pValue));
        it = aStates.top().aSprms.find(NS_ooxml::LN_CT_PPrBase_numPr);
    }
    return it->second->getSprms();
}

RTFDocumentImpl::RTFDocumentImpl(uno::Reference<io::XInputStream> const& xInputStream)
    : m_nGroup(0),
    m_aDefaultState(),
    m_bSkipUnknown(false),
    m_pCurrentKeyword(0),
    m_aFontEncodings(),
    m_aColorTable(),
    m_bFirstRun(true),
    m_bNeedPap(false),
    m_aListTableSprms()
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

Id RTFDocumentImpl::getBorderTable(sal_uInt32 nIndex)
{
    static const Id aBorderIds[] =
    {
        NS_sprm::LN_PBrcTop, NS_sprm::LN_PBrcLeft, NS_sprm::LN_PBrcBottom, NS_sprm::LN_PBrcRight
    };

    return aBorderIds[nIndex];
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
        case ERROR_HEX_INVALID:
            OSL_TRACE("%s: invalid hex char", OSL_THIS_FUNC);
            break;
    }
}

int RTFDocumentImpl::resolveChars(char ch)
{
    OStringBuffer aBuf;

    while(!Strm().IsEof() && ch != '{' && ch != '}' && ch != '\\')
    {
        if (ch != 0x0d && ch != 0x0a)
        {
            if (m_aStates.top().nCharsToSkip == 0)
                aBuf.append(ch);
            else
                m_aStates.top().nCharsToSkip--;
        }
        // read a single char if we're in hex mode
        if (m_aStates.top().nInternalState == INTERNAL_HEX)
            break;
        Strm() >> ch;
    }
    if (m_aStates.top().nInternalState != INTERNAL_HEX && !Strm().IsEof())
        Strm().SeekRel(-1);
    if (m_aStates.top().nDestinationState == DESTINATION_SKIP)
        return 0;
    OString aStr = aBuf.makeStringAndClear();
    OSL_TRACE("%s: collected '%s'", OSL_THIS_FUNC, aStr.getStr());

    OUString aOUStr(OStringToOUString(aStr, m_aStates.top().nCurrentEncoding));

    if (m_aStates.top().nDestinationState == DESTINATION_NORMAL || m_aStates.top().nDestinationState == DESTINATION_FIELDRESULT)
        text(aOUStr);
    else if (m_aStates.top().nDestinationState == DESTINATION_FONTENTRY)
    {
        // this is a font name, drop the ; at the end if it's there
        if (aOUStr.endsWithAsciiL(";", 1))
        {
            aOUStr = aOUStr.copy(0, aOUStr.getLength() - 1);
        }
        RTFValue::Pointer_t pValue(new RTFValue(aOUStr));
        m_aStates.top().aAttributes.insert(make_pair(NS_rtf::LN_XSZFFN, pValue));
    }
    else if (m_aStates.top().nDestinationState == DESTINATION_COLORTABLE)
    {
        // we hit a ';' at the end of each color entry
        sal_uInt32 color = (m_aStates.top().aCurrentColor.nRed << 16) | ( m_aStates.top().aCurrentColor.nGreen << 8)
            | m_aStates.top().aCurrentColor.nBlue;
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
        m_aStates.top().aAttributes.insert(make_pair(NS_rtf::LN_XSTZNAME1, pValue));
    }
    else if (m_aStates.top().nDestinationState == DESTINATION_FIELDINSTRUCTION)
        m_aStates.top().aFieldInstruction.append(aStr);

    return 0;
}

void RTFDocumentImpl::text(OUString& rString)
{
    if (m_aStates.top().nDestinationState == DESTINATION_LEVELTEXT)
    {
        OSL_TRACE("found text in leveltext destination");
        RTFValue::Pointer_t pValue(new RTFValue(rString));
        m_aStates.top().aAttributes.insert(make_pair(NS_ooxml::LN_CT_LevelText_val, pValue));
        return;
    }

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
    if (m_bNeedPap)
    {
        writerfilter::Reference<Properties>::Pointer_t const pPaProperties(
                new RTFReferenceProperties(m_aStates.top().aAttributes, m_aStates.top().aSprms)
                );
        Mapper().props(pPaProperties);
        m_bNeedPap = false;
    }

    if (m_aStates.top().nDestinationState == DESTINATION_FIELDINSTRUCTION)
    {
        sal_uInt8 sFieldStart[] = { 0x13 };
        Mapper().startCharacterGroup();
        Mapper().text(sFieldStart, 1);
        Mapper().endCharacterGroup();
    }
    Mapper().startCharacterGroup();
    if (m_aStates.top().nDestinationState == DESTINATION_NORMAL || m_aStates.top().nDestinationState == DESTINATION_FIELDRESULT)
        Mapper().props(pProperties);
    Mapper().utext(reinterpret_cast<sal_uInt8 const*>(rString.getStr()), rString.getLength());
    Mapper().endCharacterGroup();
    if (m_aStates.top().nDestinationState == DESTINATION_FIELDINSTRUCTION)
    {
        sal_uInt8 sFieldSep[] = { 0x14 };
        Mapper().startCharacterGroup();
        Mapper().text(sFieldSep, 1);
        Mapper().endCharacterGroup();
    }
    else if (m_aStates.top().nDestinationState == DESTINATION_FIELDRESULT)
    {
        sal_uInt8 sFieldEnd[] = { 0x15 };
        Mapper().startCharacterGroup();
        Mapper().text(sFieldEnd, 1);
        Mapper().endCharacterGroup();
    }
}

int RTFDocumentImpl::dispatchDestination(RTFKeyword nKeyword)
{
    bool bParsed = true;
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
        case RTF_FIELD:
            // A field consists of an fldinst and an fldrslt group.
            break;
        case RTF_FLDINST:
            m_aStates.top().nDestinationState = DESTINATION_FIELDINSTRUCTION;
            break;
        case RTF_FLDRSLT:
            m_aStates.top().nDestinationState = DESTINATION_FIELDRESULT;
            break;
        case RTF_LISTTABLE:
            m_aStates.top().nDestinationState = DESTINATION_LISTTABLE;
            break;
        case RTF_LIST:
            m_aStates.top().nDestinationState = DESTINATION_LISTENTRY;
            break;
        case RTF_LISTOVERRIDETABLE:
            m_aStates.top().nDestinationState = DESTINATION_LISTOVERRIDETABLE;
            break;
        case RTF_LISTOVERRIDE:
            m_aStates.top().nDestinationState = DESTINATION_LISTOVERRIDEENTRY;
            break;
        case RTF_LISTLEVEL:
            m_aStates.top().nDestinationState = DESTINATION_LISTLEVEL;
            break;
        case RTF_LISTTEXT:
            // Should be ignored by any reader that understands Word 97 through Word 2007 numbering.
            m_aStates.top().nDestinationState = DESTINATION_SKIP;
            break;
        case RTF_LEVELTEXT:
            OSL_TRACE("entered leveltext destination");
            m_aStates.top().nDestinationState = DESTINATION_LEVELTEXT;
            break;
        default:
            OSL_TRACE("%s: TODO handle destination '%s'", OSL_THIS_FUNC, m_pCurrentKeyword->getStr());
            // Make sure we skip destinations (even without \*) till we don't handle them
            m_aStates.top().nDestinationState = DESTINATION_SKIP;
            bParsed = false;
            break;
    }

    skipDestination(bParsed);
    return 0;
}

int RTFDocumentImpl::dispatchSymbol(RTFKeyword nKeyword)
{
    bool bParsed = true;
    switch (nKeyword)
    {
        case RTF_IGNORE:
            m_bSkipUnknown = true;
            return 0; // don't reset m_bSkipUnknown after this keyword
            break;
        case RTF_PAR:
            {
                // end previous paragraph
                OUString aStr = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\x0d"));
                text(aStr);
                Mapper().endParagraphGroup();
                // start new one
                Mapper().startParagraphGroup();
                // but don't emit properties yet, since they may change till the first text token arrives
                m_bNeedPap = true;
            }
            break;
        case RTF_BACKSLASH:
            if (m_aStates.top().nDestinationState == DESTINATION_FIELDINSTRUCTION)
                m_aStates.top().aFieldInstruction.append('\\');
            else
                OSL_TRACE("%s: TODO handle symbol '%s' outside fields", OSL_THIS_FUNC, m_pCurrentKeyword->getStr());
            break;
        case RTF_NOBREAK:
            {
                OUString aStr(SVT_HARD_SPACE);
                text(aStr);
            }
            break;
        case RTF_NOBRKHYPH:
            {
                OUString aStr(SVT_HARD_HYPHEN);
                text(aStr);
            }
            break;
        case RTF_OPTHYPH:
            {
                OUString aStr(SVT_SOFT_HYPHEN);
                text(aStr);
            }
            break;
        case RTF_TAB:
            {
                OUString aStr('\t');
                text(aStr);
            }
            break;
        case RTF_HEXCHAR:
            m_aStates.top().nInternalState = INTERNAL_HEX;
            break;
        default:
            OSL_TRACE("%s: TODO handle symbol '%s'", OSL_THIS_FUNC, m_pCurrentKeyword->getStr());
            bParsed = false;
            break;
    }
    skipDestination(bParsed);
    return 0;
}

int RTFDocumentImpl::dispatchFlag(RTFKeyword nKeyword)
{
    bool bParsed = true;
    int nParam = 0;

    // Indentation
    switch (nKeyword)
    {
        case RTF_QC: nParam = 1; break;
        case RTF_QJ: nParam = 3; break;
        case RTF_QL: nParam = 0; break;
        case RTF_QR: nParam = 2; break;
        case RTF_QD: nParam = 4; break;
        default: break;
    }
    if (nParam > 0)
    {
        RTFValue::Pointer_t pValue(new RTFValue(nParam));
        m_aStates.top().aSprms.insert(make_pair(NS_sprm::LN_PJc, pValue));
        skipDestination(bParsed);
        return 0;
    }

    // Tab kind.
    switch (nKeyword)
    {
        case RTF_TQR: nParam = 2; break;
        case RTF_TQC: nParam = 1; break;
        case RTF_TQDEC: nParam = 3; break;
        default: break;
    }
    if (nParam > 0)
    {
        std::multimap<Id, RTFValue::Pointer_t>& rAttributes = lcl_getTabsTab(m_aStates);
        RTFValue::Pointer_t pValue(new RTFValue(nParam));
        rAttributes.insert(make_pair(NS_ooxml::LN_CT_TabStop_val, pValue));
        skipDestination(bParsed);
        return 0;
    }

    // Tab lead.
    switch (nKeyword)
    {
        case RTF_TLDOT: nParam = 1; break;
        case RTF_TLMDOT: nParam = NS_ooxml::LN_Value_ST_TabTlc_middleDot; break;
        case RTF_TLHYPH: nParam = 2; break;
        case RTF_TLUL: nParam = 3; break;
        case RTF_TLTH: nParam = 2; break; // thick line is not supported by dmapper, this is just a hack
        case RTF_TLEQ: nParam = 0; break; // equal sign isn't, either
        default: break;
    }
    if (nParam > 0)
    {
        std::multimap<Id, RTFValue::Pointer_t>& rAttributes = lcl_getTabsTab(m_aStates);
        RTFValue::Pointer_t pValue(new RTFValue(nParam));
        rAttributes.insert(make_pair(NS_ooxml::LN_CT_TabStop_leader, pValue));
        skipDestination(bParsed);
        return 0;
    }

    // Border types
    switch (nKeyword)
    {
        case RTF_BRDRS: nParam = editeng::SOLID; break;
        case RTF_BRDRDOT: nParam = editeng::DOTTED; break;
        case RTF_BRDRDASH: nParam = editeng::DASHED; break;
        case RTF_BRDRDB: nParam = editeng::DOUBLE; break;
        case RTF_BRDRTNTHSG: nParam = editeng::THINTHICK_SMALLGAP; break;
        case RTF_BRDRTNTHMG: nParam = editeng::THINTHICK_MEDIUMGAP; break;
        case RTF_BRDRTNTHLG: nParam = editeng::THINTHICK_LARGEGAP; break;
        case RTF_BRDRTHTNSG: nParam = editeng::THICKTHIN_SMALLGAP; break;
        case RTF_BRDRTHTNMG: nParam = editeng::THICKTHIN_MEDIUMGAP; break;
        case RTF_BRDRTHTNLG: nParam = editeng::THICKTHIN_LARGEGAP; break;
        case RTF_BRDREMBOSS: nParam = editeng::EMBOSSED; break;
        case RTF_BRDRENGRAVE: nParam = editeng::ENGRAVED; break;
        case RTF_BRDROUTSET: nParam = editeng::OUTSET; break;
        case RTF_BRDRINSET: nParam = editeng::INSET; break;
        case RTF_BRDRNONE: nParam = editeng::NO_STYLE; break;
        default: break;
    }
    if (nParam > 0)
    {
        RTFValue::Pointer_t pValue(new RTFValue(nParam));

        for (int i = 0; i < 4; i++)
        {
            std::multimap<Id, RTFValue::Pointer_t>::iterator it = m_aStates.top().aSprms.find(getBorderTable(i));
            if (it != m_aStates.top().aSprms.end())
            {
                std::multimap<Id, RTFValue::Pointer_t>& rAttributes = it->second->getAttributes();
                rAttributes.insert(make_pair(NS_rtf::LN_BRCTYPE, pValue));
            }
        }
        skipDestination(bParsed);
        return 0;
    }

    // Trivial flags
    switch (nKeyword)
    {
        case RTF_KEEP: nParam = NS_sprm::LN_PFKeep; break;
        case RTF_KEEPN: nParam = NS_sprm::LN_PFKeepFollow; break;
        case RTF_WIDCTLPAR: nParam = NS_sprm::LN_PFWidowControl; break;
        default: break;
    }
    if (nParam > 0)
    {
        RTFValue::Pointer_t pValue(new RTFValue(1));
        m_aStates.top().aSprms.insert(make_pair(nParam, pValue));
        skipDestination(bParsed);
        return 0;
    }

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
        case RTF_NOWIDCTLPAR:
            {
                RTFValue::Pointer_t pValue(new RTFValue(0));
                m_aStates.top().aSprms.insert(make_pair(NS_sprm::LN_PFWidowControl, pValue));
            }
            break;
        case RTF_BOX:
            {
                std::multimap<Id, RTFValue::Pointer_t> aAttributes;
                RTFValue::Pointer_t pValue(new RTFValue(aAttributes));
                m_aStates.top().aSprms.insert(make_pair(NS_sprm::LN_PBrcTop, pValue));
                m_aStates.top().aSprms.insert(make_pair(NS_sprm::LN_PBrcLeft, pValue));
                m_aStates.top().aSprms.insert(make_pair(NS_sprm::LN_PBrcBottom, pValue));
                m_aStates.top().aSprms.insert(make_pair(NS_sprm::LN_PBrcRight, pValue));
            }
            break;
        default:
            OSL_TRACE("%s: TODO handle flag '%s'", OSL_THIS_FUNC, m_pCurrentKeyword->getStr());
            bParsed = false;
            break;
    }
    skipDestination(bParsed);
    return 0;
}

int RTFDocumentImpl::dispatchValue(RTFKeyword nKeyword, int nParam)
{
    bool bParsed = true;
    int nSprm = 0;
    // Trivial sprms.
    switch (nKeyword)
    {
        case RTF_AF: nSprm = NS_sprm::LN_CRgFtc1; break;
        case RTF_FS: nSprm = NS_sprm::LN_CHps; break;
        case RTF_AFS: nSprm = NS_sprm::LN_CHpsBi; break;
        case RTF_FPRQ: nSprm = NS_rtf::LN_PRQ; break;
        case RTF_ANIMTEXT: nSprm = NS_sprm::LN_CSfxText; break;
        case RTF_EXPNDTW: nSprm = NS_sprm::LN_CDxaSpace; break;
        case RTF_KERNING: nSprm = NS_sprm::LN_CHpsKern; break;
        case RTF_CHARSCALEX: nSprm = NS_sprm::LN_CCharScale; break;
        case RTF_FI: nSprm = NS_sprm::LN_PDxaLeft1; break;
        case RTF_LI: nSprm = NS_sprm::LN_PDxaLeft; break;
        case RTF_LIN: nSprm = 0x845e; break;
        case RTF_RI: nSprm = NS_sprm::LN_PDxaRight; break;
        case RTF_RIN: nSprm = 0x845d; break;
        case RTF_SB: nSprm = NS_sprm::LN_PDyaBefore; break;
        case RTF_SA: nSprm = NS_sprm::LN_PDyaAfter; break;
        case RTF_LANG: nSprm = NS_sprm::LN_CRgLid0; break;
        case RTF_LANGFE: nSprm = NS_sprm::LN_CRgLid1; break;
        default: break;
    }
    if (nSprm > 0)
    {
        RTFValue::Pointer_t pValue(new RTFValue(nParam));
        m_aStates.top().aSprms.insert(make_pair(nSprm, pValue));
        skipDestination(bParsed);
        return 0;
    }

    // Trivial attributes.
    switch (nKeyword)
    {
        case RTF_SBASEDON: nSprm = NS_rtf::LN_ISTDBASE; break;
        case RTF_SNEXT: nSprm = NS_rtf::LN_ISTDNEXT; break;
        // NS_sprm::LN_PDyaLine could be used, but that won't work with slmult
        case RTF_SL: nSprm = NS_ooxml::LN_CT_Spacing_line; break;
        default: break;
    }
    if (nSprm > 0)
    {
        RTFValue::Pointer_t pValue(new RTFValue(nParam));
        m_aStates.top().aAttributes.insert(make_pair(nSprm, pValue));
        skipDestination(bParsed);
        return 0;
    }

    // Then check for the more complex ones.
    switch (nKeyword)
    {
        case RTF_F:
            if (m_aStates.top().nDestinationState == DESTINATION_FONTENTRY)
                m_aStates.top().nCurrentFontIndex = nParam;
            else
            {
                RTFValue::Pointer_t pValue(new RTFValue(nParam));
                m_aStates.top().aSprms.insert(make_pair(NS_sprm::LN_CRgFtc0, pValue));
                m_aStates.top().nCurrentEncoding = getEncodingTable(nParam);
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
                m_aFontEncodings[m_aStates.top().nCurrentFontIndex] = rtl_getTextEncodingFromWindowsCodePage(aRTFEncodings[i].codepage);
            }
            break;
        case RTF_CF:
            {
                RTFValue::Pointer_t pValue(new RTFValue(getColorTable(nParam)));
                // NS_sprm::LN_CIco won't work, that would be an index in a static table
                m_aStates.top().aAttributes.insert(make_pair(NS_ooxml::LN_CT_Color_val, pValue));
            }
            break;
        case RTF_S:
            if (m_aStates.top().nDestinationState == DESTINATION_STYLEENTRY)
            {
                m_aStates.top().nCurrentStyleIndex = nParam;
                RTFValue::Pointer_t pValue(new RTFValue(nParam));
                m_aStates.top().aAttributes.insert(make_pair(NS_rtf::LN_ISTD, pValue));
            }
            else
            {
                RTFValue::Pointer_t pValue(new RTFValue(nParam));
                m_aStates.top().aAttributes.insert(make_pair(NS_rtf::LN_ISTD, pValue));
            }
            break;
        case RTF_CS:
            if (m_aStates.top().nDestinationState == DESTINATION_STYLEENTRY)
            {
                m_aStates.top().nCurrentStyleIndex = nParam;
                RTFValue::Pointer_t pValue(new RTFValue(nParam));
                m_aStates.top().aAttributes.insert(make_pair(NS_rtf::LN_ISTD, pValue));
                RTFValue::Pointer_t pTValue(new RTFValue(2));
                m_aStates.top().aAttributes.insert(make_pair(NS_rtf::LN_SGC, pTValue)); // character style
            }
            else
            {
                RTFValue::Pointer_t pValue(new RTFValue(nParam));
                m_aStates.top().aAttributes.insert(make_pair(NS_rtf::LN_ISTD, pValue));
            }
            break;
        case RTF_DEFF:
            {
                RTFValue::Pointer_t pValue(new RTFValue(nParam));
                m_aDefaultState.aSprms.insert(make_pair(NS_sprm::LN_CRgFtc0, pValue));
            }
            break;
        case RTF_DEFLANG:
            {
                RTFValue::Pointer_t pValue(new RTFValue(nParam));
                m_aDefaultState.aSprms.insert(make_pair(NS_sprm::LN_CRgLid0, pValue));
            }
            break;
        case RTF_ADEFLANG:
            {
                RTFValue::Pointer_t pValue(new RTFValue(nParam));
                m_aDefaultState.aSprms.insert(make_pair(NS_sprm::LN_CLidBi, pValue));
            }
            break;
        case RTF_CHCBPAT:
            {
                std::multimap<Id, RTFValue::Pointer_t> aAttributes;
                RTFValue::Pointer_t pInnerValue(new RTFValue(getColorTable(nParam)));
                aAttributes.insert(make_pair(NS_ooxml::LN_CT_Shd_fill, pInnerValue));
                RTFValue::Pointer_t pValue(new RTFValue(aAttributes));
                m_aStates.top().aSprms.insert(make_pair(NS_sprm::LN_CShd, pValue));
            }
            break;
        case RTF_CBPAT:
            {
                std::multimap<Id, RTFValue::Pointer_t> aAttributes;
                RTFValue::Pointer_t pInnerValue(new RTFValue(getColorTable(nParam)));
                aAttributes.insert(make_pair(NS_ooxml::LN_CT_Shd_fill, pInnerValue));
                RTFValue::Pointer_t pValue(new RTFValue(aAttributes));
                m_aStates.top().aSprms.insert(make_pair(NS_sprm::LN_PShd, pValue));
            }
            break;
        case RTF_ULC:
            {
                RTFValue::Pointer_t pValue(new RTFValue(getColorTable(nParam)));
                m_aStates.top().aSprms.insert(make_pair(0x6877, pValue));
            }
            break;
        case RTF_UP:
            {
                OUString aValue(RTL_CONSTASCII_USTRINGPARAM("superscript"));
                RTFValue::Pointer_t pValue(new RTFValue(aValue));
                m_aStates.top().aSprms.insert(make_pair(NS_ooxml::LN_EG_RPrBase_vertAlign, pValue));
            }
            break;
        case RTF_DN:
            {
                OUString aValue(RTL_CONSTASCII_USTRINGPARAM("subscript"));
                RTFValue::Pointer_t pValue(new RTFValue(aValue));
                m_aStates.top().aSprms.insert(make_pair(NS_ooxml::LN_EG_RPrBase_vertAlign, pValue));
            }
            break;
        case RTF_HORZVERT:
            {
                RTFValue::Pointer_t pValue(new RTFValue(true));
                m_aStates.top().aAttributes.insert(make_pair(NS_ooxml::LN_CT_EastAsianLayout_vert, pValue));
                if (nParam)
                    // rotate fits to a single line
                    m_aStates.top().aAttributes.insert(make_pair(NS_ooxml::LN_CT_EastAsianLayout_vertCompress, pValue));
            }
            break;
        case RTF_EXPND:
            {
                RTFValue::Pointer_t pValue(new RTFValue(nParam/5));
                m_aStates.top().aSprms.insert(make_pair(NS_sprm::LN_CDxaSpace, pValue));
            }
            break;
        case RTF_TWOINONE:
            {
                RTFValue::Pointer_t pValue(new RTFValue(true));
                m_aStates.top().aAttributes.insert(make_pair(NS_ooxml::LN_CT_EastAsianLayout_combine, pValue));
                if (nParam > 0)
                {
                    RTFValue::Pointer_t pBValue(new RTFValue(nParam));
                    m_aStates.top().aAttributes.insert(make_pair(NS_ooxml::LN_CT_EastAsianLayout_combineBrackets, pBValue));
                }
            }
            break;
        case RTF_SLMULT:
            if (nParam > 0)
            {
                RTFValue::Pointer_t pValue(new RTFValue(NS_ooxml::LN_Value_wordprocessingml_ST_LineSpacingRule_auto));
                m_aStates.top().aAttributes.insert(make_pair(NS_ooxml::LN_CT_Spacing_lineRule, pValue));
            }
            break;
        case RTF_BRDRW:
            {
                // dmapper expects it in 1/8 pt, we have it in twip
                RTFValue::Pointer_t pValue(new RTFValue(nParam * 2 / 5));

                for (int i = 0; i < 4; i++)
                {
                    std::multimap<Id, RTFValue::Pointer_t>::iterator it = m_aStates.top().aSprms.find(getBorderTable(i));
                    if (it != m_aStates.top().aSprms.end())
                    {
                        std::multimap<Id, RTFValue::Pointer_t>& rAttributes = it->second->getAttributes();
                        rAttributes.insert(make_pair(NS_rtf::LN_DPTLINEWIDTH, pValue));
                    }
                }
            }
            break;
        case RTF_BRDRCF:
            {
                RTFValue::Pointer_t pValue(new RTFValue(getColorTable(nParam)));

                for (int i = 0; i < 4; i++)
                {
                    std::multimap<Id, RTFValue::Pointer_t>::iterator it = m_aStates.top().aSprms.find(getBorderTable(i));
                    if (it != m_aStates.top().aSprms.end())
                    {
                        std::multimap<Id, RTFValue::Pointer_t>& rAttributes = it->second->getAttributes();
                        rAttributes.insert(make_pair(NS_ooxml::LN_CT_Border_color, pValue));
                    }
                }
            }
            break;
        case RTF_BRSP:
            {
                // dmapper expects it in points, we have it in twip
                RTFValue::Pointer_t pValue(new RTFValue(nParam / 20));

                for (int i = 0; i < 4; i++)
                {
                    std::multimap<Id, RTFValue::Pointer_t>::iterator it = m_aStates.top().aSprms.find(getBorderTable(i));
                    if (it != m_aStates.top().aSprms.end())
                    {
                        std::multimap<Id, RTFValue::Pointer_t>& rAttributes = it->second->getAttributes();
                        rAttributes.insert(make_pair(NS_rtf::LN_DPTSPACE, pValue));
                    }
                }
            }
            break;
        case RTF_TX:
            {
                std::multimap<Id, RTFValue::Pointer_t>& rAttributes = lcl_getTabsTab(m_aStates);
                RTFValue::Pointer_t pTabposValue(new RTFValue(nParam));
                rAttributes.insert(make_pair(NS_ooxml::LN_CT_TabStop_pos, pTabposValue));
            }
            break;
        case RTF_ILVL:
            {
                std::multimap<Id, RTFValue::Pointer_t>& rSprms = lcl_getNumPr(m_aStates);
                RTFValue::Pointer_t pValue(new RTFValue(nParam));
                rSprms.insert(make_pair(NS_sprm::LN_PIlvl, pValue));
            }
        case RTF_LISTID:
            {
                RTFValue::Pointer_t pValue(new RTFValue(nParam));
                if (m_aStates.top().nDestinationState == DESTINATION_LISTENTRY)
                    m_aStates.top().aAttributes.insert(make_pair(NS_ooxml::LN_CT_AbstractNum_abstractNumId, pValue));
                else if (m_aStates.top().nDestinationState == DESTINATION_LISTOVERRIDEENTRY)
                    m_aStates.top().aSprms.insert(make_pair(NS_ooxml::LN_CT_Num_abstractNumId, pValue));
            }
            break;
        case RTF_LEVELSTARTAT:
            {
                RTFValue::Pointer_t pValue(new RTFValue(nParam));
                m_aStates.top().aSprms.insert(make_pair(NS_rtf::LN_ISTARTAT, pValue));
            }
            break;
        case RTF_LEVELNFC:
            {
                RTFValue::Pointer_t pValue(new RTFValue(nParam));
                m_aStates.top().aSprms.insert(make_pair(NS_rtf::LN_NFC, pValue));
            }
            break;
        case RTF_LS:
            {
                RTFValue::Pointer_t pValue(new RTFValue(nParam));
                if (m_aStates.top().nDestinationState == DESTINATION_LISTOVERRIDEENTRY)
                    m_aStates.top().aAttributes.insert(make_pair(NS_rtf::LN_LSID, pValue));
                else
                {
                    std::multimap<Id, RTFValue::Pointer_t>& rSprms = lcl_getNumPr(m_aStates);
                    rSprms.insert(make_pair(NS_sprm::LN_PIlfo, pValue));
                }
            }
            break;
        case RTF_U:
            if ((SAL_MIN_INT16 <= nParam) && (nParam <= SAL_MAX_INT16))
            {
                OUString aStr(static_cast<sal_Unicode>(nParam));
                text(aStr);
                m_aStates.top().nCharsToSkip = m_aStates.top().nUc;
            }
            break;
        default:
            OSL_TRACE("%s: TODO handle value '%s'", OSL_THIS_FUNC, m_pCurrentKeyword->getStr());
            bParsed = false;
            break;
    }
    skipDestination(bParsed);
    return 0;
}

int RTFDocumentImpl::dispatchToggle(RTFKeyword nKeyword, bool bParam, int nParam)
{
    bool bParsed = true;
    int nSprm = 0;

    // Map all underline keywords to a single sprm.
    switch (nKeyword)
    {
        case RTF_UL: nSprm = 1; break;
        case RTF_ULD: nSprm = 4; break;
        case RTF_ULDASH: nSprm = 7; break;
        case RTF_ULDASHD: nSprm = 9; break;
        case RTF_ULDASHDD: nSprm = 10; break;
        case RTF_ULDB: nSprm = 3; break;
        case RTF_ULHWAVE: nSprm = 27; break;
        case RTF_ULLDASH: nSprm = 39; break;
        case RTF_ULNONE: nSprm = 0; break;
        case RTF_ULTH: nSprm = 6; break;
        case RTF_ULTHD: nSprm = 20; break;
        case RTF_ULTHDASH: nSprm = 23; break;
        case RTF_ULTHDASHD: nSprm = 25; break;
        case RTF_ULTHDASHDD: nSprm = 26; break;
        case RTF_ULTHLDASH: nSprm = 55; break;
        case RTF_ULULDBWAVE: nSprm = 43; break;
        case RTF_ULW: nSprm = 2; break;
        case RTF_ULWAVE: nSprm = 11; break;
        default: break;
    }
    if (nSprm > 0)
    {
        RTFValue::Pointer_t pValue(new RTFValue((!bParam || nParam != 0) ? nSprm : 0));
        m_aStates.top().aSprms.insert(make_pair(NS_sprm::LN_CKul, pValue));
        skipDestination(bParsed);
        return 0;
    }

    // Accent characters (over dot / over coma).
    switch (nKeyword)
    {
        case RTF_ACCNONE: nSprm = 0; break;
        case RTF_ACCDOT: nSprm = 1; break;
        case RTF_ACCCOMMA: nSprm = 2; break;
        case RTF_ACCCIRCLE: nSprm = 3; break;
        case RTF_ACCUNDERDOT: nSprm = 4; break;
        default: break;
    }
    if (nSprm > 0)
    {
        RTFValue::Pointer_t pValue(new RTFValue((!bParam || nParam != 0) ? nSprm : 0));
        m_aStates.top().aSprms.insert(make_pair(NS_sprm::LN_CKcd, pValue));
        skipDestination(bParsed);
        return 0;
    }

    switch (nKeyword)
    {
        case RTF_B: nSprm = NS_sprm::LN_CFBold; break;
        case RTF_AB: nSprm = NS_sprm::LN_CFBoldBi; break;
        case RTF_I: nSprm = NS_sprm::LN_CFItalic; break;
        case RTF_UL: nSprm = NS_sprm::LN_CKul; break;
        case RTF_OUTL: nSprm = NS_sprm::LN_CFOutline; break;
        case RTF_SHAD: nSprm = NS_sprm::LN_CFShadow; break;
        case RTF_V: nSprm = NS_sprm::LN_CFVanish; break;
        case RTF_STRIKE: nSprm = NS_sprm::LN_CFStrike; break;
        case RTF_STRIKED: nSprm = NS_sprm::LN_CFDStrike; break;
        case RTF_SCAPS: nSprm = NS_sprm::LN_CFSmallCaps; break;
        case RTF_IMPR: nSprm = NS_sprm::LN_CFImprint; break;
        default:
            OSL_TRACE("%s: TODO handle toggle '%s'", OSL_THIS_FUNC, m_pCurrentKeyword->getStr());
            bParsed = false;
            break;
    }
    if (nSprm > 0)
    {
        RTFValue::Pointer_t pValue(new RTFValue(!bParam || nParam != 0));
        m_aStates.top().aSprms.insert(make_pair(nSprm, pValue));
    }

    skipDestination(bParsed);
    return 0;
}

void RTFDocumentImpl::skipDestination(bool bParsed)
{
    if (m_bSkipUnknown)
    {
        if (!bParsed)
        {
            OSL_TRACE("%s: skipping destination", OSL_THIS_FUNC);
            m_aStates.top().nDestinationState = DESTINATION_SKIP;
        }
        m_bSkipUnknown = false;
    }
}

int RTFDocumentImpl::dispatchKeyword(OString& rKeyword, bool bParam, int nParam)
{
    if (m_aStates.top().nDestinationState == DESTINATION_SKIP)
        return 0;
    //OSL_TRACE("%s: keyword '\\%s' with param? %d param val: '%d'", OSL_THIS_FUNC,
    //        rKeyword.getStr(), (bParam ? 1 : 0), (bParam ? nParam : 0));
    int i, ret;
    for (i = 0; i < nRTFControlWords; i++)
    {
        if (!strcmp(rKeyword.getStr(), aRTFControlWords[i].sKeyword))
            break;
    }
    if (i == nRTFControlWords)
    {
        OSL_TRACE("%s: unknown keyword '\\%s'", OSL_THIS_FUNC, rKeyword.getStr());
        skipDestination(false);
        return 0;
    }

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
            // and symbols
            if ((ret = dispatchSymbol(aRTFControlWords[i].nIndex)))
                return ret;
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
    //OSL_TRACE("%s before push: %d", OSL_THIS_FUNC, m_nGroup);

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
    //OSL_TRACE("%s before pop: m_nGroup %d, dest state: %d", OSL_THIS_FUNC, m_nGroup, m_aStates.top().nDestinationState);

    RTFReferenceTable::Entry_t aEntry;
    bool bFontEntryEnd = false;
    bool bStyleEntryEnd = false;
    std::multimap<Id, RTFValue::Pointer_t> aSprms;
    std::multimap<Id, RTFValue::Pointer_t> aAttributes;
    bool bListEntryEnd = false;
    bool bListLevelEnd = false;
    bool bListOverrideEntryEnd = false;
    bool bLevelTextEnd = false;

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
    else if (m_aStates.top().nDestinationState == DESTINATION_LISTOVERRIDETABLE)
    {
        std::multimap<Id, RTFValue::Pointer_t> aDummyAttributes;
        writerfilter::Reference<Properties>::Pointer_t const pProp(new RTFReferenceProperties(aDummyAttributes, m_aListTableSprms));
        RTFReferenceTable::Entries_t aListTableEntries;
        aListTableEntries.insert(make_pair(0, pProp));
        writerfilter::Reference<Table>::Pointer_t const pTable(new RTFReferenceTable(aListTableEntries));
        Mapper().table(NS_rtf::LN_LISTTABLE, pTable);
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
    else if (m_aStates.top().nDestinationState == DESTINATION_LISTENTRY)
    {
        aAttributes = m_aStates.top().aAttributes;
        aSprms = m_aStates.top().aSprms;
        bListEntryEnd = true;
    }
    else if (m_aStates.top().nDestinationState == DESTINATION_LISTLEVEL)
    {
        aAttributes = m_aStates.top().aAttributes;
        aSprms = m_aStates.top().aSprms;
        bListLevelEnd = true;
    }
    else if (m_aStates.top().nDestinationState == DESTINATION_LISTOVERRIDEENTRY)
    {
        aAttributes = m_aStates.top().aAttributes;
        aSprms = m_aStates.top().aSprms;
        bListOverrideEntryEnd = true;
    }
    else if (m_aStates.top().nDestinationState == DESTINATION_FIELDINSTRUCTION)
    {
        OUString aOUStr(OStringToOUString(m_aStates.top().aFieldInstruction.makeStringAndClear(), RTL_TEXTENCODING_UTF8));
        text(aOUStr);
    }
    else if (m_aStates.top().nDestinationState == DESTINATION_LEVELTEXT)
    {
        aAttributes = m_aStates.top().aAttributes;
        bLevelTextEnd = true;
    }

    m_aStates.pop();

    m_nGroup--;

    if (bFontEntryEnd)
        m_aStates.top().aFontTableEntries.insert(make_pair(aEntry.first, aEntry.second));
    else if (bStyleEntryEnd)
        m_aStates.top().aStyleTableEntries.insert(make_pair(aEntry.first, aEntry.second));
    // list table
    else if (bListEntryEnd)
    {
        RTFValue::Pointer_t pValue(new RTFValue(aAttributes, aSprms));
        m_aListTableSprms.insert(make_pair(NS_ooxml::LN_CT_Numbering_abstractNum, pValue));
    }
    else if (bListLevelEnd)
    {
        RTFValue::Pointer_t pInnerValue(new RTFValue(m_aStates.top().nListLevelNum++));
        aAttributes.insert(make_pair(NS_ooxml::LN_CT_Lvl_ilvl, pInnerValue));

        RTFValue::Pointer_t pValue(new RTFValue(aAttributes, aSprms));
        m_aStates.top().aSprms.insert(make_pair(NS_ooxml::LN_CT_AbstractNum_lvl, pValue));
    }
    // list override table
    else if (bListOverrideEntryEnd)
    {
        RTFValue::Pointer_t pValue(new RTFValue(aAttributes, aSprms));
        m_aListTableSprms.insert(make_pair(NS_ooxml::LN_CT_Numbering_num, pValue));
    }
    else if (bLevelTextEnd)
    {
        RTFValue::Pointer_t pValue(new RTFValue(aAttributes));
        m_aStates.top().aSprms.insert(make_pair(NS_ooxml::LN_CT_Lvl_lvlText, pValue));
    }

    return 0;
}

int RTFDocumentImpl::resolveParse()
{
    OSL_TRACE("%s", OSL_THIS_FUNC);
    char ch;
    int ret;
    // for hex chars
    int b = 0, count = 2;

    while ((Strm() >> ch, !Strm().IsEof()))
    {
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
                        OSL_TRACE("%s: hex internal state", OSL_THIS_FUNC);
                        b = b << 4;
                        if (isdigit(ch))
                            b += (char) ch - '0';
                        else
                        {
                            if (islower(ch))
                            {
                                if (ch < 'a' || ch > 'f')
                                    return ERROR_HEX_INVALID;
                                b += (char) ch - 'a';
                            }
                            else
                            {
                                if (ch < 'A' || ch > 'F')
                                    return ERROR_HEX_INVALID;
                                b += (char) ch - 'A';
                            }
                            b += 10;
                        }
                        count--;
                        if (!count)
                        {
                            if ((ret = resolveChars(b)))
                                return ret;
                            count = 2;
                            b = 0;
                            m_aStates.top().nInternalState = INTERNAL_NORMAL;
                        }
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
    nCurrentEncoding(0),
    aFieldInstruction(),
    nUc(1),
    nCharsToSkip(0),
    nListLevelNum(0)
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
