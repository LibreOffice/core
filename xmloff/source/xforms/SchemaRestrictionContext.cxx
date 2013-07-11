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


#include "SchemaRestrictionContext.hxx"
#include "xformsapi.hxx"

#include <xmloff/xmltoken.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmltkmap.hxx>
#include <xmloff/xmlimp.hxx>

#include <sax/tools/converter.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/uno/Type.hxx>
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/util/Time.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/util/Duration.hpp>
#include <com/sun/star/xforms/XDataTypeRepository.hpp>
#include <com/sun/star/xsd/DataTypeClass.hpp>
#include <com/sun/star/xsd/WhiteSpaceTreatment.hpp>

#include <tools/debug.hxx>


using com::sun::star::uno::Reference;
using com::sun::star::uno::Exception;
using com::sun::star::uno::Any;
using com::sun::star::uno::makeAny;
using com::sun::star::uno::UNO_QUERY;
using namespace com::sun::star;
using com::sun::star::util::Duration;
using com::sun::star::xml::sax::XAttributeList;
using com::sun::star::beans::XPropertySet;
using com::sun::star::beans::XPropertySetInfo;
using com::sun::star::xforms::XDataTypeRepository;
using namespace xmloff::token;




static SvXMLTokenMapEntry aAttributes[] =
{
    TOKEN_MAP_ENTRY( NONE, BASE ),
    XML_TOKEN_MAP_END
};

static SvXMLTokenMapEntry aChildren[] =
{
    TOKEN_MAP_ENTRY( XSD, LENGTH         ),
    TOKEN_MAP_ENTRY( XSD, MINLENGTH      ),
    TOKEN_MAP_ENTRY( XSD, MAXLENGTH      ),
    TOKEN_MAP_ENTRY( XSD, MININCLUSIVE   ),
    TOKEN_MAP_ENTRY( XSD, MINEXCLUSIVE   ),
    TOKEN_MAP_ENTRY( XSD, MAXINCLUSIVE   ),
    TOKEN_MAP_ENTRY( XSD, MAXEXCLUSIVE   ),
    TOKEN_MAP_ENTRY( XSD, PATTERN        ),
    // ??? XML_ENUMERATION
    TOKEN_MAP_ENTRY( XSD, WHITESPACE     ),
    TOKEN_MAP_ENTRY( XSD, TOTALDIGITS    ),
    TOKEN_MAP_ENTRY( XSD, FRACTIONDIGITS ),
    XML_TOKEN_MAP_END
};


SchemaRestrictionContext::SchemaRestrictionContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    Reference<com::sun::star::xforms::XDataTypeRepository>& rRepository,
    const OUString& sTypeName ) :
        TokenContext( rImport, nPrefix, rLocalName, aAttributes, aChildren ),
        mxRepository( rRepository ),
        msTypeName( sTypeName ),
        msBaseName()
{
    DBG_ASSERT( mxRepository.is(), "need repository" );
}

SchemaRestrictionContext::~SchemaRestrictionContext()
{
}

void SchemaRestrictionContext::CreateDataType()
{
    // only do something if we don't have a data type already
    if( mxDataType.is() )
        return;

    DBG_ASSERT( !msBaseName.isEmpty(), "no base name?" );
    DBG_ASSERT( mxRepository.is(), "no repository?" );

    try
    {
        mxDataType =
            Reference<XPropertySet>(
                mxRepository->cloneDataType(
                    xforms_getBasicTypeName( mxRepository,
                                          GetImport().GetNamespaceMap(),
                                          msBaseName ),
                    msTypeName ),
                UNO_QUERY );
    }
    catch( const Exception& )
    {
        OSL_FAIL( "exception during type creation" );
    }
    DBG_ASSERT( mxDataType.is(), "can't create type" );
}

void SchemaRestrictionContext::HandleAttribute(
    sal_uInt16 nToken,
    const OUString& rValue )
{
    if( nToken == XML_BASE )
    {
        msBaseName = rValue;
    }
}

typedef Any (*convert_t)( const OUString& );

Any xforms_string( const OUString& rValue )
{
    return makeAny( rValue );
}

Any xforms_int32( const OUString& rValue )
{
    sal_Int32 nValue;
    bool bSuccess = ::sax::Converter::convertNumber( nValue, rValue );
    return bSuccess ? makeAny( nValue ) : Any();
}

Any xforms_int16( const OUString& rValue )
{
    sal_Int32 nValue;
    bool bSuccess = ::sax::Converter::convertNumber( nValue, rValue );
    return bSuccess ? makeAny( static_cast<sal_Int16>( nValue ) ) : Any();
}

Any xforms_whitespace( const OUString& rValue )
{
    Any aValue;
    if( IsXMLToken( rValue, XML_PRESERVE ) )
        aValue <<= com::sun::star::xsd::WhiteSpaceTreatment::Preserve;
    else if( IsXMLToken( rValue, XML_REPLACE ) )
        aValue <<= com::sun::star::xsd::WhiteSpaceTreatment::Replace;
    else if( IsXMLToken( rValue, XML_COLLAPSE ) )
        aValue <<= com::sun::star::xsd::WhiteSpaceTreatment::Collapse;
    return aValue;
}

Any xforms_double( const OUString& rValue )
{
    double fValue;
    bool bSuccess = ::sax::Converter::convertDouble( fValue, rValue );
    return bSuccess ? makeAny( fValue ) : Any();
}

Any xforms_date( const OUString& rValue )
{
    Any aAny;

    // parse ISO date
    sal_Int32 nPos1 = rValue.indexOf( sal_Unicode('-') );
    sal_Int32 nPos2 = rValue.indexOf( sal_Unicode('-'), nPos1 + 1 );
    if( nPos1 > 0  &&  nPos2 > 0 )
    {
        util::Date aDate;
        aDate.Year = static_cast<sal_uInt16>(
                     rValue.copy( 0, nPos1 ).toInt32() );
        aDate.Month = static_cast<sal_uInt16>(
                      rValue.copy( nPos1 + 1, nPos2 - nPos1 - 1 ).toInt32() );
        aDate.Day   = static_cast<sal_uInt16>(
                      rValue.copy( nPos2 + 1 ).toInt32() );
        aAny <<= aDate;
    }
    return aAny;
}

Any xforms_dateTime( const OUString& rValue )
{
    util::DateTime aDateTime;
    bool const bSuccess = ::sax::Converter::parseDateTime(aDateTime, 0, rValue);
    return bSuccess ? makeAny( aDateTime ) : Any();
}

Any xforms_time( const OUString& rValue )
{
    Any aAny;
    Duration aDuration;
    if (::sax::Converter::convertDuration( aDuration, rValue ))
    {
        com::sun::star::util::Time aTime;
        aTime.Hours = aDuration.Hours;
        aTime.Minutes = aDuration.Minutes;
        aTime.Seconds = aDuration.Seconds;
        aTime.NanoSeconds = aDuration.NanoSeconds;
        aAny <<= aTime;
    }
    return aAny;
}


SvXMLImportContext* SchemaRestrictionContext::HandleChild(
    sal_uInt16 nToken,
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference<XAttributeList>& xAttrList )
{
    // find value
    OUString sValue;
    sal_Int16 nLength = xAttrList->getLength();
    for( sal_Int16 n = 0; n < nLength; n++ )
    {
        if( IsXMLToken( xAttrList->getNameByIndex( n ), XML_VALUE ) )
            sValue = xAttrList->getValueByIndex( n );
    }

    // determine property name + suitable converter
    OUString sPropertyName;
    convert_t pConvert = NULL;
    switch( nToken )
    {
    case XML_LENGTH:
        sPropertyName = "Length";
        pConvert = &xforms_int32;
        break;
    case XML_MINLENGTH:
        sPropertyName = "MinLength";
        pConvert = &xforms_int32;
        break;
    case XML_MAXLENGTH:
        sPropertyName = "MaxLength";
        pConvert = &xforms_int32;
        break;
    case XML_TOTALDIGITS:
        sPropertyName = "TotalDigits";
        pConvert = &xforms_int32;
        break;
    case XML_FRACTIONDIGITS:
        sPropertyName = "FractionDigits";
        pConvert = &xforms_int32;
        break;
    case XML_PATTERN:
        sPropertyName = "Pattern";
        pConvert = &xforms_string;
        break;
    case XML_WHITESPACE:
        sPropertyName = "WhiteSpace";
        pConvert = &xforms_whitespace;
        break;
    case XML_MININCLUSIVE:
    case XML_MINEXCLUSIVE:
    case XML_MAXINCLUSIVE:
    case XML_MAXEXCLUSIVE:
        {
            // these attributes are mapped to different properties.
            // To determine the property name, we use an attribute
            // dependent prefix and a type dependent suffix. The
            // converter is only type dependent.

            // first, attribute-dependent prefix
            switch( nToken )
            {
            case XML_MININCLUSIVE:
                sPropertyName = "MinInclusive";
                break;
            case XML_MINEXCLUSIVE:
                sPropertyName = "MinExclusive";
                break;
            case XML_MAXINCLUSIVE:
                sPropertyName = "MaxInclusive";
                break;
            case XML_MAXEXCLUSIVE:
                sPropertyName = "MaxExclusive";
                break;
            }

            // second, type-dependent suffix + converter
            switch( xforms_getTypeClass( mxRepository,
                                      GetImport().GetNamespaceMap(),
                                      msBaseName ) )
            {
            case com::sun::star::xsd::DataTypeClass::DECIMAL:
            case com::sun::star::xsd::DataTypeClass::DOUBLE:
            case com::sun::star::xsd::DataTypeClass::FLOAT:
                sPropertyName += "Double";
                pConvert = &xforms_double;
                break;
            case com::sun::star::xsd::DataTypeClass::DATETIME:
                sPropertyName += "DateTime";
                pConvert = &xforms_dateTime;
                break;
            case com::sun::star::xsd::DataTypeClass::DATE:
                sPropertyName += "Date";
                pConvert = &xforms_date;
                break;
            case com::sun::star::xsd::DataTypeClass::TIME:
                sPropertyName += "Time";
                pConvert = &xforms_time;
                break;
            case com::sun::star::xsd::DataTypeClass::gYear:
            case com::sun::star::xsd::DataTypeClass::gDay:
            case com::sun::star::xsd::DataTypeClass::gMonth:
                sPropertyName += "Int";
                pConvert = &xforms_int16;
                break;

            case com::sun::star::xsd::DataTypeClass::STRING:
            case com::sun::star::xsd::DataTypeClass::anyURI:
            case com::sun::star::xsd::DataTypeClass::BOOLEAN:
                // invalid: These shouldn't have min/max-inclusive
                break;

                /* data types not yet supported:
                   case com::sun::star::xsd::DataTypeClass::DURATION:
                   case com::sun::star::xsd::DataTypeClass::gYearMonth:
                   case com::sun::star::xsd::DataTypeClass::gMonthDay:
                   case com::sun::star::xsd::DataTypeClass::hexBinary:
                   case com::sun::star::xsd::DataTypeClass::base64Binary:
                   case com::sun::star::xsd::DataTypeClass::QName:
                   case com::sun::star::xsd::DataTypeClass::NOTATION:
                */
            }
        }
        break;

    default:
        OSL_FAIL( "unknown facet" );
    }

    // finally, set the property
    CreateDataType();
    if( mxDataType.is()
        && !sPropertyName.isEmpty()
        && pConvert != NULL
        && mxDataType->getPropertySetInfo()->hasPropertyByName(sPropertyName) )
    {
        try
        {
            mxDataType->setPropertyValue( sPropertyName, pConvert( sValue ) );
        }
        catch( const Exception& )
        {
            ; // can't set property? Then ignore.
        }
    }

    return new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
