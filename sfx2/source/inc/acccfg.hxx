/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: acccfg.hxx,v $
 * $Revision: 1.7 $
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

#ifndef _ACCCFG_HXX
#define _ACCCFG_HXX

// #include *****************************************************************

#include <vcl/fixed.hxx>
#ifndef _SV_BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#include <vcl/dialog.hxx>
#include <vcl/lstbox.hxx>
#include <sfx2/minarray.hxx>

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
DECL_2BYTEARRAY(USHORTArr, USHORT, 10, 10)
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

