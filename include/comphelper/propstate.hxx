/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_COMPHELPER_PROPSTATE_HXX
#define INCLUDED_COMPHELPER_PROPSTATE_HXX

#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/uno/Sequence.hxx>


#include <cppuhelper/propshlp.hxx>
#include <cppuhelper/proptypehlp.hxx>
#include <cppuhelper/weak.hxx>
#include <comphelper/uno3.hxx>
#include <comphelper/broadcasthelper.hxx>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <comphelper/comphelperdllapi.h>


//= property helper classes



namespace comphelper
{



    //= OPropertyStateHelper

    /// helper class for implementing property states
    class COMPHELPER_DLLPUBLIC OPropertyStateHelper :public ::cppu::OPropertySetHelper2
                                                    ,public ::com::sun::star::beans::XPropertyState
    {
    public:
        OPropertyStateHelper(::cppu::OBroadcastHelper& rBHlp):OPropertySetHelper2(rBHlp) { }
        OPropertyStateHelper(::cppu::OBroadcastHelper& rBHlp,
                             ::cppu::IEventNotificationHook *i_pFireEvents);

        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(const ::com::sun::star::uno::Type& aType) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // XPropertyState
        virtual ::com::sun::star::beans::PropertyState SAL_CALL
            getPropertyState(const OUString& PropertyName) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyState> SAL_CALL
            getPropertyStates(const ::com::sun::star::uno::Sequence< OUString >& aPropertyName) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL
            setPropertyToDefault(const OUString& PropertyName) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::com::sun::star::uno::Any SAL_CALL
            getPropertyDefault(const OUString& aPropertyName) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;

    // access via handle
        virtual ::com::sun::star::beans::PropertyState  getPropertyStateByHandle(sal_Int32 nHandle);
        virtual void                                    setPropertyToDefaultByHandle(sal_Int32 nHandle);
        virtual ::com::sun::star::uno::Any              getPropertyDefaultByHandle(sal_Int32 nHandle) const;

    protected:
        virtual ~OPropertyStateHelper();

        void firePropertyChange(sal_Int32 nHandle, const ::com::sun::star::uno::Any& aNewValue, const ::com::sun::star::uno::Any& aOldValue);

        static css::uno::Sequence<css::uno::Type> getTypes();
    };


    //= OPropertyStateHelper

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


}   // namespace comphelper


#endif // INCLUDED_COMPHELPER_PROPSTATE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
