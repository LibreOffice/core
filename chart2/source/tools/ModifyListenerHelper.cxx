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

#include <ModifyListenerHelper.hxx>

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;

namespace chart
{

ModifyEventForwarder::ModifyEventForwarder()
{
}

// ____ XModifyBroadcaster ____
void SAL_CALL ModifyEventForwarder::addModifyListener( const Reference< util::XModifyListener >& aListener )
{
    std::unique_lock aGuard(m_aMutex);

    m_aModifyListeners.addInterface( aGuard, aListener );
}

void SAL_CALL ModifyEventForwarder::removeModifyListener( const Reference< util::XModifyListener >& aListener )
{
    std::unique_lock aGuard(m_aMutex);

    m_aModifyListeners.removeInterface( aGuard, aListener );
}

// ____ XModifyListener ____
void SAL_CALL ModifyEventForwarder::modified( const lang::EventObject& aEvent )
{
    std::unique_lock aGuard(m_aMutex);

    if( m_aModifyListeners.getLength(aGuard) == 0 )
        return;

    m_aModifyListeners.notifyEach( aGuard, &util::XModifyListener::modified, aEvent );
}

// ____ XEventListener (base of XModifyListener) ____
void SAL_CALL ModifyEventForwarder::disposing( const lang::EventObject& /* Source */ )
{
    // nothing
}

// ____ WeakComponentImplHelperBase ____
void ModifyEventForwarder::disposing(std::unique_lock<std::mutex>& rGuard)
{
    // dispose was called at this
    m_aModifyListeners.disposeAndClear( rGuard, lang::EventObject( static_cast<cppu::OWeakObject*>(this) ) );
}

} //  namespace chart::ModifyListenerHelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
