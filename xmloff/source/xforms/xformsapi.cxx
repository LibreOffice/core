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


#include "xformsapi.hxx"

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/xforms/XFormsSupplier.hpp>
#include <com/sun/star/xforms/XDataTypeRepository.hpp>
#include <com/sun/star/xforms/XModel.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/xsd/DataTypeClass.hpp>

#include <comphelper/processfactory.hxx>
#include <tools/debug.hxx>

#include <xmloff/xmltoken.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmltkmap.hxx>

using rtl::OUString;
using com::sun::star::uno::Reference;
using com::sun::star::uno::Sequence;
using com::sun::star::uno::UNO_QUERY;
using com::sun::star::uno::UNO_QUERY_THROW;
using com::sun::star::beans::XPropertySet;
using com::sun::star::container::XNameAccess;
using com::sun::star::lang::XMultiServiceFactory;
using com::sun::star::xforms::XFormsSupplier;
using com::sun::star::xforms::XDataTypeRepository;
using com::sun::star::container::XNameContainer;
using comphelper::getProcessServiceFactory;
using com::sun::star::uno::makeAny;
using com::sun::star::uno::Any;
using com::sun::star::uno::Exception;

using namespace com::sun::star;
using namespace xmloff::token;

static Reference<XPropertySet> lcl_createPropertySet( const OUString& rServiceName )
{
    Reference<XMultiServiceFactory> xFactory = getProcessServiceFactory();
    DBG_ASSERT( xFactory.is(), "can't get service factory" );

    Reference<XPropertySet> xModel( xFactory->createInstance( rServiceName ),
                                    UNO_QUERY_THROW );
    DBG_ASSERT( xModel.is(), "can't create model" );

    return xModel;
}

Reference<XPropertySet> xforms_createXFormsModel()
{
    return lcl_createPropertySet( OUSTRING( "com.sun.star.xforms.Model" ) );
}

void xforms_addXFormsModel(
    const Reference<frame::XModel>& xDocument,
    const Reference<XPropertySet>& xModel )
{
    bool bSuccess = false;
    try
    {
        Reference<XFormsSupplier> xSupplier( xDocument, UNO_QUERY );
        if( xSupplier.is() )
        {
            Reference<XNameContainer> xForms = xSupplier->getXForms();
            if( xForms.is() )
            {
                OUString sName;
                xModel->getPropertyValue( OUSTRING("ID")) >>= sName;
                xForms->insertByName( sName, makeAny( xModel ) );
                bSuccess = true;
            }
        }
    }
    catch( const Exception& )
    {
        ; // no success!
    }

    // TODO: implement proper error handling
    DBG_ASSERT( bSuccess, "can't import model" );
    (void)bSuccess;
}

static Reference<XPropertySet> lcl_findXFormsBindingOrSubmission(
    Reference<frame::XModel>& xDocument,
    const rtl::OUString& rBindingID,
    bool bBinding )
{
    // find binding by iterating over all models, and look for the
    // given binding ID

    Reference<XPropertySet> xRet;
    try
    {
        // get supplier
        Reference<XFormsSupplier> xSupplier( xDocument, UNO_QUERY );
        if( xSupplier.is() )
        {
            // get XForms models
            Reference<XNameContainer> xForms = xSupplier->getXForms();
            if( xForms.is() )
            {
                // iterate over all models
                Sequence<OUString> aNames = xForms->getElementNames();
                const OUString* pNames = aNames.getConstArray();
                sal_Int32 nNames = aNames.getLength();
                for( sal_Int32 n = 0; (n < nNames) && !xRet.is(); n++ )
                {
                    Reference<xforms::XModel> xModel(
                        xForms->getByName( pNames[n] ), UNO_QUERY );
                    if( xModel.is() )
                    {
                        // ask model for bindings
                        Reference<XNameAccess> xBindings(
                            bBinding
                                ? xModel->getBindings()
                                : xModel->getSubmissions(),
                            UNO_QUERY_THROW );

                        // finally, ask binding for name
                        if( xBindings->hasByName( rBindingID ) )
                            xRet.set( xBindings->getByName( rBindingID ),
                                      UNO_QUERY );
                    }
                }
            }
        }
    }
    catch( const Exception& )
    {
        ; // no success!
    }

    // TODO: if (!xRet.is()) rImport.SetError(...);

    return xRet;
}

Reference<XPropertySet> xforms_findXFormsBinding(
    Reference<frame::XModel>& xDocument,
    const rtl::OUString& rBindingID )
{
    return lcl_findXFormsBindingOrSubmission( xDocument, rBindingID, true );
}

Reference<XPropertySet> xforms_findXFormsSubmission(
    Reference<frame::XModel>& xDocument,
    const rtl::OUString& rBindingID )
{
    return lcl_findXFormsBindingOrSubmission( xDocument, rBindingID, false );
}

void xforms_setValue( Reference<XPropertySet>& xPropertySet,
                   const OUString& rName,
                   const Any rAny )
{
    xPropertySet->setPropertyValue( rName, rAny );
}

#define TOKEN_MAP_ENTRY(NAMESPACE,TOKEN) { XML_NAMESPACE_##NAMESPACE, xmloff::token::XML_##TOKEN, xmloff::token::XML_##TOKEN }
static SvXMLTokenMapEntry aTypes[] =
{
    TOKEN_MAP_ENTRY( XSD, STRING  ),
    TOKEN_MAP_ENTRY( XSD, DECIMAL ),
    TOKEN_MAP_ENTRY( XSD, DOUBLE ),
    TOKEN_MAP_ENTRY( XSD, FLOAT ),
    TOKEN_MAP_ENTRY( XSD, BOOLEAN ),
    TOKEN_MAP_ENTRY( XSD, ANYURI ),
    TOKEN_MAP_ENTRY( XSD, DATETIME_XSD ),
    TOKEN_MAP_ENTRY( XSD, DATE ),
    TOKEN_MAP_ENTRY( XSD, TIME ),
    TOKEN_MAP_ENTRY( XSD, YEAR ),
    TOKEN_MAP_ENTRY( XSD, MONTH ),
    TOKEN_MAP_ENTRY( XSD, DAY ),
    XML_TOKEN_MAP_END
};

sal_uInt16 xforms_getTypeClass(
    const Reference<XDataTypeRepository>&
    #ifdef DBG_UTIL
    xRepository
    #endif
    ,
    const SvXMLNamespaceMap& rNamespaceMap,
    const OUString& rXMLName )
{
    // translate name into token for local name
    OUString sLocalName;
    sal_uInt16 nPrefix = rNamespaceMap.GetKeyByAttrName(rXMLName, &sLocalName);
    SvXMLTokenMap aMap( aTypes );
    sal_uInt16 mnToken = aMap.Get( nPrefix, sLocalName );

    sal_uInt16 nTypeClass = com::sun::star::xsd::DataTypeClass::STRING;
    if( mnToken != XML_TOK_UNKNOWN )
    {
        // we found an XSD name: then get the proper API name for it
        DBG_ASSERT( xRepository.is(), "can't find type without repository");
        switch( mnToken )
        {
        case XML_STRING:
            nTypeClass = com::sun::star::xsd::DataTypeClass::STRING;
            break;
        case XML_ANYURI:
            nTypeClass = com::sun::star::xsd::DataTypeClass::anyURI;
            break;
        case XML_DECIMAL:
            nTypeClass = com::sun::star::xsd::DataTypeClass::DECIMAL;
            break;
        case XML_DOUBLE:
            nTypeClass = com::sun::star::xsd::DataTypeClass::DOUBLE;
            break;
        case XML_FLOAT:
            nTypeClass = com::sun::star::xsd::DataTypeClass::FLOAT;
            break;
        case XML_BOOLEAN:
            nTypeClass = com::sun::star::xsd::DataTypeClass::BOOLEAN;
            break;
        case XML_DATETIME_XSD:
            nTypeClass = com::sun::star::xsd::DataTypeClass::DATETIME;
            break;
        case XML_DATE:
            nTypeClass = com::sun::star::xsd::DataTypeClass::DATE;
            break;
        case XML_TIME:
            nTypeClass = com::sun::star::xsd::DataTypeClass::TIME;
            break;
        case XML_YEAR:
            nTypeClass = com::sun::star::xsd::DataTypeClass::gYear;
            break;
        case XML_DAY:
            nTypeClass = com::sun::star::xsd::DataTypeClass::gDay;
            break;
        case XML_MONTH:
            nTypeClass = com::sun::star::xsd::DataTypeClass::gMonth;
            break;

            /* data types not yet supported:
            nTypeClass = com::sun::star::xsd::DataTypeClass::DURATION;
            nTypeClass = com::sun::star::xsd::DataTypeClass::gYearMonth;
            nTypeClass = com::sun::star::xsd::DataTypeClass::gMonthDay;
            nTypeClass = com::sun::star::xsd::DataTypeClass::hexBinary;
            nTypeClass = com::sun::star::xsd::DataTypeClass::base64Binary;
            nTypeClass = com::sun::star::xsd::DataTypeClass::QName;
            nTypeClass = com::sun::star::xsd::DataTypeClass::NOTATION;
            */
        }
    }

    return nTypeClass;
}


rtl::OUString xforms_getTypeName(
    const Reference<XDataTypeRepository>& xRepository,
    const SvXMLNamespaceMap& rNamespaceMap,
    const OUString& rXMLName )
{
    OUString sLocalName;
    sal_uInt16 nPrefix = rNamespaceMap.GetKeyByAttrName(rXMLName, &sLocalName);
    SvXMLTokenMap aMap( aTypes );
    sal_uInt16 mnToken = aMap.Get( nPrefix, sLocalName );
    return ( mnToken == XML_TOK_UNKNOWN )
        ? rXMLName
        : xforms_getBasicTypeName( xRepository, rNamespaceMap, rXMLName );
}

rtl::OUString xforms_getBasicTypeName(
    const Reference<XDataTypeRepository>& xRepository,
    const SvXMLNamespaceMap& rNamespaceMap,
    const OUString& rXMLName )
{
    OUString sTypeName = rXMLName;
    try
    {
        sTypeName =
            xRepository->getBasicDataType(
                xforms_getTypeClass( xRepository, rNamespaceMap, rXMLName ) )
            ->getName();
    }
    catch( const Exception& )
    {
        OSL_FAIL( "exception during type creation" );
    }
    return sTypeName;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
