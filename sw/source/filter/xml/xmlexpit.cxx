/*************************************************************************
 *
 *  $RCSfile: xmlexpit.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 09:06:49 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "xmlexpit.hxx"

#include <xmloff/xmluconv.hxx>

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _XMLOFF_XMLTOKEN_HXX
#include <xmloff/xmltoken.hxx>
#endif

#ifndef _SFXITEMPOOL_HXX
#include <svtools/itempool.hxx>
#endif

#ifndef _SFXPOOLITEM_HXX
#include <svtools/poolitem.hxx>
#endif

#ifndef _SFXITEMSET_HXX
#include <svtools/itemset.hxx>
#endif

#ifndef _XMLOFF_ATTRLIST_HXX
#include <xmloff/attrlist.hxx>
#endif

#ifndef _XMLOFF_NMSPMAP_HXX
#include <xmloff/nmspmap.hxx>
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include <xmloff/xmlnmspe.hxx>
#endif

#ifndef _SVX_XMLCNITM_HXX
#include <svx/xmlcnitm.hxx>
#endif

#ifndef _XMLOFF_XMLEXP_HXX
#include <xmloff/xmlexp.hxx>
#endif

#ifndef _SVSTDARR_USHORTS
#define _SVSTDARR_USHORTS
#include <svtools/svstdarr.hxx>
#endif



#ifndef _HINTIDS_HXX
#include "hintids.hxx"
#endif

#ifndef SW_UNOMID_HXX
#include "unomid.h"
#endif

#ifndef _SVX_UNOMID_HXX
#include <svx/unomid.hxx>
#endif

#ifndef _SVX_LRSPITEM_HXX
#include <svx/lrspitem.hxx>
#endif

#ifndef _SVX_ULSPITEM_HXX
#include <svx/ulspitem.hxx>
#endif

#ifndef _SVX_SHADITEM_HXX
#include <svx/shaditem.hxx>
#endif

#ifndef _SVX_BOXITEM_HXX
#include <svx/boxitem.hxx>
#endif

#ifndef _SVX_BRKITEM_HXX
#include <svx/brkitem.hxx>
#endif

#ifndef _SVX_KEEPITEM_HXX
#include <svx/keepitem.hxx>
#endif

#ifndef _SVX_BRSHITEM_HXX
#include <svx/brshitem.hxx>
#endif

#ifndef _FMTPDSC_HXX
#include "fmtpdsc.hxx"
#endif

#ifndef _FMTORNT_HXX
#include "fmtornt.hxx"
#endif

#ifndef _FMTFSIZE_HXX
#include "fmtfsize.hxx"
#endif

#ifndef _FMTLSPLT_HXX
#include "fmtlsplt.hxx"
#endif

#ifndef _XMLOFF_XMLTOKEN_HXX
#include <xmloff/xmltoken.hxx>
#endif

#ifndef _XMLOFF_PROPERTYHANDLERFACTORY_HXX
#include <xmloff/prhdlfac.hxx>
#endif

#ifndef _XMLOFF_XMLTYPES_HXX
#include <xmloff/xmltypes.hxx>
#endif

#ifndef _SW_XMLITHLP_HXX
#include "xmlithlp.hxx"
#endif


using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::xmloff::token;
using ::com::sun::star::uno::Any;

/** fills the given attribute list with the items in the given set */
void SvXMLExportItemMapper::exportXML( SvXMLAttributeList& rAttrList,
                                const SfxItemSet& rSet,
                                const SvXMLUnitConverter& rUnitConverter,
                                const SvXMLNamespaceMap& rNamespaceMap,
                                sal_uInt16 nFlags,
                                SvUShorts* pIndexArray ) const
{
    const sal_uInt16 nCount = mrMapEntries->getCount();
    sal_uInt16 nIndex = 0;

    while( nIndex < nCount )
    {
        SvXMLItemMapEntry* pEntry = mrMapEntries->getByIndex( nIndex );

        // we have a valid map entry here, so lets use it...
        if( 0 == (pEntry->nMemberId & MID_FLAG_NO_ITEM_EXPORT) )
        {
            const SfxPoolItem* pItem = GetItem( rSet, pEntry->nWhichId,
                                                nFlags );
            // do we have an item?
            if(pItem)
            {
                if( 0 != (pEntry->nMemberId & MID_FLAG_ELEMENT_ITEM_EXPORT) )
                {
                    // element items do not add any properties,
                    // we export it later
                    if( pIndexArray )
                        pIndexArray->Insert( nIndex, pIndexArray->Count() );

                }
                else
                {
                    exportXML( rAttrList, *pItem, *pEntry, rUnitConverter,
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

void SvXMLExportItemMapper::exportXML( SvXMLAttributeList& rAttrList,
                                 const SfxPoolItem& rItem,
                                 const SvXMLItemMapEntry& rEntry,
                                 const SvXMLUnitConverter& rUnitConverter,
                                 const SvXMLNamespaceMap& rNamespaceMap,
                                 sal_uInt16 nFlags,
                                 const SfxItemSet *pSet ) const
{
    if( 0 != (rEntry.nMemberId & MID_FLAG_SPECIAL_ITEM_EXPORT) )
    {
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
                if( sPrefix.getLength() )
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
    else if( 0 == (rEntry.nMemberId & MID_FLAG_ELEMENT_ITEM_EXPORT) )
    {
        OUString aValue;
        if( QueryXMLValue(rItem, aValue, rEntry.nMemberId & MID_FLAG_MASK,
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
                          const SvUShorts& rIndexArray ) const
{
    const sal_uInt16 nCount = rIndexArray.Count();

    sal_Bool bItemsExported = sal_False;
    for( sal_uInt16 nIndex = 0; nIndex < nCount; nIndex++ )
    {
        const sal_uInt16 nElement = rIndexArray.GetObject( nIndex );
        SvXMLItemMapEntry* pEntry = mrMapEntries->getByIndex( nElement );
        DBG_ASSERT( 0 != (pEntry->nMemberId & MID_FLAG_ELEMENT_ITEM_EXPORT),
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
                                                   sal_uInt32 nWhichId,
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

/** fills the given attribute list with the items in the given set */
void SvXMLExportItemMapper::exportXML( SvXMLAttributeList& rAttrList,
                                 const SfxItemSet& rSet,
                                 const SvXMLUnitConverter& rUnitConverter,
                                 const SvXMLNamespaceMap& rNamespaceMap,
                                 sal_uInt16 nFlags /* = 0 */ ) const
{
    exportXML( rAttrList, rSet, rUnitConverter, rNamespaceMap, nFlags, 0 );
}


void SvXMLExportItemMapper::exportXML( SvXMLAttributeList& rAttrList,
                    const SfxPoolItem& rItem,
                    const SvXMLUnitConverter& rUnitConverter,
                    const SvXMLNamespaceMap& rNamespaceMap,
                    sal_uInt16 nFlags ) const
{
    OUString sCDATA( GetXMLToken(XML_CDATA) );

    const sal_uInt16 nWhich = rItem.Which();

    const sal_uInt16 nCount = mrMapEntries->getCount();
    sal_uInt16 nIndex = 0;

    while( nIndex < nCount )
    {
        SvXMLItemMapEntry* pEntry = mrMapEntries->getByIndex( nIndex );
        if( pEntry->nWhichId == nWhich &&
            0 == (pEntry->nMemberId & MID_FLAG_ELEMENT_ITEM_EXPORT) )
            exportXML( rAttrList, rItem, *pEntry,
                       rUnitConverter, rNamespaceMap, nFlags, 0 );

        nIndex++;
    }
}

void SvXMLExportItemMapper::exportXML( SvXMLExport& rExport,
                    const SfxItemSet& rSet,
                    const SvXMLUnitConverter& rUnitConverter,
                    XMLTokenEnum ePropToken,
                    sal_uInt16 nFlags ) const
{
    SvUShorts aIndexArray;

    exportXML( rExport.GetAttrList(), rSet, rUnitConverter,
               rExport.GetNamespaceMap(), nFlags, &aIndexArray );

    if( rExport.GetAttrList().getLength() > 0L ||
        (nFlags & XML_EXPORT_FLAG_EMPTY) != 0 ||
        aIndexArray.Count() != 0 )
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
    MID_FLAG_SPECIAL_ITEM_EXPORT flag set */
void SvXMLExportItemMapper::handleSpecialItem( SvXMLAttributeList& rAttrList,
                                    const SvXMLItemMapEntry& rEntry,
                                    const SfxPoolItem& rItem,
                                    const SvXMLUnitConverter& rUnitConverter,
                                    const SvXMLNamespaceMap& rNamespaceMap,
                                    const SfxItemSet* pSet /* = NULL */ ) const
{
    DBG_ERROR( "special item not handled in xml export" );
}

/** this method is called for every item that has the
    MID_FLAG_NO_ITEM_EXPORT flag set */
void SvXMLExportItemMapper::handleNoItem( SvXMLAttributeList& rAttrList,
                               const SvXMLItemMapEntry& rEntry,
                               const SvXMLUnitConverter& rUnitConverter,
                               const SvXMLNamespaceMap& rNamespaceMap,
                               const SfxItemSet& rSet ) const
{
    DBG_ERROR( "no item not handled in xml export" );
}

/** this method is called for every item that has the
    MID_FLAG_ELEMENT_EXPORT flag set */
void SvXMLExportItemMapper::handleElementItem(
                        SvXMLExport& rExport,
                        const SvXMLItemMapEntry& rEntry,
                        const SfxPoolItem& rItem,
                        const SvXMLUnitConverter& rUnitConverter,
                        const SfxItemSet& rSet,
                        sal_uInt16 nFlags ) const
{
    DBG_ERROR( "element item not handled in xml export" );
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

        case ITEMID_LRSPACE:
        {
            const SvxLRSpaceItem* pLRSpace = PTR_CAST(SvxLRSpaceItem, &rItem);
            DBG_ASSERT( pLRSpace != NULL, "Wrong Which-ID!" );

            bOk = sal_True;
            switch( nMemberId )
            {
                case  MID_L_MARGIN:
                    if(pLRSpace->GetPropLeft() != 100)
                        rUnitConverter.convertPercent( aOut, pLRSpace->GetPropLeft() );
                    else
                        rUnitConverter.convertMeasure( aOut, pLRSpace->GetTxtLeft() );
                    break;

                case  MID_R_MARGIN:
                    if(pLRSpace->GetPropRight() != 100)
                        rUnitConverter.convertPercent( aOut, pLRSpace->GetPropRight() );
                    else
                        rUnitConverter.convertMeasure( aOut, pLRSpace->GetRight() );
                    break;

                case MID_FIRST_AUTO:
                    if( pLRSpace->IsAutoFirst() )
                        rUnitConverter.convertBool( aOut, pLRSpace->IsAutoFirst() );
                    else
                        bOk = sal_False;
                    break;

                case  MID_FIRST_LINE_INDENT:
                    if( !pLRSpace->IsAutoFirst() )
                    {
                        if(pLRSpace->GetPropTxtFirstLineOfst() != 100)
                            rUnitConverter.convertPercent(
                                aOut, pLRSpace->GetPropTxtFirstLineOfst() );
                        else
                            rUnitConverter.convertMeasure( aOut, pLRSpace->GetTxtFirstLineOfst() );
                    }
                    else
                        bOk = sal_False;
                    break;

                default:
                    DBG_ERROR( "unknown member id!");
                    bOk = sal_False;
                    break;
            }
        }
        break;

        case ITEMID_ULSPACE:
        {
            const SvxULSpaceItem* pULSpace = PTR_CAST(SvxULSpaceItem, &rItem);
            DBG_ASSERT( pULSpace != NULL, "Wrong Which-ID!" );

            switch( nMemberId )
            {
                case MID_UP_MARGIN:
                    if( pULSpace->GetPropUpper() != 100 )
                        rUnitConverter.convertPercent( aOut, pULSpace->GetPropUpper() );
                    else
                        rUnitConverter.convertMeasure( aOut, pULSpace->GetUpper() );
                    break;

                case MID_LO_MARGIN:
                    if( pULSpace->GetPropLower() != 100 )
                        rUnitConverter.convertPercent( aOut, pULSpace->GetPropLower() );
                    else
                        rUnitConverter.convertMeasure( aOut, pULSpace->GetLower() );
                    break;

                default:
                    DBG_ERROR("unknown MemberId");
            };

            bOk = sal_True;
        }
        break;

        case ITEMID_SHADOW:
        {
            const SvxShadowItem* pShadow = PTR_CAST(SvxShadowItem, &rItem);
            DBG_ASSERT( pShadow != NULL, "Wrong Which-ID" );

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

            rUnitConverter.convertColor( aOut, pShadow->GetColor() );
            aOut.append( sal_Unicode(' ') );
            rUnitConverter.convertMeasure( aOut, nX );
            aOut.append( sal_Unicode(' ') );
            rUnitConverter.convertMeasure( aOut, nY );

            bOk = sal_True;
        }
        break;

        case ITEMID_BOX:
        {
            SvxBoxItem* pBox = PTR_CAST(SvxBoxItem, &rItem);
            DBG_ASSERT( pBox != NULL, "Wrong WHich-ID" );

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

                        bEqual = nDistance == pLeft->GetDistance() &&
                                 nInWidth  == pLeft->GetInWidth()  &&
                                 nOutWidth == pLeft->GetOutWidth() &&
                                 nDistance == pRight->GetDistance()  &&
                                 nInWidth  == pRight->GetInWidth()   &&
                                 nOutWidth == pRight->GetOutWidth()  &&
                                 nDistance == pBottom->GetDistance()  &&
                                 nInWidth  == pBottom->GetInWidth()   &&
                                 nOutWidth == pBottom->GetOutWidth();
                    }

                    switch( nMemberId )
                    {
                        case ALL_BORDER_LINE_WIDTH:
                            if( !bEqual || pTop->GetDistance() == 0 )
                                return sal_False;
                            break;
                        case LEFT_BORDER_LINE_WIDTH:
                            if( bEqual || NULL == pLeft ||
                                0 == pLeft->GetDistance() )
                                return sal_False;
                            break;
                        case RIGHT_BORDER_LINE_WIDTH:
                            if( bEqual || NULL == pRight ||
                                0 == pRight->GetDistance() )
                                return sal_False;
                            break;
                        case TOP_BORDER_LINE_WIDTH:
                            if( bEqual || NULL == pTop ||
                                0 == pTop->GetDistance() )
                                return sal_False;
                            break;
                        case BOTTOM_BORDER_LINE_WIDTH:
                            if( bEqual || NULL == pBottom ||
                                0 == pBottom->GetDistance() )
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
                    rUnitConverter.convertMeasure( aOut, nLeftDist );
                    break;
                case RIGHT_BORDER_PADDING:
                    rUnitConverter.convertMeasure( aOut, nRightDist );
                    break;
                case TOP_BORDER_PADDING:
                    rUnitConverter.convertMeasure( aOut, nTopDist );
                    break;
                case BOTTOM_BORDER_PADDING:
                    rUnitConverter.convertMeasure( aOut, nBottomDist );
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
                        sal_Int32 nWidth = pLine->GetOutWidth();
                        const sal_uInt16 nDistance = pLine->GetDistance();
                        if( 0 != nDistance )
                        {
                            nWidth += nDistance;
                            nWidth += pLine->GetInWidth();
                        }

                        enum XMLTokenEnum eStyle =
                            (0 == nDistance) ? XML_SOLID : XML_DOUBLE;

                        rUnitConverter.convertMeasure( aOut, nWidth );
                        aOut.append( sal_Unicode( ' ' ) );
                        aOut.append( GetXMLToken( eStyle ) );
                        aOut.append( sal_Unicode( ' ' ) );
                        rUnitConverter.convertColor( aOut, pLine->GetColor() );

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
                    rUnitConverter.convertMeasure( aOut, pLine->GetInWidth() );
                    aOut.append( sal_Unicode( ' ' ) );
                    rUnitConverter.convertMeasure( aOut, pLine->GetDistance() );
                    aOut.append( sal_Unicode( ' ' ) );
                    rUnitConverter.convertMeasure( aOut, pLine->GetOutWidth() );
                    break;
                }

            bOk = sal_True;
        }
        break;

        case ITEMID_FMTBREAK:
        {
            const SvxFmtBreakItem* pFmtBreak = PTR_CAST(SvxFmtBreakItem, &rItem);
            DBG_ASSERT( pFmtBreak != NULL, "Wrong Which-ID" );

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

        case ITEMID_FMTKEEP:
        {
            SvxFmtKeepItem* pFmtKeep = PTR_CAST(SvxFmtKeepItem, &rItem);
            DBG_ASSERT( pFmtKeep != NULL, "Wrong Which-ID" );

            rUnitConverter.convertBool( aOut, pFmtKeep->GetValue() );
            bOk = sal_True;
        }
        break;

        case ITEMID_BRUSH:
        {
            SvxBrushItem* pBrush = PTR_CAST(SvxBrushItem, &rItem);
            DBG_ASSERT( pBrush != NULL, "Wrong Which-ID" );

            // note: the graphic is only exported if nMemberId equals
            //       MID_GRAPHIC..
            //       If not, only the color or transparency is exported

            switch( nMemberId )
            {
                case MID_BACK_COLOR:
                    if ( pBrush->GetColor().GetTransparency() )
                        aOut.append( GetXMLToken(XML_TRANSPARENT) );
                    else
                        rUnitConverter.convertColor( aOut, pBrush->GetColor());
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
            DBG_ASSERT( pPageDesc != NULL, "Wrong Which-ID" );

            if( MID_PAGEDESC_PAGENUMOFFSET==nMemberId )
            {

                rUnitConverter.convertNumber(
                    aOut, (sal_Int32)pPageDesc->GetNumOffset() );
                bOk = sal_True;
            }
        }
        break;

        case RES_LAYOUT_SPLIT:
        case RES_ROW_SPLIT:
        {
            const SfxBoolItem* pSplit = PTR_CAST(SfxBoolItem, &rItem);
            DBG_ASSERT( pSplit != NULL, "Wrong Which-ID" );

            rUnitConverter.convertBool( aOut, pSplit->GetValue() );
            bOk = sal_True;
        }
        break;

        case RES_HORI_ORIENT:
        {
            SwFmtHoriOrient* pHoriOrient = PTR_CAST(SwFmtHoriOrient, &rItem);
            DBG_ASSERT( pHoriOrient != NULL, "Wrong Which-ID" );

            rUnitConverter.convertEnum( aOut, pHoriOrient->GetHoriOrient(),
                                        aXMLTableAlignMap );
            bOk = sal_True;
        }
        break;

        case RES_VERT_ORIENT:
        {
            SwFmtVertOrient* pVertOrient = PTR_CAST(SwFmtVertOrient, &rItem);
            DBG_ASSERT( pVertOrient != NULL, "Wrong Which-ID" );

            rUnitConverter.convertEnum( aOut, pVertOrient->GetVertOrient(),
                                        aXMLTableVAlignMap );
            bOk = sal_True;
        }
        break;

        case RES_FRM_SIZE:
        {
            SwFmtFrmSize* pFrmSize = PTR_CAST(SwFmtFrmSize, &rItem);
            DBG_ASSERT( pFrmSize != NULL, "Wrong Which-ID" );

            sal_Bool bOutHeight = sal_False;
            switch( nMemberId )
            {
                case MID_FRMSIZE_REL_WIDTH:
                    if( pFrmSize->GetWidthPercent() )
                    {
                        rUnitConverter.convertPercent( aOut, pFrmSize->GetWidthPercent() );
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
                rUnitConverter.convertMeasure( aOut, pFrmSize->GetHeight() );
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
            }
        }
        break;

        case RES_COLLAPSING_BORDERS:
        {
            const SfxBoolItem* pBorders = PTR_CAST(SfxBoolItem, &rItem);
            DBG_ASSERT( pBorders != NULL, "Wrong RES-ID" );

            aOut.append( pBorders->GetValue()
                         ? GetXMLToken( XML_COLLAPSING )
                         : GetXMLToken( XML_SEPARATING ) );
            bOk = sal_True;
        }
        break;

        default:
            DBG_ERROR("GetXMLValue not implemented for this item.");
            break;
    }

    if ( bOk )
        rValue = aOut.makeStringAndClear();

    return bOk;
}
