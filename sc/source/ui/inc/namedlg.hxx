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

#ifndef SC_NAMEDLG_HXX
#define SC_NAMEDLG_HXX

#include <vcl/morebtn.hxx>
#include <vcl/combobox.hxx>
#include <vcl/group.hxx>
#include <vcl/fixed.hxx>
#include "rangenam.hxx"
#include "anyrefdg.hxx"

class ScViewData;
class ScDocument;


//==================================================================

class ScNameDlg : public ScAnyRefDlg
{
private:
    FixedLine       aFlName;
    ComboBox        aEdName;

    FixedLine       aFlAssign;
    formula::RefEdit        aEdAssign;
    formula::RefButton      aRbAssign;

    FixedLine       aFlType;
    CheckBox        aBtnPrintArea;
    CheckBox        aBtnColHeader;
    CheckBox        aBtnCriteria;
    CheckBox        aBtnRowHeader;

    OKButton        aBtnOk;
    CancelButton    aBtnCancel;
    HelpButton      aBtnHelp;
    PushButton      aBtnAdd;
    PushButton      aBtnRemove;
    MoreButton      aBtnMore;
    sal_Bool            bSaved;

    const String    aStrAdd;    // "Hinzufuegen"
    const String    aStrModify; // "Aendern"
    const String    errMsgInvalidSym;

    ScViewData*     pViewData;
    ScDocument*     pDoc;
    ScRangeName     aLocalRangeName;
    const ScAddress theCursorPos;
    Selection       theCurSel;

#ifdef _NAMEDLG_CXX
private:
    void Init();
    void UpdateChecks();
    void UpdateNames();
    void CalcCurTableAssign( String& aAssign, ScRangeData* pRangeData );

    // Handler:
    DECL_LINK( OkBtnHdl, void * );
    DECL_LINK( CancelBtnHdl, void * );
    DECL_LINK( AddBtnHdl, void * );
    DECL_LINK( RemoveBtnHdl, void * );
    DECL_LINK( EdModifyHdl, Edit * );
    DECL_LINK( NameSelectHdl, void * );
    DECL_LINK( AssignGetFocusHdl, void * );
#endif

protected:

    virtual void    RefInputDone( sal_Bool bForced = false );


public:
                    ScNameDlg( SfxBindings* pB, SfxChildWindow* pCW, Window* pParent,
                               ScViewData*      ptrViewData,
                               const ScAddress& aCursorPos );
                    ~ScNameDlg();

    virtual void    SetReference( const ScRange& rRef, ScDocument* pDoc );
    virtual sal_Bool    IsRefInputMode() const;

    virtual void    SetActive();
    virtual sal_Bool    Close();

};



#endif // SC_NAMEDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
