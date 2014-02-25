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

#include "DrawModelBroadcaster.hxx"
#include <osl/diagnose.h>
#include <rtl/strbuf.hxx>
#include <svx/svdmodel.hxx>
#include <svx/unomod.hxx>

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
    throw (uno::RuntimeException, std::exception)
{
    maEventListeners.addInterface( xListener );
}

void SAL_CALL ScDrawModelBroadcaster::removeEventListener( const uno::Reference< document::XEventListener >& xListener )
    throw (uno::RuntimeException, std::exception)
{
    maEventListeners.removeInterface( xListener );
}

void ScDrawModelBroadcaster::Notify( SfxBroadcaster&,
        const SfxHint& rHint )
{
    const SdrHint *pSdrHint = PTR_CAST( SdrHint, &rHint );
    if( !pSdrHint )
        return;

    document::EventObject aEvent;
    if( !SvxUnoDrawMSFactory::createEvent( mpDrawModel, pSdrHint, aEvent ) )
        return;

    ::cppu::OInterfaceIteratorHelper aIter( maEventListeners );
    while( aIter.hasMoreElements() )
    {
        uno::Reference < document::XEventListener > xListener( aIter.next(), uno::UNO_QUERY );
        try
        {
            xListener->notifyEvent( aEvent );
        }
        catch( const uno::RuntimeException& r )
        {
            (void) r;
#if OSL_DEBUG_LEVEL > 1
            OStringBuffer aError("Runtime exception caught while notifying shape.:\n");
            aError.append(OUStringToOString(r.Message, RTL_TEXTENCODING_ASCII_US));
            OSL_FAIL( aError.getStr() );
#endif
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
