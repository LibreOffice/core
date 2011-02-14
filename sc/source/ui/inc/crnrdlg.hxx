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

#ifndef SC_CRNRDLG_HXX
#define SC_CRNRDLG_HXX

#include "anyrefdg.hxx"
#include "rangelst.hxx"
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>

#include <hash_map>

class ScViewData;
class ScDocument;


//============================================================================

class ScColRowNameRangesDlg : public ScAnyRefDlg
{
public:
                    ScColRowNameRangesDlg( SfxBindings* pB, SfxChildWindow* pCW, Window* pParent,
                                 ScViewData*    ptrViewData );
                    ~ScColRowNameRangesDlg();

    virtual void    SetReference( const ScRange& rRef, ScDocument* pDoc );

    virtual sal_Bool    IsRefInputMode() const;
    virtual void    SetActive();
    virtual sal_Bool    Close();

private:
    FixedLine       aFlAssign;
    ListBox         aLbRange;

    formula::RefEdit        aEdAssign;
    formula::RefButton      aRbAssign;
    RadioButton     aBtnColHead;
    RadioButton     aBtnRowHead;
    FixedText       aFtAssign2;
    formula::RefEdit        aEdAssign2;
    formula::RefButton      aRbAssign2;

    OKButton        aBtnOk;
    CancelButton    aBtnCancel;
    HelpButton      aBtnHelp;
    PushButton      aBtnAdd;
    PushButton      aBtnRemove;

    ScRange         theCurArea;
    ScRange         theCurData;

    ScRangePairListRef  xColNameRanges;
    ScRangePairListRef  xRowNameRanges;

    typedef ::std::hash_map< String, ScRange, ScStringHashCode, ::std::equal_to<String> > NameRangeMap;
    NameRangeMap    aRangeMap;
    ScViewData*     pViewData;
    ScDocument*     pDoc;
    formula::RefEdit*       pEdActive;
    sal_Bool            bDlgLostFocus;

#ifdef _CRNRDLG_CXX
private:
    void Init               ();
    void UpdateNames        ();
    void UpdateRangeData    ( const ScRange& rRange, sal_Bool bColName );
    void SetColRowData( const ScRange& rLabelRange,sal_Bool bRef=sal_False);
    void AdjustColRowData( const ScRange& rDataRange,sal_Bool bRef=sal_False);
    DECL_LINK( CancelBtnHdl, void * );
    DECL_LINK( OkBtnHdl, void * );
    DECL_LINK( AddBtnHdl, void * );
    DECL_LINK( RemoveBtnHdl, void * );
    DECL_LINK( Range1SelectHdl, void * );
    DECL_LINK( Range1DataModifyHdl, void * );
    DECL_LINK( ColClickHdl, void * );
    DECL_LINK( RowClickHdl, void * );
    DECL_LINK( Range2DataModifyHdl, void * );
    DECL_LINK( GetFocusHdl, Control* );
    DECL_LINK( LoseFocusHdl, Control* );
#endif
};



#endif // SC_CRNRDLG_HXX

