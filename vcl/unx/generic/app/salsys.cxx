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
#include "precompiled_vcl.hxx"

#include <unx/salunx.h>
#include <unx/saldata.hxx>
#include <unx/salinst.h>
#include <unx/saldisp.hxx>
#include <unx/salsys.h>

#include <vcl/msgbox.hxx>
#include <vcl/button.hxx>

#include <svdata.hxx>

#include <rtl/ustrbuf.hxx>
#include <osl/thread.h>

UnxSalSystem::UnxSalSystem()
{
}

UnxSalSystem::~UnxSalSystem()
{
}

int UnxSalSystem::ShowNativeMessageBox(const String& rTitle, const String& rMessage,
                                       int nButtonCombination, int nDefaultButton)
{
    int nDefButton = 0;
    std::list< String > aButtons;
    int nButtonIds[5], nBut = 0;

    ImplHideSplash();

    if( nButtonCombination == SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_OK ||
        nButtonCombination == SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_OK_CANCEL )
    {
        aButtons.push_back( Button::GetStandardText( BUTTON_OK ) );
        nButtonIds[nBut++] = SALSYSTEM_SHOWNATIVEMSGBOX_BTN_OK;
    }
    if( nButtonCombination == SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_YES_NO_CANCEL ||
        nButtonCombination == SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_YES_NO )
    {
        aButtons.push_back( Button::GetStandardText( BUTTON_YES ) );
        nButtonIds[nBut++] = SALSYSTEM_SHOWNATIVEMSGBOX_BTN_YES;
        aButtons.push_back( Button::GetStandardText( BUTTON_NO ) );
        nButtonIds[nBut++] = SALSYSTEM_SHOWNATIVEMSGBOX_BTN_NO;
        if( nDefaultButton == SALSYSTEM_SHOWNATIVEMSGBOX_BTN_NO )
            nDefButton = 1;
    }
    if( nButtonCombination == SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_OK_CANCEL ||
        nButtonCombination == SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_YES_NO_CANCEL ||
        nButtonCombination == SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_RETRY_CANCEL )
    {
        if( nButtonCombination == SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_RETRY_CANCEL )
        {
            aButtons.push_back( Button::GetStandardText( BUTTON_RETRY ) );
            nButtonIds[nBut++] = SALSYSTEM_SHOWNATIVEMSGBOX_BTN_RETRY;
        }
        aButtons.push_back( Button::GetStandardText( BUTTON_CANCEL ) );
        nButtonIds[nBut++] = SALSYSTEM_SHOWNATIVEMSGBOX_BTN_CANCEL;
        if( nDefaultButton == SALSYSTEM_SHOWNATIVEMSGBOX_BTN_CANCEL )
            nDefButton = aButtons.size()-1;
    }
    if( nButtonCombination == SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_ABORT_RETRY_IGNORE )
    {
        aButtons.push_back( Button::GetStandardText( BUTTON_ABORT ) );
        nButtonIds[nBut++] = SALSYSTEM_SHOWNATIVEMSGBOX_BTN_ABORT;
        aButtons.push_back( Button::GetStandardText( BUTTON_RETRY ) );
        nButtonIds[nBut++] = SALSYSTEM_SHOWNATIVEMSGBOX_BTN_RETRY;
        aButtons.push_back( Button::GetStandardText( BUTTON_IGNORE ) );
        nButtonIds[nBut++] = SALSYSTEM_SHOWNATIVEMSGBOX_BTN_IGNORE;
        switch( nDefaultButton )
        {
            case SALSYSTEM_SHOWNATIVEMSGBOX_BTN_RETRY: nDefButton = 1;break;
            case SALSYSTEM_SHOWNATIVEMSGBOX_BTN_IGNORE: nDefButton = 2;break;
        }
    }

    int nResult = ShowNativeDialog( rTitle, rMessage, aButtons, nDefButton );

    return nResult != -1 ? nButtonIds[ nResult ] : 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
