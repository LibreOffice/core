/*************************************************************************
 *
 *  $RCSfile: propstate.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: oj $ $Date: 2000-11-03 14:25:29 $
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
 ::com::sun::star::uno::Any SAL_CALL OPropertyStateHelper::queryInterface(const  ::com::sun::star::uno::Type& _rType) throw( ::com::sun::star::uno::RuntimeException)
{
     ::com::sun::star::uno::Any aReturn;
    // ask the base class
    aReturn = OPropertySetHelper::queryInterface(_rType);
    // our own ifaces
    if (!aReturn.hasValue())
        aReturn = ::cppu::queryInterface(_rType, static_cast< ::com::sun::star::beans::XPropertyState*>(this));

    return aReturn;
}

//------------------------------------------------------------------------------
 ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type> OPropertyStateHelper::getTypes() throw( ::com::sun::star::uno::RuntimeException)
{
    static  ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type> aTypes;
    if (!aTypes.getLength())
    {
        aTypes.realloc(4);
         ::com::sun::star::uno::Type* pTypes = aTypes.getArray();
        // base class types
        pTypes[0] = getCppuType(( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>*)NULL);
        pTypes[1] = getCppuType(( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XMultiPropertySet>*)NULL);
        pTypes[2] = getCppuType(( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XFastPropertySet>*)NULL);
        // my own type
        pTypes[3] = getCppuType(( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyState>*)NULL);
    }
    return aTypes;
}

//------------------------------------------------------------------------------
void OPropertyStateHelper::firePropertyChange(sal_Int32 nHandle, const  ::com::sun::star::uno::Any& aNewValue, const  ::com::sun::star::uno::Any& aOldValue)
{
    fire(&nHandle, &aNewValue, &aOldValue, 1, sal_False);
}

// XPropertyState
//------------------------------------------------------------------------------
 ::com::sun::star::beans::PropertyState SAL_CALL OPropertyStateHelper::getPropertyState(const ::rtl::OUString& _rsName) throw( ::com::sun::star::beans::UnknownPropertyException,  ::com::sun::star::uno::RuntimeException)
{
    cppu::IPropertyArrayHelper& rPH = getInfoHelper();
    sal_Int32 nHandle = rPH.getHandleByName(_rsName);

    if (nHandle == -1)
        throw  ::com::sun::star::beans::UnknownPropertyException();

    return getPropertyStateByHandle(nHandle);
}

//------------------------------------------------------------------------------
void SAL_CALL OPropertyStateHelper::setPropertyToDefault(const ::rtl::OUString& _rsName) throw( ::com::sun::star::beans::UnknownPropertyException,  ::com::sun::star::uno::RuntimeException)
{
    cppu::IPropertyArrayHelper& rPH = getInfoHelper();
    sal_Int32 nHandle = rPH.getHandleByName(_rsName);

    if (nHandle == -1)
        throw  ::com::sun::star::beans::UnknownPropertyException();

    setPropertyToDefaultByHandle(nHandle);
}

//------------------------------------------------------------------------------
 ::com::sun::star::uno::Any SAL_CALL OPropertyStateHelper::getPropertyDefault(const ::rtl::OUString& _rsName) throw( ::com::sun::star::beans::UnknownPropertyException,  ::com::sun::star::lang::WrappedTargetException,  ::com::sun::star::uno::RuntimeException)
{
    cppu::IPropertyArrayHelper& rPH = getInfoHelper();
    sal_Int32 nHandle = rPH.getHandleByName(_rsName);

    if (nHandle == -1)
        throw  ::com::sun::star::beans::UnknownPropertyException();

    return getPropertyDefaultByHandle(nHandle);
}

//------------------------------------------------------------------------------
 ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyState> SAL_CALL OPropertyStateHelper::getPropertyStates(const  ::com::sun::star::uno::Sequence< ::rtl::OUString >& _rPropertyNames) throw( ::com::sun::star::beans::UnknownPropertyException,  ::com::sun::star::uno::RuntimeException)
{
    sal_Int32 nLen = _rPropertyNames.getLength();
     ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyState> aRet(nLen);
     ::com::sun::star::beans::PropertyState* pValues = aRet.getArray();
    const ::rtl::OUString* pNames = _rPropertyNames.getConstArray();

    cppu::IPropertyArrayHelper& rHelper = getInfoHelper();

     ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property> aProps = rHelper.getProperties();
    const  ::com::sun::star::beans::Property* pProps = aProps.getConstArray();
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
 ::com::sun::star::beans::PropertyState OPropertyStateHelper::getPropertyStateByHandle(sal_Int32 nHandle)
{
    return  ::com::sun::star::beans::PropertyState_DIRECT_VALUE;
}

//------------------------------------------------------------------------------
void OPropertyStateHelper::setPropertyToDefaultByHandle(sal_Int32 nHandle)
{
}

//------------------------------------------------------------------------------
 ::com::sun::star::uno::Any OPropertyStateHelper::getPropertyDefaultByHandle( sal_Int32 nHandle ) const
{
    return  ::com::sun::star::uno::Any();
}

//.........................................................................
}
//.........................................................................

