/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "xmlexpit.hxx"

#include <osl/diagnose.h>
#include <rtl/ustrbuf.hxx>
#include <sax/tools/converter.hxx>
#include <svl/itempool.hxx>
#include <svl/poolitem.hxx>
#include <svl/itemset.hxx>
#include <utility>
#include <xmloff/xmluconv.hxx>
#include <xmloff/attrlist.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/prhdlfac.hxx>
#include <xmloff/xmltypes.hxx>
#include <editeng/xmlcnitm.hxx>
#include <xmloff/xmlexp.hxx>
#include <xmloff/xmlprhdl.hxx>
#include <editeng/memberids.h>
#include <hintids.hxx>
#include <unomid.h>
#include <svx/unomid.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/shaditem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/formatbreakitem.hxx>
#include <editeng/keepitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <fmtpdsc.hxx>
#include <fmtornt.hxx>
#include <fmtfsize.hxx>

#include <fmtlsplt.hxx>
#include "xmlithlp.hxx"

#include <fmtrowsplt.hxx>

using ::editeng::SvxBorderLine;
using namespace ::com::sun::star;
using namespace ::xmloff::token;
using uno::Any;

// fills the given attribute list with the items in the given set
void SvXMLExportItemMapper::exportXML( const SvXMLExport& rExport,
                                SvXMLAttributeList& rAttrList,
                                const SfxItemSet& rSet,
                                const SvXMLUnitConverter& rUnitConverter,
                                const SvXMLNamespaceMap& rNamespaceMap,
                                std::vector<sal_uInt16> *pIndexArray ) const
{
    const sal_uInt16 nCount = mrMapEntries->getCount();
    sal_uInt16 nIndex = 0;

    while( nIndex < nCount )
    {
        SvXMLItemMapEntry const & rEntry = mrMapEntries->getByIndex( nIndex );

        // we have a valid map entry here, so lets use it...
        if( 0 == (rEntry.nMemberId & MID_SW_FLAG_NO_ITEM_EXPORT) )
        {
            const SfxPoolItem* pItem = GetItem( rSet, rEntry.nWhichId );
            // do we have an item?
            if(pItem)
            {
                if( 0 != (rEntry.nMemberId & MID_SW_FLAG_ELEMENT_ITEM_EXPORT) )
                {
                    // element items do not add any properties,
                    // we export it later
                    if( pIndexArray )
                        pIndexArray->push_back( nIndex );

                }
                else
                {
                    exportXML( rExport, rAttrList, *pItem, rEntry, rUnitConverter,
                                  rNamespaceMap, &rSet );
                }
            }
        }
        else
        {
            OSL_FAIL( "no item not handled in xml export" );
        }
        nIndex++;
    }
}

void SvXMLExportItemMapper::exportXML( const SvXMLExport& rExport,
                                 SvXMLAttributeList& rAttrList,
                                 const SfxPoolItem& rItem,
                                 const SvXMLItemMapEntry& rEntry,
                                 const SvXMLUnitConverter& rUnitConverter,
                                 const SvXMLNamespaceMap& rNamespaceMap,
                                 const SfxItemSet *pSet ) const
{
    if( 0 != (rEntry.nMemberId & MID_SW_FLAG_SPECIAL_ITEM_EXPORT) )
    {
        if( dynamic_cast<const SwFormatRowSplit*>( &rItem) !=  nullptr )
        {
            OUString aValue;
            bool bAddAttribute = true;
            if( rEntry.nNameSpace == XML_NAMESPACE_STYLE )
            {
                if( !(rExport.getExportFlags() & SvXMLExportFlags::SAVEBACKWARDCOMPATIBLE ) ||
                    !QueryXMLValue(rItem, aValue,
                    static_cast< sal_uInt16 >( rEntry.nMemberId & MID_SW_FLAG_MASK ),
                    rUnitConverter ) )
                {
                    bAddAttribute = false;
                }
            }
            else
            {
                OUStringBuffer aOut;
                const SfxBoolItem* pSplit = dynamic_cast<const SfxBoolItem*>( &rItem );
                assert(pSplit && "Wrong Which-ID");
                const sal_uInt16 eEnum = (pSplit && pSplit->GetValue()) ? 1 : 0;
                SvXMLUnitConverter::convertEnum( aOut, eEnum, aXML_KeepTogetherType );
                aValue = aOut.makeStringAndClear();
            }
            if( bAddAttribute )
            {
                const OUString sName( rNamespaceMap.GetQNameByKey( rEntry.nNameSpace,
                                GetXMLToken(rEntry.eLocalName) ) );
                rAttrList.AddAttribute( sName, aValue );
            }
        }
        if( dynamic_cast<const SvXMLAttrContainerItem*>( &rItem) !=  nullptr )
        {
            std::unique_ptr<SvXMLNamespaceMap> pNewNamespaceMap;
            const SvXMLNamespaceMap *pNamespaceMap = &rNamespaceMap;

            const SvXMLAttrContainerItem *pUnknown =
                dynamic_cast<const SvXMLAttrContainerItem*>( &rItem  );

            const sal_uInt16 nCount = pUnknown ? pUnknown->GetAttrCount() : 0;
            for( sal_uInt16 i=0; i < nCount; i++ )
            {
                const OUString sPrefix( pUnknown->GetAttrPrefix( i ) );
                if( !sPrefix.isEmpty() )
                {
                    const OUString sNamespace( pUnknown->GetAttrNamespace( i ) );

                    // if the prefix isn't defined yet or has another meaning,
                    // we have to redefine it now.
                    const sal_uInt16 nIdx = pNamespaceMap->GetIndexByPrefix( sPrefix );
                    if( USHRT_MAX == nIdx ||
                        pNamespaceMap->GetNameByIndex( nIdx ) != sNamespace )
                    {
                        if( !pNewNamespaceMap )
                        {
                            pNewNamespaceMap.reset(
                                        new SvXMLNamespaceMap( rNamespaceMap ));
                            pNamespaceMap = pNewNamespaceMap.get();
                        }
                        pNewNamespaceMap->Add( sPrefix, sNamespace );

                        rAttrList.AddAttribute( GetXMLToken(XML_XMLNS) + ":" + sPrefix,
                                                sNamespace );
                    }

                    rAttrList.AddAttribute( sPrefix + ":" + pUnknown->GetAttrLName(i),
                                            pUnknown->GetAttrValue(i) );
                }
                else
                {
                    rAttrList.AddAttribute( pUnknown->GetAttrLName(i),
                                            pUnknown->GetAttrValue(i) );
                }
            }
        }
        else
        {
            handleSpecialItem( rAttrList, rEntry, rItem, rUnitConverter,
                                  rNamespaceMap, pSet );
        }
    }
    else if( 0 == (rEntry.nMemberId & MID_SW_FLAG_ELEMENT_ITEM_EXPORT) )
    {
        bool bDone = false;
        switch (rItem.Which())
        {
            case RES_FRAMEDIR:
            {
                // Write bt-lr to the extension namespace, handle other values
                // below.
                auto pDirection = static_cast<const SvxFrameDirectionItem*>(&rItem);
                if (rEntry.nNameSpace == XML_NAMESPACE_LO_EXT
                    && pDirection->GetValue() == SvxFrameDirection::Vertical_LR_BT)
                {
                    const OUString sName(rNamespaceMap.GetQNameByKey(
                        XML_NAMESPACE_LO_EXT, GetXMLToken(XML_WRITING_MODE)));
                    rAttrList.AddAttribute(sName, GetXMLToken(XML_BT_LR));
                }
                if (rEntry.nNameSpace == XML_NAMESPACE_LO_EXT
                    || pDirection->GetValue() == SvxFrameDirection::Vertical_LR_BT)
                    bDone = true;
                break;
            }
        }

        if (!bDone)
        {
            OUString aValue;
            if( QueryXMLValue(rItem, aValue,
                              static_cast< sal_uInt16 >(
                                              rEntry.nMemberId & MID_SW_FLAG_MASK ),
                                 rUnitConverter ) )
            {
                const OUString sName(
                    rNamespaceMap.GetQNameByKey( rEntry.nNameSpace,
                                                 GetXMLToken(rEntry.eLocalName)));
                rAttrList.AddAttribute( sName, aValue );
            }
        }
    }
}

void SvXMLExportItemMapper::exportElementItems(
                          SvXMLExport& rExport,
                          const SvXMLUnitConverter& rUnitConverter,
                          const SfxItemSet &rSet,
                          SvXmlExportFlags nFlags,
                          const std::vector<sal_uInt16> &rIndexArray ) const
{
    const size_t nCount = rIndexArray.size();

    bool bItemsExported = false;
    for( size_t nIndex = 0; nIndex < nCount; ++nIndex )
    {
        const sal_uInt16 nElement = rIndexArray[ nIndex ];
        SvXMLItemMapEntry const & rEntry = mrMapEntries->getByIndex( nElement );
        OSL_ENSURE( 0 != (rEntry.nMemberId & MID_SW_FLAG_ELEMENT_ITEM_EXPORT),
                    "wrong mid flag!" );

        const SfxPoolItem* pItem = GetItem( rSet, rEntry.nWhichId );
        // do we have an item?
        if(pItem)
        {
            rExport.IgnorableWhitespace();
            handleElementItem( rExport, rEntry, *pItem, rUnitConverter,
                               rSet, nFlags);
            bItemsExported = true;
        }
    }

    if( bItemsExported )
        rExport.IgnorableWhitespace();
}

/** returns the item with the given WhichId from the given ItemSet if its
    set
*/
const SfxPoolItem* SvXMLExportItemMapper::GetItem( const SfxItemSet& rSet,
                                                   sal_uInt16 nWhichId)
{
    // first get item from itemset
    const SfxPoolItem* pItem;
    SfxItemState eState = rSet.GetItemState( nWhichId, false, &pItem );

    if( SfxItemState::SET == eState )
    {
        return pItem;
    }
    else
    {
        return nullptr;
    }
}

SvXMLExportItemMapper::SvXMLExportItemMapper( SvXMLItemMapEntriesRef rMapEntries )
{
    mrMapEntries = std::move(rMapEntries);
}

SvXMLExportItemMapper::~SvXMLExportItemMapper()
{
}

void SvXMLExportItemMapper::exportXML( SvXMLExport& rExport,
                    const SfxItemSet& rSet,
                    const SvXMLUnitConverter& rUnitConverter,
                    XMLTokenEnum ePropToken ) const
{
    std::vector<sal_uInt16> aIndexArray;

    exportXML( rExport, rExport.GetAttrList(), rSet, rUnitConverter,
               rExport.GetNamespaceMap(), &aIndexArray );

    if( rExport.GetAttrList().getLength() > 0 || !aIndexArray.empty() )
    {
        rExport.IgnorableWhitespace();

        SvXMLElementExport aElem( rExport, XML_NAMESPACE_STYLE, ePropToken,
                                  false, false );
        exportElementItems( rExport, rUnitConverter,
                            rSet, SvXmlExportFlags::IGN_WS, aIndexArray );
    }
}

/** this method is called for every item that has the
    MID_SW_FLAG_SPECIAL_ITEM_EXPORT flag set */
void SvXMLExportItemMapper::handleSpecialItem( SvXMLAttributeList& /*rAttrList*/,
                                    const SvXMLItemMapEntry& /*rEntry*/,
                                    const SfxPoolItem& /*rItem*/,
                                    const SvXMLUnitConverter& /*rUnitConverter*/,
                                    const SvXMLNamespaceMap& /*rNamespaceMap*/,
                                    const SfxItemSet* /*pSet*/ /* = NULL */ ) const
{
    OSL_FAIL( "special item not handled in xml export" );
}

/** this method is called for every item that has the
    MID_SW_FLAG_ELEMENT_EXPORT flag set */
void SvXMLExportItemMapper::handleElementItem(
                        SvXMLExport& /*rExport*/,
                        const SvXMLItemMapEntry& /*rEntry*/,
                        const SfxPoolItem& /*rItem*/,
                        const SvXMLUnitConverter& /*rUnitConverter*/,
                        const SfxItemSet& /*rSet*/,
                        SvXmlExportFlags /*nFlags*/ ) const
{
    OSL_FAIL( "element item not handled in xml export" );
}

static bool lcl_isOdfDoubleLine( const SvxBorderLine* pLine )
{
    bool bIsOdfDouble = false;
    switch (pLine->GetBorderLineStyle())
    {
        case SvxBorderLineStyle::DOUBLE:
        case SvxBorderLineStyle::THINTHICK_SMALLGAP:
        case SvxBorderLineStyle::THINTHICK_MEDIUMGAP:
        case SvxBorderLineStyle::THINTHICK_LARGEGAP:
        case SvxBorderLineStyle::THICKTHIN_SMALLGAP:
        case SvxBorderLineStyle::THICKTHIN_MEDIUMGAP:
        case SvxBorderLineStyle::THICKTHIN_LARGEGAP:
            bIsOdfDouble = true;
            break;
        default:
            break;
    }
    return bIsOdfDouble;
}

bool SvXMLExportItemMapper::QueryXMLValue(
    const SfxPoolItem& rItem,
    OUString& rValue,
    sal_uInt16 nMemberId,
    const SvXMLUnitConverter& rUnitConverter )
{
    bool bOk = false;
    OUStringBuffer aOut;

    switch ( rItem.Which() )
    {

        case RES_LR_SPACE:
        {
            const SvxLRSpaceItem& rLRSpace = dynamic_cast<const SvxLRSpaceItem&>(rItem);

            bOk = true;
            switch( nMemberId )
            {
                case  MID_L_MARGIN:
                    if (rLRSpace.GetPropLeft() != 100)
                    {
                        ::sax::Converter::convertPercent(
                                aOut, rLRSpace.GetPropLeft() );
                    }
                    else
                    {
                        rUnitConverter.convertMeasureToXML(
                                aOut, rLRSpace.GetLeft() );
                    }
                    break;

                case  MID_R_MARGIN:
                    if (rLRSpace.GetPropRight() != 100)
                    {
                        ::sax::Converter::convertPercent(
                                aOut, rLRSpace.GetPropRight() );
                    }
                    else
                    {
                        rUnitConverter.convertMeasureToXML(
                                aOut, rLRSpace.GetRight() );
                    }
                    break;

                case MID_FIRST_AUTO:
                    if (rLRSpace.IsAutoFirst())
                    {
                        ::sax::Converter::convertBool(
                                aOut, rLRSpace.IsAutoFirst() );
                    }
                    else
                        bOk = false;
                    break;

                case  MID_FIRST_LINE_INDENT:
                    if (!rLRSpace.IsAutoFirst())
                    {
                        if (rLRSpace.GetPropTextFirstLineOfst() != 100)
                        {
                            ::sax::Converter::convertPercent(
                                aOut, rLRSpace.GetPropTextFirstLineOfst() );
                        }
                        else
                        {
                            rUnitConverter.convertMeasureToXML(
                                    aOut, rLRSpace.GetTextFirstLineOfst() );
                        }
                    }
                    else
                        bOk = false;
                    break;

                default:
                    OSL_FAIL( "unknown member id!");
                    bOk = false;
                    break;
            }
        }
        break;

        case RES_UL_SPACE:
        {
            const SvxULSpaceItem& rULSpace = dynamic_cast<const SvxULSpaceItem&>(rItem);

            switch( nMemberId )
            {
                case MID_UP_MARGIN:
                    if (rULSpace.GetPropUpper() != 100)
                    {
                        ::sax::Converter::convertPercent(
                                aOut, rULSpace.GetPropUpper() );
                    }
                    else
                    {
                        rUnitConverter.convertMeasureToXML(
                                aOut, rULSpace.GetUpper() );
                    }
                    break;

                case MID_LO_MARGIN:
                    if (rULSpace.GetPropLower() != 100)
                    {
                        ::sax::Converter::convertPercent(
                                aOut, rULSpace.GetPropLower() );
                    }
                    else
                    {
                        rUnitConverter.convertMeasureToXML(
                                aOut, rULSpace.GetLower() );
                    }
                    break;

                default:
                    OSL_FAIL("unknown MemberId");
            };

            bOk = true;
        }
        break;

        case RES_SHADOW:
        {
            const SvxShadowItem* pShadow = dynamic_cast<const SvxShadowItem*>( &rItem );
            assert(pShadow && "Wrong Which-ID");
            if (pShadow)
            {
                sal_Int32 nX = 1, nY = 1;
                switch( pShadow->GetLocation() )
                {
                    case SvxShadowLocation::TopLeft:
                        nX = -1;
                        nY = -1;
                        break;
                    case SvxShadowLocation::TopRight:
                        nY = -1;
                        break;
                    case SvxShadowLocation::BottomLeft:
                        nX = -1;
                        break;
                    case SvxShadowLocation::BottomRight:
                        break;
                    case SvxShadowLocation::NONE:
                    default:
                        rValue = GetXMLToken(XML_NONE);
                        return true;
                }

                nX *= pShadow->GetWidth();
                nY *= pShadow->GetWidth();

                ::sax::Converter::convertColor(aOut, pShadow->GetColor());
                aOut.append( ' ' );
                rUnitConverter.convertMeasureToXML( aOut, nX );
                aOut.append( ' ' );
                rUnitConverter.convertMeasureToXML( aOut, nY );

                bOk = true;
            }
        }
        break;

        case RES_BOX:
        {
            const SvxBoxItem* pBox = dynamic_cast<const SvxBoxItem*>( &rItem );
            assert(pBox && "Wrong Which-ID");
            if (pBox)
            {
                /**
                   xml -> MemberId

                   border-padding           ALL_BORDER_PADDING
                   border-padding-before    LEFT_BORDER_PADDING
                   border-padding-after RIGHT_BORDER_PADDING
                   border-padding-start TOP_BORDER_PADDING
                   border-padding-end       BOTTOM_BORDER_PADDING

                   border                   ALL_BORDER
                   border-before            LEFT_BORDER
                   border-after         RIGHT_BORDER
                   border-start         TOP_BORDER
                   border-end               BOTTOM_BORDER

                   border-line-width            ALL_BORDER_LINE_WIDTH
                   border-line-width-before LEFT_BORDER_LINE_WIDTH
                   border-line-width-after      RIGHT_BORDER_LINE_WIDTH
                   border-line-width-start      TOP_BORDER_LINE_WIDTH
                   border-line-width-end        BOTTOM_BORDER_LINE_WIDTH
                */

                const SvxBorderLine* pLeft    = pBox->GetLeft();
                const SvxBorderLine* pRight   = pBox->GetRight();
                const SvxBorderLine* pTop     = pBox->GetTop();
                const SvxBorderLine* pBottom  = pBox->GetBottom();
                const sal_uInt16 nTopDist     = pBox->GetDistance( SvxBoxItemLine::TOP );
                const sal_uInt16 nBottomDist  = pBox->GetDistance( SvxBoxItemLine::BOTTOM );
                const sal_uInt16 nLeftDist    = pBox->GetDistance( SvxBoxItemLine::LEFT );
                const sal_uInt16 nRightDist   = pBox->GetDistance( SvxBoxItemLine::RIGHT );

                // check if we need to export it
                switch( nMemberId )
                {
                    case ALL_BORDER_PADDING:
                    case LEFT_BORDER_PADDING:
                    case RIGHT_BORDER_PADDING:
                    case TOP_BORDER_PADDING:
                    case BOTTOM_BORDER_PADDING:
                    {
                        bool bEqual = nLeftDist == nRightDist &&
                                          nLeftDist == nTopDist &&
                                          nLeftDist == nBottomDist;
                        // don't export individual paddings if all paddings are equal and
                        // don't export all padding if some paddings are not equal
                        if( (bEqual && ALL_BORDER_PADDING != nMemberId) ||
                            (!bEqual && ALL_BORDER_PADDING == nMemberId) )
                            return false;
                    }
                    break;
                    case ALL_BORDER:
                    case LEFT_BORDER:
                    case RIGHT_BORDER:
                    case TOP_BORDER:
                    case BOTTOM_BORDER:
                    {
                        bool bEqual = ( nullptr == pTop && nullptr == pBottom &&
                                            nullptr == pLeft && nullptr == pRight ) ||
                                          ( pTop && pBottom && pLeft && pRight &&
                                           *pTop == *pBottom  && *pTop == *pLeft &&
                                            *pTop == *pRight );

                        // don't export individual borders if all are the same and
                        // don't export all borders if some are not equal
                        if( (bEqual && ALL_BORDER != nMemberId) ||
                            (!bEqual && ALL_BORDER == nMemberId) )
                            return false;
                    }
                    break;
                    case ALL_BORDER_LINE_WIDTH:
                    case LEFT_BORDER_LINE_WIDTH:
                    case RIGHT_BORDER_LINE_WIDTH:
                    case TOP_BORDER_LINE_WIDTH:
                    case BOTTOM_BORDER_LINE_WIDTH:
                    {
                        // if no line is set, there is nothing to export
                        if( !pTop && !pBottom && !pLeft && !pRight )
                            return false;

                        bool bEqual = nullptr != pTop &&
                                          nullptr != pBottom &&
                                          nullptr != pLeft &&
                                          nullptr != pRight;

                        if( bEqual )
                        {
                            const sal_uInt16 nDistance = pTop->GetDistance();
                            const sal_uInt16 nInWidth  = pTop->GetInWidth();
                            const sal_uInt16 nOutWidth = pTop->GetOutWidth();
                            const long nWidth = pTop->GetWidth();

                            bEqual = nDistance == pLeft->GetDistance() &&
                                     nInWidth  == pLeft->GetInWidth()  &&
                                     nOutWidth == pLeft->GetOutWidth() &&
                                     nWidth == pLeft->GetWidth() &&
                                     nDistance == pRight->GetDistance()  &&
                                     nInWidth  == pRight->GetInWidth()   &&
                                     nOutWidth == pRight->GetOutWidth()  &&
                                     nWidth == pRight->GetWidth()  &&
                                     nDistance == pBottom->GetDistance()  &&
                                     nInWidth  == pBottom->GetInWidth()   &&
                                     nOutWidth == pBottom->GetOutWidth() &&
                                     nWidth == pBottom->GetWidth();
                        }

                        switch( nMemberId )
                        {
                            case ALL_BORDER_LINE_WIDTH:
                                if( !bEqual || pTop->GetDistance() == 0 ||
                                    !lcl_isOdfDoubleLine( pTop ) )
                                    return false;
                                break;
                            case LEFT_BORDER_LINE_WIDTH:
                                if( bEqual || nullptr == pLeft ||
                                    0 == pLeft->GetDistance() ||
                                    !lcl_isOdfDoubleLine( pLeft ) )
                                    return false;
                                break;
                            case RIGHT_BORDER_LINE_WIDTH:
                                if( bEqual || nullptr == pRight ||
                                    0 == pRight->GetDistance() ||
                                    !lcl_isOdfDoubleLine( pRight ) )
                                    return false;
                                break;
                            case TOP_BORDER_LINE_WIDTH:
                                if( bEqual || nullptr == pTop ||
                                    0 == pTop->GetDistance() ||
                                    !lcl_isOdfDoubleLine( pTop ) )
                                    return false;
                                break;
                            case BOTTOM_BORDER_LINE_WIDTH:
                                if( bEqual || nullptr == pBottom ||
                                    0 == pBottom->GetDistance() ||
                                    !lcl_isOdfDoubleLine( pBottom ) )
                                    return false;
                                break;
                        }
                    }
                    break;
                }

                // now export it export
                switch( nMemberId )
                {
                    // padding
                    case ALL_BORDER_PADDING:
                    case LEFT_BORDER_PADDING:
                        rUnitConverter.convertMeasureToXML( aOut, nLeftDist );
                        break;
                    case RIGHT_BORDER_PADDING:
                        rUnitConverter.convertMeasureToXML( aOut, nRightDist );
                        break;
                    case TOP_BORDER_PADDING:
                        rUnitConverter.convertMeasureToXML( aOut, nTopDist );
                        break;
                    case BOTTOM_BORDER_PADDING:
                        rUnitConverter.convertMeasureToXML( aOut, nBottomDist );
                        break;

                        // border
                    case ALL_BORDER:
                    case LEFT_BORDER:
                    case RIGHT_BORDER:
                    case TOP_BORDER:
                    case BOTTOM_BORDER:
                    {
                        const SvxBorderLine* pLine;
                        switch( nMemberId )
                        {
                        case ALL_BORDER:
                        case LEFT_BORDER:
                            pLine = pLeft;
                            break;
                        case RIGHT_BORDER:
                            pLine = pRight;
                            break;
                        case TOP_BORDER:
                            pLine = pTop;
                            break;
                        case BOTTOM_BORDER:
                            pLine = pBottom;
                            break;
                        default:
                            pLine = nullptr;
                            break;
                        }

                        if( nullptr != pLine )
                        {
                            sal_Int32 nWidth = pLine->GetWidth();

                            enum XMLTokenEnum eStyle = XML_SOLID;
                            bool bNoBorder = false;
                            switch (pLine->GetBorderLineStyle())
                            {
                                case SvxBorderLineStyle::SOLID:
                                    eStyle = XML_SOLID;
                                    break;
                                case SvxBorderLineStyle::DOTTED:
                                    eStyle = XML_DOTTED;
                                    break;
                                case SvxBorderLineStyle::DASHED:
                                    eStyle = XML_DASHED;
                                    break;
                                case SvxBorderLineStyle::FINE_DASHED:
                                    eStyle = XML_FINE_DASHED;
                                    break;
                                case SvxBorderLineStyle::DASH_DOT:
                                    eStyle = XML_DASH_DOT;
                                    break;
                                case SvxBorderLineStyle::DASH_DOT_DOT:
                                    eStyle = XML_DASH_DOT_DOT;
                                    break;
                                case SvxBorderLineStyle::DOUBLE_THIN:
                                    eStyle = XML_DOUBLE_THIN;
                                    break;
                                case SvxBorderLineStyle::DOUBLE:
                                case SvxBorderLineStyle::THINTHICK_SMALLGAP:
                                case SvxBorderLineStyle::THINTHICK_MEDIUMGAP:
                                case SvxBorderLineStyle::THINTHICK_LARGEGAP:
                                case SvxBorderLineStyle::THICKTHIN_SMALLGAP:
                                case SvxBorderLineStyle::THICKTHIN_MEDIUMGAP:
                                case SvxBorderLineStyle::THICKTHIN_LARGEGAP:
                                    eStyle = XML_DOUBLE;
                                    break;
                                case SvxBorderLineStyle::EMBOSSED:
                                    eStyle = XML_RIDGE;
                                    break;
                                case SvxBorderLineStyle::ENGRAVED:
                                    eStyle = XML_GROOVE;
                                    break;
                                case SvxBorderLineStyle::INSET:
                                    eStyle = XML_INSET;
                                    break;
                                case SvxBorderLineStyle::OUTSET:
                                    eStyle = XML_OUTSET;
                                    break;
                                default:
                                    bNoBorder = true;
                            }

                            if ( !bNoBorder )
                            {
                                ::sax::Converter::convertMeasure(aOut, nWidth,
                                       util::MeasureUnit::TWIP,
                                       util::MeasureUnit::POINT);
                                aOut.append( ' ' );
                                aOut.append( GetXMLToken( eStyle ) );
                                aOut.append( ' ' );
                                ::sax::Converter::convertColor(aOut,
                                        pLine->GetColor());
                            }
                        }
                        else
                        {
                            aOut.append( GetXMLToken(XML_NONE) );
                        }
                    }
                    break;

                    // width
                    case ALL_BORDER_LINE_WIDTH:
                    case LEFT_BORDER_LINE_WIDTH:
                    case RIGHT_BORDER_LINE_WIDTH:
                    case TOP_BORDER_LINE_WIDTH:
                    case BOTTOM_BORDER_LINE_WIDTH:
                        const SvxBorderLine* pLine;
                        switch( nMemberId )
                        {
                        case ALL_BORDER_LINE_WIDTH:
                        case LEFT_BORDER_LINE_WIDTH:
                            pLine = pLeft;
                            break;
                        case RIGHT_BORDER_LINE_WIDTH:
                            pLine = pRight;
                            break;
                        case TOP_BORDER_LINE_WIDTH:
                            pLine = pTop;
                            break;
                        case BOTTOM_BORDER_LINE_WIDTH:
                            pLine = pBottom;
                            break;
                        default:
                            return false;
                        }
                        rUnitConverter.convertMeasureToXML( aOut, pLine->GetInWidth() );
                        aOut.append( ' ' );
                        rUnitConverter.convertMeasureToXML( aOut, pLine->GetDistance() );
                        aOut.append( ' ' );
                        rUnitConverter.convertMeasureToXML( aOut, pLine->GetOutWidth() );
                        break;
                }
                bOk = true;
            }
        }
        break;

        case RES_BREAK:
        {
            const SvxFormatBreakItem& rFormatBreak = dynamic_cast<const SvxFormatBreakItem&>(rItem);

            sal_uInt16 eEnum = 0;

            switch( nMemberId )
            {
            case MID_BREAK_BEFORE:
                switch (rFormatBreak.GetBreak())
                {
                    case SvxBreak::ColumnBefore:
                        eEnum = 1;
                        break;
                    case SvxBreak::PageBefore:
                        eEnum = 2;
                        break;
                    case SvxBreak::NONE:
                        eEnum = 0;
                        break;
                    default:
                        return false;
                }
                break;
            case MID_BREAK_AFTER:
                switch (rFormatBreak.GetBreak())
                {
                    case SvxBreak::ColumnAfter:
                        eEnum = 1;
                        break;
                    case SvxBreak::PageAfter:
                        eEnum = 2;
                        break;
                    case SvxBreak::NONE:
                        eEnum = 0;
                        break;
                    default:
                        return false;
                }
                break;
            }

            bOk = SvXMLUnitConverter::convertEnum( aOut, eEnum, psXML_BreakType );
        }
        break;

        case RES_KEEP:
        {
            const SvxFormatKeepItem* pFormatKeep = dynamic_cast<const SvxFormatKeepItem*>( &rItem );
            assert(pFormatKeep && "Wrong Which-ID");
            if (pFormatKeep)
            {
                aOut.append( pFormatKeep->GetValue()
                             ? GetXMLToken( XML_ALWAYS )
                             : GetXMLToken( XML_AUTO ) );
                bOk = true;
            }
        }
        break;

        case RES_BACKGROUND:
        {
            const SvxBrushItem& rBrush = dynamic_cast<const SvxBrushItem&>(rItem);

            // note: the graphic is only exported if nMemberId equals
            //       MID_GRAPHIC..
            //       If not, only the color or transparency is exported

            switch( nMemberId )
            {
                case MID_BACK_COLOR:
                    if ( rBrush.GetColor().GetTransparency() )
                        aOut.append( GetXMLToken(XML_TRANSPARENT) );
                    else
                    {
                        ::sax::Converter::convertColor(aOut,
                                rBrush.GetColor());
                    }
                    bOk = true;
                    break;

                case MID_GRAPHIC_POSITION:
                    switch (rBrush.GetGraphicPos())
                    {
                    case GPOS_LT:
                    case GPOS_MT:
                    case GPOS_RT:
                        aOut.append( GetXMLToken(XML_TOP) );
                        bOk = true;
                        break;
                    case GPOS_LM:
                    case GPOS_MM:
                    case GPOS_RM:
                        aOut.append( GetXMLToken(XML_CENTER) );
                        bOk = true;
                        break;
                    case GPOS_LB:
                    case GPOS_MB:
                    case GPOS_RB:
                        aOut.append( GetXMLToken(XML_BOTTOM) );
                        bOk = true;
                        break;
                    default:
                        ;
                    }

                    if( bOk )
                    {
                        aOut.append( ' ' );

                        switch (rBrush.GetGraphicPos())
                        {
                        case GPOS_LT:
                        case GPOS_LB:
                        case GPOS_LM:
                            aOut.append( GetXMLToken(XML_LEFT) );
                            break;
                        case GPOS_MT:
                        case GPOS_MM:
                        case GPOS_MB:
                            aOut.append( GetXMLToken(XML_CENTER) );
                            break;
                        case GPOS_RM:
                        case GPOS_RT:
                        case GPOS_RB:
                            aOut.append( GetXMLToken(XML_RIGHT) );
                            break;
                        default:
                            ;
                        }
                    }
                    break;

                case MID_GRAPHIC_REPEAT:
                {
                    SvxGraphicPosition eGraphicPos = rBrush.GetGraphicPos();
                    if( GPOS_AREA == eGraphicPos )
                    {
                        aOut.append( GetXMLToken(XML_BACKGROUND_STRETCH)  );
                        bOk = true;
                    }
                    else if( GPOS_NONE != eGraphicPos && GPOS_TILED != eGraphicPos  )
                    {
                        aOut.append( GetXMLToken(XML_BACKGROUND_NO_REPEAT) );
                        bOk = true;
                    }
                }
                break;

                case MID_GRAPHIC_FILTER:
                    if (rBrush.GetGraphicPos() != GPOS_NONE &&
                        !rBrush.GetGraphicFilter().isEmpty())
                    {
                        aOut.append(rBrush.GetGraphicFilter());
                        bOk = true;
                    }
                    break;
            }
        }
        break;

        case RES_PAGEDESC:
        {
            const SwFormatPageDesc& rPageDesc = dynamic_cast<const SwFormatPageDesc&>(rItem);

            if( MID_PAGEDESC_PAGENUMOFFSET==nMemberId )
            {
                ::boost::optional<sal_uInt16> oNumOffset = rPageDesc.GetNumOffset();
                if (oNumOffset && oNumOffset.get() > 0)
                {
                    // #i114163# positiveInteger only!
                    sal_Int32 const number(oNumOffset.get());
                    aOut.append(number);
                }
                else
                {
                    aOut.append(GetXMLToken(XML_AUTO));
                }
                bOk = true;
            }
        }
        break;

        case RES_LAYOUT_SPLIT:
        case RES_ROW_SPLIT:
        {
            const SfxBoolItem* pSplit = dynamic_cast<const SfxBoolItem*>( &rItem );
            assert(pSplit && "Wrong Which-ID");
            if (pSplit)
            {
                ::sax::Converter::convertBool( aOut, pSplit->GetValue() );
                bOk = true;
            }
        }
        break;

        case RES_HORI_ORIENT:
        {
            const SwFormatHoriOrient* pHoriOrient = dynamic_cast<const SwFormatHoriOrient*>( &rItem );
            assert(pHoriOrient && "Wrong Which-ID");
            if (pHoriOrient)
            {
                SvXMLUnitConverter::convertEnum( aOut, pHoriOrient->GetHoriOrient(),
                                            aXMLTableAlignMap );
                bOk = true;
            }
        }
        break;

        case RES_VERT_ORIENT:
        {
            const SwFormatVertOrient* pVertOrient = dynamic_cast<const SwFormatVertOrient*>( &rItem );
            assert(pVertOrient && "Wrong Which-ID");

            SvXMLUnitConverter::convertEnum( aOut, pVertOrient->GetVertOrient(),
                                        aXMLTableVAlignMap );
            bOk = true;
        }
        break;

        case RES_FRM_SIZE:
        {
            const SwFormatFrameSize& rFrameSize = dynamic_cast<const SwFormatFrameSize&>(rItem);

            bool bOutHeight = false;
            switch( nMemberId )
            {
                case MID_FRMSIZE_REL_WIDTH:
                    if (rFrameSize.GetWidthPercent())
                    {
                        ::sax::Converter::convertPercent(
                                aOut, rFrameSize.GetWidthPercent() );
                        bOk = true;
                    }
                    break;
                case MID_FRMSIZE_MIN_HEIGHT:
                    if( ATT_MIN_SIZE == rFrameSize.GetHeightSizeType() )
                        bOutHeight = true;
                    break;
                case MID_FRMSIZE_FIX_HEIGHT:
                    if( ATT_FIX_SIZE == rFrameSize.GetHeightSizeType() )
                        bOutHeight = true;
                    break;
            }

            if( bOutHeight )
            {
                rUnitConverter.convertMeasureToXML(aOut, rFrameSize.GetHeight());
                bOk = true;
            }
        }
        break;

        case RES_FRAMEDIR:
        {
            Any aAny;
            bOk = rItem.QueryValue( aAny );
            if( bOk )
            {
                std::unique_ptr<XMLPropertyHandler> pWritingModeHandler =
                    XMLPropertyHandlerFactory::CreatePropertyHandler(
                        XML_TYPE_TEXT_WRITING_MODE_WITH_DEFAULT );
                OUString sValue;
                bOk = pWritingModeHandler->exportXML( sValue, aAny,
                                                      rUnitConverter );
                if( bOk )
                    aOut.append( sValue );
            }
        }
        break;

        case RES_COLLAPSING_BORDERS:
        {
            const SfxBoolItem* pBorders = dynamic_cast<const SfxBoolItem*>( &rItem );
            assert(pBorders && "Wrong RES-ID");
            if (pBorders)
            {
                aOut.append( pBorders->GetValue()
                             ? GetXMLToken( XML_COLLAPSING )
                             : GetXMLToken( XML_SEPARATING ) );
                bOk = true;
            }
        }
        break;

        default:
            OSL_FAIL("GetXMLValue not implemented for this item.");
            break;
    }

    if ( bOk )
        rValue = aOut.makeStringAndClear();

    return bOk;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
