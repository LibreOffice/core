/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: property.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 22:52:01 $
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

    namespace starlang = ::com::sun::star::lang;

//------------------------------------------------------------------
void copyProperties(const staruno::Reference<starbeans::XPropertySet>& _rxSource,
                    const staruno::Reference<starbeans::XPropertySet>& _rxDest)
{
    if (!_rxSource.is() || !_rxDest.is())
    {
        OSL_ENSURE(sal_False, "copyProperties : invalid arguments !");
    }

    staruno::Reference< starbeans::XPropertySetInfo > xSourceProps = _rxSource->getPropertySetInfo();
    staruno::Reference< starbeans::XPropertySetInfo > xDestProps = _rxDest->getPropertySetInfo();

    staruno::Sequence< starbeans::Property > aSourceProps = xSourceProps->getProperties();
    const starbeans::Property* pSourceProps = aSourceProps.getConstArray();
    starbeans::Property aDestProp;
    for (sal_Int32 i=0; i<aSourceProps.getLength(); ++i, ++pSourceProps)
    {
        if ( xDestProps->hasPropertyByName(pSourceProps->Name) )
        {
            try
            {
                aDestProp = xDestProps->getPropertyByName(pSourceProps->Name);
                if (0 == (aDestProp.Attributes & starbeans::PropertyAttribute::READONLY))
                    _rxDest->setPropertyValue(pSourceProps->Name, _rxSource->getPropertyValue(pSourceProps->Name));
            }
            catch (staruno::Exception&)
            {
                OSL_ENSURE(sal_False, "copyProperties : could not transfer at least one of the properties !");
            }
        }
    }
}

//------------------------------------------------------------------
sal_Bool hasProperty(const rtl::OUString& _rName, const staruno::Reference<starbeans::XPropertySet>& _rxSet)
{
    if (_rxSet.is())
    {
        //  XPropertySetInfoRef xInfo(rxSet->getPropertySetInfo());
        return _rxSet->getPropertySetInfo()->hasPropertyByName(_rName);
    }
    return sal_False;
}

//------------------------------------------------------------------
void RemoveProperty(staruno::Sequence<starbeans::Property>& _rProps, const rtl::OUString& _rPropName)
{
    sal_Int32 nLen = _rProps.getLength();

    // binaere Suche
    const starbeans::Property* pProperties = _rProps.getConstArray();
    const starbeans::Property* pResult = ::std::lower_bound(pProperties, pProperties + nLen, _rPropName,PropertyStringLessFunctor());

    // gefunden ?
    if ( pResult && (pResult != pProperties + nLen) && (pResult->Name == _rPropName) )
    {
        OSL_ENSURE(pResult->Name.equals(_rPropName), "::RemoveProperty Properties nicht sortiert");
        removeElementAt(_rProps, pResult - pProperties);
    }
}

//------------------------------------------------------------------
void ModifyPropertyAttributes(staruno::Sequence<starbeans::Property>& seqProps, const ::rtl::OUString& sPropName, sal_Int16 nAddAttrib, sal_Int16 nRemoveAttrib)
{
    sal_Int32 nLen = seqProps.getLength();

    // binaere Suche
    starbeans::Property* pProperties = seqProps.getArray();
    starbeans::Property* pResult = ::std::lower_bound(pProperties, pProperties + nLen,sPropName, PropertyStringLessFunctor());

    // gefunden ?
    if ( pResult && (pResult != pProperties + nLen) && (pResult->Name == sPropName) )
    {
        pResult->Attributes |= nAddAttrib;
        pResult->Attributes &= ~nRemoveAttrib;
    }
}

//------------------------------------------------------------------
sal_Bool tryPropertyValue(staruno::Any& _rConvertedValue, staruno::Any& _rOldValue, const staruno::Any& _rValueToSet, staruno::Any& _rCurrentValue, const staruno::Type& _rExpectedType)
{
    sal_Bool bModified(sal_False);
    if (_rCurrentValue.getValue() != _rValueToSet.getValue())
    {
        if ( _rValueToSet.hasValue() && ( !_rExpectedType.equals( _rValueToSet.getValueType() ) ) )
        {
            _rConvertedValue = staruno::Any( NULL, _rExpectedType.getTypeLibType() );

            if  ( !uno_type_assignData(
                    const_cast< void* >( _rConvertedValue.getValue() ), _rConvertedValue.getValueType().getTypeLibType(),
                    const_cast< void* >( _rValueToSet.getValue() ), _rValueToSet.getValueType().getTypeLibType(),
                    reinterpret_cast< uno_QueryInterfaceFunc >(
                        staruno::cpp_queryInterface),
                    reinterpret_cast< uno_AcquireFunc >(staruno::cpp_acquire),
                    reinterpret_cast< uno_ReleaseFunc >(staruno::cpp_release)
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

