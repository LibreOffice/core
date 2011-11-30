/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
    return new TYPE(_pParent, m_xObjectModel, m_aContext.getLegacyServiceFactory());
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


