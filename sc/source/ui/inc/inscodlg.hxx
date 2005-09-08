/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: inscodlg.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 21:34:27 $
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

#ifndef SC_INSCODLG_HXX
#define SC_INSCODLG_HXX

#ifndef _SV_DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif

#ifndef _SV_BUTTON_HXX //autogen
#include <vcl/imagebtn.hxx>
#endif

#ifndef _SV_FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif

//------------------------------------------------------------------------
//CHINA001 #define INS_CONT_NOEMPTY     0x0100
//CHINA001 #define INS_CONT_TRANS       0x0200
//CHINA001 #define INS_CONT_LINK        0x0400
//CHINA001
//CHINA001 #define SC_CELL_SHIFT_DISABLE_DOWN   0x01
//CHINA001 #define SC_CELL_SHIFT_DISABLE_RIGHT  0x02
#include "scui_def.hxx"

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
    FixedLine       aFlFrame;
    CheckBox        aBtnInsAll;
    CheckBox        aBtnInsStrings;
    CheckBox        aBtnInsNumbers;
    CheckBox        aBtnInsDateTime;
    CheckBox        aBtnInsFormulas;
    CheckBox        aBtnInsNotes;
    CheckBox        aBtnInsAttrs;
    CheckBox        aBtnInsObjects;

    FixedLine       aFlSep1;
    FixedLine       aFlOptions;
    CheckBox        aBtnSkipEmptyCells;
    CheckBox        aBtnTranspose;
    CheckBox        aBtnLink;

    FixedLine       aFlOperation;
    RadioButton     aRbNoOp;
    RadioButton     aRbAdd;
    RadioButton     aRbSub;
    RadioButton     aRbMul;
    RadioButton     aRbDiv;

    FixedLine       aFlSep2;
    FixedLine       aFlMove;
    RadioButton     aRbMoveNone;
    RadioButton     aRbMoveDown;
    RadioButton     aRbMoveRight;

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


