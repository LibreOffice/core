/*************************************************************************
 *
 *  $RCSfile: inscodlg.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:44:59 $
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

#ifndef SC_INSCODLG_HXX
#define SC_INSCODLG_HXX

#ifndef _SV_DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif

#ifndef _SV_BUTTON_HXX //autogen
#include <vcl/imagebtn.hxx>
#endif

#ifndef _SV_GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif

#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif

//------------------------------------------------------------------------
#define INS_CONT_NOEMPTY    0x0100
#define INS_CONT_TRANS      0x0200
#define INS_CONT_LINK       0x0400

#define SC_CELL_SHIFT_DISABLE_DOWN  0x01
#define SC_CELL_SHIFT_DISABLE_RIGHT 0x02

class ScInsertContentsDlg : public ModalDialog
{
public:
            ScInsertContentsDlg( Window*        pParent,
                                 USHORT         nCheckDefaults = 0,
                                 const String*  pStrTitle = NULL );
            ~ScInsertContentsDlg();

    USHORT      GetInsContentsCmdBits() const;
    USHORT      GetFormulaCmdBits() const;
    BOOL        IsSkipEmptyCells() const {return aBtnSkipEmptyCells.IsChecked();}
    BOOL        IsTranspose() const {return aBtnTranspose.IsChecked();}
    BOOL        IsLink() const {return aBtnLink.IsChecked();}
    InsCellCmd  GetMoveMode();

    void    SetOtherDoc( BOOL bSet );
    void    SetFillMode( BOOL bSet );
    void    SetChangeTrack( BOOL bSet );
    void    SetCellShiftDisabled( int nDisable );

private:
    CheckBox        aBtnInsAll;
    CheckBox        aBtnInsStrings;
    CheckBox        aBtnInsNumbers;
    CheckBox        aBtnInsDateTime;
    CheckBox        aBtnInsFormulas;
    CheckBox        aBtnInsNotes;
    CheckBox        aBtnInsAttrs;
    GroupBox        aGbFrame;

    CheckBox        aBtnSkipEmptyCells;
    CheckBox        aBtnTranspose;
    CheckBox        aBtnLink;
    GroupBox        aGbOptions;

    RadioButton     aRbNoOp;
    RadioButton     aRbAdd;
    RadioButton     aRbSub;
    RadioButton     aRbMul;
    RadioButton     aRbDiv;
    GroupBox        aGbOperation;

    RadioButton     aRbMoveNone;
    RadioButton     aRbMoveDown;
    RadioButton     aRbMoveRight;
    GroupBox        aGbMove;

    OKButton        aBtnOk;
    CancelButton    aBtnCancel;
    HelpButton      aBtnHelp;

    BOOL            bOtherDoc;
    BOOL            bFillMode;
    BOOL            bChangeTrack;
    BOOL            bMoveDownDisabled;
    BOOL            bMoveRightDisabled;

    static BOOL     bPreviousAllCheck;
    static USHORT   nPreviousChecks;
    static USHORT   nPreviousChecks2;
    static USHORT   nPreviousFormulaChecks;
    static USHORT   nPreviousMoveMode;          // enum InsCellCmd

    void DisableChecks( BOOL bInsAllChecked = TRUE );
    void TestModes();

    // Handler
    DECL_LINK( InsAllHdl, void* );
    DECL_LINK( LinkBtnHdl, void* );
};


#endif // SC_INSCODLG_HXX


