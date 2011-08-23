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

#include <tools/debug.hxx>
#include <com/sun/star/document/XEventsSupplier.hpp>
#include "xmlnmspe.hxx"
#include "XMLEventsImportContext.hxx"
#include "XMLShapePropertySetContext.hxx"
#include "XMLTextColumnsContext.hxx"
#include "XMLBackgroundImageContext.hxx"
#include "txtprmap.hxx"

#include "XMLTextShapeStyleContext.hxx"
namespace binfilter {

using namespace ::rtl;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::beans;
using namespace ::binfilter::xmloff::token;

class XMLTextShapePropertySetContext_Impl : public XMLShapePropertySetContext
{
public:
    XMLTextShapePropertySetContext_Impl( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const OUString& rLName,
        const Reference< XAttributeList >& xAttrList,
        ::std::vector< XMLPropertyState > &rProps,
        const UniReference < SvXMLImportPropertyMapper > &rMap );

    virtual ~XMLTextShapePropertySetContext_Impl();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
        const OUString& rLocalName,
        const Reference< XAttributeList >& xAttrList,
        ::std::vector< XMLPropertyState > &rProperties,
        const XMLPropertyState& rProp);
};

XMLTextShapePropertySetContext_Impl::XMLTextShapePropertySetContext_Impl(
                 SvXMLImport& rImport, sal_uInt16 nPrfx,
                 const OUString& rLName,
                 const Reference< XAttributeList > & xAttrList,
                 ::std::vector< XMLPropertyState > &rProps,
                 const UniReference < SvXMLImportPropertyMapper > &rMap ) :
    XMLShapePropertySetContext( rImport, nPrfx, rLName, xAttrList, rProps,
                                rMap )
{
}

XMLTextShapePropertySetContext_Impl::~XMLTextShapePropertySetContext_Impl()
{
}

SvXMLImportContext *XMLTextShapePropertySetContext_Impl::CreateChildContext(
                   sal_uInt16 nPrefix,
                   const OUString& rLocalName,
                   const Reference< XAttributeList > & xAttrList,
                   ::std::vector< XMLPropertyState > &rProperties,
                   const XMLPropertyState& rProp )
{
    SvXMLImportContext *pContext = 0;

    switch( xMapper->getPropertySetMapper()
                    ->GetEntryContextId( rProp.mnIndex ) )
    {
    case CTF_TEXTCOLUMNS:
        pContext = new XMLTextColumnsContext( GetImport(), nPrefix,
                                                   rLocalName, xAttrList, rProp,
                                                   rProperties );
        break;

    case CTF_BACKGROUND_URL:
        DBG_ASSERT( rProp.mnIndex >= 3 &&
                    CTF_BACKGROUND_TRANSPARENCY == 
                        xMapper->getPropertySetMapper()
                        ->GetEntryContextId( rProp.mnIndex-3 ) &&
                    CTF_BACKGROUND_POS  == xMapper->getPropertySetMapper()
                        ->GetEntryContextId( rProp.mnIndex-2 ) &&
                    CTF_BACKGROUND_FILTER  == xMapper->getPropertySetMapper()
                        ->GetEntryContextId( rProp.mnIndex-1 ),
                    "invalid property map!");
        pContext = 
            new XMLBackgroundImageContext( GetImport(), nPrefix,
                                           rLocalName, xAttrList,
                                           rProp,
                                           rProp.mnIndex-2,
                                           rProp.mnIndex-1,
                                           rProp.mnIndex-3,
                                           rProperties );
        break;
    }
    
    if( !pContext )
        pContext = XMLShapePropertySetContext::CreateChildContext(
                        nPrefix, rLocalName, xAttrList, rProperties, rProp );
    
    return pContext;
}

//----------------------------------------------------------------------------- 

void XMLTextShapeStyleContext::SetAttribute( sal_uInt16 nPrefixKey,
                                        const OUString& rLocalName,
                                        const OUString& rValue )
{
    if( XML_NAMESPACE_STYLE == nPrefixKey &&
        IsXMLToken( rLocalName, XML_AUTO_UPDATE ) )
    {
          if( IsXMLToken( rValue, XML_TRUE ) )
            bAutoUpdate = sal_True;
    }
    else
    {
        XMLShapeStyleContext::SetAttribute( nPrefixKey, rLocalName, rValue );
    }
}

TYPEINIT1( XMLTextShapeStyleContext, XMLShapeStyleContext );

XMLTextShapeStyleContext::XMLTextShapeStyleContext( SvXMLImport& rImport,
        sal_uInt16 nPrfx, const OUString& rLName,
        const Reference< XAttributeList > & xAttrList,
        SvXMLStylesContext& rStyles, sal_uInt16 nFamily,
        sal_Bool bDefaultStyle ) :
    XMLShapeStyleContext( rImport, nPrfx, rLName, xAttrList, rStyles,
                          nFamily ),
    bAutoUpdate( sal_False ),
    sIsAutoUpdate( RTL_CONSTASCII_USTRINGPARAM( "IsAutoUpdate" ) )
{
}

XMLTextShapeStyleContext::~XMLTextShapeStyleContext()
{
}

SvXMLImportContext *XMLTextShapeStyleContext::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const Reference< XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if( XML_NAMESPACE_STYLE == nPrefix &&
        IsXMLToken( rLocalName, XML_PROPERTIES ) )
    {
        UniReference < SvXMLImportPropertyMapper > xImpPrMap =
            GetStyles()->GetImportPropertyMapper( GetFamily() );
        if( xImpPrMap.is() )
        {		
            pContext = new XMLTextShapePropertySetContext_Impl(
                    GetImport(), nPrefix, rLocalName, xAttrList,
                    GetProperties(), xImpPrMap );
        }
    }
    else if ( (XML_NAMESPACE_OFFICE == nPrefix) &&
              IsXMLToken( rLocalName, XML_EVENTS ) )
    {
        // create and remember events import context 
        // (for delayed processing of events)
        pContext = new XMLEventsImportContext( GetImport(), nPrefix,
                                                   rLocalName);
        xEventContext = pContext;
    }
        
    if( !pContext )
        pContext = XMLShapeStyleContext::CreateChildContext( nPrefix, rLocalName,
                                                          xAttrList );

    return pContext;
}

void XMLTextShapeStyleContext::CreateAndInsert( sal_Bool bOverwrite )
{
    XMLShapeStyleContext::CreateAndInsert( bOverwrite );
    Reference < XStyle > xStyle = GetStyle();
    if( !xStyle.is() || !(bOverwrite || IsNew()) )
        return;
    
    Reference < XPropertySet > xPropSet( xStyle, UNO_QUERY );
    Reference< XPropertySetInfo > xPropSetInfo =
                xPropSet->getPropertySetInfo();
    if( xPropSetInfo->hasPropertyByName( sIsAutoUpdate ) )
    {
        Any aAny;
        sal_Bool bTmp = bAutoUpdate;
        aAny.setValue( &bTmp, ::getBooleanCppuType() );
        xPropSet->setPropertyValue( sIsAutoUpdate, aAny );
    }

    // tell the style about it's events (if applicable)
    if( xEventContext.Is() )
    {
        // set event suppplier and release reference to context
        Reference<XEventsSupplier> xEventsSupplier(xStyle, UNO_QUERY);
        ((XMLEventsImportContext *)&xEventContext)->SetEvents(xEventsSupplier);
        xEventContext = 0;
    }
}


void XMLTextShapeStyleContext::Finish( sal_Bool bOverwrite )
{
    XMLPropStyleContext::Finish( bOverwrite );
}
}//end of namespace binfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
