/*************************************************************************
 *
 *  $RCSfile: xmlimppr.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: sab $ $Date: 2000-09-25 14:45:01 $
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


#ifndef _XMLOFF_PROPERTYSETMAPPER_HXX
#include "xmlprmap.hxx"
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif
#ifndef _XMLOFF_XMLCNITM_HXX
#include "xmlcnitm.hxx"
#endif

#ifndef _XMLOFF_XMLIMPPR_HXX
#include "xmlimppr.hxx"
#endif

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::rtl;
using namespace ::std;

SvXMLImportPropertyMapper::SvXMLImportPropertyMapper(
        const UniReference< XMLPropertySetMapper >& rMapper ) :
    maPropMapper  ( rMapper )
{
}

SvXMLImportPropertyMapper::~SvXMLImportPropertyMapper()
{
}

/** fills the given itemset with the attributes in the given list */
void SvXMLImportPropertyMapper::importXML(
        vector< XMLPropertyState >& rProperties,
           Reference< XAttributeList > xAttrList,
           const SvXMLUnitConverter& rUnitConverter,
        const SvXMLNamespaceMap& rNamespaceMap ) const
{
    INT16 nAttr = xAttrList->getLength();

//  SvXMLAttrContainerItem *pUnknownItem = 0;

    for( INT16 i=0; i < nAttr; i++ )
    {
        const OUString& rAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName, aPrefix, aNamespace;
        USHORT nPrefix = rNamespaceMap.GetKeyByAttrName( rAttrName, &aPrefix,
                                                    &aLocalName, &aNamespace );

        if( XML_NAMESPACE_XMLNS == nPrefix )
            continue;

        const OUString& rValue = xAttrList->getValueByIndex( i );

        sal_Int32 nIndex = -1;  // index of actual property map entry
        sal_uInt32 nFlags = 0;  // flags of actual property map entry
        do
        {
            // find an entry for this attribute
            nIndex = maPropMapper->GetEntryIndex( nPrefix, aLocalName, nIndex );

            if( nIndex > -1 )
            {
                // create a XMLPropertyState with an empty value

                nFlags = maPropMapper->GetEntryFlags( nIndex );
                if( ( nFlags & (MID_FLAG_NO_ITEM_IMPORT |
                               MID_FLAG_ELEMENT_ITEM_IMPORT) ) == 0 )
                {
                    XMLPropertyState aNewProperty( nIndex );
                    sal_Int32 nReference = -1;

                    // if this is a multi attribute check if another attribute already set
                    // this any. If so use this as a initial value
                    if( ( nFlags & MID_FLAG_MERGE_PROPERTY ) != 0 )
                    {
                        const OUString aAPIName( maPropMapper->GetEntryAPIName( nIndex ) );
                        const sal_Int32 nSize = rProperties.size();
                        for( nReference = 0; nReference < nSize; nReference++ )
                        {
                            if( (nIndex != rProperties[nReference].mnIndex) && (maPropMapper->GetEntryAPIName( rProperties[nReference].mnIndex ) == aAPIName ))
                            {
                                aNewProperty = rProperties[nReference];
                                aNewProperty.mnIndex = nIndex;
                                break;
                            }
                        }

                        if( nReference == nSize )
                            nReference = -1;
                    }

                    sal_Bool bSet = sal_False;
                    if( ( nFlags & MID_FLAG_SPECIAL_ITEM_IMPORT ) == 0 )
                    {
                        // let the XMLPropertySetMapper decide how to import the value
                        bSet = maPropMapper->importXML( rValue, aNewProperty,
                                                 rUnitConverter );
                    }
                    else
                    {
                        bSet = handleSpecialItem( aNewProperty, rProperties,
                                                  rValue, rUnitConverter,
                                                     rNamespaceMap );
                    }
                    // store the property in the given vector
                    if( bSet )
                    {
                        if( nReference == -1 )
                            rProperties.push_back( aNewProperty );
                        else
                            rProperties[nReference] = aNewProperty;
                    }
                }
                else
                    handleNoItem( nIndex, rProperties, rValue, rUnitConverter,
                                                            rNamespaceMap );
            }
            else
            {
                /*
                if( USHRT_MAX != mnUnknownWhich &&
                    // TODO: PI
                    (XML_NAMESPACE_NONE == nPrefix ||
                     XML_NAMESPACE_UNKNOWN == nPrefix) )
                {
                    if( !pUnknownItem )
                    {
                        const SfxPoolItem* pItem = 0;
                        if( SFX_ITEM_SET == rSet.GetItemState( mnUnknownWhich, TRUE,
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
                            pUnknownItem = new SvXMLAttrContainerItem( mnUnknownWhich );
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
                */
            }
        }
        while( ( nIndex >= 0 ) && (( nFlags & MID_FLAG_MULTI_PROPERTY ) != 0 ) );
    }

/*
    if( pUnknownItem )
    {
        rSet.Put( *pUnknownItem );
        delete pUnknownItem;
    }
*/

    finished( rProperties );
}

/** this method is called for every item that has the MID_FLAG_SPECIAL_ITEM_IMPORT flag set */
BOOL SvXMLImportPropertyMapper::handleSpecialItem(
        XMLPropertyState& rProperty,
        vector< XMLPropertyState >& rProperties,
        const OUString& rValue,
        const SvXMLUnitConverter& rUnitConverter,
        const SvXMLNamespaceMap& rNamespaceMap ) const
{
    DBG_ERROR( "unsuported special item in xml import" );
    return FALSE;
}

/** this method is called for every item that has the MID_FLAG_NO_ITEM_IMPORT flag set */
BOOL SvXMLImportPropertyMapper::handleNoItem(
        sal_Int32 nIndex,
        vector< XMLPropertyState >& rProperties,
          const OUString& rValue,
        const SvXMLUnitConverter& rUnitConverter,
        const SvXMLNamespaceMap& rNamespaceMap) const
{
    DBG_ERROR( "unsuported no item in xml import" );
    return FALSE;
}

void SvXMLImportPropertyMapper::finished(
        vector< XMLPropertyState >& rProperties ) const
{
    // nothing to do here
}
