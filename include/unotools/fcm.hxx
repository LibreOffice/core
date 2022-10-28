/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#include <com/sun/star/frame/XController2.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/uno/Reference.hxx>

namespace utl
{
inline void ConnectModelController(const css::uno::Reference<css::frame::XModel>& xModel,
                                   const css::uno::Reference<css::frame::XController>& xController)
{
    xController->attachModel(xModel);
    xModel->connectController(xController);
    xModel->setCurrentController(xController);
}

// Introduce frame/controller/model to each other
inline void
ConnectFrameControllerModel(const css::uno::Reference<css::frame::XFrame>& xFrame,
                            const css::uno::Reference<css::frame::XController2>& xController,
                            const css::uno::Reference<css::frame::XModel>& xModel)
{
    ConnectModelController(xModel, xController);
    if (xFrame)
        xFrame->setComponent(xController->getComponentWindow(), xController);
    // creates the view and menu
    // for correct menu creation the initialized component must be already set into the frame
    xController->attachFrame(xFrame);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
