/*************************************************************************
 *
 *  $RCSfile: stringrepresentation.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-11-16 12:12:56 $
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

#ifndef EXTENSIONS_SOURCE_PROPCTRLR_STRINGREPRESENTATION_HXX
#include "stringrepresentation.hxx"
#endif

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_UTIL_DATETIME_HPP_
#include <com/sun/star/util/DateTime.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_DATE_HPP_
#include <com/sun/star/util/Date.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_TIME_HPP_
#include <com/sun/star/util/Time.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
/** === end UNO includes === **/

#ifndef _EXTENSIONS_PROPCTRLR_MODULEPRC_HXX_
#include "modulepcr.hxx"
#endif
#ifndef _EXTENSIONS_FORMCTRLR_PROPRESID_HRC_
#include "formresid.hrc"
#endif
#ifndef _EXTENSIONS_PROPCTRLR_PROPERTYINFO_HXX_
#include "propertyinfo.hxx"
#endif
#ifndef _EXTENSIONS_PROPCTRLR_FORMMETADATA_HXX_
#include "formmetadata.hxx"
#endif

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _DATE_HXX
#include <tools/date.hxx>
#endif
#ifndef _TOOLS_TIME_HXX
#include <tools/time.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _COMPHELPER_EXTRACT_HXX_
#include <cppuhelper/extract.hxx>
#endif

#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif

#ifndef _UNOTOOLS_DATETIME_HXX_
#include <unotools/datetime.hxx>
#endif

#include <functional>

//........................................................................
namespace pcr
{
//........................................................................

    using namespace ::com::sun::star;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::script;

    //====================================================================
    //= StringRepresentation
    //====================================================================
    //------------------------------------------------------------------------
    StringRepresentation::StringRepresentation( const Reference< XTypeConverter >& _rxConverter )
        :m_xTypeConverter( _rxConverter )
    {
    }

    //------------------------------------------------------------------------
    ::rtl::OUString StringRepresentation::convertSimpleToString( const Any& _rValue )
    {
        ::rtl::OUString sReturn;
        if ( m_xTypeConverter.is() && _rValue.hasValue() )
        {
            try
            {
                Any aConvertedToString;
                aConvertedToString = m_xTypeConverter->convertToSimpleType( _rValue, TypeClass_STRING );
                aConvertedToString >>= sReturn;
            }
            catch( CannotConvertException& ) { }
            catch( IllegalArgumentException& ) { }
        }
        return sReturn;
    }

    //--------------------------------------------------------------------
    namespace
    {
        struct ConvertIntegerToString : public ::std::unary_function< sal_Int32, String >
        {
            String operator()( sal_Int32 _rIntValue ) const
            {
                return String::CreateFromInt32( _rIntValue );
            }
        };

        struct StringIdentity : public ::std::unary_function< ::rtl::OUString, String >
        {
            String operator()( ::rtl::OUString _rValue ) const
            {
                return _rValue;
            }
        };

        template < class ElementType, class Translator >
        ::rtl::OUString composeSequenceElements( const Sequence< ElementType >& _rElements, const Translator& _rTranslator )
        {
            String sCompose;

            // loop through the elements and concatenate the string representations of the integers
            // (separated by a line break)
            const ElementType* pElements = _rElements.getConstArray();
            const ElementType* pElementsEnd = pElements + _rElements.getLength();
            for ( ; pElements != pElementsEnd; ++pElements )
            {
                sCompose += _rTranslator( *pElements );
                if ( pElements != pElementsEnd )
                    sCompose += '\n';
            }

            return sCompose;
        }
    }

    //--------------------------------------------------------------------
    bool StringRepresentation::convertGenericValueToString( const Any& _rValue, ::rtl::OUString& _rStringRep )
    {
        bool bCanConvert = true;

        switch ( _rValue.getValueTypeClass() )
        {
        case TypeClass_STRING:
            _rValue >>= _rStringRep;
            break;

        case TypeClass_BOOLEAN:
        {
            String aEntries( ModuleRes( RID_STR_BOOL ) );
            sal_Bool bValue = sal_False;
            _rValue >>= bValue;
            _rStringRep = bValue ? aEntries.GetToken( 1 ) : aEntries.GetToken( 0 );
        }
        break;

        // some sequence types
        case TypeClass_SEQUENCE:
        {
            Sequence< ::rtl::OUString> aStringValues;
            Sequence< sal_uInt16 > aUInt16Values;
            Sequence< sal_Int16 > aInt16Values;
            Sequence< sal_uInt32 > aUInt32Values;
            Sequence< sal_Int32 > aInt32Values;

            // string sequences
            if ( _rValue >>= aStringValues )
            {
                _rStringRep = composeSequenceElements( aStringValues, StringIdentity() );
            }
            // uInt16 sequences
            else if ( _rValue >>= aUInt16Values )
            {
                _rStringRep = composeSequenceElements( aUInt16Values, ConvertIntegerToString() );
            }
            // Int16 sequences
            else if ( _rValue >>= aInt16Values )
            {
                _rStringRep = composeSequenceElements( aInt16Values, ConvertIntegerToString() );
            }
            // uInt32 sequences
            else if ( _rValue >>= aUInt32Values )
            {
                _rStringRep = composeSequenceElements( aUInt32Values, ConvertIntegerToString() );
            }
            // Int32 sequences
            else if ( _rValue >>= aInt32Values )
            {
                _rStringRep = composeSequenceElements( aInt32Values, ConvertIntegerToString() );
            }
            else
                bCanConvert = false;
        }
        break;

        // some structs
        case TypeClass_STRUCT:
            if ( _rValue.getValueType().equals( ::getCppuType( static_cast< util::Date* >( NULL ) ) ) )
            {
                // weird enough, the string representation of dates, as used
                // by the control displaying dates, and thus as passed through the layers,
                // is YYYYMMDD.
                util::Date aUnoDate;
                _rValue >>= aUnoDate;

                ::Date aDate;
                ::utl::typeConvert( aUnoDate, aDate );

                _rStringRep = String::CreateFromInt32( aDate.GetDate() );
            }
            else if ( _rValue.getValueType().equals( ::getCppuType( static_cast< util::Time* >( NULL ) ) ) )
            {
                // similar for time (HHMMSSHH)
                util::Time aUnoTime;
                _rValue >>= aUnoTime;

                ::Time aTime;
                ::utl::typeConvert( aUnoTime, aTime );

                _rStringRep = String::CreateFromInt32( aTime.GetTime() );
            }
            else if ( _rValue.getValueType().equals( ::getCppuType( static_cast< util::DateTime* >( NULL ) ) ) )
            {
                util::DateTime aUnoDateTime;
                _rValue >>= aUnoDateTime;

                ::rtl::OUStringBuffer aBuffer;
                aBuffer.append( (sal_Int32)aUnoDateTime.Year );
                aBuffer.appendAscii( "-" );
                aBuffer.append( (sal_Int32)aUnoDateTime.Month );
                aBuffer.appendAscii( "-" );
                aBuffer.append( (sal_Int32)aUnoDateTime.Day );

                aBuffer.appendAscii( " " );

                aBuffer.append( (sal_Int32)aUnoDateTime.Hours );
                aBuffer.appendAscii( ":" );
                aBuffer.append( (sal_Int32)aUnoDateTime.Minutes );
                aBuffer.appendAscii( ":" );
                aBuffer.append( (sal_Int32)aUnoDateTime.Seconds );
                aBuffer.appendAscii( ":" );
                aBuffer.append( (sal_Int32)aUnoDateTime.HundredthSeconds );

                _rStringRep = aBuffer.makeStringAndClear();
            }
            else
                bCanConvert = false;
            break;

        default:
            bCanConvert = false;
            break;
        }

        return bCanConvert;
    }

    //--------------------------------------------------------------------
    ::rtl::OUString StringRepresentation::getStringRepFromPropertyValue( const Any& _rValue, sal_Int32 _nPropertyId, const IPropertyInfoService* _pMetaData )
    {
        // try the most basic types
        ::rtl::OUString sReturn = convertSimpleToString( _rValue );

        // try enum properties
        if ( _pMetaData )
        {
            sal_uInt32  nPropertyUIFlags = _pMetaData->getPropertyUIFlags( _nPropertyId );
            if ( ( nPropertyUIFlags & PROP_FLAG_ENUM ) != 0 )
            {
                sal_Int32 nIntValue = -1;
                if ( ::cppu::enum2int( nIntValue, _rValue ) )
                {
                    if ( ( nPropertyUIFlags & PROP_FLAG_ENUM_ONE ) == PROP_FLAG_ENUM_ONE )
                        // enum value starting with 1
                        --nIntValue;

                    ::std::vector< String > aEnumStrings = _pMetaData->getPropertyEnumRepresentations( _nPropertyId );
                    if ( ( nIntValue >= 0 ) && ( nIntValue < (sal_Int32)aEnumStrings.size() ) )
                    {
                        sReturn = aEnumStrings[ nIntValue ];
                    }
                    else
                        DBG_ERROR( "StringRepresentation::getStringRepFromPropertyValue: could not translate an enum value" );
                }
            }
        }

        if ( ( !sReturn.getLength() && _rValue.hasValue() ) || ( _rValue.getValueTypeClass() == TypeClass_BOOLEAN ) )
        {
        #if OSL_DEBUG_LEVEL > 0
            bool bCanConvert =
        #endif
            convertGenericValueToString( _rValue, sReturn );

        #if OSL_DEBUG_LEVEL > 0
            if ( !bCanConvert )
            {
                ::rtl::OString sMessage( "StringRepresentation::getStringRepFromPropertyValue: cannot convert values of type '" );
                sMessage += ::rtl::OString( _rValue.getValueType().getTypeName().getStr(), _rValue.getValueType().getTypeName().getLength(), RTL_TEXTENCODING_ASCII_US );
                sMessage += ::rtl::OString( "'!" );
                DBG_ERROR( sMessage.getStr() );
            }
        #endif

        }

        return sReturn;
    }

    //------------------------------------------------------------------------
    namespace
    {
        sal_Int32 getStringPos( const String& _rEntry, const ::std::vector< String >& _rEntries)
        {
            ::std::vector< String >::const_iterator pStart = _rEntries.begin();
            ::std::vector< String >::const_iterator pEnd = _rEntries.end();
            for ( ::std::vector< String >::const_iterator pEntries = pStart; pEntries != pEnd; ++pEntries )
            {
                if ( _rEntry == *pEntries )
                    return pEntries - pStart;
            }
            return -1;
        }
    }

    //--------------------------------------------------------------------
    bool StringRepresentation::convertStringToGenericValue( const ::rtl::OUString& _rStringRep, Any& _rValue, const Type& _rTargetType )
    {
        bool bCanConvert = true;

        switch ( _rTargetType.getTypeClass() )
        {
        case TypeClass_STRING:
            _rValue <<= _rStringRep;
            break;

        case TypeClass_BOOLEAN:
        {
            String sBooleanValues( ModuleRes( RID_STR_BOOL ) );
            if ( sBooleanValues.GetToken(0) == String( _rStringRep ) )
                _rValue <<= (sal_Bool)sal_False;
            else
                _rValue <<= (sal_Bool)sal_True;
        }
        break;

        case TypeClass_SEQUENCE:
        {
            Type aElementType = ::comphelper::getSequenceElementType( _rTargetType );

            String aStr( _rStringRep );
            switch ( aElementType.getTypeClass() )
            {
                case TypeClass_STRING:
                {
                    sal_uInt32 nEntryCount = aStr.GetTokenCount('\n');
                    Sequence< ::rtl::OUString> aStringSeq( nEntryCount );
                    ::rtl::OUString* pStringArray = aStringSeq.getArray();

                    for (sal_Int32 i=0; i<aStringSeq.getLength(); ++i, ++pStringArray)
                        *pStringArray = aStr.GetToken((sal_uInt16)i, '\n');
                    _rValue <<= aStringSeq;
                }
                break;
                case TypeClass_UNSIGNED_SHORT:
                {
                    sal_uInt32 nEntryCount = aStr.GetTokenCount('\n');
                    Sequence<sal_uInt16> aSeq( nEntryCount );

                    sal_uInt16* pArray = aSeq.getArray();

                    for (sal_Int32 i=0; i<aSeq.getLength(); ++i, ++pArray)
                        *pArray = (sal_uInt16)aStr.GetToken((sal_uInt16)i, '\n').ToInt32();

                    _rValue <<= aSeq;

                }
                break;
                case TypeClass_SHORT:
                {
                    sal_uInt32 nEntryCount = aStr.GetTokenCount('\n');
                    Sequence<sal_Int16> aSeq( nEntryCount );

                    sal_Int16* pArray = aSeq.getArray();

                    for (sal_Int32 i=0; i<aSeq.getLength(); ++i, ++pArray)
                        *pArray = (sal_Int16)aStr.GetToken((sal_uInt16)i, '\n').ToInt32();

                    _rValue <<= aSeq;

                }
                break;
                case TypeClass_LONG:
                {
                    sal_uInt32 nEntryCount = aStr.GetTokenCount('\n');
                    Sequence<sal_Int32> aSeq( nEntryCount );

                    sal_Int32* pArray = aSeq.getArray();

                    for (sal_Int32 i=0; i<aSeq.getLength(); ++i, ++pArray)
                        *pArray = aStr.GetToken((sal_uInt16)i, '\n').ToInt32();

                    _rValue <<= aSeq;

                }
                break;
                case TypeClass_UNSIGNED_LONG:
                {
                    sal_uInt32 nEntryCount = aStr.GetTokenCount('\n');
                    Sequence<sal_uInt32> aSeq( nEntryCount );

                    sal_uInt32* pArray = aSeq.getArray();

                    for (sal_Int32 i=0; i<aSeq.getLength(); ++i, ++pArray)
                        *pArray = aStr.GetToken((sal_uInt16)i, '\n').ToInt32();

                    _rValue <<= aSeq;

                }
                break;
                default:
                    bCanConvert = false;
                    break;
            }
        }
        break;

        case TypeClass_STRUCT:
            if ( _rTargetType.equals( ::getCppuType( static_cast< util::Date* >( NULL ) ) ) )
            {
                // weird enough, the string representation of dates, as used
                // by the control displaying dates, and thus as passed through the layers,
                // is YYYYMMDD.
                ::Date aDate( _rStringRep.toInt32() );

                util::Date aUnoDate;
                ::utl::typeConvert( aDate, aUnoDate );

                _rValue <<= aUnoDate;
            }
            else if ( _rTargetType.equals( ::getCppuType( static_cast< util::Time* >( NULL ) ) ) )
            {
                // similar for time (HHMMSSHH)
                ::Time aTime( _rStringRep.toInt32() );

                util::Time aUnoTime;
                ::utl::typeConvert( aTime, aUnoTime );

                _rValue <<= aUnoTime;
            }
            else if ( _rTargetType.equals( ::getCppuType( static_cast< util::DateTime* >( NULL ) ) ) )
            {
                util::DateTime aUnoDateTime;

                sal_Int32 nSepPos = -1, nPrevSepPos = 0;

                nSepPos = _rStringRep.indexOf( '-', nPrevSepPos = nSepPos + 1 );
                aUnoDateTime.Year = (sal_Int16)_rStringRep.copy( nPrevSepPos, nSepPos - nPrevSepPos ).toInt32();

                nSepPos = _rStringRep.indexOf( '-', nPrevSepPos = nSepPos + 1 );
                aUnoDateTime.Month = (sal_Int16)_rStringRep.copy( nPrevSepPos, nSepPos - nPrevSepPos ).toInt32();

                nSepPos = _rStringRep.indexOf( ' ', nPrevSepPos = nSepPos + 1 );
                aUnoDateTime.Day = (sal_Int16)_rStringRep.copy( nPrevSepPos, nSepPos - nPrevSepPos ).toInt32();

                nSepPos = _rStringRep.indexOf( ':', nPrevSepPos = nSepPos + 1 );
                aUnoDateTime.Hours = (sal_Int16)_rStringRep.copy( nPrevSepPos, nSepPos - nPrevSepPos ).toInt32();

                nSepPos = _rStringRep.indexOf( ':', nPrevSepPos = nSepPos + 1 );
                aUnoDateTime.Minutes = (sal_Int16)_rStringRep.copy( nPrevSepPos, nSepPos - nPrevSepPos ).toInt32();

                nSepPos = _rStringRep.indexOf( ':', nPrevSepPos = nSepPos + 1 );
                aUnoDateTime.Seconds = (sal_Int16)_rStringRep.copy( nPrevSepPos, nSepPos - nPrevSepPos ).toInt32();

                aUnoDateTime.Seconds = (sal_Int16)_rStringRep.copy( nSepPos + 1 ).toInt32();

                _rValue <<= aUnoDateTime;
            }
            else
                bCanConvert = false;
            break;

        default:
            bCanConvert = false;
            break;
        }

        return bCanConvert;
    }

    //--------------------------------------------------------------------
    Any StringRepresentation::getPropertyValueFromStringRep( const ::rtl::OUString& _rStringRep, const Type& _rTargetType,
            sal_Int32 _nPropertyId, const IPropertyInfoService* _pMetaData )
    {
        Any aReturn;

        // enum properties
        if ( _pMetaData )
        {
            sal_uInt32  nPropertyUIFlags = _pMetaData->getPropertyUIFlags( _nPropertyId );
            if ( ( nPropertyUIFlags & PROP_FLAG_ENUM ) != 0 )
            {
                ::std::vector< String > aEnumStrings = _pMetaData->getPropertyEnumRepresentations( _nPropertyId );
                sal_Int32 nPos = getStringPos( _rStringRep, aEnumStrings );
                if ( -1 != nPos )
                {
                    if ( ( nPropertyUIFlags & PROP_FLAG_ENUM_ONE ) == PROP_FLAG_ENUM_ONE )
                        // enum value starting with 1
                        ++nPos;

                    switch ( _rTargetType.getTypeClass() )
                    {
                        case TypeClass_ENUM:
                            aReturn = ::cppu::int2enum( nPos, _rTargetType );
                            break;

                        case TypeClass_SHORT:
                            aReturn <<= (sal_Int16)nPos;
                            break;

                        case TypeClass_UNSIGNED_SHORT:
                            aReturn <<= (sal_uInt16)nPos;
                            break;

                        case TypeClass_UNSIGNED_LONG:
                            aReturn <<= (sal_uInt32)nPos;
                            break;

                        default:
                            aReturn <<= (sal_Int32)nPos;
                            break;
                    }
                }
                else
                    DBG_ERROR("OPropertyBrowserController::getPropertyValueFromStringRep: could not translate the enum string!");

                return aReturn;
            }
        }

        TypeClass ePropertyType = _rTargetType.getTypeClass();
        switch ( ePropertyType )
        {
        case TypeClass_FLOAT:
        case TypeClass_DOUBLE:
        case TypeClass_BYTE:
        case TypeClass_SHORT:
        case TypeClass_LONG:
        case TypeClass_HYPER:
        case TypeClass_UNSIGNED_SHORT:
        case TypeClass_UNSIGNED_LONG:
        case TypeClass_UNSIGNED_HYPER:
            try
            {
                if ( m_xTypeConverter.is() )
                    aReturn = m_xTypeConverter->convertToSimpleType( makeAny( _rStringRep ), ePropertyType );
            }
            catch( const CannotConvertException& ) { }
            catch( const IllegalArgumentException& ) { }
            break;

        default:
        #if OSL_DEBUG_LEVEL > 0
            bool bCanConvert =
        #endif
            convertStringToGenericValue( _rStringRep, aReturn, _rTargetType );

        #if OSL_DEBUG_LEVEL > 0
            // could not convert ...
            if ( !bCanConvert && _rStringRep.getLength() )
            {
                ::rtl::OString sMessage( "StringRepresentation::getPropertyValueFromStringRep: cannot convert into values of type '" );
                sMessage += ::rtl::OString( _rTargetType.getTypeName().getStr(), _rTargetType.getTypeName().getLength(), RTL_TEXTENCODING_ASCII_US );
                sMessage += ::rtl::OString( "'!" );
                DBG_ERROR( sMessage.getStr() );
            }
        #endif
        }

        return aReturn;
    }

//........................................................................
}   // namespace pcr
//........................................................................

