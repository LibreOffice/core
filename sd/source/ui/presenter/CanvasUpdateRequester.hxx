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

#ifndef INCLUDED_SD_SOURCE_UI_PRESENTER_CANVASUPDATEREQUESTER_HXX
#define INCLUDED_SD_SOURCE_UI_PRESENTER_CANVASUPDATEREQUESTER_HXX

#include <com/sun/star/rendering/XSpriteCanvas.hpp>
#include <boost/noncopyable.hpp>
#include <sal/types.h>
#include <tools/solar.h>
#include <tools/link.hxx>
#include <memory>
#include <vector>

struct ImplSVEvent;

namespace sd { namespace presenter {

/** Each UpdateRequester handles update requests (calls to
    XCanvas::updateScreen()) for one shared canvas (a canvas that has one or
    more PresenterCanvas wrappers).  Multiple calls are collected and lead
    to a single call to updateScreen.
*/
class CanvasUpdateRequester : private ::boost::noncopyable
{
public:
    /** @return the Canvas UpdateRequester object for the given shared canvas.
                A new object is created when it does not already exist.
    */
    static std::shared_ptr<CanvasUpdateRequester> Instance (
        const css::uno::Reference<css::rendering::XSpriteCanvas>& rxCanvas);

    void RequestUpdate (const bool bUpdateAll);

private:
    CanvasUpdateRequester (const css::uno::Reference<css::rendering::XSpriteCanvas>& rxCanvas);
    ~CanvasUpdateRequester();
    class Deleter; friend class Deleter;

    typedef ::std::vector<
        ::std::pair<
            css::uno::Reference<css::rendering::XSpriteCanvas>,
           std::shared_ptr<CanvasUpdateRequester> > > RequesterMap;
    static RequesterMap maRequesterMap;

    css::uno::Reference<css::rendering::XSpriteCanvas> mxCanvas;
    ImplSVEvent * mnUserEventId;
    bool mbUpdateFlag;
    DECL_LINK_TYPED(Callback, void*, void);
};

} } // end of namespace ::sd::presenter

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
