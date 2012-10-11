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


using rtl::OUString;
using com::sun::star::uno::Reference;
using com::sun::star::uno::Exception;
using com::sun::star::uno::Any;
using com::sun::star::uno::makeAny;
using com::sun::star::uno::UNO_QUERY;
using com::sun::star::util::Date;
using com::sun::star::util::DateTime;
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
        Date aDate;
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
    DateTime aDateTime;
    bool const bSuccess = ::sax::Converter::convertDateTime(aDateTime, rValue);
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
        aTime.HundredthSeconds = aDuration.MilliSeconds / 10;
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
        sPropertyName = OUString(RTL_CONSTASCII_USTRINGPARAM("Length"));
        pConvert = &xforms_int32;
        break;
    case XML_MINLENGTH:
        sPropertyName = OUString(RTL_CONSTASCII_USTRINGPARAM("MinLength"));
        pConvert = &xforms_int32;
        break;
    case XML_MAXLENGTH:
        sPropertyName = OUString(RTL_CONSTASCII_USTRINGPARAM("MaxLength"));
        pConvert = &xforms_int32;
        break;
    case XML_TOTALDIGITS:
        sPropertyName = OUString(RTL_CONSTASCII_USTRINGPARAM("TotalDigits"));
        pConvert = &xforms_int32;
        break;
    case XML_FRACTIONDIGITS:
        sPropertyName =OUString(RTL_CONSTASCII_USTRINGPARAM("FractionDigits"));
        pConvert = &xforms_int32;
        break;
    case XML_PATTERN:
        sPropertyName = OUString(RTL_CONSTASCII_USTRINGPARAM("Pattern"));
        pConvert = &xforms_string;
        break;
    case XML_WHITESPACE:
        sPropertyName = OUString(RTL_CONSTASCII_USTRINGPARAM("WhiteSpace"));
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
                sPropertyName = OUString(RTL_CONSTASCII_USTRINGPARAM("MinInclusive"));
                break;
            case XML_MINEXCLUSIVE:
                sPropertyName = OUString(RTL_CONSTASCII_USTRINGPARAM("MinExclusive"));
                break;
            case XML_MAXINCLUSIVE:
                sPropertyName = OUString(RTL_CONSTASCII_USTRINGPARAM("MaxInclusive"));
                break;
            case XML_MAXEXCLUSIVE:
                sPropertyName = OUString(RTL_CONSTASCII_USTRINGPARAM("MaxExclusive"));
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
                sPropertyName += OUString(RTL_CONSTASCII_USTRINGPARAM("Double"));
                pConvert = &xforms_double;
                break;
            case com::sun::star::xsd::DataTypeClass::DATETIME:
                sPropertyName += OUString(RTL_CONSTASCII_USTRINGPARAM("DateTime"));
                pConvert = &xforms_dateTime;
                break;
            case com::sun::star::xsd::DataTypeClass::DATE:
                sPropertyName += OUString(RTL_CONSTASCII_USTRINGPARAM("Date"));
                pConvert = &xforms_date;
                break;
            case com::sun::star::xsd::DataTypeClass::TIME:
                sPropertyName += OUString(RTL_CONSTASCII_USTRINGPARAM("Time"));
                pConvert = &xforms_time;
                break;
            case com::sun::star::xsd::DataTypeClass::gYear:
            case com::sun::star::xsd::DataTypeClass::gDay:
            case com::sun::star::xsd::DataTypeClass::gMonth:
                sPropertyName += OUString(RTL_CONSTASCII_USTRINGPARAM("Int"));
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
