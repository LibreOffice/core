/*************************************************************************
 *
 *  $RCSfile: dinfedt.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:52:22 $
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
#ifndef _SFX_DINFEDT_HXX
#define _SFX_DINFEDT_HXX

// include ---------------------------------------------------------------

#ifndef _SV_EDIT_HXX //autogen wg. Edit
#include <vcl/edit.hxx>
#endif
#ifndef _SV_DIALOG_HXX //autogen wg. ModalDialog
#include <vcl/dialog.hxx>
#endif
#ifndef _SV_GROUP_HXX //autogen wg. GroupBox
#include <vcl/group.hxx>
#endif
#ifndef _SV_BUTTON_HXX //autogen wg. OKButton
#include <vcl/button.hxx>
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
    InfoEdit_Impl   aInfo1ED;
    InfoEdit_Impl   aInfo2ED;
    InfoEdit_Impl   aInfo3ED;
    InfoEdit_Impl   aInfo4ED;
    GroupBox        aInfoGB;
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

