/*************************************************************************
 *
 *  $RCSfile: xmlimpit.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: mib $ $Date: 2001-07-04 14:14:42 $
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

#include "xmlimpit.hxx"
#include <xmloff/xmluconv.hxx>

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

using namespace ::rtl;
using namespace ::com::sun::star;

SvXMLImportItemMapper::SvXMLImportItemMapper(
                                SvXMLItemMapEntriesRef rMapEntries,
                                USHORT nUnknWhich ) :
    mrMapEntries( rMapEntries ),
    nUnknownWhich( nUnknWhich )
{
}

SvXMLImportItemMapper::~SvXMLImportItemMapper()
{
}

/** fills the given itemset with the attributes in the given list */
void SvXMLImportItemMapper::importXML( SfxItemSet& rSet,
                                      uno::Reference< xml::sax::XAttributeList > xAttrList,
                                      const SvXMLUnitConverter& rUnitConverter,
                                       const SvXMLNamespaceMap& rNamespaceMap ) const
{
    INT16 nAttr = xAttrList->getLength();

    SvXMLAttrContainerItem *pUnknownItem = 0;
    for( INT16 i=0; i < nAttr; i++ )
    {
        const OUString& rAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName, aPrefix, aNamespace;
        USHORT nPrefix =
            rNamespaceMap.GetKeyByAttrName( rAttrName, &aPrefix, &aLocalName,
                                            &aNamespace );
        if( XML_NAMESPACE_XMLNS == nPrefix )
            continue;

        const OUString& rValue = xAttrList->getValueByIndex( i );

        // find a map entry for this attribute
        SvXMLItemMapEntry* pEntry = mrMapEntries->getByName( nPrefix, aLocalName );

        if( pEntry )
        {
                // we have a valid map entry here, so lets use it...
                if( 0 == (pEntry->nMemberId & (MID_FLAG_NO_ITEM_IMPORT|
                                           MID_FLAG_ELEMENT_ITEM_IMPORT)) )
                {
                // first get item from itemset
                const SfxPoolItem* pItem = 0;
                SfxItemState eState = rSet.GetItemState( pEntry->nWhichId, TRUE,
                                                         &pItem );

                // if its not set, try the pool
                if(SFX_ITEM_SET != eState && SFX_WHICH_MAX > pEntry->nWhichId )
                    pItem = &rSet.GetPool()->GetDefaultItem(pEntry->nWhichId);

                // do we have an item?
                if(eState >= SFX_ITEM_DEFAULT && pItem)
                {
                    SfxPoolItem *pNewItem = pItem->Clone();
                    BOOL bPut = FALSE;

                    if( 0 == (pEntry->nMemberId&MID_FLAG_SPECIAL_ITEM_IMPORT) )
                    {
                        bPut = pNewItem->importXML( rValue,
                                            pEntry->nMemberId & MID_FLAG_MASK,
                                            rUnitConverter );

                    }
                    else
                    {
                        bPut = handleSpecialItem( *pEntry, *pNewItem, rSet,
                                                  rValue, rUnitConverter,
                                                  rNamespaceMap );
                    }

                    if( bPut )
                        rSet.Put( *pNewItem );

                    delete pNewItem;
                }
                else
                {
                    DBG_ERROR( "Could not get a needed item for xml import!" );
                }
            }
            else if( 0 != (pEntry->nMemberId & MID_FLAG_NO_ITEM_IMPORT) )
            {
                handleNoItem( *pEntry, rSet, rValue, rUnitConverter,
                              rNamespaceMap );
            }
        }
        else if( USHRT_MAX != nUnknownWhich &&
                 // TODO: PI
                   (XML_NAMESPACE_NONE == nPrefix ||
                  XML_NAMESPACE_UNKNOWN == nPrefix) )
        {
            if( !pUnknownItem )
            {
                const SfxPoolItem* pItem = 0;
                if( SFX_ITEM_SET == rSet.GetItemState( nUnknownWhich, TRUE,
                                                       &pItem ) )
                {
                    SfxPoolItem *pNew = pItem->Clone();
                    pUnknownItem = PTR_CAST( SvXMLAttrContainerItem, pNew );
                    DBG_ASSERT( pUnknownItem,
                                "SvXMLAttrContainerItem expected" );
                    if( !pUnknownItem )
                        delete pNew;
                }
                else
                {
                    pUnknownItem = new SvXMLAttrContainerItem( nUnknownWhich );
                }
            }
            if( pUnknownItem )
            {
                if( XML_NAMESPACE_NONE == nPrefix )
                    pUnknownItem->AddAttr( aLocalName, rValue );
                else
                    pUnknownItem->AddAttr( aPrefix, aNamespace, aLocalName,
                                           rValue );
            }
        }
    }

    if( pUnknownItem )
    {
        rSet.Put( *pUnknownItem );
        delete pUnknownItem;
    }

    finished( rSet );
}

/** this method is called for every item that has the
    MID_FLAG_SPECIAL_ITEM_IMPORT flag set */
BOOL
SvXMLImportItemMapper::handleSpecialItem(  const SvXMLItemMapEntry& rEntry,
                                            SfxPoolItem& rItem,
                                            SfxItemSet& rSet,
                                            const OUString& rValue,
                                            const SvXMLUnitConverter& rUnitConverter,
                                            const SvXMLNamespaceMap& rNamespaceMap ) const
{
    DBG_ERROR( "unsuported special item in xml import" );
    return FALSE;
}

/** this method is called for every item that has the
    MID_FLAG_NO_ITEM_IMPORT flag set */
BOOL SvXMLImportItemMapper::handleNoItem( const SvXMLItemMapEntry& rEntry,
                                           SfxItemSet& rSet,
                                           const OUString& rValue,
                                           const SvXMLUnitConverter& rUnitConverter,
                                           const SvXMLNamespaceMap& rNamespaceMap) const
{
    DBG_ERROR( "unsuported no item in xml import" );
    return FALSE;
}

void SvXMLImportItemMapper::finished( SfxItemSet& ) const
{
    // nothing to do here
}

