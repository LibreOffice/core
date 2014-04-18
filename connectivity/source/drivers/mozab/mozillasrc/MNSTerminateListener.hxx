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

#ifndef INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_MOZAB_MOZILLASRC_MNSTERMINATELISTENER_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_MOZAB_MOZILLASRC_MNSTERMINATELISTENER_HXX

#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/frame/XTerminateListener.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>

//class to implement the XTerminateListener interface
class MNSTerminateListener : public ::cppu::WeakImplHelper1< ::com::sun::star::frame::XTerminateListener >
{

private:

    // XEventListener
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw( ::com::sun::star::uno::RuntimeException );

    // XTerminateListener
    virtual void SAL_CALL queryTermination( const ::com::sun::star::lang::EventObject& aEvent ) throw( ::com::sun::star::frame::TerminationVetoException, ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL notifyTermination( const ::com::sun::star::lang::EventObject& aEvent ) throw( ::com::sun::star::uno::RuntimeException );


public:
    MNSTerminateListener();
    virtual  ~MNSTerminateListener();
public:
    static ::com::sun::star::uno::Reference< ::com::sun::star::frame::XTerminateListener>          mxTerminateListener;
    static void addTerminateListener();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
