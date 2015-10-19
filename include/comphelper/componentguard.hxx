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

#ifndef INCLUDED_COMPHELPER_COMPONENTGUARD_HXX
#define INCLUDED_COMPHELPER_COMPONENTGUARD_HXX

#include <com/sun/star/lang/DisposedException.hpp>

#include <cppuhelper/weak.hxx>
#include <cppuhelper/interfacecontainer.hxx>


namespace comphelper
{



    //= ComponentGuard

    class ComponentGuard
    {
    public:
        ComponentGuard( ::cppu::OWeakObject& i_component, ::cppu::OBroadcastHelper & i_broadcastHelper )
            :m_aGuard( i_broadcastHelper.rMutex )
        {
            if ( i_broadcastHelper.bDisposed )
                throw css::lang::DisposedException( OUString(), &i_component );
        }

        ~ComponentGuard()
        {
        }

        void clear()    { m_aGuard.clear(); }
        void reset()    { m_aGuard.reset(); }

    private:
        ::osl::ResettableMutexGuard m_aGuard;
    };


} // namespace comphelper


#endif // INCLUDED_COMPHELPER_COMPONENTGUARD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
