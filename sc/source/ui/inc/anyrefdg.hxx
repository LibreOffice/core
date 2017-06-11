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

#ifndef INCLUDED_SC_SOURCE_UI_INC_ANYREFDG_HXX
#define INCLUDED_SC_SOURCE_UI_INC_ANYREFDG_HXX

#include <vcl/button.hxx>
#include <vcl/edit.hxx>
#include <vcl/accel.hxx>
#include <sfx2/basedlgs.hxx>
#include <sfx2/tabdlg.hxx>
#include "address.hxx"
#include "compiler.hxx"
#include <formula/funcutl.hxx>
#include "IAnyRefDialog.hxx"
#include "scresid.hxx"
#include "scmod.hxx"

#include <memory>

class SfxObjectShell;
class ScRange;
class ScDocument;
class ScTabViewShell;
class ScRefHandler;
class ScRangeList;

class ScFormulaReferenceHelper
{
    IAnyRefDialog*      m_pDlg;
    ::std::unique_ptr<ScCompiler>         pRefComp;
    VclPtr<formula::RefEdit>    pRefEdit;               // active input field
    VclPtr<formula::RefButton>  pRefBtn;                // associated button
    VclPtr<vcl::Window>         m_pWindow;
    SfxBindings*        m_pBindings;
    ::std::unique_ptr<Accelerator>
                        pAccel;                 // for Enter/Escape
    ::std::vector<VclPtr<vcl::Window> > m_aHiddenWidgets;    // vector of hidden Controls
    sal_Int32           m_nOldBorderWidth;      // border width for expanded dialog
    SCTAB               nRefTab;                // used for ShowReference

    OUString            sOldDialogText;         // Original title of the dialog window
    Size                aOldDialogSize;         // Original size of the dialog window
    Point               aOldEditPos;            // Original position of the input field
    Size                aOldEditSize;           // Original size of the input field
    long                mnOldEditWidthReq;
    Point               aOldButtonPos;          // Original position of the button
    VclPtr<vcl::Window> mpOldEditParent;        // Original parent of the edit field and the button
    bool                mbOldDlgLayoutEnabled;  // Original layout state of parent dialog
    bool                mbOldEditParentLayoutEnabled;  // Original layout state of edit widget parent

    bool                bEnableColorRef;
    bool                bHighlightRef;
    bool                bAccInserted;

    DECL_LINK( AccelSelectHdl, Accelerator&, void );

public:
    ScFormulaReferenceHelper(IAnyRefDialog* _pDlg,SfxBindings* _pBindings);
    ~ScFormulaReferenceHelper();
    void dispose();

    void                ShowSimpleReference(const OUString& rStr);
    void                ShowFormulaReference(const OUString& rStr);
    bool                ParseWithNames( ScRangeList& rRanges, const OUString& rStr, ScDocument* pDoc );
    void                Init();

    void                ShowReference(const OUString& rStr);
    void                ReleaseFocus( formula::RefEdit* pEdit );
    void                HideReference( bool bDoneRefMode = true );
    void                RefInputStart( formula::RefEdit* pEdit, formula::RefButton* pButton );
    void                RefInputDone( bool bForced );
    void                ToggleCollapsed( formula::RefEdit* pEdit, formula::RefButton* pButton );

    void         SetWindow(vcl::Window* _pWindow) { m_pWindow = _pWindow; }
    void                DoClose( sal_uInt16 nId );
    static void         SetDispatcherLock( bool bLock );
    static void         EnableSpreadsheets( bool bFlag = true );
    static void         ViewShellChanged();

    static              void enableInput(bool _bInput);

public:
    static bool         CanInputStart( const formula::RefEdit *pEdit ){ return !!pEdit; }
    bool                CanInputDone( bool bForced ){   return pRefEdit && (bForced || !pRefBtn);   }
};

class SC_DLLPUBLIC ScRefHandler :
                    public IAnyRefDialog
{
    VclPtr<vcl::Window>  m_rWindow;
    bool                 m_bInRefMode;

public:
    friend class        formula::RefButton;
    friend class        formula::RefEdit;

private:
    ScFormulaReferenceHelper
                        m_aHelper;
    SfxBindings*        pMyBindings;

    VclPtr<vcl::Window> pActiveWin;
    Idle                aIdle;
    OUString            aDocName;               // document on which the dialog was opened

    DECL_LINK( UpdateFocusHdl, Timer*, void );

protected:
    void                disposeRefHandler();
    bool                DoClose( sal_uInt16 nId );

    static void         SetDispatcherLock( bool bLock );

    virtual void        RefInputStart( formula::RefEdit* pEdit, formula::RefButton* pButton = nullptr ) override;
    virtual void        RefInputDone( bool bForced = false ) override;

    bool                ParseWithNames( ScRangeList& rRanges, const OUString& rStr, ScDocument* pDoc );

public:
                        ScRefHandler( vcl::Window &rWindow, SfxBindings* pB, bool bBindRef );
    virtual             ~ScRefHandler() override;

    virtual void        SetReference( const ScRange& rRef, ScDocument* pDoc ) override = 0;
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
    static inline bool  CanInputStart( const formula::RefEdit *pEdit );
    inline  bool        CanInputDone( bool bForced );
};

template<  class TWindow, bool bBindRef = true >
class ScRefHdlrImplBase: public TWindow, public ScRefHandler
{
private:
    template<class TBindings, class TChildWindow, class TParentWindow >
    ScRefHdlrImplBase( TBindings* pB, TChildWindow* pCW,
        TParentWindow* pParent, const OUString& rID, const OUString& rUIXMLDescription );

    template<class TParentWindow, class TArg>
    ScRefHdlrImplBase( TParentWindow* pParent, const OUString& rID, const OUString& rUIXMLDescription, const TArg &rArg, SfxBindings *pB );

    virtual ~ScRefHdlrImplBase() override;

    template<class, class, bool> friend struct ScRefHdlrImpl;
};

template<class TWindow, bool bBindRef>
template<class TBindings, class TChildWindow, class TParentWindow>
ScRefHdlrImplBase<TWindow, bBindRef>::ScRefHdlrImplBase( TBindings* pB, TChildWindow* pCW,
                 TParentWindow* pParent, const OUString& rID, const OUString& rUIXMLDescription )
    : TWindow(pB, pCW, pParent, rID, rUIXMLDescription )
    , ScRefHandler( *static_cast<TWindow*>(this), pB, bBindRef )
{
}

template<class TWindow, bool bBindRef >
template<class TParentWindow, class TArg>
ScRefHdlrImplBase<TWindow,bBindRef>::ScRefHdlrImplBase( TParentWindow* pParent, const OUString& rID, const OUString& rUIXMLDescription, const TArg &rArg, SfxBindings *pB )
    : TWindow( pParent, rID, rUIXMLDescription, rArg ),
    ScRefHandler( *static_cast<TWindow*>(this), pB, bBindRef )
{
}

template<class TWindow, bool bBindRef >
ScRefHdlrImplBase<TWindow,bBindRef>::~ScRefHdlrImplBase(){}

template<class TDerived, class TBase, bool bBindRef = true>
struct ScRefHdlrImpl: ScRefHdlrImplBase< TBase, bBindRef >
{
    enum { UNKNOWN_SLOTID = 0U, SLOTID = UNKNOWN_SLOTID };

    template<class T1, class T2, class T3, class T4>
    ScRefHdlrImpl( const T1 & rt1, const T2 & rt2, const T3& rt3, const T4& rt4 )
        : ScRefHdlrImplBase<TBase, bBindRef >(rt1, rt2, rt3, rt4)
    {
        SC_MOD()->RegisterRefWindow( static_cast<sal_uInt16>( TDerived::SLOTID ), this );
    }

    template<class T1, class T2, class T3, class T4, class T5>
    ScRefHdlrImpl( const T1 & rt1, const T2 & rt2, const T3& rt3, const T4& rt4, const T5& rt5 )
        : ScRefHdlrImplBase<TBase, bBindRef >(rt1, rt2, rt3, rt4, rt5)
    {
        SC_MOD()->RegisterRefWindow( static_cast<sal_uInt16>( TDerived::SLOTID ), this );
    }

    virtual void dispose() override
    {
        SC_MOD()->UnregisterRefWindow( static_cast<sal_uInt16>( TDerived::SLOTID ), this );
        ScRefHdlrImplBase<TBase, bBindRef >::disposeRefHandler();
        TBase::dispose();
    }

    ~ScRefHdlrImpl()
    {
        TBase::disposeOnce();
    }
};

struct ScAnyRefDlg : ::ScRefHdlrImpl< ScAnyRefDlg, SfxModelessDialog>
{
    template<class T1, class T2, class T3, class T4, class T5>
    ScAnyRefDlg( const T1 & rt1, const T2 & rt2, const T3& rt3, const T4& rt4, const T5& rt5 )
        : ScRefHdlrImpl< ScAnyRefDlg, SfxModelessDialog>(rt1, rt2, rt3, rt4, rt5)
    {
    }
};

inline bool ScRefHandler::CanInputStart( const formula::RefEdit *pEdit )
{
    return ScFormulaReferenceHelper::CanInputStart( pEdit );
}

inline  bool ScRefHandler::CanInputDone( bool bForced )
{
    return m_aHelper.CanInputDone( bForced );
}

#endif // INCLUDED_SC_SOURCE_UI_INC_ANYREFDG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
