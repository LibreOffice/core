/*************************************************************************
 *
 *  $RCSfile: xmlscripti.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: hr $ $Date: 2004-11-09 13:33:01 $
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

#include "xmlscripti.hxx"
#include "xmlnmspe.hxx"
#include "xmltoken.hxx"
#include "xmlimp.hxx"
#include "nmspmap.hxx"
#include "XMLEventsImportContext.hxx"
#include "xmlbasici.hxx"

#include <com/sun/star/document/XEventsSupplier.hpp>

using namespace rtl;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::frame;
using namespace com::sun::star::document;
using namespace com::sun::star::xml::sax;
using namespace ::xmloff::token;


// =============================================================================
// XMLScriptChildContext: context for <office:script> element
// =============================================================================

class XMLScriptChildContext : public SvXMLImportContext
{
private:
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > m_xModel;
    ::rtl::OUString m_aLanguage;

public:
    XMLScriptChildContext( SvXMLImport& rImport, USHORT nPrfx, const ::rtl::OUString& rLName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel>& rxModel,
        const ::rtl::OUString& rLanguage );
    virtual ~XMLScriptChildContext();

    virtual SvXMLImportContext* CreateChildContext( USHORT nPrefix, const ::rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );

    virtual void EndElement();
};

// -----------------------------------------------------------------------------

XMLScriptChildContext::XMLScriptChildContext( SvXMLImport& rImport, USHORT nPrfx, const ::rtl::OUString& rLName,
        const Reference< frame::XModel >& rxModel, const ::rtl::OUString& rLanguage )
    :SvXMLImportContext( rImport, nPrfx, rLName )
    ,m_xModel( rxModel )
    ,m_aLanguage( rLanguage )
{
}

// -----------------------------------------------------------------------------

XMLScriptChildContext::~XMLScriptChildContext()
{
}

// -----------------------------------------------------------------------------

SvXMLImportContext* XMLScriptChildContext::CreateChildContext(
    USHORT nPrefix, const ::rtl::OUString& rLocalName,
    const Reference< xml::sax::XAttributeList >& xAttrList )
{
    SvXMLImportContext* pContext = NULL;

    ::rtl::OUString aBasic( GetImport().GetNamespaceMap().GetPrefixByKey( XML_NAMESPACE_OOO ) );
    aBasic += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ":Basic" ) );

    if ( m_aLanguage == aBasic && nPrefix == XML_NAMESPACE_OOO && IsXMLToken( rLocalName, XML_LIBRARIES ) )
        pContext = new XMLBasicImportContext( GetImport(), nPrefix, rLocalName, m_xModel );

    if ( !pContext )
        pContext = SvXMLImportContext::CreateChildContext( nPrefix, rLocalName, xAttrList );

    return pContext;
}

// -----------------------------------------------------------------------------

void XMLScriptChildContext::EndElement()
{
}

// =============================================================================
// XMLScriptContext: context for <office:scripts> element
// =============================================================================

XMLScriptContext::XMLScriptContext( SvXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLName,
        const Reference<XModel>& rDocModel )
    :SvXMLImportContext( rImport, nPrfx, rLName )
    ,m_xModel( rDocModel )
{
}

// -----------------------------------------------------------------------------

XMLScriptContext::~XMLScriptContext()
{
}

// -----------------------------------------------------------------------------

SvXMLImportContext* XMLScriptContext::CreateChildContext(
    sal_uInt16 nPrefix, const OUString& rLName,
    const Reference<XAttributeList>& xAttrList )
{
    SvXMLImportContext* pContext = NULL;

    if ( nPrefix == XML_NAMESPACE_OFFICE )
    {
        if ( IsXMLToken( rLName, XML_EVENT_LISTENERS ) )
        {
            Reference< XEventsSupplier> xSupplier( GetImport().GetModel(), UNO_QUERY );
            pContext = new XMLEventsImportContext( GetImport(), nPrefix, rLName, xSupplier );
        }
        else if ( IsXMLToken( rLName, XML_SCRIPT ) )
        {
            ::rtl::OUString aAttrName( GetImport().GetNamespaceMap().GetPrefixByKey( XML_NAMESPACE_SCRIPT ) );
            aAttrName += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ":language" ) );
            if ( xAttrList.is() )
            {
                ::rtl::OUString aLanguage = xAttrList->getValueByName( aAttrName );
                pContext = new XMLScriptChildContext( GetImport(), nPrefix, rLName, m_xModel, aLanguage );
            }
        }
    }

    if ( !pContext )
        pContext = SvXMLImportContext::CreateChildContext( nPrefix, rLName, xAttrList);

    return pContext;
}

// -----------------------------------------------------------------------------

void XMLScriptContext::EndElement()
{
}

// -----------------------------------------------------------------------------
