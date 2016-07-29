/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/beans/PropertyAttribute.hpp>
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
#include <filter/msfilter/util.hxx>
#include <comphelper/string.hxx>
#include <tools/globname.hxx>
#include <tools/datetimeutils.hxx>
#include <comphelper/classids.hxx>
#include <comphelper/embeddedobjectcontainer.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <comphelper/sequence.hxx>
#include <sfx2/sfxbasemodel.hxx>
#include <oox/mathml/import.hxx>
#include <ooxml/resourceids.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/drawingml/drawingmltypes.hxx>
#include <rtl/uri.hxx>
#include <dmapper/DomainMapper_Impl.hxx>
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

static Id lcl_getParagraphBorder(sal_uInt32 nIndex)
{
    static const Id aBorderIds[] =
    {
        NS_ooxml::LN_CT_PBdr_top, NS_ooxml::LN_CT_PBdr_left, NS_ooxml::LN_CT_PBdr_bottom, NS_ooxml::LN_CT_PBdr_right
    };

    return aBorderIds[nIndex];
}

static void lcl_putNestedAttribute(RTFSprms& rSprms, Id nParent, Id nId, RTFValue::Pointer_t pValue,
                                   RTFOverwrite eOverwrite = RTFOverwrite::YES, bool bAttribute = true)
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

static void lcl_putNestedSprm(RTFSprms& rSprms, Id nParent, Id nId, RTFValue::Pointer_t pValue, RTFOverwrite eOverwrite = RTFOverwrite::NO_APPEND)
{
    lcl_putNestedAttribute(rSprms, nParent, nId, pValue, eOverwrite, false);
}

static RTFValue::Pointer_t lcl_getNestedAttribute(RTFSprms& rSprms, Id nParent, Id nId)
{
    RTFValue::Pointer_t pParent = rSprms.find(nParent);
    if (!pParent)
        return RTFValue::Pointer_t();
    RTFSprms& rAttributes = pParent->getAttributes();
    return rAttributes.find(nId);
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

static void
lcl_putBorderProperty(RTFStack& aStates, Id nId, RTFValue::Pointer_t pValue)
{
    RTFSprms* pAttributes = nullptr;
    if (aStates.top().nBorderState == RTFBorderState::PARAGRAPH_BOX)
        for (int i = 0; i < 4; i++)
        {
            RTFValue::Pointer_t p = aStates.top().aParagraphSprms.find(lcl_getParagraphBorder(i));
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
        pAttributes = &lcl_getLastAttributes(aStates.top().aParagraphSprms, NS_ooxml::LN_CT_PrBase_pBdr);
    else if (aStates.top().nBorderState == RTFBorderState::CELL)
        pAttributes = &lcl_getLastAttributes(aStates.top().aTableCellSprms, NS_ooxml::LN_CT_TcPrBase_tcBorders);
    else if (aStates.top().nBorderState == RTFBorderState::PAGE)
        pAttributes = &lcl_getLastAttributes(aStates.top().aSectionSprms, NS_ooxml::LN_EG_SectPrContents_pgBorders);
    if (pAttributes)
        pAttributes->set(nId, pValue);
}

static OString lcl_DTTM22OString(long lDTTM)
{
    return DateTimeToOString(msfilter::util::DTTM2DateTime(lDTTM));
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

static const char* lcl_RtfToString(RTFKeyword nKeyword)
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

RTFDocumentImpl::~RTFDocumentImpl()
{
}

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
        setNeedSect(); // first call that succeeds

        // set the requested default font, if there are none
        RTFValue::Pointer_t pFont = lcl_getNestedAttribute(m_aDefaultState.aCharacterSprms, NS_ooxml::LN_EG_RPrBase_rFonts, NS_ooxml::LN_CT_Fonts_ascii);
        RTFValue::Pointer_t pCurrentFont = lcl_getNestedAttribute(m_aStates.top().aCharacterSprms, NS_ooxml::LN_EG_RPrBase_rFonts, NS_ooxml::LN_CT_Fonts_ascii);
        if (pFont && !pCurrentFont)
            lcl_putNestedAttribute(m_aStates.top().aCharacterSprms, NS_ooxml::LN_EG_RPrBase_rFonts, NS_ooxml::LN_CT_Fonts_ascii, pFont);
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
    for (RTFSprms::Iterator_t it = rProps.getSprms().begin(); it != rProps.getSprms().end(); ++it)
    {
        // createStyleProperties() puts properties to rPr, but here we need a flat list.
        if (it->first == NS_ooxml::LN_CT_Style_rPr)
        {
            // rPr can have both attributes and SPRMs, copy over both types.
            RTFSprms& rRPrSprms = it->second->getSprms();
            for (RTFSprms::Iterator_t itRPrSprm = rRPrSprms.begin(); itRPrSprm != rRPrSprms.end(); ++itRPrSprm)
                rStyleSprms.set(itRPrSprm->first, itRPrSprm->second);

            RTFSprms& rRPrAttributes = it->second->getAttributes();
            for (RTFSprms::Iterator_t itRPrAttribute = rRPrAttributes.begin(); itRPrAttribute != rRPrAttributes.end(); ++itRPrAttribute)
                rStyleAttributes.set(itRPrAttribute->first, itRPrAttribute->second);
        }
        else
            rStyleSprms.set(it->first, it->second);
    }

    RTFSprms& rAttributes = rProps.getAttributes();
    for (RTFSprms::Iterator_t itAttr = rAttributes.begin(); itAttr != rAttributes.end(); ++itAttr)
        rStyleAttributes.set(itAttr->first, itAttr->second);
}

writerfilter::Reference<Properties>::Pointer_t RTFDocumentImpl::getProperties(RTFSprms& rAttributes, RTFSprms& rSprms)
{
    int nStyle = 0;
    if (!m_aStates.empty())
        nStyle = m_aStates.top().nCurrentStyleIndex;
    RTFReferenceTable::Entries_t::iterator it = m_aStyleTableEntries.find(nStyle);
    if (it != m_aStyleTableEntries.end())
    {
        RTFReferenceProperties& rProps = *static_cast<RTFReferenceProperties*>(it->second.get());

        // cloneAndDeduplicate() wants to know about only a single "style", so
        // let's merge paragraph and character style properties here.
        RTFReferenceTable::Entries_t::iterator itChar = m_aStyleTableEntries.end();
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
                Buf_t(BUFFER_PROPS, pValue));
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
        m_aStates.top().pCurrentBuffer->push_back(Buf_t(BUFFER_PROPS, pValue));
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

void RTFDocumentImpl::sectBreak(bool bFinal = false)
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
        std::map<int, rtl_TextEncoding>::iterator it = m_aFontEncodings.find(nFontIndex);
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
    }
}

RTFError RTFDocumentImpl::resolvePict(bool const bInline, uno::Reference<drawing::XShape> const& i_xShape)
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
                    return RTFError::HEX_INVALID;
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
        return RTFError::OK;

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
        return RTFError::OK;
    }

    if (xPropertySet.is())
        xPropertySet->setPropertyValue("GraphicURL", uno::Any(aGraphicUrl));

    if (m_aStates.top().bInListpicture)
    {
        // Send the shape directly, no section is started, to additional properties will be ignored anyway.
        Mapper().startShape(xShape);
        Mapper().endShape();
        return RTFError::OK;
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
    for (RTFSprms::Iterator_t i = m_aStates.top().aCharacterAttributes.begin(); i != m_aStates.top().aCharacterAttributes.end(); ++i)
        if (i->first == NS_ooxml::LN_CT_NonVisualDrawingProps_name || i->first == NS_ooxml::LN_CT_NonVisualDrawingProps_descr)
            aDocprAttributes.set(i->first, i->second);
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
        for (RTFSprms::Iterator_t i = m_aStates.top().aCharacterAttributes.begin(); i != m_aStates.top().aCharacterAttributes.end(); ++i)
        {
            if (i->first == NS_ooxml::LN_CT_WrapSquare_wrapText)
                aAnchorWrapAttributes.set(i->first, i->second);
        }
        sal_Int32 nWrap = -1;
        for (RTFSprms::Iterator_t i = m_aStates.top().aCharacterSprms.begin(); i != m_aStates.top().aCharacterSprms.end(); ++i)
        {
            if (i->first == NS_ooxml::LN_EG_WrapType_wrapNone || i->first == NS_ooxml::LN_EG_WrapType_wrapTight)
            {
                nWrap = i->first;

                // If there is a wrap polygon prepared by RTFSdrImport, pick it up here.
                if (i->first == NS_ooxml::LN_EG_WrapType_wrapTight && !m_aStates.top().aShape.aWrapPolygonSprms.empty())
                    i->second->getSprms().set(NS_ooxml::LN_CT_WrapTight_wrapPolygon, std::make_shared<RTFValue>(RTFSprms(), m_aStates.top().aShape.aWrapPolygonSprms));

                aAnchorSprms.set(i->first, i->second);
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
        m_aStates.top().pCurrentBuffer->push_back(Buf_t(BUFFER_PROPS, pValue));
    }

    return RTFError::OK;
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
                    SAL_INFO_IF(m_aStates.top().eRunType != RTFParserState::DBCH, "writerfilter.rtf", "invalid Shift-JIS without DBCH");
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
        pCurrentBuffer->push_back(Buf_t(BUFFER_STARTRUN));
        auto pValue = std::make_shared<RTFValue>(*sValue);
        pCurrentBuffer->push_back(Buf_t(BUFFER_TEXT, pValue));
        pCurrentBuffer->push_back(Buf_t(BUFFER_ENDRUN));
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
        m_aStates.top().appendDestinationText(rString);
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
                RTFReferenceTable::Entries_t::iterator lb = m_aFontTableEntries.lower_bound(m_nCurrentFontIndex);
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
        m_aStates.top().appendDestinationText(rString);
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
        m_aTableBufferStack.back().push_back(Buf_t(BUFFER_UTEXT, std::make_shared<RTFValue>(rString)));
        return;
    }

    checkFirstRun();
    checkNeedPap();

    // Don't return earlier, a bookmark start has to be in a paragraph group.
    if (m_aStates.top().eDestination == Destination::BOOKMARKSTART)
    {
        m_aStates.top().appendDestinationText(rString);
        return;
    }

    RTFBuffer_t* pCurrentBuffer = m_aStates.top().pCurrentBuffer;

    if (!pCurrentBuffer && m_aStates.top().eDestination != Destination::FOOTNOTE)
        Mapper().startCharacterGroup();
    else if (pCurrentBuffer)
    {
        RTFValue::Pointer_t pValue;
        pCurrentBuffer->push_back(Buf_t(BUFFER_STARTRUN, pValue));
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
        pCurrentBuffer->push_back(Buf_t(BUFFER_UTEXT, pValue));
    }

    m_bNeedCr = true;

    if (!pCurrentBuffer && m_aStates.top().eDestination != Destination::FOOTNOTE)
        Mapper().endCharacterGroup();
    else if (pCurrentBuffer)
    {
        RTFValue::Pointer_t pValue;
        pCurrentBuffer->push_back(Buf_t(BUFFER_ENDRUN, pValue));
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
        lcl_putNestedAttribute(rState.aTableRowSprms, NS_ooxml::LN_CT_TblPrBase_tblW, NS_ooxml::LN_CT_TblWidth_type, pUnitValue);
        auto pWValue = std::make_shared<RTFValue>(nCurrentCellX);
        lcl_putNestedAttribute(rState.aTableRowSprms, NS_ooxml::LN_CT_TblPrBase_tblW, NS_ooxml::LN_CT_TblWidth_w, pWValue);
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
        lcl_putNestedSprm(rState.aTableRowSprms, NS_ooxml::LN_CT_TblPrBase_tblCellMar, NS_ooxml::LN_CT_TblCellMar_left, std::make_shared<RTFValue>(aAttributes));
        lcl_putNestedSprm(rState.aTableRowSprms, NS_ooxml::LN_CT_TblPrBase_tblCellMar, NS_ooxml::LN_CT_TblCellMar_right, std::make_shared<RTFValue>(aAttributes));
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
    for (size_t i = 0; i < rBuffer.size(); ++i)
    {
        SAL_WARN_IF(BUFFER_CELLEND == boost::get<0>(rBuffer[i]),
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
        if (boost::get<0>(aTuple) == BUFFER_PROPS)
        {
            // Construct properties via getProperties() and not directly, to take care of deduplication.
            writerfilter::Reference<Properties>::Pointer_t const pProp(
                getProperties(boost::get<1>(aTuple)->getAttributes(), boost::get<1>(aTuple)->getSprms())
            );
            Mapper().props(pProp);
        }
        else if (boost::get<0>(aTuple) == BUFFER_NESTROW)
        {
            TableRowBuffer& rRowBuffer(*boost::get<2>(aTuple));

            replayRowBuffer(rRowBuffer.buffer, rRowBuffer.cellsSprms,
                            rRowBuffer.cellsAttributes, rRowBuffer.nCells);

            sendProperties(rRowBuffer.pParaProperties,
                           rRowBuffer.pFrameProperties, rRowBuffer.pRowProperties);
        }
        else if (boost::get<0>(aTuple) == BUFFER_CELLEND)
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
        else if (boost::get<0>(aTuple) == BUFFER_STARTRUN)
            Mapper().startCharacterGroup();
        else if (boost::get<0>(aTuple) == BUFFER_TEXT)
        {
            sal_uInt8 const nValue = boost::get<1>(aTuple)->getInt();
            Mapper().text(&nValue, 1);
        }
        else if (boost::get<0>(aTuple) == BUFFER_UTEXT)
        {
            OUString const aString(boost::get<1>(aTuple)->getString());
            Mapper().utext(reinterpret_cast<sal_uInt8 const*>(aString.getStr()), aString.getLength());
        }
        else if (boost::get<0>(aTuple) == BUFFER_ENDRUN)
            Mapper().endCharacterGroup();
        else if (boost::get<0>(aTuple) == BUFFER_PAR)
            parBreak();
        else if (boost::get<0>(aTuple) == BUFFER_STARTSHAPE)
            m_pSdrImport->resolve(boost::get<1>(aTuple)->getShape(), false, RTFSdrImport::SHAPE);
        else if (boost::get<0>(aTuple) == BUFFER_RESOLVESHAPE)
            m_pSdrImport->resolve(boost::get<1>(aTuple)->getShape(), true, RTFSdrImport::SHAPE);
        else if (boost::get<0>(aTuple) == BUFFER_ENDSHAPE)
            m_pSdrImport->close();
        else if (boost::get<0>(aTuple) == BUFFER_RESOLVESUBSTREAM)
        {
            RTFSprms& rAttributes = boost::get<1>(aTuple)->getAttributes();
            sal_Size nPos = rAttributes.find(0)->getInt();
            Id nId = rAttributes.find(1)->getInt();
            OUString aCustomMark = rAttributes.find(2)->getString();
            resolveSubstream(nPos, nId, aCustomMark);
        }
        else
            assert(false);
    }

}

RTFError RTFDocumentImpl::dispatchDestination(RTFKeyword nKeyword)
{
    setNeedSect();
    checkUnicode(/*bUnicode =*/ true, /*bHex =*/ true);
    RTFSkipDestination aSkip(*this);
    // special case \upr: ignore everything except nested \ud
    if (Destination::UPR == m_aStates.top().eDestination && RTF_UD != nKeyword)
    {
        m_aStates.top().eDestination = Destination::SKIP;
        aSkip.setParsed(false);
    }
    else
        switch (nKeyword)
        {
        case RTF_RTF:
            break;
        case RTF_FONTTBL:
            m_aStates.top().eDestination = Destination::FONTTABLE;
            break;
        case RTF_COLORTBL:
            m_aStates.top().eDestination = Destination::COLORTABLE;
            break;
        case RTF_STYLESHEET:
            m_aStates.top().eDestination = Destination::STYLESHEET;
            break;
        case RTF_FIELD:
            m_aStates.top().eDestination = Destination::FIELD;
            break;
        case RTF_FLDINST:
        {
            // Look for the field type
            sal_Size nPos = Strm().Tell();
            OStringBuffer aBuf;
            char ch = 0;
            bool bFoundCode = false;
            bool bInKeyword = false;
            while (!bFoundCode && ch != '}')
            {
                Strm().ReadChar(ch);
                if ('\\' == ch)
                    bInKeyword = true;
                if (!bInKeyword  && isalnum(ch))
                    aBuf.append(ch);
                else if (bInKeyword && isspace(ch))
                    bInKeyword = false;
                if (!aBuf.isEmpty() && !isalnum(ch))
                    bFoundCode = true;
            }

            if (aBuf.toString() == "INCLUDEPICTURE")
            {
                // Extract the field argument of INCLUDEPICTURE: we handle that
                // at a tokenizer level, as DOCX has no such field.
                aBuf.append(ch);
                while (true)
                {
                    Strm().ReadChar(ch);
                    if (ch == '}')
                        break;
                    aBuf.append(ch);
                }
                OUString aFieldCommand = OStringToOUString(aBuf.toString(), RTL_TEXTENCODING_UTF8);
                boost::tuple<OUString, std::vector<OUString>, std::vector<OUString> > aResult = writerfilter::dmapper::lcl_SplitFieldCommand(aFieldCommand);
                m_aPicturePath = boost::get<1>(aResult).empty() ? OUString() : boost::get<1>(aResult).front();
            }

            Strm().Seek(nPos);

            // Form data should be handled only for form fields if any
            if (aBuf.toString().indexOf(OString("FORM")) != -1)
                m_bFormField = true;

            singleChar(cFieldStart);
            m_aStates.top().eDestination = Destination::FIELDINSTRUCTION;
        }
        break;
        case RTF_FLDRSLT:
            m_aStates.top().eDestination = Destination::FIELDRESULT;
            break;
        case RTF_LISTTABLE:
            m_aStates.top().eDestination = Destination::LISTTABLE;
            break;
        case RTF_LISTPICTURE:
            m_aStates.top().eDestination = Destination::LISTPICTURE;
            m_aStates.top().bInListpicture = true;
            break;
        case RTF_LIST:
            m_aStates.top().eDestination = Destination::LISTENTRY;
            break;
        case RTF_LISTNAME:
            m_aStates.top().eDestination = Destination::LISTNAME;
            break;
        case RTF_LFOLEVEL:
            m_aStates.top().eDestination = Destination::LFOLEVEL;
            m_aStates.top().aTableSprms.clear();
            break;
        case RTF_LISTOVERRIDETABLE:
            m_aStates.top().eDestination = Destination::LISTOVERRIDETABLE;
            break;
        case RTF_LISTOVERRIDE:
            m_aStates.top().eDestination = Destination::LISTOVERRIDEENTRY;
            break;
        case RTF_LISTLEVEL:
            m_aStates.top().eDestination = Destination::LISTLEVEL;
            break;
        case RTF_LEVELTEXT:
            m_aStates.top().eDestination = Destination::LEVELTEXT;
            break;
        case RTF_LEVELNUMBERS:
            m_aStates.top().eDestination = Destination::LEVELNUMBERS;
            break;
        case RTF_SHPPICT:
            m_aStates.top().resetFrame();
            m_aStates.top().eDestination = Destination::SHPPICT;
            break;
        case RTF_PICT:
            if (m_aStates.top().eDestination != Destination::SHAPEPROPERTYVALUE)
                m_aStates.top().eDestination = Destination::PICT; // as character
            else
                m_aStates.top().eDestination = Destination::SHAPEPROPERTYVALUEPICT; // anchored inside a shape
            break;
        case RTF_PICPROP:
            m_aStates.top().eDestination = Destination::PICPROP;
            break;
        case RTF_SP:
            m_aStates.top().eDestination = Destination::SHAPEPROPERTY;
            break;
        case RTF_SN:
            m_aStates.top().eDestination = Destination::SHAPEPROPERTYNAME;
            break;
        case RTF_SV:
            m_aStates.top().eDestination = Destination::SHAPEPROPERTYVALUE;
            break;
        case RTF_SHP:
            m_bNeedCrOrig = m_bNeedCr;
            m_aStates.top().eDestination = Destination::SHAPE;
            m_aStates.top().bInShape = true;
            break;
        case RTF_SHPINST:
            m_aStates.top().eDestination = Destination::SHAPEINSTRUCTION;
            break;
        case RTF_NESTTABLEPROPS:
            // do not set any properties of outer table at nested table!
            m_aStates.top().aTableCellSprms = m_aDefaultState.aTableCellSprms;
            m_aStates.top().aTableCellAttributes =
                m_aDefaultState.aTableCellAttributes;
            m_aNestedTableCellsSprms.clear();
            m_aNestedTableCellsAttributes.clear();
            m_nNestedCells = 0;
            m_aStates.top().eDestination = Destination::NESTEDTABLEPROPERTIES;
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
                sal_Size nPos = m_nGroupStartPos - 1;
                switch (nKeyword)
                {
                case RTF_HEADER:
                    nId = NS_ooxml::LN_headerr;
                    break;
                case RTF_FOOTER:
                    nId = NS_ooxml::LN_footerr;
                    break;
                case RTF_HEADERL:
                    nId = NS_ooxml::LN_headerl;
                    break;
                case RTF_HEADERR:
                    nId = NS_ooxml::LN_headerr;
                    break;
                case RTF_HEADERF:
                    nId = NS_ooxml::LN_headerf;
                    break;
                case RTF_FOOTERL:
                    nId = NS_ooxml::LN_footerl;
                    break;
                case RTF_FOOTERR:
                    nId = NS_ooxml::LN_footerr;
                    break;
                case RTF_FOOTERF:
                    nId = NS_ooxml::LN_footerf;
                    break;
                default:
                    break;
                }
                m_nHeaderFooterPositions.push(std::make_pair(nId, nPos));
                m_aStates.top().eDestination = Destination::SKIP;
            }
            break;
        case RTF_FOOTNOTE:
            checkFirstRun();
            if (!m_pSuperstream)
            {
                Id nId = NS_ooxml::LN_footnote;

                // Check if this is an endnote.
                OStringBuffer aBuf;
                char ch;
                sal_Size nCurrent = Strm().Tell();
                for (int i = 0; i < 7; ++i)
                {
                    Strm().ReadChar(ch);
                    aBuf.append(ch);
                }
                Strm().Seek(nCurrent);
                OString aKeyword = aBuf.makeStringAndClear();
                if (aKeyword.equals("\\ftnalt"))
                    nId = NS_ooxml::LN_endnote;

                if (m_aStates.top().pCurrentBuffer == &m_aSuperBuffer)
                    m_aStates.top().pCurrentBuffer = nullptr;
                bool bCustomMark = false;
                OUString aCustomMark;
                while (m_aSuperBuffer.size())
                {
                    Buf_t aTuple = m_aSuperBuffer.front();
                    m_aSuperBuffer.pop_front();
                    if (boost::get<0>(aTuple) == BUFFER_UTEXT)
                    {
                        aCustomMark = boost::get<1>(aTuple)->getString();
                        bCustomMark = true;
                    }
                }
                m_aStates.top().eDestination = Destination::FOOTNOTE;
                if (bCustomMark)
                    Mapper().startCharacterGroup();
                if (!m_aStates.top().pCurrentBuffer)
                    resolveSubstream(m_nGroupStartPos - 1, nId, aCustomMark);
                else
                {
                    RTFSprms aAttributes;
                    aAttributes.set(Id(0), std::make_shared<RTFValue>(m_nGroupStartPos - 1));
                    aAttributes.set(Id(1), std::make_shared<RTFValue>(nId));
                    aAttributes.set(Id(2), std::make_shared<RTFValue>(aCustomMark));
                    m_aStates.top().pCurrentBuffer->push_back(Buf_t(BUFFER_RESOLVESUBSTREAM, std::make_shared<RTFValue>(aAttributes)));
                }
                if (bCustomMark)
                {
                    m_aStates.top().aCharacterAttributes.clear();
                    m_aStates.top().aCharacterSprms.clear();
                    auto pValue = std::make_shared<RTFValue>(1);
                    m_aStates.top().aCharacterAttributes.set(NS_ooxml::LN_CT_FtnEdnRef_customMarkFollows, pValue);
                    text(aCustomMark);
                    Mapper().endCharacterGroup();
                }
                m_aStates.top().eDestination = Destination::SKIP;
            }
            break;
        case RTF_BKMKSTART:
            m_aStates.top().eDestination = Destination::BOOKMARKSTART;
            break;
        case RTF_BKMKEND:
            m_aStates.top().eDestination = Destination::BOOKMARKEND;
            break;
        case RTF_XE:
            m_aStates.top().eDestination = Destination::INDEXENTRY;
            break;
        case RTF_TC:
        case RTF_TCN:
            m_aStates.top().eDestination = Destination::TOCENTRY;
            break;
        case RTF_REVTBL:
            m_aStates.top().eDestination = Destination::REVISIONTABLE;
            break;
        case RTF_ANNOTATION:
            if (!m_pSuperstream)
            {
                resolveSubstream(m_nGroupStartPos - 1, NS_ooxml::LN_annotation);
                m_aStates.top().eDestination = Destination::SKIP;
            }
            else
            {
                // If there is an author set, emit it now.
                if (!m_aAuthor.isEmpty() || !m_aAuthorInitials.isEmpty())
                {
                    RTFSprms aAttributes;
                    if (!m_aAuthor.isEmpty())
                    {
                        auto pValue = std::make_shared<RTFValue>(m_aAuthor);
                        aAttributes.set(NS_ooxml::LN_CT_TrackChange_author, pValue);
                    }
                    if (!m_aAuthorInitials.isEmpty())
                    {
                        auto pValue = std::make_shared<RTFValue>(m_aAuthorInitials);
                        aAttributes.set(NS_ooxml::LN_CT_Comment_initials, pValue);
                    }
                    writerfilter::Reference<Properties>::Pointer_t pProperties = std::make_shared<RTFReferenceProperties>(aAttributes);
                    Mapper().props(pProperties);
                }
            }
            break;
        case RTF_SHPTXT:
        case RTF_DPTXBXTEXT:
        {
            bool bPictureFrame = false;
            for (size_t i = 0; i < m_aStates.top().aShape.aProperties.size(); ++i)
            {
                std::pair<OUString, OUString>& rProperty = m_aStates.top().aShape.aProperties[i];
                if (rProperty.first == "shapeType" && rProperty.second == OUString::number(ESCHER_ShpInst_PictureFrame))
                {
                    bPictureFrame = true;
                    break;
                }
            }
            if (bPictureFrame)
                // Skip text on picture frames.
                m_aStates.top().eDestination = Destination::SKIP;
            else
            {
                m_aStates.top().eDestination = Destination::SHAPETEXT;
                checkFirstRun();
                dispatchFlag(RTF_PARD);
                m_bNeedPap = true;
                if (nKeyword == RTF_SHPTXT)
                {
                    if (!m_aStates.top().pCurrentBuffer)
                        m_pSdrImport->resolve(m_aStates.top().aShape, false, RTFSdrImport::SHAPE);
                    else
                    {
                        auto pValue = std::make_shared<RTFValue>(m_aStates.top().aShape);
                        m_aStates.top().pCurrentBuffer->push_back(Buf_t(BUFFER_STARTSHAPE, pValue));
                    }
                }
            }
        }
        break;
        case RTF_FORMFIELD:
            if (m_aStates.top().eDestination == Destination::FIELDINSTRUCTION)
                m_aStates.top().eDestination = Destination::FORMFIELD;
            break;
        case RTF_FFNAME:
            m_aStates.top().eDestination = Destination::FORMFIELDNAME;
            break;
        case RTF_FFL:
            m_aStates.top().eDestination = Destination::FORMFIELDLIST;
            break;
        case RTF_DATAFIELD:
            m_aStates.top().eDestination = Destination::DATAFIELD;
            break;
        case RTF_INFO:
            m_aStates.top().eDestination = Destination::INFO;
            break;
        case RTF_CREATIM:
            m_aStates.top().eDestination = Destination::CREATIONTIME;
            break;
        case RTF_REVTIM:
            m_aStates.top().eDestination = Destination::REVISIONTIME;
            break;
        case RTF_PRINTIM:
            m_aStates.top().eDestination = Destination::PRINTTIME;
            break;
        case RTF_AUTHOR:
            m_aStates.top().eDestination = Destination::AUTHOR;
            break;
        case RTF_KEYWORDS:
            m_aStates.top().eDestination = Destination::KEYWORDS;
            break;
        case RTF_OPERATOR:
            m_aStates.top().eDestination = Destination::OPERATOR;
            break;
        case RTF_COMPANY:
            m_aStates.top().eDestination = Destination::COMPANY;
            break;
        case RTF_COMMENT:
            m_aStates.top().eDestination = Destination::COMMENT;
            break;
        case RTF_OBJECT:
        {
            // beginning of an OLE Object
            m_aStates.top().eDestination = Destination::OBJECT;

            // check if the object is in a special container (e.g. a table)
            if (!m_aStates.top().pCurrentBuffer)
            {
                // the object is in a table or another container.
                // Don't try to treat it as an OLE object (fdo#53594).
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
                m_aStates.top().eDestination = Destination::SKIP;
            }
            else
            {
                m_aStates.top().eDestination = Destination::OBJDATA;
            }
            break;
        case RTF_OBJCLASS:
            m_aStates.top().eDestination = Destination::OBJCLASS;
            break;
        case RTF_RESULT:
            m_aStates.top().eDestination = Destination::RESULT;
            break;
        case RTF_ATNDATE:
            m_aStates.top().eDestination = Destination::ANNOTATIONDATE;
            break;
        case RTF_ATNAUTHOR:
            m_aStates.top().eDestination = Destination::ANNOTATIONAUTHOR;
            break;
        case RTF_ATNREF:
            m_aStates.top().eDestination = Destination::ANNOTATIONREFERENCE;
            break;
        case RTF_FALT:
            m_aStates.top().eDestination = Destination::FALT;
            break;
        case RTF_FLYMAINCNT:
            m_aStates.top().eDestination = Destination::FLYMAINCONTENT;
            break;
        case RTF_LISTTEXT:
        // Should be ignored by any reader that understands Word 97 through Word 2007 numbering.
        case RTF_NONESTTABLES:
            // This destination should be ignored by readers that support nested tables.
            m_aStates.top().eDestination = Destination::SKIP;
            break;
        case RTF_DO:
            m_aStates.top().eDestination = Destination::DRAWINGOBJECT;
            break;
        case RTF_PN:
            m_aStates.top().eDestination = Destination::PARAGRAPHNUMBERING;
            break;
        case RTF_PNTEXT:
            // This destination should be ignored by readers that support paragraph numbering.
            m_aStates.top().eDestination = Destination::SKIP;
            break;
        case RTF_PNTXTA:
            m_aStates.top().eDestination = Destination::PARAGRAPHNUMBERING_TEXTAFTER;
            break;
        case RTF_PNTXTB:
            m_aStates.top().eDestination = Destination::PARAGRAPHNUMBERING_TEXTBEFORE;
            break;
        case RTF_TITLE:
            m_aStates.top().eDestination = Destination::TITLE;
            break;
        case RTF_SUBJECT:
            m_aStates.top().eDestination = Destination::SUBJECT;
            break;
        case RTF_DOCCOMM:
            m_aStates.top().eDestination = Destination::DOCCOMM;
            break;
        case RTF_ATRFSTART:
            m_aStates.top().eDestination = Destination::ANNOTATIONREFERENCESTART;
            break;
        case RTF_ATRFEND:
            m_aStates.top().eDestination = Destination::ANNOTATIONREFERENCEEND;
            break;
        case RTF_ATNID:
            m_aStates.top().eDestination = Destination::ATNID;
            break;
        case RTF_MMATH:
        case RTF_MOMATHPARA:
            // Nothing to do here (just enter the destination) till RTF_MMATHPR is implemented.
            break;
        case RTF_MR:
            m_aStates.top().eDestination = Destination::MR;
            break;
        case RTF_MCHR:
            m_aStates.top().eDestination = Destination::MCHR;
            break;
        case RTF_MPOS:
            m_aStates.top().eDestination = Destination::MPOS;
            break;
        case RTF_MVERTJC:
            m_aStates.top().eDestination = Destination::MVERTJC;
            break;
        case RTF_MSTRIKEH:
            m_aStates.top().eDestination = Destination::MSTRIKEH;
            break;
        case RTF_MDEGHIDE:
            m_aStates.top().eDestination = Destination::MDEGHIDE;
            break;
        case RTF_MTYPE:
            m_aStates.top().eDestination = Destination::MTYPE;
            break;
        case RTF_MGROW:
            m_aStates.top().eDestination = Destination::MGROW;
            break;
        case RTF_MHIDETOP:
        case RTF_MHIDEBOT:
        case RTF_MHIDELEFT:
        case RTF_MHIDERIGHT:
            // SmOoxmlImport::handleBorderBox will ignore these anyway, so silently ignore for now.
            m_aStates.top().eDestination = Destination::SKIP;
            break;
        case RTF_MSUBHIDE:
            m_aStates.top().eDestination = Destination::MSUBHIDE;
            break;
        case RTF_MSUPHIDE:
            m_aStates.top().eDestination = Destination::MSUPHIDE;
            break;
        case RTF_MBEGCHR:
            m_aStates.top().eDestination = Destination::MBEGCHR;
            break;
        case RTF_MSEPCHR:
            m_aStates.top().eDestination = Destination::MSEPCHR;
            break;
        case RTF_MENDCHR:
            m_aStates.top().eDestination = Destination::MENDCHR;
            break;
        case RTF_UPR:
            m_aStates.top().eDestination = Destination::UPR;
            break;
        case RTF_UD:
            // Anything inside \ud is just normal Unicode content.
            m_aStates.top().eDestination = Destination::NORMAL;
            break;
        case RTF_BACKGROUND:
            m_aStates.top().eDestination = Destination::BACKGROUND;
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
            m_aStates.top().eDestination = Destination::SHAPEGROUP;
            m_aStates.top().bInShapeGroup = true;
        }
        break;
        case RTF_FTNSEP:
            m_aStates.top().eDestination = Destination::FOOTNOTESEPARATOR;
            m_aStates.top().aCharacterAttributes.set(NS_ooxml::LN_CT_FtnEdn_type, std::make_shared<RTFValue>(NS_ooxml::LN_Value_doc_ST_FtnEdn_separator));
            break;
        default:
        {
            // Check if it's a math token.
            RTFMathSymbol aSymbol;
            aSymbol.eKeyword = nKeyword;
            if (RTFTokenizer::lookupMathKeyword(aSymbol))
            {
                m_aMathBuffer.appendOpeningTag(aSymbol.nToken);
                m_aStates.top().eDestination = aSymbol.eDestination;
                return RTFError::OK;
            }

            SAL_INFO("writerfilter", "TODO handle destination '" << lcl_RtfToString(nKeyword) << "'");
            // Make sure we skip destinations (even without \*) till we don't handle them
            m_aStates.top().eDestination = Destination::SKIP;
            aSkip.setParsed(false);
        }
        break;
        }

    // new destination => use new destination text
    m_aStates.top().pDestinationText = &m_aStates.top().aDestinationText;

    return RTFError::OK;
}

RTFError RTFDocumentImpl::dispatchSymbol(RTFKeyword nKeyword)
{
    setNeedSect();
    if (nKeyword != RTF_HEXCHAR)
        checkUnicode(/*bUnicode =*/ true, /*bHex =*/ true);
    else
        checkUnicode(/*bUnicode =*/ true, /*bHex =*/ false);
    RTFSkipDestination aSkip(*this);

    if (RTF_LINE == nKeyword)
    {
        // very special handling since text() will eat lone '\n'
        singleChar('\n');
        return RTFError::OK;
    }
    // Trivial symbols
    sal_uInt8 cCh = 0;
    switch (nKeyword)
    {
    case RTF_TAB:
        cCh = '\t';
        break;
    case RTF_BACKSLASH:
        cCh = '\\';
        break;
    case RTF_LBRACE:
        cCh = '{';
        break;
    case RTF_RBRACE:
        cCh = '}';
        break;
    case RTF_EMDASH:
        cCh = 151;
        break;
    case RTF_ENDASH:
        cCh = 150;
        break;
    case RTF_BULLET:
        cCh = 149;
        break;
    case RTF_LQUOTE:
        cCh = 145;
        break;
    case RTF_RQUOTE:
        cCh = 146;
        break;
    case RTF_LDBLQUOTE:
        cCh = 147;
        break;
    case RTF_RDBLQUOTE:
        cCh = 148;
        break;
    default:
        break;
    }
    if (cCh > 0)
    {
        OUString aStr(OStringToOUString(OString(cCh), RTL_TEXTENCODING_MS_1252));
        text(aStr);
        return RTFError::OK;
    }

    switch (nKeyword)
    {
    case RTF_IGNORE:
    {
        m_bSkipUnknown = true;
        aSkip.setReset(false);
        return RTFError::OK;
    }
    break;
    case RTF_PAR:
    {
        if (m_aStates.top().eDestination == Destination::FOOTNOTESEPARATOR)
            break; // just ignore it - only thing we read in here is CHFTNSEP
        checkFirstRun();
        bool bNeedPap = m_bNeedPap;
        checkNeedPap();
        if (bNeedPap)
            runProps();
        if (!m_aStates.top().pCurrentBuffer)
        {
            parBreak();
            // Not in table? Reset max width.
            if (m_nCellxMax)
            {
                // Was in table, but not anymore -> tblEnd.
                RTFSprms aAttributes;
                RTFSprms aSprms;
                aSprms.set(NS_ooxml::LN_tblEnd, std::make_shared<RTFValue>(1));
                writerfilter::Reference<Properties>::Pointer_t pProperties = std::make_shared<RTFReferenceProperties>(aAttributes, aSprms);
                Mapper().props(pProperties);
            }
            m_nCellxMax = 0;
        }
        else if (m_aStates.top().eDestination != Destination::SHAPETEXT)
        {
            RTFValue::Pointer_t pValue;
            m_aStates.top().pCurrentBuffer->push_back(
                Buf_t(BUFFER_PAR, pValue));
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
        {
            sectBreak();
            if (m_nResetBreakOnSectBreak != RTF_invalid)
            {
                // this should run on _second_ \sect after \page
                dispatchSymbol(m_nResetBreakOnSectBreak); // lazy reset
                m_nResetBreakOnSectBreak = RTF_invalid;
                m_bNeedSect = false; // dispatchSymbol set it
            }
        }
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
        m_aStates.top().nInternalState = RTFInternalState::HEX;
        break;
    case RTF_CELL:
    case RTF_NESTCELL:
    {
        checkFirstRun();
        if (m_bNeedPap)
        {
            // There were no runs in the cell, so we need to send paragraph and character properties here.
            auto pPValue = std::make_shared<RTFValue>(m_aStates.top().aParagraphAttributes, m_aStates.top().aParagraphSprms);
            m_aTableBufferStack.back().push_back(Buf_t(BUFFER_PROPS, pPValue));
            auto pCValue = std::make_shared<RTFValue>(m_aStates.top().aCharacterAttributes, m_aStates.top().aCharacterSprms);
            m_aTableBufferStack.back().push_back(Buf_t(BUFFER_PROPS, pCValue));
        }

        RTFValue::Pointer_t pValue;
        m_aTableBufferStack.back().push_back(Buf_t(BUFFER_CELLEND, pValue));
        m_bNeedPap = true;
    }
    break;
    case RTF_NESTROW:
    {
        std::shared_ptr<TableRowBuffer> const pBuffer(
            new TableRowBuffer(
                m_aTableBufferStack.back(),
                m_aNestedTableCellsSprms,
                m_aNestedTableCellsAttributes,
                m_nNestedCells));
        prepareProperties(m_aStates.top(),
                          pBuffer->pParaProperties,
                          pBuffer->pFrameProperties,
                          pBuffer->pRowProperties,
                          m_nNestedCells, m_nNestedCurrentCellX);

        assert(m_aStates.top().pCurrentBuffer == &m_aTableBufferStack.back());
        if (m_aTableBufferStack.size() == 1)
        {
            throw io::WrongFormatException(
                "mismatch between \\itap and number of \\nestrow", nullptr);
        }
        // note: there may be several states pointing to table buffer!
        for (size_t i = 0; i < m_aStates.size(); ++i)
        {
            if (m_aStates[i].pCurrentBuffer == &m_aTableBufferStack.back())
            {
                m_aStates[i].pCurrentBuffer =
                    &m_aTableBufferStack[m_aTableBufferStack.size()-2];
            }
        }
        m_aTableBufferStack.pop_back();
        m_aTableBufferStack.back().push_back(
            Buf_t(BUFFER_NESTROW, RTFValue::Pointer_t(), pBuffer));

        m_aNestedTableCellsSprms.clear();
        m_aNestedTableCellsAttributes.clear();
        m_nNestedCells = 0;
        m_bNeedPap = true;
    }
    break;
    case RTF_ROW:
    {
        bool bRestored = false;
        // Ending a row, but no cells defined?
        // See if there was an invalid table row reset, so we can restore cell infos to help invalid documents.
        if (!m_nTopLevelCurrentCellX && m_nBackupTopLevelCurrentCellX)
        {
            restoreTableRowProperties();
            bRestored = true;
        }

        // If the right edge of the last cell (row width) is smaller than the width of some other row, mimic WW8TabDesc::CalcDefaults(): resize the last cell
        const int MINLAY = 23; // sw/inc/swtypes.hxx, minimal possible size of frames.
        if ((m_nCellxMax - m_nTopLevelCurrentCellX) >= MINLAY)
        {
            auto pXValueLast = m_aStates.top().aTableRowSprms.find(NS_ooxml::LN_CT_TblGridBase_gridCol, false);
            auto pXValue = std::make_shared<RTFValue>(pXValueLast->getInt() + m_nCellxMax - m_nTopLevelCurrentCellX);
            m_aStates.top().aTableRowSprms.eraseLast(NS_ooxml::LN_CT_TblGridBase_gridCol);
            m_aStates.top().aTableRowSprms.set(NS_ooxml::LN_CT_TblGridBase_gridCol, pXValue, RTFOverwrite::NO_APPEND);
            m_nTopLevelCurrentCellX = m_nCellxMax;
        }

        if (m_nTopLevelCells)
        {
            // Make a backup before we start popping elements
            m_aTableInheritingCellsSprms = m_aTopLevelTableCellsSprms;
            m_aTableInheritingCellsAttributes = m_aTopLevelTableCellsAttributes;
            m_nInheritingCells = m_nTopLevelCells;
        }
        else
        {
            // No table definition? Then inherit from the previous row
            m_aTopLevelTableCellsSprms = m_aTableInheritingCellsSprms;
            m_aTopLevelTableCellsAttributes = m_aTableInheritingCellsAttributes;
            m_nTopLevelCells = m_nInheritingCells;
        }

        while (m_aTableBufferStack.size() > 1)
        {
            SAL_WARN("writerfilter.rtf", "dropping extra table buffer");
            // note: there may be several states pointing to table buffer!
            for (size_t i = 0; i < m_aStates.size(); ++i)
            {
                if (m_aStates[i].pCurrentBuffer == &m_aTableBufferStack.back())
                {
                    m_aStates[i].pCurrentBuffer =
                        &m_aTableBufferStack.front();
                }
            }
            m_aTableBufferStack.pop_back();
        }

        replayRowBuffer(m_aTableBufferStack.back(),
                        m_aTopLevelTableCellsSprms, m_aTopLevelTableCellsAttributes,
                        m_nTopLevelCells);

        // The scope of the table cell defaults is one row.
        m_aDefaultState.aTableCellSprms.clear();
        m_aStates.top().aTableCellSprms = m_aDefaultState.aTableCellSprms;
        m_aStates.top().aTableCellAttributes = m_aDefaultState.aTableCellAttributes;

        writerfilter::Reference<Properties>::Pointer_t paraProperties;
        writerfilter::Reference<Properties>::Pointer_t frameProperties;
        writerfilter::Reference<Properties>::Pointer_t rowProperties;
        prepareProperties(m_aStates.top(),
                          paraProperties, frameProperties, rowProperties,
                          m_nTopLevelCells, m_nTopLevelCurrentCellX);
        sendProperties(paraProperties, frameProperties, rowProperties);

        m_bNeedPap = true;
        m_bNeedFinalPar = true;
        m_aTableBufferStack.back().clear();
        m_nTopLevelCells = 0;

        if (bRestored)
            // We restored cell definitions, clear these now.
            // This is necessary, as later cell definitions want to overwrite the restored ones.
            resetTableRowProperties();
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
        checkFirstRun();
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
    {
        if (m_aStates.top().pCurrentBuffer == &m_aSuperBuffer)
            // Stop buffering, there will be no custom mark for this footnote or endnote.
            m_aStates.top().pCurrentBuffer = nullptr;
        break;
    }
    case RTF_PAGE:
    {
        // Ignore page breaks inside tables.
        if (m_aStates.top().pCurrentBuffer == &m_aTableBufferStack.back())
            break;

        // If we're inside a continuous section, we should send a section break, not a page one.
        RTFValue::Pointer_t pBreak = m_aStates.top().aSectionSprms.find(NS_ooxml::LN_EG_SectPrContents_type);
        // Unless we're on a title page.
        RTFValue::Pointer_t pTitlePg = m_aStates.top().aSectionSprms.find(NS_ooxml::LN_EG_SectPrContents_titlePg);
        if (((pBreak.get() && pBreak->getInt() == static_cast<sal_Int32>(NS_ooxml::LN_Value_ST_SectionMark_continuous))
                || m_nResetBreakOnSectBreak == RTF_SBKNONE)
                && !(pTitlePg.get() && pTitlePg->getInt()))
        {
            if (m_bWasInFrame)
            {
                dispatchSymbol(RTF_PAR);
                m_bWasInFrame = false;
            }
            sectBreak();
            // note: this will not affect the following section break
            // but the one just pushed
            dispatchFlag(RTF_SBKPAGE);
            if (m_bNeedPar)
                dispatchSymbol(RTF_PAR);
            m_bIgnoreNextContSectBreak = true;
            // arrange to clean up the synthetic RTF_SBKPAGE
            m_nResetBreakOnSectBreak = RTF_SBKNONE;
        }
        else
        {
            checkFirstRun();
            checkNeedPap();
            sal_uInt8 sBreak[] = { 0xc };
            Mapper().text(sBreak, 1);
            if (!m_bNeedPap)
            {
                parBreak();
                m_bNeedPap = true;
            }
            m_bNeedCr = true;
        }
    }
    break;
    case RTF_CHPGN:
    {
        OUString aStr("PAGE");
        singleChar(cFieldStart);
        text(aStr);
        singleChar(cFieldSep, true);
        singleChar(cFieldEnd);
    }
    break;
    case RTF_CHFTNSEP:
    {
        static const sal_Unicode uFtnEdnSep = 0x3;
        Mapper().utext(reinterpret_cast<const sal_uInt8*>(&uFtnEdnSep), 1);
    }
    break;
    default:
    {
        SAL_INFO("writerfilter", "TODO handle symbol '" << lcl_RtfToString(nKeyword) << "'");
        aSkip.setParsed(false);
    }
    break;
    }
    return RTFError::OK;
}


static  int lcl_getNumberFormat(int nParam)
{
    static const int aMap[] =
    {
        NS_ooxml::LN_Value_ST_NumberFormat_decimal,
        NS_ooxml::LN_Value_ST_NumberFormat_upperRoman,
        NS_ooxml::LN_Value_ST_NumberFormat_lowerRoman,
        NS_ooxml::LN_Value_ST_NumberFormat_upperLetter,
        NS_ooxml::LN_Value_ST_NumberFormat_lowerLetter,
        NS_ooxml::LN_Value_ST_NumberFormat_ordinal,
        NS_ooxml::LN_Value_ST_NumberFormat_cardinalText,
        NS_ooxml::LN_Value_ST_NumberFormat_ordinalText,
        NS_ooxml::LN_Value_ST_NumberFormat_none,     // Undefined in RTF 1.8 spec.
        NS_ooxml::LN_Value_ST_NumberFormat_none,     // Undefined in RTF 1.8 spec.
        NS_ooxml::LN_Value_ST_NumberFormat_ideographDigital,
        NS_ooxml::LN_Value_ST_NumberFormat_japaneseCounting,
        NS_ooxml::LN_Value_ST_NumberFormat_aiueo,
        NS_ooxml::LN_Value_ST_NumberFormat_iroha,
        NS_ooxml::LN_Value_ST_NumberFormat_decimalFullWidth,
        NS_ooxml::LN_Value_ST_NumberFormat_decimalHalfWidth,
        NS_ooxml::LN_Value_ST_NumberFormat_japaneseLegal,
        NS_ooxml::LN_Value_ST_NumberFormat_japaneseDigitalTenThousand ,
        NS_ooxml::LN_Value_ST_NumberFormat_decimalEnclosedCircleChinese,
        NS_ooxml::LN_Value_ST_NumberFormat_decimalFullWidth2,
        NS_ooxml::LN_Value_ST_NumberFormat_aiueoFullWidth,
        NS_ooxml::LN_Value_ST_NumberFormat_irohaFullWidth,
        NS_ooxml::LN_Value_ST_NumberFormat_decimalZero,
        NS_ooxml::LN_Value_ST_NumberFormat_bullet,
        NS_ooxml::LN_Value_ST_NumberFormat_ganada,
        NS_ooxml::LN_Value_ST_NumberFormat_chosung,
        NS_ooxml::LN_Value_ST_NumberFormat_decimalEnclosedFullstop,
        NS_ooxml::LN_Value_ST_NumberFormat_decimalEnclosedParen,
        NS_ooxml::LN_Value_ST_NumberFormat_decimalEnclosedCircleChinese,
        NS_ooxml::LN_Value_ST_NumberFormat_ideographEnclosedCircle,
        NS_ooxml::LN_Value_ST_NumberFormat_ideographTraditional,
        NS_ooxml::LN_Value_ST_NumberFormat_ideographZodiac,
        NS_ooxml::LN_Value_ST_NumberFormat_ideographZodiacTraditional,
        NS_ooxml::LN_Value_ST_NumberFormat_taiwaneseCounting,
        NS_ooxml::LN_Value_ST_NumberFormat_ideographLegalTraditional,
        NS_ooxml::LN_Value_ST_NumberFormat_taiwaneseCountingThousand,
        NS_ooxml::LN_Value_ST_NumberFormat_taiwaneseDigital,
        NS_ooxml::LN_Value_ST_NumberFormat_chineseCounting,
        NS_ooxml::LN_Value_ST_NumberFormat_chineseLegalSimplified,
        NS_ooxml::LN_Value_ST_NumberFormat_chineseCountingThousand,
        NS_ooxml::LN_Value_ST_NumberFormat_decimal,
        NS_ooxml::LN_Value_ST_NumberFormat_koreanDigital,
        NS_ooxml::LN_Value_ST_NumberFormat_koreanCounting,
        NS_ooxml::LN_Value_ST_NumberFormat_koreanLegal,
        NS_ooxml::LN_Value_ST_NumberFormat_koreanDigital2,
        NS_ooxml::LN_Value_ST_NumberFormat_hebrew1,
        NS_ooxml::LN_Value_ST_NumberFormat_arabicAlpha,
        NS_ooxml::LN_Value_ST_NumberFormat_hebrew2,
        NS_ooxml::LN_Value_ST_NumberFormat_arabicAbjad
    };
    const int nLen = SAL_N_ELEMENTS(aMap);
    int nValue = 0;
    if (nParam >= 0 && nParam < nLen)
        nValue = aMap[nParam];
    else  // 255 and the other cases.
        nValue = NS_ooxml::LN_Value_ST_NumberFormat_none;
    return nValue;
}


// Checks if rName is contained at least once in rProperties as a key.
bool lcl_findPropertyName(const std::vector<beans::PropertyValue>& rProperties, const OUString& rName)
{
    for (std::vector<beans::PropertyValue>::const_iterator it = rProperties.begin(); it != rProperties.end(); ++it)
    {
        if (it->Name == rName)
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

RTFError RTFDocumentImpl::dispatchFlag(RTFKeyword nKeyword)
{
    setNeedSect();
    checkUnicode(/*bUnicode =*/ true, /*bHex =*/ true);
    RTFSkipDestination aSkip(*this);
    int nParam = -1;
    int nSprm = -1;

    // Underline flags.
    switch (nKeyword)
    {
    case RTF_ULD:
        nSprm = NS_ooxml::LN_Value_ST_Underline_dotted;
        break;
    case RTF_ULW:
        nSprm = NS_ooxml::LN_Value_ST_Underline_words;
        break;
    default:
        break;
    }
    if (nSprm >= 0)
    {
        auto pValue = std::make_shared<RTFValue>(nSprm);
        m_aStates.top().aCharacterAttributes.set(NS_ooxml::LN_CT_Underline_val, pValue);
        return RTFError::OK;
    }

    // Indentation
    switch (nKeyword)
    {
    case RTF_QC:
        nParam = NS_ooxml::LN_Value_ST_Jc_center;
        break;
    case RTF_QJ:
        nParam = NS_ooxml::LN_Value_ST_Jc_both;
        break;
    case RTF_QL:
        nParam = NS_ooxml::LN_Value_ST_Jc_left;
        break;
    case RTF_QR:
        nParam = NS_ooxml::LN_Value_ST_Jc_right;
        break;
    case RTF_QD:
        nParam = NS_ooxml::LN_Value_ST_Jc_both;
        break;
    default:
        break;
    }
    if (nParam >= 0)
    {
        auto pValue = std::make_shared<RTFValue>(nParam);
        m_aStates.top().aParagraphSprms.set(NS_ooxml::LN_CT_PPrBase_jc, pValue);
        m_bNeedPap = true;
        return RTFError::OK;
    }

    // Font Alignment
    switch (nKeyword)
    {
    case RTF_FAFIXED:
    case RTF_FAAUTO:
        nParam = NS_ooxml::LN_Value_doc_ST_TextAlignment_auto;
        break;
    case RTF_FAHANG:
        nParam = NS_ooxml::LN_Value_doc_ST_TextAlignment_top;
        break;
    case RTF_FACENTER:
        nParam = NS_ooxml::LN_Value_doc_ST_TextAlignment_center;
        break;
    case RTF_FAROMAN:
        nParam = NS_ooxml::LN_Value_doc_ST_TextAlignment_baseline;
        break;
    case RTF_FAVAR:
        nParam = NS_ooxml::LN_Value_doc_ST_TextAlignment_bottom;
        break;
    default:
        break;
    }
    if (nParam >= 0)
    {
        auto pValue = std::make_shared<RTFValue>(nParam);
        m_aStates.top().aParagraphSprms.set(NS_ooxml::LN_CT_PPrBase_textAlignment, pValue);
        return RTFError::OK;
    }

    // Tab kind.
    switch (nKeyword)
    {
    case RTF_TQR:
        nParam = NS_ooxml::LN_Value_ST_TabJc_right;
        break;
    case RTF_TQC:
        nParam = NS_ooxml::LN_Value_ST_TabJc_center;
        break;
    case RTF_TQDEC:
        nParam = NS_ooxml::LN_Value_ST_TabJc_decimal;
        break;
    default:
        break;
    }
    if (nParam >= 0)
    {
        auto pValue = std::make_shared<RTFValue>(nParam);
        m_aStates.top().aTabAttributes.set(NS_ooxml::LN_CT_TabStop_val, pValue);
        return RTFError::OK;
    }

    // Tab lead.
    switch (nKeyword)
    {
    case RTF_TLDOT:
        nParam = NS_ooxml::LN_Value_ST_TabTlc_dot;
        break;
    case RTF_TLMDOT:
        nParam = NS_ooxml::LN_Value_ST_TabTlc_middleDot;
        break;
    case RTF_TLHYPH:
        nParam = NS_ooxml::LN_Value_ST_TabTlc_hyphen;
        break;
    case RTF_TLUL:
        nParam = NS_ooxml::LN_Value_ST_TabTlc_underscore;
        break;
    case RTF_TLTH:
        nParam = NS_ooxml::LN_Value_ST_TabTlc_hyphen;
        break; // thick line is not supported by dmapper, this is just a hack
    case RTF_TLEQ:
        nParam = NS_ooxml::LN_Value_ST_TabTlc_none;
        break; // equal sign isn't, either
    default:
        break;
    }
    if (nParam >= 0)
    {
        auto pValue = std::make_shared<RTFValue>(nParam);
        m_aStates.top().aTabAttributes.set(NS_ooxml::LN_CT_TabStop_leader, pValue);
        return RTFError::OK;
    }

    // Border types
    {
        switch (nKeyword)
        {
        // brdrhair and brdrs are the same, brdrw will make a difference
        // map to values in ooxml/model.xml resource ST_Border
        case RTF_BRDRHAIR:
        case RTF_BRDRS:
            nParam = NS_ooxml::LN_Value_ST_Border_single;
            break;
        case RTF_BRDRDOT:
            nParam = NS_ooxml::LN_Value_ST_Border_dotted;
            break;
        case RTF_BRDRDASH:
            nParam = NS_ooxml::LN_Value_ST_Border_dashed;
            break;
        case RTF_BRDRDB:
            nParam = NS_ooxml::LN_Value_ST_Border_double;
            break;
        case RTF_BRDRTNTHSG:
            nParam = NS_ooxml::LN_Value_ST_Border_thinThickSmallGap;
            break;
        case RTF_BRDRTNTHMG:
            nParam = NS_ooxml::LN_Value_ST_Border_thinThickMediumGap;
            break;
        case RTF_BRDRTNTHLG:
            nParam = NS_ooxml::LN_Value_ST_Border_thinThickLargeGap;
            break;
        case RTF_BRDRTHTNSG:
            nParam = NS_ooxml::LN_Value_ST_Border_thickThinSmallGap;
            break;
        case RTF_BRDRTHTNMG:
            nParam = NS_ooxml::LN_Value_ST_Border_thickThinMediumGap;
            break;
        case RTF_BRDRTHTNLG:
            nParam = NS_ooxml::LN_Value_ST_Border_thickThinLargeGap;
            break;
        case RTF_BRDREMBOSS:
            nParam = NS_ooxml::LN_Value_ST_Border_threeDEmboss;
            break;
        case RTF_BRDRENGRAVE:
            nParam = NS_ooxml::LN_Value_ST_Border_threeDEngrave;
            break;
        case RTF_BRDROUTSET:
            nParam = NS_ooxml::LN_Value_ST_Border_outset;
            break;
        case RTF_BRDRINSET:
            nParam = NS_ooxml::LN_Value_ST_Border_inset;
            break;
        case RTF_BRDRNONE:
            nParam = NS_ooxml::LN_Value_ST_Border_none;
            break;
        default:
            break;
        }
        if (nParam >= 0)
        {
            auto pValue = std::make_shared<RTFValue>(nParam);
            lcl_putBorderProperty(m_aStates, NS_ooxml::LN_CT_Border_val, pValue);
            return RTFError::OK;
        }
    }

    // Section breaks
    switch (nKeyword)
    {
    case RTF_SBKNONE:
        nParam = NS_ooxml::LN_Value_ST_SectionMark_continuous;
        break;
    case RTF_SBKCOL:
        nParam = NS_ooxml::LN_Value_ST_SectionMark_nextColumn;
        break;
    case RTF_SBKPAGE:
        nParam = NS_ooxml::LN_Value_ST_SectionMark_nextPage;
        break;
    case RTF_SBKEVEN:
        nParam = NS_ooxml::LN_Value_ST_SectionMark_evenPage;
        break;
    case RTF_SBKODD:
        nParam = NS_ooxml::LN_Value_ST_SectionMark_oddPage;
        break;
    default:
        break;
    }
    if (nParam >= 0)
    {
        if (m_nResetBreakOnSectBreak != RTF_invalid)
        {
            m_nResetBreakOnSectBreak = nKeyword;
        }
        auto pValue = std::make_shared<RTFValue>(nParam);
        m_aStates.top().aSectionSprms.set(NS_ooxml::LN_EG_SectPrContents_type, pValue);
        return RTFError::OK;
    }

    // Footnote numbering
    switch (nKeyword)
    {
    case RTF_FTNNAR:
        nParam = NS_ooxml::LN_Value_ST_NumberFormat_decimal;
        break;
    case RTF_FTNNALC:
        nParam = NS_ooxml::LN_Value_ST_NumberFormat_lowerLetter;
        break;
    case RTF_FTNNAUC:
        nParam = NS_ooxml::LN_Value_ST_NumberFormat_upperLetter;
        break;
    case RTF_FTNNRLC:
        nParam = NS_ooxml::LN_Value_ST_NumberFormat_lowerRoman;
        break;
    case RTF_FTNNRUC:
        nParam = NS_ooxml::LN_Value_ST_NumberFormat_upperRoman;
        break;
    case RTF_FTNNCHI:
        nParam = NS_ooxml::LN_Value_ST_NumberFormat_chicago;
        break;
    default:
        break;
    }
    if (nParam >= 0)
    {
        auto pValue = std::make_shared<RTFValue>(nParam);
        lcl_putNestedSprm(m_aDefaultState.aParagraphSprms, NS_ooxml::LN_EG_SectPrContents_footnotePr, NS_ooxml::LN_CT_FtnProps_numFmt, pValue);
        return RTFError::OK;
    }

    // Footnote restart type
    switch (nKeyword)
    {
    case RTF_FTNRSTPG:
        nParam = NS_ooxml::LN_Value_ST_RestartNumber_eachPage;
        break;
    case RTF_FTNRESTART:
        nParam = NS_ooxml::LN_Value_ST_RestartNumber_eachSect;
        break;
    case RTF_FTNRSTCONT:
        nParam = NS_ooxml::LN_Value_ST_RestartNumber_continuous;
        break;
    default:
        break;
    }
    if (nParam >= 0)
    {
        auto pValue = std::make_shared<RTFValue>(nParam);
        lcl_putNestedSprm(m_aDefaultState.aParagraphSprms, NS_ooxml::LN_EG_SectPrContents_footnotePr, NS_ooxml::LN_EG_FtnEdnNumProps_numRestart, pValue);
        return RTFError::OK;
    }

    // Endnote numbering
    switch (nKeyword)
    {
    case RTF_AFTNNAR:
        nParam = NS_ooxml::LN_Value_ST_NumberFormat_decimal;
        break;
    case RTF_AFTNNALC:
        nParam = NS_ooxml::LN_Value_ST_NumberFormat_lowerLetter;
        break;
    case RTF_AFTNNAUC:
        nParam = NS_ooxml::LN_Value_ST_NumberFormat_upperLetter;
        break;
    case RTF_AFTNNRLC:
        nParam = NS_ooxml::LN_Value_ST_NumberFormat_lowerRoman;
        break;
    case RTF_AFTNNRUC:
        nParam = NS_ooxml::LN_Value_ST_NumberFormat_upperRoman;
        break;
    case RTF_AFTNNCHI:
        nParam = NS_ooxml::LN_Value_ST_NumberFormat_chicago;
        break;
    default:
        break;
    }
    if (nParam >= 0)
    {
        auto pValue = std::make_shared<RTFValue>(nParam);
        lcl_putNestedSprm(m_aDefaultState.aParagraphSprms, NS_ooxml::LN_EG_SectPrContents_endnotePr, NS_ooxml::LN_CT_EdnProps_numFmt, pValue);
        return RTFError::OK;
    }

    switch (nKeyword)
    {
    case RTF_TRQL:
        nParam = NS_ooxml::LN_Value_ST_Jc_left;
        break;
    case RTF_TRQC:
        nParam = NS_ooxml::LN_Value_ST_Jc_center;
        break;
    case RTF_TRQR:
        nParam = NS_ooxml::LN_Value_ST_Jc_right;
        break;
    default:
        break;
    }
    if (nParam >= 0)
    {
        auto pValue = std::make_shared<RTFValue>(nParam);
        m_aStates.top().aTableRowSprms.set(NS_ooxml::LN_CT_TrPrBase_jc, pValue);
        return RTFError::OK;
    }

    // Cell Text Flow
    switch (nKeyword)
    {
    case RTF_CLTXLRTB:
        nParam = NS_ooxml::LN_Value_ST_TextDirection_lrTb;
        break;
    case RTF_CLTXTBRL:
        nParam = NS_ooxml::LN_Value_ST_TextDirection_tbRl;
        break;
    case RTF_CLTXBTLR:
        nParam = NS_ooxml::LN_Value_ST_TextDirection_btLr;
        break;
    case RTF_CLTXLRTBV:
        nParam = NS_ooxml::LN_Value_ST_TextDirection_lrTbV;
        break;
    case RTF_CLTXTBRLV:
        nParam = NS_ooxml::LN_Value_ST_TextDirection_tbRlV;
        break;
    default:
        break;
    }
    if (nParam >= 0)
    {
        auto pValue = std::make_shared<RTFValue>(nParam);
        m_aStates.top().aTableCellSprms.set(NS_ooxml::LN_CT_TcPrBase_textDirection, pValue);
    }

    // Trivial paragraph flags
    switch (nKeyword)
    {
    case RTF_KEEP:
        if (m_aStates.top().pCurrentBuffer != &m_aTableBufferStack.back())
            nParam = NS_ooxml::LN_CT_PPrBase_keepLines;
        break;
    case RTF_KEEPN:
        if (m_aStates.top().pCurrentBuffer != &m_aTableBufferStack.back())
            nParam = NS_ooxml::LN_CT_PPrBase_keepNext;
        break;
    case RTF_INTBL:
    {
        m_aStates.top().pCurrentBuffer = &m_aTableBufferStack.back();
        nParam = NS_ooxml::LN_inTbl;
    }
    break;
    case RTF_PAGEBB:
        nParam = NS_ooxml::LN_CT_PPrBase_pageBreakBefore;
        break;
    default:
        break;
    }
    if (nParam >= 0)
    {
        auto pValue = std::make_shared<RTFValue>(1);
        m_aStates.top().aParagraphSprms.erase(NS_ooxml::LN_inTbl);
        m_aStates.top().aParagraphSprms.set(nParam, pValue);
        return RTFError::OK;
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
    case RTF_MAC:
        m_aDefaultState.nCurrentEncoding = RTL_TEXTENCODING_APPLE_ROMAN;
        m_aStates.top().nCurrentEncoding = m_aDefaultState.nCurrentEncoding;
        break;
    case RTF_PC:
        m_aDefaultState.nCurrentEncoding = RTL_TEXTENCODING_IBM_437;
        m_aStates.top().nCurrentEncoding = m_aDefaultState.nCurrentEncoding;
        break;
    case RTF_PCA:
        m_aDefaultState.nCurrentEncoding = RTL_TEXTENCODING_IBM_850;
        m_aStates.top().nCurrentEncoding = m_aDefaultState.nCurrentEncoding;
        break;
    case RTF_PLAIN:
    {
        m_aStates.top().aCharacterSprms = getDefaultState().aCharacterSprms;
        m_aStates.top().nCurrentEncoding = getEncoding(getFontIndex(m_nDefaultFontIndex));
        m_aStates.top().aCharacterAttributes = getDefaultState().aCharacterAttributes;
        m_aStates.top().nCurrentCharacterStyleIndex = -1;
        m_aStates.top().isRightToLeft = false;
        m_aStates.top().eRunType = RTFParserState::LOCH;
    }
    break;
    case RTF_PARD:
        if (m_bHadPicture)
            dispatchSymbol(RTF_PAR);
        // \pard is allowed between \cell and \row, but in that case it should not reset the fact that we're inside a table.
        m_aStates.top().aParagraphSprms = m_aDefaultState.aParagraphSprms;
        m_aStates.top().aParagraphAttributes = m_aDefaultState.aParagraphAttributes;

        if (m_nTopLevelCells == 0 && m_nNestedCells == 0)
        {
            // Reset that we're in a table.
            m_aStates.top().pCurrentBuffer = nullptr;
        }
        else
        {
            // We are still in a table.
            m_aStates.top().aParagraphSprms.set(NS_ooxml::LN_inTbl, std::make_shared<RTFValue>(1));
            // Ideally getDefaultSPRM() would take care of this, but it would not when we're buffering.
            m_aStates.top().aParagraphSprms.set(NS_ooxml::LN_CT_PPrBase_tabs, std::make_shared<RTFValue>());
        }
        m_aStates.top().resetFrame();

        // Reset currently selected paragraph style as well.
        // By default the style with index 0 is applied.
        {
            OUString const aName = getStyleName(0);
            if (!aName.isEmpty())
            {
                m_aStates.top().aParagraphSprms.set(NS_ooxml::LN_CT_PPrBase_pStyle, std::make_shared<RTFValue>(aName));
                m_aStates.top().nCurrentStyleIndex = 0;
            }
            else
            {
                m_aStates.top().nCurrentStyleIndex = -1;
            }
        }
        // Need to send paragraph properties again, if there will be any.
        m_bNeedPap = true;
        break;
    case RTF_SECTD:
    {
        m_aStates.top().aSectionSprms = m_aDefaultState.aSectionSprms;
        m_aStates.top().aSectionAttributes = m_aDefaultState.aSectionAttributes;
    }
    break;
    case RTF_TROWD:
    {
        // Back these up, in case later we still need this info.
        backupTableRowProperties();
        resetTableRowProperties();
        // In case the table definition is in the middle of the row
        // (invalid), make sure table definition is emitted.
        m_bNeedPap = true;
    }
    break;
    case RTF_WIDCTLPAR:
    case RTF_NOWIDCTLPAR:
    {
        auto pValue = std::make_shared<RTFValue>(int(nKeyword == RTF_WIDCTLPAR));
        m_aStates.top().aParagraphSprms.set(NS_ooxml::LN_CT_PPrBase_widowControl, pValue);
    }
    break;
    case RTF_BOX:
    {
        RTFSprms aAttributes;
        auto pValue = std::make_shared<RTFValue>(aAttributes);
        for (int i = 0; i < 4; i++)
            m_aStates.top().aParagraphSprms.set(lcl_getParagraphBorder(i), pValue);
        m_aStates.top().nBorderState = RTFBorderState::PARAGRAPH_BOX;
    }
    break;
    case RTF_LTRSECT:
    case RTF_RTLSECT:
    {
        auto pValue = std::make_shared<RTFValue>(nKeyword == RTF_LTRSECT ? 0 : 1);
        m_aStates.top().aParagraphSprms.set(NS_ooxml::LN_EG_SectPrContents_textDirection, pValue);
    }
    break;
    case RTF_LTRPAR:
    case RTF_RTLPAR:
    {
        auto pValue = std::make_shared<RTFValue>(nKeyword == RTF_LTRPAR ? 0 : 1);
        m_aStates.top().aParagraphSprms.set(NS_ooxml::LN_CT_PPrBase_bidi, pValue);
    }
    break;
    case RTF_LTRROW:
    case RTF_RTLROW:
        m_aStates.top().aTableRowSprms.set(NS_ooxml::LN_CT_TblPrBase_bidiVisual, std::make_shared<RTFValue>(int(nKeyword == RTF_RTLROW)));
        break;
    case RTF_LTRCH:
        // dmapper does not support this.
        m_aStates.top().isRightToLeft = false;
        break;
    case RTF_RTLCH:
        m_aStates.top().isRightToLeft = true;
        if (m_aDefaultState.nCurrentEncoding == RTL_TEXTENCODING_MS_1255)
            m_aStates.top().nCurrentEncoding = m_aDefaultState.nCurrentEncoding;
        break;
    case RTF_ULNONE:
    {
        auto pValue = std::make_shared<RTFValue>(NS_ooxml::LN_Value_ST_Underline_none);
        m_aStates.top().aCharacterAttributes.set(NS_ooxml::LN_CT_Underline_val, pValue);
    }
    break;
    case RTF_NONSHPPICT:
    case RTF_MMATHPICT: // Picture group used by readers not understanding \moMath group
        m_aStates.top().eDestination = Destination::SKIP;
        break;
    case RTF_CLBRDRT:
    case RTF_CLBRDRL:
    case RTF_CLBRDRB:
    case RTF_CLBRDRR:
    {
        RTFSprms aAttributes;
        RTFSprms aSprms;
        auto pValue = std::make_shared<RTFValue>(aAttributes, aSprms);
        switch (nKeyword)
        {
        case RTF_CLBRDRT:
            nParam = NS_ooxml::LN_CT_TcBorders_top;
            break;
        case RTF_CLBRDRL:
            nParam = NS_ooxml::LN_CT_TcBorders_left;
            break;
        case RTF_CLBRDRB:
            nParam = NS_ooxml::LN_CT_TcBorders_bottom;
            break;
        case RTF_CLBRDRR:
            nParam = NS_ooxml::LN_CT_TcBorders_right;
            break;
        default:
            break;
        }
        lcl_putNestedSprm(m_aStates.top().aTableCellSprms, NS_ooxml::LN_CT_TcPrBase_tcBorders, nParam, pValue);
        m_aStates.top().nBorderState = RTFBorderState::CELL;
    }
    break;
    case RTF_PGBRDRT:
    case RTF_PGBRDRL:
    case RTF_PGBRDRB:
    case RTF_PGBRDRR:
    {
        RTFSprms aAttributes;
        RTFSprms aSprms;
        auto pValue = std::make_shared<RTFValue>(aAttributes, aSprms);
        switch (nKeyword)
        {
        case RTF_PGBRDRT:
            nParam = NS_ooxml::LN_CT_PageBorders_top;
            break;
        case RTF_PGBRDRL:
            nParam = NS_ooxml::LN_CT_PageBorders_left;
            break;
        case RTF_PGBRDRB:
            nParam = NS_ooxml::LN_CT_PageBorders_bottom;
            break;
        case RTF_PGBRDRR:
            nParam = NS_ooxml::LN_CT_PageBorders_right;
            break;
        default:
            break;
        }
        lcl_putNestedSprm(m_aStates.top().aSectionSprms, NS_ooxml::LN_EG_SectPrContents_pgBorders, nParam, pValue);
        m_aStates.top().nBorderState = RTFBorderState::PAGE;
    }
    break;
    case RTF_BRDRT:
    case RTF_BRDRL:
    case RTF_BRDRB:
    case RTF_BRDRR:
    {
        RTFSprms aAttributes;
        RTFSprms aSprms;
        auto pValue = std::make_shared<RTFValue>(aAttributes, aSprms);
        switch (nKeyword)
        {
        case RTF_BRDRT:
            nParam = lcl_getParagraphBorder(0);
            break;
        case RTF_BRDRL:
            nParam = lcl_getParagraphBorder(1);
            break;
        case RTF_BRDRB:
            nParam = lcl_getParagraphBorder(2);
            break;
        case RTF_BRDRR:
            nParam = lcl_getParagraphBorder(3);
            break;
        default:
            break;
        }
        lcl_putNestedSprm(m_aStates.top().aParagraphSprms, NS_ooxml::LN_CT_PrBase_pBdr, nParam, pValue);
        m_aStates.top().nBorderState = RTFBorderState::PARAGRAPH;
    }
    break;
    case RTF_CHBRDR:
    {
        RTFSprms aAttributes;
        auto pValue = std::make_shared<RTFValue>(aAttributes);
        m_aStates.top().aCharacterSprms.set(NS_ooxml::LN_EG_RPrBase_bdr, pValue);
        m_aStates.top().nBorderState = RTFBorderState::CHARACTER;
    }
    break;
    case RTF_CLMGF:
    {
        auto pValue = std::make_shared<RTFValue>(NS_ooxml::LN_Value_ST_Merge_restart);
        m_aStates.top().aTableCellSprms.set(NS_ooxml::LN_CT_TcPrBase_hMerge, pValue);
    }
    break;
    case RTF_CLMRG:
    {
        auto pValue = std::make_shared<RTFValue>(NS_ooxml::LN_Value_ST_Merge_continue);
        m_aStates.top().aTableCellSprms.set(NS_ooxml::LN_CT_TcPrBase_hMerge, pValue);
    }
    break;
    case RTF_CLVMGF:
    {
        auto pValue = std::make_shared<RTFValue>(NS_ooxml::LN_Value_ST_Merge_restart);
        m_aStates.top().aTableCellSprms.set(NS_ooxml::LN_CT_TcPrBase_vMerge, pValue);
    }
    break;
    case RTF_CLVMRG:
    {
        auto pValue = std::make_shared<RTFValue>(NS_ooxml::LN_Value_ST_Merge_continue);
        m_aStates.top().aTableCellSprms.set(NS_ooxml::LN_CT_TcPrBase_vMerge, pValue);
    }
    break;
    case RTF_CLVERTALT:
    case RTF_CLVERTALC:
    case RTF_CLVERTALB:
    {
        switch (nKeyword)
        {
        case RTF_CLVERTALT:
            nParam = NS_ooxml::LN_Value_ST_VerticalJc_top;
            break;
        case RTF_CLVERTALC:
            nParam = NS_ooxml::LN_Value_ST_VerticalJc_center;
            break;
        case RTF_CLVERTALB:
            nParam = NS_ooxml::LN_Value_ST_VerticalJc_bottom;
            break;
        default:
            break;
        }
        auto pValue = std::make_shared<RTFValue>(nParam);
        m_aStates.top().aTableCellSprms.set(NS_ooxml::LN_CT_TcPrBase_vAlign, pValue);
    }
    break;
    case RTF_TRKEEP:
    {
        auto pValue = std::make_shared<RTFValue>(1);
        m_aStates.top().aTableRowSprms.set(NS_ooxml::LN_CT_TrPrBase_cantSplit, pValue);
    }
    break;
    case RTF_SECTUNLOCKED:
    {
        auto pValue = std::make_shared<RTFValue>(int(!nParam));
        m_aStates.top().aSectionSprms.set(NS_ooxml::LN_EG_SectPrContents_formProt, pValue);
    }
    break;
    case RTF_PGNDEC:
    case RTF_PGNUCRM:
    case RTF_PGNLCRM:
    case RTF_PGNLCLTR:
    case RTF_PGNBIDIA:
    case RTF_PGNBIDIB:
        // These should be mapped to NS_ooxml::LN_EG_SectPrContents_pgNumType, but dmapper has no API for that at the moment.
        break;
    case RTF_LOCH:
        m_aStates.top().eRunType = RTFParserState::LOCH;
        break;
    case RTF_HICH:
        m_aStates.top().eRunType = RTFParserState::HICH;
        break;
    case RTF_DBCH:
        m_aStates.top().eRunType = RTFParserState::DBCH;
        break;
    case RTF_TITLEPG:
    {
        auto pValue = std::make_shared<RTFValue>(1);
        m_aStates.top().aSectionSprms.set(NS_ooxml::LN_EG_SectPrContents_titlePg, pValue);
    }
    break;
    case RTF_SUPER:
    {
        if (!m_aStates.top().pCurrentBuffer)
            m_aStates.top().pCurrentBuffer = &m_aSuperBuffer;

        auto pValue = std::make_shared<RTFValue>("superscript");
        m_aStates.top().aCharacterSprms.set(NS_ooxml::LN_EG_RPrBase_vertAlign, pValue);
    }
    break;
    case RTF_SUB:
    {
        auto pValue = std::make_shared<RTFValue>("subscript");
        m_aStates.top().aCharacterSprms.set(NS_ooxml::LN_EG_RPrBase_vertAlign, pValue);
    }
    break;
    case RTF_NOSUPERSUB:
    {
        if (m_aStates.top().pCurrentBuffer == &m_aSuperBuffer)
        {
            replayBuffer(m_aSuperBuffer, nullptr, nullptr);
            m_aStates.top().pCurrentBuffer = nullptr;
        }
        m_aStates.top().aCharacterSprms.erase(NS_ooxml::LN_EG_RPrBase_vertAlign);
    }
    break;
    case RTF_LINEPPAGE:
    case RTF_LINECONT:
    {
        auto pValue = std::make_shared<RTFValue>(nKeyword == RTF_LINEPPAGE ? NS_ooxml::LN_Value_ST_LineNumberRestart_newPage : NS_ooxml::LN_Value_ST_LineNumberRestart_continuous);
        lcl_putNestedAttribute(m_aStates.top().aSectionSprms, NS_ooxml::LN_EG_SectPrContents_lnNumType, NS_ooxml::LN_CT_LineNumber_restart, pValue);
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
        auto pValue = std::make_shared<RTFValue>(NS_ooxml::LN_Value_ST_RestartNumber_eachSect);
        lcl_putNestedSprm(m_aDefaultState.aParagraphSprms, NS_ooxml::LN_EG_SectPrContents_footnotePr, NS_ooxml::LN_EG_FtnEdnNumProps_numRestart, pValue);
    }
    break;
    case RTF_NOLINE:
        lcl_eraseNestedAttribute(m_aStates.top().aSectionSprms, NS_ooxml::LN_EG_SectPrContents_lnNumType, NS_ooxml::LN_CT_LineNumber_distance);
        break;
    case RTF_FORMSHADE:
        // Noop, this is the default in Writer.
        break;
    case RTF_PNGBLIP:
        m_aStates.top().aPicture.eStyle = RTFBmpStyle::PNG;
        break;
    case RTF_JPEGBLIP:
        m_aStates.top().aPicture.eStyle = RTFBmpStyle::JPEG;
        break;
    case RTF_POSYT:
        m_aStates.top().aFrame.setSprm(NS_ooxml::LN_CT_FramePr_yAlign, NS_ooxml::LN_Value_doc_ST_YAlign_top);
        break;
    case RTF_POSYB:
        m_aStates.top().aFrame.setSprm(NS_ooxml::LN_CT_FramePr_yAlign, NS_ooxml::LN_Value_doc_ST_YAlign_bottom);
        break;
    case RTF_POSYC:
        m_aStates.top().aFrame.setSprm(NS_ooxml::LN_CT_FramePr_yAlign, NS_ooxml::LN_Value_doc_ST_YAlign_center);
        break;
    case RTF_POSYIN:
        m_aStates.top().aFrame.setSprm(NS_ooxml::LN_CT_FramePr_yAlign, NS_ooxml::LN_Value_doc_ST_YAlign_inside);
        break;
    case RTF_POSYOUT:
        m_aStates.top().aFrame.setSprm(NS_ooxml::LN_CT_FramePr_yAlign, NS_ooxml::LN_Value_doc_ST_YAlign_outside);
        break;
    case RTF_POSYIL:
        m_aStates.top().aFrame.setSprm(NS_ooxml::LN_CT_FramePr_yAlign, NS_ooxml::LN_Value_doc_ST_YAlign_inline);
        break;

    case RTF_PHMRG:
        m_aStates.top().aFrame.setSprm(NS_ooxml::LN_CT_FramePr_hAnchor, NS_ooxml::LN_Value_doc_ST_HAnchor_margin);
        break;
    case RTF_PVMRG:
        m_aStates.top().aFrame.setSprm(NS_ooxml::LN_CT_FramePr_vAnchor, NS_ooxml::LN_Value_doc_ST_VAnchor_margin);
        break;
    case RTF_PHPG:
        m_aStates.top().aFrame.setSprm(NS_ooxml::LN_CT_FramePr_hAnchor, NS_ooxml::LN_Value_doc_ST_HAnchor_page);
        break;
    case RTF_PVPG:
        m_aStates.top().aFrame.setSprm(NS_ooxml::LN_CT_FramePr_vAnchor, NS_ooxml::LN_Value_doc_ST_VAnchor_page);
        break;
    case RTF_PHCOL:
        m_aStates.top().aFrame.setSprm(NS_ooxml::LN_CT_FramePr_hAnchor, NS_ooxml::LN_Value_doc_ST_HAnchor_text);
        break;
    case RTF_PVPARA:
        m_aStates.top().aFrame.setSprm(NS_ooxml::LN_CT_FramePr_vAnchor, NS_ooxml::LN_Value_doc_ST_VAnchor_text);
        break;

    case RTF_POSXC:
        m_aStates.top().aFrame.setSprm(NS_ooxml::LN_CT_FramePr_xAlign, NS_ooxml::LN_Value_doc_ST_XAlign_center);
        break;
    case RTF_POSXI:
        m_aStates.top().aFrame.setSprm(NS_ooxml::LN_CT_FramePr_xAlign, NS_ooxml::LN_Value_doc_ST_XAlign_inside);
        break;
    case RTF_POSXO:
        m_aStates.top().aFrame.setSprm(NS_ooxml::LN_CT_FramePr_xAlign, NS_ooxml::LN_Value_doc_ST_XAlign_outside);
        break;
    case RTF_POSXL:
        m_aStates.top().aFrame.setSprm(NS_ooxml::LN_CT_FramePr_xAlign, NS_ooxml::LN_Value_doc_ST_XAlign_left);
        break;
    case RTF_POSXR:
        m_aStates.top().aFrame.setSprm(NS_ooxml::LN_CT_FramePr_xAlign, NS_ooxml::LN_Value_doc_ST_XAlign_right);
        break;

    case RTF_DPLINE:
    case RTF_DPRECT:
    case RTF_DPELLIPSE:
    case RTF_DPTXBX:
    case RTF_DPPOLYLINE:
    case RTF_DPPOLYGON:
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
        case RTF_DPPOLYGON:
            m_aStates.top().aDrawingObject.xShape.set(getModelFactory()->createInstance("com.sun.star.drawing.PolyPolygonShape"), uno::UNO_QUERY);
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
            std::vector<beans::PropertyValue> aDefaults = RTFSdrImport::getTextFrameDefaults(false);
            for (size_t i = 0; i < aDefaults.size(); ++i)
            {
                if (!lcl_findPropertyName(m_aStates.top().aDrawingObject.aPendingProperties, aDefaults[i].Name))
                    m_aStates.top().aDrawingObject.aPendingProperties.push_back(aDefaults[i]);
            }
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
        uno::Reference<drawing::XDrawPageSupplier> xDrawSupplier(m_xDstDoc, uno::UNO_QUERY);
        if (xDrawSupplier.is())
        {
            uno::Reference<drawing::XShapes> xShapes(xDrawSupplier->getDrawPage(), uno::UNO_QUERY);
            if (xShapes.is() && nKeyword != RTF_DPTXBX)
                xShapes->add(m_aStates.top().aDrawingObject.xShape);
        }
        if (nType)
        {
            uno::Reference<drawing::XEnhancedCustomShapeDefaulter> xDefaulter(m_aStates.top().aDrawingObject.xShape, uno::UNO_QUERY);
            xDefaulter->createCustomShapeDefaults(OUString::number(nType));
        }
        m_aStates.top().aDrawingObject.xPropertySet.set(m_aStates.top().aDrawingObject.xShape, uno::UNO_QUERY);
        std::vector<beans::PropertyValue>& rPendingProperties = m_aStates.top().aDrawingObject.aPendingProperties;
        for (std::vector<beans::PropertyValue>::iterator i = rPendingProperties.begin(); i != rPendingProperties.end(); ++i)
            m_aStates.top().aDrawingObject.xPropertySet->setPropertyValue(i->Name, i->Value);
        m_pSdrImport->resolveDhgt(m_aStates.top().aDrawingObject.xPropertySet, m_aStates.top().aDrawingObject.nDhgt, /*bOldStyle=*/true);
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
        aPropertyValue.Name = "VertOrientRelation";
        aPropertyValue.Value <<= text::RelOrientation::FRAME;
        m_aStates.top().aDrawingObject.aPendingProperties.push_back(aPropertyValue);
    }
    break;
    case RTF_CONTEXTUALSPACE:
    {
        auto pValue = std::make_shared<RTFValue>(1);
        m_aStates.top().aParagraphSprms.set(NS_ooxml::LN_CT_PPrBase_contextualSpacing, pValue);
    }
    break;
    case RTF_LINKSTYLES:
    {
        auto pValue = std::make_shared<RTFValue>(1);
        m_aSettingsTableSprms.set(NS_ooxml::LN_CT_Settings_linkStyles, pValue);
    }
    break;
    case RTF_PNLVLBODY:
    {
        auto pValue = std::make_shared<RTFValue>(2);
        m_aStates.top().aTableAttributes.set(NS_ooxml::LN_CT_AbstractNum_nsid, pValue);
    }
    break;
    case RTF_PNDEC:
    {
        auto pValue = std::make_shared<RTFValue>(0); // decimal, same as \levelnfc0
        m_aStates.top().aTableSprms.set(NS_ooxml::LN_CT_Lvl_numFmt, pValue);
    }
    break;
    case RTF_PNLVLBLT:
    {
        m_aStates.top().aTableAttributes.set(NS_ooxml::LN_CT_AbstractNum_nsid, std::make_shared<RTFValue>(1));
        m_aStates.top().aTableSprms.set(NS_ooxml::LN_CT_Lvl_numFmt, std::make_shared<RTFValue>(23)); // bullets, same as \levelnfc23
    }
    break;
    case RTF_LANDSCAPE:
    {
        auto pValue = std::make_shared<RTFValue>(NS_ooxml::LN_Value_ST_PageOrientation_landscape);
        lcl_putNestedAttribute(m_aDefaultState.aSectionSprms,
                               NS_ooxml::LN_EG_SectPrContents_pgSz, NS_ooxml::LN_CT_PageSz_orient, pValue);
    }
    case RTF_LNDSCPSXN:
    {
        auto pValue = std::make_shared<RTFValue>(NS_ooxml::LN_Value_ST_PageOrientation_landscape);
        lcl_putNestedAttribute(m_aStates.top().aSectionSprms,
                               NS_ooxml::LN_EG_SectPrContents_pgSz, NS_ooxml::LN_CT_PageSz_orient, pValue);
    }
    break;
    case RTF_SHPBXPAGE:
        m_aStates.top().aShape.nHoriOrientRelation = text::RelOrientation::PAGE_FRAME;
        m_aStates.top().aShape.nHoriOrientRelationToken = NS_ooxml::LN_Value_wordprocessingDrawing_ST_RelFromH_page;
        break;
    case RTF_SHPBYPAGE:
        m_aStates.top().aShape.nVertOrientRelation = text::RelOrientation::PAGE_FRAME;
        m_aStates.top().aShape.nVertOrientRelationToken = NS_ooxml::LN_Value_wordprocessingDrawing_ST_RelFromV_page;
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
        m_aStates.top().aFrame.setSprm(NS_ooxml::LN_CT_FramePr_wrap, NS_ooxml::LN_Value_doc_ST_Wrap_notBeside);
        break;
    case RTF_MNOR:
        m_bMathNor = true;
        break;
    case RTF_REVISIONS:
        m_aSettingsTableSprms.set(NS_ooxml::LN_CT_Settings_trackRevisions, std::make_shared<RTFValue>(1));
        break;
    case RTF_BRDRSH:
        lcl_putBorderProperty(m_aStates, NS_ooxml::LN_CT_Border_shadow, std::make_shared<RTFValue>(1));
        break;
    case RTF_NOCOLBAL:
        m_aSettingsTableSprms.set(NS_ooxml::LN_CT_Compat_noColumnBalance, std::make_shared<RTFValue>(1));
        break;
    case RTF_MARGMIRROR:
        m_aSettingsTableSprms.set(NS_ooxml::LN_CT_Settings_mirrorMargins, std::make_shared<RTFValue>(1));
        break;
    case RTF_SAUTOUPD:
        m_aStates.top().aTableSprms.set(NS_ooxml::LN_CT_Style_autoRedefine, std::make_shared<RTFValue>(1));
        break;
    case RTF_WIDOWCTRL:
        m_aSettingsTableSprms.set(NS_ooxml::LN_CT_Settings_widowControl, std::make_shared<RTFValue>(1));
        break;
    case RTF_LINEBETCOL:
        lcl_putNestedAttribute(m_aStates.top().aSectionSprms, NS_ooxml::LN_EG_SectPrContents_cols, NS_ooxml::LN_CT_Columns_sep, std::make_shared<RTFValue>(1));
        break;
    case RTF_PGNRESTART:
        lcl_putNestedAttribute(m_aStates.top().aSectionSprms, NS_ooxml::LN_EG_SectPrContents_pgNumType, NS_ooxml::LN_CT_PageNumber_start, std::make_shared<RTFValue>(1));
        break;
    case RTF_PGNUCLTR:
    {
        auto pIntValue = std::make_shared<RTFValue>(NS_ooxml::LN_Value_ST_NumberFormat_upperLetter);
        lcl_putNestedAttribute(m_aStates.top().aSectionSprms, NS_ooxml::LN_EG_SectPrContents_pgNumType, NS_ooxml::LN_CT_PageNumber_fmt, pIntValue);
    }
    break;
    default:
    {
        SAL_INFO("writerfilter", "TODO handle flag '" << lcl_RtfToString(nKeyword) << "'");
        aSkip.setParsed(false);
    }
    break;
    }
    return RTFError::OK;
}

RTFError RTFDocumentImpl::dispatchValue(RTFKeyword nKeyword, int nParam)
{
    setNeedSect();
    checkUnicode(/*bUnicode =*/ nKeyword != RTF_U, /*bHex =*/ true);
    RTFSkipDestination aSkip(*this);
    int nSprm = 0;
    auto pIntValue = std::make_shared<RTFValue>(nParam);
    // Trivial table sprms.
    switch (nKeyword)
    {
    case RTF_LEVELJC:
    {
        nSprm = NS_ooxml::LN_CT_Lvl_lvlJc;
        int nValue = 0;
        switch (nParam)
        {
        case 0:
            nValue = NS_ooxml::LN_Value_ST_Jc_left;
            break;
        case 1:
            nValue = NS_ooxml::LN_Value_ST_Jc_center;
            break;
        case 2:
            nValue = NS_ooxml::LN_Value_ST_Jc_right;
            break;
        }
        pIntValue = std::make_shared<RTFValue>(nValue);
        break;
    }
    case RTF_LEVELNFC:
        nSprm = NS_ooxml::LN_CT_Lvl_numFmt;
        pIntValue = std::make_shared<RTFValue>(lcl_getNumberFormat(nParam));
        break;
    case RTF_LEVELSTARTAT:
        nSprm = NS_ooxml::LN_CT_Lvl_start;
        break;
    case RTF_LEVELPICTURE:
        nSprm = NS_ooxml::LN_CT_Lvl_lvlPicBulletId;
        break;
    case RTF_SBASEDON:
        nSprm = NS_ooxml::LN_CT_Style_basedOn;
        pIntValue = std::make_shared<RTFValue>(getStyleName(nParam));
        break;
    default:
        break;
    }
    if (nSprm > 0)
    {
        m_aStates.top().aTableSprms.set(nSprm, pIntValue);
        return RTFError::OK;
    }
    // Trivial character sprms.
    switch (nKeyword)
    {
    case RTF_FS:
    case RTF_AFS:
        nSprm = (m_aStates.top().isRightToLeft || m_aStates.top().eRunType == RTFParserState::HICH) ? NS_ooxml::LN_EG_RPrBase_szCs : NS_ooxml::LN_EG_RPrBase_sz;
        break;
    case RTF_ANIMTEXT:
        nSprm = NS_ooxml::LN_EG_RPrBase_effect;
        break;
    case RTF_EXPNDTW:
        nSprm = NS_ooxml::LN_EG_RPrBase_spacing;
        break;
    case RTF_KERNING:
        nSprm = NS_ooxml::LN_EG_RPrBase_kern;
        break;
    case RTF_CHARSCALEX:
        nSprm = NS_ooxml::LN_EG_RPrBase_w;
        break;
    default:
        break;
    }
    if (nSprm > 0)
    {
        m_aStates.top().aCharacterSprms.set(nSprm, pIntValue);
        return RTFError::OK;
    }
    // Trivial character attributes.
    switch (nKeyword)
    {
    case RTF_LANG:
    case RTF_ALANG:
        if (m_aStates.top().isRightToLeft || m_aStates.top().eRunType == RTFParserState::HICH)
        {
            nSprm = NS_ooxml::LN_CT_Language_bidi;
        }
        else if (m_aStates.top().eRunType == RTFParserState::DBCH)
        {
            nSprm = NS_ooxml::LN_CT_Language_eastAsia;
        }
        else
        {
            assert(m_aStates.top().eRunType == RTFParserState::LOCH);
            nSprm = NS_ooxml::LN_CT_Language_val;
        }
        break;
    case RTF_LANGFE: // this one is always CJK apparently
        nSprm = NS_ooxml::LN_CT_Language_eastAsia;
        break;
    default:
        break;
    }
    if (nSprm > 0)
    {
        LanguageTag aTag((LanguageType)nParam);
        auto pValue = std::make_shared<RTFValue>(aTag.getBcp47());
        lcl_putNestedAttribute(m_aStates.top().aCharacterSprms, NS_ooxml::LN_EG_RPrBase_lang, nSprm, pValue);
        // Language is a character property, but we should store it at a paragraph level as well for fields.
        if (nKeyword == RTF_LANG && m_bNeedPap)
            lcl_putNestedAttribute(m_aStates.top().aParagraphSprms, NS_ooxml::LN_EG_RPrBase_lang, nSprm, pValue);
        return RTFError::OK;
    }
    // Trivial paragraph sprms.
    switch (nKeyword)
    {
    case RTF_ITAP:
        nSprm = NS_ooxml::LN_tblDepth;
        break;
    default:
        break;
    }
    if (nSprm > 0)
    {
        m_aStates.top().aParagraphSprms.set(nSprm, pIntValue);
        if (nKeyword == RTF_ITAP && nParam > 0)
        {
            while (m_aTableBufferStack.size() < sal::static_int_cast<size_t>(nParam))
            {
                m_aTableBufferStack.push_back(RTFBuffer_t());
            }
            // Invalid tables may omit INTBL after ITAP
            dispatchFlag(RTF_INTBL); // sets newly pushed buffer as current
            assert(m_aStates.top().pCurrentBuffer == &m_aTableBufferStack.back());
        }
        return RTFError::OK;
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
        return RTFError::OK;

    // Frame size / position.
    Id nId = 0;
    switch (nKeyword)
    {
    case RTF_ABSW:
        nId = NS_ooxml::LN_CT_FramePr_w;
        break;
    case RTF_ABSH:
        nId = NS_ooxml::LN_CT_FramePr_h;
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
        if (m_aStates.top().pCurrentBuffer != &m_aTableBufferStack.back())
            m_aStates.top().aFrame.setSprm(nId, nParam);

        return RTFError::OK;
    }

    // Then check for the more complex ones.
    switch (nKeyword)
    {
    case RTF_F:
    case RTF_AF:
        if (m_aStates.top().isRightToLeft || m_aStates.top().eRunType == RTFParserState::HICH)
        {
            nSprm = NS_ooxml::LN_CT_Fonts_cs;
        }
        else if (m_aStates.top().eRunType == RTFParserState::DBCH)
        {
            nSprm = NS_ooxml::LN_CT_Fonts_eastAsia;
        }
        else
        {
            assert(m_aStates.top().eRunType == RTFParserState::LOCH);
            nSprm = NS_ooxml::LN_CT_Fonts_ascii;
        }
        if (m_aStates.top().eDestination == Destination::FONTTABLE || m_aStates.top().eDestination == Destination::FONTENTRY)
        {
            m_aFontIndexes.push_back(nParam);
            m_nCurrentFontIndex = getFontIndex(nParam);
        }
        else if (m_aStates.top().eDestination == Destination::LISTLEVEL)
        {
            RTFSprms aFontAttributes;
            aFontAttributes.set(nSprm, std::make_shared<RTFValue>(m_aFontNames[getFontIndex(nParam)]));
            RTFSprms aRunPropsSprms;
            aRunPropsSprms.set(NS_ooxml::LN_EG_RPrBase_rFonts, std::make_shared<RTFValue>(aFontAttributes));
            m_aStates.top().aTableSprms.set(NS_ooxml::LN_CT_Lvl_rPr, std::make_shared<RTFValue>(RTFSprms(), aRunPropsSprms), RTFOverwrite::NO_APPEND);
        }
        else
        {
            m_nCurrentFontIndex = getFontIndex(nParam);
            auto pValue = std::make_shared<RTFValue>(getFontName(m_nCurrentFontIndex));
            lcl_putNestedAttribute(m_aStates.top().aCharacterSprms, NS_ooxml::LN_EG_RPrBase_rFonts, nSprm, pValue);
            if (nKeyword == RTF_F)
                m_aStates.top().nCurrentEncoding = getEncoding(m_nCurrentFontIndex);
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
            return RTFError::OK;

        m_nCurrentEncoding = rtl_getTextEncodingFromWindowsCodePage(aRTFEncodings[i].codepage);
        m_aStates.top().nCurrentEncoding = m_nCurrentEncoding;
    }
    break;
    case RTF_ANSICPG:
    {
        m_aDefaultState.nCurrentEncoding = rtl_getTextEncodingFromWindowsCodePage(nParam);
        m_aStates.top().nCurrentEncoding = rtl_getTextEncodingFromWindowsCodePage(nParam);
    }
    break;
    case RTF_CPG:
        m_nCurrentEncoding = rtl_getTextEncodingFromWindowsCodePage(nParam);
        m_aStates.top().nCurrentEncoding = m_nCurrentEncoding;
        break;
    case RTF_CF:
    {
        RTFSprms aAttributes;
        auto pValue = std::make_shared<RTFValue>(getColorTable(nParam));
        aAttributes.set(NS_ooxml::LN_CT_Color_val, pValue);
        m_aStates.top().aCharacterSprms.set(NS_ooxml::LN_EG_RPrBase_color, std::make_shared<RTFValue>(aAttributes));
    }
    break;
    case RTF_S:
    {
        m_aStates.top().nCurrentStyleIndex = nParam;

        if (m_aStates.top().eDestination == Destination::STYLESHEET || m_aStates.top().eDestination == Destination::STYLEENTRY)
        {
            m_nCurrentStyleIndex = nParam;
            auto pValue = std::make_shared<RTFValue>(NS_ooxml::LN_Value_ST_StyleType_paragraph);
            m_aStates.top().aTableAttributes.set(NS_ooxml::LN_CT_Style_type, pValue); // paragraph style
        }
        else
        {
            OUString aName = getStyleName(nParam);
            if (!aName.isEmpty())
            {
                if (m_aStates.top().eDestination == Destination::LISTLEVEL)
                    m_aStates.top().aTableSprms.set(NS_ooxml::LN_CT_Lvl_pStyle, std::make_shared<RTFValue>(aName));
                else
                    m_aStates.top().aParagraphSprms.set(NS_ooxml::LN_CT_PPrBase_pStyle, std::make_shared<RTFValue>(aName));

            }
        }
    }
    break;
    case RTF_CS:
        m_aStates.top().nCurrentCharacterStyleIndex = nParam;
        if (m_aStates.top().eDestination == Destination::STYLESHEET || m_aStates.top().eDestination == Destination::STYLEENTRY)
        {
            m_nCurrentStyleIndex = nParam;
            auto pValue = std::make_shared<RTFValue>(NS_ooxml::LN_Value_ST_StyleType_character);
            m_aStates.top().aTableAttributes.set(NS_ooxml::LN_CT_Style_type, pValue); // character style
        }
        else
        {
            OUString aName = getStyleName(nParam);
            if (!aName.isEmpty())
                m_aStates.top().aCharacterSprms.set(NS_ooxml::LN_EG_RPrBase_rStyle, std::make_shared<RTFValue>(aName));
        }
        break;
    case RTF_DS:
        if (m_aStates.top().eDestination == Destination::STYLESHEET || m_aStates.top().eDestination == Destination::STYLEENTRY)
        {
            m_nCurrentStyleIndex = nParam;
            auto pValue = std::make_shared<RTFValue>(0); // TODO no value in enum StyleType?
            m_aStates.top().aTableAttributes.set(NS_ooxml::LN_CT_Style_type, pValue); // section style
        }
        break;
    case RTF_TS:
        if (m_aStates.top().eDestination == Destination::STYLESHEET || m_aStates.top().eDestination == Destination::STYLEENTRY)
        {
            m_nCurrentStyleIndex = nParam;
            // FIXME the correct value would be NS_ooxml::LN_Value_ST_StyleType_table but maybe table styles mess things up in dmapper, be cautious and disable them for now
            auto pValue = std::make_shared<RTFValue>(0);
            m_aStates.top().aTableAttributes.set(NS_ooxml::LN_CT_Style_type, pValue); // table style
        }
        break;
    case RTF_DEFF:
        m_nDefaultFontIndex = nParam;
        break;
    case RTF_DEFLANG:
    case RTF_ADEFLANG:
    {
        LanguageTag aTag((LanguageType)nParam);
        auto pValue = std::make_shared<RTFValue>(aTag.getBcp47());
        lcl_putNestedAttribute(m_aStates.top().aCharacterSprms, (nKeyword == RTF_DEFLANG ? NS_ooxml::LN_EG_RPrBase_lang : NS_ooxml::LN_CT_Language_bidi), nSprm, pValue);
    }
    break;
    case RTF_CHCBPAT:
    {
        auto pValue = std::make_shared<RTFValue>(nParam ? getColorTable(nParam) : COL_AUTO);
        lcl_putNestedAttribute(m_aStates.top().aCharacterSprms, NS_ooxml::LN_EG_RPrBase_shd, NS_ooxml::LN_CT_Shd_fill, pValue);
    }
    break;
    case RTF_CLCBPAT:
    case RTF_CLCBPATRAW:
    {
        auto pValue = std::make_shared<RTFValue>(getColorTable(nParam));
        lcl_putNestedAttribute(m_aStates.top().aTableCellSprms, NS_ooxml::LN_CT_TcPrBase_shd, NS_ooxml::LN_CT_Shd_fill, pValue);
    }
    break;
    case RTF_CBPAT:
        if (nParam)
        {
            auto pValue = std::make_shared<RTFValue>(getColorTable(nParam));
            lcl_putNestedAttribute(m_aStates.top().aParagraphSprms, NS_ooxml::LN_CT_PrBase_shd, NS_ooxml::LN_CT_Shd_fill, pValue);
        }
        break;
    case RTF_ULC:
    {
        auto pValue = std::make_shared<RTFValue>(getColorTable(nParam));
        m_aStates.top().aCharacterSprms.set(0x6877, pValue);
    }
    break;
    case RTF_HIGHLIGHT:
    {
        auto pValue = std::make_shared<RTFValue>(nParam ? getColorTable(nParam) : COL_AUTO);
        m_aStates.top().aCharacterSprms.set(NS_ooxml::LN_EG_RPrBase_highlight, pValue);
    }
    break;
    case RTF_UP:
    case RTF_DN:
    {
        auto pValue = std::make_shared<RTFValue>(nParam * (nKeyword == RTF_UP ? 1 : -1));
        m_aStates.top().aCharacterSprms.set(NS_ooxml::LN_EG_RPrBase_position, pValue);
    }
    break;
    case RTF_HORZVERT:
    {
        auto pValue = std::make_shared<RTFValue>(int(true));
        m_aStates.top().aCharacterAttributes.set(NS_ooxml::LN_CT_EastAsianLayout_vert, pValue);
        if (nParam)
            // rotate fits to a single line
            m_aStates.top().aCharacterAttributes.set(NS_ooxml::LN_CT_EastAsianLayout_vertCompress, pValue);
    }
    break;
    case RTF_EXPND:
    {
        auto pValue = std::make_shared<RTFValue>(nParam/5);
        m_aStates.top().aCharacterSprms.set(NS_ooxml::LN_EG_RPrBase_spacing, pValue);
    }
    break;
    case RTF_TWOINONE:
    {
        auto pValue = std::make_shared<RTFValue>(int(true));
        m_aStates.top().aCharacterAttributes.set(NS_ooxml::LN_CT_EastAsianLayout_combine, pValue);
        nId = 0;
        switch (nParam)
        {
        case 0:
            nId = NS_ooxml::LN_Value_ST_CombineBrackets_none;
            break;
        case 1:
            nId = NS_ooxml::LN_Value_ST_CombineBrackets_round;
            break;
        case 2:
            nId = NS_ooxml::LN_Value_ST_CombineBrackets_square;
            break;
        case 3:
            nId = NS_ooxml::LN_Value_ST_CombineBrackets_angle;
            break;
        case 4:
            nId = NS_ooxml::LN_Value_ST_CombineBrackets_curly;
            break;
        }
        if (nId > 0)
            m_aStates.top().aCharacterAttributes.set(NS_ooxml::LN_CT_EastAsianLayout_combineBrackets, std::make_shared<RTFValue>(nId));
    }
    break;
    case RTF_SL:
    {
        // This is similar to RTF_ABSH, negative value means 'exact', positive means 'at least'.
        auto pValue = std::make_shared<RTFValue>(NS_ooxml::LN_Value_doc_ST_LineSpacingRule_atLeast);
        if (nParam < 0)
        {
            pValue = std::make_shared<RTFValue>(NS_ooxml::LN_Value_doc_ST_LineSpacingRule_exact);
            pIntValue = std::make_shared<RTFValue>(-nParam);
        }
        m_aStates.top().aParagraphAttributes.set(NS_ooxml::LN_CT_Spacing_lineRule, pValue);
        m_aStates.top().aParagraphAttributes.set(NS_ooxml::LN_CT_Spacing_line, pIntValue);
    }
    break;
    case RTF_SLMULT:
        if (nParam > 0)
        {
            auto pValue = std::make_shared<RTFValue>(NS_ooxml::LN_Value_doc_ST_LineSpacingRule_auto);
            m_aStates.top().aParagraphAttributes.set(NS_ooxml::LN_CT_Spacing_lineRule, pValue);
        }
        break;
    case RTF_BRDRW:
    {
        // dmapper expects it in 1/8 pt, we have it in twip - but avoid rounding 1 to 0
        if (nParam > 1)
            nParam = nParam * 2 / 5;
        auto pValue = std::make_shared<RTFValue>(nParam);
        lcl_putBorderProperty(m_aStates, NS_ooxml::LN_CT_Border_sz, pValue);
    }
    break;
    case RTF_BRDRCF:
    {
        auto pValue = std::make_shared<RTFValue>(getColorTable(nParam));
        lcl_putBorderProperty(m_aStates, NS_ooxml::LN_CT_Border_color, pValue);
    }
    break;
    case RTF_BRSP:
    {
        // dmapper expects it in points, we have it in twip
        auto pValue = std::make_shared<RTFValue>(nParam / 20);
        lcl_putBorderProperty(m_aStates, NS_ooxml::LN_CT_Border_space, pValue);
    }
    break;
    case RTF_TX:
    {
        m_aStates.top().aTabAttributes.set(NS_ooxml::LN_CT_TabStop_pos, pIntValue);
        auto pValue = std::make_shared<RTFValue>(m_aStates.top().aTabAttributes);
        lcl_putNestedSprm(m_aStates.top().aParagraphSprms, NS_ooxml::LN_CT_PPrBase_tabs, NS_ooxml::LN_CT_Tabs_tab, pValue);
        m_aStates.top().aTabAttributes.clear();
    }
    break;
    case RTF_ILVL:
        lcl_putNestedSprm(m_aStates.top().aParagraphSprms, NS_ooxml::LN_CT_PPrBase_numPr, NS_ooxml::LN_CT_NumPr_ilvl, pIntValue);
        break;
    case RTF_LISTTEMPLATEID:
        // This one is not referenced anywhere, so it's pointless to store it at the moment.
        break;
    case RTF_LISTID:
    {
        if (m_aStates.top().eDestination == Destination::LISTENTRY)
            m_aStates.top().aTableAttributes.set(NS_ooxml::LN_CT_AbstractNum_abstractNumId, pIntValue);
        else if (m_aStates.top().eDestination == Destination::LISTOVERRIDEENTRY)
            m_aStates.top().aTableSprms.set(NS_ooxml::LN_CT_Num_abstractNumId, pIntValue);
    }
    break;
    case RTF_LS:
    {
        if (m_aStates.top().eDestination == Destination::LISTOVERRIDEENTRY)
            m_aStates.top().aTableAttributes.set(NS_ooxml::LN_CT_AbstractNum_nsid, pIntValue);
        else
            lcl_putNestedSprm(m_aStates.top().aParagraphSprms, NS_ooxml::LN_CT_PPrBase_tabs, NS_ooxml::LN_CT_NumPr_numId, pIntValue);
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
            if (m_aStates.top().eDestination == Destination::LEVELNUMBERS)
            {
                if (nParam != ';')
                    m_aStates.top().aLevelNumbers.push_back(sal_Int32(nParam));
            }
            else
                m_aUnicodeBuffer.append(static_cast<sal_Unicode>(nParam));
            m_aStates.top().nCharsToSkip = m_aStates.top().nUc;
        }
        break;
    case RTF_LEVELFOLLOW:
    {
        OUString sValue;
        switch (nParam)
        {
        case 0:
            sValue = "tab";
            break;
        case 1:
            sValue = "space";
            break;
        case 2:
            sValue = "nothing";
            break;
        }
        if (!sValue.isEmpty())
            m_aStates.top().aTableSprms.set(NS_ooxml::LN_CT_Lvl_suff, std::make_shared<RTFValue>(sValue));
    }
    break;
    case RTF_FPRQ:
    {
        sal_Int32 nValue = 0;
        switch (nParam)
        {
        case 0:
            nValue = NS_ooxml::LN_Value_ST_Pitch_default;
            break;
        case 1:
            nValue = NS_ooxml::LN_Value_ST_Pitch_fixed;
            break;
        case 2:
            nValue = NS_ooxml::LN_Value_ST_Pitch_variable;
            break;
        }
        if (nValue)
        {
            RTFSprms aAttributes;
            aAttributes.set(NS_ooxml::LN_CT_Pitch_val, std::make_shared<RTFValue>(nValue));
            m_aStates.top().aTableSprms.set(NS_ooxml::LN_CT_Font_pitch, std::make_shared<RTFValue>(aAttributes));
        }
    }
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
        m_aStates.top().aPicture.nGoalWidth = convertTwipToMm100(nParam);
        break;
    case RTF_PICHGOAL:
        m_aStates.top().aPicture.nGoalHeight = convertTwipToMm100(nParam);
        break;
    case RTF_PICCROPL:
        m_aStates.top().aPicture.nCropL = convertTwipToMm100(nParam);
        break;
    case RTF_PICCROPR:
        m_aStates.top().aPicture.nCropR = convertTwipToMm100(nParam);
        break;
    case RTF_PICCROPT:
        m_aStates.top().aPicture.nCropT = convertTwipToMm100(nParam);
        break;
    case RTF_PICCROPB:
        m_aStates.top().aPicture.nCropB = convertTwipToMm100(nParam);
        break;
    case RTF_SHPWRK:
    {
        int nValue = 0;
        switch (nParam)
        {
        case 0:
            nValue = NS_ooxml::LN_Value_wordprocessingDrawing_ST_WrapText_bothSides;
            break;
        case 1:
            nValue = NS_ooxml::LN_Value_wordprocessingDrawing_ST_WrapText_left;
            break;
        case 2:
            nValue = NS_ooxml::LN_Value_wordprocessingDrawing_ST_WrapText_right;
            break;
        case 3:
            nValue = NS_ooxml::LN_Value_wordprocessingDrawing_ST_WrapText_largest;
            break;
        default:
            break;
        }
        auto pValue = std::make_shared<RTFValue>(nValue);
        RTFValue::Pointer_t pTight = m_aStates.top().aCharacterSprms.find(NS_ooxml::LN_EG_WrapType_wrapTight);
        if (pTight)
            pTight->getAttributes().set(NS_ooxml::LN_CT_WrapTight_wrapText, pValue);
        else
            m_aStates.top().aCharacterAttributes.set(NS_ooxml::LN_CT_WrapSquare_wrapText, pValue);
    }
    break;
    case RTF_SHPWR:
    {
        switch (nParam)
        {
        case 1:
            m_aStates.top().aShape.nWrap = css::text::WrapTextMode_NONE;
            break;
        case 2:
            m_aStates.top().aShape.nWrap = css::text::WrapTextMode_PARALLEL;
            break;
        case 3:
            m_aStates.top().aShape.nWrap = css::text::WrapTextMode_THROUGHT;
            m_aStates.top().aCharacterSprms.set(NS_ooxml::LN_EG_WrapType_wrapNone, std::make_shared<RTFValue>());
            break;
        case 4:
            m_aStates.top().aShape.nWrap = css::text::WrapTextMode_PARALLEL;
            m_aStates.top().aCharacterSprms.set(NS_ooxml::LN_EG_WrapType_wrapTight, std::make_shared<RTFValue>());
            break;
        case 5:
            m_aStates.top().aShape.nWrap = css::text::WrapTextMode_THROUGHT;
            break;
        }
    }
    break;
    case RTF_CELLX:
    {
        int& rCurrentCellX((Destination::NESTEDTABLEPROPERTIES == m_aStates.top().eDestination) ? m_nNestedCurrentCellX : m_nTopLevelCurrentCellX);
        int nCellX = nParam - rCurrentCellX;
        const int COL_DFLT_WIDTH = 41; // sw/source/filter/inc/wrtswtbl.hxx, minimal possible width of cells.
        if (!nCellX)
            nCellX = COL_DFLT_WIDTH;

        // If there is a negative left margin, then the first cellx is relative to that.
        RTFValue::Pointer_t pTblInd = m_aStates.top().aTableRowSprms.find(NS_ooxml::LN_CT_TblPrBase_tblInd);
        if (rCurrentCellX == 0 && pTblInd.get())
        {
            RTFValue::Pointer_t pWidth = pTblInd->getAttributes().find(NS_ooxml::LN_CT_TblWidth_w);
            if (pWidth.get() && pWidth->getInt() < 0)
                nCellX = -1 * (pWidth->getInt() - nParam);
        }

        rCurrentCellX = nParam;
        auto pXValue = std::make_shared<RTFValue>(nCellX);
        m_aStates.top().aTableRowSprms.set(NS_ooxml::LN_CT_TblGridBase_gridCol, pXValue, RTFOverwrite::NO_APPEND);
        if (Destination::NESTEDTABLEPROPERTIES == m_aStates.top().eDestination)
        {
            m_nNestedCells++;
            // Push cell properties.
            m_aNestedTableCellsSprms.push_back(
                m_aStates.top().aTableCellSprms);
            m_aNestedTableCellsAttributes.push_back(
                m_aStates.top().aTableCellAttributes);
        }
        else
        {
            m_nTopLevelCells++;
            // Push cell properties.
            m_aTopLevelTableCellsSprms.push_back(
                m_aStates.top().aTableCellSprms);
            m_aTopLevelTableCellsAttributes.push_back(
                m_aStates.top().aTableCellAttributes);
        }

        m_aStates.top().aTableCellSprms = m_aDefaultState.aTableCellSprms;
        m_aStates.top().aTableCellAttributes = m_aDefaultState.aTableCellAttributes;
        // We assume text after a row definition always belongs to the table, to handle text before the real INTBL token
        dispatchFlag(RTF_INTBL);
        if (!m_nCellxMax)
        {
            // Wasn't in table, but now is -> tblStart.
            RTFSprms aAttributes;
            RTFSprms aSprms;
            aSprms.set(NS_ooxml::LN_tblStart, std::make_shared<RTFValue>(1));
            writerfilter::Reference<Properties>::Pointer_t pProperties = std::make_shared<RTFReferenceProperties>(aAttributes, aSprms);
            Mapper().props(pProperties);
        }
        m_nCellxMax = std::max(m_nCellxMax, nParam);
    }
    break;
    case RTF_TRRH:
    {
        OUString hRule("auto");
        if (nParam < 0)
        {
            auto pAbsValue = std::make_shared<RTFValue>(-nParam);
            pIntValue.swap(pAbsValue);

            hRule = "exact";
        }
        else if (nParam > 0)
            hRule = "atLeast";

        lcl_putNestedAttribute(m_aStates.top().aTableRowSprms, NS_ooxml::LN_CT_TrPrBase_trHeight, NS_ooxml::LN_CT_Height_val, pIntValue);

        auto pHRule = std::make_shared<RTFValue>(hRule);
        lcl_putNestedAttribute(m_aStates.top().aTableRowSprms, NS_ooxml::LN_CT_TrPrBase_trHeight, NS_ooxml::LN_CT_Height_hRule, pHRule);
    }
    break;
    case RTF_TRLEFT:
    {
        // the value is in twips
        lcl_putNestedAttribute(m_aStates.top().aTableRowSprms,
                               NS_ooxml::LN_CT_TblPrBase_tblInd, NS_ooxml::LN_CT_TblWidth_type,
                               std::make_shared<RTFValue>(NS_ooxml::LN_Value_ST_TblWidth_dxa));
        lcl_putNestedAttribute(m_aStates.top().aTableRowSprms,
                               NS_ooxml::LN_CT_TblPrBase_tblInd, NS_ooxml::LN_CT_TblWidth_w,
                               std::make_shared<RTFValue>(nParam));
    }
    break;
    case RTF_COLS:
        lcl_putNestedAttribute(m_aStates.top().aSectionSprms, NS_ooxml::LN_EG_SectPrContents_cols, NS_ooxml::LN_CT_Columns_num, pIntValue);
        break;
    case RTF_COLSX:
        lcl_putNestedAttribute(m_aStates.top().aSectionSprms, NS_ooxml::LN_EG_SectPrContents_cols, NS_ooxml::LN_CT_Columns_space, pIntValue);
        break;
    case RTF_COLNO:
        lcl_putNestedSprm(m_aStates.top().aSectionSprms, NS_ooxml::LN_EG_SectPrContents_cols, NS_ooxml::LN_CT_Columns_col, pIntValue);
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
                               NS_ooxml::LN_EG_SectPrContents_pgSz, NS_ooxml::LN_CT_PageSz_h, pIntValue);
    case RTF_PGHSXN:
        lcl_putNestedAttribute(m_aStates.top().aSectionSprms,
                               NS_ooxml::LN_EG_SectPrContents_pgSz, NS_ooxml::LN_CT_PageSz_h, pIntValue);
        break;
    case RTF_PAPERW: // fall through: set the default + current value
        lcl_putNestedAttribute(m_aDefaultState.aSectionSprms,
                               NS_ooxml::LN_EG_SectPrContents_pgSz, NS_ooxml::LN_CT_PageSz_w, pIntValue);
    case RTF_PGWSXN:
        lcl_putNestedAttribute(m_aStates.top().aSectionSprms,
                               NS_ooxml::LN_EG_SectPrContents_pgSz, NS_ooxml::LN_CT_PageSz_w, pIntValue);
        break;
    case RTF_MARGL: // fall through: set the default + current value
        lcl_putNestedAttribute(m_aDefaultState.aSectionSprms,
                               NS_ooxml::LN_EG_SectPrContents_pgMar, NS_ooxml::LN_CT_PageMar_left, pIntValue);
    case RTF_MARGLSXN:
        lcl_putNestedAttribute(m_aStates.top().aSectionSprms,
                               NS_ooxml::LN_EG_SectPrContents_pgMar, NS_ooxml::LN_CT_PageMar_left, pIntValue);
        break;
    case RTF_MARGR: // fall through: set the default + current value
        lcl_putNestedAttribute(m_aDefaultState.aSectionSprms,
                               NS_ooxml::LN_EG_SectPrContents_pgMar, NS_ooxml::LN_CT_PageMar_right, pIntValue);
    case RTF_MARGRSXN:
        lcl_putNestedAttribute(m_aStates.top().aSectionSprms,
                               NS_ooxml::LN_EG_SectPrContents_pgMar, NS_ooxml::LN_CT_PageMar_right, pIntValue);
        break;
    case RTF_MARGT: // fall through: set the default + current value
        lcl_putNestedAttribute(m_aDefaultState.aSectionSprms, NS_ooxml::LN_EG_SectPrContents_pgMar, NS_ooxml::LN_CT_PageMar_top, pIntValue);
    case RTF_MARGTSXN:
        lcl_putNestedAttribute(m_aStates.top().aSectionSprms, NS_ooxml::LN_EG_SectPrContents_pgMar, NS_ooxml::LN_CT_PageMar_top, pIntValue);
        break;
    case RTF_MARGB: // fall through: set the default + current value
        lcl_putNestedAttribute(m_aDefaultState.aSectionSprms, NS_ooxml::LN_EG_SectPrContents_pgMar, NS_ooxml::LN_CT_PageMar_bottom, pIntValue);
    case RTF_MARGBSXN:
        lcl_putNestedAttribute(m_aStates.top().aSectionSprms, NS_ooxml::LN_EG_SectPrContents_pgMar, NS_ooxml::LN_CT_PageMar_bottom, pIntValue);
        break;
    case RTF_HEADERY:
        lcl_putNestedAttribute(m_aStates.top().aSectionSprms, NS_ooxml::LN_EG_SectPrContents_pgMar, NS_ooxml::LN_CT_PageMar_header, pIntValue);
        break;
    case RTF_FOOTERY:
        lcl_putNestedAttribute(m_aStates.top().aSectionSprms, NS_ooxml::LN_EG_SectPrContents_pgMar, NS_ooxml::LN_CT_PageMar_footer, pIntValue);
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
        auto pValue = std::make_shared<RTFValue>(m_aAuthors[nParam]);
        lcl_putNestedAttribute(m_aStates.top().aCharacterSprms, NS_ooxml::LN_trackchange, NS_ooxml::LN_CT_TrackChange_author, pValue);
    }
    break;
    case RTF_REVDTTM:
    case RTF_REVDTTMDEL:
    {
        OUString aStr(OStringToOUString(lcl_DTTM22OString(nParam), m_aStates.top().nCurrentEncoding));
        auto pValue = std::make_shared<RTFValue>(aStr);
        lcl_putNestedAttribute(m_aStates.top().aCharacterSprms, NS_ooxml::LN_trackchange, NS_ooxml::LN_CT_TrackChange_date, pValue);
    }
    break;
    case RTF_SHPLEFT:
        m_aStates.top().aShape.nLeft = convertTwipToMm100(nParam);
        break;
    case RTF_SHPTOP:
        m_aStates.top().aShape.nTop = convertTwipToMm100(nParam);
        break;
    case RTF_SHPRIGHT:
        m_aStates.top().aShape.nRight = convertTwipToMm100(nParam);
        break;
    case RTF_SHPBOTTOM:
        m_aStates.top().aShape.nBottom = convertTwipToMm100(nParam);
        break;
    case RTF_SHPZ:
        m_aStates.top().aShape.oZ.reset(nParam);
        break;
    case RTF_FFTYPE:
        switch (nParam)
        {
        case 0:
            m_nFormFieldType = RTFFormFieldType::TEXT;
            break;
        case 1:
            m_nFormFieldType = RTFFormFieldType::CHECKBOX;
            break;
        case 2:
            m_nFormFieldType = RTFFormFieldType::LIST;
            break;
        default:
            m_nFormFieldType = RTFFormFieldType::NONE;
            break;
        }
        break;
    case RTF_FFDEFRES:
        if (m_nFormFieldType == RTFFormFieldType::CHECKBOX)
            m_aFormfieldSprms.set(NS_ooxml::LN_CT_FFCheckBox_default, pIntValue);
        else if (m_nFormFieldType == RTFFormFieldType::LIST)
            m_aFormfieldSprms.set(NS_ooxml::LN_CT_FFDDList_default, pIntValue);
        break;
    case RTF_FFRES:
        // 25 means undefined, see [MS-DOC] 2.9.79, FFDataBits.
        if (m_nFormFieldType == RTFFormFieldType::CHECKBOX && nParam != 25)
            m_aFormfieldSprms.set(NS_ooxml::LN_CT_FFCheckBox_checked, pIntValue);
        else if (m_nFormFieldType == RTFFormFieldType::LIST)
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
            comphelper::SequenceAsHashMap aSeq = m_xDocumentProperties->getDocumentStatistics();
            OUString aName;
            switch (nKeyword)
            {
            case RTF_NOFPAGES:
                aName = "PageCount";
                nParam = 99;
                break;
            case RTF_NOFWORDS:
                aName = "WordCount";
                break;
            case RTF_NOFCHARS:
                aName = "CharacterCount";
                break;
            case RTF_NOFCHARSWS:
                aName = "NonWhitespaceCharacterCount";
                break;
            default:
                break;
            }
            if (!aName.isEmpty())
            {
                aSeq[aName] = uno::makeAny(sal_Int32(nParam));
                m_xDocumentProperties->setDocumentStatistics(aSeq.getAsConstNamedValueList());
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
        m_aStates.top().aFrame.setSprm(NS_ooxml::LN_CT_FramePr_hSpace, nParam);
        break;
    case RTF_DFRMTXTY:
        m_aStates.top().aFrame.setSprm(NS_ooxml::LN_CT_FramePr_vSpace, nParam);
        break;
    case RTF_DXFRTEXT:
    {
        m_aStates.top().aFrame.setSprm(NS_ooxml::LN_CT_FramePr_hSpace, nParam);
        m_aStates.top().aFrame.setSprm(NS_ooxml::LN_CT_FramePr_vSpace, nParam);
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
        lcl_putNestedAttribute(m_aStates.top().aParagraphSprms, NS_ooxml::LN_CT_PPrBase_spacing, NS_ooxml::LN_CT_Spacing_before, pIntValue);
        break;
    case RTF_SA:
        lcl_putNestedAttribute(m_aStates.top().aParagraphSprms, NS_ooxml::LN_CT_PPrBase_spacing, NS_ooxml::LN_CT_Spacing_after, pIntValue);
        break;
    case RTF_DPX:
        m_aStates.top().aDrawingObject.nLeft = convertTwipToMm100(nParam);
        break;
    case RTF_DPY:
        m_aStates.top().aDrawingObject.nTop = convertTwipToMm100(nParam);
        break;
    case RTF_DPXSIZE:
        m_aStates.top().aDrawingObject.nRight = convertTwipToMm100(nParam);
        break;
    case RTF_DPYSIZE:
        m_aStates.top().aDrawingObject.nBottom = convertTwipToMm100(nParam);
        break;
    case RTF_PNSTART:
        m_aStates.top().aTableSprms.set(NS_ooxml::LN_CT_Lvl_start, pIntValue);
        break;
    case RTF_PNF:
    {
        auto pValue = std::make_shared<RTFValue>(m_aFontNames[getFontIndex(nParam)]);
        RTFSprms aAttributes;
        aAttributes.set(NS_ooxml::LN_CT_Fonts_ascii, pValue);
        lcl_putNestedSprm(m_aStates.top().aTableSprms, NS_ooxml::LN_CT_Lvl_rPr, NS_ooxml::LN_EG_RPrBase_rFonts, std::make_shared<RTFValue>(aAttributes));
    }
    break;
    case RTF_VIEWSCALE:
        m_aSettingsTableAttributes.set(NS_ooxml::LN_CT_Zoom_percent, pIntValue);
        break;
    case RTF_BIN:
    {
        m_aStates.top().nInternalState = RTFInternalState::BIN;
        m_aStates.top().nBinaryToRead = nParam;
    }
    break;
    case RTF_DPLINECOR:
        m_aStates.top().aDrawingObject.nLineColorR = nParam;
        m_aStates.top().aDrawingObject.bHasLineColor = true;
        break;
    case RTF_DPLINECOG:
        m_aStates.top().aDrawingObject.nLineColorG = nParam;
        m_aStates.top().aDrawingObject.bHasLineColor = true;
        break;
    case RTF_DPLINECOB:
        m_aStates.top().aDrawingObject.nLineColorB = nParam;
        m_aStates.top().aDrawingObject.bHasLineColor = true;
        break;
    case RTF_DPFILLBGCR:
        m_aStates.top().aDrawingObject.nFillColorR = nParam;
        m_aStates.top().aDrawingObject.bHasFillColor = true;
        break;
    case RTF_DPFILLBGCG:
        m_aStates.top().aDrawingObject.nFillColorG = nParam;
        m_aStates.top().aDrawingObject.bHasFillColor = true;
        break;
    case RTF_DPFILLBGCB:
        m_aStates.top().aDrawingObject.nFillColorB = nParam;
        m_aStates.top().aDrawingObject.bHasFillColor = true;
        break;
    case RTF_CLSHDNG:
    {
        int nValue = -1;
        switch (nParam)
        {
        case 500:
            nValue = NS_ooxml::LN_Value_ST_Shd_pct5;
            break;
        case 1000:
            nValue = NS_ooxml::LN_Value_ST_Shd_pct10;
            break;
        case 1200:
            nValue = NS_ooxml::LN_Value_ST_Shd_pct12;
            break;
        case 1500:
            nValue = NS_ooxml::LN_Value_ST_Shd_pct15;
            break;
        case 2000:
            nValue = NS_ooxml::LN_Value_ST_Shd_pct20;
            break;
        case 2500:
            nValue = NS_ooxml::LN_Value_ST_Shd_pct25;
            break;
        case 3000:
            nValue = NS_ooxml::LN_Value_ST_Shd_pct30;
            break;
        case 3500:
            nValue = NS_ooxml::LN_Value_ST_Shd_pct35;
            break;
        case 3700:
            nValue = NS_ooxml::LN_Value_ST_Shd_pct37;
            break;
        case 4000:
            nValue = NS_ooxml::LN_Value_ST_Shd_pct40;
            break;
        case 4500:
            nValue = NS_ooxml::LN_Value_ST_Shd_pct45;
            break;
        case 5000:
            nValue = NS_ooxml::LN_Value_ST_Shd_pct50;
            break;
        case 5500:
            nValue = NS_ooxml::LN_Value_ST_Shd_pct55;
            break;
        case 6000:
            nValue = NS_ooxml::LN_Value_ST_Shd_pct60;
            break;
        case 6200:
            nValue = NS_ooxml::LN_Value_ST_Shd_pct62;
            break;
        case 6500:
            nValue = NS_ooxml::LN_Value_ST_Shd_pct65;
            break;
        case 7000:
            nValue = NS_ooxml::LN_Value_ST_Shd_pct70;
            break;
        case 7500:
            nValue = NS_ooxml::LN_Value_ST_Shd_pct75;
            break;
        case 8000:
            nValue = NS_ooxml::LN_Value_ST_Shd_pct80;
            break;
        case 8500:
            nValue = NS_ooxml::LN_Value_ST_Shd_pct85;
            break;
        case 8700:
            nValue = NS_ooxml::LN_Value_ST_Shd_pct87;
            break;
        case 9000:
            nValue = NS_ooxml::LN_Value_ST_Shd_pct90;
            break;
        case 9500:
            nValue = NS_ooxml::LN_Value_ST_Shd_pct95;
            break;
        default:
            break;
        }
        if (nValue != -1)
            lcl_putNestedAttribute(m_aStates.top().aTableCellSprms, NS_ooxml::LN_CT_TcPrBase_shd, NS_ooxml::LN_CT_Shd_val, std::make_shared<RTFValue>(nValue));
    }
    break;
    case RTF_DODHGT:
        m_aStates.top().aDrawingObject.nDhgt = nParam;
        break;
    case RTF_DPPOLYCOUNT:
        if (nParam >= 0)
        {
            m_aStates.top().aDrawingObject.nPolyLineCount = nParam;
        }
        break;
    case RTF_DPPTX:
    {
        RTFDrawingObject& rDrawingObject = m_aStates.top().aDrawingObject;

        if (rDrawingObject.aPolyLinePoints.empty())
            dispatchValue(RTF_DPPOLYCOUNT, 2);

        rDrawingObject.aPolyLinePoints.push_back(awt::Point(convertTwipToMm100(nParam), 0));
    }
    break;
    case RTF_DPPTY:
    {
        RTFDrawingObject& rDrawingObject = m_aStates.top().aDrawingObject;
        if (!rDrawingObject.aPolyLinePoints.empty())
        {
            rDrawingObject.aPolyLinePoints.back().Y = convertTwipToMm100(nParam);
            rDrawingObject.nPolyLineCount--;
            if (rDrawingObject.nPolyLineCount == 0)
            {
                uno::Sequence< uno::Sequence<awt::Point> >aPointSequenceSequence =
                {
                    comphelper::containerToSequence(rDrawingObject.aPolyLinePoints)
                };
                rDrawingObject.xPropertySet->setPropertyValue("PolyPolygon", uno::Any(aPointSequenceSequence));
            }
        }
    }
    break;
    case RTF_SHPFBLWTXT:
        // Shape is below text -> send it to the background.
        m_aStates.top().aShape.bInBackground = nParam;
        break;
    case RTF_CLPADB:
    case RTF_CLPADL:
    case RTF_CLPADR:
    case RTF_CLPADT:
    {
        RTFSprms aAttributes;
        aAttributes.set(NS_ooxml::LN_CT_TblWidth_type, std::make_shared<RTFValue>(NS_ooxml::LN_Value_ST_TblWidth_dxa));
        aAttributes.set(NS_ooxml::LN_CT_TblWidth_w, std::make_shared<RTFValue>(nParam));
        // Top and left is swapped, that's what Word does.
        switch (nKeyword)
        {
        case RTF_CLPADB:
            nSprm = NS_ooxml::LN_CT_TcMar_bottom;
            break;
        case RTF_CLPADL:
            nSprm = NS_ooxml::LN_CT_TcMar_top;
            break;
        case RTF_CLPADR:
            nSprm = NS_ooxml::LN_CT_TcMar_right;
            break;
        case RTF_CLPADT:
            nSprm = NS_ooxml::LN_CT_TcMar_left;
            break;
        default:
            break;
        }
        lcl_putNestedSprm(m_aStates.top().aTableCellSprms, NS_ooxml::LN_CT_TcPrBase_tcMar, nSprm, std::make_shared<RTFValue>(aAttributes));
    }
    break;
    case RTF_TRPADDFB:
    case RTF_TRPADDFL:
    case RTF_TRPADDFR:
    case RTF_TRPADDFT:
    {
        RTFSprms aAttributes;
        switch (nParam)
        {
        case 3:
            aAttributes.set(NS_ooxml::LN_CT_TblWidth_type, std::make_shared<RTFValue>(NS_ooxml::LN_Value_ST_TblWidth_dxa));
            break;
        }
        switch (nKeyword)
        {
        case RTF_TRPADDFB:
            nSprm = NS_ooxml::LN_CT_TcMar_bottom;
            break;
        case RTF_TRPADDFL:
            nSprm = NS_ooxml::LN_CT_TcMar_left;
            break;
        case RTF_TRPADDFR:
            nSprm = NS_ooxml::LN_CT_TcMar_right;
            break;
        case RTF_TRPADDFT:
            nSprm = NS_ooxml::LN_CT_TcMar_top;
            break;
        default:
            break;
        }
        lcl_putNestedAttribute(m_aStates.top().aTableCellSprms, NS_ooxml::LN_CT_TcPrBase_tcMar, nSprm, std::make_shared<RTFValue>(aAttributes));
        lcl_putNestedAttribute(m_aDefaultState.aTableCellSprms, NS_ooxml::LN_CT_TcPrBase_tcMar, nSprm, std::make_shared<RTFValue>(aAttributes));
    }
    break;
    case RTF_TRPADDB:
    case RTF_TRPADDL:
    case RTF_TRPADDR:
    case RTF_TRPADDT:
    {
        RTFSprms aAttributes;
        aAttributes.set(NS_ooxml::LN_CT_TblWidth_w, std::make_shared<RTFValue>(nParam));
        switch (nKeyword)
        {
        case RTF_TRPADDB:
            nSprm = NS_ooxml::LN_CT_TcMar_bottom;
            break;
        case RTF_TRPADDL:
            nSprm = NS_ooxml::LN_CT_TcMar_left;
            break;
        case RTF_TRPADDR:
            nSprm = NS_ooxml::LN_CT_TcMar_right;
            break;
        case RTF_TRPADDT:
            nSprm = NS_ooxml::LN_CT_TcMar_top;
            break;
        default:
            break;
        }
        lcl_putNestedSprm(m_aStates.top().aTableCellSprms, NS_ooxml::LN_CT_TcPrBase_tcMar, nSprm, std::make_shared<RTFValue>(aAttributes));
        lcl_putNestedSprm(m_aDefaultState.aTableCellSprms, NS_ooxml::LN_CT_TcPrBase_tcMar, nSprm, std::make_shared<RTFValue>(aAttributes));
    }
    break;
    case RTF_FI:
        lcl_putNestedAttribute(m_aStates.top().aParagraphSprms, NS_ooxml::LN_CT_PPrBase_ind, NS_ooxml::LN_CT_Ind_firstLine, pIntValue);
        break;
    case RTF_LI:
    {
        lcl_putNestedAttribute(m_aStates.top().aParagraphSprms, NS_ooxml::LN_CT_PPrBase_ind, NS_ooxml::LN_CT_Ind_left, pIntValue);
        // It turns out \li should reset the \fi inherited from the stylesheet.
        // So set the direct formatting to zero, if we don't have such direct formatting yet.
        lcl_putNestedAttribute(m_aStates.top().aParagraphSprms, NS_ooxml::LN_CT_PPrBase_ind, NS_ooxml::LN_CT_Ind_firstLine, std::make_shared<RTFValue>(0),
                               RTFOverwrite::NO_IGNORE);
    }
    break;
    case RTF_RI:
        lcl_putNestedAttribute(m_aStates.top().aParagraphSprms, NS_ooxml::LN_CT_PPrBase_ind, NS_ooxml::LN_CT_Ind_right, pIntValue);
        break;
    case RTF_LIN:
        lcl_putNestedAttribute(m_aStates.top().aParagraphSprms, NS_ooxml::LN_CT_PPrBase_ind, NS_ooxml::LN_CT_Ind_start, pIntValue);
        break;
    case RTF_RIN:
        lcl_putNestedAttribute(m_aStates.top().aParagraphSprms, NS_ooxml::LN_CT_PPrBase_ind, NS_ooxml::LN_CT_Ind_end, pIntValue);
        break;
    case RTF_OUTLINELEVEL:
        m_aStates.top().aParagraphSprms.set(NS_ooxml::LN_CT_PPrBase_outlineLvl, pIntValue);
        break;
    case RTF_TRGAPH:
        // Half of the space between the cells of a table row: default left/right table cell margin.
        if (nParam > 0)
        {
            RTFSprms aAttributes;
            aAttributes.set(NS_ooxml::LN_CT_TblWidth_type, std::make_shared<RTFValue>(NS_ooxml::LN_Value_ST_TblWidth_dxa));
            aAttributes.set(NS_ooxml::LN_CT_TblWidth_w, pIntValue);
            lcl_putNestedSprm(m_aStates.top().aTableRowSprms, NS_ooxml::LN_CT_TblPrBase_tblCellMar, NS_ooxml::LN_CT_TblCellMar_left, std::make_shared<RTFValue>(aAttributes));
            lcl_putNestedSprm(m_aStates.top().aTableRowSprms, NS_ooxml::LN_CT_TblPrBase_tblCellMar, NS_ooxml::LN_CT_TblCellMar_right, std::make_shared<RTFValue>(aAttributes));
        }
        break;
    case RTF_TRFTSWIDTH:
        lcl_putNestedAttribute(m_aStates.top().aTableRowSprms, NS_ooxml::LN_CT_TblPrBase_tblW, NS_ooxml::LN_CT_TblWidth_type, pIntValue);
        break;
    case RTF_TRWWIDTH:
        lcl_putNestedAttribute(m_aStates.top().aTableRowSprms, NS_ooxml::LN_CT_TblPrBase_tblW, NS_ooxml::LN_CT_TblWidth_w, pIntValue);
        break;
    case RTF_DIBITMAP:
        m_aStates.top().aPicture.eStyle = RTFBmpStyle::DIBITMAP;
        break;
    default:
    {
        SAL_INFO("writerfilter", "TODO handle value '" << lcl_RtfToString(nKeyword) << "'");
        aSkip.setParsed(false);
    }
    break;
    }
    return RTFError::OK;
}

RTFError RTFDocumentImpl::dispatchToggle(RTFKeyword nKeyword, bool bParam, int nParam)
{
    setNeedSect();
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
        nSprm = (m_aStates.top().isRightToLeft || m_aStates.top().eRunType == RTFParserState::HICH) ? NS_ooxml::LN_EG_RPrBase_bCs : NS_ooxml::LN_EG_RPrBase_b;
        break;
    case RTF_I:
    case RTF_AI:
        nSprm = (m_aStates.top().isRightToLeft || m_aStates.top().eRunType == RTFParserState::HICH) ? NS_ooxml::LN_EG_RPrBase_iCs : NS_ooxml::LN_EG_RPrBase_i;
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
        lcl_putNestedAttribute(m_aStates.top().aCharacterSprms, NS_ooxml::LN_trackchange, NS_ooxml::LN_token, pValue);
    }
    break;
    case RTF_SBAUTO:
        lcl_putNestedAttribute(m_aStates.top().aParagraphSprms, NS_ooxml::LN_CT_PPrBase_spacing, NS_ooxml::LN_CT_Spacing_beforeAutospacing, pBoolValue);
        break;
    case RTF_SAAUTO:
        lcl_putNestedAttribute(m_aStates.top().aParagraphSprms, NS_ooxml::LN_CT_PPrBase_spacing, NS_ooxml::LN_CT_Spacing_afterAutospacing, pBoolValue);
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
        SAL_INFO("writerfilter", "TODO handle toggle '" << lcl_RtfToString(nKeyword) << "'");
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
        m_aStates.top().eRunType = RTFParserState::LOCH;

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

RTFError RTFDocumentImpl::popState()
{
    //SAL_INFO("writerfilter", OSL_THIS_FUNC << " before pop: m_pTokenizer->getGroup() " << m_pTokenizer->getGroup() <<
    //                         ", dest state: " << m_aStates.top().eDestination);

    checkUnicode(/*bUnicode =*/ true, /*bHex =*/ true);
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
            lcl_putNestedAttribute(m_aDefaultState.aCharacterSprms, NS_ooxml::LN_EG_RPrBase_rFonts, NS_ooxml::LN_CT_Fonts_ascii, pValue);
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
        for (RTFSprms::Iterator_t i = aState.aListLevelEntries.begin(); i != aState.aListLevelEntries.end(); ++i)
            aState.aTableSprms.set(i->first, i->second, RTFOverwrite::NO_APPEND);
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
            m_aStates.top().pCurrentBuffer->push_back(Buf_t(BUFFER_PROPS, pFFValue));
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
            catch(const rtl::MalformedUriException& rException)
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
            for (std::vector< std::pair<OUString, OUString> >::iterator i = aState.aShape.aGroupProperties.begin(); i != aState.aShape.aGroupProperties.end(); ++i)
                m_pSdrImport->appendGroupProperty(i->first, i->second);
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
        OUString aStr(OStringToOUString(lcl_DTTM22OString(m_aStates.top().pDestinationText->makeStringAndClear().toInt32()),
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
            // parBreak modify m_aStates.top() so we can't apply resetFrame directly on aState
            m_aStates.top().resetFrame();
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
            m_aStates.top().pCurrentBuffer->push_back(Buf_t(BUFFER_PROPS, std::make_shared<RTFValue>(RTFSprms(), aTCSprms)));
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
            lcl_putNestedSprm(m_aStates.top().aParagraphSprms, NS_ooxml::LN_CT_PPrBase_numPr, NS_ooxml::LN_CT_NumPr_ilvl, pIlvlValue);
            lcl_putNestedSprm(m_aStates.top().aParagraphSprms, NS_ooxml::LN_CT_PPrBase_tabs, NS_ooxml::LN_CT_NumPr_numId, pIdValue);
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
                        Buf_t(BUFFER_ENDSHAPE));
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
      eRunType(LOCH),
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
      nHRule(NS_ooxml::LN_Value_doc_ST_HeightRule_auto),
      nAnchorType(0)
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
        nW = nValue;
        break;
    case NS_ooxml::LN_CT_FramePr_h:
        nH = nValue;
        break;
    case NS_ooxml::LN_CT_FramePr_x:
        nX = nValue;
        break;
    case NS_ooxml::LN_CT_FramePr_y:
        nY = nValue;
        break;
    case NS_ooxml::LN_CT_FramePr_hSpace:
        nHoriPadding = nValue;
        break;
    case NS_ooxml::LN_CT_FramePr_vSpace:
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
    case NS_ooxml::LN_CT_FramePr_wrap:
        oWrap = nValue;
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
            if (nX != 0)
                pValue = std::make_shared<RTFValue>(nX);
            break;
        case NS_ooxml::LN_CT_FramePr_y:
            if (nY != 0)
                pValue = std::make_shared<RTFValue>(nY);
            break;
        case NS_ooxml::LN_CT_FramePr_h:
            if (nH != 0)
            {
                if (nHRule == NS_ooxml::LN_Value_doc_ST_HeightRule_exact)
                    pValue = std::make_shared<RTFValue>(-nH); // The negative value just sets nHRule
                else
                    pValue = std::make_shared<RTFValue>(nH);
            }
            break;
        case NS_ooxml::LN_CT_FramePr_w:
            if (nW != 0)
                pValue = std::make_shared<RTFValue>(nW);
            break;
        case NS_ooxml::LN_CT_FramePr_hSpace:
            if (nHoriPadding != 0)
                pValue = std::make_shared<RTFValue>(nHoriPadding);
            break;
        case NS_ooxml::LN_CT_FramePr_vSpace:
            if (nVertPadding != 0)
                pValue = std::make_shared<RTFValue>(nVertPadding);
            break;
        case NS_ooxml::LN_CT_FramePr_hAnchor:
        {
            if (nHoriAnchor == 0)
                nHoriAnchor = NS_ooxml::LN_Value_doc_ST_HAnchor_margin;
            pValue = std::make_shared<RTFValue>(nHoriAnchor);
        }
        break;
        case NS_ooxml::LN_CT_FramePr_vAnchor:
        {
            if (nVertAnchor == 0)
                nVertAnchor = NS_ooxml::LN_Value_doc_ST_VAnchor_margin;
            pValue = std::make_shared<RTFValue>(nVertAnchor);
        }
        break;
        case NS_ooxml::LN_CT_FramePr_xAlign:
            pValue = std::make_shared<RTFValue>(nHoriAlign);
            break;
        case NS_ooxml::LN_CT_FramePr_yAlign:
            pValue = std::make_shared<RTFValue>(nVertAlign);
            break;
        case NS_ooxml::LN_CT_FramePr_hRule:
        {
            if (nH < 0)
                nHRule = NS_ooxml::LN_Value_doc_ST_HeightRule_exact;
            else if (nH > 0)
                nHRule = NS_ooxml::LN_Value_doc_ST_HeightRule_atLeast;
            pValue = std::make_shared<RTFValue>(nHRule);
        }
        break;
        case NS_ooxml::LN_CT_FramePr_wrap:
            if (oWrap)
                pValue = std::make_shared<RTFValue>(*oWrap);
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
    return nX != 0 || nY != 0 || nW != 0 || nH != 0 ||
           nHoriPadding != 0 || nVertPadding != 0 ||
           nHoriAlign != 0 || nHoriAnchor != 0 || nVertAlign != 0 || nVertAnchor != 0 ||
           nAnchorType != 0;
}

} // namespace rtftok
} // namespace writerfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
