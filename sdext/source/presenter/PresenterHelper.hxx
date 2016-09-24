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

#ifndef INCLUDED_SDEXT_SOURCE_PRESENTER_PRESENTERHELPER_HXX
#define INCLUDED_SDEXT_SOURCE_PRESENTER_PRESENTERHELPER_HXX

#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/presentation/XSlideShowController.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

namespace sdext { namespace presenter {

/** Collection of helper functions that do not fit in anywhere else.
    Provide access to frequently used strings of the drawing framework.
*/
namespace PresenterHelper
{
    extern const OUString msPaneURLPrefix;
    extern const OUString msCenterPaneURL;
    extern const OUString msFullScreenPaneURL;

    extern const OUString msViewURLPrefix;
    extern const OUString msPresenterScreenURL;
    extern const OUString msSlideSorterURL;

    extern const OUString msResourceActivationEvent;
    extern const OUString msResourceDeactivationEvent;

    extern const OUString msDefaultPaneStyle;
    extern const OUString msDefaultViewStyle;

    /** Return the slide show controller of a running presentation that has
        the same document as the given framework controller.
        @return
            When no presentation is running this method returns an empty reference.
    */
    css::uno::Reference<css::presentation::XSlideShowController> GetSlideShowController (
        const css::uno::Reference<css::frame::XController>& rxController);
}

} } // end of namespace presenter

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
