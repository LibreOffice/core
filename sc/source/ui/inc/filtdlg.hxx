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

#pragma once

#include <address.hxx>
#include "anyrefdg.hxx"
#include <queryparam.hxx>
#include <filterentries.hxx>
#include <queryentry.hxx>

#include <memory>
#include <deque>
#include <vector>
#include <map>

class ScFilterOptionsMgr;
class ScViewData;
class ScDocument;
class ScQueryItem;

class ScFilterDlg : public ScAnyRefDlgController
{
    struct EntryList
    {
        ScFilterEntries maFilterEntries;
        size_t mnHeaderPos;

        EntryList(const EntryList&) = delete;
        const EntryList& operator=(const EntryList&) = delete;

        EntryList();
    };
    typedef std::map<SCCOL, std::unique_ptr<EntryList>> EntryListsMap;
public:
    ScFilterDlg(SfxBindings* pB, SfxChildWindow* pCW, weld::Window* pParent,
                const SfxItemSet& rArgSet);
    virtual ~ScFilterDlg() override;

    virtual void    SetReference( const ScRange& rRef, ScDocument& rDoc ) override;

    virtual bool    IsRefInputMode() const override;
    virtual void    SetActive() override;

    virtual void    Close() override;
    void            SliderMoved();
    size_t          GetSliderPos() const;
    void            RefreshEditRow( size_t nOffset );

private:
    const OUString aStrUndefined;
    const OUString aStrNone;

    const OUString aStrEmpty;
    const OUString aStrNotEmpty;
    const OUString aStrColumn;

    std::unique_ptr<ScFilterOptionsMgr> pOptionsMgr;

    const sal_uInt16        nWhichQuery;
    ScQueryParam        theQueryData;
    std::unique_ptr<ScQueryItem> pOutItem;
    ScViewData*         pViewData;
    ScDocument*         pDoc;
    SCTAB               nSrcTab;

    std::vector<weld::ComboBox*> maValueEdArr;
    std::vector<weld::ComboBox*> maFieldLbArr;
    std::vector<weld::ComboBox*> maCondLbArr;
    std::vector<weld::ComboBox*> maConnLbArr;
    std::vector<weld::Button*>   maRemoveBtnArr;

    std::deque<bool>   maHasDates;
    std::deque<bool>   maRefreshExceptQuery;
    bool                bRefInputMode;

    EntryListsMap m_EntryLists;

    // Hack: RefInput control
    std::unique_ptr<Timer>  pTimer;

    std::unique_ptr<weld::ComboBox> m_xLbConnect1;
    std::unique_ptr<weld::ComboBox> m_xLbField1;
    std::unique_ptr<weld::ComboBox> m_xLbCond1;
    std::unique_ptr<weld::ComboBox> m_xEdVal1;
    std::unique_ptr<weld::Button>   m_xBtnRemove1;

    std::unique_ptr<weld::ComboBox> m_xLbConnect2;
    std::unique_ptr<weld::ComboBox> m_xLbField2;
    std::unique_ptr<weld::ComboBox> m_xLbCond2;
    std::unique_ptr<weld::ComboBox> m_xEdVal2;
    std::unique_ptr<weld::Button>   m_xBtnRemove2;

    std::unique_ptr<weld::ComboBox> m_xLbConnect3;
    std::unique_ptr<weld::ComboBox> m_xLbField3;
    std::unique_ptr<weld::ComboBox> m_xLbCond3;
    std::unique_ptr<weld::ComboBox> m_xEdVal3;
    std::unique_ptr<weld::Button>   m_xBtnRemove3;

    std::unique_ptr<weld::ComboBox> m_xLbConnect4;
    std::unique_ptr<weld::ComboBox> m_xLbField4;
    std::unique_ptr<weld::ComboBox> m_xLbCond4;
    std::unique_ptr<weld::ComboBox> m_xEdVal4;
    std::unique_ptr<weld::Button>   m_xBtnRemove4;

    std::unique_ptr<weld::Widget> m_xContents;
    std::unique_ptr<weld::ScrolledWindow> m_xScrollBar;
    std::unique_ptr<weld::Expander> m_xExpander;

    std::unique_ptr<weld::Button> m_xBtnClear;
    std::unique_ptr<weld::Button> m_xBtnOk;
    std::unique_ptr<weld::Button> m_xBtnCancel;

    std::unique_ptr<weld::CheckButton> m_xBtnCase;
    std::unique_ptr<weld::CheckButton> m_xBtnRegExp;
    std::unique_ptr<weld::CheckButton> m_xBtnHeader;
    std::unique_ptr<weld::CheckButton> m_xBtnUnique;
    std::unique_ptr<weld::CheckButton> m_xBtnCopyResult;
    std::unique_ptr<weld::ComboBox> m_xLbCopyArea;
    std::unique_ptr<formula::RefEdit> m_xEdCopyArea;
    std::unique_ptr<formula::RefButton> m_xRbCopyArea;
    std::unique_ptr<weld::CheckButton> m_xBtnDestPers;
    std::unique_ptr<weld::Label> m_xFtDbAreaLabel;
    std::unique_ptr<weld::Label> m_xFtDbArea;

private:
    void            Init            ( const SfxItemSet& rArgSet );
    void            FillFieldLists  ();
    void            UpdateValueList ( size_t nList );
    void            UpdateHdrInValueList( size_t nList );
    void            ClearValueList  ( size_t nList );
    size_t          GetFieldSelPos  ( SCCOL nField );
    ScQueryItem*    GetOutputItem   ();
    void            SetValString    ( const OUString& rQueryStr,
                                      const ScQueryEntry::Item& rItem,
                                      OUString& rValStr );

    // Handler:
    DECL_LINK( LbSelectHdl,  weld::ComboBox&, void );
    DECL_LINK( ValModifyHdl, weld::ComboBox&, void );
    DECL_LINK( CheckBoxHdl,  weld::Button&, void );
    DECL_LINK( BtnClearHdl,  weld::Button&, void );
    DECL_LINK( BtnRemoveHdl, weld::Button&, void );
    DECL_LINK( EndDlgHdl,    weld::Button&, void );
    DECL_LINK( ScrollHdl, weld::ScrolledWindow&, void );
    DECL_LINK( MoreExpandedHdl, weld::Expander&, void );

    // Hack: RefInput control
    DECL_LINK( TimeOutHdl, Timer*, void );
};

class ScSpecialFilterDlg : public ScAnyRefDlgController
{
public:
    ScSpecialFilterDlg(SfxBindings* pB, SfxChildWindow* pCW, weld::Window* pParent,
                       const SfxItemSet& rArgSet);
    virtual ~ScSpecialFilterDlg() override;

    virtual void    SetReference( const ScRange& rRef, ScDocument& rDoc ) override;

    virtual bool    IsRefInputMode() const override;
    virtual void    SetActive() override;

    virtual void    Close() override;

private:
    const OUString aStrUndefined;

    std::unique_ptr<ScFilterOptionsMgr> pOptionsMgr;

    const sal_uInt16    nWhichQuery;
    const ScQueryParam  theQueryData;
    std::unique_ptr<ScQueryItem> pOutItem;
    ScViewData*         pViewData;
    ScDocument*         pDoc;

    bool                bRefInputMode;

    formula::RefEdit* m_pRefInputEdit;

    std::unique_ptr<weld::ComboBox> m_xLbFilterArea;
    std::unique_ptr<formula::RefEdit> m_xEdFilterArea;
    std::unique_ptr<formula::RefButton> m_xRbFilterArea;

    std::unique_ptr<weld::Expander> m_xExpander;
    std::unique_ptr<weld::CheckButton> m_xBtnCase;
    std::unique_ptr<weld::CheckButton> m_xBtnRegExp;
    std::unique_ptr<weld::CheckButton> m_xBtnHeader;
    std::unique_ptr<weld::CheckButton> m_xBtnUnique;
    std::unique_ptr<weld::CheckButton> m_xBtnCopyResult;
    std::unique_ptr<weld::ComboBox> m_xLbCopyArea;
    std::unique_ptr<formula::RefEdit> m_xEdCopyArea;
    std::unique_ptr<formula::RefButton> m_xRbCopyArea;
    std::unique_ptr<weld::CheckButton> m_xBtnDestPers;
    std::unique_ptr<weld::Label> m_xFtDbAreaLabel;
    std::unique_ptr<weld::Label> m_xFtDbArea;

    std::unique_ptr<weld::Button> m_xBtnOk;
    std::unique_ptr<weld::Button> m_xBtnCancel;

    std::unique_ptr<weld::Frame> m_xFilterFrame;
    std::unique_ptr<weld::Label> m_xFilterLabel;

private:
    void            Init( const SfxItemSet& rArgSet );
    ScQueryItem*    GetOutputItem( const ScQueryParam& rParam,
                                    const ScRange& rSource );

    // Handler
    DECL_LINK( FilterAreaSelHdl, weld::ComboBox&, void );
    DECL_LINK( FilterAreaModHdl, formula::RefEdit&, void );
    DECL_LINK( EndDlgHdl,  weld::Button&, void );

    // RefInput control
    DECL_LINK( RefInputEditHdl, formula::RefEdit&, void );
    DECL_LINK( RefInputButtonHdl, formula::RefButton&, void );
    void RefInputHdl();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
