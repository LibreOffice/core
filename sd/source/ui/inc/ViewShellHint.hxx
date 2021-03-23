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

#include <svl/hint.hxx>

namespace sd
{
/** Local derivation of the SfxHint class that defines some hint ids that
    are used by the ViewShell class and its descendants.
*/
class ViewShellHint : public SfxHint
{
public:
    enum HintId
    {
        // Indicate that a page resize is about to begin.
        HINT_PAGE_RESIZE_START,
        // Indicate that a page resize has been completed.
        HINT_PAGE_RESIZE_END,
        // Indicate that an edit mode change is about to begin.
        HINT_CHANGE_EDIT_MODE_START,
        // Indicate that an edit mode change has been completed.
        HINT_CHANGE_EDIT_MODE_END,

        HINT_COMPLEX_MODEL_CHANGE_START,
        HINT_COMPLEX_MODEL_CHANGE_END
    };

    ViewShellHint(HintId nHintId);

    HintId GetHintId() const { return meHintId; }

private:
    HintId meHintId;
};

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
