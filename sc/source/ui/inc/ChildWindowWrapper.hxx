/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef CHILD_WINDOW_WRAPPER_HXX
#define CHILD_WINDOW_WRAPPER_HXX

#include <sfx2/basedlgs.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/childwin.hxx>

#include "tabvwsh.hxx"

template <sal_Int16 WindowID>
class ChildWindowWrapper : public SfxChildWindow
{
public:
    ChildWindowWrapper( Window* pParentP, sal_uInt16 nId,
                  SfxBindings* pBindings, SfxChildWinInfo* pInfo ) :
        SfxChildWindow(pParentP, nId)
    {
        ScTabViewShell* pViewShell = getTabViewShell( pBindings );
        if (!pViewShell)
            pViewShell = PTR_CAST( ScTabViewShell, SfxViewShell::Current() );
        OSL_ENSURE(pViewShell, "Missing view shell!");

        if (pViewShell)
            pWindow = pViewShell->CreateRefDialog( pBindings, this, pInfo, pParentP, WindowID );
        else
            pWindow = NULL;

        if (pViewShell && !pWindow)
            pViewShell->GetViewFrame()->SetChildWindow( nId, false );
    }

    static SfxChildWindow* CreateImpl(
                Window *pParent, sal_uInt16 nId,
                SfxBindings *pBindings, SfxChildWinInfo* pInfo )
    {
        SfxChildWindow* pWindow = new ChildWindowWrapper(pParent, nId, pBindings, pInfo);
        return pWindow;
    }

    static void RegisterChildWindow (
                    sal_Bool   bVisible = sal_False,
                    SfxModule* pModule  = NULL,
                    sal_uInt16 nFlags   = 0)
    {
        SfxChildWinFactory* pFactory = new SfxChildWinFactory(ChildWindowWrapper::CreateImpl, WindowID, CHILDWIN_NOPOS );
        pFactory->aInfo.nFlags |= nFlags;
        pFactory->aInfo.bVisible = bVisible;
        SfxChildWindow::RegisterChildWindow(pModule, pFactory);
    }

    virtual SfxChildWinInfo GetInfo() const
    {
        SfxChildWinInfo aInfo = SfxChildWindow::GetInfo();
        ((SfxModelessDialog*)GetWindow())->FillInfo( aInfo );
        return aInfo;
    }

    static sal_uInt16 GetChildWindowId()
    {
        return WindowID;
    }

private:
    static ScTabViewShell* getTabViewShell( SfxBindings *pBindings )
    {
        if( !pBindings )
            return NULL;
        SfxDispatcher* pDispacher = pBindings ->GetDispatcher();
        if( !pDispacher )
            return NULL;
        SfxViewFrame* pFrame = pDispacher->GetFrame();
        if( !pFrame )
            return NULL;
        SfxViewShell* pViewShell = pFrame->GetViewShell();
        if( !pViewShell )
            return NULL;
        return dynamic_cast<ScTabViewShell*>( pViewShell );
    }
};

#endif // CHILD_WINDOW_WRAPPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
