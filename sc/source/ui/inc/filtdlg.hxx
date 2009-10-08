/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: filtdlg.hxx,v $
 * $Revision: 1.7 $
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
    FixedLine       aFlOptions; \
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
    OKButton        aBtnOk; \
    CancelButton    aBtnCancel; \
    HelpButton      aBtnHelp; \
    MoreButton      aBtnMore; \
    const String    aStrUndefined; \
    const String    aStrNoName; \
    const String    aStrNone;


#define _INIT_COMMON_FILTER_RSCOBJS \
    aFlOptions      ( this, ScResId( FL_OPTIONS ) ), \
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
    aBtnOk          ( this, ScResId( BTN_OK ) ), \
    aBtnCancel      ( this, ScResId( BTN_CANCEL ) ), \
    aBtnHelp        ( this, ScResId( BTN_HELP ) ), \
    aBtnMore        ( this, ScResId( BTN_MORE ) ), \
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

    virtual BOOL    IsRefInputMode() const;
    virtual void    SetActive();

    virtual BOOL    Close();

private:
    FixedLine       aFlCriteria;
    //----------------------------
    ListBox         aLbField1;
    ListBox         aLbCond1;
    ComboBox        aEdVal1;
    //----------------------------
    ListBox         aLbConnect1;
    ListBox         aLbField2;
    ListBox         aLbCond2;
    ComboBox        aEdVal2;
    //----------------------------
    ListBox         aLbConnect2;
    ListBox         aLbField3;
    ListBox         aLbCond3;
    ComboBox        aEdVal3;
    //----------------------------
    FixedText       aFtConnect;
    FixedText       aFtField;
    FixedText       aFtCond;
    FixedText       aFtVal;

    _COMMON_FILTER_RSCOBJS

    const String    aStrEmpty;
    const String    aStrNotEmpty;
    const String    aStrRow;
    const String    aStrColumn;

    ScFilterOptionsMgr* pOptionsMgr;

    const USHORT        nWhichQuery;
    const ScQueryParam  theQueryData;
    ScQueryItem*        pOutItem;
    ScViewData*         pViewData;
    ScDocument*         pDoc;
    SCTAB               nSrcTab;

    ComboBox*           aValueEdArr[3];
    ListBox*            aFieldLbArr[3];
    ListBox*            aCondLbArr[3];
    USHORT              nFieldCount;
    BOOL                bRefInputMode;

    TypedScStrCollection*   pEntryLists[MAXCOLCOUNT];
    USHORT              nHeaderPos[MAXCOLCOUNT];

    // Hack: RefInput-Kontrolle
    Timer*  pTimer;

#ifdef _FILTDLG_CXX
private:
    void            Init            ( const SfxItemSet& rArgSet );
    void            FillFieldLists  ();
    void            FillAreaList    ();
    void            UpdateValueList ( USHORT nList );
    void            UpdateHdrInValueList( USHORT nList );
    void            ClearValueList  ( USHORT nList );
    USHORT          GetFieldSelPos  ( SCCOL nField );
    ScQueryItem*    GetOutputItem   ();

    // Handler:
    DECL_LINK( LbSelectHdl,  ListBox* );
    DECL_LINK( ValModifyHdl, ComboBox* );
    DECL_LINK( CheckBoxHdl,  CheckBox* );
    DECL_LINK( EndDlgHdl,    Button* );
    DECL_LINK( MoreClickHdl, MoreButton* );

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

    virtual BOOL    IsRefInputMode() const;
    virtual void    SetActive();

    virtual BOOL    Close();

private:
    ListBox     aLbFilterArea;
    FixedText   aFtFilterArea;
    formula::RefEdit    aEdFilterArea;
    formula::RefButton  aRbFilterArea;

    _COMMON_FILTER_RSCOBJS

    ScFilterOptionsMgr* pOptionsMgr;

    const USHORT        nWhichQuery;
    const ScQueryParam  theQueryData;
    ScQueryItem*        pOutItem;
    ScViewData*         pViewData;
    ScDocument*         pDoc;

    formula::RefEdit*           pRefInputEdit;
    BOOL                bRefInputMode;

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

    // Hack: RefInput-Kontrolle
    DECL_LINK( TimeOutHdl,       Timer* );
#endif
};



#endif // SC_FILTDLG_HXX

