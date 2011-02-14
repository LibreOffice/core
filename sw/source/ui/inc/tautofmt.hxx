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
#ifndef SW_TAUTOFMT_HXX
#define SW_TAUTOFMT_HXX
#include <sfx2/basedlgs.hxx>

#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif

#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif

#ifndef _MOREBTN_HXX //autogen
#include <vcl/morebtn.hxx>
#endif

#ifndef _VIRDEV_HXX //autogen
#include <vcl/virdev.hxx>
#endif

class SwTableAutoFmt;
class AutoFmtPreview;
class SwTableAutoFmtTbl;
class SwWrtShell;

//------------------------------------------------------------------------

enum AutoFmtLine { TOP_LINE, BOTTOM_LINE, LEFT_LINE, RIGHT_LINE };

//========================================================================

class SwAutoFormatDlg : public SfxModalDialog
{
    FixedLine       aFlFormat;
    ListBox         aLbFormat;
    CheckBox        aBtnNumFormat;
    CheckBox        aBtnBorder;
    CheckBox        aBtnFont;
    CheckBox        aBtnPattern;
    CheckBox        aBtnAlignment;
    FixedLine       aFlFormats;
    OKButton        aBtnOk;
    CancelButton    aBtnCancel;
    HelpButton      aBtnHelp;
    PushButton      aBtnAdd;
    PushButton      aBtnRemove;
    PushButton      aBtnRename;
    MoreButton      aBtnMore;
    String          aStrTitle;
    String          aStrLabel;
    String          aStrClose;
    String          aStrDelTitle;
    String          aStrDelMsg;
    String          aStrRenameTitle;
    String          aStrInvalidFmt;
    AutoFmtPreview* pWndPreview;

    //------------------------
    SwWrtShell*             pShell;
    SwTableAutoFmtTbl*      pTableTbl;
    sal_uInt8                   nIndex;
    sal_uInt8                   nDfltStylePos;
    sal_Bool                    bCoreDataChanged : 1;
    sal_Bool                    bSetAutoFmt : 1;


    void Init( const SwTableAutoFmt* pSelFmt );
    void UpdateChecks( const SwTableAutoFmt&, sal_Bool bEnableBtn );
    //------------------------
    DECL_LINK( CheckHdl, Button * );
    DECL_LINK( OkHdl, Button * );
    DECL_LINK( AddHdl, void * );
    DECL_LINK( RemoveHdl, void * );
    DECL_LINK( RenameHdl, void * );
    DECL_LINK( SelFmtHdl, void * );

public:
    SwAutoFormatDlg( Window* pParent, SwWrtShell* pShell,
                        sal_Bool bSetAutoFmt = sal_True,
                        const SwTableAutoFmt* pSelFmt = 0 );
    virtual ~SwAutoFormatDlg();

    void FillAutoFmtOfIndex( SwTableAutoFmt*& rToFill ) const;
};


#endif // SW_AUTOFMT_HXX

