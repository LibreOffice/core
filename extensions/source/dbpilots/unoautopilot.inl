/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: unoautopilot.inl,v $
 * $Revision: 1.5 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// no include protecttion
// this file is included from unoautopilot.hxx directly

//using namespace ::com::sun::star::uno;
//using namespace ::com::sun::star::lang;
//using namespace ::com::sun::star::beans;
//
//=====================================================================
//= OUnoAutoPilot
//=====================================================================
//---------------------------------------------------------------------
template <class TYPE, class SERVICEINFO>
OUnoAutoPilot<TYPE, SERVICEINFO>::OUnoAutoPilot(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB)
    :OUnoAutoPilot_Base(_rxORB)
{
}

//---------------------------------------------------------------------
template <class TYPE, class SERVICEINFO>
::com::sun::star::uno::Sequence<sal_Int8> SAL_CALL OUnoAutoPilot<TYPE, SERVICEINFO>::getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException)
{
    static ::cppu::OImplementationId aId;
    return aId.getImplementationId();
}

//---------------------------------------------------------------------
template <class TYPE, class SERVICEINFO>
::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL OUnoAutoPilot<TYPE, SERVICEINFO>::Create(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory)
{
    return *(new OUnoAutoPilot<TYPE, SERVICEINFO>(_rxFactory));
}

//---------------------------------------------------------------------
template <class TYPE, class SERVICEINFO>
::rtl::OUString SAL_CALL OUnoAutoPilot<TYPE, SERVICEINFO>::getImplementationName() throw(::com::sun::star::uno::RuntimeException)
{
    return getImplementationName_Static();
}

//---------------------------------------------------------------------
template <class TYPE, class SERVICEINFO>
::rtl::OUString OUnoAutoPilot<TYPE, SERVICEINFO>::getImplementationName_Static() throw(::com::sun::star::uno::RuntimeException)
{
    return SERVICEINFO().getImplementationName();
}

//---------------------------------------------------------------------
template <class TYPE, class SERVICEINFO>
::comphelper::StringSequence SAL_CALL OUnoAutoPilot<TYPE, SERVICEINFO>::getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException)
{
    return getSupportedServiceNames_Static();
}

//---------------------------------------------------------------------
template <class TYPE, class SERVICEINFO>
::comphelper::StringSequence OUnoAutoPilot<TYPE, SERVICEINFO>::getSupportedServiceNames_Static() throw(::com::sun::star::uno::RuntimeException)
{
    return SERVICEINFO().getServiceNames();
}

//---------------------------------------------------------------------
template <class TYPE, class SERVICEINFO>
::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >  SAL_CALL OUnoAutoPilot<TYPE, SERVICEINFO>::getPropertySetInfo() throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >  xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

//---------------------------------------------------------------------
template <class TYPE, class SERVICEINFO>
::cppu::IPropertyArrayHelper& OUnoAutoPilot<TYPE, SERVICEINFO>::getInfoHelper()
{
    return *const_cast<OUnoAutoPilot*>(this)->getArrayHelper();
}

//--------------------------------------------------------------------------
template <class TYPE, class SERVICEINFO>
::cppu::IPropertyArrayHelper* OUnoAutoPilot<TYPE, SERVICEINFO>::createArrayHelper( ) const
{
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property > aProps;
    describeProperties(aProps);
    return new ::cppu::OPropertyArrayHelper(aProps);
}

//--------------------------------------------------------------------------
template <class TYPE, class SERVICEINFO>
Dialog* OUnoAutoPilot<TYPE, SERVICEINFO>::createDialog(Window* _pParent)
{
    return new TYPE(_pParent, m_xObjectModel, m_xORB);
}

//--------------------------------------------------------------------------
template <class TYPE, class SERVICEINFO>
void OUnoAutoPilot<TYPE, SERVICEINFO>::implInitialize(const com::sun::star::uno::Any& _rValue)
{
    ::com::sun::star::beans::PropertyValue aArgument;
    if (_rValue >>= aArgument)
        if (0 == aArgument.Name.compareToAscii("ObjectModel"))
        {
            aArgument.Value >>= m_xObjectModel;
            return;
        }

    OUnoAutoPilot_Base::implInitialize(_rValue);
}


