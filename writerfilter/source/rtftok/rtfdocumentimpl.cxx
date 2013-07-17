/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/drawing/XEnhancedCustomShapeDefaulter.hpp>
#include <com/sun/star/graphic/GraphicProvider.hpp>
#include <com/sun/star/io/UnexpectedEOFException.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/text/XTextFrame.hpp>
#include <com/sun/star/text/SizeType.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <rtl/tencinfo.h>
#include <svl/lngmisc.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/streamwrap.hxx>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <rtl/ustring.hxx>
#include <svtools/grfmgr.hxx>
#include <vcl/graph.hxx>
#include <vcl/outdev.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wmf.hxx>
#include <filter/msfilter/util.hxx>
#include <filter/msfilter/escherex.hxx>
#include <comphelper/string.hxx>
#include <tools/globname.hxx>
#include <comphelper/classids.hxx>
#include <comphelper/embeddedobjectcontainer.hxx>
#include <sfx2/sfxbasemodel.hxx>

#include <oox/mathml/import.hxx>
#include <doctok/sprmids.hxx> // NS_sprm namespace
#include <doctok/resourceids.hxx> // NS_rtf namespace
#include <ooxml/resourceids.hxx> // NS_ooxml namespace
#include <ooxml/OOXMLFastTokens.hxx> // ooxml namespace
#include <oox/token/namespaces.hxx> // oox namespace
#include <oox/token/tokens.hxx>
#include <dmapper/GraphicHelpers.hxx>

#include <rtfsdrimport.hxx>
#include <rtftokenizer.hxx>
#include <rtflookahead.hxx>
#include <rtfcharsets.hxx>
#include <rtfreferenceproperties.hxx>
#include <rtfskipdestination.hxx>
#include <rtffly.hxx>

#define TWIP_TO_MM100(TWIP)     ((TWIP) >= 0 ? (((TWIP)*127L+36L)/72L) : (((TWIP)*127L-36L)/72L))
#define MM100_TO_EMU(MM100)     (MM100 * 360)
#define M_TOKEN(token) OOX_TOKEN(officeMath, token)
#define OPEN_M_TOKEN( rtftok, ooxtok ) \
        case RTF_M##rtftok: \
            m_aMathBuffer.appendOpeningTag(M_TOKEN(ooxtok)); \
            m_aStates.top().nDestinationState = DESTINATION_M##rtftok; \
            break

using std::make_pair;

namespace writerfilter {
namespace rtftok {

static Id lcl_getParagraphBorder(sal_uInt32 nIndex)
{
    static const Id aBorderIds[] =
    {
        NS_sprm::LN_PBrcTop, NS_sprm::LN_PBrcLeft, NS_sprm::LN_PBrcBottom, NS_sprm::LN_PBrcRight
    };

    return aBorderIds[nIndex];
}

static void lcl_putNestedAttribute(RTFSprms& rSprms, Id nParent, Id nId, RTFValue::Pointer_t pValue,
        bool bOverwrite = true, bool bAttribute = true)
{
    RTFValue::Pointer_t pParent = rSprms.find(nParent);
    if (!pParent.get())
    {
        RTFSprms aAttributes;
        if (nParent == NS_ooxml::LN_CT_TcPrBase_shd)
        {
            // RTF default is 'auto', see writerfilter::dmapper::CellColorHandler
            aAttributes.set(NS_ooxml::LN_CT_Shd_color, RTFValue::Pointer_t(new RTFValue(0x0a)));
            aAttributes.set(NS_ooxml::LN_CT_Shd_fill, RTFValue::Pointer_t(new RTFValue(0x0a)));
        }
        RTFValue::Pointer_t pParentValue(new RTFValue(aAttributes));
        rSprms.set(nParent, pParentValue, bOverwrite);
        pParent = pParentValue;
    }
    RTFSprms& rAttributes = (bAttribute ? pParent->getAttributes() : pParent->getSprms());
    rAttributes.set(nId, pValue, bOverwrite);
}

static void lcl_putNestedSprm(RTFSprms& rSprms, Id nParent, Id nId, RTFValue::Pointer_t pValue, bool bOverwrite = false)
{
    lcl_putNestedAttribute(rSprms, nParent, nId, pValue, bOverwrite, false);
}

static bool lcl_eraseNestedAttribute(RTFSprms& rSprms, Id nParent, Id nId)
{
    RTFValue::Pointer_t pParent = rSprms.find(nParent);
    if (!pParent.get())
        // It doesn't even have a parent, we're done!
        return false;
    RTFSprms& rAttributes = pParent->getAttributes();
    return rAttributes.erase(nId);
}

static RTFSprms& lcl_getLastAttributes(RTFSprms& rSprms, Id nId)
{
    RTFValue::Pointer_t p = rSprms.find(nId);
    if (p.get() && p->getSprms().size())
        return p->getSprms().back().second->getAttributes();
    else
    {
        SAL_WARN("writerfilter", "trying to set property when no type is defined");
        return rSprms;
    }
}

static void lcl_putBorderProperty(std::stack<RTFParserState>& aStates, Id nId, RTFValue::Pointer_t pValue)
{
    RTFSprms* pAttributes = 0;
    if (aStates.top().nBorderState == BORDER_PARAGRAPH_BOX)
        for (int i = 0; i < 4; i++)
        {
            RTFValue::Pointer_t p = aStates.top().aParagraphSprms.find(lcl_getParagraphBorder(i));
            if (p.get())
            {
                RTFSprms& rAttributes = p->getAttributes();
                rAttributes.set(nId, pValue);
            }
        }
    // Attributes of the last border type
    else if (aStates.top().nBorderState == BORDER_PARAGRAPH)
        pAttributes = &lcl_getLastAttributes(aStates.top().aParagraphSprms, NS_ooxml::LN_CT_PrBase_pBdr);
    else if (aStates.top().nBorderState == BORDER_CELL)
        pAttributes = &lcl_getLastAttributes(aStates.top().aTableCellSprms, NS_ooxml::LN_CT_TcPrBase_tcBorders);
    else if (aStates.top().nBorderState == BORDER_PAGE)
        pAttributes = &lcl_getLastAttributes(aStates.top().aSectionSprms, NS_ooxml::LN_EG_SectPrContents_pgBorders);
    if (pAttributes)
        pAttributes->set(nId, pValue);
}

static OString lcl_DTTM22OString(long lDTTM)
{
    return msfilter::util::DateTimeToOString(msfilter::util::DTTM2DateTime(lDTTM));
}

static writerfilter::Reference<Properties>::Pointer_t lcl_getBookmarkProperties(int nPos, OUString& rString)
{
    RTFSprms aAttributes;
    RTFValue::Pointer_t pPos(new RTFValue(nPos));
    if (!rString.isEmpty())
    {
        // If present, this should be sent first.
        RTFValue::Pointer_t pString(new RTFValue(rString));
        aAttributes.set(NS_rtf::LN_BOOKMARKNAME, pString);
    }
    aAttributes.set(NS_rtf::LN_IBKL, pPos);
    return writerfilter::Reference<Properties>::Pointer_t(new RTFReferenceProperties(aAttributes));
}

static writerfilter::Reference<Properties>::Pointer_t lcl_getBookmarkProperties(int nPos)
{
    OUString aStr;
    return lcl_getBookmarkProperties(nPos, aStr);
}

static const char* lcl_RtfToString(RTFKeyword nKeyword)
{
    for (int i = 0; i < nRTFControlWords; i++)
    {
        if (nKeyword == aRTFControlWords[i].nIndex)
            return aRTFControlWords[i].sKeyword;
    }
    return NULL;
}

static util::DateTime lcl_getDateTime(RTFParserState& aState)
{
    return util::DateTime(0 /*100sec*/, 0 /*sec*/, aState.nMinute, aState.nHour,
        aState.nDay, aState.nMonth, aState.nYear, false);
}

static void lcl_DestinationToMath(OUStringBuffer& rDestinationText, oox::formulaimport::XmlStreamBuilder& rMathBuffer, bool& rMathNor)
{
    OUString aStr = rDestinationText.makeStringAndClear();
    if (!aStr.isEmpty())
    {
        rMathBuffer.appendOpeningTag(M_TOKEN(r));
        if (rMathNor)
        {
            rMathBuffer.appendOpeningTag(M_TOKEN(rPr));
            // Same as M_TOKEN(lit)
            rMathBuffer.appendOpeningTag(M_TOKEN(nor));
            rMathBuffer.appendClosingTag(M_TOKEN(nor));
            rMathBuffer.appendClosingTag(M_TOKEN(rPr));
            rMathNor = false;
        }
        rMathBuffer.appendOpeningTag(M_TOKEN(t));
        rMathBuffer.appendCharacters(aStr);
        rMathBuffer.appendClosingTag(M_TOKEN(t));
        rMathBuffer.appendClosingTag(M_TOKEN(r));
    }
}

RTFDocumentImpl::RTFDocumentImpl(uno::Reference<uno::XComponentContext> const& xContext,
        uno::Reference<io::XInputStream> const& xInputStream,
        uno::Reference<lang::XComponent> const& xDstDoc,
        uno::Reference<frame::XFrame>    const& xFrame,
        uno::Reference<task::XStatusIndicator> const& xStatusIndicator)
    : m_xContext(xContext),
    m_xInputStream(xInputStream),
    m_xDstDoc(xDstDoc),
    m_xFrame(xFrame),
    m_xStatusIndicator(xStatusIndicator),
    m_aDefaultState(this),
    m_bSkipUnknown(false),
    m_aFontEncodings(),
    m_aFontIndexes(),
    m_aColorTable(),
    m_bFirstRun(true),
    m_bNeedPap(true),
    m_bNeedCr(false),
    m_bNeedCrOrig(false),
    m_bNeedPar(true),
    m_bNeedFinalPar(false),
    m_aListTableSprms(),
    m_aSettingsTableAttributes(),
    m_aSettingsTableSprms(),
    m_xStorage(),
    m_aTableBuffer(),
    m_aSuperBuffer(),
    m_bHasFootnote(false),
    m_pSuperstream(0),
    m_nHeaderFooterPositions(),
    m_nGroupStartPos(0),
    m_aBookmarks(),
    m_aAuthors(),
    m_aFormfieldSprms(),
    m_aFormfieldAttributes(),
    m_nFormFieldType(FORMFIELD_NONE),
    m_aObjectSprms(),
    m_aObjectAttributes(),
    m_bObject(false),
    m_aFontTableEntries(),
    m_nCurrentFontIndex(0),
    m_aStyleTableEntries(),
    m_nCurrentStyleIndex(0),
    m_bFormField(false),
    m_bIsInFrame(false),
    m_aUnicodeBuffer(),
    m_aHexBuffer(),
    m_bMathNor(false),
    m_bIgnoreNextContSectBreak(false),
    m_bNeedSect(true),
    m_bWasInFrame(false),
    m_bHadPicture(false),
    m_bHadSect(false),
    m_nCellxMax(0),
    m_nListPictureId(0)
{
    OSL_ASSERT(xInputStream.is());
    m_pInStream.reset(utl::UcbStreamHelper::CreateStream(xInputStream, sal_True));

    m_xModelFactory.set(m_xDstDoc, uno::UNO_QUERY);

    uno::Reference<document::XDocumentPropertiesSupplier> xDocumentPropertiesSupplier(m_xDstDoc, uno::UNO_QUERY);
    if (xDocumentPropertiesSupplier.is())
        m_xDocumentProperties.set(xDocumentPropertiesSupplier->getDocumentProperties(), uno::UNO_QUERY);

    m_pGraphicHelper.reset(new oox::GraphicHelper(m_xContext, xFrame, m_xStorage));

    m_pTokenizer.reset(new RTFTokenizer(*this, m_pInStream.get(), m_xStatusIndicator));
    m_pSdrImport.reset(new RTFSdrImport(*this, m_xDstDoc));
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

void RTFDocumentImpl::setSuperstream(RTFDocumentImpl *pSuperstream)
{
    m_pSuperstream = pSuperstream;
}

void RTFDocumentImpl::setAuthor(OUString& rAuthor)
{
    m_aAuthor = rAuthor;
}

void RTFDocumentImpl::setAuthorInitials(OUString& rAuthorInitials)
{
    m_aAuthorInitials = rAuthorInitials;
}

bool RTFDocumentImpl::isSubstream() const
{
    return m_pSuperstream != 0;
}

void RTFDocumentImpl::finishSubstream()
{
    checkUnicode(/*bUnicode =*/ true, /*bHex =*/ true);
    // At the end of a footnote stream, we need to emit a run break when importing from Word.
    // We can't do so unconditionally, as Writer already writes a \par at the end of the footnote.
    if (m_bNeedCr)
    {
        Mapper().startCharacterGroup();
        runBreak();
        Mapper().endCharacterGroup();
    }
}

void RTFDocumentImpl::setIgnoreFirst(OUString& rIgnoreFirst)
{
    m_aIgnoreFirst = rIgnoreFirst;
}

void RTFDocumentImpl::resolveSubstream(sal_uInt32 nPos, Id nId)
{
    OUString aStr;
    resolveSubstream(nPos, nId, aStr);
}
void RTFDocumentImpl::resolveSubstream(sal_uInt32 nPos, Id nId, OUString& rIgnoreFirst)
{
    sal_uInt32 nCurrent = Strm().Tell();
    // Seek to header position, parse, then seek back.
    RTFDocumentImpl::Pointer_t pImpl(new RTFDocumentImpl(m_xContext, m_xInputStream, m_xDstDoc, m_xFrame, m_xStatusIndicator));
    pImpl->setSuperstream(this);
    pImpl->setIgnoreFirst(rIgnoreFirst);
    if (!m_aAuthor.isEmpty())
    {
        pImpl->setAuthor(m_aAuthor);
        m_aAuthor = OUString();
    }
    if (!m_aAuthorInitials.isEmpty())
    {
        pImpl->setAuthorInitials(m_aAuthorInitials);
        m_aAuthorInitials = OUString();
    }
    pImpl->seek(nPos);
    SAL_INFO("writerfilter", "substream start");
    Mapper().substream(nId, pImpl);
    SAL_INFO("writerfilter", "substream end");
    Strm().Seek(nCurrent);
    nPos = 0;
}

void RTFDocumentImpl::checkFirstRun()
{
    if (m_bFirstRun)
    {
        // output settings table
        writerfilter::Reference<Properties>::Pointer_t const pProp(new RTFReferenceProperties(m_aSettingsTableAttributes, m_aSettingsTableSprms));
        RTFReferenceTable::Entries_t aSettingsTableEntries;
        aSettingsTableEntries.insert(make_pair(0, pProp));
        writerfilter::Reference<Table>::Pointer_t const pTable(new RTFReferenceTable(aSettingsTableEntries));
        Mapper().table(NS_ooxml::LN_settings_settings, pTable);
        // start initial paragraph
        if (!m_pSuperstream)
            Mapper().startSectionGroup();
        Mapper().startParagraphGroup();

        // set the requested default font
        RTFValue::Pointer_t pFont = m_aDefaultState.aCharacterSprms.find(NS_sprm::LN_CRgFtc0);
        if (pFont.get())
            dispatchValue(RTF_F, pFont->getInt());
        m_bFirstRun = false;
    }
}

bool RTFDocumentImpl::getFirstRun()
{
    return m_bFirstRun;
}

void RTFDocumentImpl::setNeedPar(bool bNeedPar)
{
    m_bNeedPar = bNeedPar;
}

void RTFDocumentImpl::setNeedSect(bool bNeedSect)
{
    m_bNeedSect = bNeedSect;
}

writerfilter::Reference<Properties>::Pointer_t RTFDocumentImpl::getProperties(RTFSprms& rAttributes, RTFSprms& rSprms)
{
    int nStyle = m_aStates.top().nCurrentStyleIndex;
    RTFReferenceTable::Entries_t::iterator it = m_aStyleTableEntries.find(nStyle);
    if (it != m_aStyleTableEntries.end())
    {
        RTFReferenceProperties& rProps = *(RTFReferenceProperties*)it->second.get();
        // Get rid of direct formatting what is already in the style.
        rSprms.deduplicate(rProps.getSprms());
        rAttributes.deduplicate(rProps.getAttributes());
    }
    writerfilter::Reference<Properties>::Pointer_t pRet(new RTFReferenceProperties(rAttributes, rSprms));
    return pRet;
}

void RTFDocumentImpl::checkNeedPap()
{
    if (m_bNeedPap)
    {
        m_bNeedPap = false; // reset early, so we can avoid recursion when calling ourselves
        if (!m_aStates.top().pCurrentBuffer)
        {
            writerfilter::Reference<Properties>::Pointer_t const pParagraphProperties(
                    getProperties(m_aStates.top().aParagraphAttributes, m_aStates.top().aParagraphSprms)
                    );

            // Writer will ignore a page break before a text frame, so guard it with empty paragraphs
            bool hasBreakBeforeFrame = m_aStates.top().aFrame.hasProperties() &&
                m_aStates.top().aParagraphSprms.find(NS_sprm::LN_PFPageBreakBefore).get();
            if (hasBreakBeforeFrame)
            {
                dispatchSymbol(RTF_PAR);
                m_bNeedPap = false;
            }
            Mapper().props(pParagraphProperties);
            if (hasBreakBeforeFrame)
                dispatchSymbol(RTF_PAR);

            if (m_aStates.top().aFrame.hasProperties())
            {
                writerfilter::Reference<Properties>::Pointer_t const pFrameProperties(
                        new RTFReferenceProperties(RTFSprms(), m_aStates.top().aFrame.getSprms()));
                Mapper().props(pFrameProperties);
            }
        }
        else
        {
            RTFValue::Pointer_t pValue(new RTFValue(m_aStates.top().aParagraphAttributes, m_aStates.top().aParagraphSprms));
            m_aStates.top().pCurrentBuffer->push_back(make_pair(BUFFER_PROPS, pValue));
        }
    }
}

void RTFDocumentImpl::runProps()
{
    if (!m_aStates.top().pCurrentBuffer)
    {
        writerfilter::Reference<Properties>::Pointer_t const pProperties = getProperties(m_aStates.top().aCharacterAttributes, m_aStates.top().aCharacterSprms);
        Mapper().props(pProperties);
    }
    else
    {
        RTFValue::Pointer_t pValue(new RTFValue(m_aStates.top().aCharacterAttributes, m_aStates.top().aCharacterSprms));
        m_aStates.top().pCurrentBuffer->push_back(make_pair(BUFFER_PROPS, pValue));
    }
}

void RTFDocumentImpl::runBreak()
{
    sal_uInt8 sBreak[] = { 0xd };
    Mapper().text(sBreak, 1);
    m_bNeedCr = false;
}

void RTFDocumentImpl::tableBreak()
{
    runBreak();
    Mapper().endParagraphGroup();
    Mapper().startParagraphGroup();
}

void RTFDocumentImpl::parBreak()
{
    checkFirstRun();
    checkNeedPap();
    // end previous paragraph
    Mapper().startCharacterGroup();
    runBreak();
    Mapper().endCharacterGroup();
    Mapper().endParagraphGroup();

    m_bHadPicture = false;

    // start new one
    Mapper().startParagraphGroup();
}

void RTFDocumentImpl::sectBreak(bool bFinal = false)
{
    SAL_INFO("writerfilter", OSL_THIS_FUNC << ": final? " << bFinal << ", needed? " << m_bNeedSect);
    bool bNeedSect = m_bNeedSect;
    RTFValue::Pointer_t pBreak = m_aStates.top().aSectionSprms.find(NS_sprm::LN_SBkc);
    bool bContinuous = pBreak.get() && pBreak->getInt() == 0;
    // If there is no paragraph in this section, then insert a dummy one, as required by Writer,
    // unless this is the end of the doc, we had nothing since the last section break and this is not a continuous one.
    if (m_bNeedPar && !(bFinal && !m_bNeedSect && !bContinuous))
        dispatchSymbol(RTF_PAR);
    // It's allowed to not have a non-table paragraph at the end of an RTF doc, add it now if required.
    if (m_bNeedFinalPar && bFinal)
    {
        dispatchFlag(RTF_PARD);
        dispatchSymbol(RTF_PAR);
    }
    while (!m_nHeaderFooterPositions.empty())
    {
        std::pair<Id, sal_uInt32> aPair = m_nHeaderFooterPositions.front();
        m_nHeaderFooterPositions.pop();
        resolveSubstream(aPair.second, aPair.first);
    }

    // Normally a section break at the end of the doc is necessary. Unless the
    // last control word in the document is a section break itself.
    if (!bNeedSect || !m_bHadSect)
    {
        // In case the last section is a continuous one, we don't need to output a section break.
        if (bFinal && bContinuous)
            m_aStates.top().aSectionSprms.erase(NS_sprm::LN_SBkc);
    }

    // Section properties are a paragraph sprm.
    RTFValue::Pointer_t pValue(new RTFValue(m_aStates.top().aSectionAttributes, m_aStates.top().aSectionSprms));
    RTFSprms aAttributes;
    RTFSprms aSprms;
    aSprms.set(NS_ooxml::LN_CT_PPr_sectPr, pValue);
    writerfilter::Reference<Properties>::Pointer_t const pProperties(
            new RTFReferenceProperties(aAttributes, aSprms)
            );
    // The trick is that we send properties of the previous section right now, which will be exactly what dmapper expects.
    Mapper().props(pProperties);
    Mapper().endParagraphGroup();
    if (!m_pSuperstream)
        Mapper().endSectionGroup();
    if (!bFinal)
    {
        Mapper().startSectionGroup();
        Mapper().startParagraphGroup();
    }
    m_bNeedPar = true;
    m_bNeedSect = false;
}

void RTFDocumentImpl::seek(sal_uInt32 nPos)
{
    Strm().Seek(nPos);
}

sal_uInt32 RTFDocumentImpl::getColorTable(sal_uInt32 nIndex)
{
    if (!m_pSuperstream)
    {
        if (nIndex < m_aColorTable.size())
            return m_aColorTable[nIndex];
        return 0;
    }
    else
        return m_pSuperstream->getColorTable(nIndex);
}

rtl_TextEncoding RTFDocumentImpl::getEncoding(sal_uInt32 nFontIndex)
{
    if (!m_pSuperstream)
    {
        std::map<int, rtl_TextEncoding>::iterator it = m_aFontEncodings.find(nFontIndex);
        if (it != m_aFontEncodings.end())
            return it->second;
        return msfilter::util::getBestTextEncodingFromLocale(Application::GetSettings().GetLanguageTag().getLocale());
    }
    else
        return m_pSuperstream->getEncoding(nFontIndex);
}

int RTFDocumentImpl::getFontIndex(int nIndex)
{
    if (!m_pSuperstream)
        return std::find(m_aFontIndexes.begin(), m_aFontIndexes.end(), nIndex) - m_aFontIndexes.begin();
    else
        return m_pSuperstream->getFontIndex(nIndex);
}

RTFParserState& RTFDocumentImpl::getDefaultState()
{
    if (!m_pSuperstream)
        return m_aDefaultState;
    else
        return m_pSuperstream->getDefaultState();
}

oox::GraphicHelper& RTFDocumentImpl::getGraphicHelper()
{
    return *m_pGraphicHelper;
}

void RTFDocumentImpl::resolve(Stream & rMapper)
{
    m_pMapperStream = &rMapper;
    switch (m_pTokenizer->resolveParse())
    {
        case ERROR_OK:
            SAL_INFO("writerfilter", OSL_THIS_FUNC << ": finished without errors");
            break;
        case ERROR_GROUP_UNDER:
            SAL_INFO("writerfilter", OSL_THIS_FUNC << ": unmatched '}'");
            break;
        case ERROR_GROUP_OVER:
            SAL_INFO("writerfilter", OSL_THIS_FUNC << ": unmatched '{'");
            throw io::WrongFormatException(m_pTokenizer->getPosition(), uno::Reference< uno::XInterface >());
            break;
        case ERROR_EOF:
            SAL_INFO("writerfilter", OSL_THIS_FUNC << ": unexpected end of file");
            throw io::WrongFormatException(m_pTokenizer->getPosition(), uno::Reference< uno::XInterface >());
            break;
        case ERROR_HEX_INVALID:
            SAL_INFO("writerfilter", OSL_THIS_FUNC << ": invalid hex char");
            throw io::WrongFormatException(m_pTokenizer->getPosition(), uno::Reference< uno::XInterface >());
            break;
        case ERROR_CHAR_OVER:
            SAL_INFO("writerfilter", OSL_THIS_FUNC << ": characters after last '}'");
            break;
    }
}

int RTFDocumentImpl::resolvePict(bool bInline)
{
    SvMemoryStream aStream;
    SvStream *pStream = 0;
    if (!m_pBinaryData.get())
    {
        pStream = &aStream;
        int b = 0, count = 2;

        // Feed the destination text to a stream.
        OString aStr = OUStringToOString(m_aStates.top().aDestinationText.makeStringAndClear(), RTL_TEXTENCODING_ASCII_US);
        const char *str = aStr.getStr();
        for (int i = 0; i < aStr.getLength(); ++i)
        {
            char ch = str[i];
            if (ch != 0x0d && ch != 0x0a && ch != 0x20)
            {
                b = b << 4;
                sal_Int8 parsed = m_pTokenizer->asHex(ch);
                if (parsed == -1)
                    return ERROR_HEX_INVALID;
                b += parsed;
                count--;
                if (!count)
                {
                    aStream << (char)b;
                    count = 2;
                    b = 0;
                }
            }
        }
    }
    else
        pStream = m_pBinaryData.get();

    if (!pStream->Tell())
        // No destination text? Then we'll get it later.
        return 0;

    // Store, and get its URL.
    pStream->Seek(0);
    uno::Reference<io::XInputStream> xInputStream(new utl::OInputStreamWrapper(pStream));
    WMF_EXTERNALHEADER aExtHeader;
    aExtHeader.mapMode = m_aStates.top().aPicture.eWMetafile;
    aExtHeader.xExt = m_aStates.top().aPicture.nWidth;
    aExtHeader.yExt = m_aStates.top().aPicture.nHeight;
    WMF_EXTERNALHEADER* pExtHeader = &aExtHeader;
    uno::Reference<lang::XServiceInfo> xServiceInfo(m_aStates.top().aDrawingObject.xShape, uno::UNO_QUERY);
    if (xServiceInfo.is() && xServiceInfo->supportsService("com.sun.star.text.TextFrame"))
        pExtHeader = 0;
    OUString aGraphicUrl = m_pGraphicHelper->importGraphicObject(xInputStream, pExtHeader);

    if (m_aStates.top().aPicture.nStyle != BMPSTYLE_NONE)
    {
        // In case of PNG/JPEG, the real size is known, don't use the values
        // provided by picw and pich.
        OString aURLBS(OUStringToOString(aGraphicUrl, RTL_TEXTENCODING_UTF8));
        const char aURLBegin[] = "vnd.sun.star.GraphicObject:";
        if (aURLBS.compareTo(aURLBegin, RTL_CONSTASCII_LENGTH(aURLBegin)) == 0)
        {
            Graphic aGraphic = GraphicObject(aURLBS.copy(RTL_CONSTASCII_LENGTH(aURLBegin))).GetTransformedGraphic();
            Size aSize(aGraphic.GetPrefSize());
            MapMode aMap(MAP_100TH_MM);
            if (aGraphic.GetPrefMapMode().GetMapUnit() == MAP_PIXEL)
                aSize = Application::GetDefaultDevice()->PixelToLogic(aSize, aMap);
            else
                aSize = OutputDevice::LogicToLogic(aSize, aGraphic.GetPrefMapMode(), aMap);
            m_aStates.top().aPicture.nWidth = aSize.Width();
            m_aStates.top().aPicture.nHeight = aSize.Height();
        }
    }

    // Wrap it in an XShape.
    uno::Reference<drawing::XShape> xShape;
    if (m_xModelFactory.is())
        xShape.set(m_xModelFactory->createInstance("com.sun.star.drawing.GraphicObjectShape"), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPropertySet(xShape, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPageSupplier> xDrawSupplier( m_xDstDoc, uno::UNO_QUERY);
    if ( xDrawSupplier.is() )
    {
        uno::Reference< drawing::XShapes > xShapes( xDrawSupplier->getDrawPage(), uno::UNO_QUERY );
        if ( xShapes.is() )
            xShapes->add( xShape );
    }

    // check if the picture is in an OLE object and if the \objdata element is used
    // (see RTF_OBJECT in RTFDocumentImpl::dispatchDestination)
    if (m_bObject)
    {
        // Set bitmap
        beans::PropertyValues aMediaProperties(1);
        aMediaProperties[0].Name = "URL";
        aMediaProperties[0].Value <<= aGraphicUrl;
        uno::Reference<graphic::XGraphicProvider> xGraphicProvider(graphic::GraphicProvider::create(m_xContext));
        uno::Reference<graphic::XGraphic> xGraphic = xGraphicProvider->queryGraphic(aMediaProperties);
        xPropertySet->setPropertyValue("Graphic", uno::Any(xGraphic));

        // Set the object size
        awt::Size aSize;
        aSize.Width = (m_aStates.top().aPicture.nGoalWidth ? m_aStates.top().aPicture.nGoalWidth : m_aStates.top().aPicture.nWidth);
        aSize.Height = (m_aStates.top().aPicture.nGoalHeight ? m_aStates.top().aPicture.nGoalHeight : m_aStates.top().aPicture.nHeight);
        xShape->setSize( aSize );

        RTFValue::Pointer_t pShapeValue(new RTFValue(xShape));
        m_aObjectAttributes.set(NS_ooxml::LN_shape, pShapeValue);
        return 0;
    }

    if (xPropertySet.is())
        xPropertySet->setPropertyValue("GraphicURL", uno::Any(aGraphicUrl));

    if (m_aStates.top().bInListpicture)
    {
        // Send the shape directly, no section is started, to additional properties will be ignored anyway.
        Mapper().startShape(xShape);
        Mapper().endShape();
        return 0;
    }

    // Send it to the dmapper.
    RTFSprms aSprms;
    RTFSprms aAttributes;
    // shape attribute
    RTFSprms aPicAttributes;
    RTFValue::Pointer_t pShapeValue(new RTFValue(xShape));
    aPicAttributes.set(NS_ooxml::LN_shape, pShapeValue);
    // pic sprm
    RTFSprms aGraphicDataAttributes;
    RTFSprms aGraphicDataSprms;
    RTFValue::Pointer_t pPicValue(new RTFValue(aPicAttributes));
    aGraphicDataSprms.set(NS_ooxml::LN_pic_pic, pPicValue);
    // graphicData sprm
    RTFSprms aGraphicAttributes;
    RTFSprms aGraphicSprms;
    RTFValue::Pointer_t pGraphicDataValue(new RTFValue(aGraphicDataAttributes, aGraphicDataSprms));
    aGraphicSprms.set(NS_ooxml::LN_CT_GraphicalObject_graphicData, pGraphicDataValue);
    // graphic sprm
    RTFValue::Pointer_t pGraphicValue(new RTFValue(aGraphicAttributes, aGraphicSprms));
    // extent sprm
    RTFSprms aExtentAttributes;
    int nXExt, nYExt;
    nXExt = (m_aStates.top().aPicture.nGoalWidth ? m_aStates.top().aPicture.nGoalWidth : m_aStates.top().aPicture.nWidth);
    nYExt = (m_aStates.top().aPicture.nGoalHeight ? m_aStates.top().aPicture.nGoalHeight : m_aStates.top().aPicture.nHeight);
    if (m_aStates.top().aPicture.nScaleX != 100)
        nXExt = (((long)m_aStates.top().aPicture.nScaleX) * ( nXExt - ( m_aStates.top().aPicture.nCropL + m_aStates.top().aPicture.nCropR ))) / 100L;
    if (m_aStates.top().aPicture.nScaleY != 100)
        nYExt = (((long)m_aStates.top().aPicture.nScaleY) * ( nYExt - ( m_aStates.top().aPicture.nCropT + m_aStates.top().aPicture.nCropB ))) / 100L;
    RTFValue::Pointer_t pXExtValue(new RTFValue(nXExt));
    RTFValue::Pointer_t pYExtValue(new RTFValue(nYExt));
    aExtentAttributes.set(NS_rtf::LN_XEXT, pXExtValue);
    aExtentAttributes.set(NS_rtf::LN_YEXT, pYExtValue);
    RTFValue::Pointer_t pExtentValue(new RTFValue(aExtentAttributes));
    // docpr sprm
    RTFSprms aDocprAttributes;
    for (RTFSprms::Iterator_t i = m_aStates.top().aCharacterAttributes.begin(); i != m_aStates.top().aCharacterAttributes.end(); ++i)
        if (i->first == NS_ooxml::LN_CT_NonVisualDrawingProps_name || i->first == NS_ooxml::LN_CT_NonVisualDrawingProps_descr)
            aDocprAttributes.set(i->first, i->second);
    RTFValue::Pointer_t pDocprValue(new RTFValue(aDocprAttributes));
    if (bInline)
    {
        RTFSprms aInlineAttributes;
        aInlineAttributes.set(NS_ooxml::LN_CT_Inline_distT, RTFValue::Pointer_t(new RTFValue(0)));
        aInlineAttributes.set(NS_ooxml::LN_CT_Inline_distB, RTFValue::Pointer_t(new RTFValue(0)));
        aInlineAttributes.set(NS_ooxml::LN_CT_Inline_distL, RTFValue::Pointer_t(new RTFValue(0)));
        aInlineAttributes.set(NS_ooxml::LN_CT_Inline_distR, RTFValue::Pointer_t(new RTFValue(0)));
        RTFSprms aInlineSprms;
        aInlineSprms.set(NS_ooxml::LN_CT_Inline_extent, pExtentValue);
        aInlineSprms.set(NS_ooxml::LN_CT_Inline_docPr, pDocprValue);
        aInlineSprms.set(NS_ooxml::LN_graphic_graphic, pGraphicValue);
        // inline sprm
        RTFValue::Pointer_t pValue(new RTFValue(aInlineAttributes, aInlineSprms));
        aSprms.set(NS_ooxml::LN_inline_inline, pValue);
    }
    else // anchored
    {
        // wrap sprm
        RTFSprms aAnchorWrapAttributes;
        RTFSprms aAnchorAttributes;
        for (RTFSprms::Iterator_t i = m_aStates.top().aCharacterAttributes.begin(); i != m_aStates.top().aCharacterAttributes.end(); ++i)
        {
            if (i->first == NS_ooxml::LN_CT_WrapSquare_wrapText)
                aAnchorWrapAttributes.set(i->first, i->second);
            else if (i->first == NS_rtf::LN_WR)
                aAnchorAttributes.set(i->first, i->second);
        }
        RTFValue::Pointer_t pAnchorWrapValue(new RTFValue(aAnchorWrapAttributes));
        RTFSprms aAnchorSprms;
        aAnchorSprms.set(NS_ooxml::LN_CT_Anchor_extent, pExtentValue);
        if (aAnchorWrapAttributes.size())
            aAnchorSprms.set(NS_ooxml::LN_EG_WrapType_wrapSquare, pAnchorWrapValue);

        // See OOXMLFastContextHandler::positionOffset(), we can't just put values in an RTFValue.
        if (m_aStates.top().aShape.nLeft > 0)
        {
            writerfilter::dmapper::PositionHandler::setPositionOffset(OUString::number(MM100_TO_EMU(m_aStates.top().aShape.nLeft)), false);
            aAnchorSprms.set(NS_ooxml::LN_CT_Anchor_positionH, RTFValue::Pointer_t(new RTFValue(RTFSprms())));
        }
        if (m_aStates.top().aShape.nTop > 0)
        {
            writerfilter::dmapper::PositionHandler::setPositionOffset(OUString::number(MM100_TO_EMU(m_aStates.top().aShape.nTop)), true);
            aAnchorSprms.set(NS_ooxml::LN_CT_Anchor_positionV, RTFValue::Pointer_t(new RTFValue(RTFSprms())));
        }

        aAnchorSprms.set(NS_ooxml::LN_CT_Anchor_docPr, pDocprValue);
        aAnchorSprms.set(NS_ooxml::LN_graphic_graphic, pGraphicValue);
        // anchor sprm
        RTFValue::Pointer_t pValue(new RTFValue(aAnchorAttributes, aAnchorSprms));
        aSprms.set(NS_ooxml::LN_anchor_anchor, pValue);
    }
    writerfilter::Reference<Properties>::Pointer_t const pProperties(new RTFReferenceProperties(aAttributes, aSprms));
    checkFirstRun();

    if (!m_aStates.top().pCurrentBuffer)
    {
        Mapper().props(pProperties);
        // Make sure we don't loose these properties with a too early reset.
        m_bHadPicture = true;
    }
    else
    {
        RTFValue::Pointer_t pValue(new RTFValue(aAttributes, aSprms));
        m_aStates.top().pCurrentBuffer->push_back(make_pair(BUFFER_PROPS, pValue));
    }

    return 0;
}

int RTFDocumentImpl::resolveChars(char ch)
{
    if (m_aStates.top().nInternalState == INTERNAL_BIN)
    {
        m_pBinaryData.reset(new SvMemoryStream());
        *m_pBinaryData << ch;
        for (int i = 0; i < m_aStates.top().nBinaryToRead - 1; ++i)
        {
            Strm() >> ch;
            *m_pBinaryData << ch;
        }
        m_aStates.top().nInternalState = INTERNAL_NORMAL;
        return 0;
    }

    if (m_aStates.top().nInternalState != INTERNAL_HEX)
        checkUnicode(/*bUnicode =*/ false, /*bHex =*/ true);

    OStringBuffer aBuf;

    bool bUnicodeChecked = false;
    bool bSkipped = false;
    while(!Strm().IsEof() && (m_aStates.top().nInternalState == INTERNAL_HEX || (ch != '{' && ch != '}' && ch != '\\')))
    {
        if (m_aStates.top().nInternalState == INTERNAL_HEX || (ch != 0x0d && ch != 0x0a))
        {
            if (m_aStates.top().nCharsToSkip == 0)
            {
                if (!bUnicodeChecked)
                {
                    checkUnicode(/*bUnicode =*/ true, /*bHex =*/ false);
                    bUnicodeChecked = true;
                }
                aBuf.append(ch);
            }
            else
            {
                bSkipped = true;
                m_aStates.top().nCharsToSkip--;
            }
        }
        // read a single char if we're in hex mode
        if (m_aStates.top().nInternalState == INTERNAL_HEX)
            break;
        Strm() >> ch;
    }
    if (m_aStates.top().nInternalState != INTERNAL_HEX && !Strm().IsEof())
        Strm().SeekRel(-1);

    if (m_aStates.top().nInternalState == INTERNAL_HEX && m_aStates.top().nDestinationState != DESTINATION_LEVELNUMBERS)
    {
        if (!bSkipped)
            m_aHexBuffer.append(ch);
        return 0;
    }

    if (m_aStates.top().nDestinationState == DESTINATION_SKIP)
        return 0;
    OString aStr = aBuf.makeStringAndClear();
    if (m_aStates.top().nDestinationState == DESTINATION_LEVELNUMBERS)
    {
        if (aStr.toChar() != ';')
            m_aStates.top().aLevelNumbers.push_back(sal_Int32(ch));
        return 0;
    }

    OUString aOUStr(OStringToOUString(aStr, m_aStates.top().nCurrentEncoding));
    SAL_INFO("writerfilter", OSL_THIS_FUNC << ": collected '" << aOUStr << "'");

    if (m_aStates.top().nDestinationState == DESTINATION_COLORTABLE)
    {
        // we hit a ';' at the end of each color entry
        sal_uInt32 color = (m_aStates.top().aCurrentColor.nRed << 16) | ( m_aStates.top().aCurrentColor.nGreen << 8)
            | m_aStates.top().aCurrentColor.nBlue;
        m_aColorTable.push_back(color);
        // set components back to zero
        m_aStates.top().aCurrentColor = RTFColorTableEntry();
    }
    else if (!aOUStr.isEmpty())
        text(aOUStr);

    return 0;
}

bool RTFFrame::inFrame()
{
    return nW > 0
        || nH > 0
        || nX > 0
        || nY > 0;
}

void RTFDocumentImpl::singleChar(sal_uInt8 nValue, bool bRunProps)
{
    sal_uInt8 sValue[] = { nValue };
    RTFBuffer_t* pCurrentBuffer = m_aStates.top().pCurrentBuffer;

    if (!pCurrentBuffer)
    {
        Mapper().startCharacterGroup();
        // Should we send run properties?
        if (bRunProps)
            runProps();
        Mapper().text(sValue, 1);
        Mapper().endCharacterGroup();
    }
    else
    {
        pCurrentBuffer->push_back(make_pair(BUFFER_STARTRUN, RTFValue::Pointer_t()));
        RTFValue::Pointer_t pValue(new RTFValue(*sValue));
        pCurrentBuffer->push_back(make_pair(BUFFER_TEXT, pValue));
        pCurrentBuffer->push_back(make_pair(BUFFER_ENDRUN, RTFValue::Pointer_t()));
    }
}

void RTFDocumentImpl::text(OUString& rString)
{
    if (rString.getLength() == 1 && m_aStates.top().nDestinationState != DESTINATION_DOCCOMM)
    {
        // No cheating! Tokenizer ignores bare \r and \n, their hex \'0d / \'0a form doesn't count, either.
        sal_Unicode ch = rString.getStr()[0];
        if (ch == 0x0d || ch == 0x0a)
            return;
    }

    bool bRet = true;
    switch (m_aStates.top().nDestinationState)
    {
        case DESTINATION_FONTTABLE:
        case DESTINATION_FONTENTRY:
        case DESTINATION_STYLESHEET:
        case DESTINATION_STYLEENTRY:
        case DESTINATION_REVISIONTABLE:
        case DESTINATION_REVISIONENTRY:
            {
                // ; is the end of the entry
                bool bEnd = false;
                if (rString.endsWithAsciiL(";", 1))
                {
                    rString = rString.copy(0, rString.getLength() - 1);
                    bEnd = true;
                }
                m_aStates.top().aDestinationText.append(rString);
                if (bEnd)
                {
                    switch (m_aStates.top().nDestinationState)
                    {
                        case DESTINATION_FONTTABLE:
                        case DESTINATION_FONTENTRY:
                            {
                                RTFValue::Pointer_t pValue(new RTFValue(m_aStates.top().aDestinationText.makeStringAndClear()));
                                m_aStates.top().aTableAttributes.set(NS_rtf::LN_XSZFFN, pValue);

                                writerfilter::Reference<Properties>::Pointer_t const pProp(
                                        new RTFReferenceProperties(m_aStates.top().aTableAttributes, m_aStates.top().aTableSprms)
                                        );

                                //See fdo#47347 initial invalid font entry properties are inserted first,
                                //so when we attempt to insert the correct ones, there's already an
                                //entry in the map for them, so the new ones aren't inserted.
                                RTFReferenceTable::Entries_t::iterator lb = m_aFontTableEntries.lower_bound(m_nCurrentFontIndex);
                                if (lb != m_aFontTableEntries.end() && !(m_aFontTableEntries.key_comp()(m_nCurrentFontIndex, lb->first)))
                                    lb->second = pProp;
                                else
                                    m_aFontTableEntries.insert(lb, make_pair(m_nCurrentFontIndex, pProp));
                            }
                            break;
                        case DESTINATION_STYLESHEET:
                        case DESTINATION_STYLEENTRY:
                            if (m_aStates.top().aTableAttributes.find(NS_rtf::LN_SGC))
                            {
                                RTFValue::Pointer_t pValue(new RTFValue(m_aStates.top().aDestinationText.makeStringAndClear()));
                                m_aStates.top().aTableAttributes.set(NS_rtf::LN_XSTZNAME1, pValue);

                                writerfilter::Reference<Properties>::Pointer_t const pProp(
                                        new RTFReferenceProperties(mergeAttributes(), mergeSprms())
                                        );
                                m_aStyleTableEntries.insert(make_pair(m_nCurrentStyleIndex, pProp));
                            }
                            else
                                SAL_INFO("writerfilter", "no RTF style type defined, ignoring");
                            break;
                        case DESTINATION_REVISIONTABLE:
                        case DESTINATION_REVISIONENTRY:
                            m_aAuthors[m_aAuthors.size()] = m_aStates.top().aDestinationText.makeStringAndClear();
                            break;
                        default: break;
                    }
                    resetAttributes();
                    resetSprms();
                }
            }
            break;
        case DESTINATION_LEVELTEXT:
        case DESTINATION_SHAPEPROPERTYNAME:
        case DESTINATION_SHAPEPROPERTYVALUE:
        case DESTINATION_BOOKMARKEND:
        case DESTINATION_PICT:
        case DESTINATION_SHAPEPROPERTYVALUEPICT:
        case DESTINATION_FORMFIELDNAME:
        case DESTINATION_FORMFIELDLIST:
        case DESTINATION_DATAFIELD:
        case DESTINATION_AUTHOR:
        case DESTINATION_KEYWORDS:
        case DESTINATION_OPERATOR:
        case DESTINATION_COMPANY:
        case DESTINATION_COMMENT:
        case DESTINATION_OBJDATA:
        case DESTINATION_ANNOTATIONDATE:
        case DESTINATION_ANNOTATIONAUTHOR:
        case DESTINATION_FALT:
        case DESTINATION_PARAGRAPHNUMBERING_TEXTAFTER:
        case DESTINATION_PARAGRAPHNUMBERING_TEXTBEFORE:
        case DESTINATION_TITLE:
        case DESTINATION_SUBJECT:
        case DESTINATION_DOCCOMM:
        case DESTINATION_ATNID:
        case DESTINATION_MR:
        case DESTINATION_MCHR:
        case DESTINATION_MPOS:
        case DESTINATION_MVERTJC:
        case DESTINATION_MSTRIKEH:
        case DESTINATION_MDEGHIDE:
        case DESTINATION_MBEGCHR:
        case DESTINATION_MSEPCHR:
        case DESTINATION_MENDCHR:
        case DESTINATION_MSUBHIDE:
        case DESTINATION_MSUPHIDE:
        case DESTINATION_MTYPE:
        case DESTINATION_MGROW:
            m_aStates.top().aDestinationText.append(rString);
            break;
        default:
            bRet = false;
            break;
    }
    if (bRet)
        return;

    if (!m_aIgnoreFirst.isEmpty() && m_aIgnoreFirst.equals(rString))
    {
        m_aIgnoreFirst = OUString();
        return;
    }

    // Are we in the middle of the table definition? (No cell defs yet, but we already have some cell props.)
    if (m_aStates.top().aTableCellSprms.find(NS_ooxml::LN_CT_TcPrBase_vAlign).get() &&
        m_aStates.top().nCells == 0)
    {
        m_aTableBuffer.push_back(make_pair(BUFFER_UTEXT, RTFValue::Pointer_t(new RTFValue(rString))));
        return;
    }

    checkFirstRun();
    checkNeedPap();

    // Don't return earlier, a bookmark start has to be in a paragraph group.
    if (m_aStates.top().nDestinationState == DESTINATION_BOOKMARKSTART)
    {
        m_aStates.top().aDestinationText.append(rString);
        return;
    }

    RTFBuffer_t* pCurrentBuffer = m_aStates.top().pCurrentBuffer;

    if (!pCurrentBuffer && m_aStates.top().nDestinationState != DESTINATION_FOOTNOTE)
        Mapper().startCharacterGroup();
    else if (pCurrentBuffer)
    {
        RTFValue::Pointer_t pValue;
        pCurrentBuffer->push_back(make_pair(BUFFER_STARTRUN, pValue));
    }

    if (m_aStates.top().nDestinationState == DESTINATION_NORMAL
            || m_aStates.top().nDestinationState == DESTINATION_FIELDRESULT
            || m_aStates.top().nDestinationState == DESTINATION_SHAPETEXT)
        runProps();

    if (!pCurrentBuffer)
        Mapper().utext(reinterpret_cast<sal_uInt8 const*>(rString.getStr()), rString.getLength());
    else
    {
        RTFValue::Pointer_t pValue(new RTFValue(rString));
        pCurrentBuffer->push_back(make_pair(BUFFER_UTEXT, pValue));
    }

    m_bNeedCr = true;

    if (!pCurrentBuffer && m_aStates.top().nDestinationState != DESTINATION_FOOTNOTE)
        Mapper().endCharacterGroup();
    else if(pCurrentBuffer)
    {
        RTFValue::Pointer_t pValue;
        pCurrentBuffer->push_back(make_pair(BUFFER_ENDRUN, pValue));
    }
}

void RTFDocumentImpl::replayBuffer(RTFBuffer_t& rBuffer)
{
    while (rBuffer.size())
    {
        std::pair<RTFBufferTypes, RTFValue::Pointer_t> aPair = rBuffer.front();
        rBuffer.pop_front();
        if (aPair.first == BUFFER_PROPS)
        {
            writerfilter::Reference<Properties>::Pointer_t const pProp(
                    new RTFReferenceProperties(aPair.second->getAttributes(), aPair.second->getSprms())
                    );
            Mapper().props(pProp);
        }
        else if (aPair.first == BUFFER_CELLEND)
        {
            RTFValue::Pointer_t pValue(new RTFValue(1));
            m_aStates.top().aTableCellSprms.set(NS_sprm::LN_PCell, pValue);
            writerfilter::Reference<Properties>::Pointer_t const pTableCellProperties(
                    new RTFReferenceProperties(m_aStates.top().aTableCellAttributes, m_aStates.top().aTableCellSprms)
                    );
            Mapper().props(pTableCellProperties);
            tableBreak();
            break;
        }
        else if (aPair.first == BUFFER_STARTRUN)
            Mapper().startCharacterGroup();
        else if (aPair.first == BUFFER_TEXT)
        {
            sal_uInt8 nValue = aPair.second->getInt();
            Mapper().text(&nValue, 1);
        }
        else if (aPair.first == BUFFER_UTEXT)
        {
            OUString aString(aPair.second->getString());
            Mapper().utext(reinterpret_cast<sal_uInt8 const*>(aString.getStr()), aString.getLength());
        }
        else if (aPair.first == BUFFER_ENDRUN)
            Mapper().endCharacterGroup();
        else if (aPair.first == BUFFER_PAR)
            parBreak();
        else if (aPair.first == BUFFER_STARTSHAPE)
            m_pSdrImport->resolve(aPair.second->getShape(), false);
        else if (aPair.first == BUFFER_ENDSHAPE)
            m_pSdrImport->close();
        else
            SAL_WARN("writerfilter", "should not happen");
    }

}

int RTFDocumentImpl::dispatchDestination(RTFKeyword nKeyword)
{
    checkUnicode(/*bUnicode =*/ true, /*bHex =*/ true);
    setNeedSect();
    RTFSkipDestination aSkip(*this);
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
            m_aStates.top().nDestinationState = DESTINATION_FIELD;
            break;
        case RTF_FLDINST:
            {
                // Look for the field type
                sal_Int32 nPos = Strm().Tell();
                OStringBuffer aBuf;
                char ch = 0;
                bool bFoundCode = false;
                bool bInKeyword = false;
                while (!bFoundCode && ch != '}')
                {
                    Strm() >> ch;
                    if ('\\' == ch)
                        bInKeyword = true;
                    if (!bInKeyword  && isalnum(ch))
                        aBuf.append(ch);
                    else if (bInKeyword && isspace(ch))
                        bInKeyword = false;
                    if (!aBuf.isEmpty() && !isalnum(ch))
                        bFoundCode = true;
                }
                Strm().Seek(nPos);

                // Form data should be handled only for form fields if any
                if (aBuf.toString().indexOf(OString("FORM")) != -1 )
                    m_bFormField = true;

                singleChar(0x13);
                m_aStates.top().nDestinationState = DESTINATION_FIELDINSTRUCTION;
            }
            break;
        case RTF_FLDRSLT:
            m_aStates.top().nDestinationState = DESTINATION_FIELDRESULT;
            break;
        case RTF_LISTTABLE:
            m_aStates.top().nDestinationState = DESTINATION_LISTTABLE;
            break;
        case RTF_LISTPICTURE:
            m_aStates.top().nDestinationState = DESTINATION_LISTPICTURE;
            m_aStates.top().bInListpicture = true;
            break;
        case RTF_LIST:
            m_aStates.top().nDestinationState = DESTINATION_LISTENTRY;
            break;
        case RTF_LFOLEVEL:
            m_aStates.top().nDestinationState = DESTINATION_LFOLEVEL;
            m_aStates.top().aTableSprms.clear();
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
        case RTF_LEVELTEXT:
            m_aStates.top().nDestinationState = DESTINATION_LEVELTEXT;
            break;
        case RTF_LEVELNUMBERS:
            m_aStates.top().nDestinationState = DESTINATION_LEVELNUMBERS;
            break;
        case RTF_SHPPICT:
            m_aStates.top().resetFrame();
            m_aStates.top().nDestinationState = DESTINATION_SHPPICT;
            break;
        case RTF_PICT:
            if (m_aStates.top().nDestinationState != DESTINATION_SHAPEPROPERTYVALUE)
                m_aStates.top().nDestinationState = DESTINATION_PICT; // as character
            else
                m_aStates.top().nDestinationState = DESTINATION_SHAPEPROPERTYVALUEPICT; // anchored inside a shape
            break;
        case RTF_PICPROP:
            m_aStates.top().nDestinationState = DESTINATION_PICPROP;
            break;
        case RTF_SP:
            m_aStates.top().nDestinationState = DESTINATION_SHAPEPROPERTY;
            break;
        case RTF_SN:
            m_aStates.top().nDestinationState = DESTINATION_SHAPEPROPERTYNAME;
            break;
        case RTF_SV:
            m_aStates.top().nDestinationState = DESTINATION_SHAPEPROPERTYVALUE;
            break;
        case RTF_SHP:
            m_bNeedCrOrig = m_bNeedCr;
            m_aStates.top().nDestinationState = DESTINATION_SHAPE;
            break;
        case RTF_SHPINST:
            m_aStates.top().nDestinationState = DESTINATION_SHAPEINSTRUCTION;
            break;
        case RTF_NESTTABLEPROPS:
            // Don't try to support nested tables having table styles for now.
            if (!m_aStates.top().bHasTableStyle)
                m_aStates.top().nDestinationState = DESTINATION_NESTEDTABLEPROPERTIES;
            else
                m_aStates.top().nDestinationState = DESTINATION_SKIP;
            break;
        case RTF_HEADER:
        case RTF_FOOTER:
        case RTF_HEADERL:
        case RTF_HEADERR:
        case RTF_HEADERF:
        case RTF_FOOTERL:
        case RTF_FOOTERR:
        case RTF_FOOTERF:
            if (!m_pSuperstream)
            {
                Id nId = 0;
                sal_uInt32 nPos = m_nGroupStartPos - 1;
                switch (nKeyword)
                {
                    case RTF_HEADER: nId = NS_rtf::LN_headerr; break;
                    case RTF_FOOTER: nId = NS_rtf::LN_footerr; break;
                    case RTF_HEADERL: nId = NS_rtf::LN_headerl; break;
                    case RTF_HEADERR: nId = NS_rtf::LN_headerr; break;
                    case RTF_HEADERF: nId = NS_rtf::LN_headerf; break;
                    case RTF_FOOTERL: nId = NS_rtf::LN_footerl; break;
                    case RTF_FOOTERR: nId = NS_rtf::LN_footerr; break;
                    case RTF_FOOTERF: nId = NS_rtf::LN_footerf; break;
                    default: break;
                }
                m_nHeaderFooterPositions.push(make_pair(nId, nPos));
                m_aStates.top().nDestinationState = DESTINATION_SKIP;
            }
            break;
        case RTF_FOOTNOTE:
            if (!m_pSuperstream)
            {
                Id nId = NS_rtf::LN_footnote;

                // Check if this is an endnote.
                OStringBuffer aBuf;
                char ch;
                for (int i = 0; i < 7; ++i)
                {
                    Strm() >> ch;
                    aBuf.append(ch);
                }
                OString aKeyword = aBuf.makeStringAndClear();
                if (aKeyword.equals("\\ftnalt"))
                    nId = NS_rtf::LN_endnote;

                m_bHasFootnote = true;
                if (m_aStates.top().pCurrentBuffer == &m_aSuperBuffer)
                    m_aStates.top().pCurrentBuffer = 0;
                bool bCustomMark = false;
                OUString aCustomMark;
                while (m_aSuperBuffer.size())
                {
                    std::pair<RTFBufferTypes, RTFValue::Pointer_t> aPair = m_aSuperBuffer.front();
                    m_aSuperBuffer.pop_front();
                    if (aPair.first == BUFFER_UTEXT)
                    {
                        aCustomMark = aPair.second->getString();
                        bCustomMark = true;
                    }
                }
                m_aStates.top().nDestinationState = DESTINATION_FOOTNOTE;
                if (bCustomMark)
                    Mapper().startCharacterGroup();
                resolveSubstream(m_nGroupStartPos - 1, nId, aCustomMark);
                if (bCustomMark)
                {
                    m_aStates.top().aCharacterAttributes.clear();
                    m_aStates.top().aCharacterSprms.clear();
                    RTFValue::Pointer_t pValue(new RTFValue(1));
                    m_aStates.top().aCharacterAttributes.set(NS_ooxml::LN_CT_FtnEdnRef_customMarkFollows, pValue);
                    text(aCustomMark);
                    Mapper().endCharacterGroup();
                }
                m_aStates.top().nDestinationState = DESTINATION_SKIP;
            }
            break;
        case RTF_BKMKSTART:
            m_aStates.top().nDestinationState = DESTINATION_BOOKMARKSTART;
            break;
        case RTF_BKMKEND:
            m_aStates.top().nDestinationState = DESTINATION_BOOKMARKEND;
            break;
        case RTF_REVTBL:
            m_aStates.top().nDestinationState = DESTINATION_REVISIONTABLE;
            break;
        case RTF_ANNOTATION:
            if (!m_pSuperstream)
            {
                resolveSubstream(m_nGroupStartPos - 1, NS_rtf::LN_annotation);
                m_aStates.top().nDestinationState = DESTINATION_SKIP;
            }
            else
            {
                // If there is an author set, emit it now.
                if (!m_aAuthor.isEmpty() || !m_aAuthorInitials.isEmpty())
                {
                    RTFSprms aAttributes;
                    if (!m_aAuthor.isEmpty())
                    {
                        RTFValue::Pointer_t pValue(new RTFValue(m_aAuthor));
                        aAttributes.set(NS_ooxml::LN_CT_TrackChange_author, pValue);
                    }
                    if (!m_aAuthorInitials.isEmpty())
                    {
                        RTFValue::Pointer_t pValue(new RTFValue(m_aAuthorInitials));
                        aAttributes.set(NS_ooxml::LN_CT_Comment_initials, pValue);
                    }
                    writerfilter::Reference<Properties>::Pointer_t const pProperties(new RTFReferenceProperties(aAttributes));
                    Mapper().props(pProperties);
                }
            }
            break;
        case RTF_SHPTXT:
        case RTF_DPTXBXTEXT:
            m_aStates.top().nDestinationState = DESTINATION_SHAPETEXT;
            checkFirstRun();
            dispatchFlag(RTF_PARD);
            m_bNeedPap = true;
            if (nKeyword == RTF_SHPTXT)
            {
                if (!m_aStates.top().pCurrentBuffer)
                    m_pSdrImport->resolve(m_aStates.top().aShape, false);
                else
                {
                    RTFValue::Pointer_t pValue(new RTFValue(m_aStates.top().aShape));
                    m_aStates.top().pCurrentBuffer->push_back(make_pair(BUFFER_STARTSHAPE, pValue));
                }
            }
            break;
        case RTF_FORMFIELD:
            if (m_aStates.top().nDestinationState == DESTINATION_FIELDINSTRUCTION)
                m_aStates.top().nDestinationState = DESTINATION_FORMFIELD;
            break;
        case RTF_FFNAME:
            m_aStates.top().nDestinationState = DESTINATION_FORMFIELDNAME;
            break;
        case RTF_FFL:
            m_aStates.top().nDestinationState = DESTINATION_FORMFIELDLIST;
            break;
        case RTF_DATAFIELD:
            m_aStates.top().nDestinationState = DESTINATION_DATAFIELD;
            break;
        case RTF_INFO:
            m_aStates.top().nDestinationState = DESTINATION_INFO;
            break;
        case RTF_CREATIM:
            m_aStates.top().nDestinationState = DESTINATION_CREATIONTIME;
            break;
        case RTF_REVTIM:
            m_aStates.top().nDestinationState = DESTINATION_REVISIONTIME;
            break;
        case RTF_PRINTIM:
            m_aStates.top().nDestinationState = DESTINATION_PRINTTIME;
            break;
        case RTF_AUTHOR:
            m_aStates.top().nDestinationState = DESTINATION_AUTHOR;
            break;
        case RTF_KEYWORDS:
            m_aStates.top().nDestinationState = DESTINATION_KEYWORDS;
            break;
        case RTF_OPERATOR:
            m_aStates.top().nDestinationState = DESTINATION_OPERATOR;
            break;
        case RTF_COMPANY:
            m_aStates.top().nDestinationState = DESTINATION_COMPANY;
            break;
        case RTF_COMMENT:
            m_aStates.top().nDestinationState = DESTINATION_COMMENT;
            break;
        case RTF_OBJECT:
            {
                // beginning of an OLE Object
                m_aStates.top().nDestinationState = DESTINATION_OBJECT;

                // check if the object is in a special container (e.g. a table)
                if (!m_aStates.top().pCurrentBuffer)
                {
                    // the object is in a table or another container.
                    // Don't try to treate it as an OLE object (fdo#53594).
                    // Use the \result (RTF_RESULT) element of the object instead,
                    // the result element contain picture representing the OLE Object.
                    m_bObject = true;
                }
            }
            break;
        case RTF_OBJDATA:
            // check if the object is in a special container (e.g. a table)
            if (m_aStates.top().pCurrentBuffer)
            {
                // the object is in a table or another container.
                // Use the \result (RTF_RESULT) element of the object instead,
                // of the \objdata.
                m_aStates.top().nDestinationState = DESTINATION_SKIP;
            }
            else
            {
                m_aStates.top().nDestinationState = DESTINATION_OBJDATA;
            }
            break;
        case RTF_RESULT:
            m_aStates.top().nDestinationState = DESTINATION_RESULT;
            break;
        case RTF_ATNDATE:
            m_aStates.top().nDestinationState = DESTINATION_ANNOTATIONDATE;
            break;
        case RTF_ATNAUTHOR:
            m_aStates.top().nDestinationState = DESTINATION_ANNOTATIONAUTHOR;
            break;
        case RTF_FALT:
            m_aStates.top().nDestinationState = DESTINATION_FALT;
            break;
        case RTF_FLYMAINCNT:
            m_aStates.top().nDestinationState = DESTINATION_FLYMAINCONTENT;
            break;
        case RTF_LISTTEXT:
            // Should be ignored by any reader that understands Word 97 through Word 2007 numbering.
        case RTF_NONESTTABLES:
            // This destination should be ignored by readers that support nested tables.
            m_aStates.top().nDestinationState = DESTINATION_SKIP;
            break;
        case RTF_DO:
            m_aStates.top().nDestinationState = DESTINATION_DRAWINGOBJECT;
            break;
        case RTF_PN:
            m_aStates.top().nDestinationState = DESTINATION_PARAGRAPHNUMBERING;
            break;
        case RTF_PNTEXT:
            // This destination should be ignored by readers that support paragraph numbering.
            m_aStates.top().nDestinationState = DESTINATION_SKIP;
            break;
        case RTF_PNTXTA:
            m_aStates.top().nDestinationState = DESTINATION_PARAGRAPHNUMBERING_TEXTAFTER;
            break;
        case RTF_PNTXTB:
            m_aStates.top().nDestinationState = DESTINATION_PARAGRAPHNUMBERING_TEXTBEFORE;
            break;
        case RTF_TITLE:
            // \title inside \upr but outside \ud should be ignored.
            if (m_aStates.top().nDestinationState != DESTINATION_UPR)
                m_aStates.top().nDestinationState = DESTINATION_TITLE;
            else
                m_aStates.top().nDestinationState = DESTINATION_SKIP;
            break;
        case RTF_SUBJECT:
            m_aStates.top().nDestinationState = DESTINATION_SUBJECT;
            break;
        case RTF_DOCCOMM:
            m_aStates.top().nDestinationState = DESTINATION_DOCCOMM;
            break;
        case RTF_ATRFSTART:
        case RTF_ATRFEND:
            {
                // We could send the real value here, but that would make the
                // tokenizer more complicated, and dmapper doesn't read the
                // result anyway.
                RTFValue::Pointer_t pValue(new RTFValue(0));
                m_aStates.top().nDestinationState = DESTINATION_SKIP;

                RTFSprms aAttributes;
                if (nKeyword == RTF_ATRFSTART)
                    aAttributes.set(NS_ooxml::LN_EG_RangeMarkupElements_commentRangeStart, pValue);
                else
                    aAttributes.set(NS_ooxml::LN_EG_RangeMarkupElements_commentRangeEnd, pValue);
                writerfilter::Reference<Properties>::Pointer_t const pProperties(new RTFReferenceProperties(aAttributes));
                Mapper().props(pProperties);
            }
            break;
        case RTF_ATNID:
            m_aStates.top().nDestinationState = DESTINATION_ATNID;
            break;
        case RTF_MMATH:
        case RTF_MOMATHPARA:
            // Nothing to do here (just enter the destination) till RTF_MMATHPR is implemented.
            break;
        case RTF_MR: m_aStates.top().nDestinationState = DESTINATION_MR; break;
        case RTF_MCHR: m_aStates.top().nDestinationState = DESTINATION_MCHR; break;
        case RTF_MPOS: m_aStates.top().nDestinationState = DESTINATION_MPOS; break;
        case RTF_MVERTJC: m_aStates.top().nDestinationState = DESTINATION_MVERTJC; break;
        case RTF_MSTRIKEH: m_aStates.top().nDestinationState = DESTINATION_MSTRIKEH; break;
        case RTF_MDEGHIDE: m_aStates.top().nDestinationState = DESTINATION_MDEGHIDE; break;
        case RTF_MTYPE: m_aStates.top().nDestinationState = DESTINATION_MTYPE; break;
        case RTF_MGROW: m_aStates.top().nDestinationState = DESTINATION_MGROW; break;
        case RTF_MHIDETOP:
        case RTF_MHIDEBOT:
        case RTF_MHIDELEFT:
        case RTF_MHIDERIGHT:
            // SmOoxmlImport::handleBorderBox will ignore these anyway, so silently ignore for now.
            m_aStates.top().nDestinationState = DESTINATION_SKIP;
            break;
        case RTF_MSUBHIDE: m_aStates.top().nDestinationState = DESTINATION_MSUBHIDE; break;
        case RTF_MSUPHIDE: m_aStates.top().nDestinationState = DESTINATION_MSUPHIDE; break;
        case RTF_MBEGCHR: m_aStates.top().nDestinationState = DESTINATION_MBEGCHR; break;
        case RTF_MSEPCHR: m_aStates.top().nDestinationState = DESTINATION_MSEPCHR; break;
        case RTF_MENDCHR: m_aStates.top().nDestinationState = DESTINATION_MENDCHR; break;
        OPEN_M_TOKEN(OMATH, oMath);
        OPEN_M_TOKEN(F, f);
        OPEN_M_TOKEN(FPR, fPr);
        OPEN_M_TOKEN(CTRLPR, ctrlPr);
        OPEN_M_TOKEN(NUM, num);
        OPEN_M_TOKEN(DEN, den);
        OPEN_M_TOKEN(ACC, acc);
        OPEN_M_TOKEN(ACCPR, accPr);
        OPEN_M_TOKEN(BAR, bar);
        OPEN_M_TOKEN(BARPR, barPr);
        OPEN_M_TOKEN(E, e);
        OPEN_M_TOKEN(D, d);
        OPEN_M_TOKEN(DPR, dPr);
        OPEN_M_TOKEN(FUNC, func);
        OPEN_M_TOKEN(FUNCPR, funcPr);
        OPEN_M_TOKEN(FNAME, fName);
        OPEN_M_TOKEN(LIMLOW, limLow);
        OPEN_M_TOKEN(LIMLOWPR, limLowPr);
        OPEN_M_TOKEN(LIM, lim);
        OPEN_M_TOKEN(M, m);
        OPEN_M_TOKEN(MPR, mPr);
        OPEN_M_TOKEN(MR, mr);
        OPEN_M_TOKEN(NARY, nary);
        OPEN_M_TOKEN(NARYPR, naryPr);
        OPEN_M_TOKEN(SUB, sub);
        OPEN_M_TOKEN(SUP, sup);
        OPEN_M_TOKEN(LIMUPP, limUpp);
        OPEN_M_TOKEN(LIMUPPPR, limUppPr);
        OPEN_M_TOKEN(GROUPCHR, groupChr);
        OPEN_M_TOKEN(GROUPCHRPR, groupChrPr);
        OPEN_M_TOKEN(BORDERBOX, borderBox);
        OPEN_M_TOKEN(BORDERBOXPR, borderBoxPr);
        OPEN_M_TOKEN(RAD, rad);
        OPEN_M_TOKEN(RADPR, radPr);
        OPEN_M_TOKEN(DEG, deg);
        OPEN_M_TOKEN(SSUB, sSub);
        OPEN_M_TOKEN(SSUBPR, sSubPr);
        OPEN_M_TOKEN(SSUP, sSup);
        OPEN_M_TOKEN(SSUPPR, sSupPr);
        OPEN_M_TOKEN(SSUBSUP, sSubSup);
        OPEN_M_TOKEN(SSUBSUPPR, sSubSupPr);
        OPEN_M_TOKEN(SPRE, sPre);
        OPEN_M_TOKEN(SPREPR, sPrePr);
        OPEN_M_TOKEN(BOX, box);
        OPEN_M_TOKEN(EQARR, eqArr);
        case RTF_UPR:
            m_aStates.top().nDestinationState = DESTINATION_UPR;
            break;
        case RTF_UD:
            // Anything inside \ud is just normal Unicode content.
            m_aStates.top().nDestinationState = DESTINATION_NORMAL;
            break;
        case RTF_BACKGROUND:
            m_aStates.top().nDestinationState = DESTINATION_BACKGROUND;
            m_aStates.top().bInBackground = true;
            break;
        case RTF_SHPGRP:
            {
                RTFLookahead aLookahead(Strm(), m_pTokenizer->getGroupStart());
                if (!aLookahead.hasTable())
                {
                    uno::Reference<drawing::XShapes> xGroupShape(m_xModelFactory->createInstance("com.sun.star.drawing.GroupShape"), uno::UNO_QUERY);
                    uno::Reference<drawing::XDrawPageSupplier> xDrawSupplier(m_xDstDoc, uno::UNO_QUERY);
                    if (xDrawSupplier.is())
                    {
                        uno::Reference<drawing::XShape> xShape(xGroupShape, uno::UNO_QUERY);
                        xDrawSupplier->getDrawPage()->add(xShape);
                    }
                    m_pSdrImport->pushParent(xGroupShape);
                    m_aStates.top().bCreatedShapeGroup = true;
                }
                m_aStates.top().nDestinationState = DESTINATION_SHAPEGROUP;
                m_aStates.top().bInShapeGroup = true;
            }
            break;
        default:
            SAL_INFO("writerfilter", "TODO handle destination '" << lcl_RtfToString(nKeyword) << "'");
            // Make sure we skip destinations (even without \*) till we don't handle them
            m_aStates.top().nDestinationState = DESTINATION_SKIP;
            aSkip.setParsed(false);
            break;
    }

    return 0;
}

int RTFDocumentImpl::dispatchSymbol(RTFKeyword nKeyword)
{
    if (nKeyword != RTF_HEXCHAR)
        checkUnicode(/*bUnicode =*/ true, /*bHex =*/ true);
    else
        checkUnicode(/*bUnicode =*/ true, /*bHex =*/ false);
    setNeedSect();
    RTFSkipDestination aSkip(*this);

    if (RTF_LINE == nKeyword)
    {   // very special handling since text() will eat lone '\n'
        singleChar('\n');
        return 0;
    }
    // Trivial symbols
    sal_uInt8 cCh = 0;
    switch (nKeyword)
    {
        case RTF_TAB: cCh = '\t'; break;
        case RTF_BACKSLASH: cCh = '\\'; break;
        case RTF_LBRACE: cCh = '{'; break;
        case RTF_RBRACE: cCh = '}'; break;
        case RTF_EMDASH: cCh = 151; break;
        case RTF_ENDASH: cCh = 150; break;
        case RTF_BULLET: cCh = 149; break;
        case RTF_LQUOTE: cCh = 145; break;
        case RTF_RQUOTE: cCh = 146; break;
        case RTF_LDBLQUOTE: cCh = 147; break;
        case RTF_RDBLQUOTE: cCh = 148; break;
        default: break;
    }
    if (cCh > 0)
    {
        OUString aStr(OStringToOUString(OString(cCh), RTL_TEXTENCODING_MS_1252));
        text(aStr);
        return 0;
    }

    switch (nKeyword)
    {
        case RTF_IGNORE:
            {
                m_bSkipUnknown = true;
                aSkip.setReset(false);
                return 0;
            }
            break;
        case RTF_PAR:
            {
                checkFirstRun();
                bool bNeedPap = m_bNeedPap;
                checkNeedPap();
                if (bNeedPap)
                    runProps();
                if (!m_aStates.top().pCurrentBuffer)
                {
                    parBreak();
                    // Not in table? Reset max width.
                    m_nCellxMax = 0;
                }
                else if (m_aStates.top().nDestinationState != DESTINATION_SHAPETEXT)
                {
                    RTFValue::Pointer_t pValue;
                    m_aStates.top().pCurrentBuffer->push_back(make_pair(BUFFER_PAR, pValue));
                }
                // but don't emit properties yet, since they may change till the first text token arrives
                m_bNeedPap = true;
                if (!m_aStates.top().aFrame.inFrame())
                    m_bNeedPar = false;
                m_bNeedFinalPar = false;
            }
            break;
        case RTF_SECT:
            {
                m_bHadSect = true;
                if (m_bIgnoreNextContSectBreak)
                    m_bIgnoreNextContSectBreak = false;
                else
                    sectBreak();
            }
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
        case RTF_HEXCHAR:
            m_aStates.top().nInternalState = INTERNAL_HEX;
            break;
        case RTF_CELL:
        case RTF_NESTCELL:
            {
                checkFirstRun();
                if (m_bNeedPap)
                {
                    // There were no runs in the cell, so we need to send paragraph and character properties here.
                    RTFValue::Pointer_t pPValue(new RTFValue(m_aStates.top().aParagraphAttributes, m_aStates.top().aParagraphSprms));
                    m_aTableBuffer.push_back(make_pair(BUFFER_PROPS, pPValue));
                    RTFValue::Pointer_t pCValue(new RTFValue(m_aStates.top().aCharacterAttributes, m_aStates.top().aCharacterSprms));
                    m_aTableBuffer.push_back(make_pair(BUFFER_PROPS, pCValue));
                }

                RTFValue::Pointer_t pValue;
                m_aTableBuffer.push_back(make_pair(BUFFER_CELLEND, pValue));
                m_bNeedPap = true;
            }
            break;
        case RTF_ROW:
        case RTF_NESTROW:
            {
                // If the right edge of the last cell (row width) is smaller than the width of some other row, mimic WW8TabDesc::CalcDefaults(): add a fake cell.
                const int MINLAY = 23; // sw/inc/swtypes.hxx, minimal possible size of frames.
                if (nKeyword == RTF_ROW && (m_nCellxMax - m_aStates.top().nCellX) >= MINLAY)
                    dispatchValue(RTF_CELLX, m_nCellxMax);

                if (m_aStates.top().nCells)
                {
                    // Make a backup before we start popping elements
                    m_aStates.top().aTableInheritingCellsSprms = m_aStates.top().aTableCellsSprms;
                    m_aStates.top().aTableInheritingCellsAttributes = m_aStates.top().aTableCellsAttributes;
                    m_aStates.top().nInheritingCells = m_aStates.top().nCells;
                }
                else
                {
                    // No table definition? Then inherit from the previous row
                    m_aStates.top().aTableCellsSprms = m_aStates.top().aTableInheritingCellsSprms;
                    m_aStates.top().aTableCellsAttributes = m_aStates.top().aTableInheritingCellsAttributes;
                    m_aStates.top().nCells = m_aStates.top().nInheritingCells;
                    // This can't be the first row, and we need cell width only there
                    while(m_aStates.top().aTableRowSprms.erase(NS_ooxml::LN_CT_TblGridBase_gridCol)) ;
                }
                for (int i = 0; i < m_aStates.top().nCells; ++i)
                {
                    m_aStates.top().aTableCellSprms = m_aStates.top().aTableCellsSprms.front();
                    m_aStates.top().aTableCellsSprms.pop_front();
                    m_aStates.top().aTableCellAttributes = m_aStates.top().aTableCellsAttributes.front();
                    m_aStates.top().aTableCellsAttributes.pop_front();
                    replayBuffer(m_aTableBuffer);
                }
                m_aStates.top().aTableCellSprms = m_aDefaultState.aTableCellSprms;
                m_aStates.top().aTableCellAttributes = m_aDefaultState.aTableCellAttributes;

                writerfilter::Reference<Properties>::Pointer_t const pParagraphProperties(
                        getProperties(m_aStates.top().aParagraphAttributes, m_aStates.top().aParagraphSprms)
                        );
                Mapper().props(pParagraphProperties);

                if (m_aStates.top().aFrame.hasProperties())
                {
                    writerfilter::Reference<Properties>::Pointer_t const pFrameProperties(
                            new RTFReferenceProperties(RTFSprms(), m_aStates.top().aFrame.getSprms()));
                    Mapper().props(pFrameProperties);
                }

                // Table width.
                RTFValue::Pointer_t pUnitValue(new RTFValue(3));
                lcl_putNestedAttribute(m_aStates.top().aTableRowSprms,
                        NS_ooxml::LN_CT_TblPrBase_tblW, NS_ooxml::LN_CT_TblWidth_type, pUnitValue);
                RTFValue::Pointer_t pWValue(new RTFValue(m_aStates.top().nCellX));
                lcl_putNestedAttribute(m_aStates.top().aTableRowSprms,
                        NS_ooxml::LN_CT_TblPrBase_tblW, NS_ooxml::LN_CT_TblWidth_w, pWValue);

                RTFValue::Pointer_t pRowValue(new RTFValue(1));
                if (m_aStates.top().nCells > 0)
                    m_aStates.top().aTableRowSprms.set(NS_sprm::LN_PRow, pRowValue);

                RTFValue::Pointer_t pCellMar = m_aStates.top().aTableRowSprms.find(NS_ooxml::LN_CT_TblPrBase_tblCellMar);
                if (!pCellMar.get())
                {
                    // If no cell margins are defined, the default left/right margin is 0 in Word, but not in Writer.
                    RTFSprms aAttributes;
                    aAttributes.set(NS_ooxml::LN_CT_TblWidth_type, RTFValue::Pointer_t(new RTFValue(NS_ooxml::LN_Value_ST_TblWidth_dxa)));
                    aAttributes.set(NS_ooxml::LN_CT_TblWidth_w, RTFValue::Pointer_t(new RTFValue(0)));
                    lcl_putNestedSprm(m_aStates.top().aTableRowSprms,
                            NS_ooxml::LN_CT_TblPrBase_tblCellMar, NS_ooxml::LN_CT_TblCellMar_left,
                            RTFValue::Pointer_t(new RTFValue(aAttributes)));
                    lcl_putNestedSprm(m_aStates.top().aTableRowSprms,
                            NS_ooxml::LN_CT_TblPrBase_tblCellMar, NS_ooxml::LN_CT_TblCellMar_right,
                            RTFValue::Pointer_t(new RTFValue(aAttributes)));
                }

                writerfilter::Reference<Properties>::Pointer_t const pTableRowProperties(
                        new RTFReferenceProperties(m_aStates.top().aTableRowAttributes, m_aStates.top().aTableRowSprms)
                        );
                Mapper().props(pTableRowProperties);

                tableBreak();
                m_bNeedPap = true;
                m_bNeedFinalPar = true;
                m_aTableBuffer.clear();
                m_aStates.top().nCells = 0;
                m_aStates.top().aTableCellsSprms.clear();
                m_aStates.top().aTableCellsAttributes.clear();
            }
            break;
        case RTF_COLUMN:
            {
                bool bColumns = false; // If we have multiple columns
                RTFValue::Pointer_t pCols = m_aStates.top().aSectionSprms.find(NS_ooxml::LN_EG_SectPrContents_cols);
                if (pCols.get())
                {
                    RTFValue::Pointer_t pNum = pCols->getAttributes().find(NS_ooxml::LN_CT_Columns_num);
                    if (pNum.get() && pNum->getInt() > 1)
                        bColumns = true;
                }
                if (bColumns)
                {
                    sal_uInt8 sBreak[] = { 0xe };
                    Mapper().startCharacterGroup();
                    Mapper().text(sBreak, 1);
                    Mapper().endCharacterGroup();
                }
                else
                    dispatchSymbol(RTF_PAGE);
            }
            break;
        case RTF_CHFTN:
            // Nothing to do, dmapper assumes this is the default.
            break;
        case RTF_PAGE:
            {
                // If we're inside a continous section, we should send a section break, not a page one.
                RTFValue::Pointer_t pBreak = m_aStates.top().aSectionSprms.find(NS_sprm::LN_SBkc);
                // Unless we're on a title page.
                RTFValue::Pointer_t pTitlePg = m_aStates.top().aSectionSprms.find(NS_ooxml::LN_EG_SectPrContents_titlePg);
                if ((pBreak.get() && !pBreak->getInt()) && !(pTitlePg.get() && pTitlePg->getInt()))
                {
                    if (m_bWasInFrame)
                    {
                        dispatchSymbol(RTF_PAR);
                        m_bWasInFrame = false;
                    }
                    dispatchFlag(RTF_SBKPAGE);
                    sectBreak();
                    dispatchFlag(RTF_SBKNONE);
                    if (m_bNeedPar)
                        dispatchSymbol(RTF_PAR);
                    m_bIgnoreNextContSectBreak = true;
                }
                else
                {
                    checkNeedPap();
                    sal_uInt8 sBreak[] = { 0xc };
                    Mapper().text(sBreak, 1);
                    if (!m_bNeedPap)
                        parBreak();
                    m_bNeedCr = true;
                }
            }
            break;
        case RTF_CHPGN:
            {
                OUString aStr("PAGE");
                singleChar(0x13);
                text(aStr);
                singleChar(0x14, true);
                singleChar(0x15);
            }
            break;
        default:
            {
                SAL_INFO("writerfilter", "TODO handle symbol '" << lcl_RtfToString(nKeyword) << "'");
                aSkip.setParsed(false);
            }
            break;
    }
    return 0;
}

int RTFDocumentImpl::dispatchFlag(RTFKeyword nKeyword)
{
    checkUnicode(/*bUnicode =*/ true, /*bHex =*/ true);
    setNeedSect();
    RTFSkipDestination aSkip(*this);
    int nParam = -1;
    int nSprm = -1;

    // Map all underline flags to a single sprm.
    switch (nKeyword)
    {
        case RTF_ULD: nSprm = 4; break;
        case RTF_ULW: nSprm = 2; break;
        default: break;
    }
    if (nSprm >= 0)
    {
        RTFValue::Pointer_t pValue(new RTFValue(nSprm));
        m_aStates.top().aCharacterSprms.set(NS_sprm::LN_CKul, pValue);
        return 0;
    }

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
    if (nParam >= 0)
    {
        RTFValue::Pointer_t pValue(new RTFValue(nParam));
        m_aStates.top().aParagraphSprms.set(NS_sprm::LN_PJc, pValue);
        m_bNeedPap = true;
        return 0;
    }

    // Font Alignment
    switch (nKeyword)
    {
        case RTF_FAFIXED:
        case RTF_FAAUTO: nParam = NS_ooxml::LN_Value_wordprocessingml_ST_TextAlignment_auto; break;
        case RTF_FAHANG: nParam = NS_ooxml::LN_Value_wordprocessingml_ST_TextAlignment_top; break;
        case RTF_FACENTER: nParam = NS_ooxml::LN_Value_wordprocessingml_ST_TextAlignment_center; break;
        case RTF_FAROMAN: nParam = NS_ooxml::LN_Value_wordprocessingml_ST_TextAlignment_baseline; break;
        case RTF_FAVAR: nParam = NS_ooxml::LN_Value_wordprocessingml_ST_TextAlignment_bottom; break;
        default: break;
    }
    if (nParam >= 0)
    {
        RTFValue::Pointer_t pValue(new RTFValue(nParam));
        m_aStates.top().aParagraphSprms.set(NS_sprm::LN_PWAlignFont, pValue);
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
    if (nParam >= 0)
    {
        RTFValue::Pointer_t pValue(new RTFValue(nParam));
        m_aStates.top().aTabAttributes.set(NS_ooxml::LN_CT_TabStop_val, pValue);
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
    if (nParam >= 0)
    {
        RTFValue::Pointer_t pValue(new RTFValue(nParam));
        m_aStates.top().aTabAttributes.set(NS_ooxml::LN_CT_TabStop_leader, pValue);
        return 0;
    }

    // Border types
    {
        switch (nKeyword)
        {
            // brdrhair and brdrs are the same, brdrw will make a difference
            // map to values in ooxml/model.xml resource ST_Border
            case RTF_BRDRHAIR: nParam = 5; break;
            case RTF_BRDRS: nParam = 1; break;
            case RTF_BRDRDOT: nParam = 6; break;
            case RTF_BRDRDASH: nParam = 7; break;
            case RTF_BRDRDB: nParam = 3; break;
            case RTF_BRDRTNTHSG: nParam = 11; break;
            case RTF_BRDRTNTHMG: nParam = 14; break;
            case RTF_BRDRTNTHLG: nParam = 17; break;
            case RTF_BRDRTHTNSG: nParam = 12; break;
            case RTF_BRDRTHTNMG: nParam = 15; break;
            case RTF_BRDRTHTNLG: nParam = 18; break;
            case RTF_BRDREMBOSS: nParam = 24; break;
            case RTF_BRDRENGRAVE: nParam = 25; break;
            case RTF_BRDROUTSET: nParam = 18; break;
            case RTF_BRDRINSET: nParam = 17; break;
            case RTF_BRDRNONE: nParam = 0; break;
            default: break;
        }
        if (nParam >= 0)
        {
            RTFValue::Pointer_t pValue(new RTFValue(nParam));
            lcl_putBorderProperty(m_aStates, NS_rtf::LN_BRCTYPE, pValue);
            return 0;
        }
    }

    // Section breaks
    switch (nKeyword)
    {
        case RTF_SBKNONE: nParam = 0; break;
        case RTF_SBKCOL: nParam = 1; break;
        case RTF_SBKPAGE: nParam = 2; break;
        case RTF_SBKEVEN: nParam = 3; break;
        case RTF_SBKODD: nParam = 4; break;
        default: break;
    }
    if (nParam >= 0)
    {
        RTFValue::Pointer_t pValue(new RTFValue(nParam));
        m_aStates.top().aSectionSprms.set(NS_sprm::LN_SBkc, pValue);
        return 0;
    }

    // Footnote numbering
    switch (nKeyword)
    {
        case RTF_FTNNAR: nParam = NS_ooxml::LN_Value_ST_NumberFormat_decimal; break;
        case RTF_FTNNALC: nParam = NS_ooxml::LN_Value_ST_NumberFormat_lowerLetter; break;
        case RTF_FTNNAUC: nParam = NS_ooxml::LN_Value_ST_NumberFormat_upperLetter; break;
        case RTF_FTNNRLC: nParam = NS_ooxml::LN_Value_ST_NumberFormat_lowerRoman; break;
        case RTF_FTNNRUC: nParam = NS_ooxml::LN_Value_ST_NumberFormat_upperRoman; break;
        case RTF_FTNNCHI: nParam = NS_ooxml::LN_Value_ST_NumberFormat_chicago; break;
        default: break;
    }
    if (nParam >= 0)
    {
        RTFValue::Pointer_t pValue(new RTFValue(nParam));
        lcl_putNestedSprm(m_aDefaultState.aParagraphSprms, NS_ooxml::LN_EG_SectPrContents_footnotePr, NS_ooxml::LN_CT_FtnProps_numFmt, pValue);
        return 0;
    }

    // Footnote restart type
    switch (nKeyword)
    {
        case RTF_FTNRSTPG: nParam = NS_ooxml::LN_Value_ST_RestartNumber_eachPage; break;
        case RTF_FTNRESTART: nParam = NS_ooxml::LN_Value_ST_RestartNumber_eachSect; break;
        case RTF_FTNRSTCONT: nParam = NS_ooxml::LN_Value_ST_RestartNumber_continuous; break;
        default: break;
    }
    if (nParam >= 0)
    {
        RTFValue::Pointer_t pValue(new RTFValue(nParam));
        lcl_putNestedSprm(m_aDefaultState.aParagraphSprms, NS_ooxml::LN_EG_SectPrContents_footnotePr, NS_ooxml::LN_EG_FtnEdnNumProps_numRestart, pValue);
        return 0;
    }

    // Endnote numbering
    switch (nKeyword)
    {
        case RTF_AFTNNAR: nParam = NS_ooxml::LN_Value_ST_NumberFormat_decimal; break;
        case RTF_AFTNNALC: nParam = NS_ooxml::LN_Value_ST_NumberFormat_lowerLetter; break;
        case RTF_AFTNNAUC: nParam = NS_ooxml::LN_Value_ST_NumberFormat_upperLetter; break;
        case RTF_AFTNNRLC: nParam = NS_ooxml::LN_Value_ST_NumberFormat_lowerRoman; break;
        case RTF_AFTNNRUC: nParam = NS_ooxml::LN_Value_ST_NumberFormat_upperRoman; break;
        case RTF_AFTNNCHI: nParam = NS_ooxml::LN_Value_ST_NumberFormat_chicago; break;
        default: break;
    }
    if (nParam >= 0)
    {
        RTFValue::Pointer_t pValue(new RTFValue(nParam));
        lcl_putNestedSprm(m_aDefaultState.aParagraphSprms, NS_ooxml::LN_EG_SectPrContents_endnotePr, NS_ooxml::LN_CT_EdnProps_numFmt, pValue);
        return 0;
    }

    // Cell Text Flow
    switch (nKeyword)
    {
        case RTF_CLTXLRTB:  nParam = 0; break;
        case RTF_CLTXTBRL:  nParam = 1; break;
        case RTF_CLTXBTLR:  nParam = 3; break;
        case RTF_CLTXLRTBV: nParam = 4; break;
        case RTF_CLTXTBRLV: nParam = 5; break;
        default: break;
    }
    if (nParam >= 0)
    {
        RTFValue::Pointer_t pValue(new RTFValue(nParam));
        m_aStates.top().aTableCellSprms.set(NS_ooxml::LN_CT_TcPrBase_textDirection, pValue);
    }

    // Trivial paragraph flags
    switch (nKeyword)
    {
        case RTF_KEEP:
            if (m_aStates.top().pCurrentBuffer != &m_aTableBuffer)
                nParam = NS_sprm::LN_PFKeep;
            break;
        case RTF_KEEPN:
            if (m_aStates.top().pCurrentBuffer != &m_aTableBuffer)
                nParam = NS_sprm::LN_PFKeepFollow;
            break;
        case RTF_INTBL:
            {
                m_aStates.top().pCurrentBuffer = &m_aTableBuffer;
                nParam = NS_sprm::LN_PFInTable;
            }
            break;
        case RTF_PAGEBB:
            nParam = NS_sprm::LN_PFPageBreakBefore;
            break;
        default:
            break;
    }
    if (nParam >= 0)
    {
        RTFValue::Pointer_t pValue(new RTFValue(1));
        m_aStates.top().aParagraphSprms.erase(NS_sprm::LN_PFInTable);
        m_aStates.top().aParagraphSprms.set(nParam, pValue);
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
        case RTF_PLAIN:
            {
                m_aStates.top().aCharacterSprms = getDefaultState().aCharacterSprms;
                RTFValue::Pointer_t pValue = m_aStates.top().aCharacterSprms.find(NS_sprm::LN_CRgFtc0);
                if (pValue.get())
                    m_aStates.top().nCurrentEncoding = getEncoding(pValue->getInt());
                m_aStates.top().aCharacterAttributes = getDefaultState().aCharacterAttributes;
            }
            break;
        case RTF_PARD:
            if (m_bHadPicture)
                dispatchSymbol(RTF_PAR);
            // \pard is allowed between \cell and \row, but in that case it should not reset the fact that we're inside a table.
            if (m_aStates.top().nCells == 0)
            {
                // Reset everything.
                m_aStates.top().aParagraphSprms = m_aDefaultState.aParagraphSprms;
                m_aStates.top().aParagraphAttributes = m_aDefaultState.aParagraphAttributes;
                m_aStates.top().pCurrentBuffer = 0;
            }
            else
            {
                // Reset only margins.
                lcl_eraseNestedAttribute(m_aStates.top().aParagraphSprms, NS_ooxml::LN_CT_PPrBase_spacing, NS_ooxml::LN_CT_Spacing_before);
                lcl_eraseNestedAttribute(m_aStates.top().aParagraphSprms, NS_ooxml::LN_CT_PPrBase_spacing, NS_ooxml::LN_CT_Spacing_after);
                m_aStates.top().aParagraphSprms.erase(NS_sprm::LN_PDxaLeft);
                m_aStates.top().aParagraphSprms.erase(NS_sprm::LN_PDxaRight);
            }
            m_aStates.top().resetFrame();
            break;
        case RTF_SECTD:
            {
                m_aStates.top().aSectionSprms = m_aDefaultState.aSectionSprms;
                m_aStates.top().aSectionAttributes = m_aDefaultState.aSectionAttributes;
            }
            break;
        case RTF_TROWD:
            {
                m_aStates.top().aTableRowSprms = m_aDefaultState.aTableRowSprms;
                m_aStates.top().aTableRowSprms.set(NS_ooxml::LN_CT_TblGridBase_gridCol, RTFValue::Pointer_t(new RTFValue(-1)), false);
                m_aStates.top().aTableRowAttributes = m_aDefaultState.aTableRowAttributes;
                m_aStates.top().nCellX = 0;
                // In case the table definition is in the middle of the row
                // (invalid), make sure table definition is emitted.
                m_bNeedPap = true;
            }
            break;
        case RTF_WIDCTLPAR:
        case RTF_NOWIDCTLPAR:
            {
                RTFValue::Pointer_t pValue(new RTFValue(nKeyword == RTF_WIDCTLPAR));
                m_aStates.top().aParagraphSprms.set(NS_sprm::LN_PFWidowControl, pValue);
            }
            break;
        case RTF_BOX:
            {
                RTFSprms aAttributes;
                RTFValue::Pointer_t pValue(new RTFValue(aAttributes));
                for (int i = 0; i < 4; i++)
                    m_aStates.top().aParagraphSprms.set(lcl_getParagraphBorder(i), pValue);
                m_aStates.top().nBorderState = BORDER_PARAGRAPH_BOX;
            }
            break;
        case RTF_LTRSECT:
        case RTF_RTLSECT:
            {
                RTFValue::Pointer_t pValue(new RTFValue(nKeyword == RTF_LTRSECT ? 0 : 1));
                m_aStates.top().aParagraphSprms.set(NS_sprm::LN_STextFlow, pValue);
            }
            break;
        case RTF_LTRPAR:
        case RTF_RTLPAR:
            {
                RTFValue::Pointer_t pValue(new RTFValue(nKeyword == RTF_LTRPAR ? 0 : 1));
                m_aStates.top().aParagraphSprms.set(NS_sprm::LN_PFrameTextFlow, pValue);
            }
            break;
        case RTF_LTRROW:
        case RTF_RTLROW:
            {
                RTFValue::Pointer_t pValue(new RTFValue(nKeyword == RTF_LTRROW ? 0 : 1));
                m_aStates.top().aParagraphSprms.set(NS_sprm::LN_TTextFlow, pValue);
            }
            break;
        case RTF_LTRCH:
            // dmapper does not support this.
            break;
        case RTF_RTLCH:
            if (m_aDefaultState.nCurrentEncoding == RTL_TEXTENCODING_MS_1255)
                m_aStates.top().nCurrentEncoding = m_aDefaultState.nCurrentEncoding;
            break;
        case RTF_ULNONE:
            {
                RTFValue::Pointer_t pValue(new RTFValue(0));
                m_aStates.top().aCharacterSprms.set(NS_sprm::LN_CKul, pValue);
            }
            break;
        case RTF_NONSHPPICT:
        case RTF_MMATHPICT: // Picture group used by readers not understanding \moMath group
            m_aStates.top().nDestinationState = DESTINATION_SKIP;
            break;
        case RTF_CLBRDRT:
        case RTF_CLBRDRL:
        case RTF_CLBRDRB:
        case RTF_CLBRDRR:
            {
                RTFSprms aAttributes;
                RTFSprms aSprms;
                RTFValue::Pointer_t pValue(new RTFValue(aAttributes, aSprms));
                switch (nKeyword)
                {
                    case RTF_CLBRDRT: nParam = NS_ooxml::LN_CT_TcBorders_top; break;
                    case RTF_CLBRDRL: nParam = NS_ooxml::LN_CT_TcBorders_left; break;
                    case RTF_CLBRDRB: nParam = NS_ooxml::LN_CT_TcBorders_bottom; break;
                    case RTF_CLBRDRR: nParam = NS_ooxml::LN_CT_TcBorders_right; break;
                    default: break;
                }
                lcl_putNestedSprm(m_aStates.top().aTableCellSprms, NS_ooxml::LN_CT_TcPrBase_tcBorders, nParam, pValue);
                m_aStates.top().nBorderState = BORDER_CELL;
            }
            break;
        case RTF_PGBRDRT:
        case RTF_PGBRDRL:
        case RTF_PGBRDRB:
        case RTF_PGBRDRR:
            {
                RTFSprms aAttributes;
                RTFSprms aSprms;
                RTFValue::Pointer_t pValue(new RTFValue(aAttributes, aSprms));
                switch (nKeyword)
                {
                    case RTF_PGBRDRT: nParam = NS_ooxml::LN_CT_PageBorders_top; break;
                    case RTF_PGBRDRL: nParam = NS_ooxml::LN_CT_PageBorders_left; break;
                    case RTF_PGBRDRB: nParam = NS_ooxml::LN_CT_PageBorders_bottom; break;
                    case RTF_PGBRDRR: nParam = NS_ooxml::LN_CT_PageBorders_right; break;
                    default: break;
                }
                lcl_putNestedSprm(m_aStates.top().aSectionSprms, NS_ooxml::LN_EG_SectPrContents_pgBorders, nParam, pValue);
                m_aStates.top().nBorderState = BORDER_PAGE;
            }
            break;
        case RTF_BRDRT:
        case RTF_BRDRL:
        case RTF_BRDRB:
        case RTF_BRDRR:
            {
                RTFSprms aAttributes;
                RTFSprms aSprms;
                RTFValue::Pointer_t pValue(new RTFValue(aAttributes, aSprms));
                switch (nKeyword)
                {
                    case RTF_BRDRT: nParam = lcl_getParagraphBorder(0); break;
                    case RTF_BRDRL: nParam = lcl_getParagraphBorder(1); break;
                    case RTF_BRDRB: nParam = lcl_getParagraphBorder(2); break;
                    case RTF_BRDRR: nParam = lcl_getParagraphBorder(3); break;
                    default: break;
                }
                lcl_putNestedSprm(m_aStates.top().aParagraphSprms, NS_ooxml::LN_CT_PrBase_pBdr, nParam, pValue);
                m_aStates.top().nBorderState = BORDER_PARAGRAPH;
            }
            break;
        case RTF_CLVMGF:
            {
                RTFValue::Pointer_t pValue(new RTFValue(NS_ooxml::LN_Value_ST_Merge_restart));
                m_aStates.top().aTableCellSprms.set(NS_ooxml::LN_CT_TcPrBase_vMerge, pValue);
            }
            break;
        case RTF_CLVMRG:
            {
                RTFValue::Pointer_t pValue(new RTFValue(NS_ooxml::LN_Value_ST_Merge_continue));
                m_aStates.top().aTableCellSprms.set(NS_ooxml::LN_CT_TcPrBase_vMerge, pValue);
            }
            break;
        case RTF_CLVERTALT:
        case RTF_CLVERTALC:
        case RTF_CLVERTALB:
            {
                switch (nKeyword)
                {
                    case RTF_CLVERTALT: nParam = 0; break;
                    case RTF_CLVERTALC: nParam = 1; break;
                    case RTF_CLVERTALB: nParam = 3; break;
                    default: break;
                }
                RTFValue::Pointer_t pValue(new RTFValue(nParam));
                m_aStates.top().aTableCellSprms.set(NS_ooxml::LN_CT_TcPrBase_vAlign, pValue);
            }
            break;
        case RTF_TRKEEP:
            {
                RTFValue::Pointer_t pValue(new RTFValue(1));
                m_aStates.top().aTableRowSprms.set(NS_sprm::LN_TCantSplit, pValue);
            }
        case RTF_SECTUNLOCKED:
            {
                RTFValue::Pointer_t pValue(new RTFValue(!nParam));
                m_aStates.top().aSectionSprms.set(NS_ooxml::LN_EG_SectPrContents_formProt, pValue);
            }
        case RTF_PGNDEC:
        case RTF_PGNUCRM:
        case RTF_PGNLCRM:
        case RTF_PGNUCLTR:
        case RTF_PGNLCLTR:
        case RTF_PGNBIDIA:
        case RTF_PGNBIDIB:
            // These should be mapped to NS_ooxml::LN_EG_SectPrContents_pgNumType, but dmapper has no API for that at the moment.
            break;
        case RTF_LOCH:
            // Noop, dmapper detects this automatically.
            break;
        case RTF_HICH:
            m_aStates.top().bIsCjk = true;
            break;
        case RTF_DBCH:
            m_aStates.top().bIsCjk = false;
            break;
        case RTF_TITLEPG:
            {
                RTFValue::Pointer_t pValue(new RTFValue(1));
                m_aStates.top().aSectionSprms.set(NS_ooxml::LN_EG_SectPrContents_titlePg, pValue);
            }
            break;
        case RTF_SUPER:
            {
                if (!m_aStates.top().pCurrentBuffer)
                    m_aStates.top().pCurrentBuffer = &m_aSuperBuffer;

                RTFValue::Pointer_t pValue(new RTFValue("superscript"));
                m_aStates.top().aCharacterSprms.set(NS_ooxml::LN_EG_RPrBase_vertAlign, pValue);
            }
            break;
        case RTF_SUB:
            {
                RTFValue::Pointer_t pValue(new RTFValue("subscript"));
                m_aStates.top().aCharacterSprms.set(NS_ooxml::LN_EG_RPrBase_vertAlign, pValue);
            }
            break;
        case RTF_NOSUPERSUB:
            {
                if (m_aStates.top().pCurrentBuffer == &m_aSuperBuffer)
                {
                    replayBuffer(m_aSuperBuffer);
                    m_aStates.top().pCurrentBuffer = 0;
                }
                m_aStates.top().aCharacterSprms.erase(NS_ooxml::LN_EG_RPrBase_vertAlign);
            }
            break;
        case RTF_LINEPPAGE:
        case RTF_LINECONT:
            {
                RTFValue::Pointer_t pValue(new RTFValue(nKeyword == RTF_LINEPPAGE ? 0 : 2));
                lcl_putNestedAttribute(m_aStates.top().aSectionSprms,
                        NS_ooxml::LN_EG_SectPrContents_lnNumType, NS_ooxml::LN_CT_LineNumber_restart, pValue);
            }
            break;
        case RTF_AENDDOC:
            // Noop, this is the default in Writer.
            break;
        case RTF_AENDNOTES:
            // Noop, Writer does not support having endnotes at the end of section.
            break;
        case RTF_AFTNRSTCONT:
            // Noop, this is the default in Writer.
            break;
        case RTF_AFTNRESTART:
            // Noop, Writer does not support restarting endnotes at each section.
            break;
        case RTF_FTNBJ:
            // Noop, this is the default in Writer.
            break;
        case RTF_ENDDOC:
            {
                RTFValue::Pointer_t pValue(new RTFValue(NS_ooxml::LN_Value_ST_RestartNumber_eachSect));
                lcl_putNestedSprm(m_aDefaultState.aParagraphSprms,
                        NS_ooxml::LN_EG_SectPrContents_footnotePr,
                        NS_ooxml::LN_EG_FtnEdnNumProps_numRestart, pValue);
            }
            break;
        case RTF_NOLINE:
            lcl_eraseNestedAttribute(m_aStates.top().aSectionSprms, NS_ooxml::LN_EG_SectPrContents_lnNumType, NS_ooxml::LN_CT_LineNumber_distance);
            break;
        case RTF_FORMSHADE:
            // Noop, this is the default in Writer.
            break;
        case RTF_PNGBLIP:
            m_aStates.top().aPicture.nStyle = BMPSTYLE_PNG;
            break;
        case RTF_JPEGBLIP:
            m_aStates.top().aPicture.nStyle = BMPSTYLE_JPEG;
            break;
        case RTF_POSYT: m_aStates.top().aFrame.setSprm(NS_ooxml::LN_CT_FramePr_yAlign, NS_ooxml::LN_Value_wordprocessingml_ST_YAlign_top); break;
        case RTF_POSYB: m_aStates.top().aFrame.setSprm(NS_ooxml::LN_CT_FramePr_yAlign, NS_ooxml::LN_Value_wordprocessingml_ST_YAlign_bottom); break;
        case RTF_POSYC: m_aStates.top().aFrame.setSprm(NS_ooxml::LN_CT_FramePr_yAlign, NS_ooxml::LN_Value_wordprocessingml_ST_YAlign_center); break;
        case RTF_POSYIN: m_aStates.top().aFrame.setSprm(NS_ooxml::LN_CT_FramePr_yAlign, NS_ooxml::LN_Value_wordprocessingml_ST_YAlign_inside); break;
        case RTF_POSYOUT: m_aStates.top().aFrame.setSprm(NS_ooxml::LN_CT_FramePr_yAlign, NS_ooxml::LN_Value_wordprocessingml_ST_YAlign_outside); break;
        case RTF_POSYIL: m_aStates.top().aFrame.setSprm(NS_ooxml::LN_CT_FramePr_yAlign, NS_ooxml::LN_Value_wordprocessingml_ST_YAlign_inline); break;

        case RTF_PHMRG: m_aStates.top().aFrame.setSprm(NS_ooxml::LN_CT_FramePr_hAnchor, NS_ooxml::LN_Value_wordprocessingml_ST_HAnchor_margin); break;
        case RTF_PVMRG: m_aStates.top().aFrame.setSprm(NS_ooxml::LN_CT_FramePr_vAnchor, NS_ooxml::LN_Value_wordprocessingml_ST_VAnchor_margin); break;
        case RTF_PHPG: m_aStates.top().aFrame.setSprm(NS_ooxml::LN_CT_FramePr_hAnchor, NS_ooxml::LN_Value_wordprocessingml_ST_HAnchor_page); break;
        case RTF_PVPG: m_aStates.top().aFrame.setSprm(NS_ooxml::LN_CT_FramePr_vAnchor, NS_ooxml::LN_Value_wordprocessingml_ST_VAnchor_page); break;
        case RTF_PHCOL: m_aStates.top().aFrame.setSprm(NS_ooxml::LN_CT_FramePr_hAnchor, NS_ooxml::LN_Value_wordprocessingml_ST_HAnchor_text); break;
        case RTF_PVPARA: m_aStates.top().aFrame.setSprm(NS_ooxml::LN_CT_FramePr_vAnchor, NS_ooxml::LN_Value_wordprocessingml_ST_VAnchor_text); break;

        case RTF_POSXC: m_aStates.top().aFrame.setSprm(NS_ooxml::LN_CT_FramePr_xAlign, NS_ooxml::LN_Value_wordprocessingml_ST_XAlign_center); break;
        case RTF_POSXI: m_aStates.top().aFrame.setSprm(NS_ooxml::LN_CT_FramePr_xAlign, NS_ooxml::LN_Value_wordprocessingml_ST_XAlign_inside); break;
        case RTF_POSXO: m_aStates.top().aFrame.setSprm(NS_ooxml::LN_CT_FramePr_xAlign, NS_ooxml::LN_Value_wordprocessingml_ST_XAlign_outside); break;
        case RTF_POSXL: m_aStates.top().aFrame.setSprm(NS_ooxml::LN_CT_FramePr_xAlign, NS_ooxml::LN_Value_wordprocessingml_ST_XAlign_left); break;
        case RTF_POSXR: m_aStates.top().aFrame.setSprm(NS_ooxml::LN_CT_FramePr_xAlign, NS_ooxml::LN_Value_wordprocessingml_ST_XAlign_right); break;

        case RTF_DPLINE:
        case RTF_DPRECT:
        case RTF_DPELLIPSE:
        case RTF_DPTXBX:
        case RTF_DPPOLYLINE:
                {
                    sal_Int32 nType = 0;
                    switch (nKeyword)
                    {
                        case RTF_DPLINE:
                            m_aStates.top().aDrawingObject.xShape.set(getModelFactory()->createInstance("com.sun.star.drawing.LineShape"), uno::UNO_QUERY);
                            break;
                        case RTF_DPPOLYLINE:
                            // The reason this is not a simple CustomShape is that in the old syntax we have no ViewBox info.
                            m_aStates.top().aDrawingObject.xShape.set(getModelFactory()->createInstance("com.sun.star.drawing.PolyLineShape"), uno::UNO_QUERY);
                            break;
                        case RTF_DPRECT:
                            m_aStates.top().aDrawingObject.xShape.set(getModelFactory()->createInstance("com.sun.star.drawing.RectangleShape"), uno::UNO_QUERY);
                            break;
                        case RTF_DPELLIPSE:
                            nType = ESCHER_ShpInst_Ellipse;
                            break;
                        case RTF_DPTXBX:
                            {
                                m_aStates.top().aDrawingObject.xShape.set(getModelFactory()->createInstance("com.sun.star.text.TextFrame"), uno::UNO_QUERY);
                                std::vector<beans::PropertyValue> aDefaults = m_pSdrImport->getTextFrameDefaults(false);
                                for (size_t i = 0; i < aDefaults.size(); ++i)
                                    m_aStates.top().aDrawingObject.aPendingProperties.push_back(aDefaults[i]);
                                checkFirstRun();
                                Mapper().startShape(m_aStates.top().aDrawingObject.xShape);
                                m_aStates.top().aDrawingObject.bHadShapeText = true;
                            }
                            break;
                        default:
                            break;
                    }
                    if (nType)
                            m_aStates.top().aDrawingObject.xShape.set(getModelFactory()->createInstance("com.sun.star.drawing.CustomShape"), uno::UNO_QUERY);
                    uno::Reference<drawing::XDrawPageSupplier> xDrawSupplier( m_xDstDoc, uno::UNO_QUERY);
                    if (xDrawSupplier.is())
                    {
                        uno::Reference<drawing::XShapes> xShapes(xDrawSupplier->getDrawPage(), uno::UNO_QUERY);
                        if (xShapes.is() && nKeyword != RTF_DPTXBX)
                            xShapes->add(m_aStates.top().aDrawingObject.xShape);
                    }
                    if (nType)
                    {
                            uno::Reference<drawing::XEnhancedCustomShapeDefaulter> xDefaulter(m_aStates.top().aDrawingObject.xShape, uno::UNO_QUERY);
                            xDefaulter->createCustomShapeDefaults(OUString::valueOf(nType));
                    }
                    m_aStates.top().aDrawingObject.xPropertySet.set(m_aStates.top().aDrawingObject.xShape, uno::UNO_QUERY);
                    std::vector<beans::PropertyValue>& rPendingProperties = m_aStates.top().aDrawingObject.aPendingProperties;
                    for (std::vector<beans::PropertyValue>::iterator i = rPendingProperties.begin(); i != rPendingProperties.end(); ++i)
                        m_aStates.top().aDrawingObject.xPropertySet->setPropertyValue(i->Name, i->Value);
                    m_pSdrImport->resolveDhgt(m_aStates.top().aDrawingObject.xPropertySet, m_aStates.top().aDrawingObject.nDhgt);
                }
                break;
        case RTF_DOBXMARGIN:
        case RTF_DOBYMARGIN:
                {
                    beans::PropertyValue aPropertyValue;
                    aPropertyValue.Name = (nKeyword == RTF_DOBXMARGIN ? OUString("HoriOrientRelation") : OUString("VertOrientRelation"));
                    aPropertyValue.Value <<= text::RelOrientation::PAGE_PRINT_AREA;
                    m_aStates.top().aDrawingObject.aPendingProperties.push_back(aPropertyValue);
                }
                break;
        case RTF_DOBXPAGE:
        case RTF_DOBYPAGE:
                {
                    beans::PropertyValue aPropertyValue;
                    aPropertyValue.Name = (nKeyword == RTF_DOBXPAGE ? OUString("HoriOrientRelation") : OUString("VertOrientRelation"));
                    aPropertyValue.Value <<= text::RelOrientation::PAGE_FRAME;
                    m_aStates.top().aDrawingObject.aPendingProperties.push_back(aPropertyValue);
                }
                break;
        case RTF_DOBYPARA:
                {
                    beans::PropertyValue aPropertyValue;
                    aPropertyValue.Name = OUString("VertOrientRelation");
                    aPropertyValue.Value <<= text::RelOrientation::FRAME;
                    m_aStates.top().aDrawingObject.aPendingProperties.push_back(aPropertyValue);
                }
                break;
        case RTF_CONTEXTUALSPACE:
                {
                    RTFValue::Pointer_t pValue(new RTFValue(1));
                    m_aStates.top().aParagraphSprms.set(NS_sprm::LN_PContextualSpacing, pValue);
                }
                break;
        case RTF_LINKSTYLES:
                {
                    RTFValue::Pointer_t pValue(new RTFValue(1));
                    m_aSettingsTableSprms.set(NS_ooxml::LN_CT_Settings_linkStyles, pValue);
                }
                break;
        case RTF_PNLVLBODY:
                {
                    RTFValue::Pointer_t pValue(new RTFValue(2));
                    m_aStates.top().aTableAttributes.set(NS_rtf::LN_LSID, pValue);
                }
                break;
        case RTF_PNDEC:
                {
                    RTFValue::Pointer_t pValue(new RTFValue(0)); // decimal, same as \levelnfc0
                    m_aStates.top().aTableSprms.set(NS_rtf::LN_NFC, pValue);
                }
                break;
        case RTF_PNLVLBLT:
                {
                    m_aStates.top().aTableAttributes.set(NS_rtf::LN_LSID, RTFValue::Pointer_t(new RTFValue(1)));
                    m_aStates.top().aTableSprms.set(NS_rtf::LN_NFC, RTFValue::Pointer_t(new RTFValue(23))); // bullets, same as \levelnfc23
                }
                break;
        case RTF_LANDSCAPE:
                lcl_putNestedAttribute(m_aStates.top().aSectionSprms, NS_ooxml::LN_EG_SectPrContents_pgSz, NS_ooxml::LN_CT_PageSz_orient, RTFValue::Pointer_t(new RTFValue(1)));
                break;
        case RTF_FACINGP:
                m_aSettingsTableSprms.set(NS_ooxml::LN_CT_Settings_evenAndOddHeaders, RTFValue::Pointer_t(new RTFValue(1)));
                break;
        case RTF_SHPBXPAGE:
                m_aStates.top().aShape.nHoriOrientRelation = text::RelOrientation::PAGE_FRAME;
                break;
        case RTF_SHPBYPAGE:
                m_aStates.top().aShape.nVertOrientRelation = text::RelOrientation::PAGE_FRAME;
                break;
        case RTF_DPLINEHOLLOW:
                m_aStates.top().aDrawingObject.nFLine = 0;
                break;
        case RTF_DPROUNDR:
                if (m_aStates.top().aDrawingObject.xPropertySet.is())
                    // Seems this old syntax has no way to specify a custom radius, and this is the default
                    m_aStates.top().aDrawingObject.xPropertySet->setPropertyValue("CornerRadius", uno::makeAny(sal_Int32(83)));
                break;
        case RTF_NOWRAP:
                m_aStates.top().aFrame.setSprm(NS_sprm::LN_PWr, NS_ooxml::LN_Value_wordprocessingml_ST_Wrap_notBeside);
                break;
        case RTF_MNOR:
                m_bMathNor = true;
                break;
        case RTF_REVISIONS:
                m_aSettingsTableSprms.set(NS_ooxml::LN_CT_Settings_trackRevisions, RTFValue::Pointer_t(new RTFValue(1)));
                break;
        default:
                {
                    SAL_INFO("writerfilter", "TODO handle flag '" << lcl_RtfToString(nKeyword) << "'");
                    aSkip.setParsed(false);
                }
                break;
    }
    return 0;
}

int RTFDocumentImpl::dispatchValue(RTFKeyword nKeyword, int nParam)
{
    checkUnicode(/*bUnicode =*/ nKeyword != RTF_U, /*bHex =*/ true);
    setNeedSect();
    RTFSkipDestination aSkip(*this);
    int nSprm = 0;
    RTFValue::Pointer_t pIntValue(new RTFValue(nParam));
    // Trivial table sprms.
    switch (nKeyword)
    {
        case RTF_FPRQ: nSprm = NS_rtf::LN_PRQ; break;
        case RTF_LEVELJC: nSprm = NS_ooxml::LN_CT_Lvl_lvlJc; break;
        case RTF_LEVELNFC: nSprm = NS_rtf::LN_NFC; break;
        case RTF_LEVELSTARTAT: nSprm = NS_rtf::LN_ISTARTAT; break;
        case RTF_LEVELPICTURE: nSprm = NS_ooxml::LN_CT_Lvl_lvlPicBulletId; break;
        default: break;
    }
    if (nSprm > 0)
    {
        m_aStates.top().aTableSprms.set(nSprm, pIntValue);
        return 0;
    }
    // Trivial character sprms.
    switch (nKeyword)
    {
        case RTF_FS: nSprm = NS_sprm::LN_CHps; break;
        case RTF_AFS: nSprm = NS_sprm::LN_CHpsBi; break;
        case RTF_ANIMTEXT: nSprm = NS_sprm::LN_CSfxText; break;
        case RTF_EXPNDTW: nSprm = NS_sprm::LN_CDxaSpace; break;
        case RTF_KERNING: nSprm = NS_sprm::LN_CHpsKern; break;
        case RTF_CHARSCALEX: nSprm = NS_sprm::LN_CCharScale; break;
        case RTF_LANG: nSprm = NS_sprm::LN_CRgLid0; break;
        case RTF_LANGFE: nSprm = NS_sprm::LN_CRgLid1; break;
        case RTF_ALANG: nSprm = NS_sprm::LN_CLidBi; break;
        default: break;
    }
    if (nSprm > 0)
    {
        m_aStates.top().aCharacterSprms.set(nSprm, pIntValue);
        // Language is a character property, but we should store it at a paragraph level as well for fields.
        if (nKeyword == RTF_LANG && m_bNeedPap)
            m_aStates.top().aParagraphSprms.set(nSprm, pIntValue);
        return 0;
    }
    // Trivial paragraph sprms.
    switch (nKeyword)
    {
        case RTF_FI: nSprm = NS_sprm::LN_PDxaLeft1; break;
        case RTF_LIN: nSprm = 0x845e; break;
        case RTF_RI: nSprm = NS_sprm::LN_PDxaRight; break;
        case RTF_RIN: nSprm = 0x845d; break;
        case RTF_ITAP: nSprm = NS_sprm::LN_PTableDepth; break;
        default: break;
    }
    if (nSprm > 0)
    {
        m_aStates.top().aParagraphSprms.set(nSprm, pIntValue);
        if (nKeyword == RTF_ITAP && nParam > 0)
            // Invalid tables may omit INTBL after ITAP
            dispatchFlag(RTF_INTBL);
        return 0;
    }

    // Trivial table attributes.
    switch (nKeyword)
    {
        case RTF_SBASEDON: nSprm = NS_rtf::LN_ISTDBASE; break;
        case RTF_SNEXT: nSprm = NS_rtf::LN_ISTDNEXT; break;
        default: break;
    }
    if (nSprm > 0)
    {
        m_aStates.top().aTableAttributes.set(nSprm, pIntValue);
        return 0;
    }

    // Info group.
    switch (nKeyword)
    {
        case RTF_YR:
            {
                m_aStates.top().nYear = nParam;
                nSprm = 1;
            }
            break;
        case RTF_MO:
            {
                m_aStates.top().nMonth = nParam;
                nSprm = 1;
            }
            break;
        case RTF_DY:
            {
                m_aStates.top().nDay = nParam;
                nSprm = 1;
            }
            break;
        case RTF_HR:
            {
                m_aStates.top().nHour = nParam;
                nSprm = 1;
            }
            break;
        case RTF_MIN:
            {
                m_aStates.top().nMinute = nParam;
                nSprm = 1;
            }
            break;
        default:
            break;
    }
    if (nSprm > 0)
        return 0;

    // Frame size / position.
    Id nId = 0;
    switch (nKeyword)
    {
        case RTF_ABSW:
            nId = NS_sprm::LN_PDxaWidth;
            break;
        case RTF_ABSH:
            nId = NS_sprm::LN_PWHeightAbs;
            break;
        case RTF_POSX:
            {
                nId = NS_ooxml::LN_CT_FramePr_x;
                m_aStates.top().aFrame.setSprm(NS_ooxml::LN_CT_FramePr_xAlign, 0);
            }
            break;
        case RTF_POSY:
            {
                nId = NS_ooxml::LN_CT_FramePr_y;
                m_aStates.top().aFrame.setSprm(NS_ooxml::LN_CT_FramePr_yAlign, 0);
            }
            break;
        default:
            break;
    }

    if (nId > 0)
    {
        m_bNeedPap = true;
        // Don't try to support text frames inside tables for now.
        if (m_aStates.top().pCurrentBuffer != &m_aTableBuffer)
            m_aStates.top().aFrame.setSprm(nId, nParam);

        return 0;
    }

    // Then check for the more complex ones.
    switch (nKeyword)
    {
        case RTF_F:
        case RTF_AF:
            if (nKeyword == RTF_F)
                nSprm = NS_sprm::LN_CRgFtc0;
            else
                nSprm = (m_aStates.top().bIsCjk ? NS_sprm::LN_CRgFtc1 : NS_sprm::LN_CRgFtc2);
            if (m_aStates.top().nDestinationState == DESTINATION_FONTTABLE || m_aStates.top().nDestinationState == DESTINATION_FONTENTRY)
            {
                m_aFontIndexes.push_back(nParam);
                m_nCurrentFontIndex = getFontIndex(nParam);
            }
            else if (m_aStates.top().nDestinationState == DESTINATION_LISTLEVEL)
            {
                RTFSprms aFontSprms;
                aFontSprms.set(nSprm, RTFValue::Pointer_t(new RTFValue(getFontIndex(nParam))));
                // In the context of listlevels, \af seems to imply \f.
                if (nKeyword == RTF_AF)
                    aFontSprms.set(NS_sprm::LN_CRgFtc0, RTFValue::Pointer_t(new RTFValue(getFontIndex(nParam))));
                RTFSprms aRunPropsSprms;
                aRunPropsSprms.set(NS_ooxml::LN_EG_RPrBase_rFonts, RTFValue::Pointer_t(new RTFValue(RTFSprms(), aFontSprms)));
                // If there are multiple \f or \af tokens, only handle the first one.
                if (!m_aStates.top().aTableSprms.find(NS_ooxml::LN_CT_Lvl_rPr))
                    m_aStates.top().aTableSprms.set(NS_ooxml::LN_CT_Lvl_rPr, RTFValue::Pointer_t(new RTFValue(RTFSprms(), aRunPropsSprms)));
            }
            else
            {
                int nFontIndex = getFontIndex(nParam);
                RTFValue::Pointer_t pValue(new RTFValue(nFontIndex));
                m_aStates.top().aCharacterSprms.set(nSprm, pValue);
                m_aStates.top().nCurrentEncoding = getEncoding(nFontIndex);
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

                m_aFontEncodings[m_nCurrentFontIndex] = rtl_getTextEncodingFromWindowsCodePage(aRTFEncodings[i].codepage);
            }
            break;
        case RTF_ANSICPG:
            {
                m_aDefaultState.nCurrentEncoding = rtl_getTextEncodingFromWindowsCodePage(nParam);
                m_aStates.top().nCurrentEncoding = rtl_getTextEncodingFromWindowsCodePage(nParam);
            }
            break;
        case RTF_CPG:
            m_aFontEncodings[m_nCurrentFontIndex] = rtl_getTextEncodingFromWindowsCodePage(nParam);
            break;
        case RTF_CF:
            {
                // NS_sprm::LN_CIco won't work, that would be an index in a static table
                RTFValue::Pointer_t pValue(new RTFValue(getColorTable(nParam)));
                m_aStates.top().aCharacterAttributes.set(NS_ooxml::LN_CT_Color_val, pValue);
            }
            break;
        case RTF_S:
            {
                m_aStates.top().nCurrentStyleIndex = nParam;

                if (m_aStates.top().nDestinationState == DESTINATION_STYLESHEET || m_aStates.top().nDestinationState == DESTINATION_STYLEENTRY)
                {
                    m_nCurrentStyleIndex = nParam;
                    m_aStates.top().aTableAttributes.set(NS_rtf::LN_ISTD, pIntValue);
                    RTFValue::Pointer_t pValue(new RTFValue(1));
                    m_aStates.top().aTableAttributes.set(NS_rtf::LN_SGC, pValue); // paragraph style
                }
                else
                   m_aStates.top().aParagraphAttributes.set(NS_rtf::LN_ISTD, pIntValue);
            }
            break;
        case RTF_CS:
            if (m_aStates.top().nDestinationState == DESTINATION_STYLESHEET || m_aStates.top().nDestinationState == DESTINATION_STYLEENTRY)
            {
                m_nCurrentStyleIndex = nParam;
                m_aStates.top().aTableAttributes.set(NS_rtf::LN_ISTD, pIntValue);
                RTFValue::Pointer_t pValue(new RTFValue(2));
                m_aStates.top().aTableAttributes.set(NS_rtf::LN_SGC, pValue); // character style
            }
            else
                m_aStates.top().aCharacterAttributes.set(NS_rtf::LN_ISTD, pIntValue);
            break;
        case RTF_DEFF:
            m_aDefaultState.aCharacterSprms.set(NS_sprm::LN_CRgFtc0, pIntValue);
            break;
        case RTF_DEFLANG:
            m_aDefaultState.aCharacterSprms.set(NS_sprm::LN_CRgLid0, pIntValue);
            break;
        case RTF_ADEFLANG:
            m_aDefaultState.aCharacterSprms.set(NS_sprm::LN_CLidBi, pIntValue);
            break;
        case RTF_CHCBPAT:
            {
                RTFValue::Pointer_t pValue(new RTFValue(nParam ? getColorTable(nParam) : COL_AUTO));
                lcl_putNestedAttribute(m_aStates.top().aCharacterSprms, NS_sprm::LN_CShd, NS_ooxml::LN_CT_Shd_fill, pValue);
            }
            break;
        case RTF_CLCBPAT:
            {
                RTFValue::Pointer_t pValue(new RTFValue(getColorTable(nParam)));
                lcl_putNestedAttribute(m_aStates.top().aTableCellSprms,
                        NS_ooxml::LN_CT_TcPrBase_shd, NS_ooxml::LN_CT_Shd_fill, pValue);
            }
            break;
        case RTF_CBPAT:
            if (nParam)
            {
                RTFValue::Pointer_t pValue(new RTFValue(getColorTable(nParam)));
                lcl_putNestedAttribute(m_aStates.top().aParagraphSprms, NS_sprm::LN_PShd, NS_ooxml::LN_CT_Shd_fill, pValue);
            }
            break;
        case RTF_ULC:
            {
                RTFValue::Pointer_t pValue(new RTFValue(getColorTable(nParam)));
                m_aStates.top().aCharacterSprms.set(0x6877, pValue);
            }
            break;
        case RTF_HIGHLIGHT:
            {
                RTFValue::Pointer_t pValue(new RTFValue(getColorTable(nParam)));
                m_aStates.top().aCharacterSprms.set(NS_sprm::LN_CHighlight, pValue);
            }
            break;
        case RTF_UP:
        case RTF_DN:
            {
                RTFValue::Pointer_t pValue(new RTFValue(nParam * (nKeyword == RTF_UP ? 1 : -1)));
                m_aStates.top().aCharacterSprms.set(NS_sprm::LN_CHpsPos, pValue);
            }
            break;
        case RTF_HORZVERT:
            {
                RTFValue::Pointer_t pValue(new RTFValue(true));
                m_aStates.top().aCharacterAttributes.set(NS_ooxml::LN_CT_EastAsianLayout_vert, pValue);
                if (nParam)
                    // rotate fits to a single line
                    m_aStates.top().aCharacterAttributes.set(NS_ooxml::LN_CT_EastAsianLayout_vertCompress, pValue);
            }
            break;
        case RTF_EXPND:
            {
                RTFValue::Pointer_t pValue(new RTFValue(nParam/5));
                m_aStates.top().aCharacterSprms.set(NS_sprm::LN_CDxaSpace, pValue);
            }
            break;
        case RTF_TWOINONE:
            {
                RTFValue::Pointer_t pValue(new RTFValue(true));
                m_aStates.top().aCharacterAttributes.set(NS_ooxml::LN_CT_EastAsianLayout_combine, pValue);
                if (nParam > 0)
                    m_aStates.top().aCharacterAttributes.set(NS_ooxml::LN_CT_EastAsianLayout_combineBrackets, pIntValue);
            }
            break;
        case RTF_SL:
            {
                // This is similar to RTF_ABSH, negative value means 'exact', positive means 'at least'.
                RTFValue::Pointer_t pValue(new RTFValue(NS_ooxml::LN_Value_wordprocessingml_ST_LineSpacingRule_atLeast));
                if (nParam < 0)
                {
                    pValue.reset(new RTFValue(NS_ooxml::LN_Value_wordprocessingml_ST_LineSpacingRule_exact));
                    pIntValue.reset(new RTFValue(-nParam));
                }
                m_aStates.top().aParagraphAttributes.set(NS_ooxml::LN_CT_Spacing_lineRule, pValue);
                m_aStates.top().aParagraphAttributes.set(NS_ooxml::LN_CT_Spacing_line, pIntValue);
            }
            break;
        case RTF_SLMULT:
            if (nParam > 0)
            {
                RTFValue::Pointer_t pValue(new RTFValue(NS_ooxml::LN_Value_wordprocessingml_ST_LineSpacingRule_auto));
                m_aStates.top().aParagraphAttributes.set(NS_ooxml::LN_CT_Spacing_lineRule, pValue);
            }
            break;
        case RTF_BRDRW:
            {
                // dmapper expects it in 1/8 pt, we have it in twip - but avoid rounding 1 to 0
                if (nParam > 1)
                    nParam = nParam * 2 / 5;
                RTFValue::Pointer_t pValue(new RTFValue(nParam));
                lcl_putBorderProperty(m_aStates, NS_rtf::LN_DPTLINEWIDTH, pValue);
            }
            break;
        case RTF_BRDRCF:
            {
                RTFValue::Pointer_t pValue(new RTFValue(getColorTable(nParam)));
                lcl_putBorderProperty(m_aStates, NS_ooxml::LN_CT_Border_color, pValue);
            }
            break;
        case RTF_BRSP:
            {
                // dmapper expects it in points, we have it in twip
                RTFValue::Pointer_t pValue(new RTFValue(nParam / 20));
                lcl_putBorderProperty(m_aStates, NS_rtf::LN_DPTSPACE, pValue);
            }
            break;
        case RTF_TX:
            {
                m_aStates.top().aTabAttributes.set(NS_ooxml::LN_CT_TabStop_pos, pIntValue);
                RTFValue::Pointer_t pValue(new RTFValue(m_aStates.top().aTabAttributes));
                lcl_putNestedSprm(m_aStates.top().aParagraphSprms, NS_ooxml::LN_CT_PPrBase_tabs, NS_ooxml::LN_CT_Tabs_tab, pValue);
                m_aStates.top().aTabAttributes.clear();
            }
            break;
        case RTF_ILVL:
            lcl_putNestedSprm(m_aStates.top().aParagraphSprms, NS_ooxml::LN_CT_PPrBase_numPr, NS_sprm::LN_PIlvl, pIntValue);
            break;
        case RTF_LISTTEMPLATEID:
            // This one is not referenced anywhere, so it's pointless to store it at the moment.
            break;
        case RTF_LISTID:
            {
                if (m_aStates.top().nDestinationState == DESTINATION_LISTENTRY)
                    m_aStates.top().aTableAttributes.set(NS_ooxml::LN_CT_AbstractNum_abstractNumId, pIntValue);
                else if (m_aStates.top().nDestinationState == DESTINATION_LISTOVERRIDEENTRY)
                    m_aStates.top().aTableSprms.set(NS_ooxml::LN_CT_Num_abstractNumId, pIntValue);
            }
            break;
        case RTF_LS:
            {
                if (m_aStates.top().nDestinationState == DESTINATION_LISTOVERRIDEENTRY)
                    m_aStates.top().aTableAttributes.set(NS_rtf::LN_LSID, pIntValue);
                else
                    lcl_putNestedSprm(m_aStates.top().aParagraphSprms, NS_ooxml::LN_CT_PPrBase_tabs, NS_sprm::LN_PIlfo, pIntValue);
            }
            break;
        case RTF_UC:
            if ((SAL_MIN_INT16 <= nParam) && (nParam <= SAL_MAX_INT16))
                m_aStates.top().nUc = nParam;
            break;
        case RTF_U:
            // sal_Unicode is unsigned 16-bit, RTF may represent that as a
            // signed SAL_MIN_INT16..SAL_MAX_INT16 or 0..SAL_MAX_UINT16. The
            // static_cast() will do the right thing.
            if ((SAL_MIN_INT16 <= nParam) && (nParam <= SAL_MAX_UINT16))
            {
                m_aUnicodeBuffer.append(static_cast<sal_Unicode>(nParam));
                if (m_aStates.top().nDestinationState != DESTINATION_LEVELTEXT)
                    m_aStates.top().nCharsToSkip = m_aStates.top().nUc;
            }
            break;
        case RTF_LEVELFOLLOW:
            m_aStates.top().aTableAttributes.set(NS_rtf::LN_IXCHFOLLOW, pIntValue);
            break;
        case RTF_LISTOVERRIDECOUNT:
            // Ignore this for now, the exporter always emits it with a zero parameter.
            break;
        case RTF_PICSCALEX:
            m_aStates.top().aPicture.nScaleX = nParam;
            break;
        case RTF_PICSCALEY:
            m_aStates.top().aPicture.nScaleY = nParam;
            break;
        case RTF_PICW:
            m_aStates.top().aPicture.nWidth = nParam;
            break;
        case RTF_PICH:
            m_aStates.top().aPicture.nHeight = nParam;
            break;
        case RTF_PICWGOAL:
            m_aStates.top().aPicture.nGoalWidth = TWIP_TO_MM100(nParam);
            break;
        case RTF_PICHGOAL:
            m_aStates.top().aPicture.nGoalHeight = TWIP_TO_MM100(nParam);
            break;
        case RTF_PICCROPL: m_aStates.top().aPicture.nCropL = TWIP_TO_MM100(nParam); break;
        case RTF_PICCROPR: m_aStates.top().aPicture.nCropR = TWIP_TO_MM100(nParam); break;
        case RTF_PICCROPT: m_aStates.top().aPicture.nCropT = TWIP_TO_MM100(nParam); break;
        case RTF_PICCROPB: m_aStates.top().aPicture.nCropB = TWIP_TO_MM100(nParam); break;
        case RTF_SHPWRK:
            {
                int nValue = 0;
                switch (nParam)
                {
                    case 0: nValue = NS_ooxml::LN_Value_wordprocessingDrawing_ST_WrapText_bothSides; break;
                    case 1: nValue = NS_ooxml::LN_Value_wordprocessingDrawing_ST_WrapText_left; break;
                    case 2: nValue = NS_ooxml::LN_Value_wordprocessingDrawing_ST_WrapText_right; break;
                    case 3: nValue = NS_ooxml::LN_Value_wordprocessingDrawing_ST_WrapText_largest; break;
                    default: break;
                }
                RTFValue::Pointer_t pValue(new RTFValue(nValue));
                m_aStates.top().aCharacterAttributes.set(NS_ooxml::LN_CT_WrapSquare_wrapText, pValue);
            }
            break;
        case RTF_SHPWR:
            {
                switch (nParam)
                {
                case 1:
                    m_aStates.top().aShape.nWrap = com::sun::star::text::WrapTextMode_NONE; break;
                case 2:
                    m_aStates.top().aShape.nWrap = com::sun::star::text::WrapTextMode_PARALLEL; break;
                case 3:
                    m_aStates.top().aShape.nWrap = com::sun::star::text::WrapTextMode_THROUGHT; break;
                case 4:
                    m_aStates.top().aShape.nWrap = com::sun::star::text::WrapTextMode_PARALLEL; break;
                case 5:
                    m_aStates.top().aShape.nWrap = com::sun::star::text::WrapTextMode_THROUGHT; break;
                }
            }
            break;
        case RTF_CELLX:
            {
                int nCellX = nParam - m_aStates.top().nCellX;

                // If there is a negative left margin, then the first cellx is relateve to that.
                RTFValue::Pointer_t pTblInd = m_aStates.top().aTableRowSprms.find(NS_ooxml::LN_CT_TblPrBase_tblInd);
                if (m_aStates.top().nCellX == 0 && pTblInd.get())
                {
                    RTFValue::Pointer_t pWidth = pTblInd->getAttributes().find(NS_ooxml::LN_CT_TblWidth_w);
                    if (pWidth.get() && pWidth->getInt() < 0)
                        nCellX = -1 * (pWidth->getInt() - nParam);
                }

                m_aStates.top().nCellX = nParam;
                RTFValue::Pointer_t pXValue(new RTFValue(nCellX));
                m_aStates.top().aTableRowSprms.set(NS_ooxml::LN_CT_TblGridBase_gridCol, pXValue, false);
                m_aStates.top().nCells++;

                // Push cell properties.
                m_aStates.top().aTableCellsSprms.push_back(m_aStates.top().aTableCellSprms);
                m_aStates.top().aTableCellsAttributes.push_back(m_aStates.top().aTableCellAttributes);
                m_aStates.top().aTableCellSprms = m_aDefaultState.aTableCellSprms;
                m_aStates.top().aTableCellAttributes = m_aDefaultState.aTableCellAttributes;
                // We assume text after a row definition always belongs to the table, to handle text before the real INTBL token
                dispatchFlag(RTF_INTBL);
                m_nCellxMax = std::max(m_nCellxMax, nParam);
            }
            break;
        case RTF_TRRH:
            {
                OUString hRule("auto");
                if ( nParam < 0 )
                {
                    RTFValue::Pointer_t pAbsValue(new RTFValue(-nParam));
                    pIntValue.swap( pAbsValue );

                    hRule = "exact";
                }
                else if ( nParam > 0 )
                    hRule = "atLeast";

                lcl_putNestedAttribute(m_aStates.top().aTableRowSprms,
                        NS_ooxml::LN_CT_TrPrBase_trHeight, NS_ooxml::LN_CT_Height_val, pIntValue);

                RTFValue::Pointer_t pHRule(new RTFValue(hRule));
                lcl_putNestedAttribute(m_aStates.top().aTableRowSprms,
                    NS_ooxml::LN_CT_TrPrBase_trHeight, NS_ooxml::LN_CT_Height_hRule, pHRule);
            }
            break;
        case RTF_TRLEFT:
            {
                // the value is in twips
                lcl_putNestedAttribute(m_aStates.top().aTableRowSprms,
                        NS_ooxml::LN_CT_TblPrBase_tblInd, NS_ooxml::LN_CT_TblWidth_type,
                        RTFValue::Pointer_t(new RTFValue(NS_ooxml::LN_Value_ST_TblWidth_dxa)));
                lcl_putNestedAttribute(m_aStates.top().aTableRowSprms,
                        NS_ooxml::LN_CT_TblPrBase_tblInd, NS_ooxml::LN_CT_TblWidth_w,
                        RTFValue::Pointer_t(new RTFValue(nParam)));
            }
            break;
        case RTF_COLS:
            lcl_putNestedAttribute(m_aStates.top().aSectionSprms,
                    NS_ooxml::LN_EG_SectPrContents_cols, NS_ooxml::LN_CT_Columns_num, pIntValue);
            break;
        case RTF_COLSX:
            lcl_putNestedAttribute(m_aStates.top().aSectionSprms,
                    NS_ooxml::LN_EG_SectPrContents_cols, NS_ooxml::LN_CT_Columns_space, pIntValue);
            break;
        case RTF_COLNO:
            lcl_putNestedSprm(m_aStates.top().aSectionSprms,
                    NS_ooxml::LN_EG_SectPrContents_cols, NS_ooxml::LN_CT_Columns_col, pIntValue);
            break;
        case RTF_COLW:
        case RTF_COLSR:
            {
                RTFSprms& rAttributes = lcl_getLastAttributes(m_aStates.top().aSectionSprms, NS_ooxml::LN_EG_SectPrContents_cols);
                rAttributes.set((nKeyword == RTF_COLW ? NS_ooxml::LN_CT_Column_w : NS_ooxml::LN_CT_Column_space), pIntValue);
            }
            break;
        case RTF_PAPERH: // fall through: set the default + current value
            lcl_putNestedAttribute(m_aDefaultState.aSectionSprms,
                    NS_ooxml::LN_EG_SectPrContents_pgSz, NS_ooxml::LN_CT_PageSz_h, pIntValue, true);
        case RTF_PGHSXN:
            lcl_putNestedAttribute(m_aStates.top().aSectionSprms,
                    NS_ooxml::LN_EG_SectPrContents_pgSz, NS_ooxml::LN_CT_PageSz_h, pIntValue, true);
            break;
        case RTF_PAPERW: // fall through: set the default + current value
            lcl_putNestedAttribute(m_aDefaultState.aSectionSprms,
                    NS_ooxml::LN_EG_SectPrContents_pgSz, NS_ooxml::LN_CT_PageSz_w, pIntValue, true);
        case RTF_PGWSXN:
            lcl_putNestedAttribute(m_aStates.top().aSectionSprms,
                    NS_ooxml::LN_EG_SectPrContents_pgSz, NS_ooxml::LN_CT_PageSz_w, pIntValue, true);
            break;
        case RTF_MARGL: // fall through: set the default + current value
            lcl_putNestedAttribute(m_aDefaultState.aSectionSprms,
                    NS_ooxml::LN_EG_SectPrContents_pgMar, NS_ooxml::LN_CT_PageMar_left, pIntValue, true);
        case RTF_MARGLSXN:
            lcl_putNestedAttribute(m_aStates.top().aSectionSprms,
                    NS_ooxml::LN_EG_SectPrContents_pgMar, NS_ooxml::LN_CT_PageMar_left, pIntValue, true);
            break;
        case RTF_MARGR: // fall through: set the default + current value
            lcl_putNestedAttribute(m_aDefaultState.aSectionSprms,
                    NS_ooxml::LN_EG_SectPrContents_pgMar, NS_ooxml::LN_CT_PageMar_right, pIntValue, true);
        case RTF_MARGRSXN:
            lcl_putNestedAttribute(m_aStates.top().aSectionSprms,
                    NS_ooxml::LN_EG_SectPrContents_pgMar, NS_ooxml::LN_CT_PageMar_right, pIntValue, true);
            break;
        case RTF_MARGT: // fall through: set the default + current value
            lcl_putNestedAttribute(m_aDefaultState.aSectionSprms,
                    NS_ooxml::LN_EG_SectPrContents_pgMar, NS_ooxml::LN_CT_PageMar_top, pIntValue, true);
        case RTF_MARGTSXN:
            lcl_putNestedAttribute(m_aStates.top().aSectionSprms,
                    NS_ooxml::LN_EG_SectPrContents_pgMar, NS_ooxml::LN_CT_PageMar_top, pIntValue, true);
            break;
        case RTF_MARGB: // fall through: set the default + current value
            lcl_putNestedAttribute(m_aDefaultState.aSectionSprms,
                    NS_ooxml::LN_EG_SectPrContents_pgMar, NS_ooxml::LN_CT_PageMar_bottom, pIntValue, true);
        case RTF_MARGBSXN:
            lcl_putNestedAttribute(m_aStates.top().aSectionSprms,
                    NS_ooxml::LN_EG_SectPrContents_pgMar, NS_ooxml::LN_CT_PageMar_bottom, pIntValue, true);
            break;
        case RTF_HEADERY:
            lcl_putNestedAttribute(m_aStates.top().aSectionSprms,
                    NS_ooxml::LN_EG_SectPrContents_pgMar, NS_ooxml::LN_CT_PageMar_header, pIntValue, true);
            break;
        case RTF_FOOTERY:
            lcl_putNestedAttribute(m_aStates.top().aSectionSprms,
                    NS_ooxml::LN_EG_SectPrContents_pgMar, NS_ooxml::LN_CT_PageMar_footer, pIntValue, true);
            break;
        case RTF_DEFTAB:
            m_aSettingsTableSprms.set(NS_ooxml::LN_CT_Settings_defaultTabStop, pIntValue);
            break;
        case RTF_LINEMOD:
            lcl_putNestedAttribute(m_aStates.top().aSectionSprms,
                    NS_ooxml::LN_EG_SectPrContents_lnNumType, NS_ooxml::LN_CT_LineNumber_countBy, pIntValue);
            break;
        case RTF_LINEX:
            if (nParam)
                lcl_putNestedAttribute(m_aStates.top().aSectionSprms,
                        NS_ooxml::LN_EG_SectPrContents_lnNumType, NS_ooxml::LN_CT_LineNumber_distance, pIntValue);
            break;
        case RTF_LINESTARTS:
            lcl_putNestedAttribute(m_aStates.top().aSectionSprms,
                    NS_ooxml::LN_EG_SectPrContents_lnNumType, NS_ooxml::LN_CT_LineNumber_start, pIntValue);
            break;
        case RTF_REVAUTH:
        case RTF_REVAUTHDEL:
            {
                RTFValue::Pointer_t pValue(new RTFValue(m_aAuthors[nParam]));
                lcl_putNestedAttribute(m_aStates.top().aCharacterSprms,
                        NS_ooxml::LN_trackchange, NS_ooxml::LN_CT_TrackChange_author, pValue);
            }
            break;
        case RTF_REVDTTM:
        case RTF_REVDTTMDEL:
            {
                OUString aStr(OStringToOUString(lcl_DTTM22OString(nParam), m_aStates.top().nCurrentEncoding));
                RTFValue::Pointer_t pValue(new RTFValue(aStr));
                lcl_putNestedAttribute(m_aStates.top().aCharacterSprms,
                        NS_ooxml::LN_trackchange, NS_ooxml::LN_CT_TrackChange_date, pValue);
            }
            break;
        case RTF_SHPLEFT:
            m_aStates.top().aShape.nLeft = TWIP_TO_MM100(nParam);
            break;
        case RTF_SHPTOP:
            m_aStates.top().aShape.nTop = TWIP_TO_MM100(nParam);
            break;
        case RTF_SHPRIGHT:
            m_aStates.top().aShape.nRight = TWIP_TO_MM100(nParam);
            break;
        case RTF_SHPBOTTOM:
            m_aStates.top().aShape.nBottom = TWIP_TO_MM100(nParam);
            break;
        case RTF_SHPZ:
            m_aStates.top().aShape.oZ.reset(nParam);
            break;
        case RTF_FFTYPE:
            switch (nParam)
            {
                case 0: m_nFormFieldType = FORMFIELD_TEXT; break;
                case 1: m_nFormFieldType = FORMFIELD_CHECKBOX; break;
                case 2: m_nFormFieldType = FORMFIELD_LIST; break;
                default: m_nFormFieldType = FORMFIELD_NONE; break;
            }
            break;
        case RTF_FFDEFRES:
            if (m_nFormFieldType == FORMFIELD_CHECKBOX)
                m_aFormfieldSprms.set(NS_ooxml::LN_CT_FFCheckBox_default, pIntValue);
            else if (m_nFormFieldType == FORMFIELD_LIST)
                m_aFormfieldSprms.set(NS_ooxml::LN_CT_FFDDList_default, pIntValue);
            break;
        case RTF_FFRES:
            if (m_nFormFieldType == FORMFIELD_CHECKBOX)
                m_aFormfieldSprms.set(NS_ooxml::LN_CT_FFCheckBox_checked, pIntValue);
            else if (m_nFormFieldType == FORMFIELD_LIST)
                m_aFormfieldSprms.set(NS_ooxml::LN_CT_FFDDList_result, pIntValue);
            break;
        case RTF_EDMINS:
            if (m_xDocumentProperties.is())
                m_xDocumentProperties->setEditingDuration(nParam);
            break;
        case RTF_NOFPAGES:
        case RTF_NOFWORDS:
        case RTF_NOFCHARS:
        case RTF_NOFCHARSWS:
            if (m_xDocumentProperties.is())
            {
                uno::Sequence<beans::NamedValue> aSet = m_xDocumentProperties->getDocumentStatistics();
                OUString aName;
                switch (nKeyword)
                {
                    case RTF_NOFPAGES: aName = "PageCount"; nParam = 99; break;
                    case RTF_NOFWORDS: aName = "WordCount"; break;
                    case RTF_NOFCHARS: aName = "CharacterCount"; break;
                    case RTF_NOFCHARSWS: aName = "NonWhitespaceCharacterCount"; break;
                    default: break;
                }
                if (!aName.isEmpty())
                {
                    bool bFound = false;
                    int nLen = aSet.getLength();
                    for (int i = 0; i < nLen; ++i)
                        if (aSet[i].Name.equals(aName))
                            aSet[i].Value = uno::makeAny(sal_Int32(nParam));
                    if (!bFound)
                    {
                        aSet.realloc(nLen + 1);
                        aSet[nLen].Name = aName;
                        aSet[nLen].Value = uno::makeAny(sal_Int32(nParam));
                    }
                    m_xDocumentProperties->setDocumentStatistics(aSet);
                }
            }
            break;
        case RTF_VERSION:
            if (m_xDocumentProperties.is())
                m_xDocumentProperties->setEditingCycles(nParam);
            break;
        case RTF_VERN:
            // Ignore this for now, later the RTF writer version could be used to add hacks for older buggy writers.
            break;
        case RTF_FTNSTART:
            lcl_putNestedSprm(m_aDefaultState.aParagraphSprms,
                    NS_ooxml::LN_EG_SectPrContents_footnotePr, NS_ooxml::LN_EG_FtnEdnNumProps_numStart, pIntValue);
            break;
        case RTF_AFTNSTART:
            lcl_putNestedSprm(m_aDefaultState.aParagraphSprms,
                    NS_ooxml::LN_EG_SectPrContents_endnotePr, NS_ooxml::LN_EG_FtnEdnNumProps_numStart, pIntValue);
            break;
        case RTF_DFRMTXTX:
            m_aStates.top().aFrame.setSprm(NS_sprm::LN_PDxaFromText, nParam);
            break;
        case RTF_DFRMTXTY:
            m_aStates.top().aFrame.setSprm(NS_sprm::LN_PDyaFromText, nParam);
            break;
        case RTF_DXFRTEXT:
            {
                m_aStates.top().aFrame.setSprm(NS_sprm::LN_PDxaFromText, nParam);
                m_aStates.top().aFrame.setSprm(NS_sprm::LN_PDyaFromText, nParam);
            }
            break;
        case RTF_FLYVERT:
            {
                RTFVertOrient aVertOrient(nParam);
                m_aStates.top().aFrame.setSprm(NS_ooxml::LN_CT_FramePr_yAlign, aVertOrient.GetAlign());
                m_aStates.top().aFrame.setSprm(NS_ooxml::LN_CT_FramePr_vAnchor, aVertOrient.GetAnchor());
            }
            break;
        case RTF_FLYHORZ:
            {
                RTFHoriOrient aHoriOrient(nParam);
                m_aStates.top().aFrame.setSprm(NS_ooxml::LN_CT_FramePr_xAlign, aHoriOrient.GetAlign());
                m_aStates.top().aFrame.setSprm(NS_ooxml::LN_CT_FramePr_hAnchor, aHoriOrient.GetAnchor());
            }
            break;
        case RTF_FLYANCHOR:
            m_aStates.top().aFrame.nAnchorType = nParam;
            break;
        case RTF_WMETAFILE:
            m_aStates.top().aPicture.eWMetafile = nParam;
            break;
        case RTF_SB:
            lcl_putNestedAttribute(m_aStates.top().aParagraphSprms,
                    NS_ooxml::LN_CT_PPrBase_spacing, NS_ooxml::LN_CT_Spacing_before, pIntValue, true);
            break;
        case RTF_SA:
            lcl_putNestedAttribute(m_aStates.top().aParagraphSprms,
                    NS_ooxml::LN_CT_PPrBase_spacing, NS_ooxml::LN_CT_Spacing_after, pIntValue, true);
            break;
        case RTF_DPX:
            m_aStates.top().aDrawingObject.nLeft = TWIP_TO_MM100(nParam);
            break;
        case RTF_DPY:
            m_aStates.top().aDrawingObject.nTop = TWIP_TO_MM100(nParam);
            break;
        case RTF_DPXSIZE:
            m_aStates.top().aDrawingObject.nRight = TWIP_TO_MM100(nParam);
            break;
        case RTF_DPYSIZE:
            m_aStates.top().aDrawingObject.nBottom = TWIP_TO_MM100(nParam);
            break;
        case RTF_PNSTART:
            m_aStates.top().aTableSprms.set(NS_rtf::LN_ISTARTAT, pIntValue);
            break;
        case RTF_PNF:
            {
                int nFontIndex = getFontIndex(nParam);
                RTFValue::Pointer_t pValue(new RTFValue(nFontIndex));
                lcl_putNestedSprm(m_aStates.top().aTableSprms, NS_ooxml::LN_CT_Lvl_rPr, NS_sprm::LN_CRgFtc0, pValue);
            }
            break;
        case RTF_VIEWSCALE:
            m_aSettingsTableAttributes.set(NS_ooxml::LN_CT_Zoom_percent, pIntValue);
            break;
        case RTF_BIN:
            {
                m_aStates.top().nInternalState = INTERNAL_BIN;
                m_aStates.top().nBinaryToRead = nParam;
            }
            break;
        case RTF_DPLINECOR:
            m_aStates.top().aDrawingObject.nLineColorR = nParam; m_aStates.top().aDrawingObject.bHasLineColor = true;
            break;
        case RTF_DPLINECOG:
            m_aStates.top().aDrawingObject.nLineColorG = nParam; m_aStates.top().aDrawingObject.bHasLineColor = true;
            break;
        case RTF_DPLINECOB:
            m_aStates.top().aDrawingObject.nLineColorB = nParam; m_aStates.top().aDrawingObject.bHasLineColor = true;
            break;
        case RTF_DPFILLBGCR:
            m_aStates.top().aDrawingObject.nFillColorR = nParam; m_aStates.top().aDrawingObject.bHasFillColor = true;
            break;
        case RTF_DPFILLBGCG:
            m_aStates.top().aDrawingObject.nFillColorG = nParam; m_aStates.top().aDrawingObject.bHasFillColor = true;
            break;
        case RTF_DPFILLBGCB:
            m_aStates.top().aDrawingObject.nFillColorB = nParam; m_aStates.top().aDrawingObject.bHasFillColor = true;
            break;
        case RTF_LI:
            {
                m_aStates.top().aParagraphSprms.set(NS_sprm::LN_PDxaLeft, pIntValue);
                // It turns out \li should reset the \fi inherited from the stylesheet.
                // So set the direct formatting to zero, if we don't have such direct formatting yet.
                if (!m_aStates.top().aParagraphSprms.find(NS_sprm::LN_PDxaLeft1).get())
                    m_aStates.top().aParagraphSprms.set(NS_sprm::LN_PDxaLeft1, RTFValue::Pointer_t(new RTFValue(0)));
            }
            break;
        case RTF_CLSHDNG:
            {
                int nValue = -1;
                switch (nParam)
                {
                    case 500: nValue = 2; break;
                    case 1000: nValue = 3; break;
                    case 1200: nValue = 27; break;
                    case 1500: nValue = 28; break;
                    case 2000: nValue = 4; break;
                    case 2500: nValue = 5; break;
                    case 3000: nValue = 6; break;
                    case 3500: nValue = 43; break;
                    case 3700: nValue = 44; break;
                    case 4000: nValue = 7; break;
                    case 4500: nValue = 46; break;
                    case 5000: nValue = 8; break;
                    case 5500: nValue = 49; break;
                    case 6000: nValue = 9; break;
                    case 6200: nValue = 51; break;
                    case 6500: nValue = 52; break;
                    case 7000: nValue = 10; break;
                    case 7500: nValue = 11; break;
                    case 8000: nValue = 12; break;
                    case 8500: nValue = 57; break;
                    case 8700: nValue = 58; break;
                    case 9000: nValue = 13; break;
                    case 9500: nValue = 60; break;
                    default: break;
                }
                if (nValue != -1)
                    lcl_putNestedAttribute(m_aStates.top().aTableCellSprms,
                            NS_ooxml::LN_CT_TcPrBase_shd, NS_ooxml::LN_CT_Shd_val, RTFValue::Pointer_t(new RTFValue(nValue)));
            }
            break;
        case RTF_DODHGT:
            m_aStates.top().aDrawingObject.nDhgt = nParam;
            break;
        case RTF_DPPOLYCOUNT:
            if (nParam >= 0)
            {
                m_aStates.top().aDrawingObject.nPolyLineCount = nParam;
                m_aStates.top().aDrawingObject.aPolyLinePoints.realloc(nParam);
            }
            break;
        case RTF_DPPTX:
            {
                RTFDrawingObject& rDrawingObject = m_aStates.top().aDrawingObject;

                if (!rDrawingObject.aPolyLinePoints.hasElements())
                    dispatchValue(RTF_DPPOLYCOUNT, 2);

                rDrawingObject.aPolyLinePoints[rDrawingObject.aPolyLinePoints.getLength() - rDrawingObject.nPolyLineCount].X = TWIP_TO_MM100(nParam);
            }
            break;
        case RTF_DPPTY:
            {
                RTFDrawingObject& rDrawingObject = m_aStates.top().aDrawingObject;
                if (rDrawingObject.aPolyLinePoints.hasElements())
                {
                    rDrawingObject.aPolyLinePoints[rDrawingObject.aPolyLinePoints.getLength() - rDrawingObject.nPolyLineCount].Y = TWIP_TO_MM100(nParam);
                    rDrawingObject.nPolyLineCount--;
                    if (rDrawingObject.nPolyLineCount == 0)
                    {
                        uno::Sequence< uno::Sequence<awt::Point> >aPointSequenceSequence(1);
                        aPointSequenceSequence[0] = rDrawingObject.aPolyLinePoints;
                        rDrawingObject.xPropertySet->setPropertyValue("PolyPolygon", uno::Any(aPointSequenceSequence));
                    }
                }
            }
            break;
        case RTF_SHPFBLWTXT:
            if (nParam == 1)
            {
                // Shape is below text -> send it to the background.
                m_aStates.top().aCharacterAttributes.erase(NS_ooxml::LN_CT_WrapSquare_wrapText);
                m_aStates.top().aCharacterAttributes.set(NS_rtf::LN_WR, RTFValue::Pointer_t(new RTFValue(3)));
            }
            break;
        case RTF_TS:
            m_aStates.top().bHasTableStyle = true;
            break;
        default:
            {
                SAL_INFO("writerfilter", "TODO handle value '" << lcl_RtfToString(nKeyword) << "'");
                aSkip.setParsed(false);
            }
            break;
    }
    return 0;
}

int RTFDocumentImpl::dispatchToggle(RTFKeyword nKeyword, bool bParam, int nParam)
{
    checkUnicode(/*bUnicode =*/ true, /*bHex =*/ true);
    setNeedSect();
    RTFSkipDestination aSkip(*this);
    int nSprm = -1;
    RTFValue::Pointer_t pBoolValue(new RTFValue(!bParam || nParam != 0));

    // Map all underline toggles to a single sprm.
    switch (nKeyword)
    {
        case RTF_UL: nSprm = 1; break;
        case RTF_ULDASH: nSprm = 7; break;
        case RTF_ULDASHD: nSprm = 9; break;
        case RTF_ULDASHDD: nSprm = 10; break;
        case RTF_ULDB: nSprm = 3; break;
        case RTF_ULHWAVE: nSprm = 27; break;
        case RTF_ULLDASH: nSprm = 39; break;
        case RTF_ULTH: nSprm = 6; break;
        case RTF_ULTHD: nSprm = 20; break;
        case RTF_ULTHDASH: nSprm = 23; break;
        case RTF_ULTHDASHD: nSprm = 25; break;
        case RTF_ULTHDASHDD: nSprm = 26; break;
        case RTF_ULTHLDASH: nSprm = 55; break;
        case RTF_ULULDBWAVE: nSprm = 43; break;
        case RTF_ULWAVE: nSprm = 11; break;
        default: break;
    }
    if (nSprm >= 0)
    {
        RTFValue::Pointer_t pValue(new RTFValue((!bParam || nParam != 0) ? nSprm : 0));
        m_aStates.top().aCharacterSprms.set(NS_sprm::LN_CKul, pValue);
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
    if (nSprm >= 0)
    {
        RTFValue::Pointer_t pValue(new RTFValue((!bParam || nParam != 0) ? nSprm : 0));
        m_aStates.top().aCharacterSprms.set(NS_sprm::LN_CKcd, pValue);
        return 0;
    }

    // Trivial character sprms.
    switch (nKeyword)
    {
        case RTF_B: nSprm = NS_sprm::LN_CFBold; break;
        case RTF_AB: nSprm = NS_sprm::LN_CFBoldBi; break;
        case RTF_I: nSprm = NS_sprm::LN_CFItalic; break;
        case RTF_AI: nSprm = NS_sprm::LN_CFItalicBi; break;
        case RTF_UL: nSprm = NS_sprm::LN_CKul; break;
        case RTF_OUTL: nSprm = NS_sprm::LN_CFOutline; break;
        case RTF_SHAD: nSprm = NS_sprm::LN_CFShadow; break;
        case RTF_V: nSprm = NS_sprm::LN_CFVanish; break;
        case RTF_STRIKE: nSprm = NS_sprm::LN_CFStrike; break;
        case RTF_STRIKED: nSprm = NS_sprm::LN_CFDStrike; break;
        case RTF_SCAPS: nSprm = NS_sprm::LN_CFSmallCaps; break;
        case RTF_IMPR: nSprm = NS_sprm::LN_CFImprint; break;
        case RTF_CAPS: nSprm = NS_sprm::LN_CFCaps; break;
        default: break;
    }
    if (nSprm >= 0)
    {
        m_aStates.top().aCharacterSprms.set(nSprm, pBoolValue);
        return 0;
    }

    switch (nKeyword)
    {
        case RTF_ASPALPHA:
            m_aStates.top().aParagraphSprms.set(NS_sprm::LN_PFAutoSpaceDE, pBoolValue);
            break;
        case RTF_DELETED:
        case RTF_REVISED:
            {
                RTFValue::Pointer_t pValue(new RTFValue(nKeyword == RTF_DELETED ? ooxml::OOXML_del : ooxml::OOXML_ins));
                lcl_putNestedAttribute(m_aStates.top().aCharacterSprms,
                        NS_ooxml::LN_trackchange, NS_ooxml::LN_token, pValue);
            }
            break;
        default:
            {
                SAL_INFO("writerfilter", "TODO handle toggle '" << lcl_RtfToString(nKeyword) << "'");
                aSkip.setParsed(false);
            }
            break;
    }
    return 0;
}

int RTFDocumentImpl::pushState()
{
    //SAL_INFO("writerfilter", OSL_THIS_FUNC << " before push: " << m_pTokenizer->getGroup());

    checkUnicode(/*bUnicode =*/ true, /*bHex =*/ true);
    m_nGroupStartPos = Strm().Tell();

    if (m_aStates.empty())
        m_aStates.push(m_aDefaultState);
    else
    {
        if (m_aStates.top().nDestinationState == DESTINATION_MR)
            lcl_DestinationToMath(m_aStates.top().aDestinationText, m_aMathBuffer, m_bMathNor);
        m_aStates.push(m_aStates.top());
    }
    m_aStates.top().aDestinationText.setLength(0);

    m_pTokenizer->pushGroup();

    switch (m_aStates.top().nDestinationState)
    {
        case DESTINATION_FONTTABLE:
            m_aStates.top().nDestinationState = DESTINATION_FONTENTRY;
            break;
        case DESTINATION_STYLESHEET:
            m_aStates.top().nDestinationState = DESTINATION_STYLEENTRY;
            break;
        case DESTINATION_FIELDRESULT:
        case DESTINATION_SHAPETEXT:
        case DESTINATION_FORMFIELD:
        case DESTINATION_FIELDINSTRUCTION:
            m_aStates.top().nDestinationState = DESTINATION_NORMAL;
            break;
        case DESTINATION_MNUM:
        case DESTINATION_MDEN:
        case DESTINATION_ME:
        case DESTINATION_MFNAME:
        case DESTINATION_MLIM:
        case DESTINATION_MSUB:
        case DESTINATION_MSUP:
        case DESTINATION_MDEG:
        case DESTINATION_MOMATH:
            m_aStates.top().nDestinationState = DESTINATION_MR;
            break;
        case DESTINATION_REVISIONTABLE:
            m_aStates.top().nDestinationState = DESTINATION_REVISIONENTRY;
            break;
        default:
            break;
    }

    return 0;
}

RTFSprms RTFDocumentImpl::mergeSprms()
{
    RTFSprms aSprms;
    for (RTFSprms::Iterator_t i = m_aStates.top().aTableSprms.begin();
            i != m_aStates.top().aTableSprms.end(); ++i)
        aSprms.set(i->first, i->second);
    for (RTFSprms::Iterator_t i = m_aStates.top().aCharacterSprms.begin();
            i != m_aStates.top().aCharacterSprms.end(); ++i)
        aSprms.set(i->first, i->second);
    for (RTFSprms::Iterator_t i = m_aStates.top().aParagraphSprms.begin();
            i != m_aStates.top().aParagraphSprms.end(); ++i)
        aSprms.set(i->first, i->second);
    return aSprms;
}

void RTFDocumentImpl::resetSprms()
{
    m_aStates.top().aTableSprms.clear();
    m_aStates.top().aCharacterSprms.clear();
    m_aStates.top().aParagraphSprms.clear();
}

RTFSprms RTFDocumentImpl::mergeAttributes()
{
    RTFSprms aAttributes;
    for (RTFSprms::Iterator_t i = m_aStates.top().aTableAttributes.begin();
            i != m_aStates.top().aTableAttributes.end(); ++i)
        aAttributes.set(i->first, i->second);
    for (RTFSprms::Iterator_t i = m_aStates.top().aCharacterAttributes.begin();
            i != m_aStates.top().aCharacterAttributes.end(); ++i)
        aAttributes.set(i->first, i->second);
    for (RTFSprms::Iterator_t i = m_aStates.top().aParagraphAttributes.begin();
            i != m_aStates.top().aParagraphAttributes.end(); ++i)
        aAttributes.set(i->first, i->second);
    return aAttributes;
}

void RTFDocumentImpl::resetAttributes()
{
    m_aStates.top().aTableAttributes.clear();
    m_aStates.top().aCharacterAttributes.clear();
    m_aStates.top().aParagraphAttributes.clear();
}

int RTFDocumentImpl::popState()
{
    //SAL_INFO("writerfilter", OSL_THIS_FUNC << " before pop: m_pTokenizer->getGroup() " << m_pTokenizer->getGroup() <<
    //                         ", dest state: " << m_aStates.top().nDestinationState);

    checkUnicode(/*bUnicode =*/ true, /*bHex =*/ true);
    RTFParserState aState(m_aStates.top());
    m_bWasInFrame = aState.aFrame.inFrame();

    switch (aState.nDestinationState)
    {
        case DESTINATION_FONTTABLE:
            {
                writerfilter::Reference<Table>::Pointer_t const pTable(new RTFReferenceTable(m_aFontTableEntries));
                Mapper().table(NS_rtf::LN_FONTTABLE, pTable);
            }
            break;
        case DESTINATION_STYLESHEET:
            {
                writerfilter::Reference<Table>::Pointer_t const pTable(new RTFReferenceTable(m_aStyleTableEntries));
                Mapper().table(NS_rtf::LN_STYLESHEET, pTable);
            }
            break;
        case DESTINATION_LISTOVERRIDETABLE:
            {
                RTFSprms aListTableAttributes;
                writerfilter::Reference<Properties>::Pointer_t const pProp(new RTFReferenceProperties(aListTableAttributes, m_aListTableSprms));
                RTFReferenceTable::Entries_t aListTableEntries;
                aListTableEntries.insert(make_pair(0, pProp));
                writerfilter::Reference<Table>::Pointer_t const pTable(new RTFReferenceTable(aListTableEntries));
                Mapper().table(NS_rtf::LN_LISTTABLE, pTable);
            }
            break;
        case DESTINATION_LISTENTRY:
            for (RTFSprms::Iterator_t i = aState.aListLevelEntries.begin(); i != aState.aListLevelEntries.end(); ++i)
                aState.aTableSprms.set(i->first, i->second, false);
            break;
        case DESTINATION_FIELDINSTRUCTION:
            {
                RTFValue::Pointer_t pValue(new RTFValue(m_aFormfieldAttributes, m_aFormfieldSprms));
                RTFSprms aFFAttributes;
                RTFSprms aFFSprms;
                aFFSprms.set(NS_ooxml::LN_ffdata, pValue);
                writerfilter::Reference<Properties>::Pointer_t const pProperties(new RTFReferenceProperties(aFFAttributes, aFFSprms));
                Mapper().props(pProperties);
                m_aFormfieldAttributes.clear();
                m_aFormfieldSprms.clear();
                singleChar(0x14);
            }
            break;
        case DESTINATION_FIELDRESULT:
            singleChar(0x15);
            break;
        case DESTINATION_LEVELTEXT:
            {
                OUString aStr = m_aStates.top().aDestinationText.makeStringAndClear();

                // The first character is the length of the string (the rest should be ignored).
                sal_Int32 nLength(aStr.toChar());
                OUString aValue;
                if (nLength <= aStr.getLength())
                    aValue = aStr.copy(1, nLength);
                else
                    aValue = aStr;
                RTFValue::Pointer_t pValue(new RTFValue(aValue, true));
                aState.aTableAttributes.set(NS_ooxml::LN_CT_LevelText_val, pValue);
            }
            break;
        case DESTINATION_LEVELNUMBERS:
            if (aState.aTableSprms.find(NS_ooxml::LN_CT_Lvl_lvlText))
            {
                RTFSprms& rAttributes = aState.aTableSprms.find(NS_ooxml::LN_CT_Lvl_lvlText)->getAttributes();
                RTFValue::Pointer_t pValue = rAttributes.find(NS_ooxml::LN_CT_LevelText_val);
                OUString aOrig = pValue->getString();

                OUStringBuffer aBuf;
                sal_Int32 nReplaces = 1;
                for (int i = 0; i < aOrig.getLength(); i++)
                {
                    if (std::find(aState.aLevelNumbers.begin(), aState.aLevelNumbers.end(), i+1)
                            != aState.aLevelNumbers.end())
                    {
                        aBuf.append(sal_Unicode('%'));
                        // '1.1.1' -> '%1.%2.%3', but '1.' (with '2.' prefix omitted) is %2.
                        aBuf.append(sal_Int32(nReplaces++ + aState.nListLevelNum + 1 - aState.aLevelNumbers.size()));
                    }
                    else
                        aBuf.append(aOrig.copy(i, 1));
                }

                pValue->setString(aBuf.makeStringAndClear());
            }
            break;
        case DESTINATION_SHAPEPROPERTYNAME:
            aState.aShape.aProperties.push_back(make_pair(m_aStates.top().aDestinationText.makeStringAndClear(), OUString()));
            break;
        case DESTINATION_SHAPEPROPERTYVALUE:
            if (aState.aShape.aProperties.size())
            {
                aState.aShape.aProperties.back().second = m_aStates.top().aDestinationText.makeStringAndClear();
                if (m_aStates.top().bHadShapeText)
                    m_pSdrImport->append(aState.aShape.aProperties.back().first, aState.aShape.aProperties.back().second);
            }
            break;
        case DESTINATION_PICPROP:
        case DESTINATION_SHAPEINSTRUCTION:
            if (!m_bObject && !aState.bInListpicture && !m_aStates.top().bHadShapeText && !m_aStates.top().bInShapeGroup)
                m_pSdrImport->resolve(m_aStates.top().aShape, true);
            break;
        case DESTINATION_BOOKMARKSTART:
            {
                OUString aStr = m_aStates.top().aDestinationText.makeStringAndClear();
                int nPos = m_aBookmarks.size();
                m_aBookmarks[aStr] = nPos;
                Mapper().props(lcl_getBookmarkProperties(nPos, aStr));
            }
            break;
        case DESTINATION_BOOKMARKEND:
            Mapper().props(lcl_getBookmarkProperties(m_aBookmarks[m_aStates.top().aDestinationText.makeStringAndClear()]));
            break;
        case DESTINATION_PICT:
            resolvePict(true);
            break;
        case DESTINATION_FORMFIELDNAME:
            {
                RTFValue::Pointer_t pValue(new RTFValue(m_aStates.top().aDestinationText.makeStringAndClear()));
                m_aFormfieldSprms.set(NS_ooxml::LN_CT_FFData_name, pValue);
            }
            break;
        case DESTINATION_FORMFIELDLIST:
            {
                RTFValue::Pointer_t pValue(new RTFValue(m_aStates.top().aDestinationText.makeStringAndClear()));
                m_aFormfieldSprms.set(NS_ooxml::LN_CT_FFDDList_listEntry, pValue);
            }
            break;
        case DESTINATION_DATAFIELD:
            {
                OString aStr = OUStringToOString(m_aStates.top().aDestinationText.makeStringAndClear(), aState.nCurrentEncoding);
                // decode hex dump
                OStringBuffer aBuf;
                const char *str = aStr.getStr();
                int b = 0, count = 2;
                for (int i = 0; i < aStr.getLength(); ++i)
                {
                    char ch = str[i];
                    if (ch != 0x0d && ch != 0x0a)
                    {
                        b = b << 4;
                        sal_Int8 parsed = m_pTokenizer->asHex(ch);
                        if (parsed == -1)
                            return ERROR_HEX_INVALID;
                        b += parsed;
                        count--;
                        if (!count)
                        {
                            aBuf.append((char)b);
                            count = 2;
                            b = 0;
                        }
                    }
                }
                aStr = aBuf.makeStringAndClear();
                // ignore the first bytes
                if (aStr.getLength() > 8)
                    aStr = aStr.copy(8);
                // extract name
                int nLength = aStr.toChar();
                aStr = aStr.copy(1);
                OString aName = aStr.copy(0, nLength);
                aStr = aStr.copy(nLength+1); // zero-terminated string
                // extract default text
                nLength = aStr.toChar();
                aStr = aStr.copy(1);
                RTFValue::Pointer_t pNValue(new RTFValue(OStringToOUString(aName, aState.nCurrentEncoding)));
                m_aFormfieldSprms.set(NS_ooxml::LN_CT_FFData_name, pNValue);
                if (nLength > 0)
                {
                    OString aDefaultText = aStr.copy(0, nLength);
                    RTFValue::Pointer_t pDValue(new RTFValue(OStringToOUString(aDefaultText, aState.nCurrentEncoding)));
                    m_aFormfieldSprms.set(NS_ooxml::LN_CT_FFTextInput_default, pDValue);
                }

                m_bFormField = false;
            }
            break;
        case DESTINATION_CREATIONTIME:
            if (m_xDocumentProperties.is())
                m_xDocumentProperties->setCreationDate(lcl_getDateTime(aState));
            break;
        case DESTINATION_REVISIONTIME:
            if (m_xDocumentProperties.is())
                m_xDocumentProperties->setModificationDate(lcl_getDateTime(aState));
            break;
        case DESTINATION_PRINTTIME:
            if (m_xDocumentProperties.is())
                m_xDocumentProperties->setPrintDate(lcl_getDateTime(aState));
            break;
        case DESTINATION_AUTHOR:
            if (m_xDocumentProperties.is())
                m_xDocumentProperties->setAuthor(m_aStates.top().aDestinationText.makeStringAndClear());
            break;
        case DESTINATION_KEYWORDS:
            if (m_xDocumentProperties.is())
                m_xDocumentProperties->setKeywords(comphelper::string::convertCommaSeparated(m_aStates.top().aDestinationText.makeStringAndClear()));
            break;
        case DESTINATION_COMMENT:
            if (m_xDocumentProperties.is())
                m_xDocumentProperties->setGenerator(m_aStates.top().aDestinationText.makeStringAndClear());
            break;
        case DESTINATION_SUBJECT:
            if (m_xDocumentProperties.is())
                m_xDocumentProperties->setSubject(m_aStates.top().aDestinationText.makeStringAndClear());
            break;
        case DESTINATION_DOCCOMM:
            if (m_xDocumentProperties.is())
                m_xDocumentProperties->setDescription(m_aStates.top().aDestinationText.makeStringAndClear());
            break;
        case DESTINATION_OPERATOR:
        case DESTINATION_COMPANY:
            {
                OUString aName = aState.nDestinationState == DESTINATION_OPERATOR ? OUString("Operator") : OUString("Company");
                if (m_xDocumentProperties.is())
                {
                    uno::Reference<beans::XPropertyContainer> xUserDefinedProperties = m_xDocumentProperties->getUserDefinedProperties();
                    xUserDefinedProperties->addProperty(aName, beans::PropertyAttribute::REMOVABLE,
                            uno::makeAny(m_aStates.top().aDestinationText.makeStringAndClear()));
                }
            }
            break;
        case DESTINATION_OBJDATA:
            {
                m_pObjectData.reset(new SvMemoryStream());
                int b = 0, count = 2;

                // Feed the destination text to a stream.
                OString aStr = OUStringToOString(m_aStates.top().aDestinationText.makeStringAndClear(), RTL_TEXTENCODING_ASCII_US);
                const char *str = aStr.getStr();
                for (int i = 0; i < aStr.getLength(); ++i)
                {
                    char ch = str[i];
                    if (ch != 0x0d && ch != 0x0a)
                    {
                        b = b << 4;
                        sal_Int8 parsed = m_pTokenizer->asHex(ch);
                        if (parsed == -1)
                            return ERROR_HEX_INVALID;
                        b += parsed;
                        count--;
                        if (!count)
                        {
                            *m_pObjectData << (char)b;
                            count = 2;
                            b = 0;
                        }
                    }
                }

                if (m_pObjectData->Tell())
                {
                    m_pObjectData->Seek(0);

                    // Skip ObjectHeader
                    sal_uInt32 nData;
                    *m_pObjectData >> nData; // OLEVersion
                    *m_pObjectData >> nData; // FormatID
                    *m_pObjectData >> nData; // ClassName
                    m_pObjectData->SeekRel(nData);
                    *m_pObjectData >> nData; // TopicName
                    m_pObjectData->SeekRel(nData);
                    *m_pObjectData >> nData; // ItemName
                    m_pObjectData->SeekRel(nData);
                    *m_pObjectData >> nData; // NativeDataSize
                }

                uno::Reference<io::XInputStream> xInputStream(new utl::OInputStreamWrapper(m_pObjectData.get()));
                RTFValue::Pointer_t pStreamValue(new RTFValue(xInputStream));

                RTFSprms aOLEAttributes;
                aOLEAttributes.set(NS_ooxml::LN_inputstream, pStreamValue);
                RTFValue::Pointer_t pValue(new RTFValue(aOLEAttributes));
                m_aObjectSprms.set(NS_ooxml::LN_OLEObject_OLEObject, pValue);
            }
            break;
        case DESTINATION_OBJECT:
            {
                if (!m_bObject)
                {
                    // if the object is in a special container we will use the \result
                    // element instead of the \objdata
                    // (see RTF_OBJECT in RTFDocumentImpl::dispatchDestination)
                    break;
                }

                RTFSprms aObjAttributes;
                RTFSprms aObjSprms;
                RTFValue::Pointer_t pValue(new RTFValue(m_aObjectAttributes, m_aObjectSprms));
                aObjSprms.set(NS_ooxml::LN_object, pValue);
                writerfilter::Reference<Properties>::Pointer_t const pProperties(new RTFReferenceProperties(aObjAttributes, aObjSprms));
                uno::Reference<drawing::XShape> xShape;
                RTFValue::Pointer_t pShape = m_aObjectAttributes.find(NS_ooxml::LN_shape);
                OSL_ASSERT(pShape.get());
                if (pShape.get())
                    pShape->getAny() >>= xShape;
                Mapper().startShape(xShape);
                Mapper().props(pProperties);
                Mapper().endShape();
                m_aObjectAttributes.clear();
                m_aObjectSprms.clear();
                m_bObject = false;
            }
            break;
        case DESTINATION_ANNOTATIONDATE:
            {
                OUString aStr(OStringToOUString(lcl_DTTM22OString(m_aStates.top().aDestinationText.makeStringAndClear().toInt32()),
                            aState.nCurrentEncoding));
                RTFValue::Pointer_t pValue(new RTFValue(aStr));
                RTFSprms aAnnAttributes;
                aAnnAttributes.set(NS_ooxml::LN_CT_TrackChange_date, pValue);
                writerfilter::Reference<Properties>::Pointer_t const pProperties(new RTFReferenceProperties(aAnnAttributes));
                Mapper().props(pProperties);
            }
            break;
        case DESTINATION_ANNOTATIONAUTHOR:
            m_aAuthor = m_aStates.top().aDestinationText.makeStringAndClear();
            break;
        case DESTINATION_ATNID:
            m_aAuthorInitials = m_aStates.top().aDestinationText.makeStringAndClear();
            break;
        case DESTINATION_FALT:
            {
                OUString aStr(m_aStates.top().aDestinationText.makeStringAndClear());
                RTFValue::Pointer_t pValue(new RTFValue(aStr));
                aState.aTableSprms.set(NS_ooxml::LN_CT_Font_altName, pValue);
            }
            break;
        case DESTINATION_DRAWINGOBJECT:
            if (m_aStates.top().aDrawingObject.xShape.is())
            {
                RTFDrawingObject& rDrawing = m_aStates.top().aDrawingObject;
                uno::Reference<drawing::XShape> xShape(rDrawing.xShape);
                uno::Reference<beans::XPropertySet> xPropertySet(rDrawing.xPropertySet);

                uno::Reference<lang::XServiceInfo> xServiceInfo(xShape, uno::UNO_QUERY);
                bool bTextFrame = xServiceInfo->supportsService("com.sun.star.text.TextFrame");

                if (bTextFrame)
                {
                    xPropertySet->setPropertyValue("HoriOrientPosition", uno::makeAny((sal_Int32)rDrawing.nLeft));
                    xPropertySet->setPropertyValue("VertOrientPosition", uno::makeAny((sal_Int32)rDrawing.nTop));
                }
                else
                {
                    xShape->setPosition(awt::Point(rDrawing.nLeft, rDrawing.nTop));
                }
                xShape->setSize(awt::Size(rDrawing.nRight, rDrawing.nBottom));

                if (rDrawing.bHasLineColor)
                    xPropertySet->setPropertyValue("LineColor", uno::makeAny(sal_uInt32((rDrawing.nLineColorR<<16) + (rDrawing.nLineColorG<<8) + rDrawing.nLineColorB)));
                if (rDrawing.bHasFillColor)
                    xPropertySet->setPropertyValue("FillColor", uno::makeAny(sal_uInt32((rDrawing.nFillColorR<<16) + (rDrawing.nFillColorG<<8) + rDrawing.nFillColorB)));
                else if (!bTextFrame)
                    // If there is no fill, the Word default is 100% transparency.
                    xPropertySet->setPropertyValue("FillTransparence", uno::makeAny(sal_Int32(100)));

                m_pSdrImport->resolveFLine(xPropertySet, rDrawing.nFLine);

                if (!m_aStates.top().aDrawingObject.bHadShapeText)
                {
                    Mapper().startShape(xShape);
                }
                Mapper().endShape();
            }
            break;
        case DESTINATION_SHAPE:
            m_bNeedCr = m_bNeedCrOrig;
            if (aState.aFrame.inFrame())
            {
                // parBreak modify m_aStates.top() so we can't apply resetFrame directly on aState
                m_aStates.top().resetFrame();
                parBreak();
                // Save this state for later use, so we only reset frame status only for the first shape inside a frame.
                aState = m_aStates.top();
                m_bNeedPap = true;
            }
            break;
        case DESTINATION_MOMATH:
            {
                m_aMathBuffer.appendClosingTag(M_TOKEN(oMath));

                SvGlobalName aGlobalName(SO3_SM_CLASSID);
                comphelper::EmbeddedObjectContainer aContainer;
                OUString aName;
                uno::Reference<embed::XEmbeddedObject> xObject = aContainer.CreateEmbeddedObject(aGlobalName.GetByteSequence(), aName);
                uno::Reference<util::XCloseable> xComponent(xObject->getComponent(), uno::UNO_QUERY);
                // gcc4.4 (and 4.3 and possibly older) have a problem with dynamic_cast directly to the target class,
                // so help it with an intermediate cast. I'm not sure what exactly the problem is, seems to be unrelated
                // to RTLD_GLOBAL, so most probably a gcc bug.
                oox::FormulaImportBase* pImport = dynamic_cast<oox::FormulaImportBase*>(dynamic_cast<SfxBaseModel*>(xComponent.get()));
                assert( pImport != NULL );
                pImport->readFormulaOoxml(m_aMathBuffer);
                RTFValue::Pointer_t pValue(new RTFValue(xObject));
                RTFSprms aMathAttributes;
                aMathAttributes.set(NS_ooxml::LN_starmath, pValue);
                writerfilter::Reference<Properties>::Pointer_t const pProperties(new RTFReferenceProperties(aMathAttributes));
                Mapper().props(pProperties);
                m_aMathBuffer = oox::formulaimport::XmlStreamBuilder();
            }
            break;
        case DESTINATION_MR:
            lcl_DestinationToMath(m_aStates.top().aDestinationText, m_aMathBuffer, m_bMathNor);
            break;
        case DESTINATION_MF:
            m_aMathBuffer.appendClosingTag(M_TOKEN(f));
            break;
        case DESTINATION_MFPR:
            m_aMathBuffer.appendClosingTag(M_TOKEN(fPr));
            break;
        case DESTINATION_MCTRLPR:
            m_aMathBuffer.appendClosingTag(M_TOKEN(ctrlPr));
            break;
        case DESTINATION_MNUM:
            m_aMathBuffer.appendClosingTag(M_TOKEN(num));
            break;
        case DESTINATION_MDEN:
            m_aMathBuffer.appendClosingTag(M_TOKEN(den));
            break;
        case DESTINATION_MACC:
            m_aMathBuffer.appendClosingTag(M_TOKEN(acc));
            break;
        case DESTINATION_MACCPR:
            m_aMathBuffer.appendClosingTag(M_TOKEN(accPr));
            break;
        case DESTINATION_MCHR:
        case DESTINATION_MPOS:
        case DESTINATION_MVERTJC:
        case DESTINATION_MSTRIKEH:
        case DESTINATION_MDEGHIDE:
        case DESTINATION_MBEGCHR:
        case DESTINATION_MSEPCHR:
        case DESTINATION_MENDCHR:
        case DESTINATION_MSUBHIDE:
        case DESTINATION_MSUPHIDE:
        case DESTINATION_MTYPE:
        case DESTINATION_MGROW:
            {
                sal_Int32 nMathToken = 0;
                switch (aState.nDestinationState)
                {
                    case DESTINATION_MCHR:
                        nMathToken = M_TOKEN(chr);
                        break;
                    case DESTINATION_MPOS:
                        nMathToken = M_TOKEN(pos);
                        break;
                    case DESTINATION_MVERTJC:
                        nMathToken = M_TOKEN(vertJc);
                        break;
                    case DESTINATION_MSTRIKEH:
                        nMathToken = M_TOKEN(strikeH);
                        break;
                    case DESTINATION_MDEGHIDE:
                        nMathToken = M_TOKEN(degHide);
                        break;
                    case DESTINATION_MBEGCHR:
                        nMathToken = M_TOKEN(begChr);
                        break;
                    case DESTINATION_MSEPCHR:
                        nMathToken = M_TOKEN(sepChr);
                        break;
                    case DESTINATION_MENDCHR:
                        nMathToken = M_TOKEN(endChr);
                        break;
                    case DESTINATION_MSUBHIDE:
                        nMathToken = M_TOKEN(subHide);
                        break;
                    case DESTINATION_MSUPHIDE:
                        nMathToken = M_TOKEN(supHide);
                        break;
                    case DESTINATION_MTYPE:
                        nMathToken = M_TOKEN(type);
                        break;
                    case DESTINATION_MGROW:
                        nMathToken = M_TOKEN(grow);
                        break;
                    default:
                        break;
                }

                oox::formulaimport::XmlStream::AttributeList aAttribs;
                aAttribs[M_TOKEN(val)] = m_aStates.top().aDestinationText.makeStringAndClear();
                m_aMathBuffer.appendOpeningTag(nMathToken, aAttribs);
                m_aMathBuffer.appendClosingTag(nMathToken);
            }
            break;
        case DESTINATION_ME:
            m_aMathBuffer.appendClosingTag(M_TOKEN(e));
            break;
        case DESTINATION_MBAR:
            m_aMathBuffer.appendClosingTag(M_TOKEN(bar));
            break;
        case DESTINATION_MBARPR:
            m_aMathBuffer.appendClosingTag(M_TOKEN(barPr));
            break;
        case DESTINATION_MD:
            m_aMathBuffer.appendClosingTag(M_TOKEN(d));
            break;
        case DESTINATION_MDPR:
            m_aMathBuffer.appendClosingTag(M_TOKEN(dPr));
            break;
        case DESTINATION_MFUNC:
            m_aMathBuffer.appendClosingTag(M_TOKEN(func));
            break;
        case DESTINATION_MFUNCPR:
            m_aMathBuffer.appendClosingTag(M_TOKEN(funcPr));
            break;
        case DESTINATION_MFNAME:
            m_aMathBuffer.appendClosingTag(M_TOKEN(fName));
            break;
        case DESTINATION_MLIMLOW:
            m_aMathBuffer.appendClosingTag(M_TOKEN(limLow));
            break;
        case DESTINATION_MLIMLOWPR:
            m_aMathBuffer.appendClosingTag(M_TOKEN(limLowPr));
            break;
        case DESTINATION_MLIM:
            m_aMathBuffer.appendClosingTag(M_TOKEN(lim));
            break;
        case DESTINATION_MM:
            m_aMathBuffer.appendClosingTag(M_TOKEN(m));
            break;
        case DESTINATION_MMPR:
            m_aMathBuffer.appendClosingTag(M_TOKEN(mPr));
            break;
        case DESTINATION_MMR:
            m_aMathBuffer.appendClosingTag(M_TOKEN(mr));
            break;
        case DESTINATION_MNARY:
            m_aMathBuffer.appendClosingTag(M_TOKEN(nary));
            break;
        case DESTINATION_MNARYPR:
            m_aMathBuffer.appendClosingTag(M_TOKEN(naryPr));
            break;
        case DESTINATION_MSUB:
            m_aMathBuffer.appendClosingTag(M_TOKEN(sub));
            break;
        case DESTINATION_MSUP:
            m_aMathBuffer.appendClosingTag(M_TOKEN(sup));
            break;
        case DESTINATION_MLIMUPP:
            m_aMathBuffer.appendClosingTag(M_TOKEN(limUpp));
            break;
        case DESTINATION_MLIMUPPPR:
            m_aMathBuffer.appendClosingTag(M_TOKEN(limUppPr));
            break;
        case DESTINATION_MGROUPCHR:
            m_aMathBuffer.appendClosingTag(M_TOKEN(groupChr));
            break;
        case DESTINATION_MGROUPCHRPR:
            m_aMathBuffer.appendClosingTag(M_TOKEN(groupChrPr));
            break;
        case DESTINATION_MBORDERBOX:
            m_aMathBuffer.appendClosingTag(M_TOKEN(borderBox));
            break;
        case DESTINATION_MBORDERBOXPR:
            m_aMathBuffer.appendClosingTag(M_TOKEN(borderBoxPr));
            break;
        case DESTINATION_MRAD:
            m_aMathBuffer.appendClosingTag(M_TOKEN(rad));
            break;
        case DESTINATION_MRADPR:
            m_aMathBuffer.appendClosingTag(M_TOKEN(radPr));
            break;
        case DESTINATION_MDEG:
            m_aMathBuffer.appendClosingTag(M_TOKEN(deg));
            break;
        case DESTINATION_MSSUB:
            m_aMathBuffer.appendClosingTag(M_TOKEN(sSub));
            break;
        case DESTINATION_MSSUBPR:
            m_aMathBuffer.appendClosingTag(M_TOKEN(sSubPr));
            break;
        case DESTINATION_MSSUP:
            m_aMathBuffer.appendClosingTag(M_TOKEN(sSup));
            break;
        case DESTINATION_MSSUPPR:
            m_aMathBuffer.appendClosingTag(M_TOKEN(sSupPr));
            break;
        case DESTINATION_MSSUBSUP:
            m_aMathBuffer.appendClosingTag(M_TOKEN(sSubSup));
            break;
        case DESTINATION_MSSUBSUPPR:
            m_aMathBuffer.appendClosingTag(M_TOKEN(sSubSupPr));
            break;
        case DESTINATION_MSPRE:
            m_aMathBuffer.appendClosingTag(M_TOKEN(sPre));
            break;
        case DESTINATION_MSPREPR:
            m_aMathBuffer.appendClosingTag(M_TOKEN(sPrePr));
            break;
        case DESTINATION_MBOX:
            m_aMathBuffer.appendClosingTag(M_TOKEN(box));
            break;
        case DESTINATION_MEQARR:
            m_aMathBuffer.appendClosingTag(M_TOKEN(eqArr));
            break;
        case DESTINATION_SHAPEGROUP:
            if (aState.bCreatedShapeGroup)
                m_pSdrImport->popParent();
            break;
        default:
            break;
    }

    // See if we need to end a track change
    RTFValue::Pointer_t pTrackchange = aState.aCharacterSprms.find(NS_ooxml::LN_trackchange);
    if (pTrackchange.get())
    {
        RTFSprms aTCAttributes;
        RTFValue::Pointer_t pValue(new RTFValue(0));
        aTCAttributes.set(NS_ooxml::LN_endtrackchange, pValue);
        writerfilter::Reference<Properties>::Pointer_t const pProperties(new RTFReferenceProperties(aTCAttributes));
        Mapper().props(pProperties);
    }

    // This is the end of the doc, see if we need to close the last section.
    if (m_pTokenizer->getGroup() == 1 && !m_bFirstRun)
    {
        if (m_bNeedCr)
            dispatchSymbol(RTF_PAR);
        sectBreak(true);
    }

    m_aStates.pop();

    m_pTokenizer->popGroup();

    // list table
    switch (aState.nDestinationState)
    {
        case DESTINATION_LISTENTRY:
            {
                RTFValue::Pointer_t pValue(new RTFValue(aState.aTableAttributes, aState.aTableSprms));
                m_aListTableSprms.set(NS_ooxml::LN_CT_Numbering_abstractNum, pValue, false);
            }
            break;
        case DESTINATION_PARAGRAPHNUMBERING:
            {
                RTFValue::Pointer_t pIdValue = aState.aTableAttributes.find(NS_rtf::LN_LSID);
                if (pIdValue.get())
                {
                    // Abstract numbering
                    RTFSprms aLeveltextAttributes;
                    OUString aTextValue;
                    RTFValue::Pointer_t pTextBefore = aState.aTableAttributes.find(NS_ooxml::LN_CT_LevelText_val);
                    if (pTextBefore.get())
                        aTextValue += pTextBefore->getString();
                    aTextValue += "%1";
                    RTFValue::Pointer_t pTextAfter = aState.aTableAttributes.find(NS_ooxml::LN_CT_LevelSuffix_val);
                    if (pTextAfter.get())
                        aTextValue += pTextAfter->getString();
                    RTFValue::Pointer_t pTextValue(new RTFValue(aTextValue));
                    aLeveltextAttributes.set(NS_ooxml::LN_CT_LevelText_val, pTextValue);

                    RTFSprms aLevelAttributes;
                    RTFSprms aLevelSprms;
                    RTFValue::Pointer_t pIlvlValue(new RTFValue(0));
                    aLevelAttributes.set(NS_ooxml::LN_CT_Lvl_ilvl, pIlvlValue);

                    RTFValue::Pointer_t pNfcValue = aState.aTableSprms.find(NS_rtf::LN_NFC);
                    if (pNfcValue.get())
                        aLevelSprms.set(NS_rtf::LN_NFC, pNfcValue);

                    RTFValue::Pointer_t pStartatValue = aState.aTableSprms.find(NS_rtf::LN_ISTARTAT);
                    if (pStartatValue.get())
                        aLevelSprms.set(NS_rtf::LN_ISTARTAT, pStartatValue);

                    RTFValue::Pointer_t pLeveltextValue(new RTFValue(aLeveltextAttributes));
                    aLevelSprms.set(NS_ooxml::LN_CT_Lvl_lvlText, pLeveltextValue);
                    RTFValue::Pointer_t pRunProps = aState.aTableSprms.find(NS_ooxml::LN_CT_Lvl_rPr);
                    if (pRunProps.get())
                        aLevelSprms.set(NS_ooxml::LN_CT_Lvl_rPr, pRunProps);

                    RTFSprms aAbstractAttributes;
                    RTFSprms aAbstractSprms;
                    aAbstractAttributes.set(NS_ooxml::LN_CT_AbstractNum_abstractNumId, pIdValue);
                    RTFValue::Pointer_t pLevelValue(new RTFValue(aLevelAttributes, aLevelSprms));
                    aAbstractSprms.set(NS_ooxml::LN_CT_AbstractNum_lvl, pLevelValue, false);

                    RTFSprms aListTableSprms;
                    RTFValue::Pointer_t pAbstractValue(new RTFValue(aAbstractAttributes, aAbstractSprms));
                    // It's important that Numbering_abstractNum and Numbering_num never overwrites previous values.
                    aListTableSprms.set(NS_ooxml::LN_CT_Numbering_abstractNum, pAbstractValue, false);

                    // Numbering
                    RTFSprms aNumberingAttributes;
                    RTFSprms aNumberingSprms;
                    aNumberingAttributes.set(NS_rtf::LN_LSID, pIdValue);
                    aNumberingSprms.set(NS_ooxml::LN_CT_Num_abstractNumId, pIdValue);
                    RTFValue::Pointer_t pNumberingValue(new RTFValue(aNumberingAttributes, aNumberingSprms));
                    aListTableSprms.set(NS_ooxml::LN_CT_Numbering_num, pNumberingValue, false);

                    // Table
                    RTFSprms aListTableAttributes;
                    writerfilter::Reference<Properties>::Pointer_t const pProp(new RTFReferenceProperties(aListTableAttributes, aListTableSprms));

                    RTFReferenceTable::Entries_t aListTableEntries;
                    aListTableEntries.insert(make_pair(0, pProp));
                    writerfilter::Reference<Table>::Pointer_t const pTable(new RTFReferenceTable(aListTableEntries));
                    Mapper().table(NS_rtf::LN_LISTTABLE, pTable);

                    // Use it
                    lcl_putNestedSprm(m_aStates.top().aParagraphSprms, NS_ooxml::LN_CT_PPrBase_numPr, NS_sprm::LN_PIlvl, pIlvlValue);
                    lcl_putNestedSprm(m_aStates.top().aParagraphSprms, NS_ooxml::LN_CT_PPrBase_tabs, NS_sprm::LN_PIlfo, pIdValue);
                }
            }
            break;
        case DESTINATION_PARAGRAPHNUMBERING_TEXTAFTER:
            {
                RTFValue::Pointer_t pValue(new RTFValue(aState.aDestinationText.makeStringAndClear(), true));
                m_aStates.top().aTableAttributes.set(NS_ooxml::LN_CT_LevelSuffix_val, pValue);
            }
            break;
        case DESTINATION_PARAGRAPHNUMBERING_TEXTBEFORE:
            {
                RTFValue::Pointer_t pValue(new RTFValue(aState.aDestinationText.makeStringAndClear(), true));
                m_aStates.top().aTableAttributes.set(NS_ooxml::LN_CT_LevelText_val, pValue);
            }
            break;
        case DESTINATION_LISTLEVEL:
            {
                RTFValue::Pointer_t pInnerValue(new RTFValue(m_aStates.top().nListLevelNum++));
                aState.aTableAttributes.set(NS_ooxml::LN_CT_Lvl_ilvl, pInnerValue);

                RTFValue::Pointer_t pValue(new RTFValue(aState.aTableAttributes, aState.aTableSprms));
                if (m_aStates.top().nDestinationState != DESTINATION_LFOLEVEL)
                    m_aStates.top().aListLevelEntries.set(NS_ooxml::LN_CT_AbstractNum_lvl, pValue, false);
                else
                    m_aStates.top().aTableSprms.set(NS_ooxml::LN_CT_NumLvl_lvl, pValue);
            }
            break;
        case DESTINATION_LFOLEVEL:
            {
                RTFValue::Pointer_t pInnerValue(new RTFValue(m_aStates.top().nListLevelNum++));
                aState.aTableAttributes.set(NS_ooxml::LN_CT_NumLvl_ilvl, pInnerValue);

                RTFValue::Pointer_t pValue(new RTFValue(aState.aTableAttributes, aState.aTableSprms));
                m_aStates.top().aTableSprms.set(NS_ooxml::LN_CT_Num_lvlOverride, pValue);
            }
            break;
            // list override table
        case DESTINATION_LISTOVERRIDEENTRY:
            {
                if (m_aStates.top().nDestinationState == DESTINATION_LISTOVERRIDEENTRY)
                {   // copy properties upwards so upper popState inserts it
                    m_aStates.top().aTableAttributes = aState.aTableAttributes;
                    m_aStates.top().aTableSprms = aState.aTableSprms;
                }
                else
                {
                    RTFValue::Pointer_t pValue(new RTFValue(
                                aState.aTableAttributes, aState.aTableSprms));
                    m_aListTableSprms.set(NS_ooxml::LN_CT_Numbering_num,
                            pValue, false);
                }
            }
            break;
        case DESTINATION_LEVELTEXT:
            {
                RTFValue::Pointer_t pValue(new RTFValue(aState.aTableAttributes));
                m_aStates.top().aTableSprms.set(NS_ooxml::LN_CT_Lvl_lvlText, pValue);
            }
            break;
        case DESTINATION_LEVELNUMBERS:
            m_aStates.top().aTableSprms = aState.aTableSprms;
            break;
        case DESTINATION_FIELDINSTRUCTION:
            m_aStates.top().nFieldStatus = FIELD_INSTRUCTION;
            break;
        case DESTINATION_FIELDRESULT:
            m_aStates.top().nFieldStatus = FIELD_RESULT;
            break;
        case DESTINATION_FIELD:
            if (aState.nFieldStatus == FIELD_INSTRUCTION)
                singleChar(0x15);
            break;
        case DESTINATION_SHAPEPROPERTYVALUEPICT:
            {
                m_aStates.top().aPicture = aState.aPicture;
                m_aStates.top().aDestinationText = aState.aDestinationText;
            }
            break;
        case DESTINATION_FALT:
            m_aStates.top().aTableSprms = aState.aTableSprms;
            break;
        case DESTINATION_SHAPEPROPERTYNAME:
        case DESTINATION_SHAPEPROPERTYVALUE:
        case DESTINATION_SHAPEPROPERTY:
            {
                m_aStates.top().aShape = aState.aShape;
                m_aStates.top().aPicture = aState.aPicture;
                m_aStates.top().aCharacterAttributes = aState.aCharacterAttributes;
            }
            break;
        case DESTINATION_FLYMAINCONTENT:
        case DESTINATION_SHPPICT:
        case DESTINATION_SHAPE:
            m_aStates.top().aFrame = aState.aFrame;
            if (aState.nDestinationState == DESTINATION_SHPPICT && !m_aStates.empty() && m_aStates.top().nDestinationState == DESTINATION_LISTPICTURE)
            {
                RTFSprms aAttributes;
                aAttributes.set(NS_ooxml::LN_CT_NumPicBullet_numPicBulletId, RTFValue::Pointer_t(new RTFValue(m_nListPictureId++)));
                RTFSprms aSprms;
                // Dummy value, real picture is already sent to dmapper.
                aSprms.set(NS_ooxml::LN_CT_NumPicBullet_pict, RTFValue::Pointer_t(new RTFValue(0)));
                RTFValue::Pointer_t pValue(new RTFValue(aAttributes, aSprms));
                m_aListTableSprms.set(NS_ooxml::LN_CT_Numbering_numPicBullet, pValue, false);
            }
            break;
        case DESTINATION_TITLE:
            {
                if (m_aStates.top().nDestinationState == DESTINATION_TITLE)
                    // The parent is a title as well, just append what we have so far.
                    m_aStates.top().aDestinationText.append(aState.aDestinationText.makeStringAndClear());
                else if (m_xDocumentProperties.is())
                    m_xDocumentProperties->setTitle(aState.aDestinationText.makeStringAndClear());
            }
            break;
        case DESTINATION_SHAPETEXT:
            // If we're leaving the shapetext group (it may have nested ones) and this is a shape, not an old drawingobject.
            if (m_aStates.top().nDestinationState != DESTINATION_SHAPETEXT && !m_aStates.top().aDrawingObject.bHadShapeText)
            {
                m_aStates.top().bHadShapeText = true;
                if (!m_aStates.top().pCurrentBuffer)
                    m_pSdrImport->close();
                else
                    m_aStates.top().pCurrentBuffer->push_back(make_pair(BUFFER_ENDSHAPE, RTFValue::Pointer_t()));
            }
            break;
        default:
            {
                if (m_aStates.size() && m_aStates.top().nDestinationState == DESTINATION_PICT)
                    m_aStates.top().aPicture = aState.aPicture;
            }
            break;
    }

    if (aState.pCurrentBuffer == &m_aSuperBuffer)
    {
        OSL_ASSERT(!m_aStates.empty() && m_aStates.top().pCurrentBuffer == 0);

        if (!m_bHasFootnote)
            replayBuffer(m_aSuperBuffer);

        m_bHasFootnote = false;
    }

    if (!m_aStates.empty())
    {
        m_aStates.top().nCells = aState.nCells;
        m_aStates.top().aTableCellsSprms = aState.aTableCellsSprms;
        m_aStates.top().aTableCellsAttributes = aState.aTableCellsAttributes;
    }

    return 0;
}

::std::string RTFDocumentImpl::getType() const
{
    return "RTFDocumentImpl";
}

uno::Reference<lang::XMultiServiceFactory> RTFDocumentImpl::getModelFactory()
{
    return m_xModelFactory;
}

bool RTFDocumentImpl::isInBackground()
{
    return m_aStates.top().bInBackground;
}

RTFInternalState RTFDocumentImpl::getInternalState()
{
    return m_aStates.top().nInternalState;
}

void RTFDocumentImpl::setInternalState(RTFInternalState nInternalState)
{
    m_aStates.top().nInternalState = nInternalState;
}

RTFDestinationState RTFDocumentImpl::getDestinationState()
{
    return m_aStates.top().nDestinationState;
}

void RTFDocumentImpl::setDestinationState(RTFDestinationState nDestinationState)
{
    m_aStates.top().nDestinationState = nDestinationState;
}

void RTFDocumentImpl::setDestinationText(OUString& rString)
{
    m_aStates.top().aDestinationText.setLength(0);
    m_aStates.top().aDestinationText.append(rString);
}

bool RTFDocumentImpl::getSkipUnknown()
{
    return m_bSkipUnknown;
}

void RTFDocumentImpl::setSkipUnknown(bool bSkipUnknown)
{
    m_bSkipUnknown = bSkipUnknown;
}

void RTFDocumentImpl::checkUnicode(bool bUnicode, bool bHex)
{
    if (bUnicode && !m_aUnicodeBuffer.isEmpty())
    {
        OUString aString = m_aUnicodeBuffer.makeStringAndClear();
        text(aString);
    }
    if (bHex && !m_aHexBuffer.isEmpty())
    {
        OUString aString = OStringToOUString(m_aHexBuffer.makeStringAndClear(), m_aStates.top().nCurrentEncoding);
        text(aString);
    }
}

RTFParserState::RTFParserState(RTFDocumentImpl *pDocumentImpl)
    : m_pDocumentImpl(pDocumentImpl),
    nInternalState(INTERNAL_NORMAL),
    nDestinationState(DESTINATION_NORMAL),
    nFieldStatus(FIELD_NONE),
    nBorderState(BORDER_NONE),
    aTableSprms(),
    aTableAttributes(),
    aCharacterSprms(),
    aCharacterAttributes(),
    aParagraphSprms(),
    aParagraphAttributes(),
    aSectionSprms(),
    aSectionAttributes(),
    aTableRowSprms(),
    aTableRowAttributes(),
    aTableCellSprms(),
    aTableCellAttributes(),
    aTableCellsSprms(),
    aTableCellsAttributes(),
    aTabAttributes(),
    aCurrentColor(),
    nCurrentEncoding(rtl_getTextEncodingFromWindowsCharset(0)),
    nUc(1),
    nCharsToSkip(0),
    nBinaryToRead(0),
    nListLevelNum(0),
    aListLevelEntries(),
    aLevelNumbers(),
    aPicture(),
    aShape(),
    aDrawingObject(),
    aFrame(this),
    nCellX(0),
    nCells(0),
    nInheritingCells(0),
    bIsCjk(false),
    nYear(0),
    nMonth(0),
    nDay(0),
    nHour(0),
    nMinute(0),
    nCurrentStyleIndex(-1),
    pCurrentBuffer(0),
    bHasTableStyle(false),
    bInListpicture(false),
    bInBackground(false),
    bHadShapeText(false),
    bInShapeGroup(false),
    bCreatedShapeGroup(false)
{
}

void RTFParserState::resetFrame()
{
    aFrame = RTFFrame(this);
}

RTFColorTableEntry::RTFColorTableEntry()
    : nRed(0),
    nGreen(0),
    nBlue(0)
{
}

RTFPicture::RTFPicture()
    : nWidth(0),
    nHeight(0),
    nGoalWidth(0),
    nGoalHeight(0),
    nScaleX(100),
    nScaleY(100),
    nCropT(0),
    nCropB(0),
    nCropL(0),
    nCropR(0),
    eWMetafile(0),
    nStyle(BMPSTYLE_NONE)
{
}

RTFShape::RTFShape()
    : nLeft(0),
    nTop(0),
    nRight(0),
    nBottom(0),
    nHoriOrientRelation(0),
    nVertOrientRelation(0),
    nWrap(-1)
{
}

RTFDrawingObject::RTFDrawingObject()
    : nLineColorR(0),
    nLineColorG(0),
    nLineColorB(0),
    bHasLineColor(false),
    nFillColorR(0),
    nFillColorG(0),
    nFillColorB(0),
    bHasFillColor(false),
    nDhgt(0),
    nFLine(-1),
    nPolyLineCount(0),
    bHadShapeText(false)
{
}

RTFFrame::RTFFrame(RTFParserState* pParserState)
    : m_pParserState(pParserState),
    nX(0),
    nY(0),
    nW(0),
    nH(0),
    nHoriPadding(0),
    nVertPadding(0),
    nHoriAlign(0),
    nHoriAnchor(0),
    nVertAlign(0),
    nVertAnchor(0),
    nHRule(NS_ooxml::LN_Value_wordprocessingml_ST_HeightRule_auto),
    nAnchorType(0)
{
}

void RTFFrame::setSprm(Id nId, Id nValue)
{
    if (m_pParserState->m_pDocumentImpl->getFirstRun())
    {
        m_pParserState->m_pDocumentImpl->checkFirstRun();
        m_pParserState->m_pDocumentImpl->setNeedPar(false);
    }
    switch (nId)
    {
        case NS_sprm::LN_PDxaWidth:
            nW = nValue;
            break;
        case NS_sprm::LN_PWHeightAbs:
            nH = nValue;
            break;
        case NS_ooxml::LN_CT_FramePr_x:
            nX = nValue;
            break;
        case NS_ooxml::LN_CT_FramePr_y:
            nY = nValue;
            break;
        case NS_sprm::LN_PDxaFromText:
            nHoriPadding = nValue;
            break;
        case NS_sprm::LN_PDyaFromText:
            nVertPadding = nValue;
            break;
        case NS_ooxml::LN_CT_FramePr_xAlign:
            nHoriAlign = nValue;
            break;
        case NS_ooxml::LN_CT_FramePr_hAnchor:
            nHoriAnchor = nValue;
            break;
        case NS_ooxml::LN_CT_FramePr_yAlign:
            nVertAlign = nValue;
            break;
        case NS_ooxml::LN_CT_FramePr_vAnchor:
            nVertAnchor = nValue;
            break;
        case NS_sprm::LN_PWr:
            oWrap.reset(nValue);
            break;
        default:
            break;
    }
}

RTFSprms RTFFrame::getSprms()
{
    RTFSprms sprms;

    static const Id pNames[] =
    {
        NS_ooxml::LN_CT_FramePr_x,
        NS_ooxml::LN_CT_FramePr_y,
        NS_ooxml::LN_CT_FramePr_hRule, // Make sure nHRule is processed before nH
        NS_sprm::LN_PWHeightAbs,
        NS_sprm::LN_PDxaWidth,
        NS_sprm::LN_PDxaFromText,
        NS_sprm::LN_PDyaFromText,
        NS_ooxml::LN_CT_FramePr_hAnchor,
        NS_ooxml::LN_CT_FramePr_vAnchor,
        NS_ooxml::LN_CT_FramePr_xAlign,
        NS_ooxml::LN_CT_FramePr_yAlign,
        NS_sprm::LN_PWr,
        NS_ooxml::LN_CT_FramePr_dropCap,
        NS_ooxml::LN_CT_FramePr_lines
    };

    for ( int i = 0, len = SAL_N_ELEMENTS(pNames); i < len; ++i )
    {
        Id nId = pNames[i];
        RTFValue::Pointer_t pValue;

        switch ( nId )
        {
            case NS_ooxml::LN_CT_FramePr_x:
                if ( nX != 0 )
                    pValue.reset(new RTFValue(nX));
                break;
            case NS_ooxml::LN_CT_FramePr_y:
                if ( nY != 0 )
                    pValue.reset(new RTFValue(nY));
                break;
            case NS_sprm::LN_PWHeightAbs:
                if ( nH != 0 )
                {
                    if (nHRule == NS_ooxml::LN_Value_wordprocessingml_ST_HeightRule_exact)
                        pValue.reset(new RTFValue(-nH)); // The negative value just sets nHRule
                    else
                        pValue.reset(new RTFValue(nH));
                }
                break;
            case NS_sprm::LN_PDxaWidth:
                if ( nW != 0 )
                    pValue.reset(new RTFValue(nW));
                break;
            case NS_sprm::LN_PDxaFromText:
                if ( nHoriPadding != 0 )
                    pValue.reset(new RTFValue(nHoriPadding));
                break;
            case NS_sprm::LN_PDyaFromText:
                if ( nVertPadding != 0 )
                    pValue.reset(new RTFValue(nVertPadding));
                break;
            case NS_ooxml::LN_CT_FramePr_hAnchor:
                {
                    if ( nHoriAnchor == 0 )
                        nHoriAnchor = NS_ooxml::LN_Value_wordprocessingml_ST_HAnchor_margin;
                    pValue.reset(new RTFValue(nHoriAnchor));
                }
                break;
            case NS_ooxml::LN_CT_FramePr_vAnchor:
                {
                    if ( nVertAnchor == 0 )
                        nVertAnchor = NS_ooxml::LN_Value_wordprocessingml_ST_VAnchor_margin;
                    pValue.reset(new RTFValue(nVertAnchor));
                }
                break;
            case NS_ooxml::LN_CT_FramePr_xAlign:
                pValue.reset(new RTFValue(nHoriAlign));
                break;
            case NS_ooxml::LN_CT_FramePr_yAlign:
                pValue.reset(new RTFValue(nVertAlign));
                break;
            case NS_ooxml::LN_CT_FramePr_hRule:
                {
                    if ( nH < 0 )
                        nHRule = NS_ooxml::LN_Value_wordprocessingml_ST_HeightRule_exact;
                    else if ( nH > 0 )
                        nHRule = NS_ooxml::LN_Value_wordprocessingml_ST_HeightRule_atLeast;
                    pValue.reset(new RTFValue(nHRule));
                }
                break;
            case NS_sprm::LN_PWr:
                if (oWrap)
                    pValue.reset(new RTFValue(*oWrap));
                break;
            default:
                break;
        }

        if (pValue.get())
            sprms.set(nId, pValue);
    }

    RTFSprms frameprSprms;
    RTFValue::Pointer_t pFrameprValue(new RTFValue(sprms));
    frameprSprms.set(NS_ooxml::LN_CT_PPrBase_framePr, pFrameprValue);

    return frameprSprms;
}

bool RTFFrame::hasProperties()
{
    return nX != 0 || nY != 0 || nW != 0 || nH != 0 ||
        nHoriPadding != 0 || nVertPadding != 0 ||
        nHoriAlign != 0 || nHoriAnchor != 0 || nVertAlign != 0 || nVertAnchor != 0 ||
        nAnchorType != 0;
}

} // namespace rtftok
} // namespace writerfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
