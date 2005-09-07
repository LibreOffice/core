/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: acccfg.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:53:32 $
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

#ifndef _ACCCFG_HXX
#define _ACCCFG_HXX

// #include *****************************************************************

#ifndef _SV_FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef _SV_DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif
#ifndef _SV_LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif
#include "minarray.hxx"

// forward ******************************************************************

class SfxAcceleratorManager;

// class SfxAcceleratorConfigListBox *************************************************

class SfxAcceleratorConfigListBox : public ListBox
{
    void KeyInput( const KeyEvent &rKEvt );

public:

    SfxAcceleratorConfigListBox( Window *pParent, ResId &rResId ) :
        ListBox( pParent, rResId ) {}

    void ReplaceEntry( USHORT nPos, const String &rStr );
    void ExpandEntry ( USHORT nPos, const String &rStr );
};

// class USHORTArr **********************************************************

#if !defined(_MNUCFG_HXX) && !defined(_TBXCFG_HXX)
DECL_2BYTEARRAY(USHORTArr, USHORT, 10, 10);
#endif

// class SfxAcceleratorConfigDialog **************************************************

class SfxAcceleratorConfigDialog : public ModalDialog
{
    OKButton           aOKButton;
    CancelButton       aCancelButton;
    PushButton         aChangeButton;
    PushButton         aRemoveButton;
    SfxAcceleratorConfigListBox aEntriesBox;
    FixedText          aDescriptionTextText;
    FixedText          aDescriptionInfoText;
    FixedLine          aKeyboardGroup;
    FixedText          aGroupText;
    ListBox            aGroupLBox;
    FixedText          aFunctionText;
    ListBox            aFunctionBox;
    FixedText          aKeyText;
    ListBox            aKeyBox;
    FixedLine          aFunctionsGroup;

    SfxAcceleratorManager *pMgr;

    USHORTArr     aAccelArr;
    USHORTArr     aFunctionArr;
    USHORTArr     aKeyArr;

    void OKHdl    ( Button  * );
    void ChangeHdl( Button  * );
    void RemoveHdl( Button  * );
    void SelectHdl( ListBox *pListBox );

    KeyCode PosToKeyCode   ( USHORT nPos )          const;
    USHORT  KeyCodeToPos   ( const KeyCode &rCode ) const;
    String  GetFunctionName( KeyFuncType eType )    const;

public:

    SfxAcceleratorConfigDialog( Window *pParent );
};

#endif

