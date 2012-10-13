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


#include "xmloff/xformsexport.hxx"

#include "XFormsModelExport.hxx"
#include "xformsapi.hxx"

#include <xmloff/xmlexp.hxx>
#include <xmloff/xmltoken.hxx>
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/nmspmap.hxx>
#include "DomExport.hxx"

#include <sax/tools/converter.hxx>

#include <comphelper/componentcontext.hxx>
#include <comphelper/processfactory.hxx>

#include "tools/debug.hxx"
#include <tools/diagnose_ex.h>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/xml/dom/XDocument.hpp>
#include <com/sun/star/form/binding/XValueBinding.hpp>
#include <com/sun/star/form/binding/XBindableValue.hpp>
#include <com/sun/star/form/binding/XListEntrySink.hpp>
#include <com/sun/star/form/binding/XListEntrySource.hpp>
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
#include <com/sun/star/xsd/XDataType.hpp>
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/util/Time.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/util/Duration.hpp>

using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace xmloff::token;

using rtl::OUString;
using rtl::OUStringBuffer;
using com::sun::star::beans::XPropertySet;
using com::sun::star::beans::XPropertySetInfo;
using com::sun::star::container::XIndexAccess;
using com::sun::star::container::XNameAccess;
using com::sun::star::container::XNameContainer;
using com::sun::star::container::XEnumerationAccess;
using com::sun::star::container::XEnumeration;
using com::sun::star::xml::dom::XDocument;
using com::sun::star::form::binding::XValueBinding;
using com::sun::star::form::binding::XBindableValue;
using com::sun::star::form::binding::XListEntrySink;
using com::sun::star::form::submission::XSubmissionSupplier;
using com::sun::star::beans::PropertyValue;
using com::sun::star::xsd::XDataType;
using com::sun::star::xforms::XDataTypeRepository;
using com::sun::star::xforms::XFormsSupplier;
using com::sun::star::util::Date;
using com::sun::star::util::DateTime;
using com::sun::star::util::Duration;

void exportXForms( SvXMLExport& rExport )
{
    Reference<XFormsSupplier> xSupplier( rExport.GetModel(), UNO_QUERY );
    if( xSupplier.is() )
    {
        Reference<XNameContainer> xForms = xSupplier->getXForms();
        if( xForms.is() )
        {
            Sequence<OUString> aNames = xForms->getElementNames();
            const OUString* pNames = aNames.getConstArray();
            sal_Int32 nNames = aNames.getLength();

            for( sal_Int32 n = 0; n < nNames; n++ )
            {
                Reference<XPropertySet> xModel( xForms->getByName( pNames[n] ),
                                                UNO_QUERY );
                exportXFormsModel( rExport, xModel );
            }
        }
    }
}


void exportXFormsInstance( SvXMLExport&, const Sequence<PropertyValue>& );
void exportXFormsBinding( SvXMLExport&, const Reference<XPropertySet>& );
void exportXFormsSubmission( SvXMLExport&, const Reference<XPropertySet>& );
void exportXFormsSchemas( SvXMLExport&, const Reference<com::sun::star::xforms::XModel>& );


typedef OUString (*convert_t)( const Any& );
typedef struct
{
    const sal_Char* pPropertyName;
    sal_uInt16 nPropertyNameLength;
    sal_uInt16 nNamespace;
    sal_uInt16 nToken;
    convert_t aConverter;
} ExportTable;
static void lcl_export( const Reference<XPropertySet>& rPropertySet,
                 SvXMLExport& rExport,
                 const ExportTable* pTable );

#define TABLE_ENTRY(NAME,NAMESPACE,TOKEN,CONVERTER) { NAME,sizeof(NAME)-1,XML_NAMESPACE_##NAMESPACE,xmloff::token::XML_##TOKEN, CONVERTER }
#define TABLE_END { NULL, 0, 0, 0, NULL }

// any conversion functions
OUString xforms_string( const Any& );
OUString xforms_bool( const Any& );
OUString xforms_whitespace( const Any& );
template<typename T, void (*FUNC)( OUStringBuffer&, T )> OUString xforms_convert( const Any& );
template<typename T, void (*FUNC)( OUStringBuffer&, const T& )> OUString xforms_convertRef( const Any& );

void xforms_formatDate( OUStringBuffer& aBuffer, const Date& aDate );
void xforms_formatTime( OUStringBuffer& aBuffer, const com::sun::star::util::Time& aTime );
void xforms_formatDateTime( OUStringBuffer& aBuffer, const DateTime& aDateTime );

convert_t xforms_int32    = &xforms_convert<sal_Int32,&::sax::Converter::convertNumber>;
convert_t xforms_double   = &xforms_convert<double,&::sax::Converter::convertDouble>;
convert_t xforms_dateTime = &xforms_convertRef<DateTime,&xforms_formatDateTime>;
convert_t xforms_date     = &xforms_convertRef<Date,&xforms_formatDate>;
convert_t xforms_time     = &xforms_convertRef<com::sun::star::util::Time,&xforms_formatTime>;

// other functions
static OUString lcl_getXSDType( SvXMLExport& rExport,
                         const Reference<XPropertySet>& xType );


//
// the model
//

static const ExportTable aXFormsModelTable[] =
{
    TABLE_ENTRY( "ID", NONE, ID, xforms_string ),
    TABLE_ENTRY( "SchemaRef", NONE, SCHEMA, xforms_string ),
    TABLE_END
};

void exportXFormsModel( SvXMLExport& rExport,
                        const Reference<XPropertySet>& xModelPropSet )
{
    // no model -> don't do anything!
    Reference<com::sun::star::xforms::XModel> xModel( xModelPropSet, UNO_QUERY );
    if( ! xModel.is() || ! xModelPropSet.is() )
        return;

    lcl_export( xModelPropSet, rExport, aXFormsModelTable );
    SvXMLElementExport aModelElement( rExport, XML_NAMESPACE_XFORMS, XML_MODEL,
                                      sal_True, sal_True );

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

//
// the instance
//

static const ExportTable aXFormsInstanceTable[] =
{
    TABLE_ENTRY( "InstanceURL", NONE, SRC, xforms_string ),
    TABLE_END
};

void exportXFormsInstance( SvXMLExport& rExport,
                           const Sequence<PropertyValue>& xInstance )
{
    OUString sId;
    OUString sURL;
    Reference<XDocument> xDoc;

    const PropertyValue* pInstance = xInstance.getConstArray();
    sal_Int32 nCount = xInstance.getLength();
    for( sal_Int32 i = 0; i < nCount; i++ )
    {
        OUString sName = pInstance[i].Name;
        const Any& rAny = pInstance[i].Value;
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
                              sal_True, sal_True );
    rExport.IgnorableWhitespace();
    if( xDoc.is() )
    {
        exportDom( rExport, xDoc );
    }
}


//
// the binding
//

static const ExportTable aXFormsBindingTable[] =
{
    TABLE_ENTRY( "BindingID",            NONE, ID,         xforms_string ),
    TABLE_ENTRY( "BindingExpression",    NONE, NODESET,    xforms_string ),
    TABLE_ENTRY( "ReadonlyExpression",   NONE, READONLY,   xforms_string ),
    TABLE_ENTRY( "RelevantExpression",   NONE, RELEVANT,   xforms_string ),
    TABLE_ENTRY( "RequiredExpression",   NONE, REQUIRED,   xforms_string ),
    TABLE_ENTRY( "ConstraintExpression", NONE, CONSTRAINT, xforms_string ),
    TABLE_ENTRY( "CalculateExpression",  NONE, CALCULATE,  xforms_string ),
    // type handled separatly, for type name <-> XSD type conversion
    // TABLE_ENTRY( "Type",                 NONE, TYPE,       xforms_string ),
    TABLE_END
};

void exportXFormsBinding( SvXMLExport& rExport,
                        const Reference<XPropertySet>& xBinding )
{
    // name check; generate binding ID if necessary
    {
        OUString sName;
        xBinding->getPropertyValue( OUSTRING("BindingID") ) >>= sName;
        if( sName.isEmpty() )
        {
            // if we don't have a name yet, generate one on the fly
            OUStringBuffer aBuffer;
            aBuffer.append( OUSTRING("bind_" ) );
            sal_Int64 nId = reinterpret_cast<sal_uInt64>( xBinding.get() );
            aBuffer.append( nId , 16 );
            sName = aBuffer.makeStringAndClear();
            xBinding->setPropertyValue( OUSTRING("BindingID"), makeAny(sName));
        }
    }

    lcl_export( xBinding, rExport, aXFormsBindingTable );

    // handle type attribute
    {
        OUString sTypeName;
        xBinding->getPropertyValue( OUSTRING("Type") ) >>= sTypeName;

        try
        {
            // now get type, and determine whether its a standard type. If
            // so, export the XSD name
            Reference<com::sun::star::xforms::XModel> xModel(
                xBinding->getPropertyValue( OUSTRING("Model") ),
                UNO_QUERY );
            Reference<XDataTypeRepository> xRepository(
                xModel.is() ? xModel->getDataTypeRepository() : Reference<XDataTypeRepository>() );
            if( xRepository.is() )
            {
                Reference<XPropertySet> xDataType(
                    xRepository->getDataType( sTypeName ),
                    UNO_QUERY );

                // if it's a basic data type, write out the XSD name
                // for the XSD type class
                bool bIsBasic = false;
                xDataType->getPropertyValue( OUSTRING("IsBasic") ) >>= bIsBasic;
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
        xBinding->getPropertyValue( OUSTRING("ModelNamespaces") ), UNO_QUERY);
    if( xNamespaces.is() )
    {
        // iterate over Prefixes for this binding
        Sequence<OUString> aPrefixes = xNamespaces->getElementNames();
        const OUString* pPrefixes = aPrefixes.getConstArray();
        sal_Int32 nPrefixes = aPrefixes.getLength();
        for( sal_Int32 i = 0; i < nPrefixes; i++ )
        {
            const OUString& rPrefix = pPrefixes[i];
            OUString sURI;
            xNamespaces->getByName( rPrefix ) >>= sURI;

            // check whether prefix/URI pair is in map; else write declaration
            // (we don't need to change the map, since this element has no
            // other content)
            sal_uInt16 nKey = rMap.GetKeyByPrefix( rPrefix );
            if( nKey == XML_NAMESPACE_UNKNOWN  ||
                rMap.GetNameByKey( nKey ) != sURI )
            {
                rExport.AddAttribute( OUSTRING("xmlns:") + rPrefix, sURI );
            }
        }
    }

    SvXMLElementExport aElement( rExport, XML_NAMESPACE_XFORMS, XML_BIND,
                                 sal_True, sal_True );
}


//
// the submission
//

static const ExportTable aXFormsSubmissionTable[] =
{
    TABLE_ENTRY( "ID",         NONE, ID,        xforms_string ),
    TABLE_ENTRY( "Bind",       NONE, BIND,      xforms_string ),
    TABLE_ENTRY( "Ref",        NONE, REF,       xforms_string ),
    TABLE_ENTRY( "Action",     NONE, ACTION,    xforms_string ),
    TABLE_ENTRY( "Method",     NONE, METHOD,    xforms_string ),
    TABLE_ENTRY( "Version",    NONE, VERSION,   xforms_string ),
    TABLE_ENTRY( "Indent",     NONE, INDENT,    xforms_bool ),
    TABLE_ENTRY( "MediaType",  NONE, MEDIATYPE, xforms_string ),
    TABLE_ENTRY( "Encoding",   NONE, ENCODING, xforms_string ),
    TABLE_ENTRY( "OmitXmlDeclaration",  NONE, OMIT_XML_DECLARATION, xforms_bool ),
    TABLE_ENTRY( "Standalone", NONE, STANDALONE, xforms_bool ),
    TABLE_ENTRY( "CDataSectionElement", NONE, CDATA_SECTION_ELEMENTS, xforms_string ),
    TABLE_ENTRY( "Replace",    NONE, REPLACE, xforms_string ),
    TABLE_ENTRY( "Separator",  NONE, SEPARATOR, xforms_string ),
    TABLE_ENTRY( "IncludeNamespacePrefixes", NONE, INCLUDENAMESPACEPREFIXES, xforms_string ),
    TABLE_END
};

void exportXFormsSubmission( SvXMLExport& rExport,
                             const Reference<XPropertySet>& xSubmission )
{
    lcl_export( xSubmission, rExport, aXFormsSubmissionTable );
    SvXMLElementExport aElement( rExport, XML_NAMESPACE_XFORMS, XML_SUBMISSION,
                                 sal_True, sal_True );
}



//
// export data types as XSD schema
//

static const ExportTable aDataTypeFacetTable[] =
{
    TABLE_ENTRY( "Length",               XSD, LENGTH,         xforms_int32 ),
    TABLE_ENTRY( "MinLength",            XSD, MINLENGTH,      xforms_int32 ),
    TABLE_ENTRY( "MaxLength",            XSD, MAXLENGTH,      xforms_int32 ),
    TABLE_ENTRY( "MinInclusiveInt",      XSD, MININCLUSIVE,   xforms_int32 ),
    TABLE_ENTRY( "MinExclusiveInt",      XSD, MINEXCLUSIVE,   xforms_int32 ),
    TABLE_ENTRY( "MaxInclusiveInt",      XSD, MAXINCLUSIVE,   xforms_int32 ),
    TABLE_ENTRY( "MaxExclusiveInt",      XSD, MAXEXCLUSIVE,   xforms_int32 ),
    TABLE_ENTRY( "MinInclusiveDouble",   XSD, MININCLUSIVE,   xforms_double ),
    TABLE_ENTRY( "MinExclusiveDouble",   XSD, MINEXCLUSIVE,   xforms_double ),
    TABLE_ENTRY( "MaxInclusiveDouble",   XSD, MAXINCLUSIVE,   xforms_double ),
    TABLE_ENTRY( "MaxExclusiveDouble",   XSD, MAXEXCLUSIVE,   xforms_double ),
    TABLE_ENTRY( "MinInclusiveDate",     XSD, MININCLUSIVE,   xforms_date ),
    TABLE_ENTRY( "MinExclusiveDate",     XSD, MINEXCLUSIVE,   xforms_date ),
    TABLE_ENTRY( "MaxInclusiveDate",     XSD, MAXINCLUSIVE,   xforms_date ),
    TABLE_ENTRY( "MaxExclusiveDate",     XSD, MAXEXCLUSIVE,   xforms_date ),
    TABLE_ENTRY( "MinInclusiveTime",     XSD, MININCLUSIVE,   xforms_time ),
    TABLE_ENTRY( "MinExclusiveTime",     XSD, MINEXCLUSIVE,   xforms_time ),
    TABLE_ENTRY( "MaxInclusiveTime",     XSD, MAXINCLUSIVE,   xforms_time ),
    TABLE_ENTRY( "MaxExclusiveTime",     XSD, MAXEXCLUSIVE,   xforms_time ),
    TABLE_ENTRY( "MinInclusiveDateTime", XSD, MININCLUSIVE,   xforms_dateTime ),
    TABLE_ENTRY( "MinExclusiveDateTime", XSD, MINEXCLUSIVE,   xforms_dateTime ),
    TABLE_ENTRY( "MaxInclusiveDateTime", XSD, MAXINCLUSIVE,   xforms_dateTime ),
    TABLE_ENTRY( "MaxExclusiveDateTime", XSD, MAXEXCLUSIVE,   xforms_dateTime ),
    TABLE_ENTRY( "Pattern",              XSD, PATTERN,        xforms_string ),
    // ??? XML_ENUMERATION,
    TABLE_ENTRY( "WhiteSpace",           XSD, WHITESPACE,     xforms_whitespace ),
    TABLE_ENTRY( "TotalDigits",          XSD, TOTALDIGITS,    xforms_int32 ),
    TABLE_ENTRY( "FractionDigits",       XSD, FRACTIONDIGITS, xforms_int32 ),
    TABLE_END
};

// export facets through table; use the same table as lcl_export does
static void lcl_exportDataTypeFacets( SvXMLExport& rExport,
                               const Reference<XPropertySet>& rPropertySet,
                               const ExportTable* pTable )
{
    Reference<XPropertySetInfo> xInfo = rPropertySet->getPropertySetInfo();
    for( const ExportTable* pCurrent = pTable;
         pCurrent->pPropertyName != NULL;
         pCurrent++ )
    {
        OUString sName( OUString::createFromAscii( pCurrent->pPropertyName ) );
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
                    sal_True, sal_True );
            }
        }
    }
}

static OUString lcl_getXSDType( SvXMLExport& rExport,
                         const Reference<XPropertySet>& xType )
{
    // we use string as default...
    XMLTokenEnum eToken = XML_STRING;

    sal_uInt16 nDataTypeClass = 0;
    xType->getPropertyValue( OUSTRING("TypeClass") ) >>= nDataTypeClass;
    switch( nDataTypeClass )
    {
    case com::sun::star::xsd::DataTypeClass::STRING:
        eToken = XML_STRING;
        break;
    case com::sun::star::xsd::DataTypeClass::anyURI:
        eToken = XML_ANYURI;
        break;
    case com::sun::star::xsd::DataTypeClass::DECIMAL:
        eToken = XML_DECIMAL;
        break;
    case com::sun::star::xsd::DataTypeClass::DOUBLE:
        eToken = XML_DOUBLE;
        break;
    case com::sun::star::xsd::DataTypeClass::FLOAT:
        eToken = XML_FLOAT;
        break;
    case com::sun::star::xsd::DataTypeClass::BOOLEAN:
        eToken = XML_BOOLEAN;
        break;
    case com::sun::star::xsd::DataTypeClass::DATETIME:
        eToken = XML_DATETIME_XSD;
        break;
    case com::sun::star::xsd::DataTypeClass::TIME:
        eToken = XML_TIME;
        break;
    case com::sun::star::xsd::DataTypeClass::DATE:
        eToken = XML_DATE;
        break;
    case com::sun::star::xsd::DataTypeClass::gYear:
        eToken = XML_YEAR;
        break;
    case com::sun::star::xsd::DataTypeClass::gDay:
        eToken = XML_DAY;
        break;
    case com::sun::star::xsd::DataTypeClass::gMonth:
        eToken = XML_MONTH;
        break;
    case com::sun::star::xsd::DataTypeClass::DURATION:
    case com::sun::star::xsd::DataTypeClass::gYearMonth:
    case com::sun::star::xsd::DataTypeClass::gMonthDay:
    case com::sun::star::xsd::DataTypeClass::hexBinary:
    case com::sun::star::xsd::DataTypeClass::base64Binary:
    case com::sun::star::xsd::DataTypeClass::QName:
    case com::sun::star::xsd::DataTypeClass::NOTATION:
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
    xType->getPropertyValue( OUSTRING("IsBasic") ) >>= bIsBasic;
    if( bIsBasic )
        return;

    // no basic type -> export

    // <xsd:simpleType name="...">
    OUString sName;
    xType->getPropertyValue( OUSTRING("Name") ) >>= sName;
    rExport.AddAttribute( XML_NAMESPACE_NONE, XML_NAME, sName );
    SvXMLElementExport aSimpleType( rExport,
                                    XML_NAMESPACE_XSD, XML_SIMPLETYPE,
                                    sal_True, sal_True );

    // <xsd:restriction base="xsd:...">
    rExport.AddAttribute( XML_NAMESPACE_NONE, XML_BASE,
                          lcl_getXSDType( rExport, xType ) );
    SvXMLElementExport aRestriction( rExport,
                                     XML_NAMESPACE_XSD,
                                     XML_RESTRICTION,
                                     sal_True, sal_True );

    // export facets
    lcl_exportDataTypeFacets( rExport,
                              Reference<XPropertySet>( xType, UNO_QUERY ),
                              aDataTypeFacetTable );
}

void exportXFormsSchemas( SvXMLExport& rExport,
                          const Reference<com::sun::star::xforms::XModel>& xModel )
{
    // TODO: for now, we'll fake this...
    {
        SvXMLElementExport aSchemaElem( rExport, XML_NAMESPACE_XSD, XML_SCHEMA,
                                        sal_True, sal_True );

        // now get data type repositry, and export
        Reference<XEnumerationAccess> xTypes( xModel->getDataTypeRepository(),
                                              UNO_QUERY );
        if( xTypes.is() )
        {
            Reference<XEnumeration> xEnum = xTypes->createEnumeration();
            DBG_ASSERT( xEnum.is(), "no enum?" );
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
            xPropSet->getPropertyValue( OUSTRING("ForeignSchema") ),
            UNO_QUERY );

        if( xDocument.is() )
            exportDom( rExport, xDocument );
    }
}



//
// helper functions
//

static void lcl_export( const Reference<XPropertySet>& rPropertySet,
                 SvXMLExport& rExport,
                 const ExportTable* pTable )
{
    for( const ExportTable* pCurrent = pTable;
         pCurrent->pPropertyName != NULL;
         pCurrent++ )
    {
        Any aAny = rPropertySet->getPropertyValue(
                       OUString::createFromAscii( pCurrent->pPropertyName ) );
        OUString sValue = (*pCurrent->aConverter)( aAny );

        if( !sValue.isEmpty() )
            rExport.AddAttribute(
                pCurrent->nNamespace,
                static_cast<XMLTokenEnum>( pCurrent->nToken ),
                sValue );
    }
}



//
// any conversion functions
//

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

void xforms_formatDate( OUStringBuffer& aBuffer, const Date& rDate )
{
    aBuffer.append( static_cast<sal_Int32>( rDate.Year ) );
    aBuffer.append( sal_Unicode('-') );
    aBuffer.append( static_cast<sal_Int32>( rDate.Month ) );
    aBuffer.append( sal_Unicode('-') );
    aBuffer.append( static_cast<sal_Int32>( rDate.Day ) );
}

void xforms_formatTime( OUStringBuffer& aBuffer, const com::sun::star::util::Time& rTime )
{
    Duration aDuration;
    aDuration.Hours = rTime.Hours;
    aDuration.Minutes = rTime.Minutes;
    aDuration.Seconds = rTime.Seconds;
    aDuration.MilliSeconds = rTime.HundredthSeconds * 10;
    ::sax::Converter::convertDuration( aBuffer, aDuration );
}

void xforms_formatDateTime( OUStringBuffer& aBuffer, const DateTime& aDateTime )
{
    ::sax::Converter::convertDateTime( aBuffer, aDateTime );
}

OUString xforms_whitespace( const Any& rAny )
{
    OUString sResult;
    sal_uInt16 n = sal_uInt16();
    if( rAny >>= n )
    {
        switch( n )
        {
        case com::sun::star::xsd::WhiteSpaceTreatment::Preserve:
            sResult = GetXMLToken( XML_PRESERVE );
            break;
        case com::sun::star::xsd::WhiteSpaceTreatment::Replace:
            sResult = GetXMLToken( XML_REPLACE );
            break;
        case com::sun::star::xsd::WhiteSpaceTreatment::Collapse:
            sResult = GetXMLToken( XML_COLLAPSE );
            break;
        }
    }
    return sResult;
}


/// return name of Binding
static OUString lcl_getXFormsBindName( const Reference<XPropertySet>& xBinding )
{
    OUString sProp( OUSTRING( "BindingID" ) );

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
        OUString sProp( OUSTRING("ID") );
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

        Sequence< ::rtl::OUString > aModelNames( _rXForms->getElementNames() );

        ::comphelper::ComponentContext aContext( ::comphelper::getProcessServiceFactory() );
        Reference< XNameContainer > xModelSettings(
            aContext.createComponent( "com.sun.star.document.NamedPropertyValues" ),
            UNO_QUERY_THROW );

        for (   const ::rtl::OUString* pModelName = aModelNames.getConstArray();
                pModelName != aModelNames.getConstArray() + aModelNames.getLength();
                ++pModelName
            )
        {
            Reference< XPropertySet > xModelProps( _rXForms->getByName( *pModelName ), UNO_QUERY_THROW );

            Sequence< PropertyValue > aModelSettings( 1 );
            aModelSettings[0].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ExternalData" ) );
            aModelSettings[0].Value = xModelProps->getPropertyValue( aModelSettings[0].Name );

            xModelSettings->insertByName( *pModelName, makeAny( aModelSettings ) );
        }

        if ( xModelSettings->hasElements() )
        {
            _out_rSettings.realloc( 1 );
            _out_rSettings[0].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "XFormModels" ) );
            _out_rSettings[0].Value <<= xModelSettings;
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
