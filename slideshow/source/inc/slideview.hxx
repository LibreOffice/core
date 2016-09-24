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

#ifndef INCLUDED_SLIDESHOW_SOURCE_INC_SLIDEVIEW_HXX
#define INCLUDED_SLIDESHOW_SOURCE_INC_SLIDEVIEW_HXX

#include "unoview.hxx"

/* Definition of SlideView factory method */
namespace slideshow
{
    namespace internal
    {
        class EventQueue;
        class EventMultiplexer;

        /** Factory for SlideView

            @param xView
            UNO slide view this object should encapsulate

            @param rEventQueue
            Global event queue, to be used for notification
            messages.

            @param rViewChangeFunc
            Functor to call, when the UNO view signals a repaint.
        */
        UnoViewSharedPtr createSlideView(
            css::uno::Reference< css::presentation::XSlideShowView> const& xView,
            EventQueue&                                                rEventQueue,
            EventMultiplexer&                                          rEventMultiplexer );
    }
}

#endif // INCLUDED_SLIDESHOW_SOURCE_INC_SLIDEVIEW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
