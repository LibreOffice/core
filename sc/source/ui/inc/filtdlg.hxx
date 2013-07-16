/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef SC_FILTDLG_HXX
#define SC_FILTDLG_HXX

#include <vcl/combobox.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/layout.hxx>
#include <vcl/morebtn.hxx>
#include <svtools/stdctrl.hxx>
#include "global.hxx" // -> ScQueryParam
#include "address.hxx"
#include "anyrefdg.hxx"
#include "queryparam.hxx"
#include "typedstrdata.hxx"

#include <deque>
#include <vector>
#include <map>
#include <boost/ptr_container/ptr_map.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>

//----------------------------------------------------------------------------

class ScFilterOptionsMgr;
class ScNewFilterOptionsMgr;
class ScViewData;
class ScDocument;
class ScQueryItem;

class ScFilterDlg : public ScAnyRefDlg
{
    struct EntryList : boost::noncopyable
    {
        std::vector<ScTypedStrData> maList;
        size_t mnHeaderPos;
        EntryList();
    };
    typedef boost::ptr_map<SCCOL,EntryList> EntryListsMap;
public:
                    ScFilterDlg( SfxBindings* pB, SfxChildWindow* pCW, Window* pParent,
                                 const SfxItemSet&  rArgSet );
                    ~ScFilterDlg();

    virtual void    SetReference( const ScRange& rRef, ScDocument* pDoc );

    virtual sal_Bool    IsRefInputMode() const;
    virtual void    SetActive();

    virtual sal_Bool    Close();
    void            SliderMoved();
    size_t          GetSliderPos();
    void            RefreshEditRow( size_t nOffset );

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

    CheckBox        aBtnCase;
    CheckBox        aBtnRegExp;
    CheckBox        aBtnHeader;
    CheckBox        aBtnUnique;
    CheckBox        aBtnCopyResult;
    ListBox         aLbCopyArea;
    formula::RefEdit aEdCopyArea;
    formula::RefButton aRbCopyArea;
    CheckBox        aBtnDestPers;
    FixedText       aFtDbAreaLabel;
    FixedInfo       aFtDbArea;
    const OUString aStrUndefined;
    const OUString aStrNone;

    const OUString aStrEmpty;
    const OUString aStrNotEmpty;
    const OUString aStrRow;
    const OUString aStrColumn;

    ScFilterOptionsMgr* pOptionsMgr;

    const sal_uInt16        nWhichQuery;
    ScQueryParam        theQueryData;
    ScQueryItem*        pOutItem;
    ScViewData*         pViewData;
    ScDocument*         pDoc;
    SCTAB               nSrcTab;

    std::vector<ComboBox*> maValueEdArr;
    std::vector<ListBox*>  maFieldLbArr;
    std::vector<ListBox*>  maCondLbArr;
    std::vector<ListBox*>  maConnLbArr;

    std::deque<bool>   maHasDates;
    std::deque<bool>   maRefreshExceptQuery;
    bool                bRefInputMode;

    EntryListsMap maEntryLists;

    // Hack: RefInput control
    Timer*  pTimer;

#ifdef _FILTDLG_CXX
private:
    void            Init            ( const SfxItemSet& rArgSet );
    void            FillFieldLists  ();
    void            FillAreaList    ();
    void            UpdateValueList ( size_t nList );
    void            UpdateHdrInValueList( size_t nList );
    void            ClearValueList  ( size_t nList );
    size_t          GetFieldSelPos  ( SCCOL nField );
    ScQueryItem*    GetOutputItem   ();

    // Handler:
    DECL_LINK( LbSelectHdl,  ListBox* );
    DECL_LINK( ValModifyHdl, ComboBox* );
    DECL_LINK( CheckBoxHdl,  CheckBox* );
    DECL_LINK( EndDlgHdl,    Button* );
    DECL_LINK( MoreClickHdl, void* );
    DECL_LINK( ScrollHdl, void* );

    // Hack: RefInput control
    DECL_LINK( TimeOutHdl,   Timer* );
#endif
};

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
    ListBox*         pLbFilterArea;
    formula::RefEdit*   pEdFilterArea;
    formula::RefButton* pRbFilterArea;

    VclExpander*     pExpander;
    CheckBox*        pBtnCase;
    CheckBox*        pBtnRegExp;
    CheckBox*        pBtnHeader;
    CheckBox*        pBtnUnique;
    CheckBox*        pBtnCopyResult;
    ListBox*         pLbCopyArea;
    formula::RefEdit* pEdCopyArea;
    formula::RefButton* pRbCopyArea;
    CheckBox*        pBtnDestPers;
    FixedText*       pFtDbAreaLabel;
    FixedText*       pFtDbArea;
    const OUString aStrUndefined;
    const OUString aStrNoName;

    OKButton*        pBtnOk;
    CancelButton*    pBtnCancel;
    HelpButton*      pBtnHelp;

    ScNewFilterOptionsMgr* pOptionsMgr;

    const sal_uInt16    nWhichQuery;
    const ScQueryParam  theQueryData;
    ScQueryItem*        pOutItem;
    ScViewData*         pViewData;
    ScDocument*         pDoc;

    formula::RefEdit*   pRefInputEdit;
    bool                bRefInputMode;

    // Hack: RefInput control
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

    // Hack: RefInput control
    DECL_LINK( TimeOutHdl,       Timer* );
#endif
};



#endif // SC_FILTDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
