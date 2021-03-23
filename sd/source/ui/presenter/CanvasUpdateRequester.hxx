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

#pragma once

#include <com/sun/star/uno/Reference.hxx>
#include <tools/link.hxx>
#include <memory>

namespace com::sun::star::rendering
{
class XSpriteCanvas;
}

struct ImplSVEvent;

namespace sd::presenter
{
/** Each UpdateRequester handles update requests (calls to
    XCanvas::updateScreen()) for one shared canvas (a canvas that has one or
    more PresenterCanvas wrappers).  Multiple calls are collected and lead
    to a single call to updateScreen.
*/
class CanvasUpdateRequester : public std::enable_shared_from_this<CanvasUpdateRequester>
{
public:
    CanvasUpdateRequester(const CanvasUpdateRequester&) = delete;
    CanvasUpdateRequester& operator=(const CanvasUpdateRequester&) = delete;

    /** @return the Canvas UpdateRequester object for the given shared canvas.
                A new object is created when it does not already exist.
    */
    static std::shared_ptr<CanvasUpdateRequester>
    Instance(const css::uno::Reference<css::rendering::XSpriteCanvas>& rxCanvas);

    void RequestUpdate(const bool bUpdateAll);

private:
    explicit CanvasUpdateRequester(
        const css::uno::Reference<css::rendering::XSpriteCanvas>& rxCanvas);
    ~CanvasUpdateRequester();
    class Deleter;
    friend class Deleter;

    /// keep instance alive waiting for event dispatch
    std::shared_ptr<CanvasUpdateRequester> m_pThis;
    css::uno::Reference<css::rendering::XSpriteCanvas> mxCanvas;
    ImplSVEvent* m_pUserEventId;
    bool mbUpdateFlag;

    DECL_LINK(Callback, void*, void);
};

} // end of namespace ::sd::presenter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
