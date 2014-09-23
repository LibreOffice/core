/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "scopetools.hxx"
#include "document.hxx"
#include <vcl/window.hxx>

namespace sc {

AutoCalcSwitch::AutoCalcSwitch(ScDocument& rDoc, bool bAutoCalc) :
    mrDoc(rDoc), mbOldValue(rDoc.GetAutoCalc())
{
    mrDoc.SetAutoCalc(bAutoCalc);
}

AutoCalcSwitch::~AutoCalcSwitch()
{
    mrDoc.SetAutoCalc(mbOldValue);
}

ExpandRefsSwitch::ExpandRefsSwitch(ScDocument& rDoc, bool bExpandRefs) :
    mrDoc(rDoc), mbOldValue(rDoc.IsExpandRefs())
{
    mrDoc.SetExpandRefs(bExpandRefs);
}

ExpandRefsSwitch::~ExpandRefsSwitch()
{
    mrDoc.SetExpandRefs(mbOldValue);
}

UndoSwitch::UndoSwitch(ScDocument& rDoc, bool bUndo) :
    mrDoc(rDoc), mbOldValue(rDoc.IsUndoEnabled())
{
    mrDoc.EnableUndo(bUndo);
}

UndoSwitch::~UndoSwitch()
{
    mrDoc.EnableUndo(mbOldValue);
}

IdleSwitch::IdleSwitch(ScDocument& rDoc, bool bEnableIdle) :
    mrDoc(rDoc), mbOldValue(rDoc.IsIdleEnabled())
{
    mrDoc.EnableIdle(bEnableIdle);
}

IdleSwitch::~IdleSwitch()
{
    mrDoc.EnableIdle(mbOldValue);
}

WaitPointerSwitch::WaitPointerSwitch(vcl::Window* pWin) :
    mpFrameWin(pWin)
{
    if (mpFrameWin)
        mpFrameWin->EnterWait();
}

WaitPointerSwitch::~WaitPointerSwitch()
{
    if (mpFrameWin)
        mpFrameWin->LeaveWait();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
