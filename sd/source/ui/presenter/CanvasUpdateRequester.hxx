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

#ifndef SD_PRESENTER_CANVAS_UPDATE_REQUESTER_HEADER
#define SD_PRESENTER_CANVAS_UPDATE_REQUESTER_HEADER

#include "precompiled_sd.hxx"

#include <com/sun/star/rendering/XSpriteCanvas.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <sal/types.h>
#include <tools/solar.h>
#include <tools/link.hxx>
#include <vector>

namespace css = ::com::sun::star;

namespace sd { namespace presenter {

/** Each UpdateRequester handles update requests (calls to
    XCanvas::updateScreen()) for one shared canvas (a canvas that has one or
    more PresenterCanvas wrappers).  Multiple calls are collected and lead
    to a single call to updateScreen.
*/
class CanvasUpdateRequester : private ::boost::noncopyable
{
public:
    /** Return the Canvas UpdateRequester object for the given shared
        canvas.  A new object is created when it does not already exist.
    */
    static ::boost::shared_ptr<CanvasUpdateRequester> Instance (
        const css::uno::Reference<css::rendering::XSpriteCanvas>& rxCanvas);

    void RequestUpdate (const sal_Bool bUpdateAll);

private:
    CanvasUpdateRequester (const css::uno::Reference<css::rendering::XSpriteCanvas>& rxCanvas);
    ~CanvasUpdateRequester (void);
    class Deleter; friend class Deleter;

    typedef ::std::vector<
        ::std::pair<
            css::uno::Reference<css::rendering::XSpriteCanvas>,
           ::boost::shared_ptr<CanvasUpdateRequester> > > RequesterMap;
    static RequesterMap maRequesterMap;

    css::uno::Reference<css::rendering::XSpriteCanvas> mxCanvas;
    sal_uLong mnUserEventId;
    sal_Bool mbUpdateFlag;
    DECL_LINK(Callback, void*);
};

} } // end of namespace ::sd::presenter

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
