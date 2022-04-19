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

#ifndef INCLUDED_VCL_INC_TOOLBARVALUE_HXX
#define INCLUDED_VCL_INC_TOOLBARVALUE_HXX

#include <tools/gen.hxx>
#include <vcl/salnativewidgets.hxx>

/*  Toolbarvalue:
 *
 *  Value container for toolbars detailing the grip position
 */
class ToolbarValue final : public ImplControlValue
{
public:
    ToolbarValue()
        : ImplControlValue(ControlType::Toolbar, 0)
    {
        mbIsTopDockingArea = false;
    }
    virtual ~ToolbarValue() override;
    virtual ToolbarValue* clone() const override;
    ToolbarValue(ToolbarValue const&) = default;
    ToolbarValue(ToolbarValue&&) = default;
    ToolbarValue& operator=(ToolbarValue const&) = delete; // due to ImplControlValue
    ToolbarValue& operator=(ToolbarValue&&) = delete; // due to ImplControlValue
    tools::Rectangle maGripRect;
    // indicates that this is the top aligned dockingarea
    // adjacent to the menubar, only used on Windows
    bool mbIsTopDockingArea;
};

#endif // INCLUDED_VCL_INC_TOOLBARVALUE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
