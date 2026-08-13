/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <functional>

#include <vcl/weld.hxx>

/** Warning box for "Replace cell contents?".
    With warning image and "Do not show again" checkbox. */
class ScReplaceWarnBox : public weld::MessageDialogController
{
    std::unique_ptr<weld::CheckButton> m_xWarningOnBox;

    /** Turns the "Warn before replacing cells" option off if the user unchecked
        the box. */
    void SaveWarningOnBox();

public:
    ScReplaceWarnBox(weld::Window* pParent);

    /** Opens dialog if IsDialogEnabled() returns true.
        @descr  If after executing the dialog the checkbox "Do not show again" is set,
                the method DisableDialog() will be called. */
    virtual short run() override;

    /** Asks asynchronously whether cells holding data may be overwritten and
        calls rDoneFn(true) once the user agreed.

        Behaves like run() as far as the "Warn before replacing cells" option
        goes, except under tiled rendering: there the option is turned off
        wholesale so that paste can overwrite silently
        (ScModelObj::initializeForTiledRendering), so it is neither asked nor
        written, and the "Warn me about this in the future." checkbox stays
        hidden. */
    static void AskOverwriteAsync(weld::Window* pParent, const std::function<void(bool)>& rDoneFn);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
