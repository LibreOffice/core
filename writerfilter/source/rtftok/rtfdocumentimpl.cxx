/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "rtfdocumentimpl.hxx"
#include <algorithm>
#include <memory>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/io/WrongFormatException.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <i18nlangtag/languagetag.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/streamwrap.hxx>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <filter/msfilter/util.hxx>
#include <filter/msfilter/rtfutil.hxx>
#include <comphelper/string.hxx>
#include <tools/diagnose_ex.h>
#include <tools/globname.hxx>
#include <tools/datetimeutils.hxx>
#include <comphelper/classids.hxx>
#include <comphelper/embeddedobjectcontainer.hxx>
#include <svl/lngmisc.hxx>
#include <sfx2/sfxbasemodel.hxx>
#include <sfx2/classificationhelper.hxx>
#include <oox/mathml/import.hxx>
#include <ooxml/resourceids.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/drawingml/drawingmltypes.hxx>
#include <rtl/uri.hxx>
#include <rtl/tencinfo.h>
#include <sal/log.hxx>
#include <osl/diagnose.h>
#include <oox/helper/graphichelper.hxx>
#include <vcl/wmfexternal.hxx>
#include <vcl/graph.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include "rtfsdrimport.hxx"
#include "rtfreferenceproperties.hxx"
#include "rtfskipdestination.hxx"
#include "rtftokenizer.hxx"
#include "rtflookahead.hxx"

using namespace com::sun::star;

namespace
{
/// Returns an util::DateTime from a 'YYYY. MM. DD.' string.
util::DateTime getDateTimeFromUserProp(const OUString& rString)
{
    util::DateTime aRet;
    sal_Int32 nLen = rString.getLength();
    if (nLen >= 4)
    {
        aRet.Year = rString.copy(0, 4).toInt32();

        if (nLen >= 8 && rString.match(". ", 4))
        {
            aRet.Month = rString.copy(6, 2).toInt32();

            if (nLen >= 12 && rString.match(". ", 8))
                aRet.Day = rString.copy(10, 2).toInt32();
        }
    }
    return aRet;
}
} // anonymous namespace

namespace writerfilter::rtftok
{
Id getParagraphBorder(sal_uInt32 nIndex)
{
    static const Id aBorderIds[] = { NS_ooxml::LN_CT_PBdr_top, NS_ooxml::LN_CT_PBdr_left,
                                     NS_ooxml::LN_CT_PBdr_bottom, NS_ooxml::LN_CT_PBdr_right };

    return aBorderIds[nIndex];
}

void putNestedAttribute(RTFSprms& rSprms, Id nParent, Id nId, const RTFValue::Pointer_t& pValue,
                        RTFOverwrite eOverwrite, bool bAttribute)
{
    RTFValue::Pointer_t pParent = rSprms.find(nParent, /*bFirst=*/true, /*bForWrite=*/true);
    if (!pParent)
    {
        RTFSprms aAttributes;
        if (nParent == NS_ooxml::LN_CT_TcPrBase_shd)
        {
            // RTF default is 'auto', see writerfilter::dmapper::CellColorHandler
            aAttributes.set(NS_ooxml::LN_CT_Shd_color, new RTFValue(sal_uInt32(COL_AUTO)));
            aAttributes.set(NS_ooxml::LN_CT_Shd_fill, new RTFValue(sal_uInt32(COL_AUTO)));
        }
        auto pParentValue = new RTFValue(aAttributes);
        rSprms.set(nParent, pParentValue, eOverwrite);
        pParent = pParentValue;
    }
    RTFSprms& rAttributes = (bAttribute ? pParent->getAttributes() : pParent->getSprms());
    rAttributes.set(nId, pValue, eOverwrite);
}

void putNestedSprm(RTFSprms& rSprms, Id nParent, Id nId, const RTFValue::Pointer_t& pValue,
                   RTFOverwrite eOverwrite)
{
    putNestedAttribute(rSprms, nParent, nId, pValue, eOverwrite, false);
}

RTFValue::Pointer_t getNestedAttribute(RTFSprms& rSprms, Id nParent, Id nId)
{
    RTFValue::Pointer_t pParent = rSprms.find(nParent);
    if (!pParent)
        return RTFValue::Pointer_t();
    RTFSprms& rAttributes = pParent->getAttributes();
    return rAttributes.find(nId);
}

RTFValue::Pointer_t getNestedSprm(RTFSprms& rSprms, Id nParent, Id nId)
{
    RTFValue::Pointer_t pParent = rSprms.find(nParent);
    if (!pParent)
        return RTFValue::Pointer_t();
    RTFSprms& rInner = pParent->getSprms();
    return rInner.find(nId);
}

bool eraseNestedAttribute(RTFSprms& rSprms, Id nParent, Id nId)
{
    RTFValue::Pointer_t pParent = rSprms.find(nParent);
    if (!pParent)
        // It doesn't even have a parent, we're done.
        return false;
    RTFSprms& rAttributes = pParent->getAttributes();
    return rAttributes.erase(nId);
}

RTFSprms& getLastAttributes(RTFSprms& rSprms, Id nId)
{
    RTFValue::Pointer_t p = rSprms.find(nId);
    if (p && !p->getSprms().empty())
        return p->getSprms().back().second->getAttributes();

    SAL_WARN("writerfilter.rtf", "trying to set property when no type is defined");
    return rSprms;
}

void putBorderProperty(RTFStack& aStates, Id nId, const RTFValue::Pointer_t& pValue)
{
    RTFSprms* pAttributes = nullptr;
    if (aStates.top().getBorderState() == RTFBorderState::PARAGRAPH_BOX)
        for (int i = 0; i < 4; i++)
        {
            RTFValue::Pointer_t p = aStates.top().getParagraphSprms().find(getParagraphBorder(i));
            if (p)
            {
                RTFSprms& rAttributes = p->getAttributes();
                rAttributes.set(nId, pValue);
            }
        }
    else if (aStates.top().getBorderState() == RTFBorderState::CHARACTER)
    {
        RTFValue::Pointer_t pPointer
            = aStates.top().getCharacterSprms().find(NS_ooxml::LN_EG_RPrBase_bdr);
        if (pPointer)
        {
            RTFSprms& rAttributes = pPointer->getAttributes();
            rAttributes.set(nId, pValue);
        }
    }
    // Attributes of the last border type
    else if (aStates.top().getBorderState() == RTFBorderState::PARAGRAPH)
        pAttributes
            = &getLastAttributes(aStates.top().getParagraphSprms(), NS_ooxml::LN_CT_PrBase_pBdr);
    else if (aStates.top().getBorderState() == RTFBorderState::CELL)
        pAttributes = &getLastAttributes(aStates.top().getTableCellSprms(),
                                         NS_ooxml::LN_CT_TcPrBase_tcBorders);
    else if (aStates.top().getBorderState() == RTFBorderState::PAGE)
        pAttributes = &getLastAttributes(aStates.top().getSectionSprms(),
                                         NS_ooxml::LN_EG_SectPrContents_pgBorders);
    if (pAttributes)
        pAttributes->set(nId, pValue);
}

OString DTTM22OString(long nDTTM)
{
    return DateTimeToOString(msfilter::util::DTTM2DateTime(nDTTM));
}

static RTFSprms lcl_getBookmarkProperties(int nPos, const OUString& rString)
{
    RTFSprms aAttributes;
    auto pPos = new RTFValue(nPos);
    if (!rString.isEmpty())
    {
        // If present, this should be sent first.
        auto pString = new RTFValue(rString);
        aAttributes.set(NS_ooxml::LN_CT_Bookmark_name, pString);
    }
    aAttributes.set(NS_ooxml::LN_CT_MarkupRangeBookmark_id, pPos);
    return aAttributes;
}

const char* keywordToString(RTFKeyword nKeyword)
{
    for (int i = 0; i < nRTFControlWords; i++)
    {
        if (nKeyword == aRTFControlWords[i].GetIndex())
            return aRTFControlWords[i].GetKeyword();
    }
    return nullptr;
}

static util::DateTime lcl_getDateTime(RTFParserState const& aState)
{
    return { 0 /*100sec*/,
             0 /*sec*/,
             aState.getMinute(),
             aState.getHour(),
             aState.getDay(),
             aState.getMonth(),
             static_cast<sal_Int16>(aState.getYear()),
             false };
}

static void lcl_DestinationToMath(OUStringBuffer* pDestinationText,
                                  oox::formulaimport::XmlStreamBuilder& rMathBuffer, bool& rMathNor)
{
    if (!pDestinationText)
        return;
    OUString aStr = pDestinationText->makeStringAndClear();
    if (aStr.isEmpty())
        return;
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

RTFDocumentImpl::RTFDocumentImpl(uno::Reference<uno::XComponentContext> const& xContext,
                                 uno::Reference<io::XInputStream> const& xInputStream,
                                 uno::Reference<lang::XComponent> const& xDstDoc,
                                 uno::Reference<frame::XFrame> const& xFrame,
                                 uno::Reference<task::XStatusIndicator> const& xStatusIndicator,
                                 const utl::MediaDescriptor& rMediaDescriptor)
    : m_xContext(xContext)
    , m_xInputStream(xInputStream)
    , m_xDstDoc(xDstDoc)
    , m_xFrame(xFrame)
    , m_xStatusIndicator(xStatusIndicator)
    , m_pMapperStream(nullptr)
    , m_aDefaultState(this)
    , m_bSkipUnknown(false)
    , m_bFirstRun(true)
    , m_bFirstRunException(false)
    , m_bNeedPap(true)
    , m_bNeedCr(false)
    , m_bNeedCrOrig(false)
    , m_bNeedPar(true)
    , m_bNeedFinalPar(false)
    , m_nNestedCells(0)
    , m_nTopLevelCells(0)
    , m_nInheritingCells(0)
    , m_nNestedTRLeft(0)
    , m_nTopLevelTRLeft(0)
    , m_nNestedCurrentCellX(0)
    , m_nTopLevelCurrentCellX(0)
    , m_nBackupTopLevelCurrentCellX(0)
    , m_aTableBufferStack(1) // create top-level buffer already
    , m_pSuperstream(nullptr)
    , m_nStreamType(0)
    , m_nGroupStartPos(0)
    , m_nFormFieldType(RTFFormFieldType::NONE)
    , m_bObject(false)
    , m_nCurrentFontIndex(0)
    , m_nCurrentEncoding(-1)
    , m_nDefaultFontIndex(-1)
    , m_nCurrentStyleIndex(0)
    , m_bFormField(false)
    , m_bMathNor(false)
    , m_bIgnoreNextContSectBreak(false)
    , m_nResetBreakOnSectBreak(RTF_invalid)
    , m_bNeedSect(false) // done by checkFirstRun
    , m_bWasInFrame(false)
    , m_bHadPicture(false)
    , m_bHadSect(false)
    , m_nCellxMax(0)
    , m_nListPictureId(0)
    , m_bIsNewDoc(!rMediaDescriptor.getUnpackedValueOrDefault("InsertMode", false))
    , m_rMediaDescriptor(rMediaDescriptor)
    , m_hasRHeader(false)
    , m_hasFHeader(false)
    , m_hasRFooter(false)
    , m_hasFFooter(false)
    , m_bAfterCellBeforeRow(false)
{
    OSL_ASSERT(xInputStream.is());
    m_pInStream = utl::UcbStreamHelper::CreateStream(xInputStream, true);

    m_xModelFactory.set(m_xDstDoc, uno::UNO_QUERY);

    uno::Reference<document::XDocumentPropertiesSupplier> xDocumentPropertiesSupplier(
        m_xDstDoc, uno::UNO_QUERY);
    if (xDocumentPropertiesSupplier.is())
        m_xDocumentProperties = xDocumentPropertiesSupplier->getDocumentProperties();

    m_pGraphicHelper = std::make_shared<oox::GraphicHelper>(m_xContext, xFrame, oox::StorageRef());

    m_pTokenizer = new RTFTokenizer(*this, m_pInStream.get(), m_xStatusIndicator);
    m_pSdrImport = new RTFSdrImport(*this, m_xDstDoc);
}

RTFDocumentImpl::~RTFDocumentImpl() = default;

SvStream& RTFDocumentImpl::Strm() { return *m_pInStream; }

void RTFDocumentImpl::setSuperstream(RTFDocumentImpl* pSuperstream)
{
    m_pSuperstream = pSuperstream;
}

bool RTFDocumentImpl::isSubstream() const { return m_pSuperstream != nullptr; }

void RTFDocumentImpl::finishSubstream() { checkUnicode(/*bUnicode =*/true, /*bHex =*/true); }

void RTFDocumentImpl::resolveSubstream(std::size_t nPos, Id nId)
{
    resolveSubstream(nPos, nId, OUString());
}
void RTFDocumentImpl::resolveSubstream(std::size_t nPos, Id nId, OUString const& rIgnoreFirst)
{
    sal_uInt64 const nCurrent = Strm().Tell();
    // Seek to header position, parse, then seek back.
    auto pImpl = new RTFDocumentImpl(m_xContext, m_xInputStream, m_xDstDoc, m_xFrame,
                                     m_xStatusIndicator, m_rMediaDescriptor);
    pImpl->setSuperstream(this);
    pImpl->m_nStreamType = nId;
    pImpl->m_aIgnoreFirst = rIgnoreFirst;
    if (!m_aAuthor.isEmpty())
    {
        pImpl->m_aAuthor = m_aAuthor;
        m_aAuthor.clear();
    }
    if (!m_aAuthorInitials.isEmpty())
    {
        pImpl->m_aAuthorInitials = m_aAuthorInitials;
        m_aAuthorInitials.clear();
    }
    pImpl->m_nDefaultFontIndex = m_nDefaultFontIndex;
    pImpl->Strm().Seek(nPos);
    SAL_INFO("writerfilter.rtf", "substream start");
    Mapper().substream(nId, pImpl);
    SAL_INFO("writerfilter.rtf", "substream end");
    Strm().Seek(nCurrent);
}

void RTFDocumentImpl::outputSettingsTable()
{
    writerfilter::Reference<Properties>::Pointer_t pProp
        = new RTFReferenceProperties(m_aSettingsTableAttributes, m_aSettingsTableSprms);
    RTFReferenceTable::Entries_t aSettingsTableEntries;
    aSettingsTableEntries.insert(std::make_pair(0, pProp));
    writerfilter::Reference<Table>::Pointer_t pTable = new RTFReferenceTable(aSettingsTableEntries);
    Mapper().table(NS_ooxml::LN_settings_settings, pTable);
}

void RTFDocumentImpl::checkFirstRun()
{
    if (!m_bFirstRun)
        return;

    outputSettingsTable();
    // start initial paragraph
    m_bFirstRun = false;
    assert(!m_bNeedSect || m_bFirstRunException);
    setNeedSect(true); // first call that succeeds

    // set the requested default font, if there are none
    RTFValue::Pointer_t pFont
        = getNestedAttribute(m_aDefaultState.getCharacterSprms(), NS_ooxml::LN_EG_RPrBase_rFonts,
                             NS_ooxml::LN_CT_Fonts_ascii);
    RTFValue::Pointer_t pCurrentFont
        = getNestedAttribute(m_aStates.top().getCharacterSprms(), NS_ooxml::LN_EG_RPrBase_rFonts,
                             NS_ooxml::LN_CT_Fonts_ascii);
    if (pFont && !pCurrentFont)
        putNestedAttribute(m_aStates.top().getCharacterSprms(), NS_ooxml::LN_EG_RPrBase_rFonts,
                           NS_ooxml::LN_CT_Fonts_ascii, pFont);
}

void RTFDocumentImpl::setNeedPar(bool bNeedPar) { m_bNeedPar = bNeedPar; }

void RTFDocumentImpl::setNeedSect(bool bNeedSect)
{
    if (!m_bNeedSect && bNeedSect && m_bFirstRun)
    {
        RTFLookahead aLookahead(Strm(), m_pTokenizer->getGroupStart());
        if (aLookahead.hasTable() && aLookahead.hasColumns())
        {
            m_bFirstRunException = true;
        }
    }

    // ignore setting before checkFirstRun - every keyword calls setNeedSect!
    // except the case of a table in a multicolumn section
    if (!m_bNeedSect && bNeedSect && (!m_bFirstRun || m_bFirstRunException))
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
static void lcl_copyFlatten(RTFReferenceProperties& rProps, RTFSprms& rStyleAttributes,
                            RTFSprms& rStyleSprms)
{
    for (auto& rSprm : rProps.getSprms())
    {
        // createStyleProperties() puts properties to rPr, but here we need a flat list.
        if (rSprm.first == NS_ooxml::LN_CT_Style_rPr)
        {
            // rPr can have both attributes and SPRMs, copy over both types.
            RTFSprms& rRPrSprms = rSprm.second->getSprms();
            for (const auto& rRPrSprm : rRPrSprms)
                rStyleSprms.set(rRPrSprm.first, rRPrSprm.second);

            RTFSprms& rRPrAttributes = rSprm.second->getAttributes();
            for (const auto& rRPrAttribute : rRPrAttributes)
                rStyleAttributes.set(rRPrAttribute.first, rRPrAttribute.second);
        }
        else
            rStyleSprms.set(rSprm.first, rSprm.second);
    }

    RTFSprms& rAttributes = rProps.getAttributes();
    for (const auto& rAttribute : rAttributes)
        rStyleAttributes.set(rAttribute.first, rAttribute.second);
}

writerfilter::Reference<Properties>::Pointer_t
RTFDocumentImpl::getProperties(const RTFSprms& rAttributes, RTFSprms const& rSprms, Id nStyleType)
{
    RTFSprms aSprms(rSprms);
    RTFValue::Pointer_t pAbstractList;
    int nAbstractListId = -1;
    RTFValue::Pointer_t pNumId
        = getNestedSprm(aSprms, NS_ooxml::LN_CT_PPrBase_numPr, NS_ooxml::LN_CT_NumPr_numId);
    if (pNumId)
    {
        // We have a numbering, look up the abstract list for property
        // deduplication and duplication.
        auto itNumId = m_aListOverrideTable.find(pNumId->getInt());
        if (itNumId != m_aListOverrideTable.end())
        {
            nAbstractListId = itNumId->second;
            auto itAbstract = m_aListTable.find(nAbstractListId);
            if (itAbstract != m_aListTable.end())
                pAbstractList = itAbstract->second;
        }
    }

    if (pAbstractList)
    {
        auto it = m_aInvalidListTableFirstIndents.find(nAbstractListId);
        if (it != m_aInvalidListTableFirstIndents.end())
            aSprms.deduplicateList(it->second);
    }

    int nStyle = 0;
    if (!m_aStates.empty())
        nStyle = m_aStates.top().getCurrentStyleIndex();
    auto it = m_aStyleTableEntries.find(nStyle);
    if (it != m_aStyleTableEntries.end())
    {
        // cloneAndDeduplicate() wants to know about only a single "style", so
        // let's merge paragraph and character style properties here.
        auto itChar = m_aStyleTableEntries.end();
        if (!m_aStates.empty())
        {
            int nCharStyle = m_aStates.top().getCurrentCharacterStyleIndex();
            itChar = m_aStyleTableEntries.find(nCharStyle);
        }

        RTFSprms aStyleSprms;
        RTFSprms aStyleAttributes;
        // Ensure the paragraph style is a flat list.
        // Take paragraph style into account for character properties as well,
        // as paragraph style may contain character properties.
        RTFReferenceProperties& rProps = *static_cast<RTFReferenceProperties*>(it->second.get());
        lcl_copyFlatten(rProps, aStyleAttributes, aStyleSprms);

        if (itChar != m_aStyleTableEntries.end())
        {
            // Found active character style, then update aStyleSprms/Attributes.
            if (!nStyleType || nStyleType == NS_ooxml::LN_Value_ST_StyleType_character)
            {
                RTFReferenceProperties& rCharProps
                    = *static_cast<RTFReferenceProperties*>(itChar->second.get());
                lcl_copyFlatten(rCharProps, aStyleAttributes, aStyleSprms);
            }
        }

        // Get rid of direct formatting what is already in the style.
        RTFSprms const sprms(aSprms.cloneAndDeduplicate(aStyleSprms, nStyleType, true));
        RTFSprms const attributes(
            rAttributes.cloneAndDeduplicate(aStyleAttributes, nStyleType, true));
        return new RTFReferenceProperties(attributes, sprms);
    }

    if (pAbstractList)
        aSprms.duplicateList(pAbstractList);
    writerfilter::Reference<Properties>::Pointer_t pRet
        = new RTFReferenceProperties(rAttributes, aSprms);
    return pRet;
}

void RTFDocumentImpl::checkNeedPap()
{
    if (!m_bNeedPap)
        return;

    m_bNeedPap = false; // reset early, so we can avoid recursion when calling ourselves

    if (m_aStates.empty())
        return;

    if (!m_aStates.top().getCurrentBuffer())
    {
        writerfilter::Reference<Properties>::Pointer_t const pParagraphProperties(getProperties(
            m_aStates.top().getParagraphAttributes(), m_aStates.top().getParagraphSprms(),
            NS_ooxml::LN_Value_ST_StyleType_paragraph));

        // Writer will ignore a page break before a text frame, so guard it with empty paragraphs
        bool hasBreakBeforeFrame
            = m_aStates.top().getFrame().hasProperties()
              && m_aStates.top().getParagraphSprms().find(NS_ooxml::LN_CT_PPrBase_pageBreakBefore);
        if (hasBreakBeforeFrame)
        {
            dispatchSymbol(RTF_PAR);
            m_bNeedPap = false;
        }
        Mapper().props(pParagraphProperties);
        if (hasBreakBeforeFrame)
            dispatchSymbol(RTF_PAR);

        if (m_aStates.top().getFrame().hasProperties())
        {
            writerfilter::Reference<Properties>::Pointer_t const pFrameProperties(
                new RTFReferenceProperties(RTFSprms(), m_aStates.top().getFrame().getSprms()));
            Mapper().props(pFrameProperties);
        }
    }
    else
    {
        auto pValue = new RTFValue(m_aStates.top().getParagraphAttributes(),
                                   m_aStates.top().getParagraphSprms());
        bufferProperties(*m_aStates.top().getCurrentBuffer(), pValue, nullptr);
    }
}

void RTFDocumentImpl::runProps()
{
    if (!m_aStates.top().getCurrentBuffer())
    {
        Reference<Properties>::Pointer_t const pProperties = getProperties(
            m_aStates.top().getCharacterAttributes(), m_aStates.top().getCharacterSprms(),
            NS_ooxml::LN_Value_ST_StyleType_character);
        Mapper().props(pProperties);
    }
    else
    {
        auto pValue = new RTFValue(m_aStates.top().getCharacterAttributes(),
                                   m_aStates.top().getCharacterSprms());
        bufferProperties(*m_aStates.top().getCurrentBuffer(), pValue, nullptr);
    }

    // Delete the sprm, so the trackchange range will be started only once.
    // OTOH set a boolean flag, so we'll know we need to end the range later.
    RTFValue::Pointer_t pTrackchange
        = m_aStates.top().getCharacterSprms().find(NS_ooxml::LN_trackchange);
    if (pTrackchange)
    {
        m_aStates.top().setStartedTrackchange(true);
        m_aStates.top().getCharacterSprms().erase(NS_ooxml::LN_trackchange);
    }
}

void RTFDocumentImpl::runBreak()
{
    sal_uInt8 const sBreak[] = { 0xd };
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
    SAL_INFO("writerfilter.rtf", __func__ << ": final? " << bFinal << ", needed? " << m_bNeedSect);
    bool bNeedSect = m_bNeedSect;
    RTFValue::Pointer_t pBreak
        = m_aStates.top().getSectionSprms().find(NS_ooxml::LN_EG_SectPrContents_type);
    bool bContinuous
        = pBreak
          && pBreak->getInt()
                 == static_cast<sal_Int32>(NS_ooxml::LN_Value_ST_SectionMark_continuous);
    // If there is no paragraph in this section, then insert a dummy one, as required by Writer,
    // unless this is the end of the doc, we had nothing since the last section break and this is not a continuous one.
    // Also, when pasting, it's fine to not have any paragraph inside the document at all.
    if (m_bNeedPar && (!bFinal || m_bNeedSect || bContinuous) && !isSubstream() && m_bIsNewDoc)
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
        std::pair<Id, std::size_t> aPair = m_nHeaderFooterPositions.front();
        m_nHeaderFooterPositions.pop();
        resolveSubstream(aPair.second, aPair.first);
    }

    // Normally a section break at the end of the doc is necessary. Unless the
    // last control word in the document is a section break itself.
    if (!bNeedSect || !m_bHadSect)
    {
        // In case the last section is a continuous one, we don't need to output a section break.
        if (bFinal && bContinuous)
            m_aStates.top().getSectionSprms().erase(NS_ooxml::LN_EG_SectPrContents_type);
    }

    // Section properties are a paragraph sprm.
    auto pValue
        = new RTFValue(m_aStates.top().getSectionAttributes(), m_aStates.top().getSectionSprms());
    RTFSprms aAttributes;
    RTFSprms aSprms;
    aSprms.set(NS_ooxml::LN_CT_PPr_sectPr, pValue);
    writerfilter::Reference<Properties>::Pointer_t pProperties
        = new RTFReferenceProperties(aAttributes, aSprms);

    if (bFinal && !m_pSuperstream)
        // This is the end of the document, not just the end of e.g. a header.
        // This makes sure that dmapper can set DontBalanceTextColumns=true for this section if necessary.
        Mapper().markLastSectionGroup();

    // The trick is that we send properties of the previous section right now, which will be exactly what dmapper expects.
    Mapper().props(pProperties);
    Mapper().endParagraphGroup();

    // End Section
    if (!m_pSuperstream)
    {
        m_hasFHeader = false;
        m_hasRHeader = false;
        m_hasRFooter = false;
        m_hasFFooter = false;
        Mapper().endSectionGroup();
    }
    m_bNeedPar = false;
    m_bNeedSect = false;
}

Color RTFDocumentImpl::getColorTable(sal_uInt32 nIndex)
{
    if (!m_pSuperstream)
    {
        if (nIndex < m_aColorTable.size())
            return m_aColorTable[nIndex];
        return 0;
    }

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
        if (m_aDefaultState.getCurrentEncoding() != rtl_getTextEncodingFromWindowsCharset(0))
            // We have a default encoding.
            return m_aDefaultState.getCurrentEncoding();
        // Guess based on locale.
        return msfilter::util::getBestTextEncodingFromLocale(
            Application::GetSettings().GetLanguageTag().getLocale());
    }

    return m_pSuperstream->getEncoding(nFontIndex);
}

OUString RTFDocumentImpl::getFontName(int nIndex)
{
    if (!m_pSuperstream)
        return m_aFontNames[nIndex];

    return m_pSuperstream->getFontName(nIndex);
}

int RTFDocumentImpl::getFontIndex(int nIndex)
{
    if (!m_pSuperstream)
        return std::find(m_aFontIndexes.begin(), m_aFontIndexes.end(), nIndex)
               - m_aFontIndexes.begin();

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

    return m_pSuperstream->getStyleName(nIndex);
}

Id RTFDocumentImpl::getStyleType(int nIndex)
{
    if (!m_pSuperstream)
    {
        Id nRet = 0;
        if (m_aStyleTypes.find(nIndex) != m_aStyleTypes.end())
            nRet = m_aStyleTypes[nIndex];
        return nRet;
    }

    return m_pSuperstream->getStyleType(nIndex);
}

RTFParserState& RTFDocumentImpl::getDefaultState()
{
    if (!m_pSuperstream)
        return m_aDefaultState;

    return m_pSuperstream->getDefaultState();
}

oox::GraphicHelper& RTFDocumentImpl::getGraphicHelper() { return *m_pGraphicHelper; }

bool RTFDocumentImpl::isStyleSheetImport()
{
    if (m_aStates.empty())
        return false;
    Destination eDestination = m_aStates.top().getDestination();
    return eDestination == Destination::STYLESHEET || eDestination == Destination::STYLEENTRY;
}

void RTFDocumentImpl::resolve(Stream& rMapper)
{
    m_pMapperStream = &rMapper;
    switch (m_pTokenizer->resolveParse())
    {
        case RTFError::OK:
            SAL_INFO("writerfilter.rtf", "RTFDocumentImpl::resolve: finished without errors");
            break;
        case RTFError::GROUP_UNDER:
            SAL_INFO("writerfilter.rtf", "RTFDocumentImpl::resolve: unmatched '}'");
            break;
        case RTFError::GROUP_OVER:
            SAL_INFO("writerfilter.rtf", "RTFDocumentImpl::resolve: unmatched '{'");
            throw io::WrongFormatException(m_pTokenizer->getPosition());
            break;
        case RTFError::UNEXPECTED_EOF:
            SAL_INFO("writerfilter.rtf", "RTFDocumentImpl::resolve: unexpected end of file");
            throw io::WrongFormatException(m_pTokenizer->getPosition());
            break;
        case RTFError::HEX_INVALID:
            SAL_INFO("writerfilter.rtf", "RTFDocumentImpl::resolve: invalid hex char");
            throw io::WrongFormatException(m_pTokenizer->getPosition());
            break;
        case RTFError::CHAR_OVER:
            SAL_INFO("writerfilter.rtf", "RTFDocumentImpl::resolve: characters after last '}'");
            break;
        case RTFError::CLASSIFICATION:
            SAL_INFO("writerfilter.rtf",
                     "RTFDocumentImpl::resolve: classification prevented paste");
            break;
    }
}

void RTFDocumentImpl::resolvePict(bool const bInline, uno::Reference<drawing::XShape> const& rShape)
{
    SvMemoryStream aStream;
    SvStream* pStream = nullptr;
    if (!m_pBinaryData)
    {
        pStream = &aStream;
        int b = 0;
        int count = 2;

        // Feed the destination text to a stream.
        OString aStr = OUStringToOString(m_aStates.top().getDestinationText().makeStringAndClear(),
                                         RTL_TEXTENCODING_ASCII_US);
        for (int i = 0; i < aStr.getLength(); ++i)
        {
            char ch = aStr[i];
            if (ch != 0x0d && ch != 0x0a && ch != 0x20)
            {
                b = b << 4;
                sal_Int8 parsed = msfilter::rtfutil::AsHex(ch);
                if (parsed == -1)
                    return;
                b += parsed;
                count--;
                if (!count)
                {
                    aStream.WriteChar(static_cast<char>(b));
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
    if (m_aStates.top().getPicture().eStyle == RTFBmpStyle::DIBITMAP)
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
    WmfExternal aExtHeader;
    aExtHeader.mapMode = m_aStates.top().getPicture().eWMetafile;
    aExtHeader.xExt = sal_uInt16(
        std::clamp<sal_Int32>(m_aStates.top().getPicture().nWidth, 0,
                              SAL_MAX_UINT16)); //TODO: better way to handle out-of-bounds values?
    aExtHeader.yExt = sal_uInt16(
        std::clamp<sal_Int32>(m_aStates.top().getPicture().nHeight, 0,
                              SAL_MAX_UINT16)); //TODO: better way to handle out-of-bounds values?
    WmfExternal* pExtHeader = &aExtHeader;
    uno::Reference<lang::XServiceInfo> xServiceInfo(m_aStates.top().getDrawingObject().getShape(),
                                                    uno::UNO_QUERY);
    if (xServiceInfo.is() && xServiceInfo->supportsService("com.sun.star.text.TextFrame"))
        pExtHeader = nullptr;

    uno::Reference<graphic::XGraphic> xGraphic
        = m_pGraphicHelper->importGraphic(xInputStream, pExtHeader);

    if (m_aStates.top().getPicture().eStyle != RTFBmpStyle::NONE)
    {
        // In case of PNG/JPEG, the real size is known, don't use the values
        // provided by picw and pich.

        Graphic aGraphic(xGraphic);
        Size aSize(aGraphic.GetPrefSize());
        MapMode aMap(MapUnit::Map100thMM);
        if (aGraphic.GetPrefMapMode().GetMapUnit() == MapUnit::MapPixel)
            aSize = Application::GetDefaultDevice()->PixelToLogic(aSize, aMap);
        else
            aSize = OutputDevice::LogicToLogic(aSize, aGraphic.GetPrefMapMode(), aMap);
        m_aStates.top().getPicture().nWidth = aSize.Width();
        m_aStates.top().getPicture().nHeight = aSize.Height();
    }

    // Wrap it in an XShape.
    uno::Reference<drawing::XShape> xShape(rShape);
    if (xShape.is())
    {
        uno::Reference<lang::XServiceInfo> xSI(xShape, uno::UNO_QUERY_THROW);
        if (!xSI->supportsService("com.sun.star.drawing.GraphicObjectShape"))
        {
            // it's sometimes an error to get here - but it's possible to have
            // a \pict inside the \shptxt of a \shp of shapeType 202 "TextBox"
            // and in that case xShape is the text frame; we actually need a
            // new GraphicObject then (example: fdo37691-1.rtf)
            SAL_INFO("writerfilter.rtf",
                     "cannot set graphic on existing shape, creating a new GraphicObjectShape");
            xShape.clear();
        }
    }
    if (!xShape.is())
    {
        if (m_xModelFactory.is())
            xShape.set(m_xModelFactory->createInstance("com.sun.star.drawing.GraphicObjectShape"),
                       uno::UNO_QUERY);
        uno::Reference<drawing::XDrawPageSupplier> const xDrawSupplier(m_xDstDoc, uno::UNO_QUERY);
        if (xDrawSupplier.is())
        {
            uno::Reference<drawing::XShapes> xShapes = xDrawSupplier->getDrawPage();
            if (xShapes.is())
                xShapes->add(xShape);
        }
    }

    uno::Reference<beans::XPropertySet> xPropertySet(xShape, uno::UNO_QUERY);

    if (xPropertySet.is())
        xPropertySet->setPropertyValue("Graphic", uno::Any(xGraphic));

    // check if the picture is in an OLE object and if the \objdata element is used
    // (see RTF_OBJECT in RTFDocumentImpl::dispatchDestination)
    if (m_bObject)
    {
        // Set the object size
        awt::Size aSize;
        aSize.Width
            = (m_aStates.top().getPicture().nGoalWidth ? m_aStates.top().getPicture().nGoalWidth
                                                       : m_aStates.top().getPicture().nWidth);
        aSize.Height
            = (m_aStates.top().getPicture().nGoalHeight ? m_aStates.top().getPicture().nGoalHeight
                                                        : m_aStates.top().getPicture().nHeight);
        xShape->setSize(aSize);

        // Replacement graphic is inline by default, see oox::vml::SimpleShape::implConvertAndInsert().
        xPropertySet->setPropertyValue("AnchorType",
                                       uno::makeAny(text::TextContentAnchorType_AS_CHARACTER));

        auto pShapeValue = new RTFValue(xShape);
        m_aObjectAttributes.set(NS_ooxml::LN_shape, pShapeValue);
        return;
    }

    if (m_aStates.top().getInListpicture())
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
    auto pShapeValue = new RTFValue(xShape);
    aPicAttributes.set(NS_ooxml::LN_shape, pShapeValue);
    // pic sprm
    RTFSprms aGraphicDataAttributes;
    RTFSprms aGraphicDataSprms;
    auto pPicValue = new RTFValue(aPicAttributes);
    aGraphicDataSprms.set(NS_ooxml::LN_pic_pic, pPicValue);
    // graphicData sprm
    RTFSprms aGraphicAttributes;
    RTFSprms aGraphicSprms;
    auto pGraphicDataValue = new RTFValue(aGraphicDataAttributes, aGraphicDataSprms);
    aGraphicSprms.set(NS_ooxml::LN_CT_GraphicalObject_graphicData, pGraphicDataValue);
    // graphic sprm
    auto pGraphicValue = new RTFValue(aGraphicAttributes, aGraphicSprms);
    // extent sprm
    RTFSprms aExtentAttributes;
    int nXExt = (m_aStates.top().getPicture().nGoalWidth ? m_aStates.top().getPicture().nGoalWidth
                                                         : m_aStates.top().getPicture().nWidth);
    int nYExt = (m_aStates.top().getPicture().nGoalHeight ? m_aStates.top().getPicture().nGoalHeight
                                                          : m_aStates.top().getPicture().nHeight);
    if (m_aStates.top().getPicture().nScaleX != 100)
        nXExt = (static_cast<long>(m_aStates.top().getPicture().nScaleX)
                 * (nXExt
                    - (m_aStates.top().getPicture().nCropL + m_aStates.top().getPicture().nCropR)))
                / 100L;
    if (m_aStates.top().getPicture().nScaleY != 100)
        nYExt = (static_cast<long>(m_aStates.top().getPicture().nScaleY)
                 * (nYExt
                    - (m_aStates.top().getPicture().nCropT + m_aStates.top().getPicture().nCropB)))
                / 100L;
    if (m_aStates.top().getInShape())
    {
        // Picture in shape: it looks like pib picture, so we will stretch the picture to shape size (tdf#49893)
        nXExt = m_aStates.top().getShape().getRight() - m_aStates.top().getShape().getLeft();
        nYExt = m_aStates.top().getShape().getBottom() - m_aStates.top().getShape().getTop();
    }
    auto pXExtValue = new RTFValue(oox::drawingml::convertHmmToEmu(nXExt));
    auto pYExtValue = new RTFValue(oox::drawingml::convertHmmToEmu(nYExt));
    aExtentAttributes.set(NS_ooxml::LN_CT_PositiveSize2D_cx, pXExtValue);
    aExtentAttributes.set(NS_ooxml::LN_CT_PositiveSize2D_cy, pYExtValue);
    auto pExtentValue = new RTFValue(aExtentAttributes);
    // docpr sprm
    RTFSprms aDocprAttributes;
    for (const auto& rCharacterAttribute : m_aStates.top().getCharacterAttributes())
        if (rCharacterAttribute.first == NS_ooxml::LN_CT_NonVisualDrawingProps_name
            || rCharacterAttribute.first == NS_ooxml::LN_CT_NonVisualDrawingProps_descr)
            aDocprAttributes.set(rCharacterAttribute.first, rCharacterAttribute.second);
    auto pDocprValue = new RTFValue(aDocprAttributes);
    if (bInline)
    {
        RTFSprms aInlineAttributes;
        aInlineAttributes.set(NS_ooxml::LN_CT_Inline_distT, new RTFValue(0));
        aInlineAttributes.set(NS_ooxml::LN_CT_Inline_distB, new RTFValue(0));
        aInlineAttributes.set(NS_ooxml::LN_CT_Inline_distL, new RTFValue(0));
        aInlineAttributes.set(NS_ooxml::LN_CT_Inline_distR, new RTFValue(0));
        RTFSprms aInlineSprms;
        aInlineSprms.set(NS_ooxml::LN_CT_Inline_extent, pExtentValue);
        aInlineSprms.set(NS_ooxml::LN_CT_Inline_docPr, pDocprValue);
        aInlineSprms.set(NS_ooxml::LN_graphic_graphic, pGraphicValue);
        // inline sprm
        auto pValue = new RTFValue(aInlineAttributes, aInlineSprms);
        aSprms.set(NS_ooxml::LN_inline_inline, pValue);
    }
    else // anchored
    {
        // wrap sprm
        RTFSprms aAnchorWrapAttributes;
        m_aStates.top().getShape().getAnchorAttributes().set(
            NS_ooxml::LN_CT_Anchor_behindDoc,
            new RTFValue((m_aStates.top().getShape().getInBackground()) ? 1 : 0));
        RTFSprms aAnchorSprms;
        for (const auto& rCharacterAttribute : m_aStates.top().getCharacterAttributes())
        {
            if (rCharacterAttribute.first == NS_ooxml::LN_CT_WrapSquare_wrapText)
                aAnchorWrapAttributes.set(rCharacterAttribute.first, rCharacterAttribute.second);
        }
        sal_Int32 nWrap = -1;
        for (auto& rCharacterSprm : m_aStates.top().getCharacterSprms())
        {
            if (rCharacterSprm.first == NS_ooxml::LN_EG_WrapType_wrapNone
                || rCharacterSprm.first == NS_ooxml::LN_EG_WrapType_wrapTight)
            {
                nWrap = rCharacterSprm.first;

                // If there is a wrap polygon prepared by RTFSdrImport, pick it up here.
                if (rCharacterSprm.first == NS_ooxml::LN_EG_WrapType_wrapTight
                    && !m_aStates.top().getShape().getWrapPolygonSprms().empty())
                    rCharacterSprm.second->getSprms().set(
                        NS_ooxml::LN_CT_WrapTight_wrapPolygon,
                        new RTFValue(RTFSprms(), m_aStates.top().getShape().getWrapPolygonSprms()));

                aAnchorSprms.set(rCharacterSprm.first, rCharacterSprm.second);
            }
        }

        if (m_aStates.top().getShape().getWrapSprm().first != 0)
            // Replay of a buffered shape, wrap sprm there has priority over
            // character sprms of the current state.
            aAnchorSprms.set(m_aStates.top().getShape().getWrapSprm().first,
                             m_aStates.top().getShape().getWrapSprm().second);

        aAnchorSprms.set(NS_ooxml::LN_CT_Anchor_extent, pExtentValue);
        if (!aAnchorWrapAttributes.empty() && nWrap == -1)
            aAnchorSprms.set(NS_ooxml::LN_EG_WrapType_wrapSquare,
                             new RTFValue(aAnchorWrapAttributes));

        // See OOXMLFastContextHandler::positionOffset(), we can't just put offset values in an RTFValue.
        RTFSprms aPoshAttributes;
        RTFSprms aPoshSprms;
        if (m_aStates.top().getShape().getHoriOrientRelationToken() > 0)
            aPoshAttributes.set(
                NS_ooxml::LN_CT_PosH_relativeFrom,
                new RTFValue(m_aStates.top().getShape().getHoriOrientRelationToken()));
        if (m_aStates.top().getShape().getLeft() != 0)
        {
            Mapper().positionOffset(OUString::number(oox::drawingml::convertHmmToEmu(
                                        m_aStates.top().getShape().getLeft())),
                                    /*bVertical=*/false);
            aPoshSprms.set(NS_ooxml::LN_CT_PosH_posOffset, new RTFValue());
        }
        aAnchorSprms.set(NS_ooxml::LN_CT_Anchor_positionH,
                         new RTFValue(aPoshAttributes, aPoshSprms));

        RTFSprms aPosvAttributes;
        RTFSprms aPosvSprms;
        if (m_aStates.top().getShape().getVertOrientRelationToken() > 0)
            aPosvAttributes.set(
                NS_ooxml::LN_CT_PosV_relativeFrom,
                new RTFValue(m_aStates.top().getShape().getVertOrientRelationToken()));
        if (m_aStates.top().getShape().getTop() != 0)
        {
            Mapper().positionOffset(OUString::number(oox::drawingml::convertHmmToEmu(
                                        m_aStates.top().getShape().getTop())),
                                    /*bVertical=*/true);
            aPosvSprms.set(NS_ooxml::LN_CT_PosV_posOffset, new RTFValue());
        }
        aAnchorSprms.set(NS_ooxml::LN_CT_Anchor_positionV,
                         new RTFValue(aPosvAttributes, aPosvSprms));

        aAnchorSprms.set(NS_ooxml::LN_CT_Anchor_docPr, pDocprValue);
        aAnchorSprms.set(NS_ooxml::LN_graphic_graphic, pGraphicValue);
        // anchor sprm
        auto pValue = new RTFValue(m_aStates.top().getShape().getAnchorAttributes(), aAnchorSprms);
        aSprms.set(NS_ooxml::LN_anchor_anchor, pValue);
    }
    writerfilter::Reference<Properties>::Pointer_t pProperties
        = new RTFReferenceProperties(aAttributes, aSprms);
    checkFirstRun();

    if (!m_aStates.top().getCurrentBuffer())
    {
        Mapper().props(pProperties);
        // Make sure we don't lose these properties with a too early reset.
        m_bHadPicture = true;
    }
    else
    {
        auto pValue = new RTFValue(aAttributes, aSprms);
        bufferProperties(*m_aStates.top().getCurrentBuffer(), pValue, nullptr);
    }
}

RTFError RTFDocumentImpl::resolveChars(char ch)
{
    if (m_aStates.top().getInternalState() == RTFInternalState::BIN)
    {
        m_pBinaryData = std::make_shared<SvMemoryStream>();
        m_pBinaryData->WriteChar(ch);
        for (int i = 0; i < m_aStates.top().getBinaryToRead() - 1; ++i)
        {
            Strm().ReadChar(ch);
            m_pBinaryData->WriteChar(ch);
        }
        m_aStates.top().setInternalState(RTFInternalState::NORMAL);
        return RTFError::OK;
    }

    OStringBuffer aBuf(512);

    bool bUnicodeChecked = false;
    bool bSkipped = false;

    while (!Strm().eof()
           && (m_aStates.top().getInternalState() == RTFInternalState::HEX
               || (ch != '{' && ch != '}' && ch != '\\')))
    {
        if (m_aStates.top().getInternalState() == RTFInternalState::HEX
            || (ch != 0x0d && ch != 0x0a))
        {
            if (m_aStates.top().getCharsToSkip() == 0)
            {
                if (!bUnicodeChecked)
                {
                    checkUnicode(/*bUnicode =*/true, /*bHex =*/false);
                    bUnicodeChecked = true;
                }
                aBuf.append(ch);
            }
            else
            {
                bSkipped = true;
                m_aStates.top().getCharsToSkip()--;
            }
        }

        // read a single char if we're in hex mode
        if (m_aStates.top().getInternalState() == RTFInternalState::HEX)
            break;

        if (RTL_TEXTENCODING_MS_932 == m_aStates.top().getCurrentEncoding())
        {
            unsigned char uch = ch;
            if ((uch >= 0x80 && uch <= 0x9F) || uch >= 0xE0)
            {
                // read second byte of 2-byte Shift-JIS - may be \ { }
                Strm().ReadChar(ch);
                if (m_aStates.top().getCharsToSkip() == 0)
                {
                    // fdo#79384: Word will reject Shift-JIS following \loch
                    // but apparently OOo could read and (worse) write such documents
                    SAL_INFO_IF(m_aStates.top().getRunType() != RTFParserState::RunType::DBCH,
                                "writerfilter.rtf", "invalid Shift-JIS without DBCH");
                    assert(bUnicodeChecked);
                    aBuf.append(ch);
                }
                else
                {
                    assert(bSkipped);
                    // anybody who uses \ucN with Shift-JIS is insane
                    m_aStates.top().getCharsToSkip()--;
                }
            }
        }

        Strm().ReadChar(ch);
    }
    if (m_aStates.top().getInternalState() != RTFInternalState::HEX && !Strm().eof())
        Strm().SeekRel(-1);

    if (m_aStates.top().getInternalState() == RTFInternalState::HEX
        && m_aStates.top().getDestination() != Destination::LEVELNUMBERS)
    {
        if (!bSkipped)
        {
            // note: apparently \'0d\'0a is interpreted as 2 breaks, not 1
            if ((ch == '\r' || ch == '\n')
                && m_aStates.top().getDestination() != Destination::DOCCOMM
                && m_aStates.top().getDestination() != Destination::LEVELNUMBERS
                && m_aStates.top().getDestination() != Destination::LEVELTEXT)
            {
                checkUnicode(/*bUnicode =*/false, /*bHex =*/true);
                dispatchSymbol(RTF_PAR);
            }
            else
            {
                m_aHexBuffer.append(ch);
            }
        }
        return RTFError::OK;
    }

    if (m_aStates.top().getDestination() == Destination::SKIP)
        return RTFError::OK;
    OString aStr = aBuf.makeStringAndClear();
    if (m_aStates.top().getDestination() == Destination::LEVELNUMBERS)
    {
        if (aStr.toChar() != ';')
            m_aStates.top().getLevelNumbers().push_back(sal_Int32(ch));
        return RTFError::OK;
    }

    SAL_INFO("writerfilter.rtf",
             "RTFDocumentImpl::resolveChars: collected '"
                 << OStringToOUString(aStr, m_aStates.top().getCurrentEncoding()) << "'");

    if (m_aStates.top().getDestination() == Destination::COLORTABLE)
    {
        // we hit a ';' at the end of each color entry
        m_aColorTable.push_back(m_aStates.top().getCurrentColor().GetColor());
        // set components back to zero
        m_aStates.top().getCurrentColor() = RTFColorTableEntry();
    }
    else if (!aStr.isEmpty())
        m_aHexBuffer.append(aStr);

    checkUnicode(/*bUnicode =*/false, /*bHex =*/true);
    return RTFError::OK;
}

bool RTFFrame::inFrame() const { return m_nW > 0 || m_nH > 0 || m_nX > 0 || m_nY > 0; }

void RTFDocumentImpl::singleChar(sal_uInt8 nValue, bool bRunProps)
{
    sal_uInt8 sValue[] = { nValue };
    RTFBuffer_t* pCurrentBuffer = m_aStates.top().getCurrentBuffer();

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
        auto pValue = new RTFValue(*sValue);
        pCurrentBuffer->push_back(Buf_t(BUFFER_TEXT, pValue, nullptr));
        pCurrentBuffer->push_back(Buf_t(BUFFER_ENDRUN, nullptr, nullptr));
    }
}

void RTFDocumentImpl::text(OUString& rString)
{
    if (rString.getLength() == 1 && m_aStates.top().getDestination() != Destination::DOCCOMM)
    {
        // No cheating! Tokenizer ignores bare \r and \n, their hex \'0d / \'0a form doesn't count, either.
        sal_Unicode ch = rString[0];
        if (ch == 0x0d || ch == 0x0a)
            return;
    }

    bool bRet = true;
    switch (m_aStates.top().getDestination())
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
                OUString const aName
                    = m_aStates.top().getCurrentDestinationText()->makeStringAndClear();
                switch (m_aStates.top().getDestination())
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
                        m_aStates.top().getTableAttributes().set(NS_ooxml::LN_CT_Font_name,
                                                                 new RTFValue(aName));

                        writerfilter::Reference<Properties>::Pointer_t const pProp(
                            new RTFReferenceProperties(m_aStates.top().getTableAttributes(),
                                                       m_aStates.top().getTableSprms()));

                        //See fdo#47347 initial invalid font entry properties are inserted first,
                        //so when we attempt to insert the correct ones, there's already an
                        //entry in the map for them, so the new ones aren't inserted.
                        auto lb = m_aFontTableEntries.lower_bound(m_nCurrentFontIndex);
                        if (lb != m_aFontTableEntries.end()
                            && !(m_aFontTableEntries.key_comp()(m_nCurrentFontIndex, lb->first)))
                            lb->second = pProp;
                        else
                            m_aFontTableEntries.insert(lb,
                                                       std::make_pair(m_nCurrentFontIndex, pProp));
                    }
                    break;
                    case Destination::STYLEENTRY:
                    {
                        RTFValue::Pointer_t pType
                            = m_aStates.top().getTableAttributes().find(NS_ooxml::LN_CT_Style_type);
                        if (pType)
                        {
                            // Word strips whitespace around style names.
                            m_aStyleNames[m_nCurrentStyleIndex] = aName.trim();
                            m_aStyleTypes[m_nCurrentStyleIndex] = pType->getInt();
                            auto pValue = new RTFValue(aName.trim());
                            m_aStates.top().getTableAttributes().set(NS_ooxml::LN_CT_Style_styleId,
                                                                     pValue);
                            m_aStates.top().getTableSprms().set(NS_ooxml::LN_CT_Style_name, pValue);

                            writerfilter::Reference<Properties>::Pointer_t const pProp(
                                createStyleProperties());
                            m_aStyleTableEntries.insert(
                                std::make_pair(m_nCurrentStyleIndex, pProp));
                        }
                        else
                            SAL_INFO("writerfilter.rtf", "no RTF style type defined, ignoring");
                        break;
                    }
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
            m_aStates.top().appendDestinationText(rString);
            break;
        case Destination::GENERATOR:
            // don't enlarge space sequences, eg. it was saved in LibreOffice
            if (!rString.startsWithIgnoreAsciiCase("Microsoft"))
                m_aSettingsTableSprms.set(NS_ooxml::LN_CT_Settings_longerSpaceSequence,
                                          new RTFValue(0));
            break;
        default:
            bRet = false;
            break;
    }
    if (bRet)
        return;

    if (!m_aIgnoreFirst.isEmpty() && m_aIgnoreFirst == rString)
    {
        m_aIgnoreFirst.clear();
        return;
    }

    // Are we in the middle of the table definition? (No cell defs yet, but we already have some cell props.)
    if (m_aStates.top().getTableCellSprms().find(NS_ooxml::LN_CT_TcPrBase_vAlign)
        && m_nTopLevelCells == 0)
    {
        m_aTableBufferStack.back().emplace_back(
            Buf_t(BUFFER_UTEXT, new RTFValue(rString), nullptr));
        return;
    }

    checkFirstRun();
    checkNeedPap();

    // Don't return earlier, a bookmark start has to be in a paragraph group.
    if (m_aStates.top().getDestination() == Destination::BOOKMARKSTART)
    {
        m_aStates.top().appendDestinationText(rString);
        return;
    }

    RTFBuffer_t* pCurrentBuffer = m_aStates.top().getCurrentBuffer();

    if (!pCurrentBuffer && m_aStates.top().getDestination() != Destination::FOOTNOTE)
        Mapper().startCharacterGroup();
    else if (pCurrentBuffer)
    {
        RTFValue::Pointer_t pValue;
        pCurrentBuffer->push_back(Buf_t(BUFFER_STARTRUN, pValue, nullptr));
    }

    if (m_aStates.top().getDestination() == Destination::NORMAL
        || m_aStates.top().getDestination() == Destination::FIELDRESULT
        || m_aStates.top().getDestination() == Destination::SHAPETEXT)
        runProps();

    if (!pCurrentBuffer)
        Mapper().utext(reinterpret_cast<sal_uInt8 const*>(rString.getStr()), rString.getLength());
    else
    {
        auto pValue = new RTFValue(rString);
        pCurrentBuffer->push_back(Buf_t(BUFFER_UTEXT, pValue, nullptr));
    }

    m_bNeedCr = true;

    if (!pCurrentBuffer && m_aStates.top().getDestination() != Destination::FOOTNOTE)
        Mapper().endCharacterGroup();
    else if (pCurrentBuffer)
    {
        RTFValue::Pointer_t pValue;
        pCurrentBuffer->push_back(Buf_t(BUFFER_ENDRUN, pValue, nullptr));
    }
}

void RTFDocumentImpl::prepareProperties(
    RTFParserState& rState, writerfilter::Reference<Properties>::Pointer_t& o_rpParagraphProperties,
    writerfilter::Reference<Properties>::Pointer_t& o_rpFrameProperties,
    writerfilter::Reference<Properties>::Pointer_t& o_rpTableRowProperties, int const nCells,
    int const nCurrentCellX)
{
    o_rpParagraphProperties
        = getProperties(rState.getParagraphAttributes(), rState.getParagraphSprms(),
                        NS_ooxml::LN_Value_ST_StyleType_paragraph);

    if (rState.getFrame().hasProperties())
    {
        o_rpFrameProperties = new RTFReferenceProperties(RTFSprms(), rState.getFrame().getSprms());
    }

    // Table width.
    RTFValue::Pointer_t const pTableWidthProps
        = rState.getTableRowSprms().find(NS_ooxml::LN_CT_TblPrBase_tblW);
    if (!pTableWidthProps)
    {
        auto pUnitValue = new RTFValue(3);
        putNestedAttribute(rState.getTableRowSprms(), NS_ooxml::LN_CT_TblPrBase_tblW,
                           NS_ooxml::LN_CT_TblWidth_type, pUnitValue);
        auto pWValue = new RTFValue(nCurrentCellX);
        putNestedAttribute(rState.getTableRowSprms(), NS_ooxml::LN_CT_TblPrBase_tblW,
                           NS_ooxml::LN_CT_TblWidth_w, pWValue);
    }

    if (nCells > 0)
        rState.getTableRowSprms().set(NS_ooxml::LN_tblRow, new RTFValue(1));

    RTFValue::Pointer_t const pCellMar
        = rState.getTableRowSprms().find(NS_ooxml::LN_CT_TblPrBase_tblCellMar);
    if (!pCellMar)
    {
        // If no cell margins are defined, the default left/right margin is 0 in Word, but not in Writer.
        RTFSprms aAttributes;
        aAttributes.set(NS_ooxml::LN_CT_TblWidth_type,
                        new RTFValue(NS_ooxml::LN_Value_ST_TblWidth_dxa));
        aAttributes.set(NS_ooxml::LN_CT_TblWidth_w, new RTFValue(0));
        putNestedSprm(rState.getTableRowSprms(), NS_ooxml::LN_CT_TblPrBase_tblCellMar,
                      NS_ooxml::LN_CT_TblCellMar_left, new RTFValue(aAttributes));
        putNestedSprm(rState.getTableRowSprms(), NS_ooxml::LN_CT_TblPrBase_tblCellMar,
                      NS_ooxml::LN_CT_TblCellMar_right, new RTFValue(aAttributes));
    }

    o_rpTableRowProperties
        = new RTFReferenceProperties(rState.getTableRowAttributes(), rState.getTableRowSprms());
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

void RTFDocumentImpl::replayRowBuffer(RTFBuffer_t& rBuffer, ::std::deque<RTFSprms>& rCellsSrpms,
                                      ::std::deque<RTFSprms>& rCellsAttributes, int const nCells)
{
    for (int i = 0; i < nCells; ++i)
    {
        replayBuffer(rBuffer, &rCellsSrpms.front(), &rCellsAttributes.front());
        rCellsSrpms.pop_front();
        rCellsAttributes.pop_front();
    }
    for (Buf_t& i : rBuffer)
    {
        SAL_WARN_IF(BUFFER_CELLEND == std::get<0>(i), "writerfilter.rtf", "dropping table cell!");
    }
    assert(rCellsSrpms.empty());
    assert(rCellsAttributes.empty());
}

void RTFDocumentImpl::replayBuffer(RTFBuffer_t& rBuffer, RTFSprms* const pSprms,
                                   RTFSprms const* const pAttributes)
{
    while (!rBuffer.empty())
    {
        Buf_t aTuple(rBuffer.front());
        rBuffer.pop_front();
        if (std::get<0>(aTuple) == BUFFER_PROPS)
        {
            // Construct properties via getProperties() and not directly, to take care of deduplication.
            writerfilter::Reference<Properties>::Pointer_t const pProp(getProperties(
                std::get<1>(aTuple)->getAttributes(), std::get<1>(aTuple)->getSprms(), 0));
            Mapper().props(pProp);
        }
        else if (std::get<0>(aTuple) == BUFFER_NESTROW)
        {
            TableRowBuffer& rRowBuffer(*std::get<2>(aTuple));

            replayRowBuffer(rRowBuffer.GetBuffer(), rRowBuffer.GetCellsSprms(),
                            rRowBuffer.GetCellsAttributes(), rRowBuffer.GetCells());

            sendProperties(rRowBuffer.GetParaProperties(), rRowBuffer.GetFrameProperties(),
                           rRowBuffer.GetRowProperties());
        }
        else if (std::get<0>(aTuple) == BUFFER_CELLEND)
        {
            assert(pSprms && pAttributes);
            auto pValue = new RTFValue(1);
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
            Mapper().utext(reinterpret_cast<sal_uInt8 const*>(aString.getStr()),
                           aString.getLength());
        }
        else if (std::get<0>(aTuple) == BUFFER_ENDRUN)
            Mapper().endCharacterGroup();
        else if (std::get<0>(aTuple) == BUFFER_PAR)
            parBreak();
        else if (std::get<0>(aTuple) == BUFFER_STARTSHAPE)
            m_pSdrImport->resolve(std::get<1>(aTuple)->getShape(), false, RTFSdrImport::SHAPE);
        else if (std::get<0>(aTuple) == BUFFER_RESOLVESHAPE)
        {
            // Make sure there is no current buffer while replaying the shape,
            // otherwise it gets re-buffered.
            RTFBuffer_t* pCurrentBuffer = m_aStates.top().getCurrentBuffer();
            m_aStates.top().setCurrentBuffer(nullptr);

            // Set current shape during replay, needed by e.g. wrap in
            // background.
            m_aStates.top().getShape() = std::get<1>(aTuple)->getShape();

            m_pSdrImport->resolve(std::get<1>(aTuple)->getShape(), true, RTFSdrImport::SHAPE);
            m_aStates.top().setCurrentBuffer(pCurrentBuffer);
        }
        else if (std::get<0>(aTuple) == BUFFER_ENDSHAPE)
            m_pSdrImport->close();
        else if (std::get<0>(aTuple) == BUFFER_RESOLVESUBSTREAM)
        {
            RTFSprms& rAttributes = std::get<1>(aTuple)->getAttributes();
            std::size_t nPos = rAttributes.find(0)->getInt();
            Id nId = rAttributes.find(1)->getInt();
            OUString aCustomMark = rAttributes.find(2)->getString();
            resolveSubstream(nPos, nId, aCustomMark);
        }
        else if (std::get<0>(aTuple) == BUFFER_PICTURE)
            m_aStates.top().getPicture() = std::get<1>(aTuple)->getPicture();
        else if (std::get<0>(aTuple) == BUFFER_SETSTYLE)
        {
            if (!m_aStates.empty())
                m_aStates.top().setCurrentStyleIndex(std::get<1>(aTuple)->getInt());
        }
        else
            assert(false);
    }
}

bool findPropertyName(const std::vector<beans::PropertyValue>& rProperties, const OUString& rName)
{
    return std::any_of(
        rProperties.begin(), rProperties.end(),
        [&rName](const beans::PropertyValue& rProperty) { return rProperty.Name == rName; });
}

void RTFDocumentImpl::backupTableRowProperties()
{
    if (m_nTopLevelCurrentCellX)
    {
        m_aBackupTableRowSprms = m_aStates.top().getTableRowSprms();
        m_aBackupTableRowAttributes = m_aStates.top().getTableRowAttributes();
        m_nBackupTopLevelCurrentCellX = m_nTopLevelCurrentCellX;
    }
}

void RTFDocumentImpl::restoreTableRowProperties()
{
    m_aStates.top().getTableRowSprms() = m_aBackupTableRowSprms;
    m_aStates.top().getTableRowAttributes() = m_aBackupTableRowAttributes;
    m_nTopLevelCurrentCellX = m_nBackupTopLevelCurrentCellX;
}

void RTFDocumentImpl::resetTableRowProperties()
{
    m_aStates.top().getTableRowSprms() = m_aDefaultState.getTableRowSprms();
    m_aStates.top().getTableRowSprms().set(NS_ooxml::LN_CT_TblGridBase_gridCol, new RTFValue(-1),
                                           RTFOverwrite::NO_APPEND);
    m_aStates.top().getTableRowAttributes() = m_aDefaultState.getTableRowAttributes();
    if (Destination::NESTEDTABLEPROPERTIES == m_aStates.top().getDestination())
    {
        m_nNestedTRLeft = 0;
        m_nNestedCurrentCellX = 0;
    }
    else
    {
        m_nTopLevelTRLeft = 0;
        m_nTopLevelCurrentCellX = 0;
    }
}

RTFError RTFDocumentImpl::dispatchToggle(RTFKeyword nKeyword, bool bParam, int nParam)
{
    setNeedSect(true);
    checkUnicode(/*bUnicode =*/true, /*bHex =*/true);
    RTFSkipDestination aSkip(*this);
    int nSprm = -1;
    tools::SvRef<RTFValue> pBoolValue(new RTFValue(int(!bParam || nParam != 0)));

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
        auto pValue
            = new RTFValue((!bParam || nParam != 0) ? nSprm : NS_ooxml::LN_Value_ST_Underline_none);
        m_aStates.top().getCharacterAttributes().set(NS_ooxml::LN_CT_Underline_val, pValue);
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
        auto pValue = new RTFValue((!bParam || nParam != 0) ? nSprm : 0);
        m_aStates.top().getCharacterSprms().set(NS_ooxml::LN_EG_RPrBase_em, pValue);
        return RTFError::OK;
    }

    // Trivial character sprms.
    switch (nKeyword)
    {
        case RTF_B:
        case RTF_AB:
            switch (m_aStates.top().getRunType())
            {
                case RTFParserState::RunType::HICH:
                case RTFParserState::RunType::RTLCH_LTRCH_1:
                case RTFParserState::RunType::LTRCH_RTLCH_2:
                case RTFParserState::RunType::DBCH:
                    nSprm = NS_ooxml::LN_EG_RPrBase_bCs;
                    break;
                case RTFParserState::RunType::NONE:
                case RTFParserState::RunType::LOCH:
                case RTFParserState::RunType::LTRCH_RTLCH_1:
                case RTFParserState::RunType::RTLCH_LTRCH_2:
                default:
                    nSprm = NS_ooxml::LN_EG_RPrBase_b;
                    break;
            }
            break;
        case RTF_I:
        case RTF_AI:
            switch (m_aStates.top().getRunType())
            {
                case RTFParserState::RunType::HICH:
                case RTFParserState::RunType::RTLCH_LTRCH_1:
                case RTFParserState::RunType::LTRCH_RTLCH_2:
                case RTFParserState::RunType::DBCH:
                    nSprm = NS_ooxml::LN_EG_RPrBase_iCs;
                    break;
                case RTFParserState::RunType::NONE:
                case RTFParserState::RunType::LOCH:
                case RTFParserState::RunType::LTRCH_RTLCH_1:
                case RTFParserState::RunType::RTLCH_LTRCH_2:
                default:
                    nSprm = NS_ooxml::LN_EG_RPrBase_i;
                    break;
            }
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
        m_aStates.top().getCharacterSprms().set(nSprm, pBoolValue);
        return RTFError::OK;
    }

    switch (nKeyword)
    {
        case RTF_ASPALPHA:
            m_aStates.top().getParagraphSprms().set(NS_ooxml::LN_CT_PPrBase_autoSpaceDE,
                                                    pBoolValue);
            break;
        case RTF_DELETED:
        case RTF_REVISED:
        {
            auto pValue = new RTFValue(nKeyword == RTF_DELETED ? oox::XML_del : oox::XML_ins);
            putNestedAttribute(m_aStates.top().getCharacterSprms(), NS_ooxml::LN_trackchange,
                               NS_ooxml::LN_token, pValue);
        }
        break;
        case RTF_SBAUTO:
            putNestedAttribute(m_aStates.top().getParagraphSprms(), NS_ooxml::LN_CT_PPrBase_spacing,
                               NS_ooxml::LN_CT_Spacing_beforeAutospacing, pBoolValue);
            break;
        case RTF_SAAUTO:
            putNestedAttribute(m_aStates.top().getParagraphSprms(), NS_ooxml::LN_CT_PPrBase_spacing,
                               NS_ooxml::LN_CT_Spacing_afterAutospacing, pBoolValue);
            break;
        case RTF_FACINGP:
            m_aSettingsTableSprms.set(NS_ooxml::LN_CT_Settings_evenAndOddHeaders, pBoolValue);
            break;
        case RTF_HYPHAUTO:
            m_aSettingsTableSprms.set(NS_ooxml::LN_CT_Settings_autoHyphenation, pBoolValue);
            break;
        case RTF_HYPHPAR:
            m_aStates.top().getParagraphSprms().set(NS_ooxml::LN_CT_PPrBase_suppressAutoHyphens,
                                                    new RTFValue(int(bParam && nParam == 0)));
            break;
        default:
        {
            SAL_INFO("writerfilter.rtf",
                     "TODO handle toggle '" << keywordToString(nKeyword) << "'");
            aSkip.setParsed(false);
        }
        break;
    }
    return RTFError::OK;
}

RTFError RTFDocumentImpl::pushState()
{
    //SAL_INFO("writerfilter.rtf", __func__ << " before push: " << m_pTokenizer->getGroup());

    checkUnicode(/*bUnicode =*/true, /*bHex =*/true);
    m_nGroupStartPos = Strm().Tell();

    if (m_aStates.empty())
        m_aStates.push(m_aDefaultState);
    else
    {
        // fdo#85812 group resets run type of _current_ and new state (but not RTL)
        if (m_aStates.top().getRunType() != RTFParserState::RunType::LTRCH_RTLCH_2
            && m_aStates.top().getRunType() != RTFParserState::RunType::RTLCH_LTRCH_2)
        {
            m_aStates.top().setRunType(RTFParserState::RunType::NONE);
        }

        if (m_aStates.top().getDestination() == Destination::MR)
            lcl_DestinationToMath(m_aStates.top().getCurrentDestinationText(), m_aMathBuffer,
                                  m_bMathNor);
        m_aStates.push(m_aStates.top());
    }
    m_aStates.top().getDestinationText().setLength(0); // was copied: always reset!

    m_pTokenizer->pushGroup();

    switch (m_aStates.top().getDestination())
    {
        case Destination::FONTTABLE:
            // this is a "faked" destination for the font entry
            m_aStates.top().setCurrentDestinationText(&m_aStates.top().getDestinationText());
            m_aStates.top().setDestination(Destination::FONTENTRY);
            break;
        case Destination::STYLESHEET:
            // this is a "faked" destination for the style sheet entry
            m_aStates.top().setCurrentDestinationText(&m_aStates.top().getDestinationText());
            m_aStates.top().setDestination(Destination::STYLEENTRY);
            {
                // the *default* is \s0 i.e. paragraph style default
                // this will be overwritten by \sN \csN \dsN \tsN
                m_nCurrentStyleIndex = 0;
                auto pValue = new RTFValue(NS_ooxml::LN_Value_ST_StyleType_paragraph);
                m_aStates.top().getTableAttributes().set(NS_ooxml::LN_CT_Style_type, pValue);
            }
            break;
        case Destination::FIELDRESULT:
        case Destination::SHAPETEXT:
        case Destination::FORMFIELD:
        case Destination::FIELDINSTRUCTION:
        case Destination::PICT:
            m_aStates.top().setDestination(Destination::NORMAL);
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
            m_aStates.top().setDestination(Destination::MR);
            break;
        case Destination::REVISIONTABLE:
            // this is a "faked" destination for the revision table entry
            m_aStates.top().setCurrentDestinationText(&m_aStates.top().getDestinationText());
            m_aStates.top().setDestination(Destination::REVISIONENTRY);
            break;
        default:
            break;
    }

    // If this is true, then ooxml:endtrackchange will be generated.  Make sure
    // we don't generate more ooxml:endtrackchange than ooxml:trackchange: new
    // state does not inherit this flag.
    m_aStates.top().setStartedTrackchange(false);

    return RTFError::OK;
}

writerfilter::Reference<Properties>::Pointer_t RTFDocumentImpl::createStyleProperties()
{
    int nBasedOn = 0;
    RTFValue::Pointer_t pBasedOn
        = m_aStates.top().getTableSprms().find(NS_ooxml::LN_CT_Style_basedOn);
    if (pBasedOn)
        nBasedOn = pBasedOn->getInt();
    if (nBasedOn == 0)
    {
        // No parent style, then mimic what Word does: ignore attributes which
        // would set a margin as formatting, but with a default value.
        for (const auto& nId :
             { NS_ooxml::LN_CT_Ind_firstLine, NS_ooxml::LN_CT_Ind_left, NS_ooxml::LN_CT_Ind_right,
               NS_ooxml::LN_CT_Ind_start, NS_ooxml::LN_CT_Ind_end })
        {
            RTFValue::Pointer_t pValue = getNestedAttribute(m_aStates.top().getParagraphSprms(),
                                                            NS_ooxml::LN_CT_PPrBase_ind, nId);
            if (pValue && pValue->getInt() == 0)
                eraseNestedAttribute(m_aStates.top().getParagraphSprms(),
                                     NS_ooxml::LN_CT_PPrBase_ind, nId);
        }
    }

    RTFValue::Pointer_t pParaProps = new RTFValue(m_aStates.top().getParagraphAttributes(),
                                                  m_aStates.top().getParagraphSprms());
    RTFValue::Pointer_t pCharProps = new RTFValue(m_aStates.top().getCharacterAttributes(),
                                                  m_aStates.top().getCharacterSprms());

    // resetSprms will clean up this modification
    m_aStates.top().getTableSprms().set(NS_ooxml::LN_CT_Style_pPr, pParaProps);
    m_aStates.top().getTableSprms().set(NS_ooxml::LN_CT_Style_rPr, pCharProps);

    writerfilter::Reference<Properties>::Pointer_t pProps(new RTFReferenceProperties(
        m_aStates.top().getTableAttributes(), m_aStates.top().getTableSprms()));
    return pProps;
}

/** 2 different representations of the styles are needed:

    1) flat content, as read from the input file:
       stored in m_aStyleTableEntries, used as reference input for
       deduplication both here and for hard formatting in getProperties()

    2) real content, with proper override of sprms/attributes where it differs
       from parent style; this is produced here and sent to domain mapper
 */
RTFReferenceTable::Entries_t RTFDocumentImpl::deduplicateStyleTable()
{
    RTFReferenceTable::Entries_t ret;
    for (auto const& it : m_aStyleTableEntries)
    {
        auto pStyle = it.second;
        ret[it.first] = pStyle;
        // ugly downcasts here, but can't easily replace the members with
        // RTFReferenceProperties because dmapper wants SvRef<Properties> anyway
        RTFValue::Pointer_t const pBasedOn(
            static_cast<RTFReferenceProperties&>(*pStyle).getSprms().find(
                NS_ooxml::LN_CT_Style_basedOn));
        if (pBasedOn)
        {
            int const nBasedOn(pBasedOn->getInt());
            // don't deduplicate yourself - especially a potential problem for the default style.
            if (it.first == nBasedOn)
                continue;

            auto const itParent(m_aStyleTableEntries.find(nBasedOn)); // definition as read!
            if (itParent != m_aStyleTableEntries.end())
            {
                auto const pStyleType(
                    static_cast<RTFReferenceProperties&>(*pStyle).getAttributes().find(
                        NS_ooxml::LN_CT_Style_type));
                assert(pStyleType);
                int const nStyleType(pStyleType->getInt());
                RTFSprms const sprms(
                    static_cast<RTFReferenceProperties&>(*pStyle).getSprms().cloneAndDeduplicate(
                        static_cast<RTFReferenceProperties&>(*itParent->second).getSprms(),
                        nStyleType));
                RTFSprms const attributes(
                    static_cast<RTFReferenceProperties&>(*pStyle)
                        .getAttributes()
                        .cloneAndDeduplicate(
                            static_cast<RTFReferenceProperties&>(*itParent->second).getAttributes(),
                            nStyleType));

                ret[it.first] = new RTFReferenceProperties(attributes, sprms);
            }
            else
            {
                SAL_WARN("writerfilter.rtf", "parent style not found: " << nBasedOn);
            }
        }
    }
    assert(ret.size() == m_aStyleTableEntries.size());
    return ret;
}

void RTFDocumentImpl::resetSprms()
{
    m_aStates.top().getTableSprms().clear();
    m_aStates.top().getCharacterSprms().clear();
    m_aStates.top().getParagraphSprms().clear();
}

void RTFDocumentImpl::resetAttributes()
{
    m_aStates.top().getTableAttributes().clear();
    m_aStates.top().getCharacterAttributes().clear();
    m_aStates.top().getParagraphAttributes().clear();
}

static bool lcl_containsProperty(const uno::Sequence<beans::Property>& rProperties,
                                 const OUString& rName)
{
    return std::any_of(rProperties.begin(), rProperties.end(),
                       [&](const beans::Property& rProperty) { return rProperty.Name == rName; });
}

RTFError RTFDocumentImpl::beforePopState(RTFParserState& rState)
{
    switch (rState.getDestination())
    {
        case Destination::FONTTABLE:
        {
            writerfilter::Reference<Table>::Pointer_t const pTable(
                new RTFReferenceTable(m_aFontTableEntries));
            Mapper().table(NS_ooxml::LN_FONTTABLE, pTable);
            if (m_nDefaultFontIndex >= 0)
            {
                auto pValue = new RTFValue(m_aFontNames[getFontIndex(m_nDefaultFontIndex)]);
                putNestedAttribute(m_aDefaultState.getCharacterSprms(),
                                   NS_ooxml::LN_EG_RPrBase_rFonts, NS_ooxml::LN_CT_Fonts_ascii,
                                   pValue);
            }
        }
        break;
        case Destination::STYLESHEET:
        {
            RTFReferenceTable::Entries_t const pStyleTableDeduplicated(deduplicateStyleTable());
            writerfilter::Reference<Table>::Pointer_t const pTable(
                new RTFReferenceTable(pStyleTableDeduplicated));
            Mapper().table(NS_ooxml::LN_STYLESHEET, pTable);
        }
        break;
        case Destination::LISTOVERRIDETABLE:
        {
            RTFSprms aListTableAttributes;
            writerfilter::Reference<Properties>::Pointer_t pProp
                = new RTFReferenceProperties(aListTableAttributes, m_aListTableSprms);
            RTFReferenceTable::Entries_t aListTableEntries;
            aListTableEntries.insert(std::make_pair(0, pProp));
            writerfilter::Reference<Table>::Pointer_t const pTable(
                new RTFReferenceTable(aListTableEntries));
            Mapper().table(NS_ooxml::LN_NUMBERING, pTable);
        }
        break;
        case Destination::LISTENTRY:
            for (const auto& rListLevelEntry : rState.getListLevelEntries())
                rState.getTableSprms().set(rListLevelEntry.first, rListLevelEntry.second,
                                           RTFOverwrite::NO_APPEND);
            break;
        case Destination::FIELDINSTRUCTION:
        {
            auto pValue = new RTFValue(m_aFormfieldAttributes, m_aFormfieldSprms);
            RTFSprms aFFAttributes;
            RTFSprms aFFSprms;
            aFFSprms.set(NS_ooxml::LN_ffdata, pValue);
            if (!m_aStates.top().getCurrentBuffer())
            {
                writerfilter::Reference<Properties>::Pointer_t pProperties
                    = new RTFReferenceProperties(aFFAttributes, aFFSprms);
                Mapper().props(pProperties);
            }
            else
            {
                auto pFFValue = new RTFValue(aFFAttributes, aFFSprms);
                bufferProperties(*m_aStates.top().getCurrentBuffer(), pFFValue, nullptr);
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
                OUString aFileURL = m_rMediaDescriptor.getUnpackedValueOrDefault(
                    utl::MediaDescriptor::PROP_URL(), OUString());
                OUString aPictureURL;
                try
                {
                    aPictureURL = rtl::Uri::convertRelToAbs(aFileURL, m_aPicturePath);
                }
                catch (const rtl::MalformedUriException& rException)
                {
                    SAL_WARN("writerfilter.rtf",
                             "rtl::Uri::convertRelToAbs() failed: " << rException.getMessage());
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
                        m_aStates.top().getDestinationText() = aBuf;
                    }
                }
                popState();
                m_aPicturePath.clear();
            }

            break;
        case Destination::LEVELTEXT:
        {
            if (&m_aStates.top().getDestinationText()
                != m_aStates.top().getCurrentDestinationText())
                break; // not for nested group
            OUString aStr = m_aStates.top().getCurrentDestinationText()->makeStringAndClear();

            // The first character is the length of the string (the rest should be ignored).
            sal_Int32 nLength(aStr.toChar());
            OUString aValue;
            if (nLength < aStr.getLength())
                aValue = aStr.copy(1, nLength);
            else
                aValue = aStr;
            auto pValue = new RTFValue(aValue, true);
            rState.getTableAttributes().set(NS_ooxml::LN_CT_LevelText_val, pValue);
        }
        break;
        case Destination::LEVELNUMBERS:
        {
            bool bNestedLevelNumbers = false;
            if (m_aStates.size() > 1)
                // Current destination is levelnumbers and parent destination is levelnumbers as well.
                bNestedLevelNumbers
                    = m_aStates[m_aStates.size() - 2].getDestination() == Destination::LEVELNUMBERS;
            if (!bNestedLevelNumbers && rState.getTableSprms().find(NS_ooxml::LN_CT_Lvl_lvlText))
            {
                RTFSprms& rAttributes
                    = rState.getTableSprms().find(NS_ooxml::LN_CT_Lvl_lvlText)->getAttributes();
                RTFValue::Pointer_t pValue = rAttributes.find(NS_ooxml::LN_CT_LevelText_val);
                if (pValue && rState.getLevelNumbersValid())
                {
                    OUString aOrig = pValue->getString();

                    OUStringBuffer aBuf(aOrig.getLength() * 2);
                    sal_Int32 nReplaces = 1;
                    for (int i = 0; i < aOrig.getLength(); i++)
                    {
                        if (std::find(rState.getLevelNumbers().begin(),
                                      rState.getLevelNumbers().end(), i + 1)
                            != rState.getLevelNumbers().end())
                        {
                            aBuf.append('%');
                            // '1.1.1' -> '%1.%2.%3', but '1.' (with '2.' prefix omitted) is %2.
                            aBuf.append(sal_Int32(nReplaces++ + rState.getListLevelNum() + 1
                                                  - rState.getLevelNumbers().size()));
                        }
                        else
                            aBuf.append(aOrig[i]);
                    }

                    pValue->setString(aBuf.makeStringAndClear());
                }
                else if (pValue)
                    // Have a value, but levelnumbers is not valid -> ignore it.
                    pValue->setString(OUString());
            }
            break;
        }
        case Destination::SHAPEPROPERTYNAME:
            if (&m_aStates.top().getDestinationText()
                != m_aStates.top().getCurrentDestinationText())
                break; // not for nested group
            rState.getShape().getProperties().emplace_back(
                m_aStates.top().getCurrentDestinationText()->makeStringAndClear(), OUString());
            break;
        case Destination::SHAPEPROPERTYVALUE:
            if (!rState.getShape().getProperties().empty())
            {
                rState.getShape().getProperties().back().second
                    = m_aStates.top().getCurrentDestinationText()->makeStringAndClear();
                if (m_aStates.top().getHadShapeText())
                    m_pSdrImport->append(rState.getShape().getProperties().back().first,
                                         rState.getShape().getProperties().back().second);
                else if (rState.getInShapeGroup() && !rState.getInShape()
                         && rState.getShape().getProperties().back().first == "rotation")
                {
                    // Rotation should be applied on the groupshape itself, not on each shape.
                    rState.getShape().getGroupProperties().push_back(
                        rState.getShape().getProperties().back());
                    rState.getShape().getProperties().pop_back();
                }
            }
            break;
        case Destination::PICPROP:
        case Destination::SHAPEINSTRUCTION:
            if (m_aStates.size() > 1
                && m_aStates[m_aStates.size() - 2].getDestination()
                       == Destination::SHAPEINSTRUCTION)
            {
                // Do not resolve shape if shape instruction destination is inside other shape instruction
            }
            else if (!m_bObject && !rState.getInListpicture() && !rState.getHadShapeText()
                     && (!rState.getInShapeGroup() || rState.getInShape()))
            {
                // Don't trigger a shape import in case we're only leaving the \shpinst of the groupshape itself.
                RTFSdrImport::ShapeOrPict eType
                    = (rState.getDestination() == Destination::SHAPEINSTRUCTION)
                          ? RTFSdrImport::SHAPE
                          : RTFSdrImport::PICT;
                if (!m_aStates.top().getCurrentBuffer() || eType != RTFSdrImport::SHAPE)
                    m_pSdrImport->resolve(m_aStates.top().getShape(), true, eType);
                else
                {
                    // Shape inside table: buffer the import to have correct anchor position.
                    // Also buffer the RTFPicture of the state stack as it contains
                    // the shape size.
                    auto pPictureValue = new RTFValue(m_aStates.top().getPicture());
                    m_aStates.top().getCurrentBuffer()->push_back(
                        Buf_t(BUFFER_PICTURE, pPictureValue, nullptr));
                    auto pValue = new RTFValue(m_aStates.top().getShape());

                    // Buffer wrap type.
                    for (const auto& rCharacterSprm : m_aStates.top().getCharacterSprms())
                    {
                        if (rCharacterSprm.first == NS_ooxml::LN_EG_WrapType_wrapNone
                            || rCharacterSprm.first == NS_ooxml::LN_EG_WrapType_wrapTight)
                        {
                            m_aStates.top().getShape().getWrapSprm() = rCharacterSprm;
                            break;
                        }
                    }

                    m_aStates.top().getCurrentBuffer()->push_back(
                        Buf_t(BUFFER_RESOLVESHAPE, pValue, nullptr));
                }
            }
            else if (rState.getInShapeGroup() && !rState.getInShape())
            {
                // End of a groupshape, as we're in shapegroup, but not in a real shape.
                for (const auto& rGroupProperty : rState.getShape().getGroupProperties())
                    m_pSdrImport->appendGroupProperty(rGroupProperty.first, rGroupProperty.second);
                rState.getShape().getGroupProperties().clear();
            }
            break;
        case Destination::BOOKMARKSTART:
        {
            if (&m_aStates.top().getDestinationText()
                != m_aStates.top().getCurrentDestinationText())
                break; // not for nested group
            OUString aStr = m_aStates.top().getCurrentDestinationText()->makeStringAndClear();
            int nPos = m_aBookmarks.size();
            m_aBookmarks[aStr] = nPos;
            if (!m_aStates.top().getCurrentBuffer())
                Mapper().props(new RTFReferenceProperties(lcl_getBookmarkProperties(nPos, aStr)));
            else
                bufferProperties(*m_aStates.top().getCurrentBuffer(),
                                 new RTFValue(lcl_getBookmarkProperties(nPos, aStr)), nullptr);
        }
        break;
        case Destination::BOOKMARKEND:
        {
            if (&m_aStates.top().getDestinationText()
                != m_aStates.top().getCurrentDestinationText())
                break; // not for nested group
            OUString aStr = m_aStates.top().getCurrentDestinationText()->makeStringAndClear();
            if (!m_aStates.top().getCurrentBuffer())
                Mapper().props(new RTFReferenceProperties(
                    lcl_getBookmarkProperties(m_aBookmarks[aStr], aStr)));
            else
                bufferProperties(*m_aStates.top().getCurrentBuffer(),
                                 new RTFValue(lcl_getBookmarkProperties(m_aBookmarks[aStr], aStr)),
                                 nullptr);
        }
        break;
        case Destination::INDEXENTRY:
        case Destination::TOCENTRY:
        {
            if (&m_aStates.top().getDestinationText()
                != m_aStates.top().getCurrentDestinationText())
                break; // not for nested group
            OUString str(m_aStates.top().getCurrentDestinationText()->makeStringAndClear());
            // dmapper expects this as a field, so let's fake something...
            auto const field((Destination::INDEXENTRY == rState.getDestination())
                                 ? OUStringLiteral(u"XE")
                                 : OUStringLiteral(u"TC"));
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
            if (&m_aStates.top().getDestinationText()
                != m_aStates.top().getCurrentDestinationText())
                break; // not for nested group
            auto pValue
                = new RTFValue(m_aStates.top().getCurrentDestinationText()->makeStringAndClear());
            m_aFormfieldSprms.set(NS_ooxml::LN_CT_FFData_name, pValue);
        }
        break;
        case Destination::FORMFIELDLIST:
        {
            if (&m_aStates.top().getDestinationText()
                != m_aStates.top().getCurrentDestinationText())
                break; // not for nested group
            auto pValue
                = new RTFValue(m_aStates.top().getCurrentDestinationText()->makeStringAndClear());
            m_aFormfieldSprms.set(NS_ooxml::LN_CT_FFDDList_listEntry, pValue);
        }
        break;
        case Destination::DATAFIELD:
        {
            if (m_bFormField)
            {
                if (&m_aStates.top().getDestinationText()
                    != m_aStates.top().getCurrentDestinationText())
                    break; // not for nested group
                OString aStr = OUStringToOString(
                    m_aStates.top().getCurrentDestinationText()->makeStringAndClear(),
                    rState.getCurrentEncoding());
                // decode hex dump
                OStringBuffer aBuf;
                int b = 0;
                int count = 2;
                for (int i = 0; i < aStr.getLength(); ++i)
                {
                    char ch = aStr[i];
                    if (ch != 0x0d && ch != 0x0a)
                    {
                        b = b << 4;
                        sal_Int8 parsed = msfilter::rtfutil::AsHex(ch);
                        if (parsed == -1)
                            return RTFError::HEX_INVALID;
                        b += parsed;
                        count--;
                        if (!count)
                        {
                            aBuf.append(static_cast<char>(b));
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
                    aStr = aStr.copy(nLength + 1); // zero-terminated string
                else
                    aStr.clear();
                // extract default text
                nLength = aStr.toChar();
                if (!aStr.isEmpty())
                    aStr = aStr.copy(1);
                auto pNValue = new RTFValue(OStringToOUString(aName, rState.getCurrentEncoding()));
                m_aFormfieldSprms.set(NS_ooxml::LN_CT_FFData_name, pNValue);
                if (nLength > 0)
                {
                    OString aDefaultText = aStr.copy(0, std::min(nLength, aStr.getLength()));
                    auto pDValue = new RTFValue(
                        OStringToOUString(aDefaultText, rState.getCurrentEncoding()));
                    m_aFormfieldSprms.set(NS_ooxml::LN_CT_FFTextInput_default, pDValue);
                }

                m_bFormField = false;
            }
        }
        break;
        case Destination::CREATIONTIME:
            if (m_xDocumentProperties.is())
                m_xDocumentProperties->setCreationDate(lcl_getDateTime(rState));
            break;
        case Destination::REVISIONTIME:
            if (m_xDocumentProperties.is())
                m_xDocumentProperties->setModificationDate(lcl_getDateTime(rState));
            break;
        case Destination::PRINTTIME:
            if (m_xDocumentProperties.is())
                m_xDocumentProperties->setPrintDate(lcl_getDateTime(rState));
            break;
        case Destination::AUTHOR:
            if (&m_aStates.top().getDestinationText()
                != m_aStates.top().getCurrentDestinationText())
                break; // not for nested group
            if (m_xDocumentProperties.is())
                m_xDocumentProperties->setAuthor(
                    m_aStates.top().getCurrentDestinationText()->makeStringAndClear());
            break;
        case Destination::KEYWORDS:
            if (&m_aStates.top().getDestinationText()
                != m_aStates.top().getCurrentDestinationText())
                break; // not for nested group
            if (m_xDocumentProperties.is())
                m_xDocumentProperties->setKeywords(comphelper::string::convertCommaSeparated(
                    m_aStates.top().getCurrentDestinationText()->makeStringAndClear()));
            break;
        case Destination::COMMENT:
            if (&m_aStates.top().getDestinationText()
                != m_aStates.top().getCurrentDestinationText())
                break; // not for nested group
            if (m_xDocumentProperties.is())
                m_xDocumentProperties->setGenerator(
                    m_aStates.top().getCurrentDestinationText()->makeStringAndClear());
            break;
        case Destination::SUBJECT:
            if (&m_aStates.top().getDestinationText()
                != m_aStates.top().getCurrentDestinationText())
                break; // not for nested group
            if (m_xDocumentProperties.is())
                m_xDocumentProperties->setSubject(
                    m_aStates.top().getCurrentDestinationText()->makeStringAndClear());
            break;
        case Destination::TITLE:
        {
            if (&m_aStates.top().getDestinationText()
                != m_aStates.top().getCurrentDestinationText())
                break; // not for nested group
            if (m_xDocumentProperties.is())
                m_xDocumentProperties->setTitle(
                    rState.getCurrentDestinationText()->makeStringAndClear());
        }
        break;

        case Destination::DOCCOMM:
            if (&m_aStates.top().getDestinationText()
                != m_aStates.top().getCurrentDestinationText())
                break; // not for nested group
            if (m_xDocumentProperties.is())
                m_xDocumentProperties->setDescription(
                    m_aStates.top().getCurrentDestinationText()->makeStringAndClear());
            break;
        case Destination::OPERATOR:
        case Destination::COMPANY:
        {
            if (&m_aStates.top().getDestinationText()
                != m_aStates.top().getCurrentDestinationText())
                break; // not for nested group
            OUString aName = rState.getDestination() == Destination::OPERATOR ? OUString("Operator")
                                                                              : OUString("Company");
            uno::Any aValue
                = uno::makeAny(m_aStates.top().getCurrentDestinationText()->makeStringAndClear());
            if (m_xDocumentProperties.is())
            {
                uno::Reference<beans::XPropertyContainer> xUserDefinedProperties
                    = m_xDocumentProperties->getUserDefinedProperties();
                uno::Reference<beans::XPropertySet> xPropertySet(xUserDefinedProperties,
                                                                 uno::UNO_QUERY);
                uno::Reference<beans::XPropertySetInfo> xPropertySetInfo
                    = xPropertySet->getPropertySetInfo();
                if (xPropertySetInfo->hasPropertyByName(aName))
                    xPropertySet->setPropertyValue(aName, aValue);
                else
                    xUserDefinedProperties->addProperty(aName, beans::PropertyAttribute::REMOVABLE,
                                                        aValue);
            }
        }
        break;
        case Destination::OBJDATA:
        {
            if (&m_aStates.top().getDestinationText()
                != m_aStates.top().getCurrentDestinationText())
                break; // not for nested group

            RTFError eError = handleEmbeddedObject();
            if (eError != RTFError::OK)
                return eError;
        }
        break;
        case Destination::OBJCLASS:
        {
            auto pValue
                = new RTFValue(m_aStates.top().getCurrentDestinationText()->makeStringAndClear());
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
            auto pOLEValue = new RTFValue(m_aOLEAttributes);
            aObjectSprms.set(NS_ooxml::LN_OLEObject_OLEObject, pOLEValue);

            RTFSprms aObjAttributes;
            RTFSprms aObjSprms;
            auto pValue = new RTFValue(m_aObjectAttributes, aObjectSprms);
            aObjSprms.set(NS_ooxml::LN_object, pValue);
            writerfilter::Reference<Properties>::Pointer_t pProperties
                = new RTFReferenceProperties(aObjAttributes, aObjSprms);
            uno::Reference<drawing::XShape> xShape;
            RTFValue::Pointer_t pShape = m_aObjectAttributes.find(NS_ooxml::LN_shape);
            OSL_ASSERT(pShape);
            if (pShape)
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
            if (&m_aStates.top().getDestinationText()
                != m_aStates.top().getCurrentDestinationText())
                break; // not for nested group
            OUString aStr(OStringToOUString(
                DTTM22OString(
                    m_aStates.top().getCurrentDestinationText()->makeStringAndClear().toInt32()),
                rState.getCurrentEncoding()));
            auto pValue = new RTFValue(aStr);
            RTFSprms aAnnAttributes;
            aAnnAttributes.set(NS_ooxml::LN_CT_TrackChange_date, pValue);
            writerfilter::Reference<Properties>::Pointer_t pProperties
                = new RTFReferenceProperties(aAnnAttributes);
            Mapper().props(pProperties);
        }
        break;
        case Destination::ANNOTATIONAUTHOR:
            if (&m_aStates.top().getDestinationText()
                != m_aStates.top().getCurrentDestinationText())
                break; // not for nested group
            m_aAuthor = m_aStates.top().getCurrentDestinationText()->makeStringAndClear();
            break;
        case Destination::ATNID:
            if (&m_aStates.top().getDestinationText()
                != m_aStates.top().getCurrentDestinationText())
                break; // not for nested group
            m_aAuthorInitials = m_aStates.top().getCurrentDestinationText()->makeStringAndClear();
            break;
        case Destination::ANNOTATIONREFERENCESTART:
        case Destination::ANNOTATIONREFERENCEEND:
        {
            if (&m_aStates.top().getDestinationText()
                != m_aStates.top().getCurrentDestinationText())
                break; // not for nested group
            OUString aStr = m_aStates.top().getCurrentDestinationText()->makeStringAndClear();
            auto pValue = new RTFValue(aStr.toInt32());
            RTFSprms aAttributes;
            if (rState.getDestination() == Destination::ANNOTATIONREFERENCESTART)
                aAttributes.set(NS_ooxml::LN_EG_RangeMarkupElements_commentRangeStart, pValue);
            else
                aAttributes.set(NS_ooxml::LN_EG_RangeMarkupElements_commentRangeEnd, pValue);
            writerfilter::Reference<Properties>::Pointer_t pProperties
                = new RTFReferenceProperties(aAttributes);
            Mapper().props(pProperties);
        }
        break;
        case Destination::ANNOTATIONREFERENCE:
        {
            if (&m_aStates.top().getDestinationText()
                != m_aStates.top().getCurrentDestinationText())
                break; // not for nested group
            OUString aStr = m_aStates.top().getCurrentDestinationText()->makeStringAndClear();
            RTFSprms aAnnAttributes;
            aAnnAttributes.set(NS_ooxml::LN_CT_Markup_id, new RTFValue(aStr.toInt32()));
            Mapper().props(new RTFReferenceProperties(aAnnAttributes));
        }
        break;
        case Destination::FALT:
        {
            if (&m_aStates.top().getDestinationText()
                != m_aStates.top().getCurrentDestinationText())
                break; // not for nested group
            OUString aStr(m_aStates.top().getCurrentDestinationText()->makeStringAndClear());
            auto pValue = new RTFValue(aStr);
            rState.getTableSprms().set(NS_ooxml::LN_CT_Font_altName, pValue);
        }
        break;
        case Destination::DRAWINGOBJECT:
            if (m_aStates.top().getDrawingObject().getShape().is())
            {
                RTFDrawingObject& rDrawing = m_aStates.top().getDrawingObject();
                const uno::Reference<drawing::XShape>& xShape(rDrawing.getShape());
                const uno::Reference<beans::XPropertySet>& xPropertySet(rDrawing.getPropertySet());

                uno::Reference<lang::XServiceInfo> xServiceInfo(xShape, uno::UNO_QUERY);
                bool bTextFrame = xServiceInfo->supportsService("com.sun.star.text.TextFrame");

                // The default is certainly not inline, but then what Word supports is just at-character.
                xPropertySet->setPropertyValue(
                    "AnchorType", uno::makeAny(text::TextContentAnchorType_AT_CHARACTER));

                if (bTextFrame)
                {
                    xPropertySet->setPropertyValue("HoriOrientPosition",
                                                   uno::makeAny(rDrawing.getLeft()));
                    xPropertySet->setPropertyValue("VertOrientPosition",
                                                   uno::makeAny(rDrawing.getTop()));
                }
                else
                {
                    xShape->setPosition(awt::Point(rDrawing.getLeft(), rDrawing.getTop()));
                }
                xShape->setSize(awt::Size(rDrawing.getRight(), rDrawing.getBottom()));

                if (rDrawing.getHasLineColor())
                {
                    uno::Any aLineColor = uno::makeAny(sal_uInt32((rDrawing.getLineColorR() << 16)
                                                                  + (rDrawing.getLineColorG() << 8)
                                                                  + rDrawing.getLineColorB()));
                    uno::Any aLineWidth;
                    RTFSdrImport::resolveLineColorAndWidth(bTextFrame, xPropertySet, aLineColor,
                                                           aLineWidth);
                }
                if (rDrawing.getHasFillColor())
                    xPropertySet->setPropertyValue(
                        "FillColor", uno::makeAny(sal_uInt32((rDrawing.getFillColorR() << 16)
                                                             + (rDrawing.getFillColorG() << 8)
                                                             + rDrawing.getFillColorB())));
                else if (!bTextFrame)
                    // If there is no fill, the Word default is 100% transparency.
                    xPropertySet->setPropertyValue("FillTransparence",
                                                   uno::makeAny(sal_Int32(100)));

                RTFSdrImport::resolveFLine(xPropertySet, rDrawing.getFLine());

                if (!m_aStates.top().getDrawingObject().getHadShapeText())
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
            if (rState.getFrame().inFrame())
            {
                // parBreak() modifies m_aStates.top() so we can't apply resetFrame() directly on aState
                resetFrame();
                parBreak();
                // Save this state for later use, so we only reset frame status only for the first shape inside a frame.
                rState = m_aStates.top();
                m_bNeedPap = true;
            }
            break;
        case Destination::MOMATH:
        {
            m_aMathBuffer.appendClosingTag(M_TOKEN(oMath));

            SvGlobalName aGlobalName(SO3_SM_CLASSID);
            comphelper::EmbeddedObjectContainer aContainer;
            OUString aName;
            uno::Reference<embed::XEmbeddedObject> xObject
                = aContainer.CreateEmbeddedObject(aGlobalName.GetByteSequence(), aName);
            if (xObject) // rhbz#1766990 starmath might not be available
            {
                uno::Reference<util::XCloseable> xComponent(xObject->getComponent(),
                                                            uno::UNO_SET_THROW);
                // gcc4.4 (and 4.3 and possibly older) have a problem with dynamic_cast directly to the target class,
                // so help it with an intermediate cast. I'm not sure what exactly the problem is, seems to be unrelated
                // to RTLD_GLOBAL, so most probably a gcc bug.
                auto& rImport = dynamic_cast<oox::FormulaImportBase&>(
                    dynamic_cast<SfxBaseModel&>(*xComponent));
                rImport.readFormulaOoxml(m_aMathBuffer);

                auto pValue = new RTFValue(xObject);
                RTFSprms aMathAttributes;
                aMathAttributes.set(NS_ooxml::LN_starmath, pValue);
                writerfilter::Reference<Properties>::Pointer_t pProperties
                    = new RTFReferenceProperties(aMathAttributes);
                Mapper().props(pProperties);
            }

            m_aMathBuffer = oox::formulaimport::XmlStreamBuilder();
        }
        break;
        case Destination::MR:
            lcl_DestinationToMath(m_aStates.top().getCurrentDestinationText(), m_aMathBuffer,
                                  m_bMathNor);
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
            switch (rState.getDestination())
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
            aAttribs[M_TOKEN(val)]
                = m_aStates.top().getCurrentDestinationText()->makeStringAndClear();
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
            if (rState.getCreatedShapeGroup())
                m_pSdrImport->popParent();
            break;
        case Destination::PROPNAME:
            if (&m_aStates.top().getDestinationText()
                != m_aStates.top().getCurrentDestinationText())
                break; // not for nested group
            rState.setPropName(m_aStates.top().getCurrentDestinationText()->makeStringAndClear());
            break;
        case Destination::STATICVAL:
            if (&m_aStates.top().getDestinationText()
                != m_aStates.top().getCurrentDestinationText())
                break; // not for nested group
            if (m_xDocumentProperties.is())
            {
                // Find out what is the key, value type and value we want to set.
                uno::Reference<beans::XPropertyContainer> xPropertyContainer
                    = m_xDocumentProperties->getUserDefinedProperties();
                const OUString& rKey = m_aStates.top().getPropName();
                OUString aStaticVal
                    = m_aStates.top().getCurrentDestinationText()->makeStringAndClear();
                uno::Any aAny;
                if (m_aStates.top().getPropType() == cppu::UnoType<OUString>::get())
                    aAny <<= aStaticVal;
                else if (m_aStates.top().getPropType() == cppu::UnoType<sal_Int32>::get())
                    aAny <<= aStaticVal.toInt32();
                else if (m_aStates.top().getPropType() == cppu::UnoType<bool>::get())
                    aAny <<= aStaticVal.toBoolean();
                else if (m_aStates.top().getPropType() == cppu::UnoType<util::DateTime>::get())
                    aAny <<= getDateTimeFromUserProp(aStaticVal);
                else if (m_aStates.top().getPropType() == cppu::UnoType<double>::get())
                    aAny <<= aStaticVal.toDouble();

                xPropertyContainer->addProperty(rKey, beans::PropertyAttribute::REMOVABLE, aAny);
            }
            break;
        case Destination::USERPROPS:
        {
            // These are the imported properties.
            uno::Reference<document::XDocumentProperties> xDocumentProperties
                = m_xDocumentProperties;

            // These are the real document properties.
            uno::Reference<document::XDocumentPropertiesSupplier> xDocumentPropertiesSupplier(
                m_xDstDoc, uno::UNO_QUERY);
            if (xDocumentPropertiesSupplier.is())
                m_xDocumentProperties = xDocumentPropertiesSupplier->getDocumentProperties();

            if (m_xDocumentProperties.is())
            {
                if (!m_bIsNewDoc)
                {
                    // Check classification.
                    if (!SfxClassificationHelper::ShowPasteInfo(SfxClassificationHelper::CheckPaste(
                            xDocumentProperties, m_xDocumentProperties)))
                        return RTFError::CLASSIFICATION;
                }

                uno::Reference<beans::XPropertyContainer> xClipboardPropertyContainer
                    = xDocumentProperties->getUserDefinedProperties();
                uno::Reference<beans::XPropertyContainer> xDocumentPropertyContainer
                    = m_xDocumentProperties->getUserDefinedProperties();
                uno::Reference<beans::XPropertySet> xClipboardPropertySet(
                    xClipboardPropertyContainer, uno::UNO_QUERY);
                uno::Reference<beans::XPropertySet> xDocumentPropertySet(xDocumentPropertyContainer,
                                                                         uno::UNO_QUERY);
                const uno::Sequence<beans::Property> aClipboardProperties
                    = xClipboardPropertySet->getPropertySetInfo()->getProperties();
                uno::Sequence<beans::Property> aDocumentProperties
                    = xDocumentPropertySet->getPropertySetInfo()->getProperties();

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
                            xDocumentPropertyContainer->addProperty(
                                rKey, beans::PropertyAttribute::REMOVABLE, aValue);
                    }
                    catch (const uno::Exception&)
                    {
                        TOOLS_WARN_EXCEPTION("writerfilter.rtf", "failed to set property " << rKey);
                    }
                }
            }
        }
        break;
        default:
            break;
    }

    return RTFError::OK;
}

void RTFDocumentImpl::afterPopState(RTFParserState& rState)
{
    // list table
    switch (rState.getDestination())
    {
        case Destination::LISTENTRY:
        {
            auto pValue = new RTFValue(rState.getTableAttributes(), rState.getTableSprms());
            m_aListTableSprms.set(NS_ooxml::LN_CT_Numbering_abstractNum, pValue,
                                  RTFOverwrite::NO_APPEND);
            m_aListTable[rState.getCurrentListIndex()] = pValue;
            m_nListLevel = -1;
            m_aInvalidListTableFirstIndents[rState.getCurrentListIndex()]
                = m_aInvalidListLevelFirstIndents;
            m_aInvalidListLevelFirstIndents.clear();
        }
        break;
        case Destination::PARAGRAPHNUMBERING:
        {
            RTFValue::Pointer_t pIdValue
                = rState.getTableAttributes().find(NS_ooxml::LN_CT_AbstractNum_nsid);
            if (pIdValue && !m_aStates.empty())
            {
                // Abstract numbering
                RTFSprms aLeveltextAttributes;
                OUString aTextValue;
                RTFValue::Pointer_t pTextBefore
                    = rState.getTableAttributes().find(NS_ooxml::LN_CT_LevelText_val);
                if (pTextBefore)
                    aTextValue += pTextBefore->getString();
                aTextValue += "%1";
                RTFValue::Pointer_t pTextAfter
                    = rState.getTableAttributes().find(NS_ooxml::LN_CT_LevelSuffix_val);
                if (pTextAfter)
                    aTextValue += pTextAfter->getString();
                auto pTextValue = new RTFValue(aTextValue);
                aLeveltextAttributes.set(NS_ooxml::LN_CT_LevelText_val, pTextValue);

                RTFSprms aLevelAttributes;
                RTFSprms aLevelSprms;
                auto pIlvlValue = new RTFValue(0);
                aLevelAttributes.set(NS_ooxml::LN_CT_Lvl_ilvl, pIlvlValue);

                RTFValue::Pointer_t pFmtValue
                    = rState.getTableSprms().find(NS_ooxml::LN_CT_Lvl_numFmt);
                if (pFmtValue)
                    aLevelSprms.set(NS_ooxml::LN_CT_Lvl_numFmt, pFmtValue);

                RTFValue::Pointer_t pStartatValue
                    = rState.getTableSprms().find(NS_ooxml::LN_CT_Lvl_start);
                if (pStartatValue)
                    aLevelSprms.set(NS_ooxml::LN_CT_Lvl_start, pStartatValue);

                auto pLeveltextValue = new RTFValue(aLeveltextAttributes);
                aLevelSprms.set(NS_ooxml::LN_CT_Lvl_lvlText, pLeveltextValue);
                RTFValue::Pointer_t pRunProps
                    = rState.getTableSprms().find(NS_ooxml::LN_CT_Lvl_rPr);
                if (pRunProps)
                    aLevelSprms.set(NS_ooxml::LN_CT_Lvl_rPr, pRunProps);

                RTFSprms aAbstractAttributes;
                RTFSprms aAbstractSprms;
                aAbstractAttributes.set(NS_ooxml::LN_CT_AbstractNum_abstractNumId, pIdValue);
                auto pLevelValue = new RTFValue(aLevelAttributes, aLevelSprms);
                aAbstractSprms.set(NS_ooxml::LN_CT_AbstractNum_lvl, pLevelValue,
                                   RTFOverwrite::NO_APPEND);

                RTFSprms aListTableSprms;
                auto pAbstractValue = new RTFValue(aAbstractAttributes, aAbstractSprms);
                // It's important that Numbering_abstractNum and Numbering_num never overwrites previous values.
                aListTableSprms.set(NS_ooxml::LN_CT_Numbering_abstractNum, pAbstractValue,
                                    RTFOverwrite::NO_APPEND);

                // Numbering
                RTFSprms aNumberingAttributes;
                RTFSprms aNumberingSprms;
                aNumberingAttributes.set(NS_ooxml::LN_CT_AbstractNum_nsid, pIdValue);
                aNumberingSprms.set(NS_ooxml::LN_CT_Num_abstractNumId, pIdValue);
                auto pNumberingValue = new RTFValue(aNumberingAttributes, aNumberingSprms);
                aListTableSprms.set(NS_ooxml::LN_CT_Numbering_num, pNumberingValue,
                                    RTFOverwrite::NO_APPEND);

                // Table
                RTFSprms aListTableAttributes;
                writerfilter::Reference<Properties>::Pointer_t pProp
                    = new RTFReferenceProperties(aListTableAttributes, aListTableSprms);

                RTFReferenceTable::Entries_t aListTableEntries;
                aListTableEntries.insert(std::make_pair(0, pProp));
                writerfilter::Reference<Table>::Pointer_t const pTable(
                    new RTFReferenceTable(aListTableEntries));
                Mapper().table(NS_ooxml::LN_NUMBERING, pTable);

                // Use it
                putNestedSprm(m_aStates.top().getParagraphSprms(), NS_ooxml::LN_CT_PPrBase_numPr,
                              NS_ooxml::LN_CT_NumPr_ilvl, pIlvlValue);
                putNestedSprm(m_aStates.top().getParagraphSprms(), NS_ooxml::LN_CT_PPrBase_numPr,
                              NS_ooxml::LN_CT_NumPr_numId, pIdValue);
            }
        }
        break;
        case Destination::PARAGRAPHNUMBERING_TEXTAFTER:
            if (!m_aStates.empty())
            {
                // FIXME: don't use pDestinationText, points to popped state
                auto pValue = new RTFValue(rState.getDestinationText().makeStringAndClear(), true);
                m_aStates.top().getTableAttributes().set(NS_ooxml::LN_CT_LevelSuffix_val, pValue);
            }
            break;
        case Destination::PARAGRAPHNUMBERING_TEXTBEFORE:
            if (!m_aStates.empty())
            {
                // FIXME: don't use pDestinationText, points to popped state
                auto pValue = new RTFValue(rState.getDestinationText().makeStringAndClear(), true);
                m_aStates.top().getTableAttributes().set(NS_ooxml::LN_CT_LevelText_val, pValue);
            }
            break;
        case Destination::LISTNAME:
            break;
        case Destination::LISTLEVEL:
            if (!m_aStates.empty())
            {
                auto pInnerValue = new RTFValue(m_aStates.top().getListLevelNum()++);
                rState.getTableAttributes().set(NS_ooxml::LN_CT_Lvl_ilvl, pInnerValue);

                auto pValue = new RTFValue(rState.getTableAttributes(), rState.getTableSprms());
                if (m_aStates.top().getDestination() != Destination::LFOLEVEL)
                    m_aStates.top().getListLevelEntries().set(NS_ooxml::LN_CT_AbstractNum_lvl,
                                                              pValue, RTFOverwrite::NO_APPEND);
                else
                    m_aStates.top().getTableSprms().set(NS_ooxml::LN_CT_NumLvl_lvl, pValue);
            }
            break;
        case Destination::LFOLEVEL:
            if (!m_aStates.empty())
            {
                auto pInnerValue = new RTFValue(m_aStates.top().getListLevelNum()++);
                rState.getTableAttributes().set(NS_ooxml::LN_CT_NumLvl_ilvl, pInnerValue);

                auto pValue = new RTFValue(rState.getTableAttributes(), rState.getTableSprms());
                m_aStates.top().getTableSprms().set(NS_ooxml::LN_CT_Num_lvlOverride, pValue,
                                                    RTFOverwrite::NO_APPEND);
            }
            break;
        // list override table
        case Destination::LISTOVERRIDEENTRY:
            if (!m_aStates.empty())
            {
                if (m_aStates.top().getDestination() == Destination::LISTOVERRIDEENTRY)
                {
                    // copy properties upwards so upper popState() inserts it
                    m_aStates.top().getTableAttributes() = rState.getTableAttributes();
                    m_aStates.top().getTableSprms() = rState.getTableSprms();
                }
                else
                {
                    auto pValue = new RTFValue(rState.getTableAttributes(), rState.getTableSprms());
                    m_aListTableSprms.set(NS_ooxml::LN_CT_Numbering_num, pValue,
                                          RTFOverwrite::NO_APPEND);
                    m_aListOverrideTable[rState.getCurrentListOverrideIndex()]
                        = rState.getCurrentListIndex();
                }
            }
            break;
        case Destination::LEVELTEXT:
            if (!m_aStates.empty())
            {
                auto pValue = new RTFValue(rState.getTableAttributes());
                m_aStates.top().getTableSprms().set(NS_ooxml::LN_CT_Lvl_lvlText, pValue);
            }
            break;
        case Destination::LEVELNUMBERS:
            if (!m_aStates.empty())
            {
                m_aStates.top().getTableSprms() = rState.getTableSprms();
                if (m_aStates.top().getDestination() == Destination::LEVELNUMBERS
                    || m_aStates.top().getDestination() == Destination::LISTLEVEL)
                    // Parent state is level number or list level, current state is
                    // level numbers: mark parent as invalid as well if necessary.
                    m_aStates.top().setLevelNumbersValid(rState.getLevelNumbersValid());
            }
            break;
        case Destination::FIELDINSTRUCTION:
            if (!m_aStates.empty())
                m_aStates.top().setFieldStatus(RTFFieldStatus::INSTRUCTION);
            break;
        case Destination::FIELDRESULT:
            if (!m_aStates.empty())
                m_aStates.top().setFieldStatus(RTFFieldStatus::RESULT);
            break;
        case Destination::FIELD:
            if (rState.getFieldStatus() == RTFFieldStatus::INSTRUCTION)
                singleChar(cFieldEnd);
            break;
        case Destination::SHAPEPROPERTYVALUEPICT:
            if (!m_aStates.empty())
            {
                m_aStates.top().getPicture() = rState.getPicture();
                // both \sp and \sv are destinations, copy the text up-ward for later
                m_aStates.top().getDestinationText() = rState.getDestinationText();
            }
            break;
        case Destination::FALT:
            if (!m_aStates.empty())
                m_aStates.top().getTableSprms() = rState.getTableSprms();
            break;
        case Destination::SHAPEPROPERTYNAME:
        case Destination::SHAPEPROPERTYVALUE:
        case Destination::SHAPEPROPERTY:
            if (!m_aStates.empty())
            {
                m_aStates.top().getShape() = rState.getShape();
                m_aStates.top().getPicture() = rState.getPicture();
                m_aStates.top().getCharacterAttributes() = rState.getCharacterAttributes();
            }
            break;
        case Destination::SHAPEINSTRUCTION:
            if (!m_aStates.empty()
                && m_aStates.top().getDestination() == Destination::SHAPEINSTRUCTION)
            {
                // Shape instruction inside other shape instruction: just copy new shape settings:
                // it will be resolved on end of topmost shape instruction destination
                m_aStates.top().getShape() = rState.getShape();
                m_aStates.top().getPicture() = rState.getPicture();
                m_aStates.top().getCharacterSprms() = rState.getCharacterSprms();
                m_aStates.top().getCharacterAttributes() = rState.getCharacterAttributes();
            }
            break;
        case Destination::FLYMAINCONTENT:
        case Destination::SHPPICT:
        case Destination::SHAPE:
            if (!m_aStates.empty())
            {
                m_aStates.top().getFrame() = rState.getFrame();
                if (rState.getDestination() == Destination::SHPPICT
                    && m_aStates.top().getDestination() == Destination::LISTPICTURE)
                {
                    RTFSprms aAttributes;
                    aAttributes.set(NS_ooxml::LN_CT_NumPicBullet_numPicBulletId,
                                    new RTFValue(m_nListPictureId++));
                    RTFSprms aSprms;
                    // Dummy value, real picture is already sent to dmapper.
                    aSprms.set(NS_ooxml::LN_CT_NumPicBullet_pict, new RTFValue(0));
                    auto pValue = new RTFValue(aAttributes, aSprms);
                    m_aListTableSprms.set(NS_ooxml::LN_CT_Numbering_numPicBullet, pValue,
                                          RTFOverwrite::NO_APPEND);
                }
            }
            break;
        case Destination::SHAPETEXT:
            if (!m_aStates.empty())
            {
                // If we're leaving the shapetext group (it may have nested ones) and this is a shape, not an old drawingobject.
                if (m_aStates.top().getDestination() != Destination::SHAPETEXT
                    && !m_aStates.top().getDrawingObject().getHadShapeText())
                {
                    m_aStates.top().setHadShapeText(true);
                    if (!m_aStates.top().getCurrentBuffer())
                        m_pSdrImport->close();
                    else
                        m_aStates.top().getCurrentBuffer()->push_back(
                            Buf_t(BUFFER_ENDSHAPE, nullptr, nullptr));
                }

                // It's allowed to declare these inside the shape text, and they
                // are expected to have an effect for the whole shape.
                if (rState.getDrawingObject().getLeft())
                    m_aStates.top().getDrawingObject().setLeft(rState.getDrawingObject().getLeft());
                if (rState.getDrawingObject().getTop())
                    m_aStates.top().getDrawingObject().setTop(rState.getDrawingObject().getTop());
                if (rState.getDrawingObject().getRight())
                    m_aStates.top().getDrawingObject().setRight(
                        rState.getDrawingObject().getRight());
                if (rState.getDrawingObject().getBottom())
                    m_aStates.top().getDrawingObject().setBottom(
                        rState.getDrawingObject().getBottom());
            }
            break;
        case Destination::PROPNAME:
            if (m_aStates.top().getDestination() == Destination::USERPROPS)
                m_aStates.top().setPropName(rState.getPropName());
            break;
        default:
        {
            if (!m_aStates.empty() && m_aStates.top().getDestination() == Destination::PICT)
                m_aStates.top().getPicture() = rState.getPicture();
        }
        break;
    }
}

RTFError RTFDocumentImpl::popState()
{
    //SAL_INFO("writerfilter", __func__ << " before pop: m_pTokenizer->getGroup() " << m_pTokenizer->getGroup() <<
    //                         ", dest state: " << m_aStates.top().eDestination);

    checkUnicode(/*bUnicode =*/true, /*bHex =*/true);
    RTFParserState aState(m_aStates.top());
    m_bWasInFrame = aState.getFrame().inFrame();

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

    RTFError eError = beforePopState(aState);
    if (eError != RTFError::OK)
        return eError;

    // See if we need to end a track change
    if (aState.getStartedTrackchange())
    {
        RTFSprms aTCSprms;
        auto pValue = new RTFValue(0);
        aTCSprms.set(NS_ooxml::LN_endtrackchange, pValue);
        if (!m_aStates.top().getCurrentBuffer())
            Mapper().props(new RTFReferenceProperties(RTFSprms(), aTCSprms));
        else
            bufferProperties(*m_aStates.top().getCurrentBuffer(),
                             new RTFValue(RTFSprms(), aTCSprms), nullptr);
    }

    // This is the end of the doc, see if we need to close the last section.
    if (m_pTokenizer->getGroup() == 1 && !m_bFirstRun)
    {
        // \par means an empty paragraph at the end of footnotes/endnotes, but
        // not in case of other substreams, like headers.
        if (m_bNeedCr && m_nStreamType != NS_ooxml::LN_footnote
            && m_nStreamType != NS_ooxml::LN_endnote && m_bIsNewDoc)
            dispatchSymbol(RTF_PAR);
        if (m_bNeedSect) // may be set by dispatchSymbol above!
            sectBreak(true);
    }

    m_aStates.pop();

    m_pTokenizer->popGroup();

    afterPopState(aState);

    if (aState.getCurrentBuffer() == &m_aSuperBuffer)
    {
        OSL_ASSERT(!m_aStates.empty() && m_aStates.top().getCurrentBuffer() == nullptr);

        if (!m_aSuperBuffer.empty())
            replayBuffer(m_aSuperBuffer, nullptr, nullptr);
    }

    if (!m_aStates.empty() && m_aStates.top().getTableRowWidthAfter() > 0
        && aState.getTableRowWidthAfter() == 0)
        // An RTF_ROW in the inner group already parsed nTableRowWidthAfter,
        // don't do it again in the outer state later.
        m_aStates.top().setTableRowWidthAfter(0);

    if (m_nResetBreakOnSectBreak != RTF_invalid && !m_aStates.empty())
    {
        // Section break type created for \page still has an effect in the
        // outer state as well.
        RTFValue::Pointer_t pType
            = aState.getSectionSprms().find(NS_ooxml::LN_EG_SectPrContents_type);
        if (pType)
            m_aStates.top().getSectionSprms().set(NS_ooxml::LN_EG_SectPrContents_type, pType);
    }

    return RTFError::OK;
}

RTFError RTFDocumentImpl::handleEmbeddedObject()
{
    OString aStr
        = OUStringToOString(m_aStates.top().getCurrentDestinationText()->makeStringAndClear(),
                            RTL_TEXTENCODING_ASCII_US);
    std::unique_ptr<SvStream> pStream(new SvMemoryStream());
    if (!msfilter::rtfutil::ExtractOLE2FromObjdata(aStr, *pStream))
        return RTFError::HEX_INVALID;

    uno::Reference<io::XInputStream> xInputStream(
        new utl::OSeekableInputStreamWrapper(pStream.release(), /*_bOwner=*/true));
    auto pStreamValue = new RTFValue(xInputStream);
    m_aOLEAttributes.set(NS_ooxml::LN_inputstream, pStreamValue);

    return RTFError::OK;
}

bool RTFDocumentImpl::isInBackground() { return m_aStates.top().getInBackground(); }

RTFInternalState RTFDocumentImpl::getInternalState() { return m_aStates.top().getInternalState(); }

void RTFDocumentImpl::setInternalState(RTFInternalState nInternalState)
{
    m_aStates.top().setInternalState(nInternalState);
}

Destination RTFDocumentImpl::getDestination() { return m_aStates.top().getDestination(); }

void RTFDocumentImpl::setDestination(Destination eDestination)
{
    m_aStates.top().setDestination(eDestination);
}

// this is a questionably named method that is used only in a very special
// situation where it looks like the "current" buffer is needed?
void RTFDocumentImpl::setDestinationText(OUString const& rString)
{
    m_aStates.top().getDestinationText().setLength(0);
    m_aStates.top().getDestinationText().append(rString);
}

bool RTFDocumentImpl::getSkipUnknown() { return m_bSkipUnknown; }

void RTFDocumentImpl::setSkipUnknown(bool bSkipUnknown) { m_bSkipUnknown = bSkipUnknown; }

static auto FilterControlChars(Destination const destination, OUString const& rString) -> OUString
{
    if (destination == Destination::LEVELNUMBERS || destination == Destination::LEVELTEXT)
    { // control characters are magic here!
        return rString;
    }
    OUStringBuffer buf(rString.getLength());
    for (sal_Int32 i = 0; i < rString.getLength(); ++i)
    {
        sal_Unicode const ch(rString[i]);
        if (!linguistic::IsControlChar(ch) || ch == '\r' || ch == '\n' || ch == '\t')
        {
            buf.append(ch);
        }
        else
        {
            SAL_INFO("writerfilter.rtf", "filtering control character");
        }
    }
    return buf.makeStringAndClear();
}

void RTFDocumentImpl::checkUnicode(bool bUnicode, bool bHex)
{
    if (bUnicode && !m_aUnicodeBuffer.isEmpty())
    {
        OUString aString = m_aUnicodeBuffer.toString();
        m_aUnicodeBuffer.setLength(0);
        aString = FilterControlChars(m_aStates.top().getDestination(), aString);
        text(aString);
    }
    if (bHex && !m_aHexBuffer.isEmpty())
    {
        rtl_TextEncoding nEncoding = m_aStates.top().getCurrentEncoding();
        if (m_aStates.top().getDestination() == Destination::FONTENTRY
            && m_aStates.top().getCurrentEncoding() == RTL_TEXTENCODING_SYMBOL)
            nEncoding = RTL_TEXTENCODING_MS_1252;
        OUString aString = OStringToOUString(m_aHexBuffer.toString(), nEncoding);
        m_aHexBuffer.setLength(0);
        aString = FilterControlChars(m_aStates.top().getDestination(), aString);
        text(aString);
    }
}

RTFParserState::RTFParserState(RTFDocumentImpl* pDocumentImpl)
    : m_pDocumentImpl(pDocumentImpl)
    , m_nInternalState(RTFInternalState::NORMAL)
    , m_eDestination(Destination::NORMAL)
    , m_eFieldStatus(RTFFieldStatus::NONE)
    , m_nBorderState(RTFBorderState::NONE)
    , m_nCurrentEncoding(rtl_getTextEncodingFromWindowsCharset(0))
    , m_nUc(1)
    , m_nCharsToSkip(0)
    , m_nBinaryToRead(0)
    , m_nListLevelNum(0)
    , m_bLevelNumbersValid(true)
    , m_aFrame(this)
    , m_eRunType(RunType::NONE)
    , m_nYear(0)
    , m_nMonth(0)
    , m_nDay(0)
    , m_nHour(0)
    , m_nMinute(0)
    , m_pCurrentDestinationText(nullptr)
    , m_nCurrentStyleIndex(-1)
    , m_nCurrentCharacterStyleIndex(-1)
    , m_pCurrentBuffer(nullptr)
    , m_bInListpicture(false)
    , m_bInBackground(false)
    , m_bHadShapeText(false)
    , m_bInShapeGroup(false)
    , m_bInShape(false)
    , m_bCreatedShapeGroup(false)
    , m_bStartedTrackchange(false)
    , m_nTableRowWidthAfter(0)
{
}

void RTFDocumentImpl::resetFrame() { m_aStates.top().getFrame() = RTFFrame(&m_aStates.top()); }

void RTFDocumentImpl::bufferProperties(RTFBuffer_t& rBuffer, const RTFValue::Pointer_t& pValue,
                                       const tools::SvRef<TableRowBuffer>& pTableProperties)
{
    rBuffer.emplace_back(
        Buf_t(BUFFER_SETSTYLE, new RTFValue(m_aStates.top().getCurrentStyleIndex()), nullptr));
    rBuffer.emplace_back(Buf_t(BUFFER_PROPS, pValue, pTableProperties));
}

RTFShape::RTFShape() = default;

RTFDrawingObject::RTFDrawingObject() = default;

RTFFrame::RTFFrame(RTFParserState* pParserState)
    : m_pDocumentImpl(pParserState->getDocumentImpl())
    , m_nX(0)
    , m_nY(0)
    , m_nW(0)
    , m_nH(0)
    , m_nHoriPadding(0)
    , m_nVertPadding(0)
    , m_nHoriAlign(0)
    , m_nHoriAnchor(0)
    , m_nVertAlign(0)
    , m_nVertAnchor(0)
    , m_nHRule(NS_ooxml::LN_Value_doc_ST_HeightRule_auto)
{
}

void RTFFrame::setSprm(Id nId, Id nValue)
{
    if (m_pDocumentImpl->getFirstRun() && !m_pDocumentImpl->isStyleSheetImport())
    {
        m_pDocumentImpl->checkFirstRun();
        m_pDocumentImpl->setNeedPar(false);
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

    static const Id pNames[]
        = { NS_ooxml::LN_CT_FramePr_x,       NS_ooxml::LN_CT_FramePr_y,
            NS_ooxml::LN_CT_FramePr_hRule, // Make sure nHRule is processed before nH
            NS_ooxml::LN_CT_FramePr_h,       NS_ooxml::LN_CT_FramePr_w,
            NS_ooxml::LN_CT_FramePr_hSpace,  NS_ooxml::LN_CT_FramePr_vSpace,
            NS_ooxml::LN_CT_FramePr_hAnchor, NS_ooxml::LN_CT_FramePr_vAnchor,
            NS_ooxml::LN_CT_FramePr_xAlign,  NS_ooxml::LN_CT_FramePr_yAlign,
            NS_ooxml::LN_CT_FramePr_wrap,    NS_ooxml::LN_CT_FramePr_dropCap,
            NS_ooxml::LN_CT_FramePr_lines };

    for (Id nId : pNames)
    {
        RTFValue::Pointer_t pValue;

        switch (nId)
        {
            case NS_ooxml::LN_CT_FramePr_x:
                if (m_nX != 0)
                    pValue = new RTFValue(m_nX);
                break;
            case NS_ooxml::LN_CT_FramePr_y:
                if (m_nY != 0)
                    pValue = new RTFValue(m_nY);
                break;
            case NS_ooxml::LN_CT_FramePr_h:
                if (m_nH != 0)
                {
                    if (m_nHRule == NS_ooxml::LN_Value_doc_ST_HeightRule_exact)
                        pValue = new RTFValue(-m_nH); // The negative value just sets nHRule
                    else
                        pValue = new RTFValue(m_nH);
                }
                break;
            case NS_ooxml::LN_CT_FramePr_w:
                if (m_nW != 0)
                    pValue = new RTFValue(m_nW);
                break;
            case NS_ooxml::LN_CT_FramePr_hSpace:
                if (m_nHoriPadding != 0)
                    pValue = new RTFValue(m_nHoriPadding);
                break;
            case NS_ooxml::LN_CT_FramePr_vSpace:
                if (m_nVertPadding != 0)
                    pValue = new RTFValue(m_nVertPadding);
                break;
            case NS_ooxml::LN_CT_FramePr_hAnchor:
            {
                if (m_nHoriAnchor == 0)
                    m_nHoriAnchor = NS_ooxml::LN_Value_doc_ST_HAnchor_margin;
                pValue = new RTFValue(m_nHoriAnchor);
            }
            break;
            case NS_ooxml::LN_CT_FramePr_vAnchor:
            {
                if (m_nVertAnchor == 0)
                    m_nVertAnchor = NS_ooxml::LN_Value_doc_ST_VAnchor_margin;
                pValue = new RTFValue(m_nVertAnchor);
            }
            break;
            case NS_ooxml::LN_CT_FramePr_xAlign:
                pValue = new RTFValue(m_nHoriAlign);
                break;
            case NS_ooxml::LN_CT_FramePr_yAlign:
                pValue = new RTFValue(m_nVertAlign);
                break;
            case NS_ooxml::LN_CT_FramePr_hRule:
            {
                if (m_nH < 0)
                    m_nHRule = NS_ooxml::LN_Value_doc_ST_HeightRule_exact;
                else if (m_nH > 0)
                    m_nHRule = NS_ooxml::LN_Value_doc_ST_HeightRule_atLeast;
                pValue = new RTFValue(m_nHRule);
            }
            break;
            case NS_ooxml::LN_CT_FramePr_wrap:
                if (m_oWrap)
                    pValue = new RTFValue(*m_oWrap);
                break;
            default:
                break;
        }

        if (pValue)
            sprms.set(nId, pValue);
    }

    RTFSprms frameprSprms;
    frameprSprms.set(NS_ooxml::LN_CT_PPrBase_framePr, new RTFValue(sprms));
    return frameprSprms;
}

bool RTFFrame::hasProperties() const
{
    return m_nX != 0 || m_nY != 0 || m_nW != 0 || m_nH != 0 || m_nHoriPadding != 0
           || m_nVertPadding != 0 || m_nHoriAlign != 0 || m_nHoriAnchor != 0 || m_nVertAlign != 0
           || m_nVertAnchor != 0;
}

} // namespace writerfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
