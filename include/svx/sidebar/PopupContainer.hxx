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
#ifndef _SVX_SIDEBAR_POPUP_CONTAINER_HXX_
#define _SVX_SIDEBAR_POPUP_CONTAINER_HXX_

#include "svx/svxdllapi.h"
#include <vcl/floatwin.hxx>

namespace svx { namespace sidebar {

/** Simple base class for popup container windows used by sidebar
    related drop downs.
    It initializes the underlying floating window with the right
    flags and closes the drop down when appropriate.
*/
class SVX_DLLPUBLIC PopupContainer
    : public FloatingWindow
{
public:
    PopupContainer (Window* pParent);
    virtual ~PopupContainer (void);

    virtual long Notify (NotifyEvent& rNEvt);
};


} } // end of namespace svx::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
