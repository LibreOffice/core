/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include "xformsapi.hxx"

#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/xforms/XFormsSupplier.hpp>
#include <com/sun/star/xforms/XDataTypeRepository.hpp>
#include <com/sun/star/xforms/Model.hpp>
#include <com/sun/star/xforms/XModel2.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/xsd/DataTypeClass.hpp>

#include <comphelper/processfactory.hxx>
#include <sal/log.hxx>
#include <tools/diagnose_ex.h>

#include <xmloff/xmltoken.hxx>
#include <xmloff/namespacemap.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltkmap.hxx>

using com::sun::star::uno::Reference;
using com::sun::star::uno::Sequence;
using com::sun::star::uno::UNO_QUERY;
using com::sun::star::uno::UNO_QUERY_THROW;
using com::sun::star::beans::XPropertySet;
using com::sun::star::container::XNameAccess;
using com::sun::star::xforms::XFormsSupplier;
using com::sun::star::xforms::XDataTypeRepository;
using com::sun::star::xforms::Model;
using com::sun::star::xforms::XModel2;
using com::sun::star::container::XNameContainer;
using com::sun::star::uno::makeAny;
using com::sun::star::uno::Any;
using com::sun::star::uno::Exception;

using namespace com::sun::star;
using namespace xmloff::token;

Reference<XModel2> xforms_createXFormsModel()
{
    Reference<XModel2> xModel = Model::create( comphelper::getProcessComponentContext() );

    return xModel;
}

void xforms_addXFormsModel(
    const Reference<frame::XModel>& xDocument,
    const Reference<xforms::XModel2>& xModel )
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
                xModel->getPropertyValue("ID") >>= sName;
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
    SAL_WARN_IF( !bSuccess, "xmloff", "can't import model" );
}

static Reference<XPropertySet> lcl_findXFormsBindingOrSubmission(
    Reference<frame::XModel> const & xDocument,
    const OUString& rBindingID,
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
                const Sequence<OUString> aNames = xForms->getElementNames();
                for( const auto& rName : aNames )
                {
                    Reference<xforms::XModel2> xModel(
                        xForms->getByName( rName ), UNO_QUERY );
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

                    if (xRet.is())
                        break;
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
    Reference<frame::XModel> const & xDocument,
    const OUString& rBindingID )
{
    return lcl_findXFormsBindingOrSubmission( xDocument, rBindingID, true );
}

Reference<XPropertySet> xforms_findXFormsSubmission(
    Reference<frame::XModel> const & xDocument,
    const OUString& rBindingID )
{
    return lcl_findXFormsBindingOrSubmission( xDocument, rBindingID, false );
}

void xforms_setValue( Reference<XPropertySet> const & xPropertySet,
                   const OUString& rName,
                   const Any& rAny )
{
    xPropertySet->setPropertyValue( rName, rAny );
}

#define TOKEN_MAP_ENTRY(NAMESPACE,TOKEN) { XML_NAMESPACE_##NAMESPACE, xmloff::token::XML_##TOKEN, xmloff::token::XML_##TOKEN }
const SvXMLTokenMapEntry aTypes[] =
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
    const Reference<XDataTypeRepository>& xRepository,
    const SvXMLNamespaceMap& rNamespaceMap,
    const OUString& rXMLName )
{
    // translate name into token for local name
    OUString sLocalName;
    sal_uInt16 nPrefix = rNamespaceMap.GetKeyByAttrValueQName(rXMLName, &sLocalName);
    static const SvXMLTokenMap aMap( aTypes );
    sal_uInt16 nToken = aMap.Get( nPrefix, sLocalName );

    sal_uInt16 nTypeClass = css::xsd::DataTypeClass::STRING;
    if( nToken != XML_TOK_UNKNOWN )
    {
        // we found an XSD name: then get the proper API name for it
        SAL_WARN_IF( !xRepository.is(), "xmloff", "can't find type without repository");
        switch( nToken )
        {
        case XML_STRING:
            nTypeClass = css::xsd::DataTypeClass::STRING;
            break;
        case XML_ANYURI:
            nTypeClass = css::xsd::DataTypeClass::anyURI;
            break;
        case XML_DECIMAL:
            nTypeClass = css::xsd::DataTypeClass::DECIMAL;
            break;
        case XML_DOUBLE:
            nTypeClass = css::xsd::DataTypeClass::DOUBLE;
            break;
        case XML_FLOAT:
            nTypeClass = css::xsd::DataTypeClass::FLOAT;
            break;
        case XML_BOOLEAN:
            nTypeClass = css::xsd::DataTypeClass::BOOLEAN;
            break;
        case XML_DATETIME_XSD:
            nTypeClass = css::xsd::DataTypeClass::DATETIME;
            break;
        case XML_DATE:
            nTypeClass = css::xsd::DataTypeClass::DATE;
            break;
        case XML_TIME:
            nTypeClass = css::xsd::DataTypeClass::TIME;
            break;
        case XML_YEAR:
            nTypeClass = css::xsd::DataTypeClass::gYear;
            break;
        case XML_DAY:
            nTypeClass = css::xsd::DataTypeClass::gDay;
            break;
        case XML_MONTH:
            nTypeClass = css::xsd::DataTypeClass::gMonth;
            break;

            /* data types not yet supported:
            nTypeClass = css::xsd::DataTypeClass::DURATION;
            nTypeClass = css::xsd::DataTypeClass::gYearMonth;
            nTypeClass = css::xsd::DataTypeClass::gMonthDay;
            nTypeClass = css::xsd::DataTypeClass::hexBinary;
            nTypeClass = css::xsd::DataTypeClass::base64Binary;
            nTypeClass = css::xsd::DataTypeClass::QName;
            nTypeClass = css::xsd::DataTypeClass::NOTATION;
            */
        }
    }

    return nTypeClass;
}


OUString xforms_getTypeName(
    const Reference<XDataTypeRepository>& xRepository,
    const SvXMLNamespaceMap& rNamespaceMap,
    const OUString& rXMLName )
{
    OUString sLocalName;
    sal_uInt16 nPrefix = rNamespaceMap.GetKeyByAttrValueQName(rXMLName, &sLocalName);
    static const SvXMLTokenMap aMap( aTypes );
    sal_uInt16 nToken = aMap.Get( nPrefix, sLocalName );
    return ( nToken == XML_TOK_UNKNOWN )
        ? rXMLName
        : xforms_getBasicTypeName( xRepository, rNamespaceMap, rXMLName );
}

OUString xforms_getBasicTypeName(
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
        TOOLS_WARN_EXCEPTION("xmloff", "exception during type creation");
    }
    return sTypeName;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
