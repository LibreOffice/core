/*************************************************************************
 *
 *  $RCSfile: SchemaRestrictionContext.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-11-16 10:12:16 $
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

#include "SchemaRestrictionContext.hxx"
#include "xformsapi.hxx"

#include <xmltoken.hxx>
#include <nmspmap.hxx>
#include <xmlnmspe.hxx>
#include <xmltkmap.hxx>
#include <xmluconv.hxx>
#include <xmlimp.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/xsd/WhiteSpaceTreatment.hpp>
#include <com/sun/star/xforms/XDataTypeRepository.hpp>
#include <com/sun/star/xsd/DataTypeClass.hpp>

#include <tools/debug.hxx>


using rtl::OUString;
using com::sun::star::uno::Reference;
using com::sun::star::uno::Exception;
using com::sun::star::uno::Any;
using com::sun::star::uno::UNO_QUERY;
using com::sun::star::xml::sax::XAttributeList;
using com::sun::star::beans::XPropertySet;
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
    USHORT nPrefix,
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

    DBG_ASSERT( msBaseName.getLength() > 0, "no base name?" );
    DBG_ASSERT( mxRepository.is(), "no repository?" );

    try
    {
        mxDataType =
            Reference<XPropertySet>(
                mxRepository->cloneDataType(
                    lcl_getTypeName( mxRepository,
                                     GetImport().GetNamespaceMap(),
                                     msBaseName ),
                    msTypeName ),
                UNO_QUERY );
    }
    catch( const Exception& )
    {
        DBG_ERROR( "exception during type creation" );
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

    // determine property name
    OUString sPropertyName;
    switch( nToken )
    {
    case XML_LENGTH:
        sPropertyName = OUString(RTL_CONSTASCII_USTRINGPARAM("Length"));
        break;
    case XML_MINLENGTH:
        sPropertyName = OUString(RTL_CONSTASCII_USTRINGPARAM("MinLength"));
        break;
    case XML_MAXLENGTH:
        sPropertyName = OUString(RTL_CONSTASCII_USTRINGPARAM("MaxLength"));
        break;
    case XML_TOTALDIGITS:
        sPropertyName = OUString(RTL_CONSTASCII_USTRINGPARAM("TotalDigits"));
        break;
    case XML_FRACTIONDIGITS:
        sPropertyName =OUString(RTL_CONSTASCII_USTRINGPARAM("FractionDigits"));
        break;
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
    case XML_PATTERN:
        sPropertyName = OUString(RTL_CONSTASCII_USTRINGPARAM("Pattern"));
        break;
    case XML_WHITESPACE:
        sPropertyName = OUString(RTL_CONSTASCII_USTRINGPARAM("WhiteSpace"));
        break;
    default:
        DBG_ERROR( "unknown facet" );
    }

    // determine & convert property value
    Any aValue;
    switch( nToken )
    {
    case XML_LENGTH:
    case XML_MINLENGTH:
    case XML_MAXLENGTH:
    case XML_TOTALDIGITS:
    case XML_FRACTIONDIGITS:
        {
            // convert int32
            sal_Int32 nValue;
            if( SvXMLUnitConverter::convertNumber( nValue, sValue ) )
                aValue <<= nValue;
        }
        break;

    case XML_MININCLUSIVE:
    case XML_MINEXCLUSIVE:
    case XML_MAXINCLUSIVE:
    case XML_MAXEXCLUSIVE:
        {
            // convert double
            double fValue;
            if( SvXMLUnitConverter::convertDouble( fValue, sValue ) )
                aValue <<= fValue;
        }
        break;

    case XML_PATTERN:
        aValue <<= sValue;
        break;

    case XML_WHITESPACE:
        if( IsXMLToken( sValue, XML_PRESERVE ) )
            aValue <<= com::sun::star::xsd::WhiteSpaceTreatment::Preserve;
        else if( IsXMLToken( sValue, XML_REPLACE ) )
            aValue <<= com::sun::star::xsd::WhiteSpaceTreatment::Replace;
        else if( IsXMLToken( sValue, XML_COLLAPSE ) )
            aValue <<= com::sun::star::xsd::WhiteSpaceTreatment::Collapse;
        break;

    default:
        DBG_ERROR( "unknown facet" );
    }

    // finally, set the property
    CreateDataType();
    if( mxDataType.is()
        && sPropertyName.getLength() > 0
        && aValue.hasValue()
        && mxDataType->getPropertySetInfo()->hasPropertyByName(sPropertyName) )
    {
        mxDataType->setPropertyValue( sPropertyName, aValue );
    }

    return new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
}
