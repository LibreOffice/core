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

#ifndef INCLUDED_EXAMPLES_COMPLEXTOOLBARCONTROLS_LISTENERHELPER_H
#define INCLUDED_EXAMPLES_COMPLEXTOOLBARCONTROLS_LISTENERHELPER_H

#include <map>
#include <vector>

#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <com/sun/star/frame/XDispatch.hpp>

#include <rtl/ustring.hxx>
#include <cppuhelper/implbase.hxx>

typedef std::vector < com::sun::star::uno::Reference < com::sun::star::frame::XStatusListener > > StatusListeners;

typedef std::map < ::rtl::OUString, StatusListeners > ListenerMap;

// For every frame there is *one* Dispatch object for all possible commands
// this struct contains an array of listeners for every supported command
// these arrays are accessed by a std::map (with the command string as index)
struct ListenerItem
{
    ListenerMap aContainer;
    ::com::sun::star::uno::Reference< com::sun::star::frame::XDispatch > xDispatch;
    ::com::sun::star::uno::Reference< com::sun::star::frame::XFrame > xFrame;
};

typedef std::vector < ListenerItem > AllListeners;

class ListenerHelper
{
public:
    void AddListener(
        const com::sun::star::uno::Reference < com::sun::star::frame::XFrame >& xFrame,
        const com::sun::star::uno::Reference < com::sun::star::frame::XStatusListener > xControl,
        const ::rtl::OUString& aCommand );
    void RemoveListener(
        const com::sun::star::uno::Reference < com::sun::star::frame::XFrame >& xFrame,
        const com::sun::star::uno::Reference < com::sun::star::frame::XStatusListener > xControl,
        const ::rtl::OUString& aCommand );
    void Notify(
        const com::sun::star::uno::Reference < com::sun::star::frame::XFrame >& xFrame,
        const ::rtl::OUString& aCommand,
        com::sun::star::frame::FeatureStateEvent& rEvent );
    com::sun::star::uno::Reference < com::sun::star::frame::XDispatch > GetDispatch(
        const com::sun::star::uno::Reference < com::sun::star::frame::XFrame >& xFrame,
        const ::rtl::OUString& aCommand );
    void AddDispatch(
        const com::sun::star::uno::Reference < com::sun::star::frame::XDispatch > xDispatch,
        const com::sun::star::uno::Reference < com::sun::star::frame::XFrame >& xFrame,
        const ::rtl::OUString& aCommand );
};

class ListenerItemEventListener : public cppu::WeakImplHelper < ::com::sun::star::lang::XEventListener >
{
    ::com::sun::star::uno::Reference< com::sun::star::frame::XFrame > mxFrame;
public:
    ListenerItemEventListener( const com::sun::star::uno::Reference < com::sun::star::frame::XFrame >& xFrame)
        : mxFrame(xFrame)
    {}
    virtual void SAL_CALL disposing( const com::sun::star::lang::EventObject& aEvent ) throw (com::sun::star::uno::RuntimeException);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
