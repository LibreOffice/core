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

#ifndef BASCTL_DLGEDLIST_HXX
#define BASCTL_DLGEDLIST_HXX

#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/container/XContainerListener.hpp>

namespace basctl
{

class DlgEdObj;

//============================================================================
// DlgEdPropListenerImpl
//============================================================================

typedef ::cppu::WeakImplHelper1< ::com::sun::star::beans::XPropertyChangeListener > PropertyChangeListenerHelper;

class DlgEdPropListenerImpl: public PropertyChangeListenerHelper
{
private:
    DlgEdObj& rDlgEdObj;

public:
    explicit DlgEdPropListenerImpl (DlgEdObj&);
    virtual ~DlgEdPropListenerImpl();

    // XEventListener
    virtual void SAL_CALL disposing( const  ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);

    // XPropertyChangeListener
    virtual void SAL_CALL propertyChange( const  ::com::sun::star::beans::PropertyChangeEvent& evt ) throw(::com::sun::star::uno::RuntimeException);

};

//============================================================================
// DlgEdEvtContListenerImpl
//============================================================================

typedef ::cppu::WeakImplHelper1< ::com::sun::star::container::XContainerListener > ContainerListenerHelper;

class DlgEdEvtContListenerImpl: public ContainerListenerHelper
{
private:
    DlgEdObj& rDlgEdObj;

public:
    explicit DlgEdEvtContListenerImpl (DlgEdObj&);
    virtual ~DlgEdEvtContListenerImpl();

    // XEventListener
    virtual void SAL_CALL disposing( const  ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);

    // XContainerListener
    virtual void SAL_CALL elementInserted( const ::com::sun::star::container::ContainerEvent& Event ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL elementReplaced( const ::com::sun::star::container::ContainerEvent& Event ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL elementRemoved( const ::com::sun::star::container::ContainerEvent& Event ) throw(::com::sun::star::uno::RuntimeException);
};

} // namespace basctl

#endif // BASCTL_DLGEDLIST_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
