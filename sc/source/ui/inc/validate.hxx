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

#ifndef SC_VALIDATE_HXX
#define SC_VALIDATE_HXX

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
    void    SetRefInputStartPostHdl(    PINPUTSTARTDLTYPE pNewHdl   ){  m_pRefInputStartPostHdl = pNewHdl;  }
    void    SetRefInputDonePreHdl( void            (ScRefHandlerCaller::*pNewHdl)() ){  m_pRefInputDonePreHdl = pNewHdl;    }

    ScRefHandlerHelper():m_pHandler(NULL), m_pSetReferenceHdl( NULL ), m_pSetActiveHdl(NULL),  m_pRefInputStartPreHdl( NULL ), m_pRefInputStartPostHdl( NULL ), m_pRefInputDonePreHdl( NULL ),  m_pRefInputDonePostHdl( NULL ){}
};

/** The "Validity" tab dialog. */
class ScValidationDlg
    : public ScRefHdlrImpl<ScValidationDlg, SfxTabDialog, false>
    , public ScRefHandlerHelper
{
    typedef ScRefHdlrImpl<ScValidationDlg, SfxTabDialog, false> ScValidationDlgBase;

    DECL_LINK( OkHdl, Button * );

    ScTabViewShell *m_pTabVwSh;
    VclHBox* m_pHBox;
    sal_uInt16 m_nValuePageId;
    bool    m_bOwnRefHdlr:1;
    bool    m_bRefInputting:1;

    bool    EnterRefStatus();
    bool    LeaveRefStatus();

public:
    explicit ScValidationDlg( Window* pParent, const SfxItemSet* pArgSet, ScTabViewShell * pTabViewSh, SfxBindings *pB = NULL );
    virtual                     ~ScValidationDlg();
    static ScValidationDlg * Find1AliveObject( Window *pAncestor )
    {
        return static_cast<ScValidationDlg *>( SC_MOD()->Find1RefWindow( SLOTID, pAncestor ) );
    }
    ScTabViewShell *GetTabViewShell()
    {
        return m_pTabVwSh;
    }

    bool    SetupRefDlg();
    bool    RemoveRefDlg( bool bRestoreModal = true );

    virtual void            SetModal( bool bModal ){ ScValidationDlgBase::SetModalInputMode( bModal ); }

    virtual void            SetReference( const ScRange& rRef, ScDocument* pDoc ) SAL_OVERRIDE
    {
        if ( m_pHandler && m_pSetReferenceHdl )
            (m_pHandler->*m_pSetReferenceHdl)( rRef, pDoc );
    }

    virtual void            SetActive() SAL_OVERRIDE
    {
        if ( m_pHandler && m_pSetActiveHdl )
            (m_pHandler->*m_pSetActiveHdl)();
    }

    void        CloseRefDialog()
    {
        OSL_FAIL( "should not execute here!!!when the edit kill focus, should remove refhandler.\r\n" );

        if ( IsInExecute() )
            EndDialog( RET_CANCEL );
        else if ( GetStyle() & WB_CLOSEABLE )
            Close();
    }

    bool IsRefInputting(){  return m_bRefInputting; }
    Window*             get_refinput_shrink_parent() { return m_pHBox; }

    virtual void        RefInputStart( formula::RefEdit* pEdit, formula::RefButton* pButton = NULL ) SAL_OVERRIDE
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

    virtual void        RefInputDone( bool bForced = false ) SAL_OVERRIDE
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

    bool Close() SAL_OVERRIDE;
};


/** The tab page "Criteria" from the Validation dialog. */
class ScTPValidationValue : public ScRefHandlerCaller, public SfxTabPage
{
public:
    explicit                    ScTPValidationValue( Window* pParent, const SfxItemSet& rArgSet );

    static SfxTabPage*          Create( Window* pParent, const SfxItemSet& rArgSet );
    static sal_uInt16*              GetRanges();

    virtual bool                FillItemSet( SfxItemSet& rArgSet ) SAL_OVERRIDE;
    virtual void                Reset( const SfxItemSet& rArgSet ) SAL_OVERRIDE;

private:
    void                        Init();

    OUString                      GetFirstFormula() const;
    OUString                      GetSecondFormula() const;

    void                        SetFirstFormula( const OUString& rFmlaStr );
    void                        SetSecondFormula( const OUString& rFmlaStr );

                                DECL_LINK(SelectHdl, void *);
                                DECL_LINK(CheckHdl, void *);

    ListBox*                    m_pLbAllow;
    CheckBox*                   m_pCbAllow;      /// Allow blank cells.
    CheckBox*                   m_pCbShow;       /// Show selection list in cell.
    CheckBox*                   m_pCbSort;       /// Sort selection list in cell.
    FixedText*                  m_pFtValue;
    ListBox*                    m_pLbValue;
    FixedText*                  m_pFtMin;
    VclContainer*               m_pMinGrid;
    formula::RefEdit*           m_pEdMin;
    VclMultiLineEdit*           m_pEdList;       /// Entries for explicit list
    FixedText*                  m_pFtMax;
    formula::RefEdit*           m_pEdMax;
    FixedText*                  m_pFtHint;       /// Hint text for cell range validity.

    OUString                    maStrMin;
    OUString                    maStrMax;
    OUString                    maStrValue;
    OUString                    maStrRange;
    OUString                    maStrList;
    sal_Unicode                 mcFmlaSep;      /// List separator in formulas.

    DECL_LINK(EditSetFocusHdl, void *);
    DECL_LINK( KillFocusHdl, Window *);
    void    OnClick( Button *pBtn );
    formula::RefEdit*           m_pRefEdit;
public:
    class ScRefButtonEx : public ::formula::RefButton
    {
        ScTPValidationValue* m_pPage;
        virtual void Click() SAL_OVERRIDE;
    public:
        ScRefButtonEx(Window* pParent, WinBits nStyle)
            : ::formula::RefButton(pParent, nStyle)
            , m_pPage(NULL)
        {
        }
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
    ScRefButtonEx*              m_pBtnRef;
    VclContainer*               m_pRefGrid;
    friend class ScRefButtonEx;
    void            SetReferenceHdl( const ScRange& , ScDocument* );
    void            SetActiveHdl();
    void            RefInputStartPreHdl( formula::RefEdit* pEdit, formula::RefButton* pButton );
    void            RefInputDonePostHdl();
    ScValidationDlg * GetValidationDlg();
public:
    sal_uInt16      GetAllowEntryPos();
    OUString        GetMinText();
    void            SetupRefDlg();
    void            RemoveRefDlg();
};


class ScTPValidationHelp : public SfxTabPage
{
private:
    TriStateBox*      pTsbHelp;
    Edit*             pEdtTitle;
    VclMultiLineEdit* pEdInputHelp;

    void    Init();

    // Handler ------------------------
    // DECL_LINK( SelectHdl, ListBox * );

public:
            ScTPValidationHelp( Window* pParent, const SfxItemSet& rArgSet );
            virtual ~ScTPValidationHelp();

    static  SfxTabPage* Create      ( Window* pParent, const SfxItemSet& rArgSet );
    virtual bool        FillItemSet ( SfxItemSet& rArgSet ) SAL_OVERRIDE;
    virtual void        Reset       ( const SfxItemSet& rArgSet ) SAL_OVERRIDE;
};

class ScTPValidationError : public SfxTabPage
{
private:
    TriStateBox* m_pTsbShow;
    ListBox* m_pLbAction;
    PushButton* m_pBtnSearch;
    Edit* m_pEdtTitle;
    FixedText* m_pFtError;
    VclMultiLineEdit* m_pEdError;

    void    Init();

    // Handler ------------------------
    DECL_LINK(SelectActionHdl, void *);
    DECL_LINK(ClickSearchHdl, void *);

public:
            ScTPValidationError( Window* pParent, const SfxItemSet& rArgSet );
            virtual ~ScTPValidationError();

    static  SfxTabPage* Create      ( Window* pParent, const SfxItemSet& rArgSet );
    virtual bool        FillItemSet ( SfxItemSet& rArgSet ) SAL_OVERRIDE;
    virtual void        Reset       ( const SfxItemSet& rArgSet ) SAL_OVERRIDE;
};

#endif // SC_VALIDATE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
