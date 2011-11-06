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



#ifndef _COMPHELPER_PROPERTY_STATE_HXX_
#define _COMPHELPER_PROPERTY_STATE_HXX_

#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/uno/Sequence.hxx>


#include <cppuhelper/propshlp.hxx>
#include <cppuhelper/proptypehlp.hxx>
#include <cppuhelper/weak.hxx>
#include <comphelper/uno3.hxx>
#include <comphelper/broadcasthelper.hxx>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include "comphelper/comphelperdllapi.h"

//=========================================================================
//= property helper classes
//=========================================================================

//.........................................................................
namespace comphelper
{
//.........................................................................

    //==================================================================
    //= OPropertyStateHelper
    //==================================================================
    /// helper class for implementing property states
    class COMPHELPER_DLLPUBLIC OPropertyStateHelper :public ::cppu::OPropertySetHelper
                                                    ,public ::com::sun::star::beans::XPropertyState
    {
    public:
        OPropertyStateHelper(::cppu::OBroadcastHelper& rBHlp):OPropertySetHelper(rBHlp) { }
        OPropertyStateHelper(::cppu::OBroadcastHelper& rBHlp,
                             ::cppu::IEventNotificationHook *i_pFireEvents);

        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(const ::com::sun::star::uno::Type& aType) throw(::com::sun::star::uno::RuntimeException);

    // XPropertyState
        virtual ::com::sun::star::beans::PropertyState SAL_CALL
            getPropertyState(const ::rtl::OUString& PropertyName) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyState> SAL_CALL
            getPropertyStates(const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyName) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL
            setPropertyToDefault(const ::rtl::OUString& PropertyName) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Any SAL_CALL
            getPropertyDefault(const ::rtl::OUString& aPropertyName) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // access via handle
        virtual ::com::sun::star::beans::PropertyState  getPropertyStateByHandle(sal_Int32 nHandle);
        virtual void                                    setPropertyToDefaultByHandle(sal_Int32 nHandle);
        virtual ::com::sun::star::uno::Any              getPropertyDefaultByHandle(sal_Int32 nHandle) const;

    protected:
        virtual ~OPropertyStateHelper();

        void firePropertyChange(sal_Int32 nHandle, const ::com::sun::star::uno::Any& aNewValue, const ::com::sun::star::uno::Any& aOldValue);

    protected:
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type> SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException);
    };

    //==================================================================
    //= OPropertyStateHelper
    //==================================================================
    class COMPHELPER_DLLPUBLIC OStatefulPropertySet  :public ::cppu::OWeakObject
                                ,public ::com::sun::star::lang::XTypeProvider
                                ,public OMutexAndBroadcastHelper    // order matters: before OPropertyStateHelper/OPropertySetHelper
                                ,public OPropertyStateHelper
    {
    protected:
        OStatefulPropertySet();
        virtual ~OStatefulPropertySet();

    protected:
        DECLARE_XINTERFACE()
        DECLARE_XTYPEPROVIDER()
    };

//.........................................................................
}   // namespace comphelper
//.........................................................................

#endif // _COMPHELPER_PROPERTY_STATE_HXX_

