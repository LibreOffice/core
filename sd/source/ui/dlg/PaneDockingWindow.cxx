/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include "PaneDockingWindow.hxx"
#include "Window.hxx"
#include "ViewShellBase.hxx"
#include "framework/FrameworkHelper.hxx"
#include "sdresid.hxx"
#include "res_bmp.hrc"
#include <sfx2/dispatch.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/taskpanelist.hxx>
#include <vcl/splitwin.hxx>
#include <vcl/svapp.hxx>
#include <tools/wintypes.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;
using ::sfx2::TitledDockingWindow;

namespace sd {

PaneDockingWindow::PaneDockingWindow(
        SfxBindings *_pBindings, SfxChildWindow *pChildWindow, ::Window* pParent,
        const ResId& rResId, const OUString& rsTitle )
        :TitledDockingWindow( _pBindings, pChildWindow, pParent, rResId )
{
    SetTitle( rsTitle );
}

PaneDockingWindow::~PaneDockingWindow (void)
{
}

void PaneDockingWindow::StateChanged( StateChangedType nType )
{
    switch (nType)
    {
        case STATE_CHANGE_INITSHOW:
            Resize();
            GetContentWindow().SetStyle(GetContentWindow().GetStyle() | WB_DIALOGCONTROL);
            break;

        case STATE_CHANGE_VISIBLE:
            
            
            
            
            
            ViewShellBase* pBase = ViewShellBase::GetViewShellBase(
                GetBindings().GetDispatcher()->GetFrame());
            if (pBase != NULL)
            {
                framework::FrameworkHelper::Instance(*pBase)->UpdateConfiguration();
            }
            break;
    }
    SfxDockingWindow::StateChanged (nType);
}

void PaneDockingWindow::MouseButtonDown (const MouseEvent& rEvent)
{
    if (rEvent.GetButtons() == MOUSE_LEFT)
    {
        
        
        
        
        
        GetContentWindow().SetStyle(GetContentWindow().GetStyle() | WB_DIALOGCONTROL);
        GetContentWindow().GrabFocus();
    }
    SfxDockingWindow::MouseButtonDown(rEvent);
}








void PaneDockingWindow::SetValidSizeRange (const Range aValidSizeRange)
{
    SplitWindow* pSplitWindow = dynamic_cast<SplitWindow*>(GetParent());
    if (pSplitWindow != NULL)
    {
        const sal_uInt16 nId (pSplitWindow->GetItemId(static_cast< ::Window*>(this)));
        const sal_uInt16 nSetId (pSplitWindow->GetSet(nId));
        
        
        const SvBorder aBorder (GetDecorationBorder());
        sal_Int32 nCompensation (pSplitWindow->IsHorizontal()
            ? aBorder.Top() + aBorder.Bottom()
            : aBorder.Left() + aBorder.Right());
        pSplitWindow->SetItemSizeRange(
            nSetId,
            Range(
                aValidSizeRange.Min() + nCompensation,
                aValidSizeRange.Max() + nCompensation));
    }
}




PaneDockingWindow::Orientation PaneDockingWindow::GetOrientation (void) const
{
    SplitWindow* pSplitWindow = dynamic_cast<SplitWindow*>(GetParent());
    if (pSplitWindow == NULL)
        return UnknownOrientation;
    else if (pSplitWindow->IsHorizontal())
        return HorizontalOrientation;
    else
        return VerticalOrientation;
}

} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
