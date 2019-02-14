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

#include <WindowUpdater.hxx>
#include <ViewShell.hxx>
#include <Window.hxx>
#include <drawdoc.hxx>
#include <View.hxx>

#include <vcl/split.hxx>
#include <sfx2/childwin.hxx>
#include <sfx2/viewfrm.hxx>

#include <algorithm>

namespace sd {

WindowUpdater::WindowUpdater()
    : mpDocument (nullptr)
{
    maCTLOptions.AddListener(this);
}

WindowUpdater::~WindowUpdater() throw ()
{
    maCTLOptions.RemoveListener(this);
}

void WindowUpdater::RegisterWindow (vcl::Window* pWindow)
{
    if (pWindow != nullptr)
    {
        tWindowList::iterator aWindowIterator (
            ::std::find (
                maWindowList.begin(), maWindowList.end(), pWindow));
        if (aWindowIterator == maWindowList.end())
        {
            // Update the device once right now and add it to the list.
            Update (pWindow);
            maWindowList.emplace_back(pWindow);
        }
    }
}

void WindowUpdater::UnregisterWindow (vcl::Window* pWindow)
{
    tWindowList::iterator aWindowIterator (
        ::std::find (
            maWindowList.begin(), maWindowList.end(), pWindow));
    if (aWindowIterator != maWindowList.end())
    {
        maWindowList.erase (aWindowIterator);
    }
}

void WindowUpdater::SetDocument (SdDrawDocument* pDocument)
{
    mpDocument = pDocument;
}

void WindowUpdater::Update (
    OutputDevice* pDevice) const
{
    if (pDevice != nullptr)
    {
        UpdateWindow (pDevice);
    }
}

void WindowUpdater::UpdateWindow (OutputDevice* pDevice) const
{
    if (pDevice == nullptr)
        return;

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

void WindowUpdater::ConfigurationChanged( utl::ConfigurationBroadcaster*, ConfigurationHints )
{
    // Set the current state at all registered output devices.
    for (auto& rxWindow : maWindowList)
        Update (rxWindow);

    // Reformat the document for the modified state to take effect.
    if (mpDocument != nullptr)
        mpDocument->ReformatAllTextObjects();

    // Invalidate the windows to make the modified state visible.
    for (auto& rxWindow : maWindowList)
        rxWindow->Invalidate();
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
