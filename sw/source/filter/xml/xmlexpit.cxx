/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include "xmlexpit.hxx"

#include <rtl/ustrbuf.hxx>
#include <sax/tools/converter.hxx>
#include <svl/itempool.hxx>
#include <svl/poolitem.hxx>
#include <svl/itemset.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/attrlist.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <editeng/xmlcnitm.hxx>
#include <xmloff/xmlexp.hxx>
#include <editeng/memberids.hrc>
#include "hintids.hxx"
#include "unomid.h"
#include <svx/unomid.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/shaditem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/brkitem.hxx>
#include <editeng/keepitem.hxx>
#include <editeng/brshitem.hxx>
#include "fmtpdsc.hxx"
#include "fmtornt.hxx"
#include "fmtfsize.hxx"

#include "fmtlsplt.hxx"
#include "xmlithlp.hxx"

#include "fmtrowsplt.hxx"


using ::editeng::SvxBorderLine;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using namespace ::com::sun::star;
using namespace ::xmloff::token;
using uno::Any;

/** fills the given attribute list with the items in the given set */
void SvXMLExportItemMapper::exportXML( const SvXMLExport& rExport,
                                SvXMLAttributeList& rAttrList,
                                const SfxItemSet& rSet,
                                const SvXMLUnitConverter& rUnitConverter,
                                const SvXMLNamespaceMap& rNamespaceMap,
                                sal_uInt16 nFlags,
                                std::vector<sal_uInt16> *pIndexArray ) const
{
    const sal_uInt16 nCount = mrMapEntries->getCount();
    sal_uInt16 nIndex = 0;

    while( nIndex < nCount )
    {
        SvXMLItemMapEntry* pEntry = mrMapEntries->getByIndex( nIndex );

        // we have a valid map entry here, so lets use it...
        if( 0 == (pEntry->nMemberId & MID_SW_FLAG_NO_ITEM_EXPORT) )
        {
            const SfxPoolItem* pItem = GetItem( rSet, pEntry->nWhichId,
                                                nFlags );
            // do we have an item?
            if(pItem)
            {
                if( 0 != (pEntry->nMemberId & MID_SW_FLAG_ELEMENT_ITEM_EXPORT) )
                {
                    // element items do not add any properties,
                    // we export it later
                    if( pIndexArray )
                        pIndexArray->push_back( nIndex );

                }
                else
                {
                    exportXML( rExport, rAttrList, *pItem, *pEntry, rUnitConverter,
                                  rNamespaceMap, nFlags, &rSet );
                }
            }
        }
        else
        {
            handleNoItem( rAttrList, *pEntry, rUnitConverter, rNamespaceMap,
                          rSet );
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
                                 sal_uInt16 /*nFlags*/,
                                 const SfxItemSet *pSet ) const
{
    if( 0 != (rEntry.nMemberId & MID_SW_FLAG_SPECIAL_ITEM_EXPORT) )
    {
        if( rItem.ISA( SwFmtRowSplit ) )
        {
            OUString aValue;
            bool bAddAttribute = true;
            if( rEntry.nNameSpace == XML_NAMESPACE_STYLE )
            {
                if( (rExport.getExportFlags() & EXPORT_SAVEBACKWARDCOMPATIBLE ) == 0 ||
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
                const SfxBoolItem* pSplit = PTR_CAST(SfxBoolItem, &rItem);
                OSL_ENSURE( pSplit != NULL, "Wrong Which-ID" );
                sal_uInt16 eEnum = pSplit->GetValue() ? 1 : 0;
                rUnitConverter.convertEnum( aOut, eEnum, aXML_KeepTogetherType );
                aValue = aOut.makeStringAndClear();
            }
            if( bAddAttribute )
            {
                OUString sName( rNamespaceMap.GetQNameByKey( rEntry.nNameSpace,
                                GetXMLToken(rEntry.eLocalName) ) );
                rAttrList.AddAttribute( sName, aValue );
            }
        }
        if( rItem.ISA( SvXMLAttrContainerItem ) )
        {
            SvXMLNamespaceMap *pNewNamespaceMap = 0;
            const SvXMLNamespaceMap *pNamespaceMap = &rNamespaceMap;

            const SvXMLAttrContainerItem *pUnknown =
                PTR_CAST( SvXMLAttrContainerItem, &rItem );

            sal_uInt16 nCount = pUnknown->GetAttrCount();
            OUStringBuffer sName;
            for( sal_uInt16 i=0; i < nCount; i++ )
            {
                OUString sPrefix( pUnknown->GetAttrPrefix( i ) );
                if( !sPrefix.isEmpty() )
                {
                    OUString sNamespace( pUnknown->GetAttrNamespace( i ) );

                    // if the prefix isn't defined yet or has another meaning,
                    // we have to redefine it now.
                    sal_uInt16 nIdx =   pNamespaceMap->GetIndexByPrefix( sPrefix );
                    if( USHRT_MAX == nIdx ||
                        pNamespaceMap->GetNameByIndex( nIdx ) != sNamespace )
                    {
                        if( !pNewNamespaceMap )
                        {
                            pNewNamespaceMap =
                                        new SvXMLNamespaceMap( rNamespaceMap );
                            pNamespaceMap = pNewNamespaceMap;
                        }
                        pNewNamespaceMap->Add( sPrefix, sNamespace );

                        sName.append( GetXMLToken(XML_XMLNS) );
                        sName.append( sal_Unicode(':') );
                        sName.append( sPrefix );
                        rAttrList.AddAttribute( sName.makeStringAndClear(),
                                                sNamespace );
                    }

                    sName.append( sPrefix );
                    sName.append( sal_Unicode(':') );
                }

                sName.append( pUnknown->GetAttrLName( i ) );
                rAttrList.AddAttribute( sName.makeStringAndClear(),
                                        pUnknown->GetAttrValue(i) );
            }

            delete pNewNamespaceMap;
        }
        else
        {
            handleSpecialItem( rAttrList, rEntry, rItem, rUnitConverter,
                                  rNamespaceMap, pSet );
        }
    }
    else if( 0 == (rEntry.nMemberId & MID_SW_FLAG_ELEMENT_ITEM_EXPORT) )
    {
        OUString aValue;
        if( QueryXMLValue(rItem, aValue,
                          static_cast< sal_uInt16 >(
                                          rEntry.nMemberId & MID_SW_FLAG_MASK ),
                             rUnitConverter ) )
        {
            OUString sName(
                rNamespaceMap.GetQNameByKey( rEntry.nNameSpace,
                                             GetXMLToken(rEntry.eLocalName)));
            rAttrList.AddAttribute( sName, aValue );
        }
    }
}

void SvXMLExportItemMapper::exportElementItems(
                          SvXMLExport& rExport,
                          const SvXMLUnitConverter& rUnitConverter,
                          const SfxItemSet &rSet,
                          sal_uInt16 nFlags,
                          const std::vector<sal_uInt16> &rIndexArray ) const
{
    const sal_uInt16 nCount = rIndexArray.size();

    sal_Bool bItemsExported = sal_False;
    for( sal_uInt16 nIndex = 0; nIndex < nCount; nIndex++ )
    {
        const sal_uInt16 nElement = rIndexArray[ nIndex ];
        SvXMLItemMapEntry* pEntry = mrMapEntries->getByIndex( nElement );
        OSL_ENSURE( 0 != (pEntry->nMemberId & MID_SW_FLAG_ELEMENT_ITEM_EXPORT),
                    "wrong mid flag!" );

        const SfxPoolItem* pItem = GetItem( rSet, pEntry->nWhichId, nFlags );
        // do we have an item?
        if(pItem)
        {
            rExport.IgnorableWhitespace();
            handleElementItem( rExport, *pEntry, *pItem, rUnitConverter,
                               rSet, nFlags);
            bItemsExported = sal_True;
        }
    }

    if( bItemsExported )
        rExport.IgnorableWhitespace();
}

/** returns the item with the givin WhichId from the given ItemSet if its
    set or its default item if its not set and the XML_EXPORT_FLAG_DEEP
    is set in the flags
*/
const SfxPoolItem* SvXMLExportItemMapper::GetItem( const SfxItemSet& rSet,
                                                   sal_uInt16 nWhichId,
                                                   sal_uInt16 nFlags )
{
    // first get item from itemset
    const SfxPoolItem* pItem;
    SfxItemState eState =
        rSet.GetItemState( nWhichId,
                           ( nFlags & XML_EXPORT_FLAG_DEEP ) != 0,
                           &pItem );

    if( SFX_ITEM_SET == eState )
    {
        return pItem;
    }
    else if( (nFlags & XML_EXPORT_FLAG_DEFAULTS) != 0 &&
              SFX_WHICH_MAX > nWhichId )
    {
        // if its not set, try the pool if we export defaults
        return &rSet.GetPool()->GetDefaultItem(nWhichId);
    }
    else
    {
        return NULL;
    }
}

SvXMLExportItemMapper::SvXMLExportItemMapper( SvXMLItemMapEntriesRef rMapEntries )
{
    mrMapEntries = rMapEntries;
}

SvXMLExportItemMapper::~SvXMLExportItemMapper()
{
}

void SvXMLExportItemMapper::exportXML( SvXMLExport& rExport,
                    const SfxItemSet& rSet,
                    const SvXMLUnitConverter& rUnitConverter,
                    XMLTokenEnum ePropToken,
                    sal_uInt16 nFlags ) const
{
    std::vector<sal_uInt16> aIndexArray;

    exportXML( rExport, rExport.GetAttrList(), rSet, rUnitConverter,
               rExport.GetNamespaceMap(), nFlags, &aIndexArray );

    if( rExport.GetAttrList().getLength() > 0L ||
        (nFlags & XML_EXPORT_FLAG_EMPTY) != 0 ||
        !aIndexArray.empty() )
    {
        if( (nFlags & XML_EXPORT_FLAG_IGN_WS) != 0 )
        {
            rExport.IgnorableWhitespace();
        }

        SvXMLElementExport aElem( rExport, XML_NAMESPACE_STYLE, ePropToken,
                                  sal_False, sal_False );
        exportElementItems( rExport, rUnitConverter,
                            rSet, nFlags, aIndexArray );
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
    MID_SW_FLAG_NO_ITEM_EXPORT flag set */
void SvXMLExportItemMapper::handleNoItem( SvXMLAttributeList& /*rAttrList*/,
                               const SvXMLItemMapEntry& /*rEntry*/,
                               const SvXMLUnitConverter& /*rUnitConverter*/,
                               const SvXMLNamespaceMap& /*rNamespaceMap*/,
                               const SfxItemSet& /*rSet*/ ) const
{
    OSL_FAIL( "no item not handled in xml export" );
}

/** this method is called for every item that has the
    MID_SW_FLAG_ELEMENT_EXPORT flag set */
void SvXMLExportItemMapper::handleElementItem(
                        SvXMLExport& /*rExport*/,
                        const SvXMLItemMapEntry& /*rEntry*/,
                        const SfxPoolItem& /*rItem*/,
                        const SvXMLUnitConverter& /*rUnitConverter*/,
                        const SfxItemSet& /*rSet*/,
                        sal_uInt16 /*nFlags*/ ) const
{
    OSL_FAIL( "element item not handled in xml export" );
}

static bool lcl_isOdfDoubleLine( const SvxBorderLine* pLine )
{
    bool bIsOdfDouble = false;
    switch (pLine->GetBorderLineStyle())
    {
        case table::BorderLineStyle::DOUBLE:
        case table::BorderLineStyle::THINTHICK_SMALLGAP:
        case table::BorderLineStyle::THINTHICK_MEDIUMGAP:
        case table::BorderLineStyle::THINTHICK_LARGEGAP:
        case table::BorderLineStyle::THICKTHIN_SMALLGAP:
        case table::BorderLineStyle::THICKTHIN_MEDIUMGAP:
        case table::BorderLineStyle::THICKTHIN_LARGEGAP:
            bIsOdfDouble = true;
            break;
        default:
            break;
    }
    return bIsOdfDouble;
}

sal_Bool SvXMLExportItemMapper::QueryXMLValue(
    const SfxPoolItem& rItem,
    OUString& rValue,
    sal_uInt16 nMemberId,
    const SvXMLUnitConverter& rUnitConverter )
{
    sal_Bool bOk = sal_False;
    OUStringBuffer aOut;

    switch ( rItem.Which() )
    {

        case RES_LR_SPACE:
        {
            const SvxLRSpaceItem* pLRSpace = PTR_CAST(SvxLRSpaceItem, &rItem);
            OSL_ENSURE( pLRSpace != NULL, "Wrong Which-ID!" );

            bOk = sal_True;
            switch( nMemberId )
            {
                case  MID_L_MARGIN:
                    if(pLRSpace->GetPropLeft() != 100)
                    {
                        ::sax::Converter::convertPercent(
                                aOut, pLRSpace->GetPropLeft() );
                    }
                    else
                    {
                        rUnitConverter.convertMeasureToXML(
                                aOut, pLRSpace->GetLeft() );
                    }
                    break;

                case  MID_R_MARGIN:
                    if(pLRSpace->GetPropRight() != 100)
                    {
                        ::sax::Converter::convertPercent(
                                aOut, pLRSpace->GetPropRight() );
                    }
                    else
                    {
                        rUnitConverter.convertMeasureToXML(
                                aOut, pLRSpace->GetRight() );
                    }
                    break;

                case MID_FIRST_AUTO:
                    if( pLRSpace->IsAutoFirst() )
                    {
                        ::sax::Converter::convertBool(
                                aOut, pLRSpace->IsAutoFirst() );
                    }
                    else
                        bOk = sal_False;
                    break;

                case  MID_FIRST_LINE_INDENT:
                    if( !pLRSpace->IsAutoFirst() )
                    {
                        if(pLRSpace->GetPropTxtFirstLineOfst() != 100)
                        {
                            ::sax::Converter::convertPercent(
                                aOut, pLRSpace->GetPropTxtFirstLineOfst() );
                        }
                        else
                        {
                            rUnitConverter.convertMeasureToXML(
                                    aOut, pLRSpace->GetTxtFirstLineOfst() );
                        }
                    }
                    else
                        bOk = sal_False;
                    break;

                default:
                    OSL_FAIL( "unknown member id!");
                    bOk = sal_False;
                    break;
            }
        }
        break;

        case RES_UL_SPACE:
        {
            const SvxULSpaceItem* pULSpace = PTR_CAST(SvxULSpaceItem, &rItem);
            OSL_ENSURE( pULSpace != NULL, "Wrong Which-ID!" );

            switch( nMemberId )
            {
                case MID_UP_MARGIN:
                    if( pULSpace->GetPropUpper() != 100 )
                    {
                        ::sax::Converter::convertPercent(
                                aOut, pULSpace->GetPropUpper() );
                    }
                    else
                    {
                        rUnitConverter.convertMeasureToXML(
                                aOut, pULSpace->GetUpper() );
                    }
                    break;

                case MID_LO_MARGIN:
                    if( pULSpace->GetPropLower() != 100 )
                    {
                        ::sax::Converter::convertPercent(
                                aOut, pULSpace->GetPropLower() );
                    }
                    else
                    {
                        rUnitConverter.convertMeasureToXML(
                                aOut, pULSpace->GetLower() );
                    }
                    break;

                default:
                    OSL_FAIL("unknown MemberId");
            };

            bOk = sal_True;
        }
        break;

        case RES_SHADOW:
        {
            const SvxShadowItem* pShadow = PTR_CAST(SvxShadowItem, &rItem);
            OSL_ENSURE( pShadow != NULL, "Wrong Which-ID" );

            sal_Int32 nX = 1, nY = 1;
            switch( pShadow->GetLocation() )
                {
                case SVX_SHADOW_TOPLEFT:
                    nX = -1;
                    nY = -1;
                    break;
                case SVX_SHADOW_TOPRIGHT:
                    nY = -1;
                    break;
                case SVX_SHADOW_BOTTOMLEFT:
                    nX = -1;
                    break;
                case SVX_SHADOW_BOTTOMRIGHT:
                    break;
                case SVX_SHADOW_NONE:
                default:
                    rValue = GetXMLToken(XML_NONE);
                    return sal_True;
                }

            nX *= pShadow->GetWidth();
            nY *= pShadow->GetWidth();

            ::sax::Converter::convertColor(aOut, pShadow->GetColor().GetColor());
            aOut.append( sal_Unicode(' ') );
            rUnitConverter.convertMeasureToXML( aOut, nX );
            aOut.append( sal_Unicode(' ') );
            rUnitConverter.convertMeasureToXML( aOut, nY );

            bOk = sal_True;
        }
        break;

        case RES_BOX:
        {
            SvxBoxItem* pBox = PTR_CAST(SvxBoxItem, &rItem);
            OSL_ENSURE( pBox != NULL, "Wrong WHich-ID" );

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
            sal_uInt16 nTopDist     = pBox->GetDistance( BOX_LINE_TOP );
            sal_uInt16 nBottomDist  = pBox->GetDistance( BOX_LINE_BOTTOM );
            sal_uInt16 nLeftDist    = pBox->GetDistance( BOX_LINE_LEFT );
            sal_uInt16 nRightDist   = pBox->GetDistance( BOX_LINE_RIGHT );


            // check if we need to export it
            switch( nMemberId )
            {
                case ALL_BORDER_PADDING:
                case LEFT_BORDER_PADDING:
                case RIGHT_BORDER_PADDING:
                case TOP_BORDER_PADDING:
                case BOTTOM_BORDER_PADDING:
                {
                    sal_Bool bEqual = nLeftDist == nRightDist &&
                                      nLeftDist == nTopDist &&
                                      nLeftDist == nBottomDist;
                    // don't export individual paddings if all paddings are equal and
                    // don't export all padding if some paddings are not equal
                    if( (bEqual && ALL_BORDER_PADDING != nMemberId) ||
                        (!bEqual && ALL_BORDER_PADDING == nMemberId) )
                        return sal_False;
                }
                break;
                case ALL_BORDER:
                case LEFT_BORDER:
                case RIGHT_BORDER:
                case TOP_BORDER:
                case BOTTOM_BORDER:
                {
                    sal_Bool bEqual = ( NULL == pTop && NULL == pBottom &&
                                        NULL == pLeft && NULL == pRight ) ||
                                      ( pTop && pBottom && pLeft && pRight &&
                                       *pTop == *pBottom  && *pTop == *pLeft &&
                                        *pTop == *pRight );

                    // don't export individual borders if all are the same and
                    // don't export all borders if some are not equal
                    if( (bEqual && ALL_BORDER != nMemberId) ||
                        (!bEqual && ALL_BORDER == nMemberId) )
                        return sal_False;
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
                        return sal_False;

                    sal_Bool bEqual = NULL != pTop &&
                                      NULL != pBottom &&
                                      NULL != pLeft &&
                                      NULL != pRight;

                    if( bEqual )
                    {
                        const sal_uInt16 nDistance = pTop->GetDistance();
                        const sal_uInt16 nInWidth  = pTop->GetInWidth();
                        const sal_uInt16 nOutWidth = pTop->GetOutWidth();
                        const sal_uInt16 nWidth = pTop->GetWidth();

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
                                return sal_False;
                            break;
                        case LEFT_BORDER_LINE_WIDTH:
                            if( bEqual || NULL == pLeft ||
                                0 == pLeft->GetDistance() ||
                                !lcl_isOdfDoubleLine( pLeft ) )
                                return sal_False;
                            break;
                        case RIGHT_BORDER_LINE_WIDTH:
                            if( bEqual || NULL == pRight ||
                                0 == pRight->GetDistance() ||
                                !lcl_isOdfDoubleLine( pRight ) )
                                return sal_False;
                            break;
                        case TOP_BORDER_LINE_WIDTH:
                            if( bEqual || NULL == pTop ||
                                0 == pTop->GetDistance() ||
                                !lcl_isOdfDoubleLine( pTop ) )
                                return sal_False;
                            break;
                        case BOTTOM_BORDER_LINE_WIDTH:
                            if( bEqual || NULL == pBottom ||
                                0 == pBottom->GetDistance() ||
                                !lcl_isOdfDoubleLine( pBottom ) )
                                return sal_False;
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
                        pLine = NULL;
                        break;
                    }

                    if( NULL != pLine )
                    {
                        sal_Int32 nWidth = pLine->GetWidth();

                        enum XMLTokenEnum eStyle = XML_SOLID;
                        bool bNoBorder = false;
                        switch (pLine->GetBorderLineStyle())
                        {
                            case table::BorderLineStyle::SOLID:
                                eStyle = XML_SOLID;
                                break;
                            case table::BorderLineStyle::DOTTED:
                                eStyle = XML_DOTTED;
                                break;
                            case table::BorderLineStyle::DASHED:
                                eStyle = XML_DASHED;
                                break;
                            case table::BorderLineStyle::DOUBLE:
                            case table::BorderLineStyle::THINTHICK_SMALLGAP:
                            case table::BorderLineStyle::THINTHICK_MEDIUMGAP:
                            case table::BorderLineStyle::THINTHICK_LARGEGAP:
                            case table::BorderLineStyle::THICKTHIN_SMALLGAP:
                            case table::BorderLineStyle::THICKTHIN_MEDIUMGAP:
                            case table::BorderLineStyle::THICKTHIN_LARGEGAP:
                                eStyle = XML_DOUBLE;
                                break;
                            case table::BorderLineStyle::EMBOSSED:
                                eStyle = XML_RIDGE;
                                break;
                            case table::BorderLineStyle::ENGRAVED:
                                eStyle = XML_GROOVE;
                                break;
                            case table::BorderLineStyle::INSET:
                                eStyle = XML_INSET;
                                break;
                            case table::BorderLineStyle::OUTSET:
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
                            aOut.append( sal_Unicode( ' ' ) );
                            aOut.append( GetXMLToken( eStyle ) );
                            aOut.append( sal_Unicode( ' ' ) );
                            ::sax::Converter::convertColor(aOut,
                                    pLine->GetColor().GetColor());
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
                        return sal_False;
                    }
                    rUnitConverter.convertMeasureToXML( aOut, pLine->GetInWidth() );
                    aOut.append( sal_Unicode( ' ' ) );
                    rUnitConverter.convertMeasureToXML( aOut, pLine->GetDistance() );
                    aOut.append( sal_Unicode( ' ' ) );
                    rUnitConverter.convertMeasureToXML( aOut, pLine->GetOutWidth() );
                    break;
                }

            bOk = sal_True;
        }
        break;

        case RES_BREAK:
        {
            const SvxFmtBreakItem* pFmtBreak = PTR_CAST(SvxFmtBreakItem, &rItem);
            OSL_ENSURE( pFmtBreak != NULL, "Wrong Which-ID" );

            sal_uInt16 eEnum = 0;

            switch( nMemberId )
            {
            case MID_BREAK_BEFORE:
                switch( pFmtBreak->GetValue() )
                {
                    case SVX_BREAK_COLUMN_BEFORE:
                        eEnum = 1;
                        break;
                    case SVX_BREAK_PAGE_BEFORE:
                        eEnum = 2;
                        break;
                    case SVX_BREAK_NONE:
                        eEnum = 0;
                        break;
                    default:
                        return sal_False;
                }
                break;
            case MID_BREAK_AFTER:
                switch( pFmtBreak->GetValue() )
                {
                    case SVX_BREAK_COLUMN_AFTER:
                        eEnum = 1;
                        break;
                    case SVX_BREAK_PAGE_AFTER:
                        eEnum = 2;
                        break;
                    case SVX_BREAK_NONE:
                        eEnum = 0;
                        break;
                    default:
                        return sal_False;
                }
                break;
            }

            bOk = rUnitConverter.convertEnum( aOut, eEnum, psXML_BreakType );
        }
        break;

        case RES_KEEP:
        {
            SvxFmtKeepItem* pFmtKeep = PTR_CAST(SvxFmtKeepItem, &rItem);
            OSL_ENSURE( pFmtKeep != NULL, "Wrong Which-ID" );

            aOut.append( pFmtKeep->GetValue()
                         ? GetXMLToken( XML_ALWAYS )
                         : GetXMLToken( XML_AUTO ) );
            bOk = sal_True;
        }
        break;

        case RES_BACKGROUND:
        {
            SvxBrushItem* pBrush = PTR_CAST(SvxBrushItem, &rItem);
            OSL_ENSURE( pBrush != NULL, "Wrong Which-ID" );

            // note: the graphic is only exported if nMemberId equals
            //       MID_GRAPHIC..
            //       If not, only the color or transparency is exported

            switch( nMemberId )
            {
                case MID_BACK_COLOR:
                    if ( pBrush->GetColor().GetTransparency() )
                        aOut.append( GetXMLToken(XML_TRANSPARENT) );
                    else
                    {
                        ::sax::Converter::convertColor(aOut,
                                pBrush->GetColor().GetColor());
                    }
                    bOk = sal_True;
                    break;

                case MID_GRAPHIC_LINK:
                    if( pBrush->GetGraphicPos() != GPOS_NONE )
                    {
                        uno::Any aAny;
                        pBrush->QueryValue( aAny, MID_GRAPHIC_URL );
                        OUString sTmp;
                        aAny >>= sTmp;
                        aOut.append( sTmp );
                        bOk = sal_True;
                    }
                    break;

                case MID_GRAPHIC_POSITION:
                    switch( pBrush->GetGraphicPos() )
                    {
                    case GPOS_LT:
                    case GPOS_MT:
                    case GPOS_RT:
                        aOut.append( GetXMLToken(XML_TOP) );
                        bOk = sal_True;
                        break;
                    case GPOS_LM:
                    case GPOS_MM:
                    case GPOS_RM:
                        aOut.append( GetXMLToken(XML_CENTER) );
                        bOk = sal_True;
                        break;
                    case GPOS_LB:
                    case GPOS_MB:
                    case GPOS_RB:
                        aOut.append( GetXMLToken(XML_BOTTOM) );
                        bOk = sal_True;
                        break;
                    default:
                        ;
                    }

                    if( bOk )
                    {
                        aOut.append( sal_Unicode( ' ' ) );

                        switch( pBrush->GetGraphicPos() )
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
                    SvxGraphicPosition eGraphicPos = pBrush->GetGraphicPos();
                    if( GPOS_AREA == eGraphicPos )
                    {
                        aOut.append( GetXMLToken(XML_BACKGROUND_STRETCH)  );
                        bOk = sal_True;
                    }
                    else if( GPOS_NONE != eGraphicPos && GPOS_TILED != eGraphicPos  )
                    {
                        aOut.append( GetXMLToken(XML_BACKGROUND_NO_REPEAT) );
                        bOk = sal_True;
                    }
                }
                break;

                case MID_GRAPHIC_FILTER:
                    if( pBrush->GetGraphicPos() != GPOS_NONE &&
                        pBrush->GetGraphicFilter() )
                    {
                        aOut.append( pBrush->GetGraphicFilter()->GetBuffer() );
                        bOk = sal_True;
                    }
                    break;
            }
        }
        break;

        case RES_PAGEDESC:
        {
            const SwFmtPageDesc* pPageDesc = PTR_CAST(SwFmtPageDesc, &rItem);
            OSL_ENSURE( pPageDesc != NULL, "Wrong Which-ID" );

            if( MID_PAGEDESC_PAGENUMOFFSET==nMemberId )
            {
                sal_Int32 const number(pPageDesc->GetNumOffset());
                if (0 >= number)
                {
                    aOut.append(GetXMLToken(XML_AUTO));
                }
                else // #i114163# positiveInteger only!
                {
                    ::sax::Converter::convertNumber(aOut, number);
                }
                bOk = sal_True;
            }
        }
        break;

        case RES_LAYOUT_SPLIT:
        case RES_ROW_SPLIT:
        {
            const SfxBoolItem* pSplit = PTR_CAST(SfxBoolItem, &rItem);
            OSL_ENSURE( pSplit != NULL, "Wrong Which-ID" );

            ::sax::Converter::convertBool( aOut, pSplit->GetValue() );
            bOk = sal_True;
        }
        break;

        case RES_HORI_ORIENT:
        {
            SwFmtHoriOrient* pHoriOrient = PTR_CAST(SwFmtHoriOrient, &rItem);
            OSL_ENSURE( pHoriOrient != NULL, "Wrong Which-ID" );

            rUnitConverter.convertEnum( aOut, pHoriOrient->GetHoriOrient(),
                                        aXMLTableAlignMap );
            bOk = sal_True;
        }
        break;

        case RES_VERT_ORIENT:
        {
            SwFmtVertOrient* pVertOrient = PTR_CAST(SwFmtVertOrient, &rItem);
            OSL_ENSURE( pVertOrient != NULL, "Wrong Which-ID" );

            rUnitConverter.convertEnum( aOut, pVertOrient->GetVertOrient(),
                                        aXMLTableVAlignMap );
            bOk = sal_True;
        }
        break;

        case RES_FRM_SIZE:
        {
            SwFmtFrmSize* pFrmSize = PTR_CAST(SwFmtFrmSize, &rItem);
            OSL_ENSURE( pFrmSize != NULL, "Wrong Which-ID" );

            sal_Bool bOutHeight = sal_False;
            switch( nMemberId )
            {
                case MID_FRMSIZE_REL_WIDTH:
                    if( pFrmSize->GetWidthPercent() )
                    {
                        ::sax::Converter::convertPercent(
                                aOut, pFrmSize->GetWidthPercent() );
                        bOk = sal_True;
                    }
                    break;
                case MID_FRMSIZE_MIN_HEIGHT:
                    if( ATT_MIN_SIZE == pFrmSize->GetHeightSizeType() )
                        bOutHeight = sal_True;
                    break;
                case MID_FRMSIZE_FIX_HEIGHT:
                    if( ATT_FIX_SIZE == pFrmSize->GetHeightSizeType() )
                        bOutHeight = sal_True;
                    break;
            }

            if( bOutHeight )
            {
                rUnitConverter.convertMeasureToXML(aOut, pFrmSize->GetHeight());
                bOk = sal_True;
            }
        }
        break;

        case RES_FRAMEDIR:
        {
            Any aAny;
            bOk = rItem.QueryValue( aAny );
            if( bOk )
            {
                const XMLPropertyHandler* pWritingModeHandler =
                    XMLPropertyHandlerFactory::CreatePropertyHandler(
                        XML_TYPE_TEXT_WRITING_MODE_WITH_DEFAULT );
                OUString sValue;
                bOk = pWritingModeHandler->exportXML( sValue, aAny,
                                                      rUnitConverter );
                if( bOk )
                    aOut.append( sValue );

                delete pWritingModeHandler;
            }
        }
        break;

        case RES_COLLAPSING_BORDERS:
        {
            const SfxBoolItem* pBorders = PTR_CAST(SfxBoolItem, &rItem);
            OSL_ENSURE( pBorders != NULL, "Wrong RES-ID" );

            aOut.append( pBorders->GetValue()
                         ? GetXMLToken( XML_COLLAPSING )
                         : GetXMLToken( XML_SEPARATING ) );
            bOk = sal_True;
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
