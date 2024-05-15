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


#include <xmloff/xformsexport.hxx>

#include "XFormsModelExport.hxx"

#include <xmloff/xmlexp.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/namespacemap.hxx>
#include <DomExport.hxx>

#include <sax/tools/converter.hxx>

#include <comphelper/processfactory.hxx>
#include <comphelper/propertyvalue.hxx>

#include <comphelper/diagnose_ex.hxx>
#include <sal/log.hxx>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/document/NamedPropertyValues.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/xml/dom/XDocument.hpp>
#include <com/sun/star/form/binding/XBindableValue.hpp>
#include <com/sun/star/form/binding/XListEntrySink.hpp>
#include <com/sun/star/form/submission/XSubmissionSupplier.hpp>
#include <com/sun/star/xforms/XModel.hpp>
#include <com/sun/star/xforms/XDataTypeRepository.hpp>
#include <com/sun/star/xforms/XFormsSupplier.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/xsd/WhiteSpaceTreatment.hpp>
#include <com/sun/star/xsd/DataTypeClass.hpp>
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/util/Time.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/util/Duration.hpp>

using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace xmloff::token;

using com::sun::star::beans::XPropertySet;
using com::sun::star::beans::XPropertySetInfo;
using com::sun::star::container::XIndexAccess;
using com::sun::star::container::XNameAccess;
using com::sun::star::container::XNameContainer;
using com::sun::star::container::XEnumerationAccess;
using com::sun::star::container::XEnumeration;
using com::sun::star::xml::dom::XDocument;
using com::sun::star::form::binding::XBindableValue;
using com::sun::star::form::binding::XListEntrySink;
using com::sun::star::form::submission::XSubmissionSupplier;
using com::sun::star::beans::PropertyValue;
using com::sun::star::xforms::XDataTypeRepository;
using com::sun::star::xforms::XFormsSupplier;
using com::sun::star::util::Duration;

void exportXForms( SvXMLExport& rExport )
{
    Reference<XFormsSupplier> xSupplier( rExport.GetModel(), UNO_QUERY );
    if( !xSupplier.is() )
        return;

    Reference<XNameContainer> xForms = xSupplier->getXForms();
    if( xForms.is() )
    {
        const Sequence<OUString> aNames = xForms->getElementNames();

        for( const auto& rName : aNames )
        {
            Reference<XPropertySet> xModel( xForms->getByName( rName ),
                                            UNO_QUERY );
            exportXFormsModel( rExport, xModel );
        }
    }
}


static void exportXFormsInstance( SvXMLExport&, const Sequence<PropertyValue>& );
static void exportXFormsBinding( SvXMLExport&, const Reference<XPropertySet>& );
static void exportXFormsSubmission( SvXMLExport&, const Reference<XPropertySet>& );
static void exportXFormsSchemas( SvXMLExport&, const Reference<css::xforms::XModel>& );


typedef OUString (*convert_t)( const Any& );

namespace {

struct ExportTable
{
    OUString pPropertyName;
    sal_uInt16 nNamespace;
    sal_uInt16 nToken;
    convert_t aConverter;
};

}

static void lcl_export( const Reference<XPropertySet>& rPropertySet,
                 SvXMLExport& rExport,
                 const ExportTable* pTable );

#define TABLE_END { u""_ustr, 0, 0, nullptr }

// any conversion functions
static OUString xforms_string( const Any& );
static OUString xforms_bool( const Any& );
static OUString xforms_whitespace( const Any& );
template<typename T, void (*FUNC)( OUStringBuffer&, T )> static OUString xforms_convert( const Any& );
template<typename T, void (*FUNC)( OUStringBuffer&, const T& )> static OUString xforms_convertRef( const Any& );

static void xforms_formatDate( OUStringBuffer& aBuffer, const util::Date& aDate );
static void xforms_formatTime( OUStringBuffer& aBuffer, const css::util::Time& aTime );
static void xforms_formatDateTime( OUStringBuffer& aBuffer, const util::DateTime& aDateTime );

static void convertNumber(OUStringBuffer & b, sal_Int32 n) {
    b.append(n);
}

convert_t const xforms_int32    = &xforms_convert<sal_Int32,&convertNumber>;
convert_t const xforms_double   = &xforms_convert<double,&::sax::Converter::convertDouble>;
convert_t const xforms_dateTime = &xforms_convertRef<util::DateTime,&xforms_formatDateTime>;
convert_t const xforms_date     = &xforms_convertRef<util::Date,&xforms_formatDate>;
convert_t const xforms_time     = &xforms_convertRef<css::util::Time,&xforms_formatTime>;

// other functions
static OUString lcl_getXSDType( SvXMLExport const & rExport,
                         const Reference<XPropertySet>& xType );


// the model


constexpr ExportTable aXFormsModelTable[] =
{
    { u"ID"_ustr, XML_NAMESPACE_NONE, xmloff::token::XML_ID, xforms_string },
    { u"SchemaRef"_ustr, XML_NAMESPACE_NONE, xmloff::token::XML_SCHEMA, xforms_string },
    TABLE_END
};

void exportXFormsModel( SvXMLExport& rExport,
                        const Reference<XPropertySet>& xModelPropSet )
{
    // no model -> don't do anything!
    Reference<css::xforms::XModel> xModel( xModelPropSet, UNO_QUERY );
    if( ! xModel.is() || ! xModelPropSet.is() )
        return;

    lcl_export( xModelPropSet, rExport, aXFormsModelTable );
    SvXMLElementExport aModelElement( rExport, XML_NAMESPACE_XFORMS, XML_MODEL,
                                      true, true );

    // instances
    Reference<XIndexAccess> xInstances( xModel->getInstances(),
                                        UNO_QUERY_THROW);
    sal_Int32 nCount = xInstances->getCount();
    sal_Int32 i = 0;
    for( i = 0; i < nCount; i++ )
    {
        Sequence<PropertyValue> aInstance;
        xInstances->getByIndex( i ) >>= aInstance;
        exportXFormsInstance( rExport, aInstance );
    }


    // bindings
    Reference<XIndexAccess> xBindings( xModel->getBindings(), UNO_QUERY_THROW);
    nCount = xBindings->getCount();
    for( i = 0; i < nCount; i++ )
    {
        Reference<XPropertySet> aBinding( xBindings->getByIndex( i ),
                                          UNO_QUERY_THROW );
        exportXFormsBinding( rExport, aBinding );
    }

    // submissions
    Reference<XIndexAccess> xSubmissions( xModel->getSubmissions(),
                                          UNO_QUERY_THROW );
    nCount = xSubmissions->getCount();
    for( i = 0; i < nCount; i++ )
    {
        Reference<XPropertySet> xSubmission( xSubmissions->getByIndex( i ),
                                          UNO_QUERY_THROW );
        exportXFormsSubmission( rExport, xSubmission );
    }

    // schemas
    exportXFormsSchemas( rExport, xModel );
}


// the instance


void exportXFormsInstance( SvXMLExport& rExport,
                           const Sequence<PropertyValue>& xInstance )
{
    OUString sId;
    OUString sURL;
    Reference<XDocument> xDoc;

    for( const auto& rProp : xInstance )
    {
        OUString sName = rProp.Name;
        const Any& rAny = rProp.Value;
        if ( sName == "ID" )
            rAny >>= sId;
        else if ( sName == "URL" )
            rAny >>= sURL;
        else if ( sName == "Instance" )
            rAny >>= xDoc;
    }

    if( !sId.isEmpty() )
        rExport.AddAttribute( XML_NAMESPACE_NONE, XML_ID, sId );

    if( !sURL.isEmpty() )
        rExport.AddAttribute( XML_NAMESPACE_NONE, XML_SRC, sURL );

    SvXMLElementExport aElem( rExport, XML_NAMESPACE_XFORMS, XML_INSTANCE,
                              true, true );
    rExport.IgnorableWhitespace();
    if( xDoc.is() )
    {
        exportDom( rExport, xDoc );
    }
}


// the binding


constexpr ExportTable aXFormsBindingTable[] =
{
    { u"BindingID"_ustr, XML_NAMESPACE_NONE, xmloff::token::XML_ID, xforms_string },
    { u"BindingExpression"_ustr, XML_NAMESPACE_NONE, xmloff::token::XML_NODESET, xforms_string },
    { u"ReadonlyExpression"_ustr, XML_NAMESPACE_NONE, xmloff::token::XML_READONLY, xforms_string },
    { u"RelevantExpression"_ustr, XML_NAMESPACE_NONE, xmloff::token::XML_RELEVANT, xforms_string },
    { u"RequiredExpression"_ustr, XML_NAMESPACE_NONE, xmloff::token::XML_REQUIRED, xforms_string },
    { u"ConstraintExpression"_ustr, XML_NAMESPACE_NONE, xmloff::token::XML_CONSTRAINT, xforms_string },
    { u"CalculateExpression"_ustr, XML_NAMESPACE_NONE, xmloff::token::XML_CALCULATE, xforms_string },
    // type handled separately, for type name <-> XSD type conversion
    // { "Type", XML_NAMESPACE_NONE, xmloff::token::XML_TYPE, xforms_string },
    TABLE_END
};

void exportXFormsBinding( SvXMLExport& rExport,
                        const Reference<XPropertySet>& xBinding )
{
    // name check; generate binding ID if necessary
    {
        OUString sName;
        xBinding->getPropertyValue( u"BindingID"_ustr ) >>= sName;
        if( sName.isEmpty() )
        {
            // if we don't have a name yet, generate one on the fly
            sal_Int64 nId = reinterpret_cast<sal_uInt64>( xBinding.get() );
            sName = "bind_" + OUString::number( nId , 16 );
            xBinding->setPropertyValue( u"BindingID"_ustr, Any(sName));
        }
    }

    lcl_export( xBinding, rExport, aXFormsBindingTable );

    // handle type attribute
    {
        OUString sTypeName;
        xBinding->getPropertyValue( u"Type"_ustr ) >>= sTypeName;

        try
        {
            // now get type, and determine whether it's a standard type. If
            // so, export the XSD name
            Reference<css::xforms::XModel> xModel(
                xBinding->getPropertyValue( u"Model"_ustr ),
                UNO_QUERY );
            Reference<XDataTypeRepository> xRepository(
                xModel.is() ? xModel->getDataTypeRepository() : Reference<XDataTypeRepository>() );
            if( xRepository.is() )
            {
                Reference<XPropertySet> xDataType =
                    xRepository->getDataType( sTypeName );

                // if it's a basic data type, write out the XSD name
                // for the XSD type class
                bool bIsBasic = false;
                xDataType->getPropertyValue( u"IsBasic"_ustr ) >>= bIsBasic;
                if( bIsBasic )
                    sTypeName = lcl_getXSDType( rExport, xDataType );
            }
        }
        catch( Exception& )
        {
            ; // ignore; just use typename
        }

        // now that we have the proper type name, write out the attribute
        if( !sTypeName.isEmpty() )
        {
            rExport.AddAttribute( XML_NAMESPACE_NONE, XML_TYPE,
                                  sTypeName );
        }
    }

    // we need to ensure all the namespaces in the binding will work correctly.
    // to do so, we will write out all missing namespace declaractions.
    const SvXMLNamespaceMap& rMap = rExport.GetNamespaceMap();
    Reference<XNameAccess> xNamespaces(
        xBinding->getPropertyValue( u"ModelNamespaces"_ustr ), UNO_QUERY);
    if( xNamespaces.is() )
    {
        // iterate over Prefixes for this binding
        const Sequence<OUString> aPrefixes = xNamespaces->getElementNames();
        for( const OUString& rPrefix : aPrefixes )
        {
            OUString sURI;
            xNamespaces->getByName( rPrefix ) >>= sURI;

            // check whether prefix/URI pair is in map; else write declaration
            // (we don't need to change the map, since this element has no
            // other content)
            sal_uInt16 nKey = rMap.GetKeyByPrefix( rPrefix );
            if( nKey == XML_NAMESPACE_UNKNOWN  ||
                rMap.GetNameByKey( nKey ) != sURI )
            {
                // add declaration if it doesn't already exist
                comphelper::AttributeList& rAttrList = rExport.GetAttrList();
                OUString sName = "xmlns:" + rPrefix;
                sal_Int16 nFound = rAttrList.GetIndexByName(sName);
                // duplicate xmlns:script, http://openoffice.org/2000/script seen
                assert(nFound == -1 || rAttrList.getValueByIndex(nFound) == sURI);
                if (nFound != -1)
                    continue;
                rAttrList.AddAttribute(sName, sURI);
            }
        }
    }

    SvXMLElementExport aElement( rExport, XML_NAMESPACE_XFORMS, XML_BIND,
                                 true, true );
}


// the submission


constexpr ExportTable aXFormsSubmissionTable[] =
{
    { u"ID"_ustr, XML_NAMESPACE_NONE, xmloff::token::XML_ID, xforms_string },
    { u"Bind"_ustr, XML_NAMESPACE_NONE, xmloff::token::XML_BIND, xforms_string },
    { u"Ref"_ustr, XML_NAMESPACE_NONE, xmloff::token::XML_REF, xforms_string },
    { u"Action"_ustr, XML_NAMESPACE_NONE, xmloff::token::XML_ACTION, xforms_string },
    { u"Method"_ustr, XML_NAMESPACE_NONE, xmloff::token::XML_METHOD, xforms_string },
    { u"Version"_ustr, XML_NAMESPACE_NONE, xmloff::token::XML_VERSION, xforms_string },
    { u"Indent"_ustr, XML_NAMESPACE_NONE, xmloff::token::XML_INDENT, xforms_bool },
    { u"MediaType"_ustr, XML_NAMESPACE_NONE, xmloff::token::XML_MEDIATYPE, xforms_string },
    { u"Encoding"_ustr, XML_NAMESPACE_NONE, xmloff::token::XML_ENCODING, xforms_string },
    { u"OmitXmlDeclaration"_ustr, XML_NAMESPACE_NONE, xmloff::token::XML_OMIT_XML_DECLARATION, xforms_bool },
    { u"Standalone"_ustr, XML_NAMESPACE_NONE, xmloff::token::XML_STANDALONE, xforms_bool },
    { u"CDataSectionElement"_ustr, XML_NAMESPACE_NONE, xmloff::token::XML_CDATA_SECTION_ELEMENTS, xforms_string },
    { u"Replace"_ustr, XML_NAMESPACE_NONE, xmloff::token::XML_REPLACE, xforms_string },
    { u"Separator"_ustr, XML_NAMESPACE_NONE, xmloff::token::XML_SEPARATOR, xforms_string },
    { u"IncludeNamespacePrefixes"_ustr, XML_NAMESPACE_NONE, xmloff::token::XML_INCLUDENAMESPACEPREFIXES, xforms_string },
    TABLE_END
};

void exportXFormsSubmission( SvXMLExport& rExport,
                             const Reference<XPropertySet>& xSubmission )
{
    lcl_export( xSubmission, rExport, aXFormsSubmissionTable );
    SvXMLElementExport aElement( rExport, XML_NAMESPACE_XFORMS, XML_SUBMISSION,
                                 true, true );
}


// export data types as XSD schema


const ExportTable aDataTypeFacetTable[] =
{
    { u"Length"_ustr, XML_NAMESPACE_XSD, xmloff::token::XML_LENGTH, xforms_int32 },
    { u"MinLength"_ustr, XML_NAMESPACE_XSD, xmloff::token::XML_MINLENGTH, xforms_int32 },
    { u"MaxLength"_ustr, XML_NAMESPACE_XSD, xmloff::token::XML_MAXLENGTH, xforms_int32 },
    { u"MinInclusiveInt"_ustr, XML_NAMESPACE_XSD, xmloff::token::XML_MININCLUSIVE, xforms_int32 },
    { u"MinExclusiveInt"_ustr, XML_NAMESPACE_XSD, xmloff::token::XML_MINEXCLUSIVE, xforms_int32 },
    { u"MaxInclusiveInt"_ustr, XML_NAMESPACE_XSD, xmloff::token::XML_MAXINCLUSIVE, xforms_int32 },
    { u"MaxExclusiveInt"_ustr, XML_NAMESPACE_XSD, xmloff::token::XML_MAXEXCLUSIVE, xforms_int32 },
    { u"MinInclusiveDouble"_ustr, XML_NAMESPACE_XSD, xmloff::token::XML_MININCLUSIVE, xforms_double },
    { u"MinExclusiveDouble"_ustr, XML_NAMESPACE_XSD, xmloff::token::XML_MINEXCLUSIVE, xforms_double },
    { u"MaxInclusiveDouble"_ustr, XML_NAMESPACE_XSD, xmloff::token::XML_MAXINCLUSIVE, xforms_double },
    { u"MaxExclusiveDouble"_ustr, XML_NAMESPACE_XSD, xmloff::token::XML_MAXEXCLUSIVE, xforms_double },
    { u"MinInclusiveDate"_ustr, XML_NAMESPACE_XSD, xmloff::token::XML_MININCLUSIVE, xforms_date },
    { u"MinExclusiveDate"_ustr, XML_NAMESPACE_XSD, xmloff::token::XML_MINEXCLUSIVE, xforms_date },
    { u"MaxInclusiveDate"_ustr, XML_NAMESPACE_XSD, xmloff::token::XML_MAXINCLUSIVE, xforms_date },
    { u"MaxExclusiveDate"_ustr, XML_NAMESPACE_XSD, xmloff::token::XML_MAXEXCLUSIVE, xforms_date },
    { u"MinInclusiveTime"_ustr, XML_NAMESPACE_XSD, xmloff::token::XML_MININCLUSIVE, xforms_time },
    { u"MinExclusiveTime"_ustr, XML_NAMESPACE_XSD, xmloff::token::XML_MINEXCLUSIVE, xforms_time },
    { u"MaxInclusiveTime"_ustr, XML_NAMESPACE_XSD, xmloff::token::XML_MAXINCLUSIVE, xforms_time },
    { u"MaxExclusiveTime"_ustr, XML_NAMESPACE_XSD, xmloff::token::XML_MAXEXCLUSIVE, xforms_time },
    { u"MinInclusiveDateTime"_ustr, XML_NAMESPACE_XSD, xmloff::token::XML_MININCLUSIVE, xforms_dateTime },
    { u"MinExclusiveDateTime"_ustr, XML_NAMESPACE_XSD, xmloff::token::XML_MINEXCLUSIVE, xforms_dateTime },
    { u"MaxInclusiveDateTime"_ustr, XML_NAMESPACE_XSD, xmloff::token::XML_MAXINCLUSIVE, xforms_dateTime },
    { u"MaxExclusiveDateTime"_ustr, XML_NAMESPACE_XSD, xmloff::token::XML_MAXEXCLUSIVE, xforms_dateTime },
    { u"Pattern"_ustr, XML_NAMESPACE_XSD, xmloff::token::XML_PATTERN, xforms_string },
    // ??? XML_ENUMERATION,
    { u"WhiteSpace"_ustr, XML_NAMESPACE_XSD, xmloff::token::XML_WHITESPACE, xforms_whitespace },
    { u"TotalDigits"_ustr, XML_NAMESPACE_XSD, xmloff::token::XML_TOTALDIGITS, xforms_int32 },
    { u"FractionDigits"_ustr, XML_NAMESPACE_XSD, xmloff::token::XML_FRACTIONDIGITS, xforms_int32 },
    TABLE_END
};

// export facets through table; use the same table as lcl_export does
static void lcl_exportDataTypeFacets( SvXMLExport& rExport,
                               const Reference<XPropertySet>& rPropertySet,
                               const ExportTable* pTable )
{
    Reference<XPropertySetInfo> xInfo = rPropertySet->getPropertySetInfo();
    for( const ExportTable* pCurrent = pTable;
         !pCurrent->pPropertyName.isEmpty();
         pCurrent++ )
    {
        OUString sName( pCurrent->pPropertyName );
        if( xInfo->hasPropertyByName( sName ) )
        {
            OUString sValue = (*pCurrent->aConverter)(
                rPropertySet->getPropertyValue( sName ) );

            if( !sValue.isEmpty() )
            {
                rExport.AddAttribute( XML_NAMESPACE_NONE, XML_VALUE, sValue );
                SvXMLElementExport aFacet(
                    rExport,
                    pCurrent->nNamespace,
                    static_cast<XMLTokenEnum>( pCurrent->nToken ),
                    true, true );
            }
        }
    }
}

static OUString lcl_getXSDType( SvXMLExport const & rExport,
                         const Reference<XPropertySet>& xType )
{
    // we use string as default...
    XMLTokenEnum eToken = XML_STRING;

    sal_uInt16 nDataTypeClass = 0;
    xType->getPropertyValue( u"TypeClass"_ustr ) >>= nDataTypeClass;
    switch( nDataTypeClass )
    {
    case css::xsd::DataTypeClass::STRING:
        eToken = XML_STRING;
        break;
    case css::xsd::DataTypeClass::anyURI:
        eToken = XML_ANYURI;
        break;
    case css::xsd::DataTypeClass::DECIMAL:
        eToken = XML_DECIMAL;
        break;
    case css::xsd::DataTypeClass::DOUBLE:
        eToken = XML_DOUBLE;
        break;
    case css::xsd::DataTypeClass::FLOAT:
        eToken = XML_FLOAT;
        break;
    case css::xsd::DataTypeClass::BOOLEAN:
        eToken = XML_BOOLEAN;
        break;
    case css::xsd::DataTypeClass::DATETIME:
        eToken = XML_DATETIME_XSD;
        break;
    case css::xsd::DataTypeClass::TIME:
        eToken = XML_TIME;
        break;
    case css::xsd::DataTypeClass::DATE:
        eToken = XML_DATE;
        break;
    case css::xsd::DataTypeClass::gYear:
        eToken = XML_YEAR;
        break;
    case css::xsd::DataTypeClass::gDay:
        eToken = XML_DAY;
        break;
    case css::xsd::DataTypeClass::gMonth:
        eToken = XML_MONTH;
        break;
    case css::xsd::DataTypeClass::DURATION:
    case css::xsd::DataTypeClass::gYearMonth:
    case css::xsd::DataTypeClass::gMonthDay:
    case css::xsd::DataTypeClass::hexBinary:
    case css::xsd::DataTypeClass::base64Binary:
    case css::xsd::DataTypeClass::QName:
    case css::xsd::DataTypeClass::NOTATION:
    default:
        OSL_FAIL( "unknown data type" );
    }

    return rExport.GetNamespaceMap().GetQNameByKey( XML_NAMESPACE_XSD,
                                                    GetXMLToken( eToken ) );
}

static void lcl_exportDataType( SvXMLExport& rExport,
                         const Reference<XPropertySet>& xType )
{
    // we do not need to export basic types; exit if we have one
    bool bIsBasic = false;
    xType->getPropertyValue( u"IsBasic"_ustr ) >>= bIsBasic;
    if( bIsBasic )
        return;

    // no basic type -> export

    // <xsd:simpleType name="...">
    OUString sName;
    xType->getPropertyValue( u"Name"_ustr ) >>= sName;
    rExport.AddAttribute( XML_NAMESPACE_NONE, XML_NAME, sName );
    SvXMLElementExport aSimpleType( rExport,
                                    XML_NAMESPACE_XSD, XML_SIMPLETYPE,
                                    true, true );

    // <xsd:restriction base="xsd:...">
    rExport.AddAttribute( XML_NAMESPACE_NONE, XML_BASE,
                          lcl_getXSDType( rExport, xType ) );
    SvXMLElementExport aRestriction( rExport,
                                     XML_NAMESPACE_XSD,
                                     XML_RESTRICTION,
                                     true, true );

    // export facets
    lcl_exportDataTypeFacets( rExport,
                              xType,
                              aDataTypeFacetTable );
}

void exportXFormsSchemas( SvXMLExport& rExport,
                          const Reference<css::xforms::XModel>& xModel )
{
    // TODO: for now, we'll fake this...
    {
        SvXMLElementExport aSchemaElem( rExport, XML_NAMESPACE_XSD, XML_SCHEMA,
                                        true, true );

        // now get data type repository, and export
        Reference<XEnumerationAccess> xTypes = xModel->getDataTypeRepository();
        if( xTypes.is() )
        {
            Reference<XEnumeration> xEnum = xTypes->createEnumeration();
            SAL_WARN_IF( !xEnum.is(), "xmloff", "no enum?" );
            while( xEnum->hasMoreElements() )
            {
                Reference<XPropertySet> xType( xEnum->nextElement(), UNO_QUERY );
                lcl_exportDataType( rExport, xType );
            }
        }
    }

    // export other, 'foreign' schemas
    Reference<XPropertySet> xPropSet( xModel, UNO_QUERY );
    if( xPropSet.is() )
    {
        Reference<XDocument> xDocument(
            xPropSet->getPropertyValue( u"ForeignSchema"_ustr ),
            UNO_QUERY );

        if( xDocument.is() )
            exportDom( rExport, xDocument );
    }
}


// helper functions


static void lcl_export( const Reference<XPropertySet>& rPropertySet,
                 SvXMLExport& rExport,
                 const ExportTable* pTable )
{
    for( const ExportTable* pCurrent = pTable;
         !pCurrent->pPropertyName.isEmpty();
         pCurrent++ )
    {
        Any aAny = rPropertySet->getPropertyValue( pCurrent->pPropertyName );
        OUString sValue = (*pCurrent->aConverter)( aAny );

        if( !sValue.isEmpty() )
            rExport.AddAttribute(
                pCurrent->nNamespace,
                static_cast<XMLTokenEnum>( pCurrent->nToken ),
                sValue );
    }
}


// any conversion functions


template<typename T, void (*FUNC)( OUStringBuffer&, T )>
OUString xforms_convert( const Any& rAny )
{
    OUStringBuffer aBuffer;
    T aData = T();
    if( rAny >>= aData )
    {
        FUNC( aBuffer, aData );
    }
    return aBuffer.makeStringAndClear();
}

template<typename T, void (*FUNC)( OUStringBuffer&, const T& )>
OUString xforms_convertRef( const Any& rAny )
{
    OUStringBuffer aBuffer;
    T aData;
    if( rAny >>= aData )
    {
        FUNC( aBuffer, aData );
    }
    return aBuffer.makeStringAndClear();
}

OUString xforms_string( const Any& rAny )
{
    OUString aResult;
    rAny >>= aResult;
    return aResult;
}

OUString xforms_bool( const Any& rAny )
{
    bool bResult = bool();
    if( rAny >>= bResult )
        return GetXMLToken( bResult ? XML_TRUE : XML_FALSE );
    OSL_FAIL( "expected boolean value" );
    return OUString();
}

void xforms_formatDate( OUStringBuffer& aBuffer, const util::Date& rDate )
{
    aBuffer.append( OUString::number( rDate.Year ) +
                "-" + OUString::number( rDate.Month ) +
                "-" + OUString::number( rDate.Day ) );
}

void xforms_formatTime( OUStringBuffer& aBuffer, const css::util::Time& rTime )
{
    Duration aDuration;
    aDuration.Hours = rTime.Hours;
    aDuration.Minutes = rTime.Minutes;
    aDuration.Seconds = rTime.Seconds;
    aDuration.NanoSeconds = rTime.NanoSeconds;
    ::sax::Converter::convertDuration( aBuffer, aDuration );
}

void xforms_formatDateTime( OUStringBuffer& aBuffer, const util::DateTime& aDateTime )
{
    ::sax::Converter::convertDateTime(aBuffer, aDateTime, nullptr);
}

OUString xforms_whitespace( const Any& rAny )
{
    OUString sResult;
    sal_uInt16 n = sal_uInt16();
    if( rAny >>= n )
    {
        switch( n )
        {
        case css::xsd::WhiteSpaceTreatment::Preserve:
            sResult = GetXMLToken( XML_PRESERVE );
            break;
        case css::xsd::WhiteSpaceTreatment::Replace:
            sResult = GetXMLToken( XML_REPLACE );
            break;
        case css::xsd::WhiteSpaceTreatment::Collapse:
            sResult = GetXMLToken( XML_COLLAPSE );
            break;
        }
    }
    return sResult;
}


/// return name of Binding
static OUString lcl_getXFormsBindName( const Reference<XPropertySet>& xBinding )
{
    OUString sProp( u"BindingID"_ustr );

    OUString sReturn;
    if( xBinding.is() &&
        xBinding->getPropertySetInfo()->hasPropertyByName( sProp ) )
    {
        xBinding->getPropertyValue( sProp ) >>= sReturn;
    }
    return sReturn;
}

// return name of binding
OUString getXFormsBindName( const Reference<XPropertySet>& xControl )
{
    Reference<XBindableValue> xBindable( xControl, UNO_QUERY );
    return xBindable.is()
        ? lcl_getXFormsBindName(
            Reference<XPropertySet>( xBindable->getValueBinding(), UNO_QUERY ))
        : OUString();
}

// return name of list binding
OUString getXFormsListBindName( const Reference<XPropertySet>& xControl )
{
    Reference<XListEntrySink> xListEntrySink( xControl, UNO_QUERY );
    return xListEntrySink.is()
        ? lcl_getXFormsBindName(
            Reference<XPropertySet>( xListEntrySink->getListEntrySource(),
                                     UNO_QUERY ) )
        : OUString();
}

OUString getXFormsSubmissionName( const Reference<XPropertySet>& xBinding )
{
    OUString sReturn;

    Reference<XSubmissionSupplier> xSubmissionSupplier( xBinding, UNO_QUERY );
    if( xSubmissionSupplier.is() )
    {
        Reference<XPropertySet> xPropertySet(
            xSubmissionSupplier->getSubmission(), UNO_QUERY );
        OUString sProp( u"ID"_ustr );
        if( xPropertySet.is() &&
            xPropertySet->getPropertySetInfo()->hasPropertyByName( sProp ) )
        {
            xPropertySet->getPropertyValue( sProp ) >>= sReturn;
        }
    }

    return sReturn;
}

void getXFormsSettings( const Reference< XNameAccess >& _rXForms, Sequence< PropertyValue >& _out_rSettings )
{
    _out_rSettings = Sequence< PropertyValue >();

    OSL_PRECOND( _rXForms.is(), "getXFormsSettings: invalid XForms container!" );
    if ( !_rXForms.is() )
        return;

    try
    {
        // we want to export some special properties of our XForms models as config-item-map-named,
        // which implies we need a PropertyValue whose value is an XNameAccess, whose keys
        // are the names of the XForm models, and which in turn provides named sequences of
        // PropertyValues - which denote the actual property values of the given named model.

        const Sequence< OUString > aModelNames( _rXForms->getElementNames() );

        Reference< XNameContainer > xModelSettings = document::NamedPropertyValues::create( comphelper::getProcessComponentContext() );

        for ( auto const & modelName : aModelNames )
        {
            Reference< XPropertySet > xModelProps( _rXForms->getByName( modelName ), UNO_QUERY_THROW );

            static constexpr OUString sExternalData = u"ExternalData"_ustr;
            Sequence<PropertyValue> aModelSettings{ comphelper::makePropertyValue(
                sExternalData, xModelProps->getPropertyValue(sExternalData)) };

            xModelSettings->insertByName( modelName, Any( aModelSettings ) );
        }

        if ( xModelSettings->hasElements() )
        {
            _out_rSettings = { comphelper::makePropertyValue(u"XFormModels"_ustr, xModelSettings) };
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("xmloff");
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
