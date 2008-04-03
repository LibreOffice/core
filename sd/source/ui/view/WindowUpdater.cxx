/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: WindowUpdater.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 15:08:34 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "WindowUpdater.hxx"

#ifndef SD_VIEW_SHELL_HXX
#include "ViewShell.hxx"
#endif
#ifndef SD_WINDOW_HXX
#include "Window.hxx"
#endif
#include "drawdoc.hxx"
#ifndef SD_VIEW_HXX
#include "View.hxx"
#endif

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




void WindowUpdater::RegisterWindow (::Window* pWindow)
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




void WindowUpdater::UnregisterWindow (::Window* pWindow)
{
    tWindowList::iterator aWindowIterator (
        ::std::find (
            maWindowList.begin(), maWindowList.end(), pWindow));
    if (aWindowIterator != maWindowList.end())
    {
        maWindowList.erase (aWindowIterator);
    }
}



void WindowUpdater::SetViewShell (ViewShell& rViewShell)
{
    mpViewShell = &rViewShell;
}




void WindowUpdater::SetDocument (SdDrawDocument* pDocument)
{
    mpDocument = pDocument;
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




void WindowUpdater::Notify (SfxBroadcaster&, const SfxHint& rHint)
{
    const SfxSimpleHint& rSimpleHint = static_cast<const SfxSimpleHint&>(rHint);
    if (rSimpleHint.GetId() == SFX_HINT_CTL_SETTINGS_CHANGED)
    {
        // #110094#-7
        // Clear the master page cache so that master pages will be redrawn.
        //if (mpViewShell != NULL)
        //{
        //    SdView* pView = mpViewShell->GetView();
        //    if (pView != NULL)
        //        pView->ReleaseMasterPagePaintCache ();
        //}
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
