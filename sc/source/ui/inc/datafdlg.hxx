/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: datafdlg.hxx,v $
 *
 *  $Revision: 1.00 $
 *
 *  last change: $Author: rt $ $Date: 2005/09/08 21:20:35 $
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

#ifndef SC_DATAFDLG_HXX
#define SC_DATAFDLG_HXX


#ifndef _SV_DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif

#ifndef _SV_BUTTON_HXX //autogen
#include <vcl/imagebtn.hxx>
#endif

#ifndef _SV_FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif


#include "global.hxx"

#include <tabvwsh.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>

#define MAX_DATAFORM_COLS   256
#define MAX_DATAFORM_ROWS   32000
#define CTRL_HEIGHT         22
#define FIXED_WIDTH         60
#define EDIT_WIDTH          140
#define FIXED_LEFT          12
#define EDIT_LEFT           78
#define LINE_HEIGHT         30

//zhangyun
class ScDataFormDlg : public ModalDialog
{
private:

    PushButton      aBtnNew;
    PushButton      aBtnDelete;
    PushButton      aBtnRestore;
    PushButton      aBtnLast;
    PushButton      aBtnNext;
    PushButton      aBtnClose;
    ScrollBar       aSlider;
    FixedText       aFixedText;
    //FixedText       aFixedText1;
    //Edit            aEdit1;

    ScTabViewShell* pTabViewShell;
    ScDocument*     pDoc;
    sal_uInt16      aColLength;
    SCROW           aCurrentRow;
    SCCOL           nStartCol;
    SCCOL           nEndCol;
    SCROW           nStartRow;
    SCROW           nEndRow;
    SCTAB           nTab;
    BOOL            bNoSelection;

    FixedText** pFixedTexts;
    Edit** pEdits;

public:
    ScDataFormDlg( Window* pParent, ScTabViewShell* pTabViewShell);
    ~ScDataFormDlg();

    void FillCtrls(SCROW nCurrentRow);
private:

    void SetButtonState();

    // Handler:
    DECL_LINK( Impl_NewHdl,     PushButton*    );
    DECL_LINK( Impl_LastHdl,    PushButton*    );
    DECL_LINK( Impl_NextHdl,    PushButton*    );

    DECL_LINK( Impl_RestoreHdl, PushButton*    );
    DECL_LINK( Impl_DeleteHdl,  PushButton*    );
    DECL_LINK( Impl_CloseHdl,   PushButton*    );

    DECL_LINK( Impl_ScrollHdl,  ScrollBar*    );
    DECL_LINK( Impl_DataModifyHdl,  Edit*    );
};
#endif // SC_DATAFDLG_HXX


