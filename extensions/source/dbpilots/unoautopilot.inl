/*************************************************************************
 *
 *  $RCSfile: unoautopilot.inl,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2001-03-06 10:06:40 $
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
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.1  2001/02/21 09:24:52  fs
 *  initial checkin - form control auto pilots
 *
 *
 *  Revision 1.0 14.02.01 10:18:07  fs
 ************************************************************************/

