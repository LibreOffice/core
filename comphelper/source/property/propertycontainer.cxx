/*************************************************************************
 *
 *  $RCSfile: propertycontainer.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: fs $ $Date: 2000-10-06 14:43:54 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
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

#ifndef _COMPHELPER_PROPERTYCONTAINER_HXX_
#include <comphelper/propertycontainer.hxx>
#endif
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif

#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _UNO_DATA_H_
#include <uno/data.h>
#endif
#ifndef _COM_SUN_STAR_UNO_GENFUNC_H_
#include <com/sun/star/uno/genfunc.h>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif

//.........................................................................
namespace comphelper
{
//.........................................................................

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;

//==========================================================================
//= OPropertyContainer
//==========================================================================
//--------------------------------------------------------------------------
OPropertyContainer::OPropertyContainer(::cppu::OBroadcastHelper& _rBHelper)
    :OPropertyContainer_Base(_rBHelper)
    ,m_bAlreadyAccessed(sal_False)
{
}

//--------------------------------------------------------------------------
Sequence< Type > SAL_CALL OPropertyContainer::getTypes() throw (RuntimeException)
{
    // just the types from our one and only base class
    ::cppu::OTypeCollection aTypes(
        ::getCppuType( static_cast< Reference< XPropertySet >* >(NULL)),
        ::getCppuType( static_cast< Reference< XFastPropertySet >* >(NULL)),
        ::getCppuType( static_cast< Reference< XMultiPropertySet >* >(NULL))
    );
    return aTypes.getTypes();
}

//--------------------------------------------------------------------------
void OPropertyContainer::registerProperty(const ::rtl::OUString& _rName, sal_Int32 _nHandle,
        sal_Int32 _nAttributes, void* _pPointerToMember, const Type& _rMemberType)
{
    OSL_ENSHURE(!m_bAlreadyAccessed, "OPropertyContainer::registerProperty : invalid call, the property sequence was already accessed !");
    OSL_ENSHURE((_nAttributes & PropertyAttribute::MAYBEVOID) == 0,
        "OPropertyContainer::registerProperty : don't use this for properties which may be void ! There is a method called \"registerMayBeVoidProperty\" for this !");
    OSL_ENSHURE(!_rMemberType.equals(::getCppuType(static_cast< Any* >(NULL))),
        "OPropertyContainer::registerProperty : don't give my the type of an uno::Any ! Really can't handle this !");
    OSL_ENSHURE(_pPointerToMember,
        "OPropertyContainer::registerProperty : you gave me nonsense : the pointer must be non-NULL");

    PropertyDescription aNewProp;
    aNewProp.sName = _rName;
    aNewProp.nHandle = _nHandle;
    aNewProp.nAttributes = _nAttributes;
    aNewProp.eLocated = PropertyDescription::ltDerivedClassRealType;
    aNewProp.aLocation.pDerivedClassMember = _pPointerToMember;
    aNewProp.aType = _rMemberType;

    implPushBackProperty(aNewProp);
}

//--------------------------------------------------------------------------
void OPropertyContainer::registerMayBeVoidProperty(const ::rtl::OUString& _rName, sal_Int32 _nHandle, sal_Int32 _nAttributes,
        Any* _pPointerToMember, const Type& _rExpectedType)
{
    OSL_ENSHURE(!m_bAlreadyAccessed, "OPropertyContainer::registerMayBeVoidProperty : invalid call, the property sequence was already accessed !");
    OSL_ENSHURE((_nAttributes & PropertyAttribute::MAYBEVOID) != 0,
        "OPropertyContainer::registerMayBeVoidProperty : why calling this when the attributes say nothing about may-be-void ?");
    OSL_ENSHURE(!_rExpectedType.equals(::getCppuType(static_cast< Any* >(NULL))),
        "OPropertyContainer::registerMayBeVoidProperty : don't give my the type of an uno::Any ! Really can't handle this !");
    OSL_ENSHURE(_pPointerToMember,
        "OPropertyContainer::registerMayBeVoidProperty : you gave me nonsense : the pointer must be non-NULL");

    _nAttributes &= PropertyAttribute::MAYBEVOID;

    PropertyDescription aNewProp;
    aNewProp.sName = _rName;
    aNewProp.nHandle = _nHandle;
    aNewProp.nAttributes = _nAttributes;
    aNewProp.eLocated = PropertyDescription::ltDerivedClassAnyType;
    aNewProp.aLocation.pDerivedClassMember = _pPointerToMember;
    aNewProp.aType = _rExpectedType;

    implPushBackProperty(aNewProp);
}


//--------------------------------------------------------------------------
void OPropertyContainer::registerPropertyNoMember(const ::rtl::OUString& _rName, sal_Int32 _nHandle, sal_Int32 _nAttributes,
        const Type& _rType, void* _pInitialValue)
{
    OSL_ENSHURE(!m_bAlreadyAccessed, "OPropertyContainer::registerMayBeVoidProperty : invalid call, the property sequence was already accessed !");
    OSL_ENSHURE(!_rType.equals(::getCppuType(static_cast< Any* >(NULL))),
        "OPropertyContainer::registerPropertyNoMember : don't give my the type of an uno::Any ! Really can't handle this !");
    OSL_ENSHURE(!_pInitialValue || ((_nAttributes & PropertyAttribute::MAYBEVOID) != 0),
        "OPropertyContainer::registerPropertyNoMember : you should not ommit the initial value if the property can't be void ! This will definitivly crash later !");

    PropertyDescription aNewProp;
    aNewProp.sName = _rName;
    aNewProp.nHandle = _nHandle;
    aNewProp.nAttributes = _nAttributes;
    aNewProp.eLocated = PropertyDescription::ltHoldMyself;
    aNewProp.aLocation.nOwnClassVectorIndex = m_aHoldProperties.size();
    if (_pInitialValue)
        m_aHoldProperties.push_back(Any(_pInitialValue, _rType));
    else
        m_aHoldProperties.push_back(Any());
    aNewProp.aType = _rType;

    implPushBackProperty(aNewProp);
}

//--------------------------------------------------------------------------
void OPropertyContainer::implPushBackProperty(const PropertyDescription& _rProp)
{
    // need one more element
    sal_Int32 nOldLen = m_aProperties.size();
    m_aProperties.resize(nOldLen + 1);
    PropertyDescription* pProps = m_aProperties.begin() + nOldLen - 1;

    // search the corect position, shifting the elements to the tail if needed
    sal_Int32 nPos = nOldLen;
    while (nPos && (_rProp.nHandle < pProps->nHandle))
    {
        *(pProps+1) = *pProps;
        --pProps;
        --nPos;
    }

    m_aProperties[nPos] = _rProp;
}

//--------------------------------------------------------------------------
sal_Bool OPropertyContainer::convertFastPropertyValue(
    Any& _rConvertedValue, Any& _rOldValue, sal_Int32 _nHandle, const Any& _rValue ) throw (IllegalArgumentException)
{
#ifdef DBG_UTIL
    m_bAlreadyAccessed = sal_True;
#endif
    sal_Bool bModified = sal_False;

    // get the property somebody is asking for
    PropertiesIterator aPos = searchHandle(_nHandle);
    if (aPos == m_aProperties.end())
    {
        OSL_ASSERT("OPropertyContainer::convertFastPropertyValue : unknown handle !");
        // should not happen if the derived class has built a correct property set info helper to be used by
        // our base class OPropertySetHelper
        return bModified;
    }

    switch (aPos->eLocated)
    {
        // similar handling for the two cases where the value is stored in an any
        case PropertyDescription::ltHoldMyself:
        case PropertyDescription::ltDerivedClassAnyType:
        {
            sal_Bool bMayBeVoid = ((aPos->nAttributes & PropertyAttribute::MAYBEVOID) != 0);
            // argument check
            if  (   !   (   (bMayBeVoid && !_rValue.hasValue())             // void is allowed if the attribute says so
                        ||  (_rValue.getValueType().equals(aPos->aType))    // else the types have to be equal
                        )
                )
                throw IllegalArgumentException(
                    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("invalid value type")),
                    static_cast< XPropertySet* >(this),
                    4);

            Any* pPropContainer = NULL;
                // the pointer to the any which holds the property value, no matter if located in the derived clas
                // or in out vector

            if (PropertyDescription::ltHoldMyself == aPos->eLocated)
            {
                OSL_ENSHURE(aPos->aLocation.nOwnClassVectorIndex < m_aHoldProperties.size(),
                    "OPropertyContainer::convertFastPropertyValue : invalid position !");
                pPropContainer = m_aHoldProperties.begin() + aPos->aLocation.nOwnClassVectorIndex;
            }
            else
                pPropContainer = reinterpret_cast<Any*>(aPos->aLocation.pDerivedClassMember);

            // check if the new value differs from the current one
            if (!pPropContainer->hasValue() || !_rValue.hasValue())
                bModified = pPropContainer->hasValue() != _rValue.hasValue();
            else
                bModified = !uno_type_equalData(const_cast<void*>(pPropContainer->getValue()), aPos->aType.getTypeLibType(), const_cast<void*>(_rValue.getValue()), aPos->aType.getTypeLibType(), cpp_queryInterface, cpp_release);

            if (bModified)
            {
                _rOldValue = *pPropContainer;
                _rConvertedValue = _rValue;
            }
        }
        break;
        case PropertyDescription::ltDerivedClassRealType:
            // here we don't allow MAYBEVOID properties, so the type check is simple
            // (remember that we don't support value conversions, so the value type has to be exactly the one
            // required for the property)
            if (!_rValue.getValueType().equals(aPos->aType))
                throw IllegalArgumentException();

            bModified = !uno_type_equalData(aPos->aLocation.pDerivedClassMember, aPos->aType.getTypeLibType(), const_cast<void*>(_rValue.getValue()), aPos->aType.getTypeLibType(), cpp_queryInterface, cpp_release);
            if (bModified)
            {
                _rOldValue.setValue(aPos->aLocation.pDerivedClassMember, aPos->aType);
                _rConvertedValue = _rValue;
            }
            break;
    }

    return bModified;
}

//--------------------------------------------------------------------------
void OPropertyContainer::setFastPropertyValue_NoBroadcast(sal_Int32 _nHandle, const Any& _rValue) throw (Exception)
{
#ifdef DBG_UTIL
    m_bAlreadyAccessed = sal_True;
#endif
    // get the property somebody is asking for
    PropertiesIterator aPos = searchHandle(_nHandle);
    if (aPos == m_aProperties.end())
    {
        OSL_ASSERT("OPropertyContainer::setFastPropertyValue_NoBroadcast : unknown handle !");
        // should not happen if the derived class has built a correct property set info helper to be used by
        // our base class OPropertySetHelper
        return;
    }

    switch (aPos->eLocated)
    {
        case PropertyDescription::ltHoldMyself:
            m_aHoldProperties[aPos->aLocation.nOwnClassVectorIndex] = _rValue;
            break;
        case PropertyDescription::ltDerivedClassAnyType:
            *reinterpret_cast< Any* >(aPos->aLocation.pDerivedClassMember) = _rValue;
            break;
        case PropertyDescription::ltDerivedClassRealType:
            OSL_ENSHURE(_rValue.getValueType().equals(aPos->aType),
                "OPropertyContainer::setFastPropertyValue_NoBroadcast : ooops .... the value is of the wrong type !");
            uno_type_copyData(aPos->aLocation.pDerivedClassMember, const_cast<void*>(_rValue.getValue()), aPos->aType.getTypeLibType(), cpp_acquire);
            break;
    }
}

//--------------------------------------------------------------------------
void OPropertyContainer::getFastPropertyValue(Any& _rValue, sal_Int32 _nHandle) const
{
#ifdef DBG_UTIL
    const_cast<OPropertyContainer*>(this)->m_bAlreadyAccessed = sal_True;
        // (are we allowed to use the mutable keyword? Means do all our compilers support this meanwhile?)
#endif

    // get the property somebody is asking for
    PropertiesIterator aPos = const_cast<OPropertyContainer*>(this)->searchHandle(_nHandle);
    if (aPos == m_aProperties.end())
    {
        OSL_ASSERT("OPropertyContainer::getFastPropertyValue : unknown handle !");
        // should not happen if the derived class has built a correct property set info helper to be used by
        // our base class OPropertySetHelper
        return;
    }

    switch (aPos->eLocated)
    {
        case PropertyDescription::ltHoldMyself:
            OSL_ENSHURE(aPos->aLocation.nOwnClassVectorIndex < m_aHoldProperties.size(),
                "OPropertyContainer::convertFastPropertyValue : invalid position !");
            _rValue = m_aHoldProperties[aPos->aLocation.nOwnClassVectorIndex];
            break;
        case PropertyDescription::ltDerivedClassAnyType:
            _rValue = *reinterpret_cast<Any*>(aPos->aLocation.pDerivedClassMember);
            break;
        case PropertyDescription::ltDerivedClassRealType:
            _rValue.setValue(aPos->aLocation.pDerivedClassMember, aPos->aType);
            break;
    }
}

//--------------------------------------------------------------------------
OPropertyContainer::PropertiesIterator OPropertyContainer::searchHandle(sal_Int32 _nHandle)
{
    PropertyDescriptionCompareByHandle aCompareOp;
    PropertyDescription aSearch;
    aSearch.nHandle = _nHandle;

    // search a lower bound
    PropertiesIterator aLowerBound = ::std::lower_bound(
        m_aProperties.begin(),
        m_aProperties.end(),
        aSearch,
        aCompareOp);

    // check for identity
    if ((aLowerBound != m_aProperties.end()) && aLowerBound->nHandle != _nHandle)
        aLowerBound = m_aProperties.end();

    return aLowerBound;
}

//--------------------------------------------------------------------------
void OPropertyContainer::modifyAttributes(sal_Int32 _nHandle, sal_Int32 _nAddAttrib, sal_Int32 _nRemoveAttrib)
{
    OSL_ENSHURE(!m_bAlreadyAccessed, "OPropertyContainer::modifyAttributes : invalid call, the property sequence was already accessed !");

    // get the property somebody is asking for
    PropertiesIterator aPos = searchHandle(_nHandle);
    if (aPos == m_aProperties.end())
    {
        OSL_ASSERT("OPropertyContainer::modifyAttributes : invalid handle !");
        // should not happen if the derived class has built a correct property set info helper to be used by
        // our base class OPropertySetHelper
        return;
    }
    aPos->nHandle |= _nAddAttrib;
    aPos->nHandle &= ~_nRemoveAttrib;
}

//--------------------------------------------------------------------------
// comparing two property instances
struct PropertyCompareByName : public ::std::binary_function< Property, Property, sal_Bool >
{
    bool operator() (const Property& x, const Property& y) const
    {
        return x.Name < y.Name;
    }
};

//..........................................................................
void OPropertyContainer::describeProperties(Sequence< Property >& _rProps) const
{
    Sequence< Property > aOwnProps(m_aProperties.size());
    Property* pOwnProps = aOwnProps.getArray();

    for (   ConstPropertiesIterator aLoop = m_aProperties.begin();
            aLoop != m_aProperties.end();
            ++aLoop, ++pOwnProps
        )
    {
        pOwnProps->Name = aLoop->sName;
        pOwnProps->Handle = aLoop->nHandle;
        pOwnProps->Attributes = aLoop->nAttributes;
        pOwnProps->Type = aLoop->aType;
    }

    // as our property vector is sorted by handles, not by name, we have to sort aOwnProps
    qsort(aOwnProps.getArray(), aOwnProps.getLength(), sizeof(Property), PropertyCompare);

    // unfortunally the STL merge function does not allow the output range to overlap one of the input ranges,
    // so we need an extra sequence
    Sequence< Property > aOutput;
    aOutput.realloc(_rProps.getLength() + aOwnProps.getLength());
    // do the merge
    ::std::merge(   _rProps.getConstArray(), _rProps.getConstArray() + _rProps.getLength(),         // input 1
                    aOwnProps.getConstArray(), aOwnProps.getConstArray() + aOwnProps.getLength(),   // input 2
                    aOutput.getArray(),                                                             // output
                    PropertyCompareByName()                                                         // compare operator
                );

    // copy the output
    _rProps = aOutput;
}

//.........................................................................
}   // namespace comphelper
//.........................................................................

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.1  2000/09/21 08:52:22  fs
 *  base class for classes which are simple property containers
 *
 ************************************************************************/

