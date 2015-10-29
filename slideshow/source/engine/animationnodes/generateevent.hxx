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

#ifndef INCLUDED_SLIDESHOW_SOURCE_ENGINE_ANIMATIONNODES_GENERATEEVENT_HXX
#define INCLUDED_SLIDESHOW_SOURCE_ENGINE_ANIMATIONNODES_GENERATEEVENT_HXX

#include "slideshowcontext.hxx"
#include "delayevent.hxx"
#include "com/sun/star/uno/Any.hxx"

namespace slideshow {
namespace internal {

/** Create an event for the given description, calling the given functor.

    @param rEventDescription
    Directly from API

    @param rFunctor
    Functor to call when event fires.

    @param rContext
    Context struct, to provide event queue

    @param nAdditionalDelay
    Additional delay, gets added on top of timeout.
*/
EventSharedPtr generateEvent(
    css::uno::Any const& rEventDescription,
    Delay::FunctorT const& rFunctor,
    SlideShowContext const& rContext,
    double nAdditionalDelay );

} // namespace internal
} // namespace slideshow

#endif // INCLUDED_SLIDESHOW_SOURCE_ENGINE_ANIMATIONNODES_GENERATEEVENT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
