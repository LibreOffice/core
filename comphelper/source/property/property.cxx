/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: property.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: vg $ $Date: 2007-01-15 14:44:49 $
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
#include "precompiled_comphelper.hxx"


#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#if OSL_DEBUG_LEVEL > 0
    #ifndef _RTL_STRBUF_HXX_
    #include <rtl/strbuf.hxx>
    #endif
    #ifndef _CPPUHELPER_EXC_HLP_HXX_
    #include <cppuhelper/exc_hlp.hxx>
    #endif
    #ifndef _OSL_THREAD_H_
    #include <osl/thread.h>
    #endif
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_ILLEGALARGUMENTEXCEPTION_HPP_
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_GENFUNC_H_
#include <com/sun/star/uno/genfunc.h>
#endif

#include <algorithm>

//.........................................................................
namespace comphelper
{

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::beans::XPropertySet;
    using ::com::sun::star::beans::XPropertySetInfo;
    using ::com::sun::star::beans::Property;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::Type;
    using ::com::sun::star::uno::cpp_queryInterface;
    using ::com::sun::star::uno::cpp_acquire;
    using ::com::sun::star::uno::cpp_release;
    /** === end UNO using === **/
    namespace PropertyAttribute = ::com::sun::star::beans::PropertyAttribute;

//------------------------------------------------------------------
void copyProperties(const Reference<XPropertySet>& _rxSource,
                    const Reference<XPropertySet>& _rxDest)
{
    if (!_rxSource.is() || !_rxDest.is())
    {
        OSL_ENSURE(sal_False, "copyProperties: invalid arguments !");
    }

    Reference< XPropertySetInfo > xSourceProps = _rxSource->getPropertySetInfo();
    Reference< XPropertySetInfo > xDestProps = _rxDest->getPropertySetInfo();

    Sequence< Property > aSourceProps = xSourceProps->getProperties();
    const Property* pSourceProps = aSourceProps.getConstArray();
    Property aDestProp;
    for (sal_Int32 i=0; i<aSourceProps.getLength(); ++i, ++pSourceProps)
    {
        if ( xDestProps->hasPropertyByName(pSourceProps->Name) )
        {
            try
            {
                aDestProp = xDestProps->getPropertyByName(pSourceProps->Name);
                if (0 == (aDestProp.Attributes & PropertyAttribute::READONLY))
                    _rxDest->setPropertyValue(pSourceProps->Name, _rxSource->getPropertyValue(pSourceProps->Name));
            }
            catch (Exception&)
            {
#if OSL_DEBUG_LEVEL > 0
                ::rtl::OStringBuffer aBuffer;
                aBuffer.append( "::comphelper::copyProperties: could not copy property '" );
                aBuffer.append( ::rtl::OString( pSourceProps->Name.getStr(), pSourceProps->Name.getLength(), RTL_TEXTENCODING_ASCII_US ) );
                aBuffer.append( "' to the destination set.\n" );

                Any aException( ::cppu::getCaughtException() );
                aBuffer.append( "Caught an exception of type '" );
                ::rtl::OUString sExceptionType( aException.getValueTypeName() );
                aBuffer.append( ::rtl::OString( sExceptionType.getStr(), sExceptionType.getLength(), RTL_TEXTENCODING_ASCII_US ) );
                aBuffer.append( "'" );

                Exception aBaseException;
                if ( ( aException >>= aBaseException ) && aBaseException.Message.getLength() )
                {
                    aBuffer.append( ", saying '" );
                    aBuffer.append( ::rtl::OString( aBaseException.Message.getStr(), aBaseException.Message.getLength(), osl_getThreadTextEncoding() ) );
                    aBuffer.append( "'" );
                }
                aBuffer.append( "." );

                OSL_ENSURE( sal_False, aBuffer.getStr() );
#endif
            }
        }
    }
}

//------------------------------------------------------------------
sal_Bool hasProperty(const rtl::OUString& _rName, const Reference<XPropertySet>& _rxSet)
{
    if (_rxSet.is())
    {
        //  XPropertySetInfoRef xInfo(rxSet->getPropertySetInfo());
        return _rxSet->getPropertySetInfo()->hasPropertyByName(_rName);
    }
    return sal_False;
}

//------------------------------------------------------------------
void RemoveProperty(Sequence<Property>& _rProps, const rtl::OUString& _rPropName)
{
    sal_Int32 nLen = _rProps.getLength();

    // binaere Suche
    const Property* pProperties = _rProps.getConstArray();
    const Property* pResult = ::std::lower_bound(pProperties, pProperties + nLen, _rPropName,PropertyStringLessFunctor());

    // gefunden ?
    if ( pResult && (pResult != pProperties + nLen) && (pResult->Name == _rPropName) )
    {
        OSL_ENSURE(pResult->Name.equals(_rPropName), "::RemoveProperty Properties nicht sortiert");
        removeElementAt(_rProps, pResult - pProperties);
    }
}

//------------------------------------------------------------------
void ModifyPropertyAttributes(Sequence<Property>& seqProps, const ::rtl::OUString& sPropName, sal_Int16 nAddAttrib, sal_Int16 nRemoveAttrib)
{
    sal_Int32 nLen = seqProps.getLength();

    // binaere Suche
    Property* pProperties = seqProps.getArray();
    Property* pResult = ::std::lower_bound(pProperties, pProperties + nLen,sPropName, PropertyStringLessFunctor());

    // gefunden ?
    if ( pResult && (pResult != pProperties + nLen) && (pResult->Name == sPropName) )
    {
        pResult->Attributes |= nAddAttrib;
        pResult->Attributes &= ~nRemoveAttrib;
    }
}

//------------------------------------------------------------------
sal_Bool tryPropertyValue(Any& _rConvertedValue, Any& _rOldValue, const Any& _rValueToSet, Any& _rCurrentValue, const Type& _rExpectedType)
{
    sal_Bool bModified(sal_False);
    if (_rCurrentValue.getValue() != _rValueToSet.getValue())
    {
        if ( _rValueToSet.hasValue() && ( !_rExpectedType.equals( _rValueToSet.getValueType() ) ) )
        {
            _rConvertedValue = Any( NULL, _rExpectedType.getTypeLibType() );

            if  ( !uno_type_assignData(
                    const_cast< void* >( _rConvertedValue.getValue() ), _rConvertedValue.getValueType().getTypeLibType(),
                    const_cast< void* >( _rValueToSet.getValue() ), _rValueToSet.getValueType().getTypeLibType(),
                    reinterpret_cast< uno_QueryInterfaceFunc >(
                        cpp_queryInterface),
                    reinterpret_cast< uno_AcquireFunc >(cpp_acquire),
                    reinterpret_cast< uno_ReleaseFunc >(cpp_release)
                  )
                )
                throw starlang::IllegalArgumentException();
        }
        else
            _rConvertedValue = _rValueToSet;

        if ( _rCurrentValue != _rConvertedValue )
        {
            _rOldValue = _rCurrentValue;
            bModified = sal_True;
        }
    }
    return bModified;
}

//.........................................................................
}
//.........................................................................

