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

#ifndef INCLUDED_SC_SOURCE_UI_INC_FILTDLG_HXX
#define INCLUDED_SC_SOURCE_UI_INC_FILTDLG_HXX

#include <vcl/combobox.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/layout.hxx>
#include <vcl/morebtn.hxx>
#include <svtools/stdctrl.hxx>
#include "global.hxx"
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

class ScFilterOptionsMgr;
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
                    ScFilterDlg( SfxBindings* pB, SfxChildWindow* pCW, vcl::Window* pParent,
                                 const SfxItemSet&  rArgSet );
                    virtual ~ScFilterDlg();

    virtual void    SetReference( const ScRange& rRef, ScDocument* pDoc ) SAL_OVERRIDE;

    virtual bool    IsRefInputMode() const SAL_OVERRIDE;
    virtual void    SetActive() SAL_OVERRIDE;

    virtual bool    Close() SAL_OVERRIDE;
    void            SliderMoved();
    size_t          GetSliderPos();
    void            RefreshEditRow( size_t nOffset );

private:
    ListBox*         pLbConnect1;
    ListBox*         pLbField1;
    ListBox*         pLbCond1;
    ComboBox*        pEdVal1;

    ListBox*         pLbConnect2;
    ListBox*         pLbField2;
    ListBox*         pLbCond2;
    ComboBox*        pEdVal2;

    ListBox*         pLbConnect3;
    ListBox*         pLbField3;
    ListBox*         pLbCond3;
    ComboBox*        pEdVal3;

    ListBox*         pLbConnect4;
    ListBox*         pLbField4;
    ListBox*         pLbCond4;
    ComboBox*        pEdVal4;

    ScrollBar*       pScrollBar;
    VclExpander*     pExpander;

    OKButton*        pBtnOk;
    CancelButton*    pBtnCancel;

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
    const OUString aStrNone;

    const OUString aStrEmpty;
    const OUString aStrNotEmpty;
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
    DECL_LINK( ScrollHdl, void* );
    DECL_LINK( MoreExpandedHdl, void* );

    // Hack: RefInput control
    DECL_LINK( TimeOutHdl,   Timer* );
};

class ScSpecialFilterDlg : public ScAnyRefDlg
{
public:
                    ScSpecialFilterDlg( SfxBindings* pB, SfxChildWindow* pCW, vcl::Window* pParent,
                                        const SfxItemSet&   rArgSet );
                    virtual ~ScSpecialFilterDlg();

    virtual void    SetReference( const ScRange& rRef, ScDocument* pDoc ) SAL_OVERRIDE;

    virtual bool    IsRefInputMode() const SAL_OVERRIDE;
    virtual void    SetActive() SAL_OVERRIDE;

    virtual bool    Close() SAL_OVERRIDE;

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

    ScFilterOptionsMgr* pOptionsMgr;

    const sal_uInt16    nWhichQuery;
    const ScQueryParam  theQueryData;
    ScQueryItem*        pOutItem;
    ScViewData*         pViewData;
    ScDocument*         pDoc;

    formula::RefEdit*   pRefInputEdit;
    bool                bRefInputMode;

    // Hack: RefInput control
    Timer*  pTimer;

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
};

#endif // INCLUDED_SC_SOURCE_UI_INC_FILTDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
