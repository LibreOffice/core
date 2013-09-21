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
#ifndef _SVX_SIDEBAR_POPUP_CONTROL_HXX_
#define _SVX_SIDEBAR_POPUP_CONTROL_HXX_

#include "svx/svxdllapi.h"
#include <vcl/ctrl.hxx>

namespace svx { namespace sidebar {

/** Base class for sidebar related popup controls.
    A PopupControl is typically a child of a PopupContainer and
    provides the actual content of a popup.
    This base class takes care of painting the proper background and
    border for sidebar popups.
    Specialize by derivation.
*/
class SVX_DLLPUBLIC PopupControl
    : public Control
{
public :
    PopupControl (
        Window* pParent,
        const ResId& rResId);
    virtual ~PopupControl (void);

    virtual void Paint (const Rectangle& rect);
};

} } // end of namespace svx::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
