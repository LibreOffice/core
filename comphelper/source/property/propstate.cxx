/*************************************************************************
 *
 *  $RCSfile: propstate.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: fs $ $Date: 2000-09-29 11:28:15 $
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

#ifndef _COMPHELPER_PROPERTY_STATE_HXX_
#include <comphelper/propstate.hxx>
#endif

#ifndef _CPPUHELPER_QUERYINTERFACE_HXX_
#include <cppuhelper/queryinterface.hxx>
#endif

//.........................................................................
namespace comphelper
{

//==============================================================================
// OPropertyStateHelper
//==============================================================================

//------------------------------------------------------------------------------
staruno::Any SAL_CALL OPropertyStateHelper::queryInterface(const staruno::Type& _rType) throw(staruno::RuntimeException)
{
    staruno::Any aReturn;
    // ask the base class
    aReturn = OPropertySetHelper::queryInterface(_rType);
    // our own ifaces
    if (!aReturn.hasValue())
        aReturn = ::cppu::queryInterface(_rType, static_cast<starbeans::XPropertyState*>(this));

    return aReturn;
}

//------------------------------------------------------------------------------
staruno::Sequence<staruno::Type> OPropertyStateHelper::getTypes() throw(staruno::RuntimeException)
{
    static staruno::Sequence<staruno::Type> aTypes;
    if (!aTypes.getLength())
    {
        aTypes.realloc(4);
        staruno::Type* pTypes = aTypes.getArray();
        // base class types
        pTypes[0] = getCppuType((staruno::Reference<starbeans::XPropertySet>*)NULL);
        pTypes[1] = getCppuType((staruno::Reference<starbeans::XMultiPropertySet>*)NULL);
        pTypes[2] = getCppuType((staruno::Reference<starbeans::XFastPropertySet>*)NULL);
        // my own type
        pTypes[3] = getCppuType((staruno::Reference<starbeans::XPropertyState>*)NULL);
    }
    return aTypes;
}

//------------------------------------------------------------------------------
void OPropertyStateHelper::firePropertyChange(sal_Int32 nHandle, const staruno::Any& aNewValue, const staruno::Any& aOldValue)
{
    fire(&nHandle, &aNewValue, &aOldValue, 1, sal_False);
}

// XPropertyState
//------------------------------------------------------------------------------
starbeans::PropertyState SAL_CALL OPropertyStateHelper::getPropertyState(const ::rtl::OUString& _rsName) throw(starbeans::UnknownPropertyException, staruno::RuntimeException)
{
    cppu::IPropertyArrayHelper& rPH = getInfoHelper();
    sal_Int32 nHandle = rPH.getHandleByName(_rsName);

    if (nHandle == -1)
        throw starbeans::UnknownPropertyException();

    return getPropertyStateByHandle(nHandle);
}

//------------------------------------------------------------------------------
void SAL_CALL OPropertyStateHelper::setPropertyToDefault(const ::rtl::OUString& _rsName) throw(starbeans::UnknownPropertyException, staruno::RuntimeException)
{
    cppu::IPropertyArrayHelper& rPH = getInfoHelper();
    sal_Int32 nHandle = rPH.getHandleByName(_rsName);

    if (nHandle == -1)
        throw starbeans::UnknownPropertyException();

    setPropertyToDefaultByHandle(nHandle);
}

//------------------------------------------------------------------------------
staruno::Any SAL_CALL OPropertyStateHelper::getPropertyDefault(const ::rtl::OUString& _rsName) throw(starbeans::UnknownPropertyException, starlang::WrappedTargetException, staruno::RuntimeException)
{
    cppu::IPropertyArrayHelper& rPH = getInfoHelper();
    sal_Int32 nHandle = rPH.getHandleByName(_rsName);

    if (nHandle == -1)
        throw starbeans::UnknownPropertyException();

    return getPropertyDefaultByHandle(nHandle);
}

//------------------------------------------------------------------------------
staruno::Sequence<starbeans::PropertyState> SAL_CALL OPropertyStateHelper::getPropertyStates(const staruno::Sequence< ::rtl::OUString >& _rPropertyNames) throw(starbeans::UnknownPropertyException, staruno::RuntimeException)
{
    sal_Int32 nLen = _rPropertyNames.getLength();
    staruno::Sequence<starbeans::PropertyState> aRet(nLen);
    starbeans::PropertyState* pValues = aRet.getArray();
    const ::rtl::OUString* pNames = _rPropertyNames.getConstArray();

    cppu::IPropertyArrayHelper& rHelper = getInfoHelper();

    staruno::Sequence<starbeans::Property> aProps = rHelper.getProperties();
    const starbeans::Property* pProps = aProps.getConstArray();
    sal_Int32 nPropCount       = aProps.getLength();

    osl::MutexGuard aGuard(rBHelper.rMutex);
    for (sal_Int32 i=0, j=0; i<nPropCount && j<nLen; ++i, ++pProps)
    {
        // get the values only for valid properties
        if (pProps->Name.equals(*pNames))
        {
            *pValues = getPropertyState(*pNames);
            ++pValues;
            ++pNames;
            ++j;
        }
    }

    return aRet;
}

//------------------------------------------------------------------------------
starbeans::PropertyState OPropertyStateHelper::getPropertyStateByHandle(sal_Int32 nHandle)
{
    return starbeans::PropertyState_DIRECT_VALUE;
}

//------------------------------------------------------------------------------
void OPropertyStateHelper::setPropertyToDefaultByHandle(sal_Int32 nHandle)
{
}

//------------------------------------------------------------------------------
staruno::Any OPropertyStateHelper::getPropertyDefaultByHandle( sal_Int32 nHandle ) const
{
    return staruno::Any();
}

//.........................................................................
}
//.........................................................................

