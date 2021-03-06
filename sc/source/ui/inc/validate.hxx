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

#include <sfx2/tabdlg.hxx>

#include "anyrefdg.hxx"
#include <sc.hrc>
#include <scmod.hxx>

struct  ScRefHandlerCaller{
    virtual ~ScRefHandlerCaller(){}
};

class ScRefHandlerHelper
{
protected:
    ScRefHandlerCaller*  m_pHandler;
// workaround VS2013 issue with pointers to things that contain virtual base class
#ifdef _WIN32
    #pragma pack(push, 16)
#endif
    void            (ScRefHandlerCaller::*m_pSetReferenceHdl)( const ScRange& , const ScDocument& );
    void            (ScRefHandlerCaller::*m_pSetActiveHdl)();
    void            (ScRefHandlerCaller::*m_pRefInputStartPreHdl)( formula::RefEdit* pEdit, const formula::RefButton* pButton );
    void            (ScRefHandlerCaller::*m_pRefInputDonePostHdl)();
#if defined( _WIN32)
   #pragma pack(pop)
#endif

public:
    typedef void            (ScRefHandlerCaller::*PFUNCSETREFHDLTYPE)( const ScRange& , const ScDocument& );
    typedef void            (ScRefHandlerCaller::*PCOMMONHDLTYPE)();
    typedef void            (ScRefHandlerCaller::*PINPUTSTARTDLTYPE)(  formula::RefEdit* pEdit, const formula::RefButton* pButton );

    void SetSetRefHdl(  PFUNCSETREFHDLTYPE pNewHdl )
    {
        m_pSetReferenceHdl = pNewHdl;
    }

    void SetSetActHdl(  PCOMMONHDLTYPE pNewHdl )
    {
        m_pSetActiveHdl = pNewHdl;
    }

    void SetHandler( ScRefHandlerCaller *pNewHandler )
    {
        m_pHandler = pNewHandler;
    }
    void    SetRefInputStartPreHdl( PINPUTSTARTDLTYPE pNewHdl   ){  m_pRefInputStartPreHdl = pNewHdl;   }
    void    SetRefInputDonePostHdl( void            (ScRefHandlerCaller::*pNewHdl)()    ){  m_pRefInputDonePostHdl = pNewHdl;   }

    ScRefHandlerHelper():m_pHandler(nullptr), m_pSetReferenceHdl( nullptr ), m_pSetActiveHdl(nullptr),  m_pRefInputStartPreHdl( nullptr ), m_pRefInputDonePostHdl( nullptr ){}
};

class ScValidationDlg;

/** The tab page "Criteria" from the Validation dialog. */
class ScTPValidationValue : public ScRefHandlerCaller, public SfxTabPage
{
    static const sal_uInt16 pValueRanges[];
public:
    explicit                    ScTPValidationValue(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rArgSet);
    static std::unique_ptr<SfxTabPage> Create( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rArgSet );
    virtual                     ~ScTPValidationValue() override;

    static const sal_uInt16*    GetRanges() { return pValueRanges; }

    virtual bool                FillItemSet( SfxItemSet* rArgSet ) override;
    virtual void                Reset( const SfxItemSet* rArgSet ) override;

private:
    void                        Init();

    OUString                    GetFirstFormula() const;
    OUString                    GetSecondFormula() const;

    void                        SetFirstFormula( const OUString& rFmlaStr );
    void                        SetSecondFormula( const OUString& rFmlaStr );

                                DECL_LINK(SelectHdl, weld::ComboBox&, void);
                                DECL_LINK(CheckHdl, weld::Button&, void);

    OUString                    maStrMin;
    OUString                    maStrMax;
    OUString                    maStrValue;
    OUString                    maStrFormula;
    OUString                    maStrRange;
    OUString                    maStrList;
    sal_Unicode                 mcFmlaSep;      /// List separator in formulas.

    DECL_LINK( EditSetFocusHdl, formula::RefEdit&, void );
    DECL_LINK( KillEditFocusHdl, formula::RefEdit&, void );
    DECL_LINK( KillButtonFocusHdl, formula::RefButton&, void );
    DECL_LINK( ClickHdl, formula::RefButton&, void );

    formula::RefEdit* m_pRefEdit;

    std::unique_ptr<weld::ComboBox> m_xLbAllow;
    std::unique_ptr<weld::CheckButton> m_xCbAllow;      /// Allow blank cells.
    std::unique_ptr<weld::CheckButton> m_xCbShow;       /// Show selection list in cell.
    std::unique_ptr<weld::CheckButton> m_xCbSort;       /// Sort selection list in cell.
    std::unique_ptr<weld::Label> m_xFtValue;
    std::unique_ptr<weld::ComboBox> m_xLbValue;
    std::unique_ptr<weld::Label> m_xFtMin;
    std::unique_ptr<weld::Widget> m_xMinGrid;
    std::unique_ptr<formula::RefEdit> m_xEdMin;
    std::unique_ptr<weld::TextView> m_xEdList;       /// Entries for explicit list
    std::unique_ptr<weld::Label> m_xFtMax;
    std::unique_ptr<formula::RefEdit> m_xEdMax;
    std::unique_ptr<weld::Label> m_xFtHint;       /// Hint text for cell range validity.
    std::unique_ptr<formula::RefButton> m_xBtnRef;
    std::unique_ptr<weld::Container> m_xRefGrid;

    weld::Container* m_pRefEditParent;
    weld::Container* m_pBtnRefParent;

    void            SetReferenceHdl( const ScRange& , const ScDocument& );
    void            SetActiveHdl();
    void            RefInputStartPreHdl(formula::RefEdit* pEdit, const formula::RefButton* pButton);
    void            RefInputDonePostHdl();
    ScValidationDlg * GetValidationDlg();
public:
    void            SetupRefDlg();
    void            RemoveRefDlg(bool bRestoreModal);
};

/** The "Validity" tab dialog. */
class ScValidationDlg
    : public ScRefHdlrControllerImpl<SfxTabDialogController, false>
    , public ScRefHandlerHelper
{
    typedef ScRefHdlrControllerImpl<SfxTabDialogController, false> ScValidationDlgBase;

    ScTabViewShell * m_pTabVwSh;
    OString m_sValuePageId;
    bool    m_bOwnRefHdlr:1;
    bool    m_bRefInputting:1;

    std::unique_ptr<weld::Container> m_xHBox;

    bool    EnterRefStatus();
    bool    LeaveRefStatus();

public:
    explicit ScValidationDlg(weld::Window* pParent, const SfxItemSet* pArgSet, ScTabViewShell* pTabViewSh);
    virtual ~ScValidationDlg() override;
    static std::shared_ptr<SfxDialogController> Find1AliveObject(const weld::Window *pAncestor)
    {
        return SC_MOD()->Find1RefWindow(SLOTID, pAncestor);
    }
    ScTabViewShell *GetTabViewShell()
    {
        return m_pTabVwSh;
    }

    bool    SetupRefDlg();
    bool    RemoveRefDlg(bool bRestoreModal);

    void            SetModal(bool bModal) { m_xDialog->set_modal(bModal); }

    virtual void EndDialog() override;

    virtual void            SetReference( const ScRange& rRef, ScDocument& rDoc ) override
    {
        if ( m_pHandler && m_pSetReferenceHdl )
            (m_pHandler->*m_pSetReferenceHdl)( rRef, rDoc );
    }

    virtual void            SetActive() override
    {
        if ( m_pHandler && m_pSetActiveHdl )
            (m_pHandler->*m_pSetActiveHdl)();
    }

    bool IsRefInputting() const {  return m_bRefInputting; }
    weld::Container* get_refinput_shrink_parent() { return m_xHBox.get(); }

    virtual void        RefInputStart( formula::RefEdit* pEdit, formula::RefButton* pButton = nullptr ) override
    {
        if( !CanInputStart( pEdit ) )
            return;

        if ( m_pHandler && m_pRefInputStartPreHdl )
            (m_pHandler->*m_pRefInputStartPreHdl)( pEdit, pButton );
        m_bRefInputting = true;
        ScValidationDlgBase::RefInputStart( pEdit, pButton );
    }

    virtual void        RefInputDone( bool bForced = false ) override
    {
        if( !CanInputDone( bForced ) )
            return;

        ScValidationDlgBase::RefInputDone( bForced );
        m_bRefInputting = false;

        if ( m_pHandler && m_pRefInputDonePostHdl )
            (m_pHandler->*m_pRefInputDonePostHdl)();
    }

    bool IsChildFocus() const;

    enum { SLOTID = SID_VALIDITY_REFERENCE };

    virtual void Close() override
    {
        if (m_bOwnRefHdlr)
        {
            if (SfxTabPage* pPage = GetTabPage(m_sValuePageId))
                static_cast<ScTPValidationValue*>(pPage)->RemoveRefDlg(false);
        }
        ScValidationDlgBase::Close();
    }
};

class ScTPValidationHelp : public SfxTabPage
{
private:
    std::unique_ptr<weld::CheckButton> m_xTsbHelp;
    std::unique_ptr<weld::Entry> m_xEdtTitle;
    std::unique_ptr<weld::TextView> m_xEdInputHelp;

public:
    ScTPValidationHelp(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rArgSet);
    static std::unique_ptr<SfxTabPage> Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rArgSet);
    virtual ~ScTPValidationHelp() override;

    virtual bool        FillItemSet ( SfxItemSet* rArgSet ) override;
    virtual void        Reset       ( const SfxItemSet* rArgSet ) override;
};

class ScTPValidationError : public SfxTabPage
{
private:
    std::unique_ptr<weld::CheckButton> m_xTsbShow;
    std::unique_ptr<weld::ComboBox> m_xLbAction;
    std::unique_ptr<weld::Button> m_xBtnSearch;
    std::unique_ptr<weld::Entry> m_xEdtTitle;
    std::unique_ptr<weld::Label> m_xFtError;
    std::unique_ptr<weld::TextView> m_xEdError;

    void    Init();

    // Handler ------------------------
    DECL_LINK(SelectActionHdl, weld::ComboBox&, void);
    DECL_LINK(ClickSearchHdl, weld::Button&, void);

public:
    ScTPValidationError(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rArgSet);
    static std::unique_ptr<SfxTabPage> Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rArgSet);
    virtual ~ScTPValidationError() override;

    virtual bool        FillItemSet ( SfxItemSet* rArgSet ) override;
    virtual void        Reset       ( const SfxItemSet* rArgSet ) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
