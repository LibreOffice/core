/*************************************************************************
 *
 *  $RCSfile: DocumentSettingsContext.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: sab $ $Date: 2001-03-11 15:53:39 $
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

#ifdef PCH
#include "filt_pch.hxx"
#endif

#pragma hdrstop

// INCLUDE ---------------------------------------------------------------

#ifndef _XMLOFF_DOCUMENTSETTINGSCONTEXT_HXX
#include "DocumentSettingsContext.hxx"
#endif
#ifndef _XMLOFF_XMLIMP_HXX
#include "xmlimp.hxx"
#endif
#ifndef _XMLOFF_XMLKYWD_HXX
#include "xmlkywd.hxx"
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif

using namespace com::sun::star;

//------------------------------------------------------------------

class XMLConfigItemSetContext : public SvXMLImportContext
{
    com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>&    rProps;

public:
    XMLConfigItemSetContext(SvXMLImport rImport, USHORT nPrfx, const NAMESPACE_RTL(OUString)& rLName,
                                    const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                    com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& rProps);
    virtual ~XMLConfigItemSetContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                                    const NAMESPACE_RTL(OUString)& rLocalName,
                                                    const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );
    virtual void EndElement();
};

XMLDocumentSettingsContext::XMLDocumentSettingsContext(SvXMLImport rImport, USHORT nPrfx, const NAMESPACE_RTL(OUString)& rLName,
                    const uno::Reference<xml::sax::XAttributeList>& xAttrList,
                    uno::Sequence<beans::PropertyValue>& rTempViewProps,
                    uno::Sequence<beans::PropertyValue>& rTempConfigProps )
    : SvXMLImportContext( rImport, nPrfx, rLName ),
    rViewProps(rTempViewProps),
    rConfigProps(rTempConfigProps)
{
    // here are no attributes
}

XMLDocumentSettingsContext::~XMLDocumentSettingsContext()
{
}

SvXMLImportContext *XMLDocumentSettingsContext::CreateChildContext( USHORT nPrefix,
                                     const NAMESPACE_RTL(OUString)& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;
    rtl::OUString sName;

    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        USHORT nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        rtl::OUString sValue = xAttrList->getValueByIndex( i );

        if (nPrefix == XML_NAMESPACE_CONFIG)
        {
            if (aLocalName.compareToAscii(sXML_name) == 0)
                sName = sValue;
        }
    }


    if (nPrefix == XML_NAMESPACE_CONFIG)
    {
        if (rLocalName.compareToAscii(sXML_config_item_set) == 0)
        {
            if (sName.compareToAscii(sXML_view_settings) == 0)
                pContext = new XMLConfigItemSetContext(GetImport(), nPrefix, rLocalName, xAttrList, rViewProps);
            else if (sName.compareToAscii(sXML_configuration_settings) == 0)
                pContext = new XMLConfigItemSetContext(GetImport(), nPrefix, rLocalName, xAttrList, rConfigProps);
        }
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

void XMLDocumentSettingsContext::EndElement()
{
}

XMLConfigItemSetContext::XMLConfigItemSetContext(SvXMLImport rImport, USHORT nPrfx, const NAMESPACE_RTL(OUString)& rLName,
                    const uno::Reference<xml::sax::XAttributeList>& xAttrList,
                    uno::Sequence<beans::PropertyValue>& rTempProps )
    : SvXMLImportContext( rImport, nPrfx, rLName ),
    rProps(rTempProps)
{
    // here are no attributes
}

XMLConfigItemSetContext::~XMLConfigItemSetContext()
{
}

SvXMLImportContext *XMLConfigItemSetContext::CreateChildContext( USHORT nPrefix,
                                     const NAMESPACE_RTL(OUString)& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;
    rtl::OUString sName;

    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        USHORT nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        rtl::OUString sValue = xAttrList->getValueByIndex( i );

        if (nPrefix == XML_NAMESPACE_CONFIG)
        {
            if (aLocalName.compareToAscii(sXML_name) == 0)
                sName = sValue;
        }
    }


    if (nPrefix == XML_NAMESPACE_CONFIG)
    {
        if (rLocalName.compareToAscii(sXML_config_item) == 0)
        {
            // add a PropertyValue to the sequence and create the context
        }
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

void XMLConfigItemSetContext::EndElement()
{
}

