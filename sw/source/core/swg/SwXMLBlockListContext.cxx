/*************************************************************************
 *
 *  $RCSfile: SwXMLBlockListContext.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: mtg $ $Date: 2001-05-02 16:45:40 $
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
#ifndef _SW_XMLBLOCKLIST_CONTEXT_HXX
#include <SwXMLBlockListContext.hxx>
#endif

#ifndef _UNOTOOLS_CHARCLASS_HXX
#include <unotools/charclass.hxx>
#endif


using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;
using namespace ::rtl;

SwXMLBlockListContext::SwXMLBlockListContext(
   SwXMLBlockListImport& rImport,
   sal_uInt16 nPrefix,
   const OUString& rLocalName,
   const com::sun::star::uno::Reference<
   com::sun::star::xml::sax::XAttributeList > & xAttrList ) :
    rLocalRef (rImport),
    SvXMLImportContext ( rImport, nPrefix, rLocalName )
{
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for (sal_Int16 i=0; i < nAttrCount; i++)
    {
        const OUString& rAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix = rImport.GetNamespaceMap().GetKeyByAttrName( rAttrName, &aLocalName);
        const OUString& rAttrValue = xAttrList->getValueByIndex( i );
        if (XML_NAMESPACE_BLOCKLIST == nPrefix)
        {
            if (aLocalName.compareToAscii( sXML_list_name ) == 0)
            {
                rImport.getBlockList().SetName(rAttrValue);
                break;
            }
        }
    }

        //rImport.getBlockList().SetName(xAttrList->getValueByName( OUString::createFromAscii(sXML_list_name) ));
}

SwXMLBlockListContext::~SwXMLBlockListContext ( void )
{
}

SvXMLImportContext *SwXMLBlockListContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;
    if (nPrefix == XML_NAMESPACE_BLOCKLIST &&
        rLocalName.compareToAscii( sXML_block ) == 0)
        pContext = new SwXMLBlockContext (rLocalRef, nPrefix, rLocalName, xAttrList);
    else
        pContext = new SvXMLImportContext( rLocalRef, nPrefix, rLocalName);
    return pContext;
}

SwXMLBlockContext::SwXMLBlockContext(
   SwXMLBlockListImport& rImport,
   sal_uInt16 nPrefix,
   const OUString& rLocalName,
   const com::sun::star::uno::Reference<
   com::sun::star::xml::sax::XAttributeList > & xAttrList ) :
    rLocalRef(rImport),
    SvXMLImportContext ( rImport, nPrefix, rLocalName )
{
    static const CharClass & rCC = GetAppCharClass();
    String aShort, aLong, aPackageName;
    BOOL bTextOnly = FALSE;

    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for (sal_Int16 i=0; i < nAttrCount; i++)
    {
        const OUString& rAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix = rImport.GetNamespaceMap().GetKeyByAttrName( rAttrName, &aLocalName);
        const OUString& rAttrValue = xAttrList->getValueByIndex( i );
        if (XML_NAMESPACE_BLOCKLIST == nPrefix)
        {
            if (aLocalName.compareToAscii( sXML_abbreviated_name ) == 0)
            {
                aShort = rCC.upper(rAttrValue);
            }
            else if (aLocalName.compareToAscii( sXML_name ) == 0)
            {
                aLong = rAttrValue;
            }
            else if (aLocalName.compareToAscii( sXML_package_name ) == 0)
            {
                aPackageName = rAttrValue;
            }
            else if (aLocalName.compareToAscii( sXML_unformatted_text ) == 0)
            {
                if (rAttrValue.compareToAscii( sXML_true ) == 0)
                    bTextOnly = TRUE;
            }
        }
    }
    if (!aShort.Len() || !aLong.Len() || !aPackageName.Len())
        return;
    rImport.getBlockList().AddName( aShort, aLong, aPackageName, bTextOnly);
}

SwXMLBlockContext::~SwXMLBlockContext ( void )
{
}

SwXMLTextBlockDocumentContext::SwXMLTextBlockDocumentContext(
   SwXMLTextBlockImport& rImport,
   sal_uInt16 nPrefix,
   const OUString& rLocalName,
   const com::sun::star::uno::Reference<
   com::sun::star::xml::sax::XAttributeList > & xAttrList ) :
    rLocalRef(rImport),
    SvXMLImportContext ( rImport, nPrefix, rLocalName )
{
}

SvXMLImportContext *SwXMLTextBlockDocumentContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;
    if (nPrefix == XML_NAMESPACE_OFFICE &&
        rLocalName.compareToAscii( sXML_body ) == 0)
        pContext = new SwXMLTextBlockBodyContext (rLocalRef, nPrefix, rLocalName, xAttrList);
    else
        pContext = new SvXMLImportContext( rLocalRef, nPrefix, rLocalName);
    return pContext;
}
SwXMLTextBlockDocumentContext::~SwXMLTextBlockDocumentContext ( void )
{
}
SwXMLTextBlockBodyContext::SwXMLTextBlockBodyContext(
   SwXMLTextBlockImport& rImport,
   sal_uInt16 nPrefix,
   const OUString& rLocalName,
   const com::sun::star::uno::Reference<
   com::sun::star::xml::sax::XAttributeList > & xAttrList ) :
    rLocalRef(rImport),
    SvXMLImportContext ( rImport, nPrefix, rLocalName )
{
}

SvXMLImportContext *SwXMLTextBlockBodyContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;
    if (nPrefix == XML_NAMESPACE_TEXT &&
        rLocalName.compareToAscii( sXML_p ) == 0)
        pContext = new SwXMLTextBlockParContext (rLocalRef, nPrefix, rLocalName, xAttrList);
    else
        pContext = new SvXMLImportContext( rLocalRef, nPrefix, rLocalName);
    return pContext;
}
SwXMLTextBlockBodyContext::~SwXMLTextBlockBodyContext ( void )
{
}
SwXMLTextBlockParContext::SwXMLTextBlockParContext(
   SwXMLTextBlockImport& rImport,
   sal_uInt16 nPrefix,
   const OUString& rLocalName,
   const com::sun::star::uno::Reference<
   com::sun::star::xml::sax::XAttributeList > & xAttrList ) :
    rLocalRef(rImport),
    SvXMLImportContext ( rImport, nPrefix, rLocalName )
{
}

void SwXMLTextBlockParContext::Characters( const ::rtl::OUString& rChars )
{
    rLocalRef.m_rText.Append ( rChars.getStr());
}
SwXMLTextBlockParContext::~SwXMLTextBlockParContext ( void )
{
    if (rLocalRef.bTextOnly)
        rLocalRef.m_rText.AppendAscii( "\015" );
    else
    {
        if (rLocalRef.m_rText.GetChar ( rLocalRef.m_rText.Len()) != ' ' )
            rLocalRef.m_rText.AppendAscii( " " );
    }
}
