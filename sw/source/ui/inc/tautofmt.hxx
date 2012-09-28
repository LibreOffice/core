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
#ifndef SW_TAUTOFMT_HXX
#define SW_TAUTOFMT_HXX
#include <sfx2/basedlgs.hxx>

#include <vcl/fixed.hxx>

#include <vcl/lstbox.hxx>

#include <vcl/button.hxx>

#include <vcl/morebtn.hxx>

#include <vcl/virdev.hxx>

class SwTableAutoFmt;
class AutoFmtPreview;
class SwTableAutoFmtTbl;
class SwWrtShell;

//------------------------------------------------------------------------

enum AutoFmtLine { TOP_LINE, BOTTOM_LINE, LEFT_LINE, RIGHT_LINE };

//========================================================================

class SwAutoFormatDlg : public SfxModalDialog
{
    ListBox*        m_pLbFormat;
    VclContainer*   m_pFormatting;
    CheckBox*       m_pBtnNumFormat;
    CheckBox*       m_pBtnBorder;
    CheckBox*       m_pBtnFont;
    CheckBox*       m_pBtnPattern;
    CheckBox*       m_pBtnAlignment;
    OKButton*       m_pBtnOk;
    CancelButton*   m_pBtnCancel;
    PushButton*     m_pBtnAdd;
    PushButton*     m_pBtnRemove;
    PushButton*     m_pBtnRename;
    String          aStrTitle;
    String          aStrLabel;
    String          aStrClose;
    String          aStrDelTitle;
    String          aStrDelMsg;
    String          aStrRenameTitle;
    String          aStrInvalidFmt;
    AutoFmtPreview* m_pWndPreview;

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
    DECL_LINK(OkHdl, void *);
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
