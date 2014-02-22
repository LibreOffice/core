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
    
    tWindowList::iterator aWindowIterator (maWindowList.begin());
    while (aWindowIterator != maWindowList.end())
        Update (*aWindowIterator++);

    
    if (mpDocument != NULL)
        mpDocument->ReformatAllTextObjects();

    
    aWindowIterator = maWindowList.begin();
    while (aWindowIterator != maWindowList.end())
        (*aWindowIterator++)->Invalidate();
}


} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
