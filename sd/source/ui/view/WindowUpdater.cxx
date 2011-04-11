/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "WindowUpdater.hxx"
#include "ViewShell.hxx"
#include "Window.hxx"
#include "drawdoc.hxx"
#include "View.hxx"

#include <vcl/split.hxx>
#include <sfx2/childwin.hxx>
#include <sfx2/viewfrm.hxx>
#include <svl/smplhint.hxx>

#include <algorithm>

namespace sd {

WindowUpdater::WindowUpdater (void)
    : mpViewShell (NULL),
      mpDocument (NULL)
{
    maCTLOptions.AddListener(this);
}




WindowUpdater::~WindowUpdater (void) throw ()
{
    maCTLOptions.RemoveListener(this);
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
                aLanguage = LANGUAGE_ARABIC_SAUDI_ARABIA;
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




void WindowUpdater::ConfigurationChanged( utl::ConfigurationBroadcaster*, sal_uInt32 )
{
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


} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
