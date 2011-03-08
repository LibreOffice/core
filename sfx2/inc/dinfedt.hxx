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
#ifndef _SFX_DINFEDT_HXX
#define _SFX_DINFEDT_HXX

// include ---------------------------------------------------------------

#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/edit.hxx>
#include <vcl/fixed.hxx>

// class InfoEdit_Impl ---------------------------------------------------

class InfoEdit_Impl : public Edit
{
public:
    InfoEdit_Impl( Window* pParent, const ResId& rResId ) :
        Edit( pParent, rResId ) {}

    virtual void    KeyInput( const KeyEvent& rKEvent );
};

// class SfxDocInfoEditDlg -----------------------------------------------

class SfxDocInfoEditDlg : public ModalDialog
{
private:
    FixedLine       aInfoFL;
    InfoEdit_Impl   aInfo1ED;
    InfoEdit_Impl   aInfo2ED;
    InfoEdit_Impl   aInfo3ED;
    InfoEdit_Impl   aInfo4ED;
    OKButton        aOkBT;
    CancelButton    aCancelBT;
    HelpButton      aHelpBtn;

public:
    SfxDocInfoEditDlg( Window* pParent );

    void    SetText1( const String &rStr) { aInfo1ED.SetText( rStr ); }
    void    SetText2( const String &rStr) { aInfo2ED.SetText( rStr ); }
    void    SetText3( const String &rStr) { aInfo3ED.SetText( rStr ); }
    void    SetText4( const String &rStr) { aInfo4ED.SetText( rStr ); }

    String  GetText1() const { return aInfo1ED.GetText(); }
    String  GetText2() const { return aInfo2ED.GetText(); }
    String  GetText3() const { return aInfo3ED.GetText(); }
    String  GetText4() const { return aInfo4ED.GetText(); }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
