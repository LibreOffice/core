/*************************************************************************
 *
 *  $RCSfile: TaskPaneFocusManager.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 14:36:21 $
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

#include "TaskPaneFocusManager.hxx"

#include <vcl/window.hxx>
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_EVENT_HXX
#include <vcl/event.hxx>
#endif


namespace sd { namespace toolpanel {


FocusManager* FocusManager::spInstance = NULL;


FocusManager& FocusManager::Instance (void)
{
    if (spInstance == NULL)
    {
        ::vos::OGuard aGuard (::Application::GetSolarMutex());
        if (spInstance == NULL)
            spInstance = new FocusManager ();
    }
    return *spInstance;
}




FocusManager::FocusManager (void)
{
}




FocusManager::~FocusManager (void)
{
}




void FocusManager::RegisterLink (::Window* pParent, ::Window* pChild)
{
    RegisterUpLink (pChild, pParent);
    RegisterDownLink (pParent, pChild);
}




void FocusManager::RegisterUpLink (::Window* pSource, ::Window* pTarget)
{
    maUpLinks[pSource] = pTarget;
    pSource->AddEventListener (LINK (this, FocusManager, WindowEventListener));
}




void FocusManager::RegisterDownLink (::Window* pSource, ::Window* pTarget)
{
    maDownLinks[pSource] = pTarget;
    pSource->AddEventListener (LINK (this, FocusManager, WindowEventListener));
}




bool FocusManager::TransferFocusUp (::Window* pWindow)
{
    bool bSuccess (false);

    HashMap::iterator aTarget = maUpLinks.find (pWindow);
    if (aTarget != maUpLinks.end())
    {
        aTarget->second->GrabFocus();
        bSuccess = true;
    }

    return bSuccess;
}




bool FocusManager::TransferFocusDown (::Window* pWindow)
{
    bool bSuccess (false);

    HashMap::iterator aTarget = maDownLinks.find (pWindow);
    if (aTarget != maDownLinks.end())
    {
        aTarget->second->GrabFocus();
        bSuccess = true;
    }

    return bSuccess;
}






IMPL_LINK(FocusManager, WindowEventListener, VclSimpleEvent*, pEvent)
{
    if (pEvent!=NULL && pEvent->ISA(VclWindowEvent))
    {
        VclWindowEvent* pWindowEvent = static_cast<VclWindowEvent*>(pEvent);
        switch (pWindowEvent->GetId())
        {
            //            case VCLEVENT_WINDOW_KEYUP:
            case VCLEVENT_WINDOW_KEYINPUT:
                Window* pSource = pWindowEvent->GetWindow();
                KeyEvent* pKeyEvent =
                    static_cast<KeyEvent*>(pWindowEvent->GetData());
                if (pKeyEvent->GetKeyCode() == KEY_RETURN)
                    TransferFocusDown (pSource);
                else if (pKeyEvent->GetKeyCode() == KEY_ESCAPE)
                    TransferFocusUp (pSource);
                break;
        }
    }
    return 0;
}


} } // end of namespace ::sd::toolpanel
