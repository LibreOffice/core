/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef SDEXT_PRESENTER_VIEW_HELPER_HXX
#define SDEXT_PRESENTER_VIEW_HELPER_HXX

#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/presentation/XSlideShowController.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <boost/noncopyable.hpp>

namespace css = ::com::sun::star;

namespace sdext { namespace presenter {

/** Collection of helper functions that do not fit in anywhere else.
    Provide access to frequently used strings of the drawing framework.
*/
class PresenterHelper
    : ::boost::noncopyable
{
public:
    static const ::rtl::OUString msPaneURLPrefix;
    static const ::rtl::OUString msCenterPaneURL;
    static const ::rtl::OUString msFullScreenPaneURL;

    static const ::rtl::OUString msViewURLPrefix;
    static const ::rtl::OUString msPresenterScreenURL;
    static const ::rtl::OUString msSlideSorterURL;

    static const ::rtl::OUString msResourceActivationEvent;
    static const ::rtl::OUString msResourceDeactivationEvent;

    static const ::rtl::OUString msDefaultPaneStyle;
    static const ::rtl::OUString msDefaultViewStyle;

    /** Return the slide show controller of a running presentation that has
        the same document as the given framework controller.
        @return
            When no presentation is running this method returns an empty reference.
    */
    static css::uno::Reference<css::presentation::XSlideShowController> GetSlideShowController (
        const css::uno::Reference<css::frame::XController>& rxController);

private:
    PresenterHelper (void);
    ~PresenterHelper (void);
};

} } // end of namespace presenter

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
