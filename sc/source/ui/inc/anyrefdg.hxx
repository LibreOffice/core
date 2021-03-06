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

#include <sfx2/basedlgs.hxx>
#include <address.hxx>
#include <formula/funcutl.hxx>
#include "IAnyRefDialog.hxx"

#include <memory>

class SfxObjectShell;
class ScDocument;
class ScRangeList;
class ScCompiler;

class ScFormulaReferenceHelper
{
    IAnyRefDialog*      m_pDlg;
    ::std::unique_ptr<ScCompiler>         m_pRefComp;
    formula::RefEdit*       m_pRefEdit;           // active input field
    formula::RefButton*     m_pRefBtn;            // associated button
    weld::Dialog*               m_pDialog;
    SfxBindings*        m_pBindings;
    SCTAB               m_nRefTab;                // used for ShowReference

    OUString            m_sOldDialogText;         // Original title of the dialog window

    bool                m_bEnableColorRef;
    bool                m_bHighlightRef;

    DECL_LINK( ActivateHdl, weld::Widget&, bool );

public:
    ScFormulaReferenceHelper(IAnyRefDialog* _pDlg, SfxBindings* _pBindings);
    ~ScFormulaReferenceHelper() COVERITY_NOEXCEPT_FALSE;
    void dispose();

    void                ShowSimpleReference(const OUString& rStr);
    void                ShowFormulaReference(const OUString& rStr);
    bool                ParseWithNames( ScRangeList& rRanges, const OUString& rStr, const ScDocument& rDoc );
    void                Init();

    void                ShowReference(const OUString& rStr);
    void                ReleaseFocus( formula::RefEdit* pEdit );
    void                HideReference( bool bDoneRefMode = true );
    void                RefInputStart( formula::RefEdit* pEdit, formula::RefButton* pButton );
    void                RefInputDone( bool bForced );
    void                ToggleCollapsed( formula::RefEdit* pEdit, formula::RefButton* pButton );

    void                SetDialog(weld::Dialog* pDialog) { m_pDialog = pDialog; }
    void                DoClose( sal_uInt16 nId );
    void                SetDispatcherLock( bool bLock );
    static void         EnableSpreadsheets( bool bFlag = true );
    static void         ViewShellChanged();

    static              void enableInput(bool _bInput);

public:
    static bool         CanInputStart( const formula::RefEdit *pEdit ){ return !!pEdit; }
    bool                CanInputDone(bool bForced) { return m_pRefEdit && (bForced || !m_pRefBtn); }
};

class ScRefHandler : public IAnyRefDialog
{
    weld::DialogController* m_pController;
    bool                 m_bInRefMode;

private:
    ScFormulaReferenceHelper
                        m_aHelper;
    SfxBindings*        m_pMyBindings;

    OUString            m_aDocName;               // document on which the dialog was opened

protected:
    void                disposeRefHandler();
    bool                DoClose( sal_uInt16 nId );

    void                SetDispatcherLock( bool bLock );

    virtual void        RefInputStart( formula::RefEdit* pEdit, formula::RefButton* pButton = nullptr ) override;
    virtual void        RefInputDone( bool bForced = false ) override;

    bool                ParseWithNames( ScRangeList& rRanges, const OUString& rStr, const ScDocument& pDoc );

public:
                        ScRefHandler(SfxDialogController &rController, SfxBindings* pB, bool bBindRef);
    virtual             ~ScRefHandler() COVERITY_NOEXCEPT_FALSE override;

    virtual void        SetReference( const ScRange& rRef, ScDocument& rDoc ) override = 0;
    virtual void        AddRefEntry() override;

    virtual bool        IsRefInputMode() const override;
    virtual bool        IsTableLocked() const override;
    virtual bool        IsDocAllowed( SfxObjectShell* pDocSh ) const override;

    virtual void        ShowReference(const OUString& rStr) override;
    virtual void        HideReference( bool bDoneRefMode = true ) override;

    virtual void        ToggleCollapsed( formula::RefEdit* pEdit, formula::RefButton* pButton ) override;
    virtual void        ReleaseFocus( formula::RefEdit* pEdit ) override;

    virtual void        ViewShellChanged() override;
    void                SwitchToDocument();

    virtual void        SetActive() override = 0;

public:
    bool                EnterRefMode();
    bool                LeaveRefMode();
    static bool CanInputStart( const formula::RefEdit *pEdit )
    {
        return ScFormulaReferenceHelper::CanInputStart( pEdit );
    }
    bool CanInputDone( bool bForced )
    {
        return m_aHelper.CanInputDone( bForced );
    }
};

template<class TBase, bool bBindRef = true>
struct ScRefHdlrControllerImpl : public TBase, public ScRefHandler
{
    enum { UNKNOWN_SLOTID = 0U, SLOTID = UNKNOWN_SLOTID };

    ScRefHdlrControllerImpl(weld::Window* pParent, const OUString& rUIXMLDescription, const OString& rID, const SfxItemSet* pArg, SfxBindings *pB)
        : TBase(pParent, rUIXMLDescription, rID, pArg)
        , ScRefHandler(*static_cast<TBase*>(this), pB, bBindRef)
    {
    }

    ScRefHdlrControllerImpl(SfxBindings* pB, SfxChildWindow* pCW, weld::Window* pParent, const OUString& rUIXMLDescription, const OString& rID)
        : TBase(pB, pCW, pParent, rUIXMLDescription, rID)
        , ScRefHandler(*static_cast<TBase*>(this), pB, bBindRef)
    {
    }
};

struct ScAnyRefDlgController : ScRefHdlrControllerImpl<SfxModelessDialogController>
{
    ScAnyRefDlgController(SfxBindings* rt1, SfxChildWindow* rt2, weld::Window* rt3, const OUString& rt4, const OString& rt5)
        : ScRefHdlrControllerImpl<SfxModelessDialogController>(rt1, rt2, rt3, rt4, rt5)
    {
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
