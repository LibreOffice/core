/*************************************************************************
 *
 *  $RCSfile: SwXMLSectionList.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: mtg $ $Date: 2001-02-16 09:32:27 $
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
 *  Contributor(s): Martin Gallwey (gallwey@sun.com)
 *
 *
 ************************************************************************/
#pragma hdrstop

#ifndef _APP_HXX //autogen
#include <vcl/svapp.hxx>
#endif

#define _SVSTDARR_STRINGSDTOR
#define _SVSTDARR_STRINGS
#include <svtools/svstdarr.hxx>

#ifndef _SW_XMLSECTIONLIST_HXX
#include <SwXMLSectionList.hxx>
#endif

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;
using namespace ::rtl;

sal_Char __READONLY_DATA sXML_np__block_list[] = "_block-list";

SwXMLSectionList::SwXMLSectionList ( SvStrings & rNewSectionList)
: rSectionList ( rNewSectionList )
{
    GetNamespaceMap().AddAtIndex( XML_NAMESPACE_BLOCKLIST_IDX, sXML_np__block_list,
                                     sXML_n_block_list, XML_NAMESPACE_BLOCKLIST );
}

SwXMLSectionList::~SwXMLSectionList ( void )
{
}

SvXMLImportContext *SwXMLSectionList::CreateContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;
/*
    String sName;
    if (nPrefix == XML_NAMESPACE_TEXT && (rLocalName.compareToAscii( sXML_section ) == 0))
    {
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;

    for (sal_Int16 i=0; i < nAttrCount; i++)
    {
        const OUString& rAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix = GetNamespaceMap().GetKeyByAttrName( rAttrName, &aLocalName);
        const OUString& rAttrValue = xAttrList->getValueByIndex( i );
        if (XML_NAMESPACE_TEXT == nPrefix)
        {
            if (aLocalName.compareToAscii( sXML_name ) == 0)
            {
                sName = rAttrValue;
            }
        }
    }
    if ( sName.Len() )
        rSectionList.Insert ( new String(sName), rSectionList.Count() );

    if( XML_NAMESPACE_OFFICE == nPrefix &&
        rLocalName.compareToAscii( sXML_document ) == 0 )
        pContext = new SvXMLSectionListContext( *this, nPrefix, rLocalName, xAttrList );
    else
    }
    if (nPrefix == XML_NAMESPACE_OFFICE && rLocalName.compareToAscii ( sXML_document) == 0)
    {
        pContext = SvXMLImport::CreateContext( nPrefix, rLocalName, xAttrList );
    }
    else
*/
        if  (nPrefix == XML_NAMESPACE_OFFICE && rLocalName.compareToAscii ( sXML_body ) == 0 ||
         nPrefix == XML_NAMESPACE_TEXT &&
        (rLocalName.compareToAscii( sXML_p ) == 0 ||
         rLocalName.compareToAscii( sXML_h ) == 0 ||
         rLocalName.compareToAscii( sXML_a ) == 0 ||
         rLocalName.compareToAscii( sXML_span ) == 0 ||
         rLocalName.compareToAscii( sXML_section ) == 0 ||
         rLocalName.compareToAscii( sXML_index_body ) == 0 ||
         rLocalName.compareToAscii( sXML_index_title ) == 0 ||
         rLocalName.compareToAscii( sXML_insertion ) == 0 ||
         rLocalName.compareToAscii( sXML_deletion ) == 0 ) )
        pContext = new SvXMLSectionListContext (*this, nPrefix, rLocalName, xAttrList);
    else
        pContext = SvXMLImport::CreateContext( nPrefix, rLocalName, xAttrList );
        /*
        pContext = new SvXMLIgnoreSectionListContext (*this, nPrefix, rLocalName, xAttrList);
        */
    return pContext;
}

SvXMLSectionListContext::SvXMLSectionListContext(
   SwXMLSectionList& rImport,
   sal_uInt16 nPrefix,
   const OUString& rLocalName,
   const com::sun::star::uno::Reference<
   com::sun::star::xml::sax::XAttributeList > & xAttrList ) :
   rLocalRef(rImport),
   SvXMLImportContext ( rImport, nPrefix, rLocalName )
{
}

SvXMLImportContext *SvXMLSectionListContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;
    String sName;

    if (nPrefix == XML_NAMESPACE_TEXT && (rLocalName.compareToAscii( sXML_section ) == 0 ||
                                          rLocalName.compareToAscii( sXML_bookmark) == 0 ) )
    {
        sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;

        for (sal_Int16 i=0; i < nAttrCount; i++)
        {
            const OUString& rAttrName = xAttrList->getNameByIndex( i );
            OUString aLocalName;
            sal_uInt16 nPrefix = rLocalRef.GetNamespaceMap().GetKeyByAttrName( rAttrName, &aLocalName);
            const OUString& rAttrValue = xAttrList->getValueByIndex( i );
            if (XML_NAMESPACE_TEXT == nPrefix)
            {
                if (aLocalName.compareToAscii( sXML_name ) == 0)
                {
                    sName = rAttrValue;
                }
            }
        }
        if ( sName.Len() )
            rLocalRef.rSectionList.Insert ( new String(sName), rLocalRef.rSectionList.Count() );
    }
    /*
    if (nPrefix == XML_NAMESPACE_TEXT &&
        rLocalName.compareToAscii( sXML_section ) == 0)
        pContext = new SvXMLSectionListContext (rLocalRef, nPrefix, rLocalName, xAttrList);
    else
        pContext = new SvXMLImportContext( rLocalRef, nPrefix, rLocalName);
    */
    pContext = new SvXMLSectionListContext (rLocalRef, nPrefix, rLocalName, xAttrList);
    return pContext;
}
SvXMLSectionListContext::~SvXMLSectionListContext ( void )
{
}

SvXMLIgnoreSectionListContext::SvXMLIgnoreSectionListContext(
   SwXMLSectionList& rImport,
   sal_uInt16 nPrefix,
   const OUString& rLocalName,
   const com::sun::star::uno::Reference<
   com::sun::star::xml::sax::XAttributeList > & xAttrList ) :
   rLocalRef(rImport),
   SvXMLImportContext ( rImport, nPrefix, rLocalName )
{
}

SvXMLIgnoreSectionListContext::~SvXMLIgnoreSectionListContext ( void )
{
}
SvXMLImportContext *SvXMLIgnoreSectionListContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference< xml::sax::XAttributeList > & xAttrList )
{
    return  new SvXMLIgnoreSectionListContext (rLocalRef, nPrefix, rLocalName, xAttrList);
}
