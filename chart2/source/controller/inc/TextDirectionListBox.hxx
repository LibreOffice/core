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

#ifndef CHART2_TEXTDIRECTIONLISTBOX_HXX
#define CHART2_TEXTDIRECTIONLISTBOX_HXX

#include <svx/frmdirlbox.hxx>

class Window;

namespace chart
{

class TextDirectionListBox : public svx::FrameDirectionListBox
{
public:
    explicit            TextDirectionListBox( Window* pParent, const ResId& rResId,
                            Window* pWindow1 = 0, Window* pWindow2 = 0 );
    explicit            TextDirectionListBox( Window* pParent,
                            Window* pWindow1 = 0, Window* pWindow2 = 0 );
    virtual             ~TextDirectionListBox();
};

} //namespace chart

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
