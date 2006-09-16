/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: stringrepresentation.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 13:23:57 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_extensions.hxx"

#ifndef EXTENSIONS_SOURCE_PROPCTRLR_STRINGREPRESENTATION_HXX
#include "stringrepresentation.hxx"
#endif
#ifndef EXTENSIONS_SOURCE_PROPCTRLR_ENUMREPRESENTATION_HXX
#include "enumrepresentation.hxx"
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

#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif

#ifndef _UNOTOOLS_DATETIME_HXX_
#include <unotools/datetime.hxx>
#endif

#include <functional>
#include <algorithm>

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
        struct ConvertIntegerFromAndToString
        {
            ::rtl::OUString operator()( sal_Int32 _rIntValue ) const
            {
                return ::rtl::OUString::valueOf( (sal_Int32)_rIntValue );
            }
            sal_Int32 operator()( const ::rtl::OUString& _rStringValue ) const
            {
                return _rStringValue.toInt32();
            }
        };

        struct StringIdentity
        {
            ::rtl::OUString operator()( const ::rtl::OUString& _rValue ) const
            {
                return _rValue;
            }
        };

        template < class ElementType, class Transformer >
        ::rtl::OUString composeSequenceElements( const Sequence< ElementType >& _rElements, const Transformer& _rTransformer )
        {
            String sCompose;

            // loop through the elements and concatenate the string representations of the integers
            // (separated by a line break)
            const ElementType* pElements = _rElements.getConstArray();
            const ElementType* pElementsEnd = pElements + _rElements.getLength();
            for ( ; pElements != pElementsEnd; ++pElements )
            {
                sCompose += String( _rTransformer( *pElements ) );
                if ( pElements != pElementsEnd )
                    sCompose += '\n';
            }

            return sCompose;
        }

        template < class ElementType, class Transformer >
        void splitComposedStringToSequence( const ::rtl::OUString& _rComposed, Sequence< ElementType >& _out_SplitUp, const Transformer& _rTransformer )
        {
            _out_SplitUp.realloc( 0 );
            if ( !_rComposed.getLength() )
                return;
            sal_Int32 tokenPos = 0;
            do
            {
                _out_SplitUp.realloc( _out_SplitUp.getLength() + 1 );
                _out_SplitUp[ _out_SplitUp.getLength() - 1 ] = (ElementType)_rTransformer( _rComposed.getToken( 0, '\n', tokenPos ) );
            }
            while ( tokenPos != -1 );
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
            String aEntries( PcrRes( RID_STR_BOOL ) );
            sal_Bool bValue = sal_False;
            _rValue >>= bValue;
            _rStringRep = bValue ? aEntries.GetToken( 1 ) : aEntries.GetToken( 0 );
        }
        break;

        // some sequence types
        case TypeClass_SEQUENCE:
        {
            Sequence< ::rtl::OUString > aStringValues;
            Sequence< sal_Int8 > aInt8Values;
            Sequence< sal_uInt16 > aUInt16Values;
            Sequence< sal_Int16 > aInt16Values;
            Sequence< sal_uInt32 > aUInt32Values;
            Sequence< sal_Int32 > aInt32Values;

            // string sequences
            if ( _rValue >>= aStringValues )
            {
                _rStringRep = composeSequenceElements( aStringValues, StringIdentity() );
            }
            // byte sequences
            else if ( _rValue >>= aInt8Values )
            {
                _rStringRep = composeSequenceElements( aInt8Values, ConvertIntegerFromAndToString() );
            }
            // uInt16 sequences
            else if ( _rValue >>= aUInt16Values )
            {
                _rStringRep = composeSequenceElements( aUInt16Values, ConvertIntegerFromAndToString() );
            }
            // Int16 sequences
            else if ( _rValue >>= aInt16Values )
            {
                _rStringRep = composeSequenceElements( aInt16Values, ConvertIntegerFromAndToString() );
            }
            // uInt32 sequences
            else if ( _rValue >>= aUInt32Values )
            {
                _rStringRep = composeSequenceElements( aUInt32Values, ConvertIntegerFromAndToString() );
            }
            // Int32 sequences
            else if ( _rValue >>= aInt32Values )
            {
                _rStringRep = composeSequenceElements( aInt32Values, ConvertIntegerFromAndToString() );
            }
            else
                bCanConvert = false;
        }
        break;

        // some structs
        case TypeClass_STRUCT:
            OSL_ENSURE( false, "StringRepresentation::convertGenericValueToString(STRUCT): this is dead code - isn't it?" );
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
                aBuffer.appendAscii( "." );
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
    ::rtl::OUString StringRepresentation::convertPropertyValueToStringRepresentation( const Any& _rValue )
    {
        ::rtl::OUString sReturn;
        if ( !convertGenericValueToString( _rValue, sReturn ) )
        {
            sReturn = convertSimpleToString( _rValue );
        #ifdef DBG_UTIL
            if ( !sReturn.getLength() && _rValue.hasValue() )
            {
                ::rtl::OString sMessage( "StringRepresentation::convertPropertyValueToStringRepresentation: cannot convert values of type '" );
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
            String sBooleanValues( PcrRes( RID_STR_BOOL ) );
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
                    Sequence< ::rtl::OUString > aElements;
                    splitComposedStringToSequence( aStr, aElements, StringIdentity() );
                    _rValue <<= aElements;
                }
                break;
                case TypeClass_SHORT:
                {
                    Sequence< sal_Int16 > aElements;
                    splitComposedStringToSequence( aStr, aElements, ConvertIntegerFromAndToString() );
                    _rValue <<= aElements;
                }
                break;
                case TypeClass_UNSIGNED_SHORT:
                {
                    Sequence< sal_uInt16 > aElements;
                    splitComposedStringToSequence( aStr, aElements, ConvertIntegerFromAndToString() );
                    _rValue <<= aElements;
                }
                break;
                case TypeClass_LONG:
                {
                    Sequence< sal_Int32 > aElements;
                    splitComposedStringToSequence( aStr, aElements, ConvertIntegerFromAndToString() );
                    _rValue <<= aElements;
                }
                break;
                case TypeClass_UNSIGNED_LONG:
                {
                    Sequence< sal_uInt32 > aElements;
                    splitComposedStringToSequence( aStr, aElements, ConvertIntegerFromAndToString() );
                    _rValue <<= aElements;
                }
                break;
                case TypeClass_BYTE:
                {
                    Sequence< sal_Int8 > aElements;
                    splitComposedStringToSequence( aStr, aElements, ConvertIntegerFromAndToString() );
                    _rValue <<= aElements;
                }
                break;
                default:
                    bCanConvert = false;
                    break;
            }
        }
        break;

        case TypeClass_STRUCT:
            OSL_ENSURE( false, "StringRepresentation::convertStringToGenericValue(STRUCT): this is dead code - isn't it?" );
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

                nSepPos = _rStringRep.indexOf( '.', nPrevSepPos = nSepPos + 1 );
                aUnoDateTime.Seconds = (sal_Int16)_rStringRep.copy( nPrevSepPos, nSepPos - nPrevSepPos ).toInt32();

                aUnoDateTime.HundredthSeconds = (sal_Int16)_rStringRep.copy( nSepPos + 1 ).toInt32();

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
    Any StringRepresentation::convertStringRepresentationToPropertyValue( const ::rtl::OUString& _rStringRep, const Type& _rTargetType )
    {
        Any aReturn;

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
                ::rtl::OString sMessage( "StringRepresentation::convertStringRepresentationToPropertyValue: cannot convert into values of type '" );
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

