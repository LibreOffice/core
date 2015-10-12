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
#ifndef INCLUDED_COMPHELPER_EVTLISTENERHLP_HXX
#define INCLUDED_COMPHELPER_EVTLISTENERHLP_HXX

#include <cppuhelper/implbase.hxx>
#include <com/sun/star/lang/XEventListener.hpp>
#include <osl/diagnose.h>
#include <cppuhelper/weakref.hxx>
#include <comphelper/comphelperdllapi.h>


namespace comphelper
{



    //= OCommandsListener
    // is helper class to avoid a cycle in refcount between the XEventListener
    // and the member XComponent

    class COMPHELPER_DLLPUBLIC OEventListenerHelper : public ::cppu::WeakImplHelper< ::com::sun::star::lang::XEventListener >
    {
        ::com::sun::star::uno::WeakReference< ::com::sun::star::lang::XEventListener> m_xListener;
    public:
        OEventListenerHelper(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener>& _rxListener);
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    };

}   // namespace comphelper

#endif // INCLUDED_COMPHELPER_EVTLISTENERHLP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
