/*************************************************************************
 *
 *  $RCSfile: xmlexppr.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: mib $ $Date: 2000-10-19 14:25:17 $
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

#include "xmlexppr.hxx"

#ifndef _XMLOFF_XMLKYWD_HXX
#include "xmlkywd.hxx"
#endif

#ifndef _XMLOFF_ATTRLIST_HXX
#include "attrlist.hxx"
#endif

#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif

#ifndef _XMLOFF_XMLCNITM_HXX
#include "xmlcnitm.hxx"
#endif

#ifndef _SVSTDARR_USHORTS
#define _SVSTDARR_USHORTS
#include <svtools/svstdarr.hxx>
#endif

using namespace ::rtl;
using namespace ::com::sun::star;

///////////////////////////////////////////////////////////////////////////////
//
// ctor/dtor , class SvXMLExportPropertyMapper
//

SvXMLExportPropertyMapper::SvXMLExportPropertyMapper(
        const UniReference< XMLPropertySetMapper >& rMapper )
: maPropMapper( rMapper )
{
}

SvXMLExportPropertyMapper::~SvXMLExportPropertyMapper()
{
}

///////////////////////////////////////////////////////////////////////////////
//
// public methods
//

/** fills the given attribute list with the items in the given set */
void SvXMLExportPropertyMapper::exportXML( SvXMLAttributeList& rAttrList,
        const ::std::vector< XMLPropertyState >& rProperties,
        const SvXMLUnitConverter& rUnitConverter,
        const SvXMLNamespaceMap& rNamespaceMap,
        sal_uInt16 nFlags ) const
{
    _exportXML( rAttrList, rProperties, rUnitConverter, rNamespaceMap,
                nFlags, 0 );
}


void SvXMLExportPropertyMapper::exportXML( SvXMLAttributeList& rAttrList,
        const XMLPropertyState& rProperty,
        const SvXMLUnitConverter& rUnitConverter,
        const SvXMLNamespaceMap& rNamespaceMap,
        sal_uInt16 nFlags ) const
{
    if( ( maPropMapper->GetEntryFlags( rProperty.mnIndex ) &
                MID_FLAG_ELEMENT_ITEM_EXPORT ) == 0 )
        _exportXML( rAttrList, rProperty, rUnitConverter, rNamespaceMap,
                    nFlags );
}

void SvXMLExportPropertyMapper::exportXML(
           const uno::Reference< xml::sax::XDocumentHandler > & rHandler,
        const ::std::vector< XMLPropertyState >& rProperties,
        const SvXMLUnitConverter& rUnitConverter,
        const SvXMLNamespaceMap& rNamespaceMap,
        sal_uInt16 nFlags ) const
{
    SvXMLAttributeList *pAttrList = new SvXMLAttributeList();
    uno::Reference< xml::sax::XAttributeList > xAttrList( pAttrList );

    SvUShorts aIndexArray;

    _exportXML( *pAttrList, rProperties, rUnitConverter, rNamespaceMap,
                nFlags, &aIndexArray );

    if( pAttrList->getLength() > 0L || (nFlags & XML_EXPORT_FLAG_EMPTY) != 0 ||
        aIndexArray.Count() != 0 )
    {
        if( (nFlags & XML_EXPORT_FLAG_IGN_WS) != 0 )
        {
            OUString sWS( OUString::createFromAscii(sXML_WS) );
            rHandler->ignorableWhitespace( sWS );
        }

        OUString sLName( OUString::createFromAscii(sXML_properties) );
        OUString sName = rNamespaceMap.GetQNameByKey( XML_NAMESPACE_STYLE,
                                                      sLName );
        rHandler->startElement( sName, xAttrList );

        exportElementItems( rHandler, rUnitConverter, rNamespaceMap,
                            rProperties, nFlags, aIndexArray );

        rHandler->endElement( sName );
    }
}

/** this method is called for every item that has the
    MID_FLAG_SPECIAL_ITEM_EXPORT flag set */
void SvXMLExportPropertyMapper::handleSpecialItem(
        SvXMLAttributeList& rAttrList,
        const XMLPropertyState& rProperty,
        const SvXMLUnitConverter& rUnitConverter,
        const SvXMLNamespaceMap& rNamespaceMap,
        const ::std::vector< XMLPropertyState > *pProperties,
        sal_uInt32 nIdx ) const
{
    DBG_ERROR( "special item not handled in xml export" );
}

/** this method is called for every item that has the
    MID_FLAG_NO_ITEM_EXPORT flag set */
void SvXMLExportPropertyMapper::handleNoItem(
        SvXMLAttributeList& rAttrList,
        const XMLPropertyState& rProperty,
        const SvXMLUnitConverter& rUnitConverter,
        const SvXMLNamespaceMap& rNamespaceMap,
        const ::std::vector< XMLPropertyState > *pProperties,
        sal_uInt32 nIdx ) const
{
    DBG_ERROR( "no item not handled in xml export" );
}

/** this method is called for every item that has the
    MID_FLAG_ELEMENT_EXPORT flag set */
void SvXMLExportPropertyMapper::handleElementItem(
        const uno::Reference< xml::sax::XDocumentHandler > & rHandler,
        const XMLPropertyState& rProperty,
        const SvXMLUnitConverter& rUnitConverter,
        const SvXMLNamespaceMap& rNamespaceMap,
        sal_uInt16 nFlags,
        const ::std::vector< XMLPropertyState > *pProperties,
        sal_uInt32 nIdx ) const
{
    DBG_ERROR( "element item not handled in xml export" );
}

///////////////////////////////////////////////////////////////////////////////
//
// protected methods
//

/** fills the given attribute list with the items in the given set */
void SvXMLExportPropertyMapper::_exportXML(
        SvXMLAttributeList& rAttrList,
        const ::std::vector< XMLPropertyState >& rProperties,
        const SvXMLUnitConverter& rUnitConverter,
        const SvXMLNamespaceMap& rNamespaceMap,
        sal_uInt16 nFlags,
        SvUShorts* pIndexArray ) const
{
    const sal_uInt32 nCount = rProperties.size();
    sal_uInt32 nIndex = 0;

    while( nIndex < nCount )
    {
        if( rProperties[nIndex].mnIndex >= 0 )  // valid entry?
        {
            // we have a valid map entry here, so lets use it...
            if( ( maPropMapper->GetEntryFlags( rProperties[nIndex].mnIndex )
                        & MID_FLAG_NO_ITEM_EXPORT ) == 0 )
            {
                if( ( maPropMapper->GetEntryFlags( rProperties[nIndex].mnIndex )
                            & MID_FLAG_ELEMENT_ITEM_EXPORT ) != 0 )
                {
                    // element items do not add any properties,
                    // we export it later
                    if( pIndexArray )
                        pIndexArray->Insert( nIndex, pIndexArray->Count() );
                }
                else
                {
                    _exportXML( rAttrList, rProperties[nIndex], rUnitConverter,
                               rNamespaceMap, nFlags, &rProperties, nIndex );
                }
            }
            else
            {
                handleNoItem( rAttrList, rProperties[nIndex], rUnitConverter,
                              rNamespaceMap, &rProperties, nIndex );
            }
        }

        nIndex++;
    }
}

void SvXMLExportPropertyMapper::_exportXML(
        SvXMLAttributeList& rAttrList,
        const XMLPropertyState& rProperty,
        const SvXMLUnitConverter& rUnitConverter,
        const SvXMLNamespaceMap& rNamespaceMap,
        sal_uInt16 nFlags,
        const ::std::vector< XMLPropertyState > *pProperties,
        sal_uInt32 nIdx ) const
{
    OUString sCDATA( OUString::createFromAscii( sXML_CDATA ) );

    if ( ( maPropMapper->GetEntryFlags( rProperty.mnIndex ) &
                MID_FLAG_SPECIAL_ITEM_EXPORT ) != 0 )
    {
        /* Currently, we have nothing like a SvXMLAttrContainerItem, since we use properties instead of poolitems

        if( rItem.ISA( SvXMLAttrContainerItem ) )
        {
            SvXMLNamespaceMap *pNewNamespaceMap = 0;
            const SvXMLNamespaceMap *pNamespaceMap = &rNamespaceMap;

            const SvXMLAttrContainerItem *pUnknown = PTR_CAST( SvXMLAttrContainerItem, &rItem );

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

                        sName.appendAscii( sXML_xmlns );
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
        */
        {
            handleSpecialItem( rAttrList, rProperty, rUnitConverter,
                               rNamespaceMap, pProperties, nIdx );
        }
    }
    else if ( ( maPropMapper->GetEntryFlags( rProperty.mnIndex ) &
                    MID_FLAG_ELEMENT_ITEM_EXPORT ) == 0 )
    {
        OUString aValue;
        const OUString sName( rNamespaceMap.GetQNameByKey(
                    maPropMapper->GetEntryNameSpace( rProperty.mnIndex ),
                    maPropMapper->GetEntryXMLName( rProperty.mnIndex ) ) );

        sal_Bool bRemove = sal_False;
        if( ( maPropMapper->GetEntryFlags( rProperty.mnIndex ) &
                    MID_FLAG_MERGE_ATTRIBUTE ) != 0 )
        {
            aValue = rAttrList.getValueByName( sName );
            bRemove = aValue.getLength() != 0;
        }

        if( maPropMapper->exportXML( aValue, rProperty, rUnitConverter ) )
        {
            if( bRemove )
                rAttrList.RemoveAttribute( sName );
            rAttrList.AddAttribute( sName, sCDATA, aValue );
        }
    }
}

void SvXMLExportPropertyMapper::exportElementItems(
        const uno::Reference< xml::sax::XDocumentHandler > & rHandler,
        const SvXMLUnitConverter& rUnitConverter,
        const SvXMLNamespaceMap& rNamespaceMap,
        const ::std::vector< XMLPropertyState >& rProperties,
        sal_uInt16 nFlags,
        const SvUShorts& rIndexArray ) const
{
    const sal_uInt16 nCount = rIndexArray.Count();

    sal_Bool bItemsExported = sal_False;
    OUString sWS( OUString::createFromAscii(sXML_WS) );
    for( sal_uInt16 nIndex = 0; nIndex < nCount; nIndex++ )
    {
        const sal_uInt16 nElement = rIndexArray.GetObject( nIndex );

        DBG_ASSERT( 0 != ( maPropMapper->GetEntryFlags(
                rProperties[nElement].mnIndex ) & MID_FLAG_ELEMENT_ITEM_EXPORT),
                "wrong mid flag!" );

        rHandler->ignorableWhitespace( sWS );
        handleElementItem( rHandler, rProperties[nElement], rUnitConverter,
                           rNamespaceMap, nFlags, &rProperties, nElement );
        bItemsExported = sal_True;
    }

    if( bItemsExported )
        rHandler->ignorableWhitespace( sWS );
}
