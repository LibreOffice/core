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

#ifndef INCLUDED_SC_SOURCE_UI_INC_VALIDATE_HXX
#define INCLUDED_SC_SOURCE_UI_INC_VALIDATE_HXX

#include <sfx2/tabdlg.hxx>
#include <vcl/edit.hxx>
#include <vcl/fixed.hxx>
#include <vcl/layout.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/vclmedit.hxx>
#include <svtools/svmedit.hxx>

#include "anyrefdg.hxx"

struct  ScRefHandlerCaller{
    virtual ~ScRefHandlerCaller(){}
};
class ScRefHandlerHelper
{
protected:
    ScRefHandlerCaller  *m_pHandler;
    void            (ScRefHandlerCaller::*m_pSetReferenceHdl)( const ScRange& , ScDocument* );
    void            (ScRefHandlerCaller::*m_pSetActiveHdl)();
    void            (ScRefHandlerCaller::*m_pRefInputStartPreHdl)( formula::RefEdit* pEdit, formula::RefButton* pButton );
    void            (ScRefHandlerCaller::*m_pRefInputStartPostHdl)( formula::RefEdit* pEdit, formula::RefButton* pButton );
    void            (ScRefHandlerCaller::*m_pRefInputDonePreHdl)();
    void            (ScRefHandlerCaller::*m_pRefInputDonePostHdl)();

public:
    typedef void            (ScRefHandlerCaller::*PFUNCSETREFHDLTYPE)( const ScRange& , ScDocument* );
    typedef void            (ScRefHandlerCaller::*PCOMMONHDLTYPE)();
    typedef void            (ScRefHandlerCaller::*PINPUTSTARTDLTYPE)(  formula::RefEdit* pEdit, formula::RefButton* pButton );

    PFUNCSETREFHDLTYPE  SetSetRefHdl(  PFUNCSETREFHDLTYPE pNewHdl )
    {
        PFUNCSETREFHDLTYPE pOldHdl = m_pSetReferenceHdl;
        m_pSetReferenceHdl = pNewHdl;
        return pOldHdl;
    }

    PCOMMONHDLTYPE  SetSetActHdl(  PCOMMONHDLTYPE pNewHdl )
    {
        PCOMMONHDLTYPE pOldHdl = m_pSetActiveHdl;
        m_pSetActiveHdl = pNewHdl;
        return pOldHdl;
    }

    ScRefHandlerCaller  *SetHandler( ScRefHandlerCaller *pNewHandler )
    {
        ScRefHandlerCaller  *pOldHandler = m_pHandler;
        m_pHandler = pNewHandler;
        return pOldHandler;
    }
    void    SetRefInputStartPreHdl( PINPUTSTARTDLTYPE pNewHdl   ){  m_pRefInputStartPreHdl = pNewHdl;   }
    void    SetRefInputDonePostHdl( void            (ScRefHandlerCaller::*pNewHdl)()    ){  m_pRefInputDonePostHdl = pNewHdl;   }

    ScRefHandlerHelper():m_pHandler(nullptr), m_pSetReferenceHdl( nullptr ), m_pSetActiveHdl(nullptr),  m_pRefInputStartPreHdl( nullptr ), m_pRefInputStartPostHdl( nullptr ), m_pRefInputDonePreHdl( nullptr ),  m_pRefInputDonePostHdl( nullptr ){}
};

class ScValidationDlg;

/** The tab page "Criteria" from the Validation dialog. */
class ScTPValidationValue : public ScRefHandlerCaller, public SfxTabPage
{
    static const sal_uInt16 pValueRanges[];
public:
    explicit                    ScTPValidationValue( vcl::Window* pParent, const SfxItemSet& rArgSet );
    virtual                     ~ScTPValidationValue();
    virtual void                dispose() override;
    static VclPtr<SfxTabPage>          Create( vcl::Window* pParent, const SfxItemSet* rArgSet );
    static const sal_uInt16*    GetRanges() { return pValueRanges; }

    virtual bool                FillItemSet( SfxItemSet* rArgSet ) override;
    virtual void                Reset( const SfxItemSet* rArgSet ) override;

private:
    void                        Init();

    OUString                    GetFirstFormula() const;
    OUString                    GetSecondFormula() const;

    void                        SetFirstFormula( const OUString& rFmlaStr );
    void                        SetSecondFormula( const OUString& rFmlaStr );

                                DECL_LINK_TYPED(SelectHdl, ListBox&, void);
                                DECL_LINK_TYPED(CheckHdl, Button*, void);

    VclPtr<ListBox>                    m_pLbAllow;
    VclPtr<CheckBox>                   m_pCbAllow;      /// Allow blank cells.
    VclPtr<CheckBox>                   m_pCbShow;       /// Show selection list in cell.
    VclPtr<CheckBox>                   m_pCbSort;       /// Sort selection list in cell.
    VclPtr<FixedText>                  m_pFtValue;
    VclPtr<ListBox>                    m_pLbValue;
    VclPtr<FixedText>                  m_pFtMin;
    VclPtr<VclContainer>               m_pMinGrid;
    VclPtr<formula::RefEdit>           m_pEdMin;
    VclPtr<VclMultiLineEdit>           m_pEdList;       /// Entries for explicit list
    VclPtr<FixedText>                  m_pFtMax;
    VclPtr<formula::RefEdit>           m_pEdMax;
    VclPtr<FixedText>                  m_pFtHint;       /// Hint text for cell range validity.

    OUString                    maStrMin;
    OUString                    maStrMax;
    OUString                    maStrValue;
    OUString                    maStrRange;
    OUString                    maStrList;
    sal_Unicode                 mcFmlaSep;      /// List separator in formulas.

    DECL_LINK_TYPED( EditSetFocusHdl, Control&, void );
    DECL_LINK_TYPED( KillFocusHdl, Control&, void );
    void    OnClick( Button *pBtn );
    VclPtr<formula::RefEdit>           m_pRefEdit;
public:
    class ScRefButtonEx : public ::formula::RefButton
    {
        VclPtr<ScTPValidationValue> m_pPage;
        virtual void Click() override;
    public:
        ScRefButtonEx(vcl::Window* pParent, WinBits nStyle)
            : ::formula::RefButton(pParent, nStyle)
            , m_pPage(nullptr)
        {
        }
        virtual ~ScRefButtonEx();
        virtual void dispose() override;
        void SetParentPage(ScTPValidationValue *pPage)
        {
            m_pPage = pPage;
        }
        ScTPValidationValue* GetParentPage()
        {
            return m_pPage;
        }
    };
private:
    VclPtr<ScRefButtonEx>              m_pBtnRef;
    VclPtr<VclContainer>               m_pRefGrid;
    friend class ScRefButtonEx;
    void            SetReferenceHdl( const ScRange& , ScDocument* );
    void            SetActiveHdl();
    void            RefInputStartPreHdl( formula::RefEdit* pEdit, formula::RefButton* pButton );
    void            RefInputDonePostHdl();
    ScValidationDlg * GetValidationDlg();
public:
    void            SetupRefDlg();
    void            RemoveRefDlg();
};

/** The "Validity" tab dialog. */
class ScValidationDlg
    : public ScRefHdlrImpl<ScValidationDlg, SfxTabDialog, false>
    , public ScRefHandlerHelper
{
    typedef ScRefHdlrImpl<ScValidationDlg, SfxTabDialog, false> ScValidationDlgBase;

    ScTabViewShell *m_pTabVwSh;
    VclPtr<VclHBox> m_pHBox;
    sal_uInt16 m_nValuePageId;
    bool    m_bOwnRefHdlr:1;
    bool    m_bRefInputting:1;

    bool    EnterRefStatus();
    bool    LeaveRefStatus();

public:
    explicit ScValidationDlg( vcl::Window* pParent, const SfxItemSet* pArgSet, ScTabViewShell * pTabViewSh, SfxBindings *pB = nullptr );
    virtual                     ~ScValidationDlg() { disposeOnce(); }
    virtual void                dispose() override
    {
        if( m_bOwnRefHdlr )
            RemoveRefDlg( false );
        m_pHBox.clear();
        ScRefHdlrImpl<ScValidationDlg, SfxTabDialog, false>::dispose();
    }
    static ScValidationDlg * Find1AliveObject( vcl::Window *pAncestor )
    {
        return static_cast<ScValidationDlg *>( SC_MOD()->Find1RefWindow( SLOTID, pAncestor ) );
    }
    ScTabViewShell *GetTabViewShell()
    {
        return m_pTabVwSh;
    }

    bool    SetupRefDlg();
    bool    RemoveRefDlg( bool bRestoreModal = true );

    void            SetModal( bool bModal ){ ScValidationDlgBase::SetModalInputMode( bModal ); }

    virtual void            SetReference( const ScRange& rRef, ScDocument* pDoc ) override
    {
        if ( m_pHandler && m_pSetReferenceHdl )
            (m_pHandler->*m_pSetReferenceHdl)( rRef, pDoc );
    }

    virtual void            SetActive() override
    {
        if ( m_pHandler && m_pSetActiveHdl )
            (m_pHandler->*m_pSetActiveHdl)();
    }

    bool IsRefInputting(){  return m_bRefInputting; }
    vcl::Window*             get_refinput_shrink_parent() { return m_pHBox; }

    virtual void        RefInputStart( formula::RefEdit* pEdit, formula::RefButton* pButton = nullptr ) override
    {
        if( !CanInputStart( pEdit ) )
            return;

        if ( m_pHandler && m_pRefInputStartPreHdl )
            (m_pHandler->*m_pRefInputStartPreHdl)( pEdit, pButton );
        m_bRefInputting = true;
        ScValidationDlgBase::RefInputStart( pEdit, pButton );
        if ( m_pHandler && m_pRefInputStartPostHdl )
            (m_pHandler->*m_pRefInputStartPostHdl)( pEdit, pButton );
    }

    virtual void        RefInputDone( bool bForced = false ) override
    {
        if( !CanInputDone( bForced ) )
            return;

        if ( m_pHandler && m_pRefInputDonePreHdl )
            (m_pHandler->*m_pRefInputDonePreHdl)();

        ScValidationDlgBase::RefInputDone( bForced );
        m_bRefInputting = false;

        if ( m_pHandler && m_pRefInputDonePostHdl )
            (m_pHandler->*m_pRefInputDonePostHdl)();
    }

    bool IsChildFocus();

    enum { SLOTID = SID_VALIDITY_REFERENCE };

    bool Close() override
    {
        if( m_bOwnRefHdlr )
        {
            if (SfxTabPage* pPage = GetTabPage(m_nValuePageId))
                static_cast<ScTPValidationValue*>(pPage)->RemoveRefDlg();
        }

        return ScValidationDlgBase::Close();
    }
};

class ScTPValidationHelp : public SfxTabPage
{
private:
    VclPtr<TriStateBox>      pTsbHelp;
    VclPtr<Edit>             pEdtTitle;
    VclPtr<VclMultiLineEdit> pEdInputHelp;

    void    Init();

public:
            ScTPValidationHelp( vcl::Window* pParent, const SfxItemSet& rArgSet );
            virtual ~ScTPValidationHelp();
    virtual void dispose() override;

    static  VclPtr<SfxTabPage> Create      ( vcl::Window* pParent, const SfxItemSet* rArgSet );
    virtual bool        FillItemSet ( SfxItemSet* rArgSet ) override;
    virtual void        Reset       ( const SfxItemSet* rArgSet ) override;
};

class ScTPValidationError : public SfxTabPage
{
private:
    VclPtr<TriStateBox> m_pTsbShow;
    VclPtr<ListBox> m_pLbAction;
    VclPtr<PushButton> m_pBtnSearch;
    VclPtr<Edit> m_pEdtTitle;
    VclPtr<FixedText> m_pFtError;
    VclPtr<VclMultiLineEdit> m_pEdError;

    void    Init();

    // Handler ------------------------
    DECL_LINK_TYPED(SelectActionHdl, ListBox&, void);
    DECL_LINK_TYPED(ClickSearchHdl, Button*, void);

public:
            ScTPValidationError( vcl::Window* pParent, const SfxItemSet& rArgSet );
            virtual ~ScTPValidationError();
    virtual void dispose() override;

    static  VclPtr<SfxTabPage> Create      ( vcl::Window* pParent, const SfxItemSet* rArgSet );
    virtual bool        FillItemSet ( SfxItemSet* rArgSet ) override;
    virtual void        Reset       ( const SfxItemSet* rArgSet ) override;
};

#endif // INCLUDED_SC_SOURCE_UI_INC_VALIDATE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
