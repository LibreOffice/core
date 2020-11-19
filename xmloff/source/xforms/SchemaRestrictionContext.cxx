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
#include <xmloff/namespacemap.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltkmap.hxx>
#include <xmloff/xmlimp.hxx>

#include <sax/tools/converter.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/util/Time.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/util/Duration.hpp>
#include <com/sun/star/xforms/XDataTypeRepository.hpp>
#include <com/sun/star/xsd/DataTypeClass.hpp>
#include <com/sun/star/xsd/WhiteSpaceTreatment.hpp>

#include <sal/log.hxx>
#include <tools/diagnose_ex.h>


using com::sun::star::uno::Reference;
using com::sun::star::uno::Exception;
using com::sun::star::uno::Any;
using com::sun::star::uno::makeAny;
using namespace com::sun::star;
using com::sun::star::util::Duration;
using com::sun::star::xml::sax::XFastAttributeList;
using com::sun::star::xforms::XDataTypeRepository;
using namespace xmloff::token;


SchemaRestrictionContext::SchemaRestrictionContext(
    SvXMLImport& rImport,
    Reference<css::xforms::XDataTypeRepository> const & rRepository,
    const OUString& sTypeName ) :
        TokenContext( rImport ),
        mxRepository( rRepository ),
        msTypeName( sTypeName ),
        msBaseName()
{
    SAL_WARN_IF( !mxRepository.is(), "xmloff", "need repository" );
}

void SchemaRestrictionContext::CreateDataType()
{
    // only do something if we don't have a data type already
    if( mxDataType.is() )
        return;

    SAL_WARN_IF( msBaseName.isEmpty(), "xmloff", "no base name?" );
    SAL_WARN_IF( !mxRepository.is(), "xmloff", "no repository?" );

    try
    {
        mxDataType =
                mxRepository->cloneDataType(
                    xforms_getBasicTypeName( mxRepository,
                                          GetImport().GetNamespaceMap(),
                                          msBaseName ),
                    msTypeName );
    }
    catch( const Exception& )
    {
        TOOLS_WARN_EXCEPTION("xmloff", "exception during type creation");
    }
    SAL_WARN_IF( !mxDataType.is(), "xmloff", "can't create type" );
}

void SchemaRestrictionContext::HandleAttribute(
    sal_Int32 nAttributeToken,
    const OUString& rValue )
{
    if( nAttributeToken == XML_ELEMENT(NONE, XML_BASE) )
    {
        msBaseName = rValue;
    }
}

typedef Any (*convert_t)( const OUString& );

static Any xforms_string( const OUString& rValue )
{
    return makeAny( rValue );
}

static Any xforms_int32( const OUString& rValue )
{
    sal_Int32 nValue;
    bool bSuccess = ::sax::Converter::convertNumber( nValue, rValue );
    return bSuccess ? makeAny( nValue ) : Any();
}

static Any xforms_int16( const OUString& rValue )
{
    sal_Int32 nValue;
    bool bSuccess = ::sax::Converter::convertNumber( nValue, rValue );
    return bSuccess ? makeAny( static_cast<sal_Int16>( nValue ) ) : Any();
}

static Any xforms_whitespace( const OUString& rValue )
{
    Any aValue;
    if( IsXMLToken( rValue, XML_PRESERVE ) )
        aValue <<= css::xsd::WhiteSpaceTreatment::Preserve;
    else if( IsXMLToken( rValue, XML_REPLACE ) )
        aValue <<= css::xsd::WhiteSpaceTreatment::Replace;
    else if( IsXMLToken( rValue, XML_COLLAPSE ) )
        aValue <<= css::xsd::WhiteSpaceTreatment::Collapse;
    return aValue;
}

static Any xforms_double( const OUString& rValue )
{
    double fValue;
    bool bSuccess = ::sax::Converter::convertDouble( fValue, rValue );
    return bSuccess ? makeAny( fValue ) : Any();
}

static Any xforms_date( const OUString& rValue )
{
    Any aAny;

    // parse ISO date
    sal_Int32 nPos1 = rValue.indexOf( '-' );
    sal_Int32 nPos2 = rValue.indexOf( '-', nPos1 + 1 );
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

static Any xforms_dateTime( const OUString& rValue )
{
    util::DateTime aDateTime;
    bool const bSuccess = ::sax::Converter::parseDateTime(aDateTime, rValue);
    return bSuccess ? makeAny( aDateTime ) : Any();
}

static Any xforms_time( const OUString& rValue )
{
    Any aAny;
    Duration aDuration;
    if (::sax::Converter::convertDuration( aDuration, rValue ))
    {
        css::util::Time aTime;
        aTime.Hours = aDuration.Hours;
        aTime.Minutes = aDuration.Minutes;
        aTime.Seconds = aDuration.Seconds;
        aTime.NanoSeconds = aDuration.NanoSeconds;
        aAny <<= aTime;
    }
    return aAny;
}


SvXMLImportContext* SchemaRestrictionContext::HandleChild(
    sal_Int32 nElementToken,
    const Reference<XFastAttributeList>& xAttrList )
{
    // find value
    OUString sValue;
    for( auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList) )
    {
        if( ( aIter.getToken() & TOKEN_MASK) == XML_VALUE )
        {
            sValue = aIter.toString();
            break;
        }
    }

    // determine property name + suitable converter
    OUString sPropertyName;
    convert_t pConvert = nullptr;
    switch( nElementToken & TOKEN_MASK )
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
            switch( nElementToken )
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
            case css::xsd::DataTypeClass::DECIMAL:
            case css::xsd::DataTypeClass::DOUBLE:
            case css::xsd::DataTypeClass::FLOAT:
                sPropertyName += "Double";
                pConvert = &xforms_double;
                break;
            case css::xsd::DataTypeClass::DATETIME:
                sPropertyName += "DateTime";
                pConvert = &xforms_dateTime;
                break;
            case css::xsd::DataTypeClass::DATE:
                sPropertyName += "Date";
                pConvert = &xforms_date;
                break;
            case css::xsd::DataTypeClass::TIME:
                sPropertyName += "Time";
                pConvert = &xforms_time;
                break;
            case css::xsd::DataTypeClass::gYear:
            case css::xsd::DataTypeClass::gDay:
            case css::xsd::DataTypeClass::gMonth:
                sPropertyName += "Int";
                pConvert = &xforms_int16;
                break;

            case css::xsd::DataTypeClass::STRING:
            case css::xsd::DataTypeClass::anyURI:
            case css::xsd::DataTypeClass::BOOLEAN:
                // invalid: These shouldn't have min/max-inclusive
                break;

                /* data types not yet supported:
                   case css::xsd::DataTypeClass::DURATION:
                   case css::xsd::DataTypeClass::gYearMonth:
                   case css::xsd::DataTypeClass::gMonthDay:
                   case css::xsd::DataTypeClass::hexBinary:
                   case css::xsd::DataTypeClass::base64Binary:
                   case css::xsd::DataTypeClass::QName:
                   case css::xsd::DataTypeClass::NOTATION:
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
        && pConvert != nullptr
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

    return new SvXMLImportContext( GetImport() );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
