/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/childwin.hxx>

#include "tabvwsh.hxx"

template <sal_Int16 WindowID>
class ChildControllerWrapper : public SfxChildWindow
{
public:
    ChildControllerWrapper(vcl::Window* pParentP, sal_uInt16 nId,
                           SfxBindings* pBindings, const SfxChildWinInfo* pInfo)
        : SfxChildWindow(pParentP, nId)
    {
        ScTabViewShell* pViewShell = getTabViewShell( pBindings );
        if (!pViewShell)
            pViewShell = dynamic_cast< ScTabViewShell *>( SfxViewShell::Current() );
        OSL_ENSURE(pViewShell, "Missing view shell!");

        if (pViewShell)
            SetController(pViewShell->CreateRefDialogController(pBindings, this, pInfo, pParentP->GetFrameWeld(), WindowID));

        if (pViewShell && !GetController())
            pViewShell->GetViewFrame()->SetChildWindow( nId, false );
    }

    static std::unique_ptr<SfxChildWindow> CreateImpl(
                vcl::Window *pParent, sal_uInt16 nId,
                SfxBindings *pBindings, SfxChildWinInfo* pInfo )
    {
        return std::make_unique<ChildControllerWrapper>(pParent, nId, pBindings, pInfo);
    }

    static void RegisterChildWindow (
                    bool   bVisible = false,
                    SfxModule* pModule  = nullptr,
                    SfxChildWindowFlags nFlags = SfxChildWindowFlags::NONE)
    {
        auto pFactory = std::make_unique<SfxChildWinFactory>(ChildControllerWrapper::CreateImpl, WindowID, CHILDWIN_NOPOS );
        pFactory->aInfo.nFlags |= nFlags;
        pFactory->aInfo.bVisible = bVisible;
        SfxChildWindow::RegisterChildWindow(pModule, std::move(pFactory));
    }

    static sal_uInt16 GetChildWindowId()
    {
        return WindowID;
    }

private:
    static ScTabViewShell* getTabViewShell( const SfxBindings *pBindings )
    {
        if( !pBindings )
            return nullptr;
        SfxDispatcher* pDispacher = pBindings ->GetDispatcher();
        if( !pDispacher )
            return nullptr;
        SfxViewFrame* pFrame = pDispacher->GetFrame();
        if( !pFrame )
            return nullptr;
        SfxViewShell* pViewShell = pFrame->GetViewShell();
        if( !pViewShell )
            return nullptr;
        return dynamic_cast<ScTabViewShell*>( pViewShell );
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
