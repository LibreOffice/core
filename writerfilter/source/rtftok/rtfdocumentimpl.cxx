/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *       Miklos Vajna <vmiklos@frugalware.org>
 * Portions created by the Initial Developer are Copyright (C) 2011 the
 * Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include <rtfdocumentimpl.hxx>
#include <rtftypes.hxx>
#include <rtfcontrolwords.hxx>
#include <rtfvalue.hxx>
#include <rtfsprm.hxx>
#include <rtfreferenceproperties.hxx>
#include <doctok/sprmids.hxx> // NS_sprm namespace
#include <doctok/resourceids.hxx> // NS_rtf namespace
#include <ooxml/resourceids.hxx> // NS_ooxml namespace
#include <ooxml/OOXMLFastTokens.hxx> // ooxml namespace
#include <unotools/ucbstreamhelper.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/tencinfo.h>
#include <svl/lngmisc.hxx>
#include <editeng/borderline.hxx>
#include <unotools/streamwrap.hxx>
#include <svx/msdffdef.hxx>
#include <com/sun/star/drawing/XEnhancedCustomShapeDefaulter.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/text/WritingMode.hpp>

#define TWIP_TO_MM100(TWIP)     ((TWIP) >= 0 ? (((TWIP)*127L+36L)/72L) : (((TWIP)*127L-36L)/72L))

using std::make_pair;
using rtl::OString;
using rtl::OStringBuffer;
using rtl::OUString;
using rtl::OUStringBuffer;
using rtl::OUStringToOString;

namespace writerfilter {
namespace rtftok {

static RTFSprms_t& lcl_getNumPr(std::stack<RTFParserState>& aStates)
{
    // insert the numpr sprm if necessary
    RTFValue::Pointer_t p = RTFSprm::find(aStates.top().aParagraphSprms, NS_ooxml::LN_CT_PPrBase_numPr);
    if (!p.get())
    {
        RTFSprms_t aAttributes;
        RTFSprms_t aSprms;
        RTFValue::Pointer_t pValue(new RTFValue(aAttributes, aSprms));
        aStates.top().aParagraphSprms.push_back(make_pair(NS_ooxml::LN_CT_PPrBase_numPr, pValue));
        p = RTFSprm::find(aStates.top().aParagraphSprms, NS_ooxml::LN_CT_PPrBase_numPr);
    }
    return p->getSprms();
}

static Id lcl_getParagraphBorder(sal_uInt32 nIndex)
{
    static const Id aBorderIds[] =
    {
        NS_sprm::LN_PBrcTop, NS_sprm::LN_PBrcLeft, NS_sprm::LN_PBrcBottom, NS_sprm::LN_PBrcRight
    };

    return aBorderIds[nIndex];
}

static void lcl_putNestedAttribute(RTFSprms_t& rSprms, Id nParent, Id nId, RTFValue::Pointer_t pValue,
        bool bOverwrite = false, bool bAttribute = true)
{
    RTFValue::Pointer_t pParent = RTFSprm::find(rSprms, nParent);
    if (!pParent.get())
    {
        RTFSprms_t aAttributes;
        RTFValue::Pointer_t pParentValue(new RTFValue(aAttributes));
        rSprms.push_back(make_pair(nParent, pParentValue));
        pParent = pParentValue;
    }
    RTFSprms_t& rAttributes = (bAttribute ? pParent->getAttributes() : pParent->getSprms());
    if (bOverwrite)
        for (RTFSprms_t::iterator i = rAttributes.begin(); i != rAttributes.end(); ++i)
            if (i->first == nId)
            {
                i->second = pValue;
                return;
            }
    rAttributes.push_back(make_pair(nId, pValue));
}

static void lcl_putNestedSprm(RTFSprms_t& rSprms, Id nParent, Id nId, RTFValue::Pointer_t pValue, bool bOverwrite = false)
{
    lcl_putNestedAttribute(rSprms, nParent, nId, pValue, bOverwrite, false);
}

static RTFSprms_t& lcl_getLastAttributes(RTFSprms_t& rSprms, Id nId)
{
    RTFValue::Pointer_t p = RTFSprm::find(rSprms, nId);
    if (p->getSprms().size())
        return p->getSprms().back().second->getAttributes();
    else
    {
        OSL_FAIL("trying to set property when no type is defined");
        return p->getSprms();
    }
}

static void lcl_putBorderProperty(std::stack<RTFParserState>& aStates, Id nId, RTFValue::Pointer_t pValue)
{
    if (aStates.top().nBorderState == BORDER_PARAGRAPH)
        for (int i = 0; i < 4; i++)
        {
            RTFValue::Pointer_t p = RTFSprm::find(aStates.top().aParagraphSprms, lcl_getParagraphBorder(i));
            if (p.get())
            {
                RTFSprms_t& rAttributes = p->getAttributes();
                rAttributes.push_back(make_pair(nId, pValue));
            }
        }
    else if (aStates.top().nBorderState == BORDER_CELL)
    {
        // Attributes of the last border type
        RTFSprms_t& rAttributes = lcl_getLastAttributes(aStates.top().aTableCellSprms, NS_ooxml::LN_CT_TcPrBase_tcBorders);
        rAttributes.push_back(make_pair(nId, pValue));
    }
    else if (aStates.top().nBorderState == BORDER_PAGE)
    {
        // Attributes of the last border type
        RTFSprms_t& rAttributes = lcl_getLastAttributes(aStates.top().aSectionSprms, NS_ooxml::LN_EG_SectPrContents_pgBorders);
        rAttributes.push_back(make_pair(nId, pValue));
    }
}

static void lcl_Break(Stream& rMapper)
{
    sal_uInt8 sBreak[] = { 0xd };
    rMapper.text(sBreak, 1);
}

static void lcl_TableBreak(Stream& rMapper)
{
    lcl_Break(rMapper);
    rMapper.endParagraphGroup();
    rMapper.startParagraphGroup();
}

// NEEDSWORK: DocxAttributeOutput's impl_AppendTwoDigits does the same.
static void lcl_AppendTwoDigits(OStringBuffer &rBuffer, sal_Int32 nNum)
{
    if ( nNum < 0 || nNum > 99 )
    {
        rBuffer.append( "00" );
        return;
    }

    if ( nNum < 10 )
        rBuffer.append( '0' );

    rBuffer.append( nNum );
}

// NEEDSWORK: sw::ms::DTTM2DateTime and DocxAttributeOutput's
// impl_DateTimeToOString could be combined to do the same.
static OString lcl_DTTM22OString(long lDTTM)
{
    sal_uInt16 lMin = (sal_uInt16)(lDTTM & 0x0000003F);
    lDTTM >>= 6;
    sal_uInt16 lHour= (sal_uInt16)(lDTTM & 0x0000001F);
    lDTTM >>= 5;
    sal_uInt16 lDay = (sal_uInt16)(lDTTM & 0x0000001F);
    lDTTM >>= 5;
    sal_uInt16 lMon = (sal_uInt16)(lDTTM & 0x0000000F);
    lDTTM >>= 4;
    sal_uInt16 lYear= (sal_uInt16)(lDTTM & 0x000001FF) + 1900;

    OStringBuffer aBuf;
    aBuf.append(sal_Int32(lYear));
    aBuf.append('-');
    lcl_AppendTwoDigits(aBuf, lMon);
    aBuf.append('-');
    lcl_AppendTwoDigits(aBuf, lDay);
    aBuf.append('T');
    lcl_AppendTwoDigits(aBuf, lHour);
    aBuf.append(':');
    lcl_AppendTwoDigits(aBuf, lMin);
    aBuf.append(":00Z");
    return aBuf.makeStringAndClear();
}

static writerfilter::Reference<Properties>::Pointer_t lcl_getBookmarkProperties(int nPos, OUString& rString)
{
    RTFSprms_t aAttributes;
    RTFValue::Pointer_t pPos(new RTFValue(nPos));
    if (rString.getLength())
    {
        // If present, this should be sent first.
        RTFValue::Pointer_t pString(new RTFValue(rString));
        aAttributes.push_back(make_pair(NS_rtf::LN_BOOKMARKNAME, pString));
    }
    aAttributes.push_back(make_pair(NS_rtf::LN_IBKL, pPos));
    return writerfilter::Reference<Properties>::Pointer_t(new RTFReferenceProperties(aAttributes));
}

static writerfilter::Reference<Properties>::Pointer_t lcl_getBookmarkProperties(int nPos)
{
    OUString aStr;
    return lcl_getBookmarkProperties(nPos, aStr);
}

static int lcl_AsHex(char ch)
{
    int ret = 0;
    if (isdigit(ch))
        ret = ch - '0';
    else
    {
        if (islower(ch))
        {
            if (ch < 'a' || ch > 'f')
                return -1;
            ret = ch - 'a';
        }
        else
        {
            if (ch < 'A' || ch > 'F')
                return -1;
            ret = ch - 'A';
        }
        ret += 10;
    }
    return ret;
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

// NEEDSWORK: wwUtility::BGRToRGB does the same.
sal_uInt32 lcl_BGRToRGB(sal_uInt32 nColor)
{
    sal_uInt8
        r(static_cast<sal_uInt8>(nColor&0xFF)),
        g(static_cast<sal_uInt8>(((nColor)>>8)&0xFF)),
        b(static_cast<sal_uInt8>((nColor>>16)&0xFF)),
        t(static_cast<sal_uInt8>((nColor>>24)&0xFF));
    nColor = (t<<24) + (r<<16) + (g<<8) + b;
    return nColor;
}

RTFDocumentImpl::RTFDocumentImpl(uno::Reference<uno::XComponentContext> const& xContext,
        uno::Reference<io::XInputStream> const& xInputStream,
        uno::Reference<lang::XComponent> const& xDstDoc,
        uno::Reference<frame::XFrame>    const& xFrame)
    : m_xContext(xContext),
    m_xInputStream(xInputStream),
    m_xDstDoc(xDstDoc),
    m_xFrame(xFrame),
    m_nGroup(0),
    m_aDefaultState(),
    m_bSkipUnknown(false),
    m_aFontEncodings(),
    m_aColorTable(),
    m_bFirstRun(true),
    m_bNeedPap(false),
    m_aListTableSprms(),
    m_aSettingsTableSprms(),
    m_xStorage(),
    m_aTableBuffer(),
    m_aSuperBuffer(),
    m_aShapetextBuffer(),
    m_pCurrentBuffer(0),
    m_bHasFootnote(false),
    m_bIsSubstream(false),
    m_nHeaderFooterPositions(),
    m_nGroupStartPos(0),
    m_aBookmarks(),
    m_aAuthors()
{
    OSL_ASSERT(xInputStream.is());
    m_pInStream = utl::UcbStreamHelper::CreateStream(xInputStream, sal_True);

    m_xModelFactory.set(m_xDstDoc, uno::UNO_QUERY);
    OSL_ASSERT(m_xModelFactory.is());

    m_pGraphicHelper = new oox::GraphicHelper(m_xContext, xFrame, m_xStorage);

    uno::Reference<drawing::XDrawPageSupplier> xDrawings(m_xDstDoc, uno::UNO_QUERY);
    OSL_ASSERT(xDrawings.is());
    m_xDrawPage.set(xDrawings->getDrawPage(), uno::UNO_QUERY);
    OSL_ASSERT(m_xDrawPage.is());
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

void RTFDocumentImpl::setSubstream(bool bIsSubtream)
{
    m_bIsSubstream = bIsSubtream;
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
    RTFDocumentImpl::Pointer_t pImpl(new RTFDocumentImpl(m_xContext, m_xInputStream, m_xDstDoc, m_xFrame));
    pImpl->setSubstream(true);
    pImpl->setIgnoreFirst(rIgnoreFirst);
    pImpl->seek(nPos);
    OSL_TRACE("substream start");
    Mapper().substream(nId, pImpl);
    OSL_TRACE("substream end");
    Strm().Seek(nCurrent);
    nPos = 0;
}

void RTFDocumentImpl::checkFirstRun()
{
    if (m_bFirstRun)
    {
        writerfilter::Reference<Properties>::Pointer_t const pParagraphProperties(
                new RTFReferenceProperties(m_aStates.top().aParagraphAttributes, m_aStates.top().aParagraphSprms)
                );
        // output settings table
        RTFSprms_t aAttributes;
        writerfilter::Reference<Properties>::Pointer_t const pProp(new RTFReferenceProperties(aAttributes, m_aSettingsTableSprms));
        RTFReferenceTable::Entries_t aSettingsTableEntries;
        aSettingsTableEntries.insert(make_pair(0, pProp));
        writerfilter::Reference<Table>::Pointer_t const pTable(new RTFReferenceTable(aSettingsTableEntries));
        Mapper().table(NS_ooxml::LN_settings_settings, pTable);
        // start initial paragraph
        if (!m_bIsSubstream)
            Mapper().startSectionGroup();
        Mapper().startParagraphGroup();
        Mapper().props(pParagraphProperties);
        m_bFirstRun = false;
    }
}

void RTFDocumentImpl::parBreak()
{
    checkFirstRun();
    // end previous paragraph
    Mapper().startCharacterGroup();
    lcl_Break(Mapper());
    Mapper().endCharacterGroup();
    Mapper().endParagraphGroup();

    // start new one
    Mapper().startParagraphGroup();
}

void RTFDocumentImpl::sectBreak(bool bFinal = false)
{
    while (m_nHeaderFooterPositions.size())
    {
        std::pair<Id, sal_uInt32> aPair = m_nHeaderFooterPositions.front();
        m_nHeaderFooterPositions.pop();
        resolveSubstream(aPair.second, aPair.first);
    }

    RTFValue::Pointer_t pBreak = RTFSprm::find(m_aStates.top().aSectionSprms, NS_sprm::LN_SBkc);
    // In case the last section is a continous one, we don't need to output a section break.
    if (bFinal && pBreak.get() && !pBreak->getInt())
        RTFSprm::erase(m_aStates.top().aSectionSprms, NS_sprm::LN_SBkc);

    // Section properties are a paragraph sprm.
    RTFValue::Pointer_t pValue(new RTFValue(m_aStates.top().aSectionAttributes, m_aStates.top().aSectionSprms));
    RTFSprms_t aAttributes;
    RTFSprms_t aSprms;
    aSprms.push_back(make_pair(NS_ooxml::LN_CT_PPr_sectPr, pValue));
    writerfilter::Reference<Properties>::Pointer_t const pProperties(
            new RTFReferenceProperties(aAttributes, aSprms)
            );
    // The trick is that we send properties of the previous section right now, which will be exactly what dmapper expects.
    Mapper().props(pProperties);
    Mapper().endParagraphGroup();
    if (!m_bIsSubstream)
        Mapper().endSectionGroup();
    if (!bFinal)
    {
        Mapper().startSectionGroup();
        Mapper().startParagraphGroup();
    }
}

void RTFDocumentImpl::seek(sal_uInt32 nPos)
{
    Strm().Seek(nPos);
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
        case ERROR_HEX_INVALID:
            OSL_TRACE("%s: invalid hex char", OSL_THIS_FUNC);
            break;
    }
}

int RTFDocumentImpl::resolvePict(bool bInline)
{
    SvMemoryStream aStream;
    int b = 0, count = 2;

    // Feed the destination text to a stream.
    OString aStr = OUStringToOString(m_aDestinationText.makeStringAndClear(), RTL_TEXTENCODING_ASCII_US);
    const char *str = aStr.getStr();
    for (int i = 0; i < aStr.getLength(); ++i)
    {
        char ch = str[i];
        if (ch != 0x0d && ch != 0x0a)
        {
            b = b << 4;
            char parsed = lcl_AsHex(ch);
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

    // Store, and get its URL.
    aStream.Seek(0);
    uno::Reference<io::XInputStream> xInputStream(new utl::OInputStreamWrapper(&aStream));
    OUString aGraphicUrl = m_pGraphicHelper->importGraphicObject(xInputStream);

    // Wrap it in an XShape.
    uno::Reference<drawing::XShape> xShape;
    OUString aService(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.GraphicObjectShape"));
    xShape.set(m_xModelFactory->createInstance(aService), uno::UNO_QUERY);
    OSL_ASSERT(xShape.is());
    uno::Reference<beans::XPropertySet> xPropertySet(xShape, uno::UNO_QUERY);
    OSL_ASSERT(xPropertySet.is());
    xPropertySet->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("GraphicURL")), uno::Any(aGraphicUrl));

    // Send it to the dmapper.
    RTFSprms_t aSprms;
    RTFSprms_t aAttributes;
    // shape attribute
    RTFSprms_t aPicAttributes;
    RTFValue::Pointer_t pShapeValue(new RTFValue(xShape));
    aPicAttributes.push_back(make_pair(NS_ooxml::LN_shape, pShapeValue));
    // pic sprm
    RTFSprms_t aGraphicDataAttributes;
    RTFSprms_t aGraphicDataSprms;
    RTFValue::Pointer_t pPicValue(new RTFValue(aPicAttributes));
    aGraphicDataSprms.push_back(make_pair(NS_ooxml::LN_pic_pic, pPicValue));
    // graphicData sprm
    RTFSprms_t aGraphicAttributes;
    RTFSprms_t aGraphicSprms;
    RTFValue::Pointer_t pGraphicDataValue(new RTFValue(aGraphicDataAttributes, aGraphicDataSprms));
    aGraphicSprms.push_back(make_pair(NS_ooxml::LN_CT_GraphicalObject_graphicData, pGraphicDataValue));
    // graphic sprm
    RTFValue::Pointer_t pGraphicValue(new RTFValue(aGraphicAttributes, aGraphicSprms));
    // extent sprm
    RTFSprms_t aExtentAttributes;
    for (RTFSprms_t::iterator i = m_aStates.top().aCharacterAttributes.begin(); i != m_aStates.top().aCharacterAttributes.end(); ++i)
        if (i->first == NS_rtf::LN_XEXT || i->first == NS_rtf::LN_YEXT)
            aExtentAttributes.push_back(make_pair(i->first, i->second));
    RTFValue::Pointer_t pExtentValue(new RTFValue(aExtentAttributes));
    // docpr sprm
    RTFSprms_t aDocprAttributes;
    for (RTFSprms_t::iterator i = m_aStates.top().aCharacterAttributes.begin(); i != m_aStates.top().aCharacterAttributes.end(); ++i)
        if (i->first == NS_ooxml::LN_CT_NonVisualDrawingProps_name || i->first == NS_ooxml::LN_CT_NonVisualDrawingProps_descr)
            aDocprAttributes.push_back(make_pair(i->first, i->second));
    RTFValue::Pointer_t pDocprValue(new RTFValue(aDocprAttributes));
    if (bInline)
    {
        RTFSprms_t aInlineAttributes;
        RTFSprms_t aInlineSprms;
        aInlineSprms.push_back(make_pair(NS_ooxml::LN_CT_Inline_extent, pExtentValue));
        aInlineSprms.push_back(make_pair(NS_ooxml::LN_CT_Inline_docPr, pDocprValue));
        aInlineSprms.push_back(make_pair(NS_ooxml::LN_graphic_graphic, pGraphicValue));
        // inline sprm
        RTFValue::Pointer_t pValue(new RTFValue(aInlineAttributes, aInlineSprms));
        aSprms.push_back(make_pair(NS_ooxml::LN_inline_inline, pValue));
    }
    else // anchored
    {
        // wrap sprm
        RTFSprms_t aAnchorWrapAttributes;
        for (RTFSprms_t::iterator i = m_aStates.top().aCharacterAttributes.begin(); i != m_aStates.top().aCharacterAttributes.end(); ++i)
            if (i->first == NS_ooxml::LN_CT_WrapSquare_wrapText)
                aAnchorWrapAttributes.push_back(make_pair(i->first, i->second));
        RTFValue::Pointer_t pAnchorWrapValue(new RTFValue(aAnchorWrapAttributes));
        RTFSprms_t aAnchorAttributes;
        RTFSprms_t aAnchorSprms;
        aAnchorSprms.push_back(make_pair(NS_ooxml::LN_CT_Anchor_extent, pExtentValue));
        if (aAnchorWrapAttributes.size())
            aAnchorSprms.push_back(make_pair(NS_ooxml::LN_EG_WrapType_wrapSquare, pAnchorWrapValue));
        aAnchorSprms.push_back(make_pair(NS_ooxml::LN_CT_Anchor_docPr, pDocprValue));
        aAnchorSprms.push_back(make_pair(NS_ooxml::LN_graphic_graphic, pGraphicValue));
        // anchor sprm
        RTFValue::Pointer_t pValue(new RTFValue(aAnchorAttributes, aAnchorSprms));
        aSprms.push_back(make_pair(NS_ooxml::LN_anchor_anchor, pValue));
    }
    writerfilter::Reference<Properties>::Pointer_t const pProperties(new RTFReferenceProperties(aAttributes, aSprms));
    Mapper().props(pProperties);

    return 0;
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
    if (m_aStates.top().nDestinationState == DESTINATION_LEVELNUMBERS)
    {
        if (aStr.toChar() != ';')
            m_aStates.top().aLevelNumbers.push_back(sal_Int32(ch));
        return 0;
    }
    OSL_TRACE("%s: collected '%s'", OSL_THIS_FUNC, aStr.getStr());

    OUString aOUStr(OStringToOUString(aStr, m_aStates.top().nCurrentEncoding));

    if (m_aStates.top().nDestinationState == DESTINATION_COLORTABLE)
    {
        // we hit a ';' at the end of each color entry
        sal_uInt32 color = (m_aStates.top().aCurrentColor.nRed << 16) | ( m_aStates.top().aCurrentColor.nGreen << 8)
            | m_aStates.top().aCurrentColor.nBlue;
        m_aColorTable.push_back(color);
        // set components back to zero
        m_aStates.top().aCurrentColor = RTFColorTableEntry();
    }
    else
        text(aOUStr);

    return 0;
}

void RTFDocumentImpl::text(OUString& rString)
{
    bool bRet = true;
    switch (m_aStates.top().nDestinationState)
    {
        case DESTINATION_FONTENTRY:
        case DESTINATION_STYLEENTRY:
        case DESTINATION_REVISIONENTRY:
            // drop the ; at the end if it's there
            if (rString.endsWithAsciiL(";", 1))
                rString = rString.copy(0, rString.getLength() - 1);
        case DESTINATION_LEVELTEXT:
        case DESTINATION_SHAPEPROPERTYNAME:
        case DESTINATION_SHAPEPROPERTYVALUE:
        case DESTINATION_BOOKMARKEND:
        case DESTINATION_PICT:
        case DESTINATION_SHAPEPROPERTYVALUEPICT:
            m_aDestinationText.append(rString);
            break;
        default: bRet = false; break;
    }
    if (bRet)
        return;

    if (m_aIgnoreFirst.getLength() && m_aIgnoreFirst.equals(rString))
    {
        m_aIgnoreFirst = OUString();
        return;
    }

    writerfilter::Reference<Properties>::Pointer_t const pParagraphProperties(
            new RTFReferenceProperties(m_aStates.top().aParagraphAttributes, m_aStates.top().aParagraphSprms)
            );

    checkFirstRun();
    if (m_bNeedPap)
    {
        if (!m_pCurrentBuffer)
            Mapper().props(pParagraphProperties);
        else
        {
            RTFValue::Pointer_t pValue(new RTFValue(m_aStates.top().aParagraphAttributes, m_aStates.top().aParagraphSprms));
            m_pCurrentBuffer->push_back(make_pair(BUFFER_PROPS, pValue));
        }
        m_bNeedPap = false;
    }

    // Don't return earlier, a bookmark start has to be in a paragraph group.
    if (m_aStates.top().nDestinationState == DESTINATION_BOOKMARKSTART)
    {
        m_aDestinationText.append(rString);
        return;
    }

    if (!m_pCurrentBuffer && m_aStates.top().nDestinationState != DESTINATION_FOOTNOTE)
        Mapper().startCharacterGroup();
    else if (m_pCurrentBuffer)
    {
        RTFValue::Pointer_t pValue;
        m_pCurrentBuffer->push_back(make_pair(BUFFER_STARTRUN, pValue));
    }
    if (m_aStates.top().nDestinationState == DESTINATION_NORMAL
            || m_aStates.top().nDestinationState == DESTINATION_FIELDRESULT
            || m_aStates.top().nDestinationState == DESTINATION_SHAPETEXT)
    {
        if (!m_pCurrentBuffer)
        {
            writerfilter::Reference<Properties>::Pointer_t const pProperties(
                    new RTFReferenceProperties(m_aStates.top().aCharacterAttributes, m_aStates.top().aCharacterSprms)
                    );
            Mapper().props(pProperties);
        }
        else
        {
            RTFValue::Pointer_t pValue(new RTFValue(m_aStates.top().aCharacterAttributes, m_aStates.top().aCharacterSprms));
            m_pCurrentBuffer->push_back(make_pair(BUFFER_PROPS, pValue));
        }
    }
    if (!m_pCurrentBuffer)
        Mapper().utext(reinterpret_cast<sal_uInt8 const*>(rString.getStr()), rString.getLength());
    else
    {
        RTFValue::Pointer_t pValue(new RTFValue(rString));
        m_pCurrentBuffer->push_back(make_pair(BUFFER_UTEXT, pValue));
    }
    if (!m_pCurrentBuffer && m_aStates.top().nDestinationState != DESTINATION_FOOTNOTE)
        Mapper().endCharacterGroup();
    else if(m_pCurrentBuffer)
    {
        RTFValue::Pointer_t pValue;
        m_pCurrentBuffer->push_back(make_pair(BUFFER_ENDRUN, pValue));
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
            m_aStates.top().aTableCellSprms.push_back(make_pair(NS_sprm::LN_PCell, pValue));
            writerfilter::Reference<Properties>::Pointer_t const pTableCellProperties(
                    new RTFReferenceProperties(m_aStates.top().aTableCellAttributes, m_aStates.top().aTableCellSprms)
                    );
            Mapper().props(pTableCellProperties);
            lcl_TableBreak(Mapper());
            break;
        }
        else if (aPair.first == BUFFER_STARTRUN)
            Mapper().startCharacterGroup();
        else if (aPair.first == BUFFER_UTEXT)
        {
            OUString aString(aPair.second->getString());
            Mapper().utext(reinterpret_cast<sal_uInt8 const*>(aString.getStr()), aString.getLength());
        }
        else if (aPair.first == BUFFER_ENDRUN)
            Mapper().endCharacterGroup();
        else if (aPair.first == BUFFER_PAR)
            parBreak();
        else
            OSL_FAIL("should not happen");
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
            {
                sal_uInt8 sFieldStart[] = { 0x13 };
                Mapper().startCharacterGroup();
                Mapper().text(sFieldStart, 1);
                Mapper().endCharacterGroup();
                m_aStates.top().nDestinationState = DESTINATION_FIELDINSTRUCTION;
            }
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
        case RTF_LEVELTEXT:
            m_aStates.top().nDestinationState = DESTINATION_LEVELTEXT;
            break;
        case RTF_LEVELNUMBERS:
            m_aStates.top().nDestinationState = DESTINATION_LEVELNUMBERS;
            break;
        case RTF_SHPPICT:
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
            m_aStates.top().nDestinationState = DESTINATION_SHAPE;
            break;
        case RTF_SHPINST:
            m_aStates.top().nDestinationState = DESTINATION_SHAPEINSTRUCTION;
            break;
        case RTF_NESTTABLEPROPS:
            m_aStates.top().nDestinationState = DESTINATION_NESTEDTABLEPROPERTIES;
            break;
        case RTF_HEADER:
        case RTF_FOOTER:
        case RTF_HEADERL:
        case RTF_HEADERR:
        case RTF_HEADERF:
        case RTF_FOOTERL:
        case RTF_FOOTERR:
        case RTF_FOOTERF:
            if (!m_bIsSubstream)
            {
                Id nId = 0;
                sal_uInt32 nPos = m_nGroupStartPos - 1;
                switch (nKeyword)
                {
                    case RTF_HEADER: nId = NS_rtf::LN_headerr; break;
                    case RTF_FOOTER: nId = NS_rtf::LN_footerr; break;
                    case RTF_HEADERL: nId = NS_rtf::LN_headerl; break;
                    case RTF_HEADERR: nId = NS_rtf::LN_headerr; break;
                    case RTF_HEADERF: nId = NS_rtf::LN_headerr; break; // TODO figure out how to use NS_rtf::LN_headerf
                    case RTF_FOOTERL: nId = NS_rtf::LN_footerl; break;
                    case RTF_FOOTERR: nId = NS_rtf::LN_footerr; break;
                    case RTF_FOOTERF: nId = NS_rtf::LN_footerr; break; // same here, NS_rtf::LN_footerf could be used
                    default: break;
                }
                m_nHeaderFooterPositions.push(make_pair(nId, nPos));
                m_aStates.top().nDestinationState = DESTINATION_SKIP;
            }
            break;
        case RTF_FOOTNOTE:
            if (!m_bIsSubstream)
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
                m_pCurrentBuffer = 0;
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
                    m_aStates.top().aCharacterAttributes.push_back(make_pair(NS_ooxml::LN_CT_FtnEdnRef_customMarkFollows, pValue));
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
            if (!m_bIsSubstream)
            {
                resolveSubstream(m_nGroupStartPos - 1, NS_rtf::LN_annotation);
                m_aStates.top().nDestinationState = DESTINATION_SKIP;
            }
            break;
        case RTF_SHPTXT:
            m_aStates.top().nDestinationState = DESTINATION_SHAPETEXT;
            dispatchFlag(RTF_PARD);
            m_bNeedPap = true;
            OSL_ENSURE(!m_aShapetextBuffer.size(), "shapetext buffer is not empty");
            m_pCurrentBuffer = &m_aShapetextBuffer;
            break;
        case RTF_LISTTEXT:
            // Should be ignored by any reader that understands Word 97 through Word 2007 numbering.
        case RTF_NONESTTABLES:
            // This destination should be ignored by readers that support nested tables.
            m_aStates.top().nDestinationState = DESTINATION_SKIP;
            break;
        default:
            OSL_TRACE("%s: TODO handle destination '%s'", OSL_THIS_FUNC, lcl_RtfToString(nKeyword));
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
    sal_uInt8 cCh = 0;

    // Trivial symbols
    switch (nKeyword)
    {
        case RTF_LINE: cCh = '\n'; break;
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
        skipDestination(bParsed);
        return 0;
    }

    switch (nKeyword)
    {
        case RTF_IGNORE:
            m_bSkipUnknown = true;
            return 0; // don't reset m_bSkipUnknown after this keyword
            break;
        case RTF_PAR:
            {
                if (!m_pCurrentBuffer)
                    parBreak();
                else if (m_aStates.top().nDestinationState != DESTINATION_SHAPETEXT)
                {
                    RTFValue::Pointer_t pValue;
                    m_pCurrentBuffer->push_back(make_pair(BUFFER_PAR, pValue));
                }
                // but don't emit properties yet, since they may change till the first text token arrives
                m_bNeedPap = true;
            }
            break;
        case RTF_SECT:
                sectBreak();
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
                if (m_bNeedPap)
                {
                    // There were no runs in the cell, so we need to send paragraph properties here.
                    RTFValue::Pointer_t pValue(new RTFValue(m_aStates.top().aParagraphAttributes, m_aStates.top().aParagraphSprms));
                    m_aTableBuffer.push_back(make_pair(BUFFER_PROPS, pValue));
                }

                RTFValue::Pointer_t pValue;
                m_aTableBuffer.push_back(make_pair(BUFFER_CELLEND, pValue));
                m_bNeedPap = true;
            }
            break;
        case RTF_ROW:
        case RTF_NESTROW:
            {
                writerfilter::Reference<Properties>::Pointer_t const pParagraphProperties(
                        new RTFReferenceProperties(m_aStates.top().aParagraphAttributes, m_aStates.top().aParagraphSprms)
                        );
                Mapper().props(pParagraphProperties);

                // Table width.
                RTFValue::Pointer_t pUnitValue(new RTFValue(3));
                lcl_putNestedAttribute(m_aStates.top().aTableRowSprms,
                        NS_ooxml::LN_CT_TblPrBase_tblW, NS_ooxml::LN_CT_TblWidth_type, pUnitValue);
                RTFValue::Pointer_t pWidthValue(new RTFValue(m_aStates.top().nCellX));
                lcl_putNestedAttribute(m_aStates.top().aTableRowSprms,
                        NS_ooxml::LN_CT_TblPrBase_tblW, NS_ooxml::LN_CT_TblWidth_w, pWidthValue);

                RTFValue::Pointer_t pRowValue(new RTFValue(1));
                m_aStates.top().aTableRowSprms.push_back(make_pair(NS_sprm::LN_PRow, pRowValue));
                writerfilter::Reference<Properties>::Pointer_t const pTableRowProperties(
                        new RTFReferenceProperties(m_aStates.top().aTableRowAttributes, m_aStates.top().aTableRowSprms)
                        );
                Mapper().props(pTableRowProperties);

                lcl_TableBreak(Mapper());
                m_bNeedPap = true;
                m_aTableBuffer.clear();
            }
            break;
        case RTF_COLUMN:
            {
                sal_uInt8 sBreak[] = { 0xe };
                Mapper().startCharacterGroup();
                Mapper().text(sBreak, 1);
                Mapper().endCharacterGroup();
            }
            break;
        case RTF_CHFTN:
            // Nothing to do, dmapper assumes this is the default.
            break;
        default:
            OSL_TRACE("%s: TODO handle symbol '%s'", OSL_THIS_FUNC, lcl_RtfToString(nKeyword));
            bParsed = false;
            break;
    }
    skipDestination(bParsed);
    return 0;
}

int RTFDocumentImpl::dispatchFlag(RTFKeyword nKeyword)
{
    bool bParsed = true;
    int nParam = -1;

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
        m_aStates.top().aParagraphSprms.push_back(make_pair(NS_sprm::LN_PJc, pValue));
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
    if (nParam >= 0)
    {
        RTFValue::Pointer_t pValue(new RTFValue(nParam));
        m_aStates.top().aTabAttributes.push_back(make_pair(NS_ooxml::LN_CT_TabStop_val, pValue));
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
    if (nParam >= 0)
    {
        RTFValue::Pointer_t pValue(new RTFValue(nParam));
        m_aStates.top().aTabAttributes.push_back(make_pair(NS_ooxml::LN_CT_TabStop_leader, pValue));
        skipDestination(bParsed);
        return 0;
    }

    // Border types
    switch (nKeyword)
    {
        // brdrhair and brdrs are the same, brdrw will make a difference
        case RTF_BRDRHAIR: nParam = editeng::SOLID; break;
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
    if (nParam >= 0)
    {
        RTFValue::Pointer_t pValue(new RTFValue(nParam));
        lcl_putBorderProperty(m_aStates, NS_rtf::LN_BRCTYPE, pValue);
        skipDestination(bParsed);
        return 0;
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
        m_aStates.top().aSectionSprms.push_back(make_pair(NS_sprm::LN_SBkc, pValue));
        skipDestination(bParsed);
        return 0;
    }

    // Trivial paragraph flags
    switch (nKeyword)
    {
        case RTF_KEEP: nParam = NS_sprm::LN_PFKeep; break;
        case RTF_KEEPN: nParam = NS_sprm::LN_PFKeepFollow; break;
        case RTF_INTBL: m_pCurrentBuffer = &m_aTableBuffer; nParam = NS_sprm::LN_PFInTable; break;
        case RTF_PAGEBB: nParam = NS_sprm::LN_PFPageBreakBefore; break;
        default: break;
    }
    if (nParam >= 0)
    {
        RTFValue::Pointer_t pValue(new RTFValue(1));
        m_aStates.top().aParagraphSprms.push_back(make_pair(nParam, pValue));
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
        case RTF_PLAIN:
            m_aStates.top().aCharacterSprms = m_aDefaultState.aCharacterSprms;
            m_aStates.top().aCharacterAttributes = m_aDefaultState.aCharacterAttributes;
            break;
        case RTF_PARD:
            m_aStates.top().aParagraphSprms = m_aDefaultState.aParagraphSprms;
            m_aStates.top().aParagraphAttributes = m_aDefaultState.aParagraphAttributes;
            m_pCurrentBuffer = 0;
            break;
        case RTF_SECTD:
            m_aStates.top().aSectionSprms = m_aDefaultState.aSectionSprms;
            m_aStates.top().aSectionAttributes = m_aDefaultState.aSectionAttributes;
            break;
        case RTF_TROWD:
            m_aStates.top().aTableRowSprms = m_aDefaultState.aTableRowSprms;
            m_aStates.top().aTableRowAttributes = m_aDefaultState.aTableRowAttributes;
            m_aStates.top().nCellX = 0;
            break;
        case RTF_WIDCTLPAR:
        case RTF_NOWIDCTLPAR:
            {
                RTFValue::Pointer_t pValue(new RTFValue(nKeyword == RTF_WIDCTLPAR));
                m_aStates.top().aParagraphSprms.push_back(make_pair(NS_sprm::LN_PFWidowControl, pValue));
            }
            break;
        case RTF_BOX:
            {
                RTFSprms_t aAttributes;
                RTFValue::Pointer_t pValue(new RTFValue(aAttributes));
                m_aStates.top().aParagraphSprms.push_back(make_pair(NS_sprm::LN_PBrcTop, pValue));
                m_aStates.top().aParagraphSprms.push_back(make_pair(NS_sprm::LN_PBrcLeft, pValue));
                m_aStates.top().aParagraphSprms.push_back(make_pair(NS_sprm::LN_PBrcBottom, pValue));
                m_aStates.top().aParagraphSprms.push_back(make_pair(NS_sprm::LN_PBrcRight, pValue));
                m_aStates.top().nBorderState = BORDER_PARAGRAPH;
            }
            break;
        case RTF_LTRSECT:
        case RTF_RTLSECT:
            {
                RTFValue::Pointer_t pValue(new RTFValue(nKeyword == RTF_LTRSECT ? 0 : 1));
                m_aStates.top().aParagraphSprms.push_back(make_pair(NS_sprm::LN_STextFlow, pValue));
            }
            break;
        case RTF_LTRPAR:
        case RTF_RTLPAR:
            {
                RTFValue::Pointer_t pValue(new RTFValue(nKeyword == RTF_LTRPAR ? 0 : 1));
                m_aStates.top().aParagraphSprms.push_back(make_pair(NS_sprm::LN_PFrameTextFlow, pValue));
            }
            break;
        case RTF_LTRROW:
        case RTF_RTLROW:
            {
                RTFValue::Pointer_t pValue(new RTFValue(nKeyword == RTF_LTRROW ? 0 : 1));
                m_aStates.top().aParagraphSprms.push_back(make_pair(NS_sprm::LN_TTextFlow, pValue));
            }
            break;
        case RTF_LTRCH:
        case RTF_RTLCH:
            // dmapper does not support these.
            break;
        case RTF_ULNONE:
            {
                RTFValue::Pointer_t pValue(new RTFValue(0));
                m_aStates.top().aCharacterSprms.push_back(make_pair(NS_sprm::LN_CKul, pValue));
            }
            break;
        case RTF_NONSHPPICT:
            m_aStates.top().nDestinationState = DESTINATION_SKIP;
            break;
        case RTF_CLBRDRT:
        case RTF_CLBRDRL:
        case RTF_CLBRDRB:
        case RTF_CLBRDRR:
            {
                RTFSprms_t aAttributes;
                RTFSprms_t aSprms;
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
                RTFSprms_t aAttributes;
                RTFSprms_t aSprms;
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
        case RTF_CLVMGF:
            {
                RTFValue::Pointer_t pValue(new RTFValue(NS_ooxml::LN_Value_ST_Merge_restart));
                m_aStates.top().aTableCellSprms.push_back(make_pair(NS_ooxml::LN_CT_TcPrBase_vMerge, pValue));
            }
            break;
        case RTF_CLVMRG:
            {
                RTFValue::Pointer_t pValue(new RTFValue(NS_ooxml::LN_Value_ST_Merge_continue));
                m_aStates.top().aTableCellSprms.push_back(make_pair(NS_ooxml::LN_CT_TcPrBase_vMerge, pValue));
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
                m_aStates.top().aTableCellSprms.push_back(make_pair(NS_ooxml::LN_CT_TcPrBase_vAlign, pValue));
            }
            break;
        case RTF_TRKEEP:
            {
                RTFValue::Pointer_t pValue(new RTFValue(1));
                m_aStates.top().aTableRowSprms.push_back(make_pair(NS_sprm::LN_TCantSplit, pValue));
            }
        case RTF_SECTUNLOCKED:
            {
                RTFValue::Pointer_t pValue(new RTFValue(!nParam));
                m_aStates.top().aSectionSprms.push_back(make_pair(NS_ooxml::LN_EG_SectPrContents_formProt, pValue));
            }
        case RTF_PGNDEC:
        case RTF_PGNUCRM:
        case RTF_PGNLCRM:
        case RTF_PGNUCLTR:
        case RTF_PGNLCLTR:
        case RTF_PGNBIDIA:
        case RTF_PGNBIDIB:
            break;
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
                m_aStates.top().aSectionSprms.push_back(make_pair(NS_ooxml::LN_EG_SectPrContents_titlePg, pValue));
            }
            break;
        case RTF_SUPER:
            {
                m_pCurrentBuffer = &m_aSuperBuffer;
                OUString aValue(RTL_CONSTASCII_USTRINGPARAM("superscript"));
                RTFValue::Pointer_t pValue(new RTFValue(aValue));
                m_aStates.top().aCharacterSprms.push_back(make_pair(NS_ooxml::LN_EG_RPrBase_vertAlign, pValue));
            }
            break;
        case RTF_SUB:
            {
                OUString aValue(RTL_CONSTASCII_USTRINGPARAM("subscript"));
                RTFValue::Pointer_t pValue(new RTFValue(aValue));
                m_aStates.top().aCharacterSprms.push_back(make_pair(NS_ooxml::LN_EG_RPrBase_vertAlign, pValue));
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
        default:
            OSL_TRACE("%s: TODO handle flag '%s'", OSL_THIS_FUNC, lcl_RtfToString(nKeyword));
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
    RTFValue::Pointer_t pIntValue(new RTFValue(nParam));
    // Trivial table sprms.
    switch (nKeyword)
    {
        case RTF_FPRQ: nSprm = NS_rtf::LN_PRQ; break;
        case RTF_LEVELJC: nSprm = NS_ooxml::LN_CT_Lvl_lvlJc; break;
        case RTF_LEVELNFC: nSprm = NS_rtf::LN_NFC; break;
        case RTF_LEVELSTARTAT: nSprm = NS_rtf::LN_ISTARTAT; break;
        default: break;
    }
    if (nSprm > 0)
    {
        m_aStates.top().aTableSprms.push_back(make_pair(nSprm, pIntValue));
        skipDestination(bParsed);
        return 0;
    }
    // Trivial character sprms.
    switch (nKeyword)
    {
        case RTF_AF: nSprm = (m_aStates.top().bIsCjk ? NS_sprm::LN_CRgFtc1 : NS_sprm::LN_CRgFtc2); break;
        case RTF_FS: nSprm = NS_sprm::LN_CHps; break;
        case RTF_AFS: nSprm = NS_sprm::LN_CHpsBi; break;
        case RTF_ANIMTEXT: nSprm = NS_sprm::LN_CSfxText; break;
        case RTF_EXPNDTW: nSprm = NS_sprm::LN_CDxaSpace; break;
        case RTF_KERNING: nSprm = NS_sprm::LN_CHpsKern; break;
        case RTF_CHARSCALEX: nSprm = NS_sprm::LN_CCharScale; break;
        case RTF_LANG: nSprm = NS_sprm::LN_CRgLid0; break;
        case RTF_LANGFE: nSprm = NS_sprm::LN_CRgLid1; break;
        default: break;
    }
    if (nSprm > 0)
    {
        m_aStates.top().aCharacterSprms.push_back(make_pair(nSprm, pIntValue));
        skipDestination(bParsed);
        return 0;
    }
    // Trivial paragraph sprms.
    switch (nKeyword)
    {
        case RTF_FI: nSprm = NS_sprm::LN_PDxaLeft1; break;
        case RTF_LI: nSprm = NS_sprm::LN_PDxaLeft; break;
        case RTF_LIN: nSprm = 0x845e; break;
        case RTF_RI: nSprm = NS_sprm::LN_PDxaRight; break;
        case RTF_RIN: nSprm = 0x845d; break;
        case RTF_SB: nSprm = NS_sprm::LN_PDyaBefore; break;
        case RTF_SA: nSprm = NS_sprm::LN_PDyaAfter; break;
        case RTF_ITAP: nSprm = NS_sprm::LN_PTableDepth; break;
        default: break;
    }
    if (nSprm > 0)
    {
        m_aStates.top().aParagraphSprms.push_back(make_pair(nSprm, pIntValue));
        skipDestination(bParsed);
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
        m_aStates.top().aTableAttributes.push_back(make_pair(nSprm, pIntValue));
        skipDestination(bParsed);
        return 0;
    }

    // Trivial paragraph attributes.
    switch (nKeyword)
    {
        // NS_sprm::LN_PDyaLine could be used, but that won't work with slmult
        case RTF_SL: nSprm = NS_ooxml::LN_CT_Spacing_line; break;
        default: break;
    }
    if (nSprm > 0)
    {
        m_aStates.top().aParagraphAttributes.push_back(make_pair(nSprm, pIntValue));
        skipDestination(bParsed);
        return 0;
    }

    // Trivial character attributes.
    switch (nKeyword)
    {
        case RTF_PICW: nSprm = NS_rtf::LN_XEXT; if (m_aStates.top().nPictureScaleX) nParam = m_aStates.top().nPictureScaleX * nParam; break;
        case RTF_PICH: nSprm = NS_rtf::LN_YEXT; if (m_aStates.top().nPictureScaleY) nParam = m_aStates.top().nPictureScaleY * nParam; break;
        default: break;
    }
    if (nSprm > 0)
    {
        RTFValue::Pointer_t pValue(new RTFValue(nParam));
        m_aStates.top().aCharacterAttributes.push_back(make_pair(nSprm, pValue));
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
                m_aStates.top().aCharacterSprms.push_back(make_pair(NS_sprm::LN_CRgFtc0, pIntValue));
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
                // NS_sprm::LN_CIco won't work, that would be an index in a static table
                m_aStates.top().aCharacterAttributes.push_back(make_pair(NS_ooxml::LN_CT_Color_val, pIntValue));
            }
            break;
        case RTF_S:
            if (m_aStates.top().nDestinationState == DESTINATION_STYLEENTRY)
            {
                m_aStates.top().nCurrentStyleIndex = nParam;
                m_aStates.top().aTableAttributes.push_back(make_pair(NS_rtf::LN_ISTD, pIntValue));
            }
            else
                m_aStates.top().aParagraphAttributes.push_back(make_pair(NS_rtf::LN_ISTD, pIntValue));
            break;
        case RTF_CS:
            if (m_aStates.top().nDestinationState == DESTINATION_STYLEENTRY)
            {
                m_aStates.top().nCurrentStyleIndex = nParam;
                m_aStates.top().aTableAttributes.push_back(make_pair(NS_rtf::LN_ISTD, pIntValue));
                RTFValue::Pointer_t pValue(new RTFValue(2));
                m_aStates.top().aTableAttributes.push_back(make_pair(NS_rtf::LN_SGC, pValue)); // character style
            }
            else
                m_aStates.top().aCharacterAttributes.push_back(make_pair(NS_rtf::LN_ISTD, pIntValue));
            break;
        case RTF_DEFF:
                m_aDefaultState.aCharacterSprms.push_back(make_pair(NS_sprm::LN_CRgFtc0, pIntValue));
            break;
        case RTF_DEFLANG:
                m_aDefaultState.aCharacterSprms.push_back(make_pair(NS_sprm::LN_CRgLid0, pIntValue));
            break;
        case RTF_ADEFLANG:
                m_aDefaultState.aCharacterSprms.push_back(make_pair(NS_sprm::LN_CLidBi, pIntValue));
            break;
        case RTF_CHCBPAT:
            {
                RTFValue::Pointer_t pValue(new RTFValue(getColorTable(nParam)));
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
            {
                RTFValue::Pointer_t pValue(new RTFValue(getColorTable(nParam)));
                lcl_putNestedAttribute(m_aStates.top().aParagraphSprms, NS_sprm::LN_PShd, NS_ooxml::LN_CT_Shd_fill, pValue);
            }
            break;
        case RTF_ULC:
            {
                RTFValue::Pointer_t pValue(new RTFValue(getColorTable(nParam)));
                m_aStates.top().aCharacterSprms.push_back(make_pair(0x6877, pValue));
            }
            break;
        case RTF_UP: // TODO handle when point size is not shrinking
            {
                OUString aValue(RTL_CONSTASCII_USTRINGPARAM("superscript"));
                RTFValue::Pointer_t pValue(new RTFValue(aValue));
                m_aStates.top().aCharacterSprms.push_back(make_pair(NS_ooxml::LN_EG_RPrBase_vertAlign, pValue));
            }
            break;
        case RTF_DN:
            {
                OUString aValue(RTL_CONSTASCII_USTRINGPARAM("subscript"));
                RTFValue::Pointer_t pValue(new RTFValue(aValue));
                m_aStates.top().aCharacterSprms.push_back(make_pair(NS_ooxml::LN_EG_RPrBase_vertAlign, pValue));
            }
            break;
        case RTF_HORZVERT:
            {
                RTFValue::Pointer_t pValue(new RTFValue(true));
                m_aStates.top().aCharacterAttributes.push_back(make_pair(NS_ooxml::LN_CT_EastAsianLayout_vert, pValue));
                if (nParam)
                    // rotate fits to a single line
                    m_aStates.top().aCharacterAttributes.push_back(make_pair(NS_ooxml::LN_CT_EastAsianLayout_vertCompress, pValue));
            }
            break;
        case RTF_EXPND:
            {
                RTFValue::Pointer_t pValue(new RTFValue(nParam/5));
                m_aStates.top().aCharacterSprms.push_back(make_pair(NS_sprm::LN_CDxaSpace, pValue));
            }
            break;
        case RTF_TWOINONE:
            {
                RTFValue::Pointer_t pValue(new RTFValue(true));
                m_aStates.top().aCharacterAttributes.push_back(make_pair(NS_ooxml::LN_CT_EastAsianLayout_combine, pValue));
                if (nParam > 0)
                    m_aStates.top().aCharacterAttributes.push_back(make_pair(NS_ooxml::LN_CT_EastAsianLayout_combineBrackets, pIntValue));
            }
            break;
        case RTF_SLMULT:
            if (nParam > 0)
            {
                RTFValue::Pointer_t pValue(new RTFValue(NS_ooxml::LN_Value_wordprocessingml_ST_LineSpacingRule_auto));
                m_aStates.top().aParagraphAttributes.push_back(make_pair(NS_ooxml::LN_CT_Spacing_lineRule, pValue));
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
                m_aStates.top().aTabAttributes.push_back(make_pair(NS_ooxml::LN_CT_TabStop_pos, pIntValue));
                RTFValue::Pointer_t pValue(new RTFValue(m_aStates.top().aTabAttributes));
                lcl_putNestedSprm(m_aStates.top().aParagraphSprms, NS_ooxml::LN_CT_PPrBase_tabs, NS_ooxml::LN_CT_Tabs_tab, pValue);
                m_aStates.top().aTabAttributes.clear();
            }
            break;
        case RTF_ILVL:
            {
                RTFSprms_t& rSprms = lcl_getNumPr(m_aStates);
                rSprms.push_back(make_pair(NS_sprm::LN_PIlvl, pIntValue));
            }
        case RTF_LISTTEMPLATEID:
            // This one is not referenced anywhere, so it's pointless to store it at the moment.
            break;
        case RTF_LISTID:
            {
                if (m_aStates.top().nDestinationState == DESTINATION_LISTENTRY)
                    m_aStates.top().aTableAttributes.push_back(make_pair(NS_ooxml::LN_CT_AbstractNum_abstractNumId, pIntValue));
                else if (m_aStates.top().nDestinationState == DESTINATION_LISTOVERRIDEENTRY)
                    m_aStates.top().aTableSprms.push_back(make_pair(NS_ooxml::LN_CT_Num_abstractNumId, pIntValue));
            }
            break;
        case RTF_LS:
            {
                if (m_aStates.top().nDestinationState == DESTINATION_LISTOVERRIDEENTRY)
                    m_aStates.top().aTableAttributes.push_back(make_pair(NS_rtf::LN_LSID, pIntValue));
                else
                {
                    RTFSprms_t& rSprms = lcl_getNumPr(m_aStates);
                    rSprms.push_back(make_pair(NS_sprm::LN_PIlfo, pIntValue));
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
        case RTF_LEVELFOLLOW:
        case RTF_LISTOVERRIDECOUNT:
            // Ignore these for now, the exporter always emits them with a zero parameter.
            break;
        case RTF_PICSCALEX:
            m_aStates.top().nPictureScaleX = 0.01 * nParam;
            break;
        case RTF_PICSCALEY:
            m_aStates.top().nPictureScaleY = 0.01 * nParam;
            break;
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
                m_aStates.top().aCharacterAttributes.push_back(make_pair(NS_ooxml::LN_CT_WrapSquare_wrapText, pValue));
            }
            break;
        case RTF_CELLX:
            {
                int nCellX = nParam - m_aStates.top().nCellX;
                m_aStates.top().nCellX = nParam;
                RTFValue::Pointer_t pXValue(new RTFValue(nCellX));
                m_aStates.top().aTableRowSprms.push_back(make_pair(NS_ooxml::LN_CT_TblGridBase_gridCol, pXValue));

                replayBuffer(m_aTableBuffer);

                // Reset cell properties.
                RTFSprms::Pointer_t pTableCellSprms(new RTFSprms_t(m_aStates.top().aTableCellSprms));
                RTFSprms::Pointer_t pTableCellAttributes(new RTFSprms_t(m_aStates.top().aTableCellAttributes));
                m_aStates.top().aTableCellSprms = m_aDefaultState.aTableCellSprms;
                m_aStates.top().aTableCellAttributes = m_aDefaultState.aTableCellAttributes;
            }
            break;
        case RTF_TRRH:
            {
                lcl_putNestedAttribute(m_aStates.top().aTableRowSprms,
                        NS_ooxml::LN_CT_TrPrBase_trHeight, NS_ooxml::LN_CT_Height_val, pIntValue);
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
                RTFSprms_t& rAttributes = lcl_getLastAttributes(m_aStates.top().aSectionSprms, NS_ooxml::LN_EG_SectPrContents_cols);
                rAttributes.push_back(make_pair((nKeyword == RTF_COLW ? NS_ooxml::LN_CT_Column_w : NS_ooxml::LN_CT_Column_space),
                            pIntValue));
            }
            break;
        case RTF_PAPERH:
            lcl_putNestedAttribute(m_aDefaultState.aSectionSprms,
                    NS_ooxml::LN_EG_SectPrContents_pgSz, NS_ooxml::LN_CT_PageSz_h, pIntValue, true);
            break;
        case RTF_PAPERW:
            lcl_putNestedAttribute(m_aDefaultState.aSectionSprms,
                    NS_ooxml::LN_EG_SectPrContents_pgSz, NS_ooxml::LN_CT_PageSz_w, pIntValue, true);
            break;
        case RTF_PGHSXN:
            lcl_putNestedAttribute(m_aStates.top().aSectionSprms,
                    NS_ooxml::LN_EG_SectPrContents_pgSz, NS_ooxml::LN_CT_PageSz_h, pIntValue, true);
            break;
        case RTF_PGWSXN:
            lcl_putNestedAttribute(m_aStates.top().aSectionSprms,
                    NS_ooxml::LN_EG_SectPrContents_pgSz, NS_ooxml::LN_CT_PageSz_w, pIntValue, true);
            break;
        case RTF_MARGL:
            lcl_putNestedAttribute(m_aDefaultState.aSectionSprms,
                    NS_ooxml::LN_EG_SectPrContents_pgMar, NS_ooxml::LN_CT_PageMar_left, pIntValue, true);
            break;
        case RTF_MARGR:
            lcl_putNestedAttribute(m_aDefaultState.aSectionSprms,
                    NS_ooxml::LN_EG_SectPrContents_pgMar, NS_ooxml::LN_CT_PageMar_right, pIntValue, true);
            break;
        case RTF_MARGT:
            lcl_putNestedAttribute(m_aDefaultState.aSectionSprms,
                    NS_ooxml::LN_EG_SectPrContents_pgMar, NS_ooxml::LN_CT_PageMar_top, pIntValue, true);
            break;
        case RTF_MARGB:
            lcl_putNestedAttribute(m_aDefaultState.aSectionSprms,
                    NS_ooxml::LN_EG_SectPrContents_pgMar, NS_ooxml::LN_CT_PageMar_bottom, pIntValue, true);
            break;
        case RTF_MARGLSXN:
            lcl_putNestedAttribute(m_aStates.top().aSectionSprms,
                    NS_ooxml::LN_EG_SectPrContents_pgMar, NS_ooxml::LN_CT_PageMar_left, pIntValue, true);
            break;
        case RTF_MARGRSXN:
            lcl_putNestedAttribute(m_aStates.top().aSectionSprms,
                    NS_ooxml::LN_EG_SectPrContents_pgMar, NS_ooxml::LN_CT_PageMar_right, pIntValue, true);
            break;
        case RTF_MARGTSXN:
            lcl_putNestedAttribute(m_aStates.top().aSectionSprms,
                    NS_ooxml::LN_EG_SectPrContents_pgMar, NS_ooxml::LN_CT_PageMar_top, pIntValue, true);
            break;
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
            m_aSettingsTableSprms.push_back(make_pair(NS_ooxml::LN_CT_Settings_defaultTabStop, pIntValue));
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
        default:
            OSL_TRACE("%s: TODO handle value '%s'", OSL_THIS_FUNC, lcl_RtfToString(nKeyword));
            bParsed = false;
            break;
    }
    skipDestination(bParsed);
    return 0;
}

int RTFDocumentImpl::dispatchToggle(RTFKeyword nKeyword, bool bParam, int nParam)
{
    bool bParsed = true;
    int nSprm = -1;
    RTFValue::Pointer_t pBoolValue(new RTFValue(!bParam || nParam != 0));

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
    if (nSprm >= 0)
    {
        RTFValue::Pointer_t pValue(new RTFValue((!bParam || nParam != 0) ? nSprm : 0));
        m_aStates.top().aCharacterSprms.push_back(make_pair(NS_sprm::LN_CKul, pValue));
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
    if (nSprm >= 0)
    {
        RTFValue::Pointer_t pValue(new RTFValue((!bParam || nParam != 0) ? nSprm : 0));
        m_aStates.top().aCharacterSprms.push_back(make_pair(NS_sprm::LN_CKcd, pValue));
        skipDestination(bParsed);
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
        default: break;
    }
    if (nSprm >= 0)
    {
        m_aStates.top().aCharacterSprms.push_back(make_pair(nSprm, pBoolValue));
        skipDestination(bParsed);
        return 0;
    }

    switch (nKeyword)
    {
        case RTF_ASPALPHA:
            m_aStates.top().aParagraphSprms.push_back(make_pair(NS_sprm::LN_PFAutoSpaceDE, pBoolValue));
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
            OSL_TRACE("%s: TODO handle toggle '%s'", OSL_THIS_FUNC, lcl_RtfToString(nKeyword));
            bParsed = false;
            break;
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
    /*OSL_TRACE("%s: keyword '\\%s' with param? %d param val: '%d'", OSL_THIS_FUNC,
            rKeyword.getStr(), (bParam ? 1 : 0), (bParam ? nParam : 0));*/
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

    m_nGroupStartPos = Strm().Tell();
    RTFParserState aState;
    if (m_aStates.empty())
        aState = m_aDefaultState;
    else
        aState = m_aStates.top();
    m_aStates.push(aState);

    m_nGroup++;

    if (m_aStates.top().nDestinationState == DESTINATION_FONTTABLE)
        m_aStates.top().nDestinationState = DESTINATION_FONTENTRY;
    else if (m_aStates.top().nDestinationState == DESTINATION_STYLESHEET)
        m_aStates.top().nDestinationState = DESTINATION_STYLEENTRY;
    else if (m_aStates.top().nDestinationState == DESTINATION_FIELDRESULT ||
            m_aStates.top().nDestinationState == DESTINATION_SHAPETEXT)
        m_aStates.top().nDestinationState = DESTINATION_NORMAL;
    else if (m_aStates.top().nDestinationState == DESTINATION_REVISIONTABLE)
        m_aStates.top().nDestinationState = DESTINATION_REVISIONENTRY;

    return 0;
}

RTFSprms_t RTFDocumentImpl::mergeSprms()
{
    RTFSprms_t aSprms;
    for (RTFSprms_t::iterator i = m_aStates.top().aTableSprms.begin();
            i != m_aStates.top().aTableSprms.end(); ++i)
        aSprms.push_back(make_pair(i->first, i->second));
    for (RTFSprms_t::iterator i = m_aStates.top().aCharacterSprms.begin();
            i != m_aStates.top().aCharacterSprms.end(); ++i)
        aSprms.push_back(make_pair(i->first, i->second));
    for (RTFSprms_t::iterator i = m_aStates.top().aParagraphSprms.begin();
            i != m_aStates.top().aParagraphSprms.end(); ++i)
        aSprms.push_back(make_pair(i->first, i->second));
    return aSprms;
}

RTFSprms_t RTFDocumentImpl::mergeAttributes()
{
    RTFSprms_t aAttributes;
    for (RTFSprms_t::iterator i = m_aStates.top().aTableAttributes.begin();
            i != m_aStates.top().aTableAttributes.end(); ++i)
        aAttributes.push_back(make_pair(i->first, i->second));
    for (RTFSprms_t::iterator i = m_aStates.top().aCharacterAttributes.begin();
            i != m_aStates.top().aCharacterAttributes.end(); ++i)
        aAttributes.push_back(make_pair(i->first, i->second));
    for (RTFSprms_t::iterator i = m_aStates.top().aParagraphAttributes.begin();
            i != m_aStates.top().aParagraphAttributes.end(); ++i)
        aAttributes.push_back(make_pair(i->first, i->second));
    return aAttributes;
}

int RTFDocumentImpl::popState()
{
    //OSL_TRACE("%s before pop: m_nGroup %d, dest state: %d", OSL_THIS_FUNC, m_nGroup, m_aStates.top().nDestinationState);

    RTFReferenceTable::Entry_t aEntry;
    bool bFontEntryEnd = false;
    bool bStyleEntryEnd = false;
    RTFSprms_t aSprms;
    RTFSprms_t aAttributes;
    bool bListEntryEnd = false;
    bool bListLevelEnd = false;
    bool bListOverrideEntryEnd = false;
    bool bLevelTextEnd = false;
    RTFShape aShape;
    bool bPopShapeProperties = false;
    bool bPopPictureProperties = false;

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
        RTFSprms_t aListTableAttributes;
        writerfilter::Reference<Properties>::Pointer_t const pProp(new RTFReferenceProperties(aListTableAttributes, m_aListTableSprms));
        RTFReferenceTable::Entries_t aListTableEntries;
        aListTableEntries.insert(make_pair(0, pProp));
        writerfilter::Reference<Table>::Pointer_t const pTable(new RTFReferenceTable(aListTableEntries));
        Mapper().table(NS_rtf::LN_LISTTABLE, pTable);
    }
    else if (m_aStates.top().nDestinationState == DESTINATION_FONTENTRY)
    {
        RTFValue::Pointer_t pValue(new RTFValue(m_aDestinationText.makeStringAndClear()));
        m_aStates.top().aTableAttributes.push_back(make_pair(NS_rtf::LN_XSZFFN, pValue));

        bFontEntryEnd = true;
        writerfilter::Reference<Properties>::Pointer_t const pProp(
                new RTFReferenceProperties(m_aStates.top().aTableAttributes, m_aStates.top().aTableSprms)
                );
        aEntry.first = m_aStates.top().nCurrentFontIndex;
        aEntry.second = pProp;
    }
    else if (m_aStates.top().nDestinationState == DESTINATION_STYLEENTRY)
    {
        RTFValue::Pointer_t pValue(new RTFValue(m_aDestinationText.makeStringAndClear()));
        m_aStates.top().aTableAttributes.push_back(make_pair(NS_rtf::LN_XSTZNAME1, pValue));

        bStyleEntryEnd = true;
        writerfilter::Reference<Properties>::Pointer_t const pProp(
                new RTFReferenceProperties(mergeAttributes(), mergeSprms())
                );
        aEntry.first = m_aStates.top().nCurrentStyleIndex;
        aEntry.second = pProp;
    }
    else if (m_aStates.top().nDestinationState == DESTINATION_LISTENTRY)
    {
        aAttributes = m_aStates.top().aTableAttributes;
        aSprms = m_aStates.top().aTableSprms;
        for (RTFSprms_t::iterator i = m_aStates.top().aListLevelEntries.begin();
                i != m_aStates.top().aListLevelEntries.end(); ++i)
            aSprms.push_back(make_pair(i->first, i->second));
        bListEntryEnd = true;
    }
    else if (m_aStates.top().nDestinationState == DESTINATION_LISTLEVEL)
    {
        aAttributes = m_aStates.top().aTableAttributes;
        aSprms = m_aStates.top().aTableSprms;
        bListLevelEnd = true;
    }
    else if (m_aStates.top().nDestinationState == DESTINATION_LISTOVERRIDEENTRY)
    {
        aAttributes = m_aStates.top().aTableAttributes;
        aSprms = m_aStates.top().aTableSprms;
        bListOverrideEntryEnd = true;
    }
    else if (m_aStates.top().nDestinationState == DESTINATION_FIELDINSTRUCTION)
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
    else if (m_aStates.top().nDestinationState == DESTINATION_LEVELTEXT)
    {
        OUString aStr = m_aDestinationText.makeStringAndClear();

        // The first character is the length of the string (the rest should be ignored).
        sal_Int32 nLength(aStr.toChar());
        OUString aValue = aStr.copy(1, nLength);
        RTFValue::Pointer_t pValue(new RTFValue(aValue, true));
        m_aStates.top().aTableAttributes.push_back(make_pair(NS_ooxml::LN_CT_LevelText_val, pValue));

        aAttributes = m_aStates.top().aTableAttributes;
        bLevelTextEnd = true;
    }
    else if (m_aStates.top().nDestinationState == DESTINATION_LEVELNUMBERS)
    {
        RTFSprms_t& rAttributes = RTFSprm::find(m_aStates.top().aTableSprms, NS_ooxml::LN_CT_Lvl_lvlText)->getAttributes();
        RTFValue::Pointer_t pValue = RTFSprm::find(rAttributes, NS_ooxml::LN_CT_LevelText_val);
        OUString aOrig = pValue->getString();

        OUStringBuffer aBuf;
        sal_Int32 nReplaces = 1;
        for (int i = 0; i < aOrig.getLength(); i++)
        {
            if (std::find(m_aStates.top().aLevelNumbers.begin(), m_aStates.top().aLevelNumbers.end(), i+1)
                    != m_aStates.top().aLevelNumbers.end())
            {
                aBuf.append(sal_Unicode('%'));
                // '1.1.1' -> '%1.%2.%3', but '1.' (with '2.' prefix omitted) is %2.
                aBuf.append(sal_Int32(nReplaces++ + m_aStates.top().nListLevelNum + 1 - m_aStates.top().aLevelNumbers.size()));
            }
            else
                aBuf.append(aOrig.copy(i, 1));
        }
        pValue->setString(aBuf.makeStringAndClear());
    }
    else if (m_aStates.top().nDestinationState == DESTINATION_SHAPEPROPERTYNAME
            || m_aStates.top().nDestinationState == DESTINATION_SHAPEPROPERTYVALUE
            || m_aStates.top().nDestinationState == DESTINATION_SHAPEPROPERTY)
    {
        aShape = m_aStates.top().aShape;
        aAttributes = m_aStates.top().aCharacterAttributes;
        if (m_aStates.top().nDestinationState == DESTINATION_SHAPEPROPERTYNAME)
            aShape.aProperties.push_back(make_pair(m_aDestinationText.makeStringAndClear(), OUString()));
        else if (m_aStates.top().nDestinationState == DESTINATION_SHAPEPROPERTYVALUE)
            aShape.aProperties.back().second = m_aDestinationText.makeStringAndClear();
        bPopShapeProperties = true;
    }
    else if (m_aStates.top().nDestinationState == DESTINATION_PICPROP
            || m_aStates.top().nDestinationState == DESTINATION_SHAPEINSTRUCTION)
        resolveShapeProperties(m_aStates.top().aShape.aProperties);
    else if (m_aStates.top().nDestinationState == DESTINATION_REVISIONENTRY)
        m_aAuthors[m_aAuthors.size()] = m_aDestinationText.makeStringAndClear();
    else if (m_aStates.top().nDestinationState == DESTINATION_BOOKMARKSTART)
    {
        OUString aStr = m_aDestinationText.makeStringAndClear();
        int nPos = m_aBookmarks.size();
        m_aBookmarks[aStr] = nPos;
        Mapper().props(lcl_getBookmarkProperties(nPos, aStr));
    }
    else if (m_aStates.top().nDestinationState == DESTINATION_BOOKMARKEND)
        Mapper().props(lcl_getBookmarkProperties(m_aBookmarks[m_aDestinationText.makeStringAndClear()]));
    else if (m_aStates.top().nDestinationState == DESTINATION_PICT)
        resolvePict(true);
    else if (m_aStates.top().nDestinationState == DESTINATION_SHAPEPROPERTYVALUEPICT)
    {
        bPopPictureProperties = true;
        aAttributes = m_aStates.top().aCharacterAttributes;
    }
    else if (m_aStates.top().nDestinationState == DESTINATION_SHAPETEXT)
        m_pCurrentBuffer = 0; // Just disable buffering, don't empty it yet.

    // See if we need to end a track change
    RTFValue::Pointer_t pTrackchange = RTFSprm::find(m_aStates.top().aCharacterSprms, NS_ooxml::LN_trackchange);
    if (pTrackchange.get())
    {
        RTFSprms_t aTCAttributes;
        RTFValue::Pointer_t pValue(new RTFValue(0));
        aTCAttributes.push_back(make_pair(NS_ooxml::LN_endtrackchange, pValue));
        writerfilter::Reference<Properties>::Pointer_t const pProperties(new RTFReferenceProperties(aTCAttributes));
        Mapper().props(pProperties);
    }

    // This is the end of the doc, see if we need to close the last section.
    if (m_nGroup == 1)
        sectBreak(true);

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
        m_aListTableSprms.push_back(make_pair(NS_ooxml::LN_CT_Numbering_abstractNum, pValue));
    }
    else if (bListLevelEnd)
    {
        RTFValue::Pointer_t pInnerValue(new RTFValue(m_aStates.top().nListLevelNum++));
        aAttributes.push_back(make_pair(NS_ooxml::LN_CT_Lvl_ilvl, pInnerValue));

        RTFValue::Pointer_t pValue(new RTFValue(aAttributes, aSprms));
        m_aStates.top().aListLevelEntries.push_back(make_pair(NS_ooxml::LN_CT_AbstractNum_lvl, pValue));
    }
    // list override table
    else if (bListOverrideEntryEnd)
    {
        RTFValue::Pointer_t pValue(new RTFValue(aAttributes, aSprms));
        m_aListTableSprms.push_back(make_pair(NS_ooxml::LN_CT_Numbering_num, pValue));
    }
    else if (bLevelTextEnd)
    {
        RTFValue::Pointer_t pValue(new RTFValue(aAttributes));
        m_aStates.top().aTableSprms.push_back(make_pair(NS_ooxml::LN_CT_Lvl_lvlText, pValue));
    }
    else if (bPopShapeProperties)
    {
        m_aStates.top().aShape = aShape;
        m_aStates.top().aCharacterAttributes = aAttributes;
    }
    if (bPopPictureProperties)
        m_aStates.top().aCharacterAttributes = aAttributes;
    if (m_pCurrentBuffer == &m_aSuperBuffer)
    {
        if (!m_bHasFootnote)
            replayBuffer(m_aSuperBuffer);
        m_pCurrentBuffer = 0;
        m_bHasFootnote = false;
    }

    return 0;
}

void RTFDocumentImpl::createShape(OUString aStr, uno::Reference<drawing::XShape>& xShape, uno::Reference<beans::XPropertySet>& xPropertySet)
{
    xShape.set(m_xModelFactory->createInstance(aStr), uno::UNO_QUERY);
    xPropertySet.set(xShape, uno::UNO_QUERY);
}

void RTFDocumentImpl::resolveShapeProperties(std::vector< std::pair<rtl::OUString, rtl::OUString> >& rShapeProperties)
{
    int nType = -1;
    bool bPib = false;
    bool bCustom = false;

    uno::Reference<drawing::XShape> xShape;
    uno::Reference<beans::XPropertySet> xPropertySet;
    // Create this early, as custom shapes may have properties before the type arrives.
    createShape(OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.CustomShape")), xShape, xPropertySet);
    uno::Any aAny;

    for (std::vector< std::pair<rtl::OUString, rtl::OUString> >::iterator i = rShapeProperties.begin(); i != rShapeProperties.end(); ++i)
    {
        if (i->first.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("shapeType")))
        {
            nType = i->second.toInt32();
            switch (nType)
            {
                case 20: // Line
                    createShape(OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.LineShape")), xShape, xPropertySet);
                    break;
                default:
                    bCustom = true;
                    break;
            }

            // Defaults
            aAny <<= (sal_uInt32)0xffffff; // White in Word, kind of blue in Writer.
            xPropertySet->setPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("FillColor")), aAny);
        }
        else if (i->first.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("wzName")))
        {
            RTFValue::Pointer_t pValue(new RTFValue(i->second));
            m_aStates.top().aCharacterAttributes.push_back(make_pair(NS_ooxml::LN_CT_NonVisualDrawingProps_name, pValue));
        }
        else if (i->first.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("wzDescription")))
        {
            RTFValue::Pointer_t pValue(new RTFValue(i->second));
            m_aStates.top().aCharacterAttributes.push_back(make_pair(NS_ooxml::LN_CT_NonVisualDrawingProps_descr, pValue));
        }
        else if (i->first.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("pib")))
        {
            m_aDestinationText.setLength(0);
            m_aDestinationText.append(i->second);
            bPib = true;
        }
        else if (i->first.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("fillColor")))
        {
            aAny <<= lcl_BGRToRGB(i->second.toInt32());
            xPropertySet->setPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("FillColor")), aAny);
        }
        else if (i->first.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("lineColor")))
        {
            aAny <<= lcl_BGRToRGB(i->second.toInt32());
            xPropertySet->setPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("LineColor")), aAny);
        }
        else if (i->first.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("txflTextFlow")))
        {
            if (i->second.toInt32() == 1)
            {
                aAny <<= text::WritingMode_TB_RL;
                xPropertySet->setPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("TextWritingMode")), aAny);
            }
        }
        else if (i->first.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("fLine")))
        {
            if (i->second.toInt32() == 0)
            {
                aAny <<= drawing::LineStyle_NONE;
                xPropertySet->setPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("LineStyle")), aAny);
            }
        }
        else
            OSL_TRACE("%s: TODO handle shape property '%s':'%s'", OSL_THIS_FUNC,
                    OUStringToOString( i->first, RTL_TEXTENCODING_UTF8 ).getStr(),
                    OUStringToOString( i->second, RTL_TEXTENCODING_UTF8 ).getStr());
    }

    if (nType == 75) // picture frame
    {
        if (bPib)
            resolvePict(false);
        return;
    }

    m_xDrawPage->add(xShape);
    if (bCustom)
    {
        uno::Reference<drawing::XEnhancedCustomShapeDefaulter> xDefaulter(xShape, uno::UNO_QUERY);
        xDefaulter->createCustomShapeDefaults(OUString::valueOf(sal_Int32(nType)));
    }

    xShape->setPosition(awt::Point(m_aStates.top().aShape.nLeft, m_aStates.top().aShape.nTop));
    xShape->setSize(awt::Size(m_aStates.top().aShape.nRight - m_aStates.top().aShape.nLeft,
                m_aStates.top().aShape.nBottom - m_aStates.top().aShape.nTop));

    Mapper().startShape(xShape);
    Mapper().startParagraphGroup();
    replayBuffer(m_aShapetextBuffer);
    Mapper().startCharacterGroup();
    lcl_Break(Mapper());
    Mapper().endCharacterGroup();
    Mapper().endParagraphGroup();
    Mapper().endShape();
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
        //OSL_TRACE("%s: parsing character '%c'", OSL_THIS_FUNC, ch);
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
                    if (m_bIsSubstream && m_nGroup == 0)
                        return 0;
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
                        char parsed = lcl_AsHex(ch);
                        if (parsed == -1)
                            return ERROR_HEX_INVALID;
                        b += parsed;
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
    aTabAttributes(),
    aFontTableEntries(),
    nCurrentFontIndex(0),
    aCurrentColor(),
    aStyleTableEntries(),
    nCurrentStyleIndex(0),
    nCurrentEncoding(0),
    nUc(1),
    nCharsToSkip(0),
    nListLevelNum(0),
    aListLevelEntries(),
    aLevelNumbers(),
    nPictureScaleX(0),
    nPictureScaleY(0),
    aShape(),
    nCellX(0),
    bIsCjk(false)
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
