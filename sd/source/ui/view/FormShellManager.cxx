/*************************************************************************
 *
 *  $RCSfile: FormShellManager.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 14:50:00 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "FormShellManager.hxx"

#include "ViewShell.hxx"
#include "PaneManager.hxx"
#include "ViewShellBase.hxx"
#include "ObjectBarManager.hxx"
#include "Window.hxx"
#include <svx/fmshell.hxx>

namespace sd {


FormShellManager::FormShellManager (ViewShellBase& rBase)
    : mrBase(rBase),
      meStackPosition(SP_UNKNOWN)
{
    ViewShell* pShell = mrBase.GetMainViewShell();

    if (pShell != NULL)
    {
        RegisterAtCenterPane (pShell);

        // Register at the PaneManager to be informed about changes in the
        // center pane.
        mrBase.GetPaneManager().AddEventListener (
            LINK(
                this,
                FormShellManager,
                PaneManagerEventHandler));

        // Register at the form shell.
        FmFormShell* pFormShell = static_cast<FmFormShell*>(
            pShell->GetObjectBarManager().GetObjectBar(RID_FORMLAYER_TOOLBOX));
        if (pFormShell != NULL)
        {
            pFormShell->SetControlActivationHandler(
                LINK(
                    this,
                    FormShellManager,
                    FormControlActivated));
        }
    }
}




FormShellManager::~FormShellManager (void)
{
    ViewShell* pShell = mrBase.GetMainViewShell();

    if (pShell != NULL)
    {
        UnregisterAtCenterPane (pShell);

        // Unregister at the PaneManager.
        mrBase.GetPaneManager().RemoveEventListener (
            LINK(
                this,
                FormShellManager,
                PaneManagerEventHandler));


        // Unregister at the form shell.
        FmFormShell* pFormShell = static_cast<FmFormShell*>(
            pShell->GetObjectBarManager().GetObjectBar(RID_FORMLAYER_TOOLBOX));
        if (pFormShell != NULL)
        {
            pFormShell->SetControlActivationHandler(Link());
        }
    }
}




void FormShellManager::RegisterAtCenterPane (ViewShell* pShell)
{
    ::Window* pWindow = NULL;
    if (pShell != NULL)
        pWindow = pShell->GetActiveWindow();
    if (pWindow != NULL)
        pWindow->AddEventListener(
            LINK(
                this,
                FormShellManager,
                WindowEventHandler));
}




void FormShellManager::UnregisterAtCenterPane (ViewShell* pShell)
{
    ::Window* pWindow = NULL;
    if (pShell != NULL)
        pWindow = pShell->GetActiveWindow();
    if (pWindow != NULL)
        pWindow->RemoveEventListener(
            LINK(
                this,
                FormShellManager,
                WindowEventHandler));
}




IMPL_LINK(FormShellManager, FormControlActivated, FmFormShell*, EMPTYARG)
{
    // The form shell has been actived.  To give it priority in reacting to
    // slot calls the form shell is moved to the top of the object bar shell
    // stack.
    ViewShell* pShell = mrBase.GetMainViewShell();
    if (pShell!=NULL && meStackPosition!=SP_ABOVE_VIEW_SHELL)
    {
        pShell->GetObjectBarManager().MoveToTop (RID_FORMLAYER_TOOLBOX);
        meStackPosition = SP_ABOVE_VIEW_SHELL;
    }

    return 0;
}




IMPL_LINK(FormShellManager, PaneManagerEventHandler, PaneManagerEvent*, pEvent)
{
    if (pEvent->mePane == PaneManager::PT_CENTER)
    {
        switch (pEvent->meEventId)
        {
            case PaneManagerEvent::EID_VIEW_SHELL_REMOVED:
                UnregisterAtCenterPane (pEvent->mpShell);
                break;

            case PaneManagerEvent::EID_VIEW_SHELL_ADDED:
                RegisterAtCenterPane (pEvent->mpShell);
                break;
        }
    }

    return 0;
}




IMPL_LINK(FormShellManager, WindowEventHandler, VclWindowEvent*, pEvent)
{
    if (pEvent != NULL)
    {
        ::Window* pEventWindow
            = static_cast<VclWindowEvent*>(pEvent)->GetWindow();

        switch (pEvent->GetId())
        {
            case VCLEVENT_WINDOW_GETFOCUS:
            {
                // The window of the center pane got the focus.  Therefore
                // the form shell is moved to the bottom of the object bar
                // stack.
                ViewShell* pShell = mrBase.GetMainViewShell();
                if (pShell!=NULL && meStackPosition!=SP_BELOW_VIEW_SHELL)
                {
                    pShell->GetObjectBarManager().MoveBelowShell (
                        RID_FORMLAYER_TOOLBOX);
                    meStackPosition = SP_BELOW_VIEW_SHELL;
                }
            }
            break;

            case VCLEVENT_WINDOW_LOSEFOCUS:
                // We follow the sloppy focus policy.  Losing the focus is
                // ignored.  We wait for the window gets the focus again or
                // the form shell is focused.  The later, however, is
                // notified over the FormControlActivated handler, not this
                // one.
                break;
        }
    }

    return 0;
}


} // end of namespace sd
