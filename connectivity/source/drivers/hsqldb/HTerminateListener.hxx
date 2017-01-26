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
#ifndef INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_HSQLDB_HTERMINATELISTENER_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_HSQLDB_HTERMINATELISTENER_HXX

#include <cppuhelper/implbase.hxx>
#include <com/sun/star/frame/XTerminateListener.hpp>


namespace connectivity
{


    namespace hsqldb
    {
        class ODriverDelegator;
        class OConnectionController : public ::cppu::WeakImplHelper< css::frame::XTerminateListener >
        {
            ODriverDelegator* m_pDriver;
            protected:
                virtual ~OConnectionController() override {m_pDriver = nullptr;}
            public:
                explicit OConnectionController(ODriverDelegator* _pDriver) : m_pDriver(_pDriver){}

                // XEventListener
                virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;

                // XTerminateListener
                virtual void SAL_CALL queryTermination( const css::lang::EventObject& aEvent ) override;
                virtual void SAL_CALL notifyTermination( const css::lang::EventObject& aEvent ) override;
        };
    }

}   // namespace connectivity

#endif // INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_HSQLDB_HTERMINATELISTENER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
