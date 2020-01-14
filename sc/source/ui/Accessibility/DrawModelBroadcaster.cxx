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

#include <DrawModelBroadcaster.hxx>
#include <svx/svdmodel.hxx>
#include <svx/unomod.hxx>
#include <svx/svdobj.hxx>
#include <tools/diagnose_ex.h>

using namespace ::com::sun::star;

ScDrawModelBroadcaster::ScDrawModelBroadcaster( SdrModel *pDrawModel ) :
    maEventListeners( maListenerMutex ),
    mpDrawModel( pDrawModel )
{
    if (mpDrawModel)
        StartListening( *mpDrawModel );
}

ScDrawModelBroadcaster::~ScDrawModelBroadcaster()
{
    if (mpDrawModel)
        EndListening( *mpDrawModel );
}

void SAL_CALL ScDrawModelBroadcaster::addEventListener( const uno::Reference< document::XEventListener >& xListener )
{
    maEventListeners.addInterface( xListener );
}

void SAL_CALL ScDrawModelBroadcaster::removeEventListener( const uno::Reference< document::XEventListener >& xListener )
{
    maEventListeners.removeInterface( xListener );
}

void SAL_CALL ScDrawModelBroadcaster::addShapeEventListener(
                const css::uno::Reference< css::drawing::XShape >& xShape,
                const uno::Reference< document::XShapeEventListener >& xListener )
{
    assert(xShape.is() && "no shape?");
    osl::MutexGuard aGuard(maListenerMutex);
    auto rv = maShapeListeners.emplace(xShape, xListener);
    assert(rv.second && "duplicate listener?");
    (void)rv;
}

void SAL_CALL ScDrawModelBroadcaster::removeShapeEventListener(
                const css::uno::Reference< css::drawing::XShape >& xShape,
                const uno::Reference< document::XShapeEventListener >& xListener )
{
    osl::MutexGuard aGuard(maListenerMutex);
    auto it = maShapeListeners.find(xShape);
    if (it != maShapeListeners.end())
    {
        assert(it->second == xListener && "removing wrong listener?");
        (void)xListener;
        maShapeListeners.erase(it);
    }
}

void ScDrawModelBroadcaster::Notify( SfxBroadcaster&,
        const SfxHint& rHint )
{
    if (rHint.GetId() != SfxHintId::ThisIsAnSdrHint)
        return;
    const SdrHint* pSdrHint = static_cast<const SdrHint*>(&rHint);

    document::EventObject aEvent;
    if( !SvxUnoDrawMSFactory::createEvent( mpDrawModel, pSdrHint, aEvent ) )
        return;

    ::comphelper::OInterfaceIteratorHelper3 aIter( maEventListeners );
    while( aIter.hasMoreElements() )
    {
        const uno::Reference < document::XEventListener >& xListener = aIter.next();
        try
        {
            xListener->notifyEvent( aEvent );
        }
        catch( const uno::RuntimeException& )
        {
            TOOLS_WARN_EXCEPTION("sc.ui", "Runtime exception caught while notifying shape");
        }
    }

    // right now, we're only handling the specific event necessary to fix this performance problem
    if (pSdrHint->GetKind() == SdrHintKind::ObjectChange)
    {
        auto pSdrObject = const_cast<SdrObject*>(pSdrHint->GetObject());
        uno::Reference<drawing::XShape> xShape(pSdrObject->getUnoShape(), uno::UNO_QUERY);
        osl::MutexGuard aGuard(maListenerMutex);
        auto it = maShapeListeners.find(xShape);
        if (it != maShapeListeners.end())
            it->second->notifyShapeEvent(aEvent);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
