/*************************************************************************
 *
 *  $RCSfile: property.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hjs $ $Date: 2001-03-13 18:24:20 $
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

#ifndef _COMPHELPER_PROPERTY_HXX_
#define _COMPHELPER_PROPERTY_HXX_

#ifndef _CPPUHELPER_PROPTYPEHLP_HXX
#include <cppuhelper/proptypehlp.hxx>
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTY_HPP_
#include <com/sun/star/beans/Property.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

//=========================================================================
//= property helper classes
//=========================================================================

//... namespace comphelper .......................................................
namespace comphelper
{
//.........................................................................

    namespace starbeans = ::com::sun::star::beans;
    namespace staruno   = ::com::sun::star::uno;

/** compare two properties by name
*/
extern int
#if defined( WNT )
 __cdecl
#endif
#if defined( ICC ) && defined( OS2 )
_Optlink
#endif
    PropertyCompare( const void* pFirst, const void* pSecond);

//------------------------------------------------------------------
/// remove the property with the given name from the given sequence
void RemoveProperty(staruno::Sequence<starbeans::Property>& seqProps, const ::rtl::OUString& _rPropName);

//------------------------------------------------------------------
/** within the given property sequence, modify attributes of a special property
    @param  _rProps         the sequence of properties to search in
    @param  _sPropName      the name of the property which's attributes should be modified
    @param  _nAddAttrib     the attributes which should be added
    @param  _nRemoveAttrib  the attributes which should be removed
*/
void ModifyPropertyAttributes(staruno::Sequence<starbeans::Property>& _rProps, const ::rtl::OUString& _sPropName, sal_Int16 _nAddAttrib, sal_Int16 _nRemoveAttrib);

//------------------------------------------------------------------
/** check if the given set has the given property.
*/
sal_Bool hasProperty(const rtl::OUString& _rName, const staruno::Reference<starbeans::XPropertySet>& _rxSet);

//------------------------------------------------------------------
/** copy properties between property sets, in compliance with the property
    attributes of the target object
*/
void copyProperties(const staruno::Reference<starbeans::XPropertySet>& _rxSource,
                    const staruno::Reference<starbeans::XPropertySet>& _rxDest);

//==================================================================
//= property conversion helpers
//==================================================================

/** helper for implementing ::cppu::OPropertySetHelper::convertFastPropertyValue
    @param          _rConvertedValue    the conversion result (if successfull)
    @param          _rOldValue          the old value of the property, calculated from _rCurrentValue
    @param          _rValueToSet        the new value which is about to be set
    @param          _rCurrentValue      the current value of the property
    @return         sal_True, if the value could be converted and has changed
                    sal_False, if the value could be converted and has not changed
    @exception      InvalidArgumentException thrown if the value could not be converted to the requested type (which is the template argument)
*/
template <class TYPE>
sal_Bool tryPropertyValue(staruno::Any& /*out*/_rConvertedValue, staruno::Any& /*out*/_rOldValue, const staruno::Any& _rValueToSet, const TYPE& _rCurrentValue)
{
    sal_Bool bModified(sal_False);
    TYPE aNewValue;
    ::cppu::convertPropertyValue(aNewValue, _rValueToSet);
    if (aNewValue != _rCurrentValue)
    {
        _rConvertedValue <<= aNewValue;
        _rOldValue <<= _rCurrentValue;
        bModified = sal_True;
    }
    return bModified;
}

/** helper for implementing ::cppu::OPropertySetHelper::convertFastPropertyValue for enum values
    @param          _rConvertedValue    the conversion result (if successfull)
    @param          _rOldValue          the old value of the property, calculated from _rCurrentValue
    @param          _rValueToSet        the new value which is about to be set
    @param          _rCurrentValue      the current value of the property
    @return         sal_True, if the value could be converted and has changed
                    sal_False, if the value could be converted and has not changed
    @exception      InvalidArgumentException thrown if the value could not be converted to the requested type (which is the template argument)
*/
template <class ENUMTYPE>
sal_Bool tryPropertyValueEnum(staruno::Any& /*out*/_rConvertedValue, staruno::Any& /*out*/_rOldValue, const staruno::Any& _rValueToSet, const ENUMTYPE& _rCurrentValue)
{
    if (::getCppuType(&_rCurrentValue).getTypeClass() != staruno::TypeClass_ENUM)
        return tryPropertyValue(_rConvertedValue, _rOldValue, _rValueToSet, _rCurrentValue);

    sal_Bool bModified(sal_False);
    ENUMTYPE aNewValue;
    ::cppu::any2enum(aNewValue, _rValueToSet);
        // will throw an exception if not convertible

    if (aNewValue != _rCurrentValue)
    {
        _rConvertedValue <<= aNewValue;
        _rOldValue <<= _rCurrentValue;
        bModified = sal_True;
    }
    return bModified;
}

/** helper for implementing ::cppu::OPropertySetHelper::convertFastPropertyValue for boolean properties
    @param          _rConvertedValue    the conversion result (if successfull)
    @param          _rOldValue          the old value of the property, calculated from _rCurrentValue
    @param          _rValueToSet        the new value which is about to be set
    @param          _rCurrentValue      the current value of the property
    @return         sal_True, if the value could be converted and has changed
                    sal_False, if the value could be converted and has not changed
    @exception      InvalidArgumentException thrown if the value could not be converted to a boolean type
*/
inline sal_Bool tryPropertyValue(staruno::Any& /*out*/_rConvertedValue, staruno::Any& /*out*/_rOldValue, const staruno::Any& _rValueToSet, sal_Bool _bCurrentValue)
{
    sal_Bool bModified(sal_False);
    sal_Bool bNewValue;
    ::cppu::convertPropertyValue(bNewValue, _rValueToSet);
    if (bNewValue != _bCurrentValue)
    {
        _rConvertedValue.setValue(&bNewValue, ::getBooleanCppuType());
        _rOldValue.setValue(&_bCurrentValue, ::getBooleanCppuType());
        bModified = sal_True;
    }
    return bModified;
}

/** helper for implementing ::cppu::OPropertySetHelper::convertFastPropertyValue
    @param          _rConvertedValue    the conversion result (if successfull)
    @param          _rOldValue          the old value of the property, calculated from _rCurrentValue
    @param          _rValueToSet        the new value which is about to be set
    @param          _rCurrentValue      the current value of the property
    @param          _rExpectedType      the type which the property should have (if not void)
    @return         sal_True, if the value could be converted and has changed
                    sal_False, if the value could be converted and has not changed
    @exception      InvalidArgumentException thrown if the value could not be converted to the requested type (which is the template argument)
*/
sal_Bool tryPropertyValue(staruno::Any& _rConvertedValue, staruno::Any& _rOldValue, const staruno::Any& _rValueToSet, staruno::Any& _rCurrentValue, const staruno::Type& _rExpectedType);

//.........................................................................
}
//... namespace comphelper .......................................................

#endif // _COMPHELPER_PROPERTY_HXX_

