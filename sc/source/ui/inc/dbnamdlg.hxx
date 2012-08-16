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

#ifndef SC_DBNAMDLG_HXX
#define SC_DBNAMDLG_HXX

#include <vector>

#include <vcl/combobox.hxx>

#include <vcl/fixed.hxx>

#include <vcl/morebtn.hxx>
#include "anyrefdg.hxx"
#include "dbdata.hxx"
#include "expftext.hxx"

class ScViewData;
class ScDocument;


//============================================================================

class ScDbNameDlg : public ScAnyRefDlg
{
public:
                    ScDbNameDlg( SfxBindings* pB, SfxChildWindow* pCW, Window* pParent,
                                 ScViewData*    ptrViewData );
                    ~ScDbNameDlg();

    virtual void    SetReference( const ScRange& rRef, ScDocument* pDoc );

    virtual sal_Bool    IsRefInputMode() const;
    virtual void    SetActive();
    virtual sal_Bool    Close();

private:
    FixedLine           aFlName;
    ComboBox            aEdName;

    FixedLine           aFlAssign;
    formula::RefEdit            aEdAssign;
    formula::RefButton          aRbAssign;

    FixedLine           aFlOptions;
    CheckBox            aBtnHeader;
    CheckBox            aBtnDoSize;
    CheckBox            aBtnKeepFmt;
    CheckBox            aBtnStripData;
    ScExpandedFixedText aFTSource;
    FixedText           aFTOperations;

    OKButton        aBtnOk;
    CancelButton    aBtnCancel;
    HelpButton      aBtnHelp;
    PushButton      aBtnAdd;
    PushButton      aBtnRemove;
    MoreButton      aBtnMore;

    sal_Bool            bSaved;


    const String    aStrAdd;
    const String    aStrModify;
    const String    aStrInvalid;

    String          aStrSource;
    String          aStrOperations;

    ScViewData*     pViewData;
    ScDocument*     pDoc;
    sal_Bool            bRefInputMode;
    ScAddress::Details aAddrDetails;

    ScDBCollection  aLocalDbCol;
    ScRange         theCurArea;
    std::vector<ScRange> aRemoveList;

#ifdef _DBNAMDLG_CXX
private:
    void            Init();
    void            UpdateNames();
    void            UpdateDBData( const String& rStrName );
    void            SetInfoStrings( const ScDBData* pDBData );

    DECL_LINK( CancelBtnHdl, void * );
    DECL_LINK( OkBtnHdl, void * );
    DECL_LINK( AddBtnHdl, void * );
    DECL_LINK( RemoveBtnHdl, void * );
    DECL_LINK( NameModifyHdl, void * );
    DECL_LINK( AssModifyHdl, void * );
#endif
};



#endif // SC_DBNAMDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
