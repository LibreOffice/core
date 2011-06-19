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
#include "precompiled_svtools.hxx"
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
    FileURLBox::FileURLBox(Window* _pParent)
        :SvtURLBox(_pParent, INET_PROT_FILE)
    {
        DisableHistory();
    }

    //---------------------------------------------------------------------
    FileURLBox::FileURLBox( Window* _pParent, WinBits _nStyle )
        :SvtURLBox( _pParent, _nStyle, INET_PROT_FILE )
    {
        DisableHistory();
    }

    //---------------------------------------------------------------------
    FileURLBox::FileURLBox(Window* _pParent, const ResId& _rId)
        :SvtURLBox(_pParent, _rId, INET_PROT_FILE)
    {
        DisableHistory();
    }

    //---------------------------------------------------------------------
    void FileURLBox::DisplayURL( const String& _rURL )
    {
        String sOldText = GetText();

        OFileNotation aTransformer( _rURL, OFileNotation::N_URL );
        String sNewText = aTransformer.get( OFileNotation::N_SYSTEM );
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
