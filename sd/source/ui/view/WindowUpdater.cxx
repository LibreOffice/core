/*************************************************************************
 *
 *  $RCSfile: WindowUpdater.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2003-07-02 14:30:51 $
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

#include "WindowUpdater.hxx"

#include "viewshel.hxx"
#include "sdwindow.hxx"
#include "preview.hxx"
#include "drawdoc.hxx"
#include "showwin.hxx"
#include "sdview.hxx"

#ifndef _SPLIT_HXX
#include <vcl/split.hxx>
#endif
#ifndef _SFX_CHILDWIN_HXX
#include <sfx2/childwin.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX
#include <sfx2/viewfrm.hxx>
#endif
#ifndef _SFXSMPLHINT_HXX
#include <svtools/smplhint.hxx>
#endif

#include <algorithm>

namespace sd {

WindowUpdater::WindowUpdater (void)
    : mpViewShell (NULL),
      mpDocument (NULL)
{
    StartListening (maCTLOptions);
}




WindowUpdater::~WindowUpdater (void) throw ()
{
    EndListening (maCTLOptions);
}




void WindowUpdater::RegisterWindow (Window* pWindow)
{
    if (pWindow != NULL)
    {
        tWindowList::iterator aWindowIterator (
            ::std::find (
                maWindowList.begin(), maWindowList.end(), pWindow));
        if (aWindowIterator == maWindowList.end())
        {
            // Update the device once right now and add it to the list.
            Update (pWindow);
            maWindowList.push_back (pWindow);
        }
    }
}




void WindowUpdater::UnregisterWindow (Window* pWindow)
{
    tWindowList::iterator aWindowIterator (
        ::std::find (
            maWindowList.begin(), maWindowList.end(), pWindow));
    if (aWindowIterator != maWindowList.end())
    {
        maWindowList.erase (aWindowIterator);
    }
}



void WindowUpdater::SetViewShell (SdViewShell& rViewShell)
{
    mpViewShell = &rViewShell;
}




void WindowUpdater::SetDocument (SdDrawDocument* pDocument)
{
    mpDocument = pDocument;
}




void WindowUpdater::RegisterPreview (void)
{
    RegisterWindow (GetPreviewWindow());
}




void WindowUpdater::UnregisterPreview (void)
{
    UnregisterWindow (GetPreviewWindow());
}




Window* WindowUpdater::GetPreviewWindow (void) const
{
    SdShowWindow* pShowWindow = NULL;

    // Get the show window of the preview.
    if (mpViewShell != NULL)
    {
        SfxChildWindow* pPreviewChildWindow =
            mpViewShell->GetViewFrame()->GetChildWindow (
                SdPreviewChildWindow::GetChildWindowId());
        if (pPreviewChildWindow != NULL)
        {
            SdPreviewWin* pPreviewWindow =
                static_cast<SdPreviewWin*> (pPreviewChildWindow->GetWindow());
            if (pPreviewWindow!=NULL && pPreviewWindow->GetDoc()==mpViewShell->GetDoc())
                pShowWindow = pPreviewWindow->GetShowWindow();
        }
    }

    return pShowWindow;
}




void WindowUpdater::Update (
    OutputDevice* pDevice,
    SdDrawDocument* pDocument) const
{
    if (pDevice != NULL)
    {
        UpdateWindow (pDevice);
        if (pDocument != NULL)
            pDocument->ReformatAllTextObjects();
    }
}




void WindowUpdater::UpdateWindow (OutputDevice* pDevice) const
{
    if (pDevice != NULL)
    {
        SvtCTLOptions::TextNumerals aNumeralMode (maCTLOptions.GetCTLTextNumerals());

        LanguageType aLanguage;
        // Now this is a bit confusing.  The numerals in arabic languages
        // are Hindi numerals and what the western world generally uses are
        // arabic numerals.  The digits used in the Hindi language are not
        // used at all.
        switch (aNumeralMode)
        {
            case SvtCTLOptions::NUMERALS_HINDI:
                aLanguage = LANGUAGE_ARABIC;
                break;

            case SvtCTLOptions::NUMERALS_SYSTEM:
                aLanguage = LANGUAGE_SYSTEM;
                break;

            case SvtCTLOptions::NUMERALS_ARABIC:
            default:
                aLanguage = LANGUAGE_ENGLISH;
                break;
        }

        pDevice->SetDigitLanguage (aLanguage);
    }
}




void WindowUpdater::Notify (SfxBroadcaster& rBC, const SfxHint& rHint)
{
    const SfxSimpleHint& rSimpleHint = static_cast<const SfxSimpleHint&>(rHint);
    if (rSimpleHint.GetId() == SFX_HINT_CTL_SETTINGS_CHANGED)
    {
        // Clear the master page cache so that master pages will be redrawn.
        if (mpViewShell != NULL)
        {
            SdView* pView = mpViewShell->GetView();
            if (pView != NULL)
                pView->ReleaseMasterPagePaintCache ();
        }
        // Set the current state at all registered output devices.
        tWindowList::iterator aWindowIterator (maWindowList.begin());
        while (aWindowIterator != maWindowList.end())
            Update (*aWindowIterator++);

        // Reformat the document for the modified state to take effect.
        if (mpDocument != NULL)
            mpDocument->ReformatAllTextObjects();

        // Invalidate the windows to make the modified state visible.
        aWindowIterator = maWindowList.begin();
        while (aWindowIterator != maWindowList.end())
            (*aWindowIterator++)->Invalidate();
    }
}


} // end of namespace sd
