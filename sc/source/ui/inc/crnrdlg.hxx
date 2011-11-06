/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

