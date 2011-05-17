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
#ifndef SCUI_AUTOFMT_HXX
#define SCUI_AUTOFMT_HXX
#include "autofmt.hxx"

class ScAutoFormatDlg : public ModalDialog
{
public:
            ScAutoFormatDlg( Window*                    pParent,
                             ScAutoFormat*              pAutoFormat,
                             const ScAutoFormatData*    pSelFormatData,
                             ScDocument*                pDoc );
            ~ScAutoFormatDlg();

    sal_uInt16 GetIndex() const { return nIndex; }
    String GetCurrFormatName();

private:
    FixedLine       aFlFormat;
    ListBox         aLbFormat;
    ScAutoFmtPreview*   pWndPreview;
    OKButton        aBtnOk;
    CancelButton    aBtnCancel;
    HelpButton      aBtnHelp;
    PushButton      aBtnAdd;
    PushButton      aBtnRemove;
    MoreButton      aBtnMore;
    FixedLine       aFlFormatting;
    CheckBox        aBtnNumFormat;
    CheckBox        aBtnBorder;
    CheckBox        aBtnFont;
    CheckBox        aBtnPattern;
    CheckBox        aBtnAlignment;
    CheckBox        aBtnAdjust;
    PushButton      aBtnRename;
    String          aStrTitle;
    String          aStrLabel;
    String          aStrClose;
    String          aStrDelTitle;
    String          aStrDelMsg;
    String          aStrRename;

    //------------------------
    ScAutoFormat*           pFormat;
    const ScAutoFormatData* pSelFmtData;
    sal_uInt16                  nIndex;
    sal_Bool                    bCoreDataChanged;
    sal_Bool                    bFmtInserted;

    void Init           ();
    void UpdateChecks   ();
    //------------------------
    DECL_LINK( CheckHdl, Button * );
    DECL_LINK( AddHdl, void * );
    DECL_LINK( RemoveHdl, void * );
    DECL_LINK( SelFmtHdl, void * );
    DECL_LINK( CloseHdl, PushButton * );
    DECL_LINK( DblClkHdl, void * );
    DECL_LINK( RenameHdl, void *);

};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
