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

#ifndef SC_FILTDLG_HXX
#define SC_FILTDLG_HXX

#ifndef _MOREBTN_HXX //autogen
#include <vcl/morebtn.hxx>
#endif
#ifndef _COMBOBOX_HXX //autogen
#include <vcl/combobox.hxx>
#endif
#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif
#include <svtools/stdctrl.hxx>
#include "global.hxx" // -> ScQueryParam
#include "address.hxx"
#include "anyrefdg.hxx"


//----------------------------------------------------------------------------

class ScFilterOptionsMgr;
class ScRangeData;
class ScViewData;
class ScDocument;
class ScQueryItem;
class TypedScStrCollection;

//==================================================================
// Gemeinsame Resource-Objekte:

#define _COMMON_FILTER_RSCOBJS \
    CheckBox        aBtnCase; \
    CheckBox        aBtnRegExp; \
    CheckBox        aBtnHeader; \
    CheckBox        aBtnUnique; \
    CheckBox        aBtnCopyResult; \
    ListBox         aLbCopyArea; \
    formula::RefEdit        aEdCopyArea; \
    formula::RefButton      aRbCopyArea; \
    CheckBox        aBtnDestPers; \
    FixedText       aFtDbAreaLabel; \
    FixedInfo       aFtDbArea; \
    const String    aStrUndefined; \
    const String    aStrNoName; \
    const String    aStrNone;


#define _INIT_COMMON_FILTER_RSCOBJS \
    aBtnCase        ( this, ScResId( BTN_CASE ) ), \
    aBtnRegExp      ( this, ScResId( BTN_REGEXP ) ), \
    aBtnHeader      ( this, ScResId( BTN_HEADER ) ), \
    aBtnUnique      ( this, ScResId( BTN_UNIQUE ) ), \
    aBtnCopyResult  ( this, ScResId( BTN_COPY_RESULT ) ), \
    aLbCopyArea     ( this, ScResId( LB_COPY_AREA ) ), \
    aEdCopyArea     ( this, this, ScResId( ED_COPY_AREA ) ), \
    aRbCopyArea     ( this, ScResId( RB_COPY_AREA ), &aEdCopyArea, this ), \
    aBtnDestPers    ( this, ScResId( BTN_DEST_PERS ) ), \
    aFtDbAreaLabel  ( this, ScResId( FT_DBAREA_LABEL ) ), \
    aFtDbArea       ( this, ScResId( FT_DBAREA ) ), \
    aStrUndefined   ( ScResId( SCSTR_UNDEFINED ) ), \
    aStrNoName      ( ScGlobal::GetRscString(STR_DB_NONAME) ), \
    aStrNone        ( ScResId( SCSTR_NONE ) ),


//============================================================================
class ScFilterDlg : public ScAnyRefDlg
{
public:
                    ScFilterDlg( SfxBindings* pB, SfxChildWindow* pCW, Window* pParent,
                                 const SfxItemSet&  rArgSet );
                    ~ScFilterDlg();

    virtual void    SetReference( const ScRange& rRef, ScDocument* pDoc );

    virtual sal_Bool    IsRefInputMode() const;
    virtual void    SetActive();

    virtual sal_Bool    Close();
    void            SliderMoved();
    sal_uInt16          GetSliderPos();
    void            RefreshEditRow( sal_uInt16 nOffset );

private:
    FixedLine       aFlCriteria;
    //----------------------------
    ListBox         aLbConnect1;
    ListBox         aLbField1;
    ListBox         aLbCond1;
    ComboBox        aEdVal1;
    //----------------------------
    ListBox         aLbConnect2;
    ListBox         aLbField2;
    ListBox         aLbCond2;
    ComboBox        aEdVal2;
    //----------------------------
    ListBox         aLbConnect3;
    ListBox         aLbField3;
    ListBox         aLbCond3;
    ComboBox        aEdVal3;
    //----------------------------
    ListBox         aLbConnect4;
    ListBox         aLbField4;
    ListBox         aLbCond4;
    ComboBox        aEdVal4;
    //----------------------------
    FixedText       aFtConnect;
    FixedText       aFtField;
    FixedText       aFtCond;
    FixedText       aFtVal;
    FixedLine       aFlSeparator;

    ScrollBar       aScrollBar;

    FixedLine       aFlOptions;
    MoreButton      aBtnMore;
    HelpButton      aBtnHelp;
    OKButton        aBtnOk;
    CancelButton    aBtnCancel;

    _COMMON_FILTER_RSCOBJS

    const String    aStrEmpty;
    const String    aStrNotEmpty;
    const String    aStrRow;
    const String    aStrColumn;

    ScFilterOptionsMgr* pOptionsMgr;

    const sal_uInt16        nWhichQuery;
    const ScQueryParam  theQueryData;
    ScQueryItem*        pOutItem;
    ScViewData*         pViewData;
    ScDocument*         pDoc;
    SCTAB               nSrcTab;

    ComboBox*           aValueEdArr[4];
    ListBox*            aFieldLbArr[4];
    ListBox*            aCondLbArr[4];
    ListBox*            aConnLbArr[4];
    bool                mbHasDates[MAXQUERY];
    sal_Bool                bRefreshExceptQuery[MAXQUERY];
    sal_uInt16              nFieldCount;
    sal_Bool                bRefInputMode;

    TypedScStrCollection*   pEntryLists[MAXCOLCOUNT];
    sal_uInt16              nHeaderPos[MAXCOLCOUNT];

    // Hack: RefInput-Kontrolle
    Timer*  pTimer;

#ifdef _FILTDLG_CXX
private:
    void            Init            ( const SfxItemSet& rArgSet );
    void            FillFieldLists  ();
    void            FillAreaList    ();
    void            UpdateValueList ( sal_uInt16 nList );
    void            UpdateHdrInValueList( sal_uInt16 nList );
    void            ClearValueList  ( sal_uInt16 nList );
    sal_uInt16          GetFieldSelPos  ( SCCOL nField );
    ScQueryItem*    GetOutputItem   ();

    // Handler:
    DECL_LINK( LbSelectHdl,  ListBox* );
    DECL_LINK( ValModifyHdl, ComboBox* );
    DECL_LINK( CheckBoxHdl,  CheckBox* );
    DECL_LINK( EndDlgHdl,    Button* );
    DECL_LINK( MoreClickHdl, MoreButton* );
    DECL_LINK( ScrollHdl, ScrollBar* );

    // Hack: RefInput-Kontrolle
    DECL_LINK( TimeOutHdl,   Timer* );
#endif
};


//============================================================================
class ScSpecialFilterDlg : public ScAnyRefDlg
{
public:
                    ScSpecialFilterDlg( SfxBindings* pB, SfxChildWindow* pCW, Window* pParent,
                                        const SfxItemSet&   rArgSet );
                    ~ScSpecialFilterDlg();

    virtual void    SetReference( const ScRange& rRef, ScDocument* pDoc );

    virtual sal_Bool    IsRefInputMode() const;
    virtual void    SetActive();

    virtual sal_Bool    Close();

private:
    ListBox     aLbFilterArea;
    FixedText   aFtFilterArea;
    formula::RefEdit    aEdFilterArea;
    formula::RefButton  aRbFilterArea;

    FixedLine       aFlOptions;

    _COMMON_FILTER_RSCOBJS

    OKButton        aBtnOk;
    CancelButton    aBtnCancel;
    HelpButton      aBtnHelp;
    MoreButton      aBtnMore;

    ScFilterOptionsMgr* pOptionsMgr;

    const sal_uInt16        nWhichQuery;
    const ScQueryParam  theQueryData;
    ScQueryItem*        pOutItem;
    ScViewData*         pViewData;
    ScDocument*         pDoc;

    formula::RefEdit*           pRefInputEdit;
    sal_Bool                bRefInputMode;

    // Hack: RefInput-Kontrolle
    Timer*  pTimer;

#ifdef _SFILTDLG_CXX
private:
    void            Init( const SfxItemSet& rArgSet );
    ScQueryItem*    GetOutputItem( const ScQueryParam& rParam,
                                    const ScRange& rSource );

    // Handler
    DECL_LINK( FilterAreaSelHdl, ListBox* );
    DECL_LINK( FilterAreaModHdl, formula::RefEdit* );
    DECL_LINK( EndDlgHdl,        Button* );
    DECL_LINK( ScrollHdl, ScrollBar* );

    // Hack: RefInput-Kontrolle
    DECL_LINK( TimeOutHdl,       Timer* );
#endif
};



#endif // SC_FILTDLG_HXX

