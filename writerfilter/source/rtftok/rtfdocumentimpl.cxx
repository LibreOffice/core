/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/document/DocumentProperties.hpp>
#include <com/sun/star/drawing/XEnhancedCustomShapeDefaulter.hpp>
#include <com/sun/star/graphic/GraphicProvider.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <svl/lngmisc.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/streamwrap.hxx>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <vcl/wmf.hxx>
#include <vcl/layout.hxx>
#include <filter/msfilter/util.hxx>
#include <comphelper/string.hxx>
#include <svtools/grfmgr.hxx>
#include <tools/globname.hxx>
#include <tools/datetimeutils.hxx>
#include <comphelper/classids.hxx>
#include <comphelper/embeddedobjectcontainer.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <comphelper/sequence.hxx>
#include <sfx2/sfxbasemodel.hxx>
#include <sfx2/classificationhelper.hxx>
#include <sfx2/sfx.hrc>
#include <sfx2/sfxresid.hxx>
#include <oox/mathml/import.hxx>
#include <ooxml/resourceids.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/drawingml/drawingmltypes.hxx>
#include <rtl/uri.hxx>
#include <dmapper/DomainMapperFactory.hxx>
#include <rtfsdrimport.hxx>
#include <rtflookahead.hxx>
#include <rtfcharsets.hxx>
#include <rtfreferenceproperties.hxx>
#include <rtfskipdestination.hxx>
#include <rtffly.hxx>

using namespace com::sun::star;

namespace writerfilter
{
namespace rtftok
{

Id getParagraphBorder(sal_uInt32 nIndex)
{
    static const Id aBorderIds[] =
    {
        NS_ooxml::LN_CT_PBdr_top, NS_ooxml::LN_CT_PBdr_left, NS_ooxml::LN_CT_PBdr_bottom, NS_ooxml::LN_CT_PBdr_right
    };

    return aBorderIds[nIndex];
}

void putNestedAttribute(RTFSprms& rSprms, Id nParent, Id nId, const RTFValue::Pointer_t& pValue, RTFOverwrite eOverwrite, bool bAttribute)
{
    RTFValue::Pointer_t pParent = rSprms.find(nParent, /*bFirst=*/true, /*bForWrite=*/true);
    if (!pParent.get())
    {
        RTFSprms aAttributes;
        if (nParent == NS_ooxml::LN_CT_TcPrBase_shd)
        {
            // RTF default is 'auto', see writerfilter::dmapper::CellColorHandler
            aAttributes.set(NS_ooxml::LN_CT_Shd_color, std::make_shared<RTFValue>(0x0a));
            aAttributes.set(NS_ooxml::LN_CT_Shd_fill, std::make_shared<RTFValue>(0x0a));
        }
        auto pParentValue = std::make_shared<RTFValue>(aAttributes);
        rSprms.set(nParent, pParentValue, eOverwrite);
        pParent = pParentValue;
    }
    RTFSprms& rAttributes = (bAttribute ? pParent->getAttributes() : pParent->getSprms());
    rAttributes.set(nId, pValue, eOverwrite);
}

void putNestedSprm(RTFSprms& rSprms, Id nParent, Id nId, const RTFValue::Pointer_t& pValue)
{
    putNestedAttribute(rSprms, nParent, nId, pValue, RTFOverwrite::NO_APPEND, false);
}

static RTFValue::Pointer_t lcl_getNestedAttribute(RTFSprms& rSprms, Id nParent, Id nId)
{
    RTFValue::Pointer_t pParent = rSprms.find(nParent);
    if (!pParent)
        return RTFValue::Pointer_t();
    RTFSprms& rAttributes = pParent->getAttributes();
    return rAttributes.find(nId);
}

bool eraseNestedAttribute(RTFSprms& rSprms, Id nParent, Id nId)
{
    RTFValue::Pointer_t pParent = rSprms.find(nParent);
    if (!pParent.get())
        // It doesn't even have a parent, we're done.
        return false;
    RTFSprms& rAttributes = pParent->getAttributes();
    return rAttributes.erase(nId);
}

RTFSprms& getLastAttributes(RTFSprms& rSprms, Id nId)
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

void putBorderProperty(RTFStack& aStates, Id nId, const RTFValue::Pointer_t& pValue)
{
    RTFSprms* pAttributes = nullptr;
    if (aStates.top().nBorderState == RTFBorderState::PARAGRAPH_BOX)
        for (int i = 0; i < 4; i++)
        {
            RTFValue::Pointer_t p = aStates.top().aParagraphSprms.find(getParagraphBorder(i));
            if (p.get())
            {
                RTFSprms& rAttributes = p->getAttributes();
                rAttributes.set(nId, pValue);
            }
        }
    else if (aStates.top().nBorderState == RTFBorderState::CHARACTER)
    {
        RTFValue::Pointer_t pPointer = aStates.top().aCharacterSprms.find(NS_ooxml::LN_EG_RPrBase_bdr);
        if (pPointer.get())
        {
            RTFSprms& rAttributes = pPointer->getAttributes();
            rAttributes.set(nId, pValue);
        }
    }
    // Attributes of the last border type
    else if (aStates.top().nBorderState == RTFBorderState::PARAGRAPH)
        pAttributes = &getLastAttributes(aStates.top().aParagraphSprms, NS_ooxml::LN_CT_PrBase_pBdr);
    else if (aStates.top().nBorderState == RTFBorderState::CELL)
        pAttributes = &getLastAttributes(aStates.top().aTableCellSprms, NS_ooxml::LN_CT_TcPrBase_tcBorders);
    else if (aStates.top().nBorderState == RTFBorderState::PAGE)
        pAttributes = &getLastAttributes(aStates.top().aSectionSprms, NS_ooxml::LN_EG_SectPrContents_pgBorders);
    if (pAttributes)
        pAttributes->set(nId, pValue);
}

OString DTTM22OString(long nDTTM)
{
    return DateTimeToOString(msfilter::util::DTTM2DateTime(nDTTM));
}

static RTFSprms lcl_getBookmarkProperties(int nPos, OUString& rString)
{
    RTFSprms aAttributes;
    auto pPos = std::make_shared<RTFValue>(nPos);
    if (!rString.isEmpty())
    {
        // If present, this should be sent first.
        auto pString = std::make_shared<RTFValue>(rString);
        aAttributes.set(NS_ooxml::LN_CT_Bookmark_name, pString);
    }
    aAttributes.set(NS_ooxml::LN_CT_MarkupRangeBookmark_id, pPos);
    return aAttributes;
}

const char* keywordToString(RTFKeyword nKeyword)
{
    for (int i = 0; i < nRTFControlWords; i++)
    {
        if (nKeyword == aRTFControlWords[i].nIndex)
            return aRTFControlWords[i].sKeyword;
    }
    return nullptr;
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
                                 uno::Reference<frame::XFrame> const& xFrame,
                                 uno::Reference<task::XStatusIndicator> const& xStatusIndicator,
                                 const utl::MediaDescriptor& rMediaDescriptor)
    : m_xContext(xContext),
      m_xInputStream(xInputStream),
      m_xDstDoc(xDstDoc),
      m_xFrame(xFrame),
      m_xStatusIndicator(xStatusIndicator),
      m_pMapperStream(nullptr),
      m_aDefaultState(this),
      m_bSkipUnknown(false),
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
      m_nNestedCells(0),
      m_nTopLevelCells(0),
      m_nInheritingCells(0),
      m_nNestedCurrentCellX(0),
      m_nTopLevelCurrentCellX(0),
      m_nBackupTopLevelCurrentCellX(0),
      m_aTableBufferStack(1), // create top-level buffer already
      m_aSuperBuffer(),
      m_pSuperstream(nullptr),
      m_nStreamType(0),
      m_nHeaderFooterPositions(),
      m_nGroupStartPos(0),
      m_aBookmarks(),
      m_aAuthors(),
      m_aFormfieldSprms(),
      m_aFormfieldAttributes(),
      m_nFormFieldType(RTFFormFieldType::NONE),
      m_aOLEAttributes(),
      m_aObjectAttributes(),
      m_bObject(false),
      m_aFontTableEntries(),
      m_nCurrentFontIndex(0),
      m_nCurrentEncoding(-1),
      m_nDefaultFontIndex(-1),
      m_aStyleTableEntries(),
      m_nCurrentStyleIndex(0),
      m_bFormField(false),
      m_aUnicodeBuffer(),
      m_aHexBuffer(),
      m_bMathNor(false),
      m_bIgnoreNextContSectBreak(false),
      m_nResetBreakOnSectBreak(RTF_invalid),
      m_bNeedSect(false), // done by checkFirstRun
      m_bWasInFrame(false),
      m_bHadPicture(false),
      m_bHadSect(false),
      m_nCellxMax(0),
      m_nListPictureId(0),
      m_bIsNewDoc(!rMediaDescriptor.getUnpackedValueOrDefault("InsertMode", false)),
      m_rMediaDescriptor(rMediaDescriptor)
{
    OSL_ASSERT(xInputStream.is());
    m_pInStream.reset(utl::UcbStreamHelper::CreateStream(xInputStream, true));

    m_xModelFactory.set(m_xDstDoc, uno::UNO_QUERY);

    uno::Reference<document::XDocumentPropertiesSupplier> xDocumentPropertiesSupplier(m_xDstDoc, uno::UNO_QUERY);
    if (xDocumentPropertiesSupplier.is())
        m_xDocumentProperties.set(xDocumentPropertiesSupplier->getDocumentProperties(), uno::UNO_QUERY);

    m_pGraphicHelper.reset(new oox::GraphicHelper(m_xContext, xFrame, m_xStorage));

    m_pTokenizer.reset(new RTFTokenizer(*this, m_pInStream.get(), m_xStatusIndicator));
    m_pSdrImport.reset(new RTFSdrImport(*this, m_xDstDoc));
}

RTFDocumentImpl::~RTFDocumentImpl() = default;

SvStream& RTFDocumentImpl::Strm()
{
    return *m_pInStream;
}


void RTFDocumentImpl::setSuperstream(RTFDocumentImpl* pSuperstream)
{
    m_pSuperstream = pSuperstream;
}

void RTFDocumentImpl::setStreamType(Id nId)
{
    m_nStreamType = nId;
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
    return m_pSuperstream != nullptr;
}

void RTFDocumentImpl::finishSubstream()
{
    checkUnicode(/*bUnicode =*/ true, /*bHex =*/ true);
}

void RTFDocumentImpl::setIgnoreFirst(OUString& rIgnoreFirst)
{
    m_aIgnoreFirst = rIgnoreFirst;
}

void RTFDocumentImpl::resolveSubstream(sal_Size nPos, Id nId)
{
    OUString aStr;
    resolveSubstream(nPos, nId, aStr);
}
void RTFDocumentImpl::resolveSubstream(sal_Size nPos, Id nId, OUString& rIgnoreFirst)
{
    sal_Size nCurrent = Strm().Tell();
    // Seek to header position, parse, then seek back.
    auto pImpl = std::make_shared<RTFDocumentImpl>(m_xContext, m_xInputStream, m_xDstDoc, m_xFrame, m_xStatusIndicator, m_rMediaDescriptor);
    pImpl->setSuperstream(this);
    pImpl->setStreamType(nId);
    pImpl->setIgnoreFirst(rIgnoreFirst);
    if (!m_aAuthor.isEmpty())
    {
        pImpl->setAuthor(m_aAuthor);
        m_aAuthor.clear();
    }
    if (!m_aAuthorInitials.isEmpty())
    {
        pImpl->setAuthorInitials(m_aAuthorInitials);
        m_aAuthorInitials.clear();
    }
    pImpl->m_nDefaultFontIndex = m_nDefaultFontIndex;
    pImpl->seek(nPos);
    SAL_INFO("writerfilter", "substream start");
    Mapper().substream(nId, pImpl);
    SAL_INFO("writerfilter", "substream end");
    Strm().Seek(nCurrent);
}

void RTFDocumentImpl::checkFirstRun()
{
    if (m_bFirstRun)
    {
        // output settings table
        writerfilter::Reference<Properties>::Pointer_t pProp = std::make_shared<RTFReferenceProperties>(m_aSettingsTableAttributes, m_aSettingsTableSprms);
        RTFReferenceTable::Entries_t aSettingsTableEntries;
        aSettingsTableEntries.insert(std::make_pair(0, pProp));
        writerfilter::Reference<Table>::Pointer_t pTable = std::make_shared<RTFReferenceTable>(aSettingsTableEntries);
        Mapper().table(NS_ooxml::LN_settings_settings, pTable);
        // start initial paragraph
        m_bFirstRun = false;
        assert(!m_bNeedSect);
        setNeedSect(true); // first call that succeeds

        // set the requested default font, if there are none
        RTFValue::Pointer_t pFont = lcl_getNestedAttribute(m_aDefaultState.aCharacterSprms, NS_ooxml::LN_EG_RPrBase_rFonts, NS_ooxml::LN_CT_Fonts_ascii);
        RTFValue::Pointer_t pCurrentFont = lcl_getNestedAttribute(m_aStates.top().aCharacterSprms, NS_ooxml::LN_EG_RPrBase_rFonts, NS_ooxml::LN_CT_Fonts_ascii);
        if (pFont && !pCurrentFont)
            putNestedAttribute(m_aStates.top().aCharacterSprms, NS_ooxml::LN_EG_RPrBase_rFonts, NS_ooxml::LN_CT_Fonts_ascii, pFont);
    }
}


void RTFDocumentImpl::setNeedPar(bool bNeedPar)
{
    m_bNeedPar = bNeedPar;
}

void RTFDocumentImpl::setNeedSect(bool bNeedSect)
{
    // ignore setting before checkFirstRun - every keyword calls setNeedSect!
    if (!m_bNeedSect && bNeedSect && !m_bFirstRun)
    {
        if (!m_pSuperstream) // no sections in header/footer!
        {
            Mapper().startSectionGroup();
        }
        // set flag in substream too - otherwise multiple startParagraphGroup
        m_bNeedSect = bNeedSect;
        Mapper().startParagraphGroup();
        setNeedPar(true);
    }
    else if (m_bNeedSect && !bNeedSect)
    {
        m_bNeedSect = bNeedSect;
    }
}

/// Copy rProps to rStyleAttributes and rStyleSprms, but in case of nested sprms, copy their children as toplevel sprms/attributes.
static void lcl_copyFlatten(RTFReferenceProperties& rProps, RTFSprms& rStyleAttributes, RTFSprms& rStyleSprms)
{
    for (auto& rSprm : rProps.getSprms())
    {
        // createStyleProperties() puts properties to rPr, but here we need a flat list.
        if (rSprm.first == NS_ooxml::LN_CT_Style_rPr)
        {
            // rPr can have both attributes and SPRMs, copy over both types.
            RTFSprms& rRPrSprms = rSprm.second->getSprms();
            for (auto& rRPrSprm : rRPrSprms)
                rStyleSprms.set(rRPrSprm.first, rRPrSprm.second);

            RTFSprms& rRPrAttributes = rSprm.second->getAttributes();
            for (auto& rRPrAttribute : rRPrAttributes)
                rStyleAttributes.set(rRPrAttribute.first, rRPrAttribute.second);
        }
        else
            rStyleSprms.set(rSprm.first, rSprm.second);
    }

    RTFSprms& rAttributes = rProps.getAttributes();
    for (auto& rAttribute : rAttributes)
        rStyleAttributes.set(rAttribute.first, rAttribute.second);
}

writerfilter::Reference<Properties>::Pointer_t RTFDocumentImpl::getProperties(RTFSprms& rAttributes, RTFSprms& rSprms)
{
    int nStyle = 0;
    if (!m_aStates.empty())
        nStyle = m_aStates.top().nCurrentStyleIndex;
    auto it = m_aStyleTableEntries.find(nStyle);
    if (it != m_aStyleTableEntries.end())
    {
        RTFReferenceProperties& rProps = *static_cast<RTFReferenceProperties*>(it->second.get());

        // cloneAndDeduplicate() wants to know about only a single "style", so
        // let's merge paragraph and character style properties here.
        auto itChar = m_aStyleTableEntries.end();
        if (!m_aStates.empty())
        {
            int nCharStyle = m_aStates.top().nCurrentCharacterStyleIndex;
            itChar = m_aStyleTableEntries.find(nCharStyle);
        }

        RTFSprms aStyleSprms;
        RTFSprms aStyleAttributes;
        // Ensure the paragraph style is a flat list.
        lcl_copyFlatten(rProps, aStyleAttributes, aStyleSprms);

        if (itChar != m_aStyleTableEntries.end())
        {
            // Found active character style, then update aStyleSprms/Attributes.
            RTFReferenceProperties& rCharProps = *static_cast<RTFReferenceProperties*>(itChar->second.get());
            lcl_copyFlatten(rCharProps, aStyleAttributes, aStyleSprms);
        }

        // Get rid of direct formatting what is already in the style.
        RTFSprms const sprms(rSprms.cloneAndDeduplicate(aStyleSprms));
        RTFSprms const attributes(rAttributes.cloneAndDeduplicate(aStyleAttributes));
        return std::make_shared<RTFReferenceProperties>(attributes, sprms);
    }
    writerfilter::Reference<Properties>::Pointer_t pRet = std::make_shared<RTFReferenceProperties>(rAttributes, rSprms);
    return pRet;
}

void RTFDocumentImpl::checkNeedPap()
{
    if (m_bNeedPap)
    {
        m_bNeedPap = false; // reset early, so we can avoid recursion when calling ourselves

        if (m_aStates.empty())
            return;

        if (!m_aStates.top().pCurrentBuffer)
        {
            writerfilter::Reference<Properties>::Pointer_t const pParagraphProperties(
                getProperties(m_aStates.top().aParagraphAttributes, m_aStates.top().aParagraphSprms)
            );

            // Writer will ignore a page break before a text frame, so guard it with empty paragraphs
            bool hasBreakBeforeFrame = m_aStates.top().aFrame.hasProperties() &&
                                       m_aStates.top().aParagraphSprms.find(NS_ooxml::LN_CT_PPrBase_pageBreakBefore).get();
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
            auto pValue = std::make_shared<RTFValue>(m_aStates.top().aParagraphAttributes, m_aStates.top().aParagraphSprms);
            m_aStates.top().pCurrentBuffer->push_back(
                Buf_t(BUFFER_PROPS, pValue, nullptr));
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
        auto pValue = std::make_shared<RTFValue>(m_aStates.top().aCharacterAttributes, m_aStates.top().aCharacterSprms);
        m_aStates.top().pCurrentBuffer->push_back(Buf_t(BUFFER_PROPS, pValue, nullptr));
    }

    // Delete the sprm, so the trackchange range will be started only once.
    // OTOH set a boolean flag, so we'll know we need to end the range later.
    RTFValue::Pointer_t pTrackchange = m_aStates.top().aCharacterSprms.find(NS_ooxml::LN_trackchange);
    if (pTrackchange.get())
    {
        m_aStates.top().bStartedTrackchange = true;
        m_aStates.top().aCharacterSprms.erase(NS_ooxml::LN_trackchange);
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

void RTFDocumentImpl::sectBreak(bool bFinal)
{
    SAL_INFO("writerfilter", OSL_THIS_FUNC << ": final? " << bFinal << ", needed? " << m_bNeedSect);
    bool bNeedSect = m_bNeedSect;
    RTFValue::Pointer_t pBreak = m_aStates.top().aSectionSprms.find(NS_ooxml::LN_EG_SectPrContents_type);
    bool bContinuous = pBreak.get() && pBreak->getInt() == static_cast<sal_Int32>(NS_ooxml::LN_Value_ST_SectionMark_continuous);
    // If there is no paragraph in this section, then insert a dummy one, as required by Writer,
    // unless this is the end of the doc, we had nothing since the last section break and this is not a continuous one.
    // Also, when pasting, it's fine to not have any paragraph inside the document at all.
    if (m_bNeedPar && !(bFinal && !m_bNeedSect && !bContinuous) && !isSubstream() && m_bIsNewDoc)
        dispatchSymbol(RTF_PAR);
    // It's allowed to not have a non-table paragraph at the end of an RTF doc, add it now if required.
    if (m_bNeedFinalPar && bFinal)
    {
        dispatchFlag(RTF_PARD);
        dispatchSymbol(RTF_PAR);
        m_bNeedSect = bNeedSect;
    }
    while (!m_nHeaderFooterPositions.empty())
    {
        std::pair<Id, sal_Size> aPair = m_nHeaderFooterPositions.front();
        m_nHeaderFooterPositions.pop();
        resolveSubstream(aPair.second, aPair.first);
    }

    // Normally a section break at the end of the doc is necessary. Unless the
    // last control word in the document is a section break itself.
    if (!bNeedSect || !m_bHadSect)
    {
        // In case the last section is a continuous one, we don't need to output a section break.
        if (bFinal && bContinuous)
            m_aStates.top().aSectionSprms.erase(NS_ooxml::LN_EG_SectPrContents_type);
    }

    // Section properties are a paragraph sprm.
    auto pValue = std::make_shared<RTFValue>(m_aStates.top().aSectionAttributes, m_aStates.top().aSectionSprms);
    RTFSprms aAttributes;
    RTFSprms aSprms;
    aSprms.set(NS_ooxml::LN_CT_PPr_sectPr, pValue);
    writerfilter::Reference<Properties>::Pointer_t pProperties = std::make_shared<RTFReferenceProperties>(aAttributes, aSprms);

    if (bFinal && !m_pSuperstream)
        // This is the end of the document, not just the end of e.g. a header.
        // This makes sure that dmapper can set DontBalanceTextColumns=true for this section if necessary.
        Mapper().markLastSectionGroup();

    // The trick is that we send properties of the previous section right now, which will be exactly what dmapper expects.
    Mapper().props(pProperties);
    Mapper().endParagraphGroup();
    if (!m_pSuperstream)
        Mapper().endSectionGroup();
    m_bNeedPar = false;
    m_bNeedSect = false;
}

void RTFDocumentImpl::seek(sal_Size nPos)
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

rtl_TextEncoding RTFDocumentImpl::getEncoding(int nFontIndex)
{
    if (!m_pSuperstream)
    {
        auto it = m_aFontEncodings.find(nFontIndex);
        if (it != m_aFontEncodings.end())
            // We have a font encoding associated to this font.
            return it->second;
        else if (m_aDefaultState.nCurrentEncoding != rtl_getTextEncodingFromWindowsCharset(0))
            // We have a default encoding.
            return m_aDefaultState.nCurrentEncoding;
        else
            // Guess based on locale.
            return msfilter::util::getBestTextEncodingFromLocale(Application::GetSettings().GetLanguageTag().getLocale());
    }
    else
        return m_pSuperstream->getEncoding(nFontIndex);
}

OUString RTFDocumentImpl::getFontName(int nIndex)
{
    if (!m_pSuperstream)
        return m_aFontNames[nIndex];
    else
        return m_pSuperstream->getFontName(nIndex);
}

int RTFDocumentImpl::getFontIndex(int nIndex)
{
    if (!m_pSuperstream)
        return std::find(m_aFontIndexes.begin(), m_aFontIndexes.end(), nIndex) - m_aFontIndexes.begin();
    else
        return m_pSuperstream->getFontIndex(nIndex);
}

OUString RTFDocumentImpl::getStyleName(int nIndex)
{
    if (!m_pSuperstream)
    {
        OUString aRet;
        if (m_aStyleNames.find(nIndex) != m_aStyleNames.end())
            aRet = m_aStyleNames[nIndex];
        return aRet;
    }
    else
        return m_pSuperstream->getStyleName(nIndex);
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

bool RTFDocumentImpl::isStyleSheetImport()
{
    if (m_aStates.empty())
        return false;
    Destination eDestination = m_aStates.top().eDestination;
    return eDestination == Destination::STYLESHEET || eDestination == Destination::STYLEENTRY;
}

void RTFDocumentImpl::resolve(Stream& rMapper)
{
    m_pMapperStream = &rMapper;
    switch (m_pTokenizer->resolveParse())
    {
    case RTFError::OK:
        SAL_INFO("writerfilter", "RTFDocumentImpl::resolve: finished without errors");
        break;
    case RTFError::GROUP_UNDER:
        SAL_INFO("writerfilter", "RTFDocumentImpl::resolve: unmatched '}'");
        break;
    case RTFError::GROUP_OVER:
        SAL_INFO("writerfilter", "RTFDocumentImpl::resolve: unmatched '{'");
        throw io::WrongFormatException(m_pTokenizer->getPosition());
        break;
    case RTFError::UNEXPECTED_EOF:
        SAL_INFO("writerfilter", "RTFDocumentImpl::resolve: unexpected end of file");
        throw io::WrongFormatException(m_pTokenizer->getPosition());
        break;
    case RTFError::HEX_INVALID:
        SAL_INFO("writerfilter", "RTFDocumentImpl::resolve: invalid hex char");
        throw io::WrongFormatException(m_pTokenizer->getPosition());
        break;
    case RTFError::CHAR_OVER:
        SAL_INFO("writerfilter", "RTFDocumentImpl::resolve: characters after last '}'");
        break;
    case RTFError::CLASSIFICATION:
        SAL_INFO("writerfilter", "RTFDocumentImpl::resolve: classification prevented paste");
        break;
    }
}

void RTFDocumentImpl::resolvePict(bool const bInline, uno::Reference<drawing::XShape> const& i_xShape)
{
    SvMemoryStream aStream;
    SvStream* pStream = nullptr;
    if (!m_pBinaryData.get())
    {
        pStream = &aStream;
        int b = 0, count = 2;

        // Feed the destination text to a stream.
        OString aStr = OUStringToOString(m_aStates.top().aDestinationText.makeStringAndClear(), RTL_TEXTENCODING_ASCII_US);
        const char* str = aStr.getStr();
        for (int i = 0; i < aStr.getLength(); ++i)
        {
            char ch = str[i];
            if (ch != 0x0d && ch != 0x0a && ch != 0x20)
            {
                b = b << 4;
                sal_Int8 parsed = RTFTokenizer::asHex(ch);
                if (parsed == -1)
                    return;
                b += parsed;
                count--;
                if (!count)
                {
                    aStream.WriteChar((char)b);
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
        return;

    SvMemoryStream aDIBStream;
    if (m_aStates.top().aPicture.eStyle == RTFBmpStyle::DIBITMAP)
    {
        // Construct a BITMAPFILEHEADER structure before the real data.
        SvStream& rBodyStream = *pStream;
        aDIBStream.WriteChar('B');
        aDIBStream.WriteChar('M');
        // The size of the real data.
        aDIBStream.WriteUInt32(rBodyStream.Tell());
        // Reserved.
        aDIBStream.WriteUInt32(0);
        // The offset of the real data, i.e. the size of the header, including this number.
        aDIBStream.WriteUInt32(14);
        rBodyStream.Seek(0);
        aDIBStream.WriteStream(rBodyStream);
        pStream = &aDIBStream;
    }

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
        pExtHeader = nullptr;
    OUString aGraphicUrl = m_pGraphicHelper->importGraphicObject(xInputStream, pExtHeader);

    if (m_aStates.top().aPicture.eStyle != RTFBmpStyle::NONE)
    {
        // In case of PNG/JPEG, the real size is known, don't use the values
        // provided by picw and pich.
        OString aURLBS(OUStringToOString(aGraphicUrl, RTL_TEXTENCODING_UTF8));
        const char aURLBegin[] = "vnd.sun.star.GraphicObject:";
        if (aURLBS.startsWith(aURLBegin))
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
    uno::Reference<drawing::XShape> xShape(i_xShape);
    if (xShape.is())
    {
        uno::Reference<lang::XServiceInfo> xSI(xShape, uno::UNO_QUERY_THROW);
        if (!xSI->supportsService("com.sun.star.drawing.GraphicObjectShape"))
        {
            // it's sometimes an error to get here - but it's possible to have
            // a \pict inside the \shptxt of a \shp of shapeType 202 "TextBox"
            // and in that case xShape is the text frame; we actually need a
            // new GraphicObject then (example: fdo37691-1.rtf)
            SAL_INFO("writerfilter.rtf", "cannot set graphic on existing shape, creating a new GraphicObjectShape");
            xShape.clear();
        }
    }
    if (!xShape.is())
    {
        if (m_xModelFactory.is())
            xShape.set(m_xModelFactory->createInstance("com.sun.star.drawing.GraphicObjectShape"), uno::UNO_QUERY);
        uno::Reference<drawing::XDrawPageSupplier> const xDrawSupplier(m_xDstDoc, uno::UNO_QUERY);
        if (xDrawSupplier.is())
        {
            uno::Reference<drawing::XShapes> xShapes(xDrawSupplier->getDrawPage(), uno::UNO_QUERY);
            if (xShapes.is())
                xShapes->add(xShape);
        }
    }

    uno::Reference<beans::XPropertySet> xPropertySet(xShape, uno::UNO_QUERY);

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
        xShape->setSize(aSize);

        // Replacement graphic is inline by default, see oox::vml::SimpleShape::implConvertAndInsert().
        xPropertySet->setPropertyValue("AnchorType", uno::makeAny(text::TextContentAnchorType_AS_CHARACTER));

        auto pShapeValue = std::make_shared<RTFValue>(xShape);
        m_aObjectAttributes.set(NS_ooxml::LN_shape, pShapeValue);
        return;
    }

    if (xPropertySet.is())
        xPropertySet->setPropertyValue("GraphicURL", uno::Any(aGraphicUrl));

    if (m_aStates.top().bInListpicture)
    {
        // Send the shape directly, no section is started, to additional properties will be ignored anyway.
        Mapper().startShape(xShape);
        Mapper().endShape();
        return;
    }

    // Send it to the dmapper.
    RTFSprms aSprms;
    RTFSprms aAttributes;
    // shape attribute
    RTFSprms aPicAttributes;
    auto pShapeValue = std::make_shared<RTFValue>(xShape);
    aPicAttributes.set(NS_ooxml::LN_shape, pShapeValue);
    // pic sprm
    RTFSprms aGraphicDataAttributes;
    RTFSprms aGraphicDataSprms;
    auto pPicValue = std::make_shared<RTFValue>(aPicAttributes);
    aGraphicDataSprms.set(NS_ooxml::LN_pic_pic, pPicValue);
    // graphicData sprm
    RTFSprms aGraphicAttributes;
    RTFSprms aGraphicSprms;
    auto pGraphicDataValue = std::make_shared<RTFValue>(aGraphicDataAttributes, aGraphicDataSprms);
    aGraphicSprms.set(NS_ooxml::LN_CT_GraphicalObject_graphicData, pGraphicDataValue);
    // graphic sprm
    auto pGraphicValue = std::make_shared<RTFValue>(aGraphicAttributes, aGraphicSprms);
    // extent sprm
    RTFSprms aExtentAttributes;
    int nXExt, nYExt;
    nXExt = (m_aStates.top().aPicture.nGoalWidth ? m_aStates.top().aPicture.nGoalWidth : m_aStates.top().aPicture.nWidth);
    nYExt = (m_aStates.top().aPicture.nGoalHeight ? m_aStates.top().aPicture.nGoalHeight : m_aStates.top().aPicture.nHeight);
    if (m_aStates.top().aPicture.nScaleX != 100)
        nXExt = (((long)m_aStates.top().aPicture.nScaleX) * (nXExt - (m_aStates.top().aPicture.nCropL + m_aStates.top().aPicture.nCropR))) / 100L;
    if (m_aStates.top().aPicture.nScaleY != 100)
        nYExt = (((long)m_aStates.top().aPicture.nScaleY) * (nYExt - (m_aStates.top().aPicture.nCropT + m_aStates.top().aPicture.nCropB))) / 100L;
    if (m_aStates.top().bInShape)
    {
        // Picture in shape: it looks like pib picture, so we will stretch the picture to shape size (tdf#49893)
        nXExt = m_aStates.top().aShape.nRight - m_aStates.top().aShape.nLeft;
        nYExt = m_aStates.top().aShape.nBottom - m_aStates.top().aShape.nTop;
    }
    auto pXExtValue = std::make_shared<RTFValue>(oox::drawingml::convertHmmToEmu(nXExt));
    auto pYExtValue = std::make_shared<RTFValue>(oox::drawingml::convertHmmToEmu(nYExt));
    aExtentAttributes.set(NS_ooxml::LN_CT_PositiveSize2D_cx, pXExtValue);
    aExtentAttributes.set(NS_ooxml::LN_CT_PositiveSize2D_cy, pYExtValue);
    auto pExtentValue = std::make_shared<RTFValue>(aExtentAttributes);
    // docpr sprm
    RTFSprms aDocprAttributes;
    for (auto& rCharacterAttribute : m_aStates.top().aCharacterAttributes)
        if (rCharacterAttribute.first == NS_ooxml::LN_CT_NonVisualDrawingProps_name || rCharacterAttribute.first == NS_ooxml::LN_CT_NonVisualDrawingProps_descr)
            aDocprAttributes.set(rCharacterAttribute.first, rCharacterAttribute.second);
    auto pDocprValue = std::make_shared<RTFValue>(aDocprAttributes);
    if (bInline)
    {
        RTFSprms aInlineAttributes;
        aInlineAttributes.set(NS_ooxml::LN_CT_Inline_distT, std::make_shared<RTFValue>(0));
        aInlineAttributes.set(NS_ooxml::LN_CT_Inline_distB, std::make_shared<RTFValue>(0));
        aInlineAttributes.set(NS_ooxml::LN_CT_Inline_distL, std::make_shared<RTFValue>(0));
        aInlineAttributes.set(NS_ooxml::LN_CT_Inline_distR, std::make_shared<RTFValue>(0));
        RTFSprms aInlineSprms;
        aInlineSprms.set(NS_ooxml::LN_CT_Inline_extent, pExtentValue);
        aInlineSprms.set(NS_ooxml::LN_CT_Inline_docPr, pDocprValue);
        aInlineSprms.set(NS_ooxml::LN_graphic_graphic, pGraphicValue);
        // inline sprm
        auto pValue = std::make_shared<RTFValue>(aInlineAttributes, aInlineSprms);
        aSprms.set(NS_ooxml::LN_inline_inline, pValue);
    }
    else // anchored
    {
        // wrap sprm
        RTFSprms aAnchorWrapAttributes;
        m_aStates.top().aShape.aAnchorAttributes.set(NS_ooxml::LN_CT_Anchor_behindDoc, std::make_shared<RTFValue>((m_aStates.top().aShape.bInBackground) ? 1 : 0));
        RTFSprms aAnchorSprms;
        for (auto& rCharacterAttribute : m_aStates.top().aCharacterAttributes)
        {
            if (rCharacterAttribute.first == NS_ooxml::LN_CT_WrapSquare_wrapText)
                aAnchorWrapAttributes.set(rCharacterAttribute.first, rCharacterAttribute.second);
        }
        sal_Int32 nWrap = -1;
        for (auto& rCharacterSprm : m_aStates.top().aCharacterSprms)
        {
            if (rCharacterSprm.first == NS_ooxml::LN_EG_WrapType_wrapNone || rCharacterSprm.first == NS_ooxml::LN_EG_WrapType_wrapTight)
            {
                nWrap = rCharacterSprm.first;

                // If there is a wrap polygon prepared by RTFSdrImport, pick it up here.
                if (rCharacterSprm.first == NS_ooxml::LN_EG_WrapType_wrapTight && !m_aStates.top().aShape.aWrapPolygonSprms.empty())
                    rCharacterSprm.second->getSprms().set(NS_ooxml::LN_CT_WrapTight_wrapPolygon, std::make_shared<RTFValue>(RTFSprms(), m_aStates.top().aShape.aWrapPolygonSprms));

                aAnchorSprms.set(rCharacterSprm.first, rCharacterSprm.second);
            }
        }
        auto pAnchorWrapValue = std::make_shared<RTFValue>(aAnchorWrapAttributes);
        aAnchorSprms.set(NS_ooxml::LN_CT_Anchor_extent, pExtentValue);
        if (aAnchorWrapAttributes.size() && nWrap == -1)
            aAnchorSprms.set(NS_ooxml::LN_EG_WrapType_wrapSquare, pAnchorWrapValue);

        // See OOXMLFastContextHandler::positionOffset(), we can't just put offset values in an RTFValue.
        RTFSprms aPoshAttributes;
        RTFSprms aPoshSprms;
        if (m_aStates.top().aShape.nHoriOrientRelationToken > 0)
            aPoshAttributes.set(NS_ooxml::LN_CT_PosH_relativeFrom, std::make_shared<RTFValue>(m_aStates.top().aShape.nHoriOrientRelationToken));
        if (m_aStates.top().aShape.nLeft != 0)
        {
            Mapper().positionOffset(OUString::number(oox::drawingml::convertHmmToEmu(m_aStates.top().aShape.nLeft)), /*bVertical=*/false);
            aPoshSprms.set(NS_ooxml::LN_CT_PosH_posOffset, std::make_shared<RTFValue>());
        }
        aAnchorSprms.set(NS_ooxml::LN_CT_Anchor_positionH, std::make_shared<RTFValue>(aPoshAttributes, aPoshSprms));

        RTFSprms aPosvAttributes;
        RTFSprms aPosvSprms;
        if (m_aStates.top().aShape.nVertOrientRelationToken > 0)
            aPosvAttributes.set(NS_ooxml::LN_CT_PosV_relativeFrom, std::make_shared<RTFValue>(m_aStates.top().aShape.nVertOrientRelationToken));
        if (m_aStates.top().aShape.nTop != 0)
        {
            Mapper().positionOffset(OUString::number(oox::drawingml::convertHmmToEmu(m_aStates.top().aShape.nTop)), /*bVertical=*/true);
            aPosvSprms.set(NS_ooxml::LN_CT_PosV_posOffset, std::make_shared<RTFValue>());
        }
        aAnchorSprms.set(NS_ooxml::LN_CT_Anchor_positionV, std::make_shared<RTFValue>(aPosvAttributes, aPosvSprms));

        aAnchorSprms.set(NS_ooxml::LN_CT_Anchor_docPr, pDocprValue);
        aAnchorSprms.set(NS_ooxml::LN_graphic_graphic, pGraphicValue);
        // anchor sprm
        auto pValue = std::make_shared<RTFValue>(m_aStates.top().aShape.aAnchorAttributes, aAnchorSprms);
        aSprms.set(NS_ooxml::LN_anchor_anchor, pValue);
    }
    writerfilter::Reference<Properties>::Pointer_t pProperties = std::make_shared<RTFReferenceProperties>(aAttributes, aSprms);
    checkFirstRun();

    if (!m_aStates.top().pCurrentBuffer)
    {
        Mapper().props(pProperties);
        // Make sure we don't lose these properties with a too early reset.
        m_bHadPicture = true;
    }
    else
    {
        auto pValue = std::make_shared<RTFValue>(aAttributes, aSprms);
        m_aStates.top().pCurrentBuffer->push_back(Buf_t(BUFFER_PROPS, pValue, nullptr));
    }
}

RTFError RTFDocumentImpl::resolveChars(char ch)
{
    if (m_aStates.top().nInternalState == RTFInternalState::BIN)
    {
        m_pBinaryData.reset(new SvMemoryStream());
        m_pBinaryData->WriteChar(ch);
        for (int i = 0; i < m_aStates.top().nBinaryToRead - 1; ++i)
        {
            Strm().ReadChar(ch);
            m_pBinaryData->WriteChar(ch);
        }
        m_aStates.top().nInternalState = RTFInternalState::NORMAL;
        return RTFError::OK;
    }


    OStringBuffer aBuf;

    bool bUnicodeChecked = false;
    bool bSkipped = false;

    while (!Strm().IsEof() && (m_aStates.top().nInternalState == RTFInternalState::HEX || (ch != '{' && ch != '}' && ch != '\\')))
    {
        if (m_aStates.top().nInternalState == RTFInternalState::HEX || (ch != 0x0d && ch != 0x0a))
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
        if (m_aStates.top().nInternalState == RTFInternalState::HEX)
            break;

        if (RTL_TEXTENCODING_MS_932 == m_aStates.top().nCurrentEncoding)
        {
            unsigned char uch = ch;
            if ((uch >= 0x80 && uch <= 0x9F) || uch >= 0xE0)
            {
                // read second byte of 2-byte Shift-JIS - may be \ { }
                Strm().ReadChar(ch);
                if (m_aStates.top().nCharsToSkip == 0)
                {
                    // fdo#79384: Word will reject Shift-JIS following \loch
                    // but apparently OOo could read and (worse) write such documents
                    SAL_INFO_IF(m_aStates.top().eRunType != RTFParserState::RunType::DBCH, "writerfilter.rtf", "invalid Shift-JIS without DBCH");
                    assert(bUnicodeChecked);
                    aBuf.append(ch);
                }
                else
                {
                    assert(bSkipped);
                    // anybody who uses \ucN with Shift-JIS is insane
                    m_aStates.top().nCharsToSkip--;
                }
            }
        }

        Strm().ReadChar(ch);
    }
    if (m_aStates.top().nInternalState != RTFInternalState::HEX && !Strm().IsEof())
        Strm().SeekRel(-1);

    if (m_aStates.top().nInternalState == RTFInternalState::HEX && m_aStates.top().eDestination != Destination::LEVELNUMBERS)
    {
        if (!bSkipped)
            m_aHexBuffer.append(ch);
        return RTFError::OK;
    }

    if (m_aStates.top().eDestination == Destination::SKIP)
        return RTFError::OK;
    OString aStr = aBuf.makeStringAndClear();
    if (m_aStates.top().eDestination == Destination::LEVELNUMBERS)
    {
        if (aStr.toChar() != ';')
            m_aStates.top().aLevelNumbers.push_back(sal_Int32(ch));
        return RTFError::OK;
    }

    OUString aOUStr(OStringToOUString(aStr, m_aStates.top().nCurrentEncoding));
    SAL_INFO("writerfilter", "RTFDocumentImpl::resolveChars: collected '" << aOUStr << "'");

    if (m_aStates.top().eDestination == Destination::COLORTABLE)
    {
        // we hit a ';' at the end of each color entry
        sal_uInt32 color = (m_aStates.top().aCurrentColor.nRed << 16) | (m_aStates.top().aCurrentColor.nGreen << 8)
                           | m_aStates.top().aCurrentColor.nBlue;
        m_aColorTable.push_back(color);
        // set components back to zero
        m_aStates.top().aCurrentColor = RTFColorTableEntry();
    }
    else if (!aStr.isEmpty())
        m_aHexBuffer.append(aStr);

    checkUnicode(/*bUnicode =*/ false, /*bHex =*/ true);
    return RTFError::OK;
}

bool RTFFrame::inFrame()
{
    return m_nW > 0
           || m_nH > 0
           || m_nX > 0
           || m_nY > 0;
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
        pCurrentBuffer->push_back(Buf_t(BUFFER_STARTRUN, nullptr, nullptr));
        auto pValue = std::make_shared<RTFValue>(*sValue);
        pCurrentBuffer->push_back(Buf_t(BUFFER_TEXT, pValue, nullptr));
        pCurrentBuffer->push_back(Buf_t(BUFFER_ENDRUN, nullptr, nullptr));
    }
}

void RTFDocumentImpl::text(OUString& rString)
{
    if (rString.getLength() == 1 && m_aStates.top().eDestination != Destination::DOCCOMM)
    {
        // No cheating! Tokenizer ignores bare \r and \n, their hex \'0d / \'0a form doesn't count, either.
        sal_Unicode ch = rString[0];
        if (ch == 0x0d || ch == 0x0a)
            return;
    }

    bool bRet = true;
    switch (m_aStates.top().eDestination)
    {
    // Note: in fonttbl there may or may not be groups; in stylesheet
    // and revtbl groups are mandatory
    case Destination::FONTTABLE:
    case Destination::FONTENTRY:
    case Destination::STYLEENTRY:
    case Destination::LISTNAME:
    case Destination::REVISIONENTRY:
    {
        // ; is the end of the entry
        bool bEnd = false;
        if (rString.endsWith(";"))
        {
            rString = rString.copy(0, rString.getLength() - 1);
            bEnd = true;
        }
        m_aStates.top().pDestinationText->append(rString);
        if (bEnd)
        {
            // always clear, necessary in case of group-less fonttable
            OUString const aName = m_aStates.top().pDestinationText->makeStringAndClear();
            switch (m_aStates.top().eDestination)
            {
            case Destination::FONTTABLE:
            case Destination::FONTENTRY:
            {
                m_aFontNames[m_nCurrentFontIndex] = aName;
                if (m_nCurrentEncoding >= 0)
                {
                    m_aFontEncodings[m_nCurrentFontIndex] = m_nCurrentEncoding;
                    m_nCurrentEncoding = -1;
                }
                m_aStates.top().aTableAttributes.set(NS_ooxml::LN_CT_Font_name, std::make_shared<RTFValue>(aName));

                writerfilter::Reference<Properties>::Pointer_t const pProp(
                    new RTFReferenceProperties(m_aStates.top().aTableAttributes, m_aStates.top().aTableSprms)
                );

                //See fdo#47347 initial invalid font entry properties are inserted first,
                //so when we attempt to insert the correct ones, there's already an
                //entry in the map for them, so the new ones aren't inserted.
                auto lb = m_aFontTableEntries.lower_bound(m_nCurrentFontIndex);
                if (lb != m_aFontTableEntries.end() && !(m_aFontTableEntries.key_comp()(m_nCurrentFontIndex, lb->first)))
                    lb->second = pProp;
                else
                    m_aFontTableEntries.insert(lb, std::make_pair(m_nCurrentFontIndex, pProp));
            }
            break;
            case Destination::STYLEENTRY:
                if (m_aStates.top().aTableAttributes.find(NS_ooxml::LN_CT_Style_type))
                {
                    // Word strips whitespace around style names.
                    m_aStyleNames[m_nCurrentStyleIndex] = aName.trim();
                    auto pValue = std::make_shared<RTFValue>(aName.trim());
                    m_aStates.top().aTableAttributes.set(NS_ooxml::LN_CT_Style_styleId, pValue);
                    m_aStates.top().aTableSprms.set(NS_ooxml::LN_CT_Style_name, pValue);

                    writerfilter::Reference<Properties>::Pointer_t const pProp(createStyleProperties());
                    m_aStyleTableEntries.insert(std::make_pair(m_nCurrentStyleIndex, pProp));
                }
                else
                    SAL_INFO("writerfilter", "no RTF style type defined, ignoring");
                break;
            case Destination::LISTNAME:
                // TODO: what can be done with a list name?
                break;
            case Destination::REVISIONENTRY:
                m_aAuthors[m_aAuthors.size()] = aName;
                break;
            default:
                break;
            }
            resetAttributes();
            resetSprms();
        }
    }
    break;
    case Destination::LEVELTEXT:
    case Destination::SHAPEPROPERTYNAME:
    case Destination::SHAPEPROPERTYVALUE:
    case Destination::BOOKMARKEND:
    case Destination::PICT:
    case Destination::SHAPEPROPERTYVALUEPICT:
    case Destination::FORMFIELDNAME:
    case Destination::FORMFIELDLIST:
    case Destination::DATAFIELD:
    case Destination::AUTHOR:
    case Destination::KEYWORDS:
    case Destination::OPERATOR:
    case Destination::COMPANY:
    case Destination::COMMENT:
    case Destination::OBJDATA:
    case Destination::OBJCLASS:
    case Destination::ANNOTATIONDATE:
    case Destination::ANNOTATIONAUTHOR:
    case Destination::ANNOTATIONREFERENCE:
    case Destination::FALT:
    case Destination::PARAGRAPHNUMBERING_TEXTAFTER:
    case Destination::PARAGRAPHNUMBERING_TEXTBEFORE:
    case Destination::TITLE:
    case Destination::SUBJECT:
    case Destination::DOCCOMM:
    case Destination::ATNID:
    case Destination::ANNOTATIONREFERENCESTART:
    case Destination::ANNOTATIONREFERENCEEND:
    case Destination::MR:
    case Destination::MCHR:
    case Destination::MPOS:
    case Destination::MVERTJC:
    case Destination::MSTRIKEH:
    case Destination::MDEGHIDE:
    case Destination::MBEGCHR:
    case Destination::MSEPCHR:
    case Destination::MENDCHR:
    case Destination::MSUBHIDE:
    case Destination::MSUPHIDE:
    case Destination::MTYPE:
    case Destination::MGROW:
    case Destination::INDEXENTRY:
    case Destination::TOCENTRY:
    case Destination::PROPNAME:
    case Destination::STATICVAL:
        m_aStates.top().pDestinationText->append(rString);
        break;
    default:
        bRet = false;
        break;
    }
    if (bRet)
        return;

    if (!m_aIgnoreFirst.isEmpty() && m_aIgnoreFirst.equals(rString))
    {
        m_aIgnoreFirst.clear();
        return;
    }

    // Are we in the middle of the table definition? (No cell defs yet, but we already have some cell props.)
    if (m_aStates.top().aTableCellSprms.find(NS_ooxml::LN_CT_TcPrBase_vAlign).get() &&
            m_nTopLevelCells == 0)
    {
        m_aTableBufferStack.back().push_back(Buf_t(BUFFER_UTEXT, std::make_shared<RTFValue>(rString), nullptr));
        return;
    }

    checkFirstRun();
    checkNeedPap();

    // Don't return earlier, a bookmark start has to be in a paragraph group.
    if (m_aStates.top().eDestination == Destination::BOOKMARKSTART)
    {
        m_aStates.top().pDestinationText->append(rString);
        return;
    }

    RTFBuffer_t* pCurrentBuffer = m_aStates.top().pCurrentBuffer;

    if (!pCurrentBuffer && m_aStates.top().eDestination != Destination::FOOTNOTE)
        Mapper().startCharacterGroup();
    else if (pCurrentBuffer)
    {
        RTFValue::Pointer_t pValue;
        pCurrentBuffer->push_back(Buf_t(BUFFER_STARTRUN, pValue, nullptr));
    }

    if (m_aStates.top().eDestination == Destination::NORMAL
            || m_aStates.top().eDestination == Destination::FIELDRESULT
            || m_aStates.top().eDestination == Destination::SHAPETEXT)
        runProps();

    if (!pCurrentBuffer)
        Mapper().utext(reinterpret_cast<sal_uInt8 const*>(rString.getStr()), rString.getLength());
    else
    {
        auto pValue = std::make_shared<RTFValue>(rString);
        pCurrentBuffer->push_back(Buf_t(BUFFER_UTEXT, pValue, nullptr));
    }

    m_bNeedCr = true;

    if (!pCurrentBuffer && m_aStates.top().eDestination != Destination::FOOTNOTE)
        Mapper().endCharacterGroup();
    else if (pCurrentBuffer)
    {
        RTFValue::Pointer_t pValue;
        pCurrentBuffer->push_back(Buf_t(BUFFER_ENDRUN, pValue, nullptr));
    }
}

void RTFDocumentImpl::prepareProperties(
    RTFParserState& rState,
    writerfilter::Reference<Properties>::Pointer_t& o_rpParagraphProperties,
    writerfilter::Reference<Properties>::Pointer_t& o_rpFrameProperties,
    writerfilter::Reference<Properties>::Pointer_t& o_rpTableRowProperties,
    int const nCells, int const nCurrentCellX)
{
    o_rpParagraphProperties = getProperties(rState.aParagraphAttributes, rState.aParagraphSprms);

    if (rState.aFrame.hasProperties())
    {
        o_rpFrameProperties = std::make_shared<RTFReferenceProperties>(RTFSprms(), rState.aFrame.getSprms());
    }

    // Table width.
    RTFValue::Pointer_t const pTableWidthProps = rState.aTableRowSprms.find(NS_ooxml::LN_CT_TblPrBase_tblW);
    if (!pTableWidthProps.get())
    {
        auto pUnitValue = std::make_shared<RTFValue>(3);
        putNestedAttribute(rState.aTableRowSprms, NS_ooxml::LN_CT_TblPrBase_tblW, NS_ooxml::LN_CT_TblWidth_type, pUnitValue);
        auto pWValue = std::make_shared<RTFValue>(nCurrentCellX);
        putNestedAttribute(rState.aTableRowSprms, NS_ooxml::LN_CT_TblPrBase_tblW, NS_ooxml::LN_CT_TblWidth_w, pWValue);
    }

    auto pRowValue = std::make_shared<RTFValue>(1);
    if (nCells > 0)
        rState.aTableRowSprms.set(NS_ooxml::LN_tblRow, pRowValue);

    RTFValue::Pointer_t const pCellMar = rState.aTableRowSprms.find(NS_ooxml::LN_CT_TblPrBase_tblCellMar);
    if (!pCellMar.get())
    {
        // If no cell margins are defined, the default left/right margin is 0 in Word, but not in Writer.
        RTFSprms aAttributes;
        aAttributes.set(NS_ooxml::LN_CT_TblWidth_type, std::make_shared<RTFValue>(NS_ooxml::LN_Value_ST_TblWidth_dxa));
        aAttributes.set(NS_ooxml::LN_CT_TblWidth_w, std::make_shared<RTFValue>(0));
        putNestedSprm(rState.aTableRowSprms, NS_ooxml::LN_CT_TblPrBase_tblCellMar, NS_ooxml::LN_CT_TblCellMar_left, std::make_shared<RTFValue>(aAttributes));
        putNestedSprm(rState.aTableRowSprms, NS_ooxml::LN_CT_TblPrBase_tblCellMar, NS_ooxml::LN_CT_TblCellMar_right, std::make_shared<RTFValue>(aAttributes));
    }

    o_rpTableRowProperties = std::make_shared<RTFReferenceProperties>(rState.aTableRowAttributes, rState.aTableRowSprms);
}

void RTFDocumentImpl::sendProperties(
    writerfilter::Reference<Properties>::Pointer_t const& pParagraphProperties,
    writerfilter::Reference<Properties>::Pointer_t const& pFrameProperties,
    writerfilter::Reference<Properties>::Pointer_t const& pTableRowProperties)
{
    Mapper().props(pParagraphProperties);

    if (pFrameProperties)
    {
        Mapper().props(pFrameProperties);
    }

    Mapper().props(pTableRowProperties);

    tableBreak();
}

void RTFDocumentImpl::replayRowBuffer(
    RTFBuffer_t& rBuffer,
    ::std::deque<RTFSprms>& rCellsSrpms,
    ::std::deque<RTFSprms>& rCellsAttributes,
    int const nCells)
{
    for (int i = 0; i < nCells; ++i)
    {
        replayBuffer(rBuffer, &rCellsSrpms.front(), &rCellsAttributes.front());
        rCellsSrpms.pop_front();
        rCellsAttributes.pop_front();
    }
    for (Buf_t& i : rBuffer)
    {
        SAL_WARN_IF(BUFFER_CELLEND == std::get<0>(i),
                    "writerfilter.rtf", "dropping table cell!");
    }
    assert(0 == rCellsSrpms.size());
    assert(0 == rCellsAttributes.size());
}

void RTFDocumentImpl::replayBuffer(RTFBuffer_t& rBuffer,
                                   RTFSprms* const pSprms, RTFSprms const* const pAttributes)
{
    while (rBuffer.size())
    {
        Buf_t aTuple(rBuffer.front());
        rBuffer.pop_front();
        if (std::get<0>(aTuple) == BUFFER_PROPS)
        {
            // Construct properties via getProperties() and not directly, to take care of deduplication.
            writerfilter::Reference<Properties>::Pointer_t const pProp(
                getProperties(std::get<1>(aTuple)->getAttributes(), std::get<1>(aTuple)->getSprms())
            );
            Mapper().props(pProp);
        }
        else if (std::get<0>(aTuple) == BUFFER_NESTROW)
        {
            TableRowBuffer& rRowBuffer(*std::get<2>(aTuple));

            replayRowBuffer(rRowBuffer.buffer, rRowBuffer.cellsSprms,
                            rRowBuffer.cellsAttributes, rRowBuffer.nCells);

            sendProperties(rRowBuffer.pParaProperties,
                           rRowBuffer.pFrameProperties, rRowBuffer.pRowProperties);
        }
        else if (std::get<0>(aTuple) == BUFFER_CELLEND)
        {
            assert(pSprms && pAttributes);
            auto pValue = std::make_shared<RTFValue>(1);
            pSprms->set(NS_ooxml::LN_tblCell, pValue);
            writerfilter::Reference<Properties>::Pointer_t const pTableCellProperties(
                new RTFReferenceProperties(*pAttributes, *pSprms));
            Mapper().props(pTableCellProperties);
            tableBreak();
            break;
        }
        else if (std::get<0>(aTuple) == BUFFER_STARTRUN)
            Mapper().startCharacterGroup();
        else if (std::get<0>(aTuple) == BUFFER_TEXT)
        {
            sal_uInt8 const nValue = std::get<1>(aTuple)->getInt();
            Mapper().text(&nValue, 1);
        }
        else if (std::get<0>(aTuple) == BUFFER_UTEXT)
        {
            OUString const aString(std::get<1>(aTuple)->getString());
            Mapper().utext(reinterpret_cast<sal_uInt8 const*>(aString.getStr()), aString.getLength());
        }
        else if (std::get<0>(aTuple) == BUFFER_ENDRUN)
            Mapper().endCharacterGroup();
        else if (std::get<0>(aTuple) == BUFFER_PAR)
            parBreak();
        else if (std::get<0>(aTuple) == BUFFER_STARTSHAPE)
            m_pSdrImport->resolve(std::get<1>(aTuple)->getShape(), false, RTFSdrImport::SHAPE);
        else if (std::get<0>(aTuple) == BUFFER_RESOLVESHAPE)
            m_pSdrImport->resolve(std::get<1>(aTuple)->getShape(), true, RTFSdrImport::SHAPE);
        else if (std::get<0>(aTuple) == BUFFER_ENDSHAPE)
            m_pSdrImport->close();
        else if (std::get<0>(aTuple) == BUFFER_RESOLVESUBSTREAM)
        {
            RTFSprms& rAttributes = std::get<1>(aTuple)->getAttributes();
            sal_Size nPos = rAttributes.find(0)->getInt();
            Id nId = rAttributes.find(1)->getInt();
            OUString aCustomMark = rAttributes.find(2)->getString();
            resolveSubstream(nPos, nId, aCustomMark);
        }
        else
            assert(false);
    }

}

bool findPropertyName(const std::vector<beans::PropertyValue>& rProperties, const OUString& rName)
{
    for (auto& rProperty : rProperties)
    {
        if (rProperty.Name == rName)
            return true;
    }
    return false;
}

void RTFDocumentImpl::backupTableRowProperties()
{
    if (m_nTopLevelCurrentCellX)
    {
        m_aBackupTableRowSprms = m_aStates.top().aTableRowSprms;
        m_aBackupTableRowAttributes = m_aStates.top().aTableRowAttributes;
        m_nBackupTopLevelCurrentCellX = m_nTopLevelCurrentCellX;
    }
}

void RTFDocumentImpl::restoreTableRowProperties()
{
    m_aStates.top().aTableRowSprms = m_aBackupTableRowSprms;
    m_aStates.top().aTableRowAttributes = m_aBackupTableRowAttributes;
    m_nTopLevelCurrentCellX = m_nBackupTopLevelCurrentCellX;
}

void RTFDocumentImpl::resetTableRowProperties()
{
    m_aStates.top().aTableRowSprms = m_aDefaultState.aTableRowSprms;
    m_aStates.top().aTableRowSprms.set(NS_ooxml::LN_CT_TblGridBase_gridCol, std::make_shared<RTFValue>(-1), RTFOverwrite::NO_APPEND);
    m_aStates.top().aTableRowAttributes = m_aDefaultState.aTableRowAttributes;
    if (Destination::NESTEDTABLEPROPERTIES == m_aStates.top().eDestination)
        m_nNestedCurrentCellX = 0;
    else
        m_nTopLevelCurrentCellX = 0;
}

RTFError RTFDocumentImpl::dispatchToggle(RTFKeyword nKeyword, bool bParam, int nParam)
{
    setNeedSect(true);
    checkUnicode(/*bUnicode =*/ true, /*bHex =*/ true);
    RTFSkipDestination aSkip(*this);
    int nSprm = -1;
    auto pBoolValue = std::make_shared<RTFValue>(int(!bParam || nParam != 0));

    // Underline toggles.
    switch (nKeyword)
    {
    case RTF_UL:
        nSprm = NS_ooxml::LN_Value_ST_Underline_single;
        break;
    case RTF_ULDASH:
        nSprm = NS_ooxml::LN_Value_ST_Underline_dash;
        break;
    case RTF_ULDASHD:
        nSprm = NS_ooxml::LN_Value_ST_Underline_dotDash;
        break;
    case RTF_ULDASHDD:
        nSprm = NS_ooxml::LN_Value_ST_Underline_dotDotDash;
        break;
    case RTF_ULDB:
        nSprm = NS_ooxml::LN_Value_ST_Underline_double;
        break;
    case RTF_ULHWAVE:
        nSprm = NS_ooxml::LN_Value_ST_Underline_wavyHeavy;
        break;
    case RTF_ULLDASH:
        nSprm = NS_ooxml::LN_Value_ST_Underline_dashLong;
        break;
    case RTF_ULTH:
        nSprm = NS_ooxml::LN_Value_ST_Underline_thick;
        break;
    case RTF_ULTHD:
        nSprm = NS_ooxml::LN_Value_ST_Underline_dottedHeavy;
        break;
    case RTF_ULTHDASH:
        nSprm = NS_ooxml::LN_Value_ST_Underline_dashedHeavy;
        break;
    case RTF_ULTHDASHD:
        nSprm = NS_ooxml::LN_Value_ST_Underline_dashDotHeavy;
        break;
    case RTF_ULTHDASHDD:
        nSprm = NS_ooxml::LN_Value_ST_Underline_dashDotDotHeavy;
        break;
    case RTF_ULTHLDASH:
        nSprm = NS_ooxml::LN_Value_ST_Underline_dashLongHeavy;
        break;
    case RTF_ULULDBWAVE:
        nSprm = NS_ooxml::LN_Value_ST_Underline_wavyDouble;
        break;
    case RTF_ULWAVE:
        nSprm = NS_ooxml::LN_Value_ST_Underline_wave;
        break;
    default:
        break;
    }
    if (nSprm >= 0)
    {
        auto pValue = std::make_shared<RTFValue>((!bParam || nParam != 0) ? nSprm : NS_ooxml::LN_Value_ST_Underline_none);
        m_aStates.top().aCharacterAttributes.set(NS_ooxml::LN_CT_Underline_val, pValue);
        return RTFError::OK;
    }

    // Accent characters (over dot / over comma).
    switch (nKeyword)
    {
    case RTF_ACCNONE:
        nSprm = NS_ooxml::LN_Value_ST_Em_none;
        break;
    case RTF_ACCDOT:
        nSprm = NS_ooxml::LN_Value_ST_Em_dot;
        break;
    case RTF_ACCCOMMA:
        nSprm = NS_ooxml::LN_Value_ST_Em_comma;
        break;
    case RTF_ACCCIRCLE:
        nSprm = NS_ooxml::LN_Value_ST_Em_circle;
        break;
    case RTF_ACCUNDERDOT:
        nSprm = NS_ooxml::LN_Value_ST_Em_underDot;
        break;
    default:
        break;
    }
    if (nSprm >= 0)
    {
        auto pValue = std::make_shared<RTFValue>((!bParam || nParam != 0) ? nSprm : 0);
        m_aStates.top().aCharacterSprms.set(NS_ooxml::LN_EG_RPrBase_em, pValue);
        return RTFError::OK;
    }

    // Trivial character sprms.
    switch (nKeyword)
    {
    case RTF_B:
    case RTF_AB:
        nSprm = (m_aStates.top().isRightToLeft || m_aStates.top().eRunType == RTFParserState::RunType::HICH)
                ? NS_ooxml::LN_EG_RPrBase_bCs : NS_ooxml::LN_EG_RPrBase_b;
        break;
    case RTF_I:
    case RTF_AI:
        nSprm = (m_aStates.top().isRightToLeft || m_aStates.top().eRunType == RTFParserState::RunType::HICH)
                ? NS_ooxml::LN_EG_RPrBase_iCs : NS_ooxml::LN_EG_RPrBase_i;
        break;
    case RTF_OUTL:
        nSprm = NS_ooxml::LN_EG_RPrBase_outline;
        break;
    case RTF_SHAD:
        nSprm = NS_ooxml::LN_EG_RPrBase_shadow;
        break;
    case RTF_V:
        nSprm = NS_ooxml::LN_EG_RPrBase_vanish;
        break;
    case RTF_STRIKE:
        nSprm = NS_ooxml::LN_EG_RPrBase_strike;
        break;
    case RTF_STRIKED:
        nSprm = NS_ooxml::LN_EG_RPrBase_dstrike;
        break;
    case RTF_SCAPS:
        nSprm = NS_ooxml::LN_EG_RPrBase_smallCaps;
        break;
    case RTF_IMPR:
        nSprm = NS_ooxml::LN_EG_RPrBase_imprint;
        break;
    case RTF_CAPS:
        nSprm = NS_ooxml::LN_EG_RPrBase_caps;
        break;
    default:
        break;
    }
    if (nSprm >= 0)
    {
        m_aStates.top().aCharacterSprms.set(nSprm, pBoolValue);
        return RTFError::OK;
    }

    switch (nKeyword)
    {
    case RTF_ASPALPHA:
        m_aStates.top().aParagraphSprms.set(NS_ooxml::LN_CT_PPrBase_autoSpaceDE, pBoolValue);
        break;
    case RTF_DELETED:
    case RTF_REVISED:
    {
        auto pValue = std::make_shared<RTFValue>(nKeyword == RTF_DELETED ? oox::XML_del : oox::XML_ins);
        putNestedAttribute(m_aStates.top().aCharacterSprms, NS_ooxml::LN_trackchange, NS_ooxml::LN_token, pValue);
    }
    break;
    case RTF_SBAUTO:
        putNestedAttribute(m_aStates.top().aParagraphSprms, NS_ooxml::LN_CT_PPrBase_spacing, NS_ooxml::LN_CT_Spacing_beforeAutospacing, pBoolValue);
        break;
    case RTF_SAAUTO:
        putNestedAttribute(m_aStates.top().aParagraphSprms, NS_ooxml::LN_CT_PPrBase_spacing, NS_ooxml::LN_CT_Spacing_afterAutospacing, pBoolValue);
        break;
    case RTF_FACINGP:
        m_aSettingsTableSprms.set(NS_ooxml::LN_CT_Settings_evenAndOddHeaders, pBoolValue);
        break;
    case RTF_HYPHAUTO:
        m_aSettingsTableSprms.set(NS_ooxml::LN_CT_Settings_autoHyphenation, pBoolValue);
        break;
    case RTF_HYPHPAR:
        m_aStates.top().aParagraphSprms.set(NS_ooxml::LN_CT_PPrBase_suppressAutoHyphens, std::make_shared<RTFValue>(int(bParam && nParam == 0)));
        break;
    default:
    {
        SAL_INFO("writerfilter", "TODO handle toggle '" << keywordToString(nKeyword) << "'");
        aSkip.setParsed(false);
    }
    break;
    }
    return RTFError::OK;
}

RTFError RTFDocumentImpl::pushState()
{
    //SAL_INFO("writerfilter", OSL_THIS_FUNC << " before push: " << m_pTokenizer->getGroup());

    checkUnicode(/*bUnicode =*/ true, /*bHex =*/ true);
    m_nGroupStartPos = Strm().Tell();

    if (m_aStates.empty())
        m_aStates.push(m_aDefaultState);
    else
    {
        // fdo#85812 group resets run type of _current_ and new state (but not RTL)
        m_aStates.top().eRunType = RTFParserState::RunType::LOCH;

        if (m_aStates.top().eDestination == Destination::MR)
            lcl_DestinationToMath(*m_aStates.top().pDestinationText, m_aMathBuffer, m_bMathNor);
        m_aStates.push(m_aStates.top());
    }
    m_aStates.top().aDestinationText.setLength(0); // was copied: always reset!

    m_pTokenizer->pushGroup();

    switch (m_aStates.top().eDestination)
    {
    case Destination::FONTTABLE:
        // this is a "faked" destination for the font entry
        m_aStates.top().pDestinationText = &m_aStates.top().aDestinationText;
        m_aStates.top().eDestination = Destination::FONTENTRY;
        break;
    case Destination::STYLESHEET:
        // this is a "faked" destination for the style sheet entry
        m_aStates.top().pDestinationText = &m_aStates.top().aDestinationText;
        m_aStates.top().eDestination = Destination::STYLEENTRY;
        {
            // the *default* is \s0 i.e. paragraph style default
            // this will be overwritten by \sN \csN \dsN \tsN
            m_nCurrentStyleIndex = 0;
            auto pValue = std::make_shared<RTFValue>(NS_ooxml::LN_Value_ST_StyleType_paragraph);
            m_aStates.top().aTableAttributes.set(NS_ooxml::LN_CT_Style_type, pValue);
        }
        break;
    case Destination::FIELDRESULT:
    case Destination::SHAPETEXT:
    case Destination::FORMFIELD:
    case Destination::FIELDINSTRUCTION:
    case Destination::PICT:
        m_aStates.top().eDestination = Destination::NORMAL;
        break;
    case Destination::MNUM:
    case Destination::MDEN:
    case Destination::ME:
    case Destination::MFNAME:
    case Destination::MLIM:
    case Destination::MSUB:
    case Destination::MSUP:
    case Destination::MDEG:
    case Destination::MOMATH:
        m_aStates.top().eDestination = Destination::MR;
        break;
    case Destination::REVISIONTABLE:
        // this is a "faked" destination for the revision table entry
        m_aStates.top().pDestinationText = &m_aStates.top().aDestinationText;
        m_aStates.top().eDestination = Destination::REVISIONENTRY;
        break;
    default:
        break;
    }

    // If this is true, then ooxml:endtrackchange will be generated.  Make sure
    // we don't generate more ooxml:endtrackchange than ooxml:trackchange: new
    // state does not inherit this flag.
    m_aStates.top().bStartedTrackchange = false;

    return RTFError::OK;
}

writerfilter::Reference<Properties>::Pointer_t
RTFDocumentImpl::createStyleProperties()
{
    RTFValue::Pointer_t pParaProps = std::make_shared<RTFValue>(m_aStates.top().aParagraphAttributes, m_aStates.top().aParagraphSprms);
    RTFValue::Pointer_t pCharProps = std::make_shared<RTFValue>(m_aStates.top().aCharacterAttributes, m_aStates.top().aCharacterSprms);

    // resetSprms will clean up this modification
    m_aStates.top().aTableSprms.set(NS_ooxml::LN_CT_Style_pPr, pParaProps);
    m_aStates.top().aTableSprms.set(NS_ooxml::LN_CT_Style_rPr, pCharProps);

    writerfilter::Reference<Properties>::Pointer_t const pProps(
        new RTFReferenceProperties(m_aStates.top().aTableAttributes, m_aStates.top().aTableSprms));
    return pProps;
}

void RTFDocumentImpl::resetSprms()
{
    m_aStates.top().aTableSprms.clear();
    m_aStates.top().aCharacterSprms.clear();
    m_aStates.top().aParagraphSprms.clear();
}

void RTFDocumentImpl::resetAttributes()
{
    m_aStates.top().aTableAttributes.clear();
    m_aStates.top().aCharacterAttributes.clear();
    m_aStates.top().aParagraphAttributes.clear();
}

bool lcl_containsProperty(const uno::Sequence<beans::Property>& rProperties, const OUString& rName)
{
    return std::find_if(rProperties.begin(), rProperties.end(), [&](const beans::Property& rProperty)
    {
        return rProperty.Name == rName;
    }) != rProperties.end();
}

RTFError RTFDocumentImpl::popState()
{
    //SAL_INFO("writerfilter", OSL_THIS_FUNC << " before pop: m_pTokenizer->getGroup() " << m_pTokenizer->getGroup() <<
    //                         ", dest state: " << m_aStates.top().eDestination);

    checkUnicode(/*bUnicode =*/ true, /*bHex =*/ true);
    if (m_aStates.empty())
        return RTFError::GROUP_UNDER;
    RTFParserState aState(m_aStates.top());
    m_bWasInFrame = aState.aFrame.inFrame();

    // dmapper expects some content in header/footer, so if there would be nothing, add an empty paragraph.
    if (m_pTokenizer->getGroup() == 1 && m_bFirstRun)
    {
        switch (m_nStreamType)
        {
        case NS_ooxml::LN_headerl:
        case NS_ooxml::LN_headerr:
        case NS_ooxml::LN_headerf:
        case NS_ooxml::LN_footerl:
        case NS_ooxml::LN_footerr:
        case NS_ooxml::LN_footerf:
            dispatchSymbol(RTF_PAR);
            break;
        }
    }

    switch (aState.eDestination)
    {
    case Destination::FONTTABLE:
    {
        writerfilter::Reference<Table>::Pointer_t const pTable(new RTFReferenceTable(m_aFontTableEntries));
        Mapper().table(NS_ooxml::LN_FONTTABLE, pTable);
        if (m_nDefaultFontIndex >= 0)
        {
            auto pValue = std::make_shared<RTFValue>(m_aFontNames[getFontIndex(m_nDefaultFontIndex)]);
            putNestedAttribute(m_aDefaultState.aCharacterSprms, NS_ooxml::LN_EG_RPrBase_rFonts, NS_ooxml::LN_CT_Fonts_ascii, pValue);
        }
    }
    break;
    case Destination::STYLESHEET:
    {
        writerfilter::Reference<Table>::Pointer_t const pTable(new RTFReferenceTable(m_aStyleTableEntries));
        Mapper().table(NS_ooxml::LN_STYLESHEET, pTable);
    }
    break;
    case Destination::LISTOVERRIDETABLE:
    {
        RTFSprms aListTableAttributes;
        writerfilter::Reference<Properties>::Pointer_t pProp = std::make_shared<RTFReferenceProperties>(aListTableAttributes, m_aListTableSprms);
        RTFReferenceTable::Entries_t aListTableEntries;
        aListTableEntries.insert(std::make_pair(0, pProp));
        writerfilter::Reference<Table>::Pointer_t const pTable(new RTFReferenceTable(aListTableEntries));
        Mapper().table(NS_ooxml::LN_NUMBERING, pTable);
    }
    break;
    case Destination::LISTENTRY:
        for (auto& rListLevelEntry : aState.aListLevelEntries)
            aState.aTableSprms.set(rListLevelEntry.first, rListLevelEntry.second, RTFOverwrite::NO_APPEND);
        break;
    case Destination::FIELDINSTRUCTION:
    {
        auto pValue = std::make_shared<RTFValue>(m_aFormfieldAttributes, m_aFormfieldSprms);
        RTFSprms aFFAttributes;
        RTFSprms aFFSprms;
        aFFSprms.set(NS_ooxml::LN_ffdata, pValue);
        if (!m_aStates.top().pCurrentBuffer)
        {
            writerfilter::Reference<Properties>::Pointer_t pProperties = std::make_shared<RTFReferenceProperties>(aFFAttributes, aFFSprms);
            Mapper().props(pProperties);
        }
        else
        {
            auto pFFValue = std::make_shared<RTFValue>(aFFAttributes, aFFSprms);
            m_aStates.top().pCurrentBuffer->push_back(Buf_t(BUFFER_PROPS, pFFValue, nullptr));
        }
        m_aFormfieldAttributes.clear();
        m_aFormfieldSprms.clear();
        singleChar(cFieldSep);
    }
    break;
    case Destination::FIELDRESULT:
        singleChar(cFieldEnd);

        if (!m_aPicturePath.isEmpty())
        {
            // Read the picture into m_aStates.top().aDestinationText.
            pushState();
            dispatchDestination(RTF_PICT);
            if (m_aPicturePath.endsWith(".png"))
                dispatchFlag(RTF_PNGBLIP);
            OUString aFileURL = m_rMediaDescriptor.getUnpackedValueOrDefault(utl::MediaDescriptor::PROP_URL(), OUString());
            OUString aPictureURL;
            try
            {
                aPictureURL = rtl::Uri::convertRelToAbs(aFileURL, m_aPicturePath);
            }
            catch (const rtl::MalformedUriException& rException)
            {
                SAL_WARN("writerfilter", "rtl::Uri::convertRelToAbs() failed: " << rException.getMessage());
            }

            if (!aPictureURL.isEmpty())
            {
                SvFileStream aStream(aPictureURL, StreamMode::READ);
                if (aStream.IsOpen())
                {
                    OUStringBuffer aBuf;
                    while (aStream.good())
                    {
                        unsigned char ch = 0;
                        aStream.ReadUChar(ch);
                        if (ch < 16)
                            aBuf.append("0");
                        aBuf.append(OUString::number(ch, 16));
                    }
                    m_aStates.top().aDestinationText = aBuf;
                }
            }
            popState();
            m_aPicturePath.clear();
        }

        break;
    case Destination::LEVELTEXT:
    {
        if (&m_aStates.top().aDestinationText != m_aStates.top().pDestinationText)
            break; // not for nested group
        OUString aStr = m_aStates.top().pDestinationText->makeStringAndClear();

        // The first character is the length of the string (the rest should be ignored).
        sal_Int32 nLength(aStr.toChar());
        OUString aValue;
        if (nLength < aStr.getLength())
            aValue = aStr.copy(1, nLength);
        else
            aValue = aStr;
        auto pValue = std::make_shared<RTFValue>(aValue, true);
        aState.aTableAttributes.set(NS_ooxml::LN_CT_LevelText_val, pValue);
    }
    break;
    case Destination::LEVELNUMBERS:
        if (aState.aTableSprms.find(NS_ooxml::LN_CT_Lvl_lvlText))
        {
            RTFSprms& rAttributes = aState.aTableSprms.find(NS_ooxml::LN_CT_Lvl_lvlText)->getAttributes();
            RTFValue::Pointer_t pValue = rAttributes.find(NS_ooxml::LN_CT_LevelText_val);
            if (pValue)
            {
                OUString aOrig = pValue->getString();

                OUStringBuffer aBuf;
                sal_Int32 nReplaces = 1;
                for (int i = 0; i < aOrig.getLength(); i++)
                {
                    if (std::find(aState.aLevelNumbers.begin(), aState.aLevelNumbers.end(), i+1)
                            != aState.aLevelNumbers.end())
                    {
                        aBuf.append('%');
                        // '1.1.1' -> '%1.%2.%3', but '1.' (with '2.' prefix omitted) is %2.
                        aBuf.append(sal_Int32(nReplaces++ + aState.nListLevelNum + 1 - aState.aLevelNumbers.size()));
                    }
                    else
                        aBuf.append(aOrig.copy(i, 1));
                }

                pValue->setString(aBuf.makeStringAndClear());
            }
        }
        break;
    case Destination::SHAPEPROPERTYNAME:
        if (&m_aStates.top().aDestinationText != m_aStates.top().pDestinationText)
            break; // not for nested group
        aState.aShape.aProperties.push_back(std::make_pair(m_aStates.top().pDestinationText->makeStringAndClear(), OUString()));
        break;
    case Destination::SHAPEPROPERTYVALUE:
        if (aState.aShape.aProperties.size())
        {
            aState.aShape.aProperties.back().second = m_aStates.top().pDestinationText->makeStringAndClear();
            if (m_aStates.top().bHadShapeText)
                m_pSdrImport->append(aState.aShape.aProperties.back().first, aState.aShape.aProperties.back().second);
            else if (aState.bInShapeGroup && !aState.bInShape && aState.aShape.aProperties.back().first == "rotation")
            {
                // Rotation should be applied on the groupshape itself, not on each shape.
                aState.aShape.aGroupProperties.push_back(aState.aShape.aProperties.back());
                aState.aShape.aProperties.pop_back();
            }
        }
        break;
    case Destination::PICPROP:
    case Destination::SHAPEINSTRUCTION:
        if (m_aStates.size() > 1 && m_aStates[m_aStates.size() - 2].eDestination == Destination::SHAPEINSTRUCTION)
        {
            // Do not resolve shape if shape instruction destination is inside other shape instruction
        }
        else if (!m_bObject && !aState.bInListpicture && !aState.bHadShapeText && !(aState.bInShapeGroup && !aState.bInShape))
        {
            // Don't trigger a shape import in case we're only leaving the \shpinst of the groupshape itself.
            RTFSdrImport::ShapeOrPict eType = (aState.eDestination == Destination::SHAPEINSTRUCTION) ? RTFSdrImport::SHAPE : RTFSdrImport::PICT;
            if (!m_aStates.top().pCurrentBuffer || eType != RTFSdrImport::SHAPE)
                m_pSdrImport->resolve(m_aStates.top().aShape, true, eType);
            else
            {
                // Shape inside table: buffer the import to have correct anchor position.
                auto pValue = std::make_shared<RTFValue>(m_aStates.top().aShape);
                m_aStates.top().pCurrentBuffer->push_back(Buf_t(BUFFER_RESOLVESHAPE, pValue, nullptr));
            }
        }
        else if (aState.bInShapeGroup && !aState.bInShape)
        {
            // End of a groupshape, as we're in shapegroup, but not in a real shape.
            for (auto& rGroupProperty : aState.aShape.aGroupProperties)
                m_pSdrImport->appendGroupProperty(rGroupProperty.first, rGroupProperty.second);
            aState.aShape.aGroupProperties.clear();
        }
        break;
    case Destination::BOOKMARKSTART:
    {
        if (&m_aStates.top().aDestinationText != m_aStates.top().pDestinationText)
            break; // not for nested group
        OUString aStr = m_aStates.top().pDestinationText->makeStringAndClear();
        int nPos = m_aBookmarks.size();
        m_aBookmarks[aStr] = nPos;
        if (!m_aStates.top().pCurrentBuffer)
            Mapper().props(std::make_shared<RTFReferenceProperties>(lcl_getBookmarkProperties(nPos, aStr)));
        else
            m_aStates.top().pCurrentBuffer->push_back(Buf_t(BUFFER_PROPS, std::make_shared<RTFValue>(lcl_getBookmarkProperties(nPos, aStr)), nullptr));
    }
    break;
    case Destination::BOOKMARKEND:
    {
        if (&m_aStates.top().aDestinationText != m_aStates.top().pDestinationText)
            break; // not for nested group
        OUString aStr = m_aStates.top().pDestinationText->makeStringAndClear();
        if (!m_aStates.top().pCurrentBuffer)
            Mapper().props(std::make_shared<RTFReferenceProperties>(lcl_getBookmarkProperties(m_aBookmarks[aStr], aStr)));
        else
            m_aStates.top().pCurrentBuffer->push_back(Buf_t(BUFFER_PROPS, std::make_shared<RTFValue>(lcl_getBookmarkProperties(m_aBookmarks[aStr], aStr)), nullptr));
    }
    break;
    case Destination::INDEXENTRY:
    case Destination::TOCENTRY:
    {
        if (&m_aStates.top().aDestinationText != m_aStates.top().pDestinationText)
            break; // not for nested group
        OUString str(m_aStates.top().pDestinationText->makeStringAndClear());
        // dmapper expects this as a field, so let's fake something...
        OUString const field(
            (Destination::INDEXENTRY == aState.eDestination) ? OUStringLiteral("XE") : OUStringLiteral("TC"));
        str = field + " \"" + str.replaceAll("\"", "\\\"") + "\"";
        singleChar(cFieldStart);
        Mapper().utext(reinterpret_cast<sal_uInt8 const*>(str.getStr()), str.getLength());
        singleChar(cFieldSep);
        // no result
        singleChar(cFieldEnd);
    }
    break;
    case Destination::FORMFIELDNAME:
    {
        if (&m_aStates.top().aDestinationText != m_aStates.top().pDestinationText)
            break; // not for nested group
        auto pValue = std::make_shared<RTFValue>(m_aStates.top().pDestinationText->makeStringAndClear());
        m_aFormfieldSprms.set(NS_ooxml::LN_CT_FFData_name, pValue);
    }
    break;
    case Destination::FORMFIELDLIST:
    {
        if (&m_aStates.top().aDestinationText != m_aStates.top().pDestinationText)
            break; // not for nested group
        auto pValue = std::make_shared<RTFValue>(m_aStates.top().pDestinationText->makeStringAndClear());
        m_aFormfieldSprms.set(NS_ooxml::LN_CT_FFDDList_listEntry, pValue);
    }
    break;
    case Destination::DATAFIELD:
    {
        if (&m_aStates.top().aDestinationText != m_aStates.top().pDestinationText)
            break; // not for nested group
        OString aStr = OUStringToOString(m_aStates.top().pDestinationText->makeStringAndClear(), aState.nCurrentEncoding);
        // decode hex dump
        OStringBuffer aBuf;
        const char* str = aStr.getStr();
        int b = 0, count = 2;
        for (int i = 0; i < aStr.getLength(); ++i)
        {
            char ch = str[i];
            if (ch != 0x0d && ch != 0x0a)
            {
                b = b << 4;
                sal_Int8 parsed = RTFTokenizer::asHex(ch);
                if (parsed == -1)
                    return RTFError::HEX_INVALID;
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
        sal_Int32 nLength = aStr.toChar();
        if (!aStr.isEmpty())
            aStr = aStr.copy(1);
        nLength = std::min(nLength, aStr.getLength());
        OString aName = aStr.copy(0, nLength);
        if (aStr.getLength() > nLength)
            aStr = aStr.copy(nLength+1); // zero-terminated string
        else
            aStr.clear();
        // extract default text
        nLength = aStr.toChar();
        if (!aStr.isEmpty())
            aStr = aStr.copy(1);
        auto pNValue = std::make_shared<RTFValue>(OStringToOUString(aName, aState.nCurrentEncoding));
        m_aFormfieldSprms.set(NS_ooxml::LN_CT_FFData_name, pNValue);
        if (nLength > 0)
        {
            OString aDefaultText = aStr.copy(0, std::min(nLength, aStr.getLength()));
            auto pDValue = std::make_shared<RTFValue>(OStringToOUString(aDefaultText, aState.nCurrentEncoding));
            m_aFormfieldSprms.set(NS_ooxml::LN_CT_FFTextInput_default, pDValue);
        }

        m_bFormField = false;
    }
    break;
    case Destination::CREATIONTIME:
        if (m_xDocumentProperties.is())
            m_xDocumentProperties->setCreationDate(lcl_getDateTime(aState));
        break;
    case Destination::REVISIONTIME:
        if (m_xDocumentProperties.is())
            m_xDocumentProperties->setModificationDate(lcl_getDateTime(aState));
        break;
    case Destination::PRINTTIME:
        if (m_xDocumentProperties.is())
            m_xDocumentProperties->setPrintDate(lcl_getDateTime(aState));
        break;
    case Destination::AUTHOR:
        if (&m_aStates.top().aDestinationText != m_aStates.top().pDestinationText)
            break; // not for nested group
        if (m_xDocumentProperties.is())
            m_xDocumentProperties->setAuthor(m_aStates.top().pDestinationText->makeStringAndClear());
        break;
    case Destination::KEYWORDS:
        if (&m_aStates.top().aDestinationText != m_aStates.top().pDestinationText)
            break; // not for nested group
        if (m_xDocumentProperties.is())
            m_xDocumentProperties->setKeywords(comphelper::string::convertCommaSeparated(m_aStates.top().pDestinationText->makeStringAndClear()));
        break;
    case Destination::COMMENT:
        if (&m_aStates.top().aDestinationText != m_aStates.top().pDestinationText)
            break; // not for nested group
        if (m_xDocumentProperties.is())
            m_xDocumentProperties->setGenerator(m_aStates.top().pDestinationText->makeStringAndClear());
        break;
    case Destination::SUBJECT:
        if (&m_aStates.top().aDestinationText != m_aStates.top().pDestinationText)
            break; // not for nested group
        if (m_xDocumentProperties.is())
            m_xDocumentProperties->setSubject(m_aStates.top().pDestinationText->makeStringAndClear());
        break;
    case Destination::TITLE:
    {
        if (&m_aStates.top().aDestinationText != m_aStates.top().pDestinationText)
            break; // not for nested group
        if (m_xDocumentProperties.is())
            m_xDocumentProperties->setTitle(aState.pDestinationText->makeStringAndClear());
    }
    break;

    case Destination::DOCCOMM:
        if (&m_aStates.top().aDestinationText != m_aStates.top().pDestinationText)
            break; // not for nested group
        if (m_xDocumentProperties.is())
            m_xDocumentProperties->setDescription(m_aStates.top().pDestinationText->makeStringAndClear());
        break;
    case Destination::OPERATOR:
    case Destination::COMPANY:
    {
        if (&m_aStates.top().aDestinationText != m_aStates.top().pDestinationText)
            break; // not for nested group
        OUString aName = aState.eDestination == Destination::OPERATOR ? OUString("Operator") : OUString("Company");
        uno::Any aValue = uno::makeAny(m_aStates.top().pDestinationText->makeStringAndClear());
        if (m_xDocumentProperties.is())
        {
            uno::Reference<beans::XPropertyContainer> xUserDefinedProperties = m_xDocumentProperties->getUserDefinedProperties();
            uno::Reference<beans::XPropertySet> xPropertySet(xUserDefinedProperties, uno::UNO_QUERY);
            uno::Reference<beans::XPropertySetInfo> xPropertySetInfo = xPropertySet->getPropertySetInfo();
            if (xPropertySetInfo->hasPropertyByName(aName))
                xPropertySet->setPropertyValue(aName, aValue);
            else
                xUserDefinedProperties->addProperty(aName, beans::PropertyAttribute::REMOVABLE, aValue);
        }
    }
    break;
    case Destination::OBJDATA:
    {
        if (&m_aStates.top().aDestinationText != m_aStates.top().pDestinationText)
            break; // not for nested group

        RTFError eError = handleEmbeddedObject();
        if (eError != RTFError::OK)
            return eError;
    }
    break;
    case Destination::OBJCLASS:
    {
        auto pValue = std::make_shared<RTFValue>(m_aStates.top().pDestinationText->makeStringAndClear());
        m_aOLEAttributes.set(NS_ooxml::LN_CT_OLEObject_ProgID, pValue);
        break;
    }
    case Destination::OBJECT:
    {
        if (!m_bObject)
        {
            // if the object is in a special container we will use the \result
            // element instead of the \objdata
            // (see RTF_OBJECT in RTFDocumentImpl::dispatchDestination)
            break;
        }

        RTFSprms aObjectSprms;
        auto pOLEValue = std::make_shared<RTFValue>(m_aOLEAttributes);
        aObjectSprms.set(NS_ooxml::LN_OLEObject_OLEObject, pOLEValue);

        RTFSprms aObjAttributes;
        RTFSprms aObjSprms;
        auto pValue = std::make_shared<RTFValue>(m_aObjectAttributes, aObjectSprms);
        aObjSprms.set(NS_ooxml::LN_object, pValue);
        writerfilter::Reference<Properties>::Pointer_t pProperties = std::make_shared<RTFReferenceProperties>(aObjAttributes, aObjSprms);
        uno::Reference<drawing::XShape> xShape;
        RTFValue::Pointer_t pShape = m_aObjectAttributes.find(NS_ooxml::LN_shape);
        OSL_ASSERT(pShape.get());
        if (pShape.get())
            pShape->getAny() >>= xShape;
        if (xShape.is())
        {
            Mapper().startShape(xShape);
            Mapper().props(pProperties);
            Mapper().endShape();
        }
        m_aObjectAttributes.clear();
        m_aOLEAttributes.clear();
        m_bObject = false;
    }
    break;
    case Destination::ANNOTATIONDATE:
    {
        if (&m_aStates.top().aDestinationText != m_aStates.top().pDestinationText)
            break; // not for nested group
        OUString aStr(OStringToOUString(DTTM22OString(m_aStates.top().pDestinationText->makeStringAndClear().toInt32()),
                                        aState.nCurrentEncoding));
        auto pValue = std::make_shared<RTFValue>(aStr);
        RTFSprms aAnnAttributes;
        aAnnAttributes.set(NS_ooxml::LN_CT_TrackChange_date, pValue);
        writerfilter::Reference<Properties>::Pointer_t pProperties = std::make_shared<RTFReferenceProperties>(aAnnAttributes);
        Mapper().props(pProperties);
    }
    break;
    case Destination::ANNOTATIONAUTHOR:
        if (&m_aStates.top().aDestinationText != m_aStates.top().pDestinationText)
            break; // not for nested group
        m_aAuthor = m_aStates.top().pDestinationText->makeStringAndClear();
        break;
    case Destination::ATNID:
        if (&m_aStates.top().aDestinationText != m_aStates.top().pDestinationText)
            break; // not for nested group
        m_aAuthorInitials = m_aStates.top().pDestinationText->makeStringAndClear();
        break;
    case Destination::ANNOTATIONREFERENCESTART:
    case Destination::ANNOTATIONREFERENCEEND:
    {
        if (&m_aStates.top().aDestinationText != m_aStates.top().pDestinationText)
            break; // not for nested group
        OUString aStr = m_aStates.top().pDestinationText->makeStringAndClear();
        auto pValue = std::make_shared<RTFValue>(aStr.toInt32());
        RTFSprms aAttributes;
        if (aState.eDestination == Destination::ANNOTATIONREFERENCESTART)
            aAttributes.set(NS_ooxml::LN_EG_RangeMarkupElements_commentRangeStart, pValue);
        else
            aAttributes.set(NS_ooxml::LN_EG_RangeMarkupElements_commentRangeEnd, pValue);
        writerfilter::Reference<Properties>::Pointer_t pProperties = std::make_shared<RTFReferenceProperties>(aAttributes);
        Mapper().props(pProperties);
    }
    break;
    case Destination::ANNOTATIONREFERENCE:
    {
        if (&m_aStates.top().aDestinationText != m_aStates.top().pDestinationText)
            break; // not for nested group
        OUString aStr = m_aStates.top().pDestinationText->makeStringAndClear();
        RTFSprms aAnnAttributes;
        aAnnAttributes.set(NS_ooxml::LN_CT_Markup_id, std::make_shared<RTFValue>(aStr.toInt32()));
        Mapper().props(std::make_shared<RTFReferenceProperties>(aAnnAttributes));
    }
    break;
    case Destination::FALT:
    {
        if (&m_aStates.top().aDestinationText != m_aStates.top().pDestinationText)
            break; // not for nested group
        OUString aStr(m_aStates.top().pDestinationText->makeStringAndClear());
        auto pValue = std::make_shared<RTFValue>(aStr);
        aState.aTableSprms.set(NS_ooxml::LN_CT_Font_altName, pValue);
    }
    break;
    case Destination::DRAWINGOBJECT:
        if (m_aStates.top().aDrawingObject.xShape.is())
        {
            RTFDrawingObject& rDrawing = m_aStates.top().aDrawingObject;
            uno::Reference<drawing::XShape> xShape(rDrawing.xShape);
            uno::Reference<beans::XPropertySet> xPropertySet(rDrawing.xPropertySet);

            uno::Reference<lang::XServiceInfo> xServiceInfo(xShape, uno::UNO_QUERY);
            bool bTextFrame = xServiceInfo->supportsService("com.sun.star.text.TextFrame");

            // The default is certainly not inline, but then what Word supports is just at-character.
            xPropertySet->setPropertyValue("AnchorType", uno::makeAny(text::TextContentAnchorType_AT_CHARACTER));

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
            {
                uno::Any aLineColor = uno::makeAny(sal_uInt32((rDrawing.nLineColorR<<16) + (rDrawing.nLineColorG<<8) + rDrawing.nLineColorB));
                uno::Any aLineWidth;
                RTFSdrImport::resolveLineColorAndWidth(bTextFrame, xPropertySet, aLineColor, aLineWidth);
            }
            if (rDrawing.bHasFillColor)
                xPropertySet->setPropertyValue("FillColor", uno::makeAny(sal_uInt32((rDrawing.nFillColorR<<16) + (rDrawing.nFillColorG<<8) + rDrawing.nFillColorB)));
            else if (!bTextFrame)
                // If there is no fill, the Word default is 100% transparency.
                xPropertySet->setPropertyValue("FillTransparence", uno::makeAny(sal_Int32(100)));

            RTFSdrImport::resolveFLine(xPropertySet, rDrawing.nFLine);

            if (!m_aStates.top().aDrawingObject.bHadShapeText)
            {
                Mapper().startShape(xShape);
            }
            Mapper().endShape();
        }
        break;
    case Destination::PICT:
        // fdo#79319 ignore picture data if it's really a shape
        if (!m_pSdrImport->isFakePict())
        {
            resolvePict(true, m_pSdrImport->getCurrentShape());
        }
        m_bNeedFinalPar = true;
        break;
    case Destination::SHAPE:
        m_bNeedFinalPar = true;
        m_bNeedCr = m_bNeedCrOrig;
        if (aState.aFrame.inFrame())
        {
            // parBreak() modifies m_aStates.top() so we can't apply resetFrame() directly on aState
            resetFrame();
            parBreak();
            // Save this state for later use, so we only reset frame status only for the first shape inside a frame.
            aState = m_aStates.top();
            m_bNeedPap = true;
        }
        break;
    case Destination::MOMATH:
    {
        m_aMathBuffer.appendClosingTag(M_TOKEN(oMath));

        SvGlobalName aGlobalName(SO3_SM_CLASSID);
        comphelper::EmbeddedObjectContainer aContainer;
        OUString aName;
        uno::Reference<embed::XEmbeddedObject> xObject = aContainer.CreateEmbeddedObject(aGlobalName.GetByteSequence(), aName);
        uno::Reference<util::XCloseable> xComponent(xObject->getComponent(), uno::UNO_QUERY_THROW);
        // gcc4.4 (and 4.3 and possibly older) have a problem with dynamic_cast directly to the target class,
        // so help it with an intermediate cast. I'm not sure what exactly the problem is, seems to be unrelated
        // to RTLD_GLOBAL, so most probably a gcc bug.
        oox::FormulaImportBase& rImport = dynamic_cast<oox::FormulaImportBase&>(dynamic_cast<SfxBaseModel&>(*xComponent.get()));
        rImport.readFormulaOoxml(m_aMathBuffer);
        auto pValue = std::make_shared<RTFValue>(xObject);
        RTFSprms aMathAttributes;
        aMathAttributes.set(NS_ooxml::LN_starmath, pValue);
        writerfilter::Reference<Properties>::Pointer_t pProperties = std::make_shared<RTFReferenceProperties>(aMathAttributes);
        Mapper().props(pProperties);
        m_aMathBuffer = oox::formulaimport::XmlStreamBuilder();
    }
    break;
    case Destination::MR:
        lcl_DestinationToMath(*m_aStates.top().pDestinationText, m_aMathBuffer, m_bMathNor);
        break;
    case Destination::MF:
        m_aMathBuffer.appendClosingTag(M_TOKEN(f));
        break;
    case Destination::MFPR:
        m_aMathBuffer.appendClosingTag(M_TOKEN(fPr));
        break;
    case Destination::MCTRLPR:
        m_aMathBuffer.appendClosingTag(M_TOKEN(ctrlPr));
        break;
    case Destination::MNUM:
        m_aMathBuffer.appendClosingTag(M_TOKEN(num));
        break;
    case Destination::MDEN:
        m_aMathBuffer.appendClosingTag(M_TOKEN(den));
        break;
    case Destination::MACC:
        m_aMathBuffer.appendClosingTag(M_TOKEN(acc));
        break;
    case Destination::MACCPR:
        m_aMathBuffer.appendClosingTag(M_TOKEN(accPr));
        break;
    case Destination::MCHR:
    case Destination::MPOS:
    case Destination::MVERTJC:
    case Destination::MSTRIKEH:
    case Destination::MDEGHIDE:
    case Destination::MBEGCHR:
    case Destination::MSEPCHR:
    case Destination::MENDCHR:
    case Destination::MSUBHIDE:
    case Destination::MSUPHIDE:
    case Destination::MTYPE:
    case Destination::MGROW:
    {
        sal_Int32 nMathToken = 0;
        switch (aState.eDestination)
        {
        case Destination::MCHR:
            nMathToken = M_TOKEN(chr);
            break;
        case Destination::MPOS:
            nMathToken = M_TOKEN(pos);
            break;
        case Destination::MVERTJC:
            nMathToken = M_TOKEN(vertJc);
            break;
        case Destination::MSTRIKEH:
            nMathToken = M_TOKEN(strikeH);
            break;
        case Destination::MDEGHIDE:
            nMathToken = M_TOKEN(degHide);
            break;
        case Destination::MBEGCHR:
            nMathToken = M_TOKEN(begChr);
            break;
        case Destination::MSEPCHR:
            nMathToken = M_TOKEN(sepChr);
            break;
        case Destination::MENDCHR:
            nMathToken = M_TOKEN(endChr);
            break;
        case Destination::MSUBHIDE:
            nMathToken = M_TOKEN(subHide);
            break;
        case Destination::MSUPHIDE:
            nMathToken = M_TOKEN(supHide);
            break;
        case Destination::MTYPE:
            nMathToken = M_TOKEN(type);
            break;
        case Destination::MGROW:
            nMathToken = M_TOKEN(grow);
            break;
        default:
            break;
        }

        oox::formulaimport::XmlStream::AttributeList aAttribs;
        aAttribs[M_TOKEN(val)] = m_aStates.top().pDestinationText->makeStringAndClear();
        m_aMathBuffer.appendOpeningTag(nMathToken, aAttribs);
        m_aMathBuffer.appendClosingTag(nMathToken);
    }
    break;
    case Destination::ME:
        m_aMathBuffer.appendClosingTag(M_TOKEN(e));
        break;
    case Destination::MBAR:
        m_aMathBuffer.appendClosingTag(M_TOKEN(bar));
        break;
    case Destination::MBARPR:
        m_aMathBuffer.appendClosingTag(M_TOKEN(barPr));
        break;
    case Destination::MD:
        m_aMathBuffer.appendClosingTag(M_TOKEN(d));
        break;
    case Destination::MDPR:
        m_aMathBuffer.appendClosingTag(M_TOKEN(dPr));
        break;
    case Destination::MFUNC:
        m_aMathBuffer.appendClosingTag(M_TOKEN(func));
        break;
    case Destination::MFUNCPR:
        m_aMathBuffer.appendClosingTag(M_TOKEN(funcPr));
        break;
    case Destination::MFNAME:
        m_aMathBuffer.appendClosingTag(M_TOKEN(fName));
        break;
    case Destination::MLIMLOW:
        m_aMathBuffer.appendClosingTag(M_TOKEN(limLow));
        break;
    case Destination::MLIMLOWPR:
        m_aMathBuffer.appendClosingTag(M_TOKEN(limLowPr));
        break;
    case Destination::MLIM:
        m_aMathBuffer.appendClosingTag(M_TOKEN(lim));
        break;
    case Destination::MM:
        m_aMathBuffer.appendClosingTag(M_TOKEN(m));
        break;
    case Destination::MMPR:
        m_aMathBuffer.appendClosingTag(M_TOKEN(mPr));
        break;
    case Destination::MMR:
        m_aMathBuffer.appendClosingTag(M_TOKEN(mr));
        break;
    case Destination::MNARY:
        m_aMathBuffer.appendClosingTag(M_TOKEN(nary));
        break;
    case Destination::MNARYPR:
        m_aMathBuffer.appendClosingTag(M_TOKEN(naryPr));
        break;
    case Destination::MSUB:
        m_aMathBuffer.appendClosingTag(M_TOKEN(sub));
        break;
    case Destination::MSUP:
        m_aMathBuffer.appendClosingTag(M_TOKEN(sup));
        break;
    case Destination::MLIMUPP:
        m_aMathBuffer.appendClosingTag(M_TOKEN(limUpp));
        break;
    case Destination::MLIMUPPPR:
        m_aMathBuffer.appendClosingTag(M_TOKEN(limUppPr));
        break;
    case Destination::MGROUPCHR:
        m_aMathBuffer.appendClosingTag(M_TOKEN(groupChr));
        break;
    case Destination::MGROUPCHRPR:
        m_aMathBuffer.appendClosingTag(M_TOKEN(groupChrPr));
        break;
    case Destination::MBORDERBOX:
        m_aMathBuffer.appendClosingTag(M_TOKEN(borderBox));
        break;
    case Destination::MBORDERBOXPR:
        m_aMathBuffer.appendClosingTag(M_TOKEN(borderBoxPr));
        break;
    case Destination::MRAD:
        m_aMathBuffer.appendClosingTag(M_TOKEN(rad));
        break;
    case Destination::MRADPR:
        m_aMathBuffer.appendClosingTag(M_TOKEN(radPr));
        break;
    case Destination::MDEG:
        m_aMathBuffer.appendClosingTag(M_TOKEN(deg));
        break;
    case Destination::MSSUB:
        m_aMathBuffer.appendClosingTag(M_TOKEN(sSub));
        break;
    case Destination::MSSUBPR:
        m_aMathBuffer.appendClosingTag(M_TOKEN(sSubPr));
        break;
    case Destination::MSSUP:
        m_aMathBuffer.appendClosingTag(M_TOKEN(sSup));
        break;
    case Destination::MSSUPPR:
        m_aMathBuffer.appendClosingTag(M_TOKEN(sSupPr));
        break;
    case Destination::MSSUBSUP:
        m_aMathBuffer.appendClosingTag(M_TOKEN(sSubSup));
        break;
    case Destination::MSSUBSUPPR:
        m_aMathBuffer.appendClosingTag(M_TOKEN(sSubSupPr));
        break;
    case Destination::MSPRE:
        m_aMathBuffer.appendClosingTag(M_TOKEN(sPre));
        break;
    case Destination::MSPREPR:
        m_aMathBuffer.appendClosingTag(M_TOKEN(sPrePr));
        break;
    case Destination::MBOX:
        m_aMathBuffer.appendClosingTag(M_TOKEN(box));
        break;
    case Destination::MEQARR:
        m_aMathBuffer.appendClosingTag(M_TOKEN(eqArr));
        break;
    case Destination::SHAPEGROUP:
        if (aState.bCreatedShapeGroup)
            m_pSdrImport->popParent();
        break;
    case Destination::PROPNAME:
        if (&m_aStates.top().aDestinationText != m_aStates.top().pDestinationText)
            break; // not for nested group
        aState.aPropName = m_aStates.top().pDestinationText->makeStringAndClear();
        break;
    case Destination::STATICVAL:
        if (&m_aStates.top().aDestinationText != m_aStates.top().pDestinationText)
            break; // not for nested group
        if (m_xDocumentProperties.is())
        {
            // Find out what is the key, value type and value we want to set.
            uno::Reference<beans::XPropertyContainer> xPropertyContainer = m_xDocumentProperties->getUserDefinedProperties();
            uno::Reference<beans::XPropertySet> xPropertySet(xPropertyContainer, uno::UNO_QUERY);
            const OUString& rKey = m_aStates.top().aPropName;
            OUString aStaticVal = m_aStates.top().pDestinationText->makeStringAndClear();
            uno::Any aAny;
            if (m_aStates.top().aPropType == cppu::UnoType<OUString>::get())
                aAny = uno::makeAny(aStaticVal);

            xPropertyContainer->addProperty(rKey, beans::PropertyAttribute::REMOVABLE, aAny);
        }
        break;
    case Destination::USERPROPS:
    {
        // These are the imported properties.
        uno::Reference<document::XDocumentProperties> xDocumentProperties = m_xDocumentProperties;

        // These are the real document properties.
        uno::Reference<document::XDocumentPropertiesSupplier> xDocumentPropertiesSupplier(m_xDstDoc, uno::UNO_QUERY);
        if (xDocumentPropertiesSupplier.is())
            m_xDocumentProperties.set(xDocumentPropertiesSupplier->getDocumentProperties(), uno::UNO_QUERY);

        if (m_xDocumentProperties.is())
        {
            if (!m_bIsNewDoc)
            {
                // Check classification.
                if (!SfxClassificationHelper::ShowPasteInfo(SfxClassificationHelper::CheckPaste(xDocumentProperties, m_xDocumentProperties)))
                    return RTFError::CLASSIFICATION;
            }

            uno::Reference<beans::XPropertyContainer> xClipboardPropertyContainer = xDocumentProperties->getUserDefinedProperties();
            uno::Reference<beans::XPropertyContainer> xDocumentPropertyContainer = m_xDocumentProperties->getUserDefinedProperties();
            uno::Reference<beans::XPropertySet> xClipboardPropertySet(xClipboardPropertyContainer, uno::UNO_QUERY);
            uno::Reference<beans::XPropertySet> xDocumentPropertySet(xDocumentPropertyContainer, uno::UNO_QUERY);
            uno::Sequence<beans::Property> aClipboardProperties = xClipboardPropertySet->getPropertySetInfo()->getProperties();
            uno::Sequence<beans::Property> aDocumentProperties = xDocumentPropertySet->getPropertySetInfo()->getProperties();

            for (const beans::Property& rProperty : aClipboardProperties)
            {
                const OUString& rKey = rProperty.Name;
                uno::Any aValue = xClipboardPropertySet->getPropertyValue(rKey);

                try
                {
                    if (lcl_containsProperty(aDocumentProperties, rKey))
                    {
                        // When pasting, don't update existing properties.
                        if (!m_bIsNewDoc)
                            xDocumentPropertySet->setPropertyValue(rKey, aValue);
                    }
                    else
                        xDocumentPropertyContainer->addProperty(rKey, beans::PropertyAttribute::REMOVABLE, aValue);
                }
                catch (const uno::Exception& rException)
                {
                    SAL_WARN("writerfilter", "failed to set property " << rKey << ": " << rException.Message);
                }
            }
        }
    }
    break;
    default:
        break;
    }

    // See if we need to end a track change
    if (aState.bStartedTrackchange)
    {
        RTFSprms aTCSprms;
        auto pValue = std::make_shared<RTFValue>(0);
        aTCSprms.set(NS_ooxml::LN_endtrackchange, pValue);
        if (!m_aStates.top().pCurrentBuffer)
            Mapper().props(std::make_shared<RTFReferenceProperties>(RTFSprms(), aTCSprms));
        else
            m_aStates.top().pCurrentBuffer->push_back(Buf_t(BUFFER_PROPS, std::make_shared<RTFValue>(RTFSprms(), aTCSprms), nullptr));
    }

    // This is the end of the doc, see if we need to close the last section.
    if (m_pTokenizer->getGroup() == 1 && !m_bFirstRun)
    {
        // \par means an empty paragraph at the end of footnotes/endnotes, but
        // not in case of other substreams, like headers.
        if (m_bNeedCr && !(m_nStreamType == NS_ooxml::LN_footnote || m_nStreamType == NS_ooxml::LN_endnote) && m_bIsNewDoc)
            dispatchSymbol(RTF_PAR);
        if (m_bNeedSect) // may be set by dispatchSymbol above!
            sectBreak(true);
    }

    m_aStates.pop();

    m_pTokenizer->popGroup();

    // list table
    switch (aState.eDestination)
    {
    case Destination::LISTENTRY:
    {
        auto pValue = std::make_shared<RTFValue>(aState.aTableAttributes, aState.aTableSprms);
        m_aListTableSprms.set(NS_ooxml::LN_CT_Numbering_abstractNum, pValue, RTFOverwrite::NO_APPEND);
    }
    break;
    case Destination::PARAGRAPHNUMBERING:
    {
        RTFValue::Pointer_t pIdValue = aState.aTableAttributes.find(NS_ooxml::LN_CT_AbstractNum_nsid);
        if (pIdValue.get() && !m_aStates.empty())
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
            auto pTextValue = std::make_shared<RTFValue>(aTextValue);
            aLeveltextAttributes.set(NS_ooxml::LN_CT_LevelText_val, pTextValue);

            RTFSprms aLevelAttributes;
            RTFSprms aLevelSprms;
            auto pIlvlValue = std::make_shared<RTFValue>(0);
            aLevelAttributes.set(NS_ooxml::LN_CT_Lvl_ilvl, pIlvlValue);

            RTFValue::Pointer_t pFmtValue = aState.aTableSprms.find(NS_ooxml::LN_CT_Lvl_numFmt);
            if (pFmtValue.get())
                aLevelSprms.set(NS_ooxml::LN_CT_Lvl_numFmt, pFmtValue);

            RTFValue::Pointer_t pStartatValue = aState.aTableSprms.find(NS_ooxml::LN_CT_Lvl_start);
            if (pStartatValue.get())
                aLevelSprms.set(NS_ooxml::LN_CT_Lvl_start, pStartatValue);

            auto pLeveltextValue = std::make_shared<RTFValue>(aLeveltextAttributes);
            aLevelSprms.set(NS_ooxml::LN_CT_Lvl_lvlText, pLeveltextValue);
            RTFValue::Pointer_t pRunProps = aState.aTableSprms.find(NS_ooxml::LN_CT_Lvl_rPr);
            if (pRunProps.get())
                aLevelSprms.set(NS_ooxml::LN_CT_Lvl_rPr, pRunProps);

            RTFSprms aAbstractAttributes;
            RTFSprms aAbstractSprms;
            aAbstractAttributes.set(NS_ooxml::LN_CT_AbstractNum_abstractNumId, pIdValue);
            auto pLevelValue = std::make_shared<RTFValue>(aLevelAttributes, aLevelSprms);
            aAbstractSprms.set(NS_ooxml::LN_CT_AbstractNum_lvl, pLevelValue, RTFOverwrite::NO_APPEND);

            RTFSprms aListTableSprms;
            auto pAbstractValue = std::make_shared<RTFValue>(aAbstractAttributes, aAbstractSprms);
            // It's important that Numbering_abstractNum and Numbering_num never overwrites previous values.
            aListTableSprms.set(NS_ooxml::LN_CT_Numbering_abstractNum, pAbstractValue, RTFOverwrite::NO_APPEND);

            // Numbering
            RTFSprms aNumberingAttributes;
            RTFSprms aNumberingSprms;
            aNumberingAttributes.set(NS_ooxml::LN_CT_AbstractNum_nsid, pIdValue);
            aNumberingSprms.set(NS_ooxml::LN_CT_Num_abstractNumId, pIdValue);
            auto pNumberingValue = std::make_shared<RTFValue>(aNumberingAttributes, aNumberingSprms);
            aListTableSprms.set(NS_ooxml::LN_CT_Numbering_num, pNumberingValue, RTFOverwrite::NO_APPEND);

            // Table
            RTFSprms aListTableAttributes;
            writerfilter::Reference<Properties>::Pointer_t pProp = std::make_shared<RTFReferenceProperties>(aListTableAttributes, aListTableSprms);

            RTFReferenceTable::Entries_t aListTableEntries;
            aListTableEntries.insert(std::make_pair(0, pProp));
            writerfilter::Reference<Table>::Pointer_t const pTable(new RTFReferenceTable(aListTableEntries));
            Mapper().table(NS_ooxml::LN_NUMBERING, pTable);

            // Use it
            putNestedSprm(m_aStates.top().aParagraphSprms, NS_ooxml::LN_CT_PPrBase_numPr, NS_ooxml::LN_CT_NumPr_ilvl, pIlvlValue);
            putNestedSprm(m_aStates.top().aParagraphSprms, NS_ooxml::LN_CT_PPrBase_tabs, NS_ooxml::LN_CT_NumPr_numId, pIdValue);
        }
    }
    break;
    case Destination::PARAGRAPHNUMBERING_TEXTAFTER:
        if (!m_aStates.empty())
        {
            // FIXME: don't use pDestinationText, points to popped state
            auto pValue = std::make_shared<RTFValue>(aState.aDestinationText.makeStringAndClear(), true);
            m_aStates.top().aTableAttributes.set(NS_ooxml::LN_CT_LevelSuffix_val, pValue);
        }
        break;
    case Destination::PARAGRAPHNUMBERING_TEXTBEFORE:
        if (!m_aStates.empty())
        {
            // FIXME: don't use pDestinationText, points to popped state
            auto pValue = std::make_shared<RTFValue>(aState.aDestinationText.makeStringAndClear(), true);
            m_aStates.top().aTableAttributes.set(NS_ooxml::LN_CT_LevelText_val, pValue);
        }
        break;
    case Destination::LISTNAME:
        break;
    case Destination::LISTLEVEL:
        if (!m_aStates.empty())
        {
            auto pInnerValue = std::make_shared<RTFValue>(m_aStates.top().nListLevelNum++);
            aState.aTableAttributes.set(NS_ooxml::LN_CT_Lvl_ilvl, pInnerValue);

            auto pValue = std::make_shared<RTFValue>(aState.aTableAttributes, aState.aTableSprms);
            if (m_aStates.top().eDestination != Destination::LFOLEVEL)
                m_aStates.top().aListLevelEntries.set(NS_ooxml::LN_CT_AbstractNum_lvl, pValue, RTFOverwrite::NO_APPEND);
            else
                m_aStates.top().aTableSprms.set(NS_ooxml::LN_CT_NumLvl_lvl, pValue);
        }
        break;
    case Destination::LFOLEVEL:
        if (!m_aStates.empty())
        {
            auto pInnerValue = std::make_shared<RTFValue>(m_aStates.top().nListLevelNum++);
            aState.aTableAttributes.set(NS_ooxml::LN_CT_NumLvl_ilvl, pInnerValue);

            auto pValue = std::make_shared<RTFValue>(aState.aTableAttributes, aState.aTableSprms);
            m_aStates.top().aTableSprms.set(NS_ooxml::LN_CT_Num_lvlOverride, pValue, RTFOverwrite::NO_APPEND);
        }
        break;
    // list override table
    case Destination::LISTOVERRIDEENTRY:
        if (!m_aStates.empty())
        {
            if (m_aStates.top().eDestination == Destination::LISTOVERRIDEENTRY)
            {
                // copy properties upwards so upper popState inserts it
                m_aStates.top().aTableAttributes = aState.aTableAttributes;
                m_aStates.top().aTableSprms = aState.aTableSprms;
            }
            else
            {
                auto pValue = std::make_shared<RTFValue>(aState.aTableAttributes, aState.aTableSprms);
                m_aListTableSprms.set(NS_ooxml::LN_CT_Numbering_num, pValue, RTFOverwrite::NO_APPEND);
            }
        }
        break;
    case Destination::LEVELTEXT:
        if (!m_aStates.empty())
        {
            auto pValue = std::make_shared<RTFValue>(aState.aTableAttributes);
            m_aStates.top().aTableSprms.set(NS_ooxml::LN_CT_Lvl_lvlText, pValue);
        }
        break;
    case Destination::LEVELNUMBERS:
        if (!m_aStates.empty())
            m_aStates.top().aTableSprms = aState.aTableSprms;
        break;
    case Destination::FIELDINSTRUCTION:
        if (!m_aStates.empty())
            m_aStates.top().eFieldStatus = RTFFieldStatus::INSTRUCTION;
        break;
    case Destination::FIELDRESULT:
        if (!m_aStates.empty())
            m_aStates.top().eFieldStatus = RTFFieldStatus::RESULT;
        break;
    case Destination::FIELD:
        if (aState.eFieldStatus == RTFFieldStatus::INSTRUCTION)
            singleChar(cFieldEnd);
        break;
    case Destination::SHAPEPROPERTYVALUEPICT:
        if (!m_aStates.empty())
        {
            m_aStates.top().aPicture = aState.aPicture;
            // both \sp and \sv are destinations, copy the text up-ward for later
            m_aStates.top().aDestinationText = aState.aDestinationText;
        }
        break;
    case Destination::FALT:
        if (!m_aStates.empty())
            m_aStates.top().aTableSprms = aState.aTableSprms;
        break;
    case Destination::SHAPEPROPERTYNAME:
    case Destination::SHAPEPROPERTYVALUE:
    case Destination::SHAPEPROPERTY:
        if (!m_aStates.empty())
        {
            m_aStates.top().aShape = aState.aShape;
            m_aStates.top().aPicture = aState.aPicture;
            m_aStates.top().aCharacterAttributes = aState.aCharacterAttributes;
        }
        break;
    case Destination::SHAPEINSTRUCTION:
        if (!m_aStates.empty() && m_aStates.top().eDestination == Destination::SHAPEINSTRUCTION)
        {
            // Shape instruction inside other shape instruction: just copy new shape settings:
            // it will be resolved on end of topmost shape instruction destination
            m_aStates.top().aShape = aState.aShape;
            m_aStates.top().aPicture = aState.aPicture;
            m_aStates.top().aCharacterSprms = aState.aCharacterSprms;
            m_aStates.top().aCharacterAttributes = aState.aCharacterAttributes;
        }
        break;
    case Destination::FLYMAINCONTENT:
    case Destination::SHPPICT:
    case Destination::SHAPE:
        if (!m_aStates.empty())
        {
            m_aStates.top().aFrame = aState.aFrame;
            if (aState.eDestination == Destination::SHPPICT && m_aStates.top().eDestination == Destination::LISTPICTURE)
            {
                RTFSprms aAttributes;
                aAttributes.set(NS_ooxml::LN_CT_NumPicBullet_numPicBulletId, std::make_shared<RTFValue>(m_nListPictureId++));
                RTFSprms aSprms;
                // Dummy value, real picture is already sent to dmapper.
                aSprms.set(NS_ooxml::LN_CT_NumPicBullet_pict, std::make_shared<RTFValue>(0));
                auto pValue = std::make_shared<RTFValue>(aAttributes, aSprms);
                m_aListTableSprms.set(NS_ooxml::LN_CT_Numbering_numPicBullet, pValue, RTFOverwrite::NO_APPEND);
            }
        }
        break;
    case Destination::SHAPETEXT:
        if (!m_aStates.empty())
        {
            // If we're leaving the shapetext group (it may have nested ones) and this is a shape, not an old drawingobject.
            if (m_aStates.top().eDestination != Destination::SHAPETEXT && !m_aStates.top().aDrawingObject.bHadShapeText)
            {
                m_aStates.top().bHadShapeText = true;
                if (!m_aStates.top().pCurrentBuffer)
                    m_pSdrImport->close();
                else
                    m_aStates.top().pCurrentBuffer->push_back(
                        Buf_t(BUFFER_ENDSHAPE, nullptr, nullptr));
            }

            // It's allowed to declare these inside the shape text, and they
            // are expected to have an effect for the whole shape.
            if (aState.aDrawingObject.nLeft)
                m_aStates.top().aDrawingObject.nLeft = aState.aDrawingObject.nLeft;
            if (aState.aDrawingObject.nTop)
                m_aStates.top().aDrawingObject.nTop = aState.aDrawingObject.nTop;
            if (aState.aDrawingObject.nRight)
                m_aStates.top().aDrawingObject.nRight = aState.aDrawingObject.nRight;
            if (aState.aDrawingObject.nBottom)
                m_aStates.top().aDrawingObject.nBottom = aState.aDrawingObject.nBottom;
        }
        break;
    case Destination::PROPNAME:
        if (m_aStates.top().eDestination == Destination::USERPROPS)
            m_aStates.top().aPropName = aState.aPropName;
        break;
    default:
    {
        if (!m_aStates.empty() && m_aStates.top().eDestination == Destination::PICT)
            m_aStates.top().aPicture = aState.aPicture;
    }
    break;
    }

    if (aState.pCurrentBuffer == &m_aSuperBuffer)
    {
        OSL_ASSERT(!m_aStates.empty() && m_aStates.top().pCurrentBuffer == nullptr);

        if (!m_aSuperBuffer.empty())
            replayBuffer(m_aSuperBuffer, nullptr, nullptr);
    }

    return RTFError::OK;
}

RTFError RTFDocumentImpl::handleEmbeddedObject()
{
    SvMemoryStream aStream;
    int b = 0, count = 2;

    // Feed the destination text to a stream.
    OString aStr = OUStringToOString(m_aStates.top().pDestinationText->makeStringAndClear(), RTL_TEXTENCODING_ASCII_US);
    const char* str = aStr.getStr();
    for (int i = 0; i < aStr.getLength(); ++i)
    {
        char ch = str[i];
        if (ch != 0x0d && ch != 0x0a)
        {
            b = b << 4;
            sal_Int8 parsed = RTFTokenizer::asHex(ch);
            if (parsed == -1)
                return RTFError::HEX_INVALID;
            b += parsed;
            count--;
            if (!count)
            {
                aStream.WriteChar(b);
                count = 2;
                b = 0;
            }
        }
    }

    std::unique_ptr<SvStream> pStream(new SvMemoryStream());

    // Skip ObjectHeader, see [MS-OLEDS] 2.2.4.
    if (aStream.Tell())
    {
        aStream.Seek(0);
        sal_uInt32 nData;
        aStream.ReadUInt32(nData);   // OLEVersion
        aStream.ReadUInt32(nData);   // FormatID
        aStream.ReadUInt32(nData);   // ClassName
        aStream.SeekRel(nData);
        aStream.ReadUInt32(nData);   // TopicName
        aStream.SeekRel(nData);
        aStream.ReadUInt32(nData);   // ItemName
        aStream.SeekRel(nData);
        aStream.ReadUInt32(nData);   // NativeDataSize

        if (nData)
        {
            pStream->WriteStream(aStream);
            pStream->Seek(0);
        }
    }

    uno::Reference<io::XInputStream> xInputStream(new utl::OSeekableInputStreamWrapper(pStream.release(), /*_bOwner=*/true));
    auto pStreamValue = std::make_shared<RTFValue>(xInputStream);
    m_aOLEAttributes.set(NS_ooxml::LN_inputstream, pStreamValue);

    return RTFError::OK;
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

Destination RTFDocumentImpl::getDestination()
{
    return m_aStates.top().eDestination;
}

void RTFDocumentImpl::setDestination(Destination eDestination)
{
    m_aStates.top().eDestination = eDestination;
}

// this is a questionably named method that is used only in a very special
// situation where it looks like the "current" buffer is needed?
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

RTFParserState::RTFParserState(RTFDocumentImpl* pDocumentImpl)
    : m_pDocumentImpl(pDocumentImpl),
      nInternalState(RTFInternalState::NORMAL),
      eDestination(Destination::NORMAL),
      eFieldStatus(RTFFieldStatus::NONE),
      nBorderState(RTFBorderState::NONE),
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
      eRunType(RunType::LOCH),
      isRightToLeft(false),
      nYear(0),
      nMonth(0),
      nDay(0),
      nHour(0),
      nMinute(0),
      pDestinationText(nullptr),
      nCurrentStyleIndex(-1),
      nCurrentCharacterStyleIndex(-1),
      pCurrentBuffer(nullptr),
      bInListpicture(false),
      bInBackground(false),
      bHadShapeText(false),
      bInShapeGroup(false),
      bInShape(false),
      bCreatedShapeGroup(false),
      bStartedTrackchange(false)
{
}

void RTFDocumentImpl::resetFrame()
{
    m_aStates.top().aFrame = RTFFrame(&m_aStates.top());
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
      eStyle(RTFBmpStyle::NONE)
{
}

RTFShape::RTFShape()
    : nLeft(0),
      nTop(0),
      nRight(0),
      nBottom(0),
      nHoriOrientRelation(0),
      nVertOrientRelation(0),
      nHoriOrientRelationToken(0),
      nVertOrientRelationToken(0),
      nWrap(-1),
      bInBackground(false)
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
      m_nX(0),
      m_nY(0),
      m_nW(0),
      m_nH(0),
      m_nHoriPadding(0),
      m_nVertPadding(0),
      m_nHoriAlign(0),
      m_nHoriAnchor(0),
      m_nVertAlign(0),
      m_nVertAnchor(0),
      m_nHRule(NS_ooxml::LN_Value_doc_ST_HeightRule_auto),
      m_nAnchorType(0)
{
}

void RTFFrame::setSprm(Id nId, Id nValue)
{
    if (m_pParserState->m_pDocumentImpl->getFirstRun() && !m_pParserState->m_pDocumentImpl->isStyleSheetImport())
    {
        m_pParserState->m_pDocumentImpl->checkFirstRun();
        m_pParserState->m_pDocumentImpl->setNeedPar(false);
    }
    switch (nId)
    {
    case NS_ooxml::LN_CT_FramePr_w:
        m_nW = nValue;
        break;
    case NS_ooxml::LN_CT_FramePr_h:
        m_nH = nValue;
        break;
    case NS_ooxml::LN_CT_FramePr_x:
        m_nX = nValue;
        break;
    case NS_ooxml::LN_CT_FramePr_y:
        m_nY = nValue;
        break;
    case NS_ooxml::LN_CT_FramePr_hSpace:
        m_nHoriPadding = nValue;
        break;
    case NS_ooxml::LN_CT_FramePr_vSpace:
        m_nVertPadding = nValue;
        break;
    case NS_ooxml::LN_CT_FramePr_xAlign:
        m_nHoriAlign = nValue;
        break;
    case NS_ooxml::LN_CT_FramePr_hAnchor:
        m_nHoriAnchor = nValue;
        break;
    case NS_ooxml::LN_CT_FramePr_yAlign:
        m_nVertAlign = nValue;
        break;
    case NS_ooxml::LN_CT_FramePr_vAnchor:
        m_nVertAnchor = nValue;
        break;
    case NS_ooxml::LN_CT_FramePr_wrap:
        m_oWrap = nValue;
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
        NS_ooxml::LN_CT_FramePr_h,
        NS_ooxml::LN_CT_FramePr_w,
        NS_ooxml::LN_CT_FramePr_hSpace,
        NS_ooxml::LN_CT_FramePr_vSpace,
        NS_ooxml::LN_CT_FramePr_hAnchor,
        NS_ooxml::LN_CT_FramePr_vAnchor,
        NS_ooxml::LN_CT_FramePr_xAlign,
        NS_ooxml::LN_CT_FramePr_yAlign,
        NS_ooxml::LN_CT_FramePr_wrap,
        NS_ooxml::LN_CT_FramePr_dropCap,
        NS_ooxml::LN_CT_FramePr_lines
    };

    for (int i = 0, len = SAL_N_ELEMENTS(pNames); i < len; ++i)
    {
        Id nId = pNames[i];
        RTFValue::Pointer_t pValue;

        switch (nId)
        {
        case NS_ooxml::LN_CT_FramePr_x:
            if (m_nX != 0)
                pValue = std::make_shared<RTFValue>(m_nX);
            break;
        case NS_ooxml::LN_CT_FramePr_y:
            if (m_nY != 0)
                pValue = std::make_shared<RTFValue>(m_nY);
            break;
        case NS_ooxml::LN_CT_FramePr_h:
            if (m_nH != 0)
            {
                if (m_nHRule == NS_ooxml::LN_Value_doc_ST_HeightRule_exact)
                    pValue = std::make_shared<RTFValue>(-m_nH); // The negative value just sets nHRule
                else
                    pValue = std::make_shared<RTFValue>(m_nH);
            }
            break;
        case NS_ooxml::LN_CT_FramePr_w:
            if (m_nW != 0)
                pValue = std::make_shared<RTFValue>(m_nW);
            break;
        case NS_ooxml::LN_CT_FramePr_hSpace:
            if (m_nHoriPadding != 0)
                pValue = std::make_shared<RTFValue>(m_nHoriPadding);
            break;
        case NS_ooxml::LN_CT_FramePr_vSpace:
            if (m_nVertPadding != 0)
                pValue = std::make_shared<RTFValue>(m_nVertPadding);
            break;
        case NS_ooxml::LN_CT_FramePr_hAnchor:
        {
            if (m_nHoriAnchor == 0)
                m_nHoriAnchor = NS_ooxml::LN_Value_doc_ST_HAnchor_margin;
            pValue = std::make_shared<RTFValue>(m_nHoriAnchor);
        }
        break;
        case NS_ooxml::LN_CT_FramePr_vAnchor:
        {
            if (m_nVertAnchor == 0)
                m_nVertAnchor = NS_ooxml::LN_Value_doc_ST_VAnchor_margin;
            pValue = std::make_shared<RTFValue>(m_nVertAnchor);
        }
        break;
        case NS_ooxml::LN_CT_FramePr_xAlign:
            pValue = std::make_shared<RTFValue>(m_nHoriAlign);
            break;
        case NS_ooxml::LN_CT_FramePr_yAlign:
            pValue = std::make_shared<RTFValue>(m_nVertAlign);
            break;
        case NS_ooxml::LN_CT_FramePr_hRule:
        {
            if (m_nH < 0)
                m_nHRule = NS_ooxml::LN_Value_doc_ST_HeightRule_exact;
            else if (m_nH > 0)
                m_nHRule = NS_ooxml::LN_Value_doc_ST_HeightRule_atLeast;
            pValue = std::make_shared<RTFValue>(m_nHRule);
        }
        break;
        case NS_ooxml::LN_CT_FramePr_wrap:
            if (m_oWrap)
                pValue = std::make_shared<RTFValue>(*m_oWrap);
            break;
        default:
            break;
        }

        if (pValue.get())
            sprms.set(nId, pValue);
    }

    RTFSprms frameprSprms;
    frameprSprms.set(NS_ooxml::LN_CT_PPrBase_framePr, std::make_shared<RTFValue>(sprms));
    return frameprSprms;
}

bool RTFFrame::hasProperties()
{
    return m_nX != 0 || m_nY != 0 || m_nW != 0 || m_nH != 0 ||
           m_nHoriPadding != 0 || m_nVertPadding != 0 ||
           m_nHoriAlign != 0 || m_nHoriAnchor != 0 || m_nVertAlign != 0 || m_nVertAnchor != 0 ||
           m_nAnchorType != 0;
}

} // namespace rtftok
} // namespace writerfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
