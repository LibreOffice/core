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

#if !defined(VCL_DLLIMPLEMENTATION) && !defined(TOOLKIT_DLLIMPLEMENTATION) && !defined(VCL_INTERNALS)
#error "don't use this in new code"
#endif

#include <config_options.h>
#include <vcl/dllapi.h>

#include <vcl/toolkit/field.hxx>
#include <vcl/weld.hxx>

class FloatingWindow;
struct ImplCFieldFloatWin;

/*************************************************************************

Description
============

class CalendarField

This class is a DateField with which one can select a date via a DropDownButton
and the CalendarControl.

--------------------------------------------------------------------------

WinBits

See DateField

The preferences for the CalendarControl can be set via SetCalendarStyle().

--------------------------------------------------------------------------

With EnableToday()/EnableNone() we can enable a TodayButton and a NoneButton.

--------------------------------------------------------------------------

Because we only take over the start date into the field, we should query
with GetCalendar() in the SelectHandler and with GetSelectDateCount()/GetSelectDate()
the selected range. We then can e.g. take over that value to another field.

--------------------------------------------------------------------------

If a derived Calendar should be used, we can override the CreateCalendar()
method in CalendarField and create an own calendar there ourselves.

*************************************************************************/


class VCL_DLLPUBLIC CalendarField final : public DateField
{
private:
    VclPtr<ImplCFieldFloatWin> mpFloatWin;
    weld::Button*       mpTodayBtn;
    weld::Button*       mpNoneBtn;
    bool                mbToday;
    bool                mbNone;

                        DECL_DLLPRIVATE_LINK( ImplSelectHdl, weld::Calendar&, void );
                        DECL_DLLPRIVATE_LINK( ImplClickHdl, weld::Button&, void );
                        DECL_DLLPRIVATE_LINK( ImplPopupModeEndHdl, FloatingWindow*, void );

public:
                        CalendarField( vcl::Window* pParent, WinBits nWinStyle );
    virtual             ~CalendarField() override;
    virtual void        dispose() override;

    virtual bool        ShowDropDown( bool bShow ) override;

    void                EnableToday() { mbToday = true; }
    void                EnableNone() { mbNone = true; }

private:
    virtual void    StateChanged( StateChangedType nStateChange ) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
