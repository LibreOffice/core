/*************************************************************************
 *
 *  $RCSfile: xmlexpit.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: mib $ $Date: 2001-07-04 14:14:26 $
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

#ifndef _SVSTDARR_USHORTS
#define _SVSTDARR_USHORTS
#include <svtools/svstdarr.hxx>
#endif

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::xmloff::token;

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
    OUString sCDATA( GetXMLToken(XML_CDATA) );

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
                        rAttrList.AddAttribute( sName.makeStringAndClear(), sCDATA,
                                                sNamespace );
                    }

                    sName.append( sPrefix );
                    sName.append( sal_Unicode(':') );
                }

                sName.append( pUnknown->GetAttrLName( i ) );
                rAttrList.AddAttribute( sName.makeStringAndClear(), sCDATA,
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
        if( rItem.exportXML( aValue, rEntry.nMemberId & MID_FLAG_MASK,
                             rUnitConverter ) )
        {
            OUString sName(
                rNamespaceMap.GetQNameByKey( rEntry.nNameSpace,
                                             GetXMLToken(rEntry.eLocalName)));
            rAttrList.AddAttribute( sName, sCDATA, aValue );
        }
    }
}

void SvXMLExportItemMapper::exportElementItems(
                          const uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler > & rHandler,
                          const SvXMLUnitConverter& rUnitConverter,
                          const SvXMLNamespaceMap& rNamespaceMap,
                          const SfxItemSet &rSet,
                          sal_uInt16 nFlags,
                          const SvUShorts& rIndexArray ) const
{
    const sal_uInt16 nCount = rIndexArray.Count();

    sal_Bool bItemsExported = sal_False;
    OUString sWS( GetXMLToken(XML_WS) );
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
            rHandler->ignorableWhitespace( sWS );
            handleElementItem( rHandler, *pEntry, *pItem, rUnitConverter,
                               rNamespaceMap, rSet, nFlags);
            bItemsExported = sal_True;
        }
    }

    if( bItemsExported )
        rHandler->ignorableWhitespace( sWS );
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

void SvXMLExportItemMapper::exportXML( const uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler > & rHandler,
                    const SfxItemSet& rSet,
                    const SvXMLUnitConverter& rUnitConverter,
                    const SvXMLNamespaceMap& rNamespaceMap,
                    sal_uInt16 nFlags ) const
{
    SvXMLAttributeList *pAttrList = new SvXMLAttributeList();
    uno::Reference< ::com::sun::star::xml::sax::XAttributeList >  xAttrList = pAttrList;

    SvUShorts aIndexArray;

    exportXML( *pAttrList, rSet, rUnitConverter, rNamespaceMap,
               nFlags, &aIndexArray );

    if( pAttrList->getLength() > 0L ||
        (nFlags & XML_EXPORT_FLAG_EMPTY) != 0 ||
        aIndexArray.Count() != 0 )
    {
        if( (nFlags & XML_EXPORT_FLAG_IGN_WS) != 0 )
        {
            OUString sWS( GetXMLToken(XML_WS) );
            rHandler->ignorableWhitespace( sWS );
        }

        OUString sLName( GetXMLToken(XML_PROPERTIES) );
        OUString sName =
            rNamespaceMap.GetQNameByKey( XML_NAMESPACE_STYLE, sLName );
        rHandler->startElement( sName, xAttrList );

        exportElementItems( rHandler, rUnitConverter, rNamespaceMap,
                            rSet, nFlags, aIndexArray );

        rHandler->endElement( sName );
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
                        const uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler > & rHandler,
                        const SvXMLItemMapEntry& rEntry,
                        const SfxPoolItem& rItem,
                        const SvXMLUnitConverter& rUnitConverter,
                        const SvXMLNamespaceMap& rNamespaceMap,
                        const SfxItemSet& rSet,
                        sal_uInt16 nFlags ) const
{
    DBG_ERROR( "element item not handled in xml export" );
}

