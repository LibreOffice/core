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

#include <svtools/fileurlbox.hxx>
#include <osl/file.h>
#include "svl/filenotation.hxx"

//.........................................................................
namespace svt
{
//.........................................................................

    //=====================================================================
    //= FileURLBox
    //=====================================================================
    //---------------------------------------------------------------------
    FileURLBox::FileURLBox( Window* _pParent, WinBits _nStyle )
        :SvtURLBox( _pParent, _nStyle, INET_PROT_FILE )
    {
        DisableHistory();
    }

    //---------------------------------------------------------------------
    void FileURLBox::DisplayURL( const OUString& _rURL )
    {
        OUString sOldText = GetText();

        OFileNotation aTransformer( _rURL, OFileNotation::N_URL );
        OUString sNewText = aTransformer.get( OFileNotation::N_SYSTEM );
        SetText( sNewText );

        if ( sOldText != sNewText )
            Modify();

        UpdatePickList();
    }

    //---------------------------------------------------------------------
    long FileURLBox::PreNotify( NotifyEvent& _rNEvt )
    {
        switch ( _rNEvt.GetType() )
        {
        case EVENT_KEYINPUT:
            if  (   ( GetSubEdit() == _rNEvt.GetWindow()                         )
                &&  ( KEY_RETURN == _rNEvt.GetKeyEvent()->GetKeyCode().GetCode() )
                &&  ( IsInDropDown()                                             )
                )
                m_sPreservedText = GetURL();
            break;

        case EVENT_LOSEFOCUS:
            if ( IsWindowOrChild( _rNEvt.GetWindow() ) )
                DisplayURL( GetText() );
            break;
        }

        return SvtURLBox::PreNotify(_rNEvt);
    }

    //---------------------------------------------------------------------
    long FileURLBox::Notify( NotifyEvent& _rNEvt )
    {
        switch ( _rNEvt.GetType() )
        {
        case EVENT_KEYINPUT:
            if  (   ( GetSubEdit() == _rNEvt.GetWindow()                         )
                &&  ( KEY_RETURN == _rNEvt.GetKeyEvent()->GetKeyCode().GetCode() )
                &&  ( IsInDropDown()                                             )
                )
            {
                long nReturn = SvtURLBox::Notify(_rNEvt);
                DisplayURL( m_sPreservedText );
                return nReturn;
            }
            break;
        }

        return SvtURLBox::Notify(_rNEvt);
    }

//.........................................................................
}   // namespace svt
//.........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
