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
    virtual void    dispose() SAL_OVERRIDE;

    virtual void    SetReference( const ScRange& rRef, ScDocument* pDoc ) SAL_OVERRIDE;

    virtual bool    IsRefInputMode() const SAL_OVERRIDE;
    virtual void    SetActive() SAL_OVERRIDE;

    virtual bool    Close() SAL_OVERRIDE;
    void            SliderMoved();
    size_t          GetSliderPos();
    void            RefreshEditRow( size_t nOffset );

private:
    VclPtr<ListBox>         pLbConnect1;
    VclPtr<ListBox>         pLbField1;
    VclPtr<ListBox>         pLbCond1;
    VclPtr<ComboBox>        pEdVal1;

    VclPtr<ListBox>         pLbConnect2;
    VclPtr<ListBox>         pLbField2;
    VclPtr<ListBox>         pLbCond2;
    VclPtr<ComboBox>        pEdVal2;

    VclPtr<ListBox>         pLbConnect3;
    VclPtr<ListBox>         pLbField3;
    VclPtr<ListBox>         pLbCond3;
    VclPtr<ComboBox>        pEdVal3;

    VclPtr<ListBox>         pLbConnect4;
    VclPtr<ListBox>         pLbField4;
    VclPtr<ListBox>         pLbCond4;
    VclPtr<ComboBox>        pEdVal4;

    VclPtr<ScrollBar>       pScrollBar;
    VclPtr<VclExpander>     pExpander;

    VclPtr<OKButton>        pBtnOk;
    VclPtr<CancelButton>    pBtnCancel;

    VclPtr<CheckBox>        pBtnCase;
    VclPtr<CheckBox>        pBtnRegExp;
    VclPtr<CheckBox>        pBtnHeader;
    VclPtr<CheckBox>        pBtnUnique;
    VclPtr<CheckBox>        pBtnCopyResult;
    VclPtr<ListBox>         pLbCopyArea;
    VclPtr<formula::RefEdit> pEdCopyArea;
    VclPtr<formula::RefButton> pRbCopyArea;
    VclPtr<CheckBox>        pBtnDestPers;
    VclPtr<FixedText>       pFtDbAreaLabel;
    VclPtr<FixedText>       pFtDbArea;
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

    std::vector<VclPtr<ComboBox>> maValueEdArr;
    std::vector<VclPtr<ListBox>>  maFieldLbArr;
    std::vector<VclPtr<ListBox>>  maCondLbArr;
    std::vector<VclPtr<ListBox>>  maConnLbArr;

    std::deque<bool>   maHasDates;
    std::deque<bool>   maRefreshExceptQuery;
    bool                bRefInputMode;

    EntryListsMap maEntryLists;

    // Hack: RefInput control
    Timer*  pTimer;

private:
    void            Init            ( const SfxItemSet& rArgSet );
    void            FillFieldLists  ();
    void            UpdateValueList ( size_t nList );
    void            UpdateHdrInValueList( size_t nList );
    void            ClearValueList  ( size_t nList );
    size_t          GetFieldSelPos  ( SCCOL nField );
    ScQueryItem*    GetOutputItem   ();

    // Handler:
    DECL_LINK( LbSelectHdl,  ListBox* );
    DECL_LINK( ValModifyHdl, ComboBox* );
    DECL_LINK_TYPED( CheckBoxHdl,  Button*, void );
    DECL_LINK_TYPED( EndDlgHdl,    Button*, void );
    DECL_LINK_TYPED( ScrollHdl, ScrollBar*, void );
    DECL_LINK_TYPED( MoreExpandedHdl, VclExpander&, void );

    // Hack: RefInput control
    DECL_LINK_TYPED( TimeOutHdl, Timer*, void );
};

class ScSpecialFilterDlg : public ScAnyRefDlg
{
public:
                    ScSpecialFilterDlg( SfxBindings* pB, SfxChildWindow* pCW, vcl::Window* pParent,
                                        const SfxItemSet&   rArgSet );
                    virtual ~ScSpecialFilterDlg();
    virtual void    dispose() SAL_OVERRIDE;

    virtual void    SetReference( const ScRange& rRef, ScDocument* pDoc ) SAL_OVERRIDE;

    virtual bool    IsRefInputMode() const SAL_OVERRIDE;
    virtual void    SetActive() SAL_OVERRIDE;

    virtual bool    Close() SAL_OVERRIDE;

private:
    VclPtr<ListBox>         pLbFilterArea;
    VclPtr<formula::RefEdit>   pEdFilterArea;
    VclPtr<formula::RefButton> pRbFilterArea;

    VclPtr<VclExpander>     pExpander;
    VclPtr<CheckBox>        pBtnCase;
    VclPtr<CheckBox>        pBtnRegExp;
    VclPtr<CheckBox>        pBtnHeader;
    VclPtr<CheckBox>        pBtnUnique;
    VclPtr<CheckBox>        pBtnCopyResult;
    VclPtr<ListBox>         pLbCopyArea;
    VclPtr<formula::RefEdit> pEdCopyArea;
    VclPtr<formula::RefButton> pRbCopyArea;
    VclPtr<CheckBox>        pBtnDestPers;
    VclPtr<FixedText>       pFtDbAreaLabel;
    VclPtr<FixedText>       pFtDbArea;
    const OUString aStrUndefined;

    VclPtr<OKButton>        pBtnOk;
    VclPtr<CancelButton>    pBtnCancel;

    ScFilterOptionsMgr* pOptionsMgr;

    const sal_uInt16    nWhichQuery;
    const ScQueryParam  theQueryData;
    ScQueryItem*        pOutItem;
    ScViewData*         pViewData;
    ScDocument*         pDoc;

    VclPtr<formula::RefEdit>   pRefInputEdit;
    bool                bRefInputMode;

    // Hack: RefInput control
    Idle*  pIdle;

private:
    void            Init( const SfxItemSet& rArgSet );
    ScQueryItem*    GetOutputItem( const ScQueryParam& rParam,
                                    const ScRange& rSource );

    // Handler
    DECL_LINK( FilterAreaSelHdl, ListBox* );
    DECL_LINK( FilterAreaModHdl, formula::RefEdit* );
    DECL_LINK_TYPED( EndDlgHdl,  Button*, void );

    // Hack: RefInput control
    DECL_LINK_TYPED( TimeOutHdl, Idle*, void );
};

#endif // INCLUDED_SC_SOURCE_UI_INC_FILTDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
