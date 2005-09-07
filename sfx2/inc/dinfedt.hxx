/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dinfedt.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:49:17 $
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
#ifndef _SFX_DINFEDT_HXX
#define _SFX_DINFEDT_HXX

// include ---------------------------------------------------------------

#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _SV_DIALOG_HXX
#include <vcl/dialog.hxx>
#endif
#ifndef _SV_EDIT_HXX
#include <vcl/edit.hxx>
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif

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

