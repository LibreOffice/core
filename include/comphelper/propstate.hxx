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

#include <config_options.h>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/uno/Sequence.h>


#include <cppuhelper/propshlp.hxx>
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
    class UNLESS_MERGELIBS(COMPHELPER_DLLPUBLIC) OPropertyStateHelper :public ::cppu::OPropertySetHelper2
                                                    ,public css::beans::XPropertyState
    {
    public:
        OPropertyStateHelper(::cppu::OBroadcastHelper& rBHlp):OPropertySetHelper2(rBHlp) { }
        OPropertyStateHelper(::cppu::OBroadcastHelper& rBHlp,
                             ::cppu::IEventNotificationHook *i_pFireEvents);

        virtual css::uno::Any SAL_CALL queryInterface(const css::uno::Type& aType) override;

    // XPropertyState
        virtual css::beans::PropertyState SAL_CALL
            getPropertyState(const OUString& PropertyName) override;
        virtual css::uno::Sequence< css::beans::PropertyState> SAL_CALL
            getPropertyStates(const css::uno::Sequence< OUString >& aPropertyName) override final;
        virtual void SAL_CALL
            setPropertyToDefault(const OUString& PropertyName) override;
        virtual css::uno::Any SAL_CALL
            getPropertyDefault(const OUString& aPropertyName) override;

    // access via handle
        virtual css::beans::PropertyState  getPropertyStateByHandle(sal_Int32 nHandle);
        virtual void                                    setPropertyToDefaultByHandle(sal_Int32 nHandle);
        virtual css::uno::Any              getPropertyDefaultByHandle(sal_Int32 nHandle) const;

    protected:
        virtual ~OPropertyStateHelper() override;

        void firePropertyChange(sal_Int32 nHandle, const css::uno::Any& aNewValue, const css::uno::Any& aOldValue);

        static css::uno::Sequence<css::uno::Type> getTypes();
    };


    //= OPropertyStateHelper

    class UNLESS_MERGELIBS(COMPHELPER_DLLPUBLIC) OStatefulPropertySet  :public ::cppu::OWeakObject
                                ,public css::lang::XTypeProvider
                                ,public OMutexAndBroadcastHelper    // order matters: before OPropertyStateHelper/OPropertySetHelper
                                ,public OPropertyStateHelper
    {
    protected:
        OStatefulPropertySet();
        virtual ~OStatefulPropertySet() override;

    protected:
        DECLARE_XINTERFACE()
        DECLARE_XTYPEPROVIDER()
    };


}   // namespace comphelper


#endif // INCLUDED_COMPHELPER_PROPSTATE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
