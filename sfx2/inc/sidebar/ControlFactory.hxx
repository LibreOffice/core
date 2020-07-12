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
#ifndef INCLUDED_SFX2_SIDEBAR_CONTROLFACTORY_HXX
#define INCLUDED_SFX2_SIDEBAR_CONTROLFACTORY_HXX

#include <sfx2/dllapi.h>
#include <vcl/vclptr.hxx>

class RadioButton;
namespace vcl { class Window; }

namespace sfx2::sidebar {

/** Factory for controls used in sidebar panels.
    The reason to use this factory instead of creating the controls
    directly is that this way the sidebar has a little more control
    over look and feel of its controls.
*/
class ControlFactory
{
public:
    /** Create the menu button for the task bar.
    */
    static VclPtr<RadioButton> CreateMenuButton (vcl::Window* pParentWindow);

    static VclPtr<RadioButton> CreateTabItem (vcl::Window* pParentWindow);
};


} // end of namespace sfx2::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
