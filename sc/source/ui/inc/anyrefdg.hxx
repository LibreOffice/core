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

#ifndef SC_ANYREFDG_HXX
#define SC_ANYREFDG_HXX

#include <vcl/button.hxx>
#include <vcl/edit.hxx>
#include <vcl/accel.hxx>
#include <sfx2/basedlgs.hxx>
#include <sfx2/tabdlg.hxx>
#include "address.hxx"
#include "compiler.hxx"
#include "formula/funcutl.hxx"
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
    ::std::auto_ptr<ScCompiler>         pRefComp;
    formula::RefEdit*    pRefEdit;               // active input field
    formula::RefButton*  pRefBtn;                // associated button
    Window*             m_pWindow;
    SfxBindings*        m_pBindings;
    ::std::auto_ptr<Accelerator>
                        pAccel;                 // for Enter/Escape
    ::std::vector<Window*> m_aHiddenWidgets;    // vector of hidden Controls
    sal_Int32           m_nOldBorderWidth;      // border width for expanded dialog
    SCTAB               nRefTab;                // used for ShowReference

    String              sOldDialogText;         // Original title of the dialog window
    Size                aOldDialogSize;         // Original size of the dialog window
    Point               aOldEditPos;            // Original position of the input field
    Size                aOldEditSize;           // Original size of the input field
    long                mnOldEditWidthReq;
    Point               aOldButtonPos;          // Original position of the button
    Window*             mpOldEditParent;        // Original parent of the edit field and the button
    bool                mbOldDlgLayoutEnabled;  // Original layout state of parent dialog
    bool                mbOldEditParentLayoutEnabled;  // Original layout state of edit widget parent

    bool                bEnableColorRef;
    bool                bHighLightRef;
    bool                bAccInserted;

    DECL_LINK( AccelSelectHdl, Accelerator* );

public:
    ScFormulaReferenceHelper(IAnyRefDialog* _pDlg,SfxBindings* _pBindings);
    ~ScFormulaReferenceHelper();

    void                ShowSimpleReference(const OUString& rStr);
    void                ShowFormulaReference(const OUString& rStr);
    bool                ParseWithNames( ScRangeList& rRanges, const String& rStr, ScDocument* pDoc );
    void                Init();

    void                ShowReference(const OUString& rStr);
    void                ReleaseFocus( formula::RefEdit* pEdit, formula::RefButton* pButton = NULL );
    void                HideReference( bool bDoneRefMode = true );
    void                RefInputStart( formula::RefEdit* pEdit, formula::RefButton* pButton = NULL );
    void                RefInputDone( bool bForced = false );
    void                ToggleCollapsed( formula::RefEdit* pEdit, formula::RefButton* pButton = NULL );

    inline void         SetWindow(Window* _pWindow) { m_pWindow = _pWindow; }
    bool                DoClose( sal_uInt16 nId );
    void                SetDispatcherLock( bool bLock );
    void                EnableSpreadsheets( bool bFlag = true, bool bChildren = true );
    void                ViewShellChanged();

    static              void enableInput(bool _bInput);

protected:
    Window      *       GetWindow(){ return m_pWindow; }

public:
    bool                CanInputStart( const formula::RefEdit *pEdit ){ return !!pEdit; }
    bool                CanInputDone( bool bForced ){   return pRefEdit && (bForced || !pRefBtn);   }
};

//============================================================================

class SC_DLLPUBLIC ScRefHandler :
                    public IAnyRefDialog
{
    Window &    m_rWindow;
    bool        m_bInRefMode;

public:
    operator Window *(){ return &m_rWindow; }
    Window  * operator ->() { return static_cast<Window *>(*this); }
    friend class        formula::RefButton;
    friend class        formula::RefEdit;

private:
    ScFormulaReferenceHelper
                        m_aHelper;
    SfxBindings*        pMyBindings;

    Window*             pActiveWin;
    Timer               aTimer;
    String              aDocName;               // document on which the dialog was opened

    DECL_LINK( UpdateFocusHdl, void* );


protected:
    virtual sal_Bool        DoClose( sal_uInt16 nId );

    void                SetDispatcherLock( bool bLock );

    virtual void        RefInputStart( formula::RefEdit* pEdit, formula::RefButton* pButton = NULL );
    virtual void        RefInputDone( sal_Bool bForced = false );
    void                ShowSimpleReference(const OUString& rStr);
    void                ShowFormulaReference(const OUString& rStr);

    bool                ParseWithNames( ScRangeList& rRanges, const String& rStr, ScDocument* pDoc );

    void preNotify(const NotifyEvent& rEvent, const bool bBindRef);
    void stateChanged(const StateChangedType nStateChange, const bool bBindRef);

public:
                        ScRefHandler( Window &rWindow, SfxBindings* pB, bool bBindRef );
    virtual             ~ScRefHandler();

    virtual void        SetReference( const ScRange& rRef, ScDocument* pDoc ) = 0;
    virtual void        AddRefEntry();

    virtual sal_Bool        IsRefInputMode() const;
    virtual sal_Bool        IsTableLocked() const;
    virtual sal_Bool        IsDocAllowed( SfxObjectShell* pDocSh ) const;

    virtual void        ShowReference(const OUString& rStr);
    virtual void        HideReference( sal_Bool bDoneRefMode = sal_True );

    virtual void        ToggleCollapsed( formula::RefEdit* pEdit, formula::RefButton* pButton = NULL );
    virtual void        ReleaseFocus( formula::RefEdit* pEdit, formula::RefButton* pButton = NULL );

    virtual void        ViewShellChanged();
    void                SwitchToDocument();

    virtual void        SetActive() = 0;

public:
    bool                EnterRefMode();
    bool                LeaveRefMode();
    inline  bool        CanInputStart( const formula::RefEdit *pEdit );
    inline  bool        CanInputDone( sal_Bool bForced );
};

//============================================================================


class ScRefHdlModalImpl : public ModalDialog, public ScRefHandler
{
public:

    virtual long        PreNotify( NotifyEvent& rNEvt );
    virtual void        StateChanged( StateChangedType nStateChange );
protected:
    ScRefHdlModalImpl( Window* pParent, ResId& rResId );

private:
};

template<  class TWindow, bool bBindRef = true >
class ScRefHdlrImplBase: public TWindow, public ScRefHandler
{
public:
    virtual long        PreNotify( NotifyEvent& rNEvt );
    virtual void        StateChanged( StateChangedType nStateChange );

private:
    template<class TBindings, class TChildWindow, class TParentWindow, class TResId>
    ScRefHdlrImplBase( TBindings* pB, TChildWindow* pCW,
        TParentWindow* pParent, TResId nResId );

    template<class TBindings, class TChildWindow, class TParentWindow >
    ScRefHdlrImplBase( TBindings* pB, TChildWindow* pCW,
        TParentWindow* pParent, const OString& rID, const OUString& rUIXMLDescription );

    template<class TParentWindow, class TResId, class TArg>
    ScRefHdlrImplBase( TParentWindow* pParent, TResId nResId, const TArg &rArg, SfxBindings *pB = NULL );

    ~ScRefHdlrImplBase();

    template<class, class, bool> friend struct ScRefHdlrImpl;
};

template<class TWindow, bool bBindRef>
template<class TBindings, class TChildWindow, class TParentWindow, class TResId>
ScRefHdlrImplBase<TWindow, bBindRef>::ScRefHdlrImplBase( TBindings* pB, TChildWindow* pCW,
                 TParentWindow* pParent, TResId nResId)
    : TWindow(pB, pCW, pParent, ScResId(static_cast<sal_uInt16>( nResId ) ) )
    , ScRefHandler( *static_cast<TWindow*>(this), pB, bBindRef )
{
}

template<class TWindow, bool bBindRef>
template<class TBindings, class TChildWindow, class TParentWindow>
ScRefHdlrImplBase<TWindow, bBindRef>::ScRefHdlrImplBase( TBindings* pB, TChildWindow* pCW,
                 TParentWindow* pParent, const OString& rID, const OUString& rUIXMLDescription )
    : TWindow(pB, pCW, pParent, rID, rUIXMLDescription )
    , ScRefHandler( *static_cast<TWindow*>(this), pB, bBindRef )
{
}

template<class TWindow, bool bBindRef >
template<class TParentWindow, class TResId, class TArg>
ScRefHdlrImplBase<TWindow,bBindRef>::ScRefHdlrImplBase( TParentWindow* pParent, TResId nResIdP, const TArg &rArg, SfxBindings *pB )
    : TWindow( pParent, ScResId(static_cast<sal_uInt16>( nResIdP )), rArg ),
    ScRefHandler( *static_cast<TWindow*>(this), pB, bBindRef )
{
}

template<class TWindow, bool bBindRef >
ScRefHdlrImplBase<TWindow,bBindRef>::~ScRefHdlrImplBase(){}

template<class TWindow, bool bBindRef>
long ScRefHdlrImplBase<TWindow, bBindRef>::PreNotify( NotifyEvent& rNEvt )
{
    ScRefHandler::preNotify( rNEvt, bBindRef );
    return TWindow::PreNotify( rNEvt );
}

template<class TWindow, bool bBindRef>
void ScRefHdlrImplBase<TWindow, bBindRef>::StateChanged( StateChangedType nStateChange )
{
    TWindow::StateChanged( nStateChange );
    ScRefHandler::stateChanged( nStateChange, bBindRef );
}

class ScAnyRefModalDlg : public ScRefHdlModalImpl
{
public:
    ScAnyRefModalDlg(Window* pParent, ResId rResId);
};

//============================================================================
template<class TDerived, class TBase, bool bBindRef = true>
struct ScRefHdlrImpl: ScRefHdlrImplBase< TBase, bBindRef >
{
    enum { UNKNOWN_SLOTID = 0U, SLOTID = UNKNOWN_SLOTID };

    template<class T1, class T2, class T3, class T4>
    ScRefHdlrImpl( const T1 & rt1, const T2 & rt2, const T3& rt3, const T4& rt4 )
        : ScRefHdlrImplBase<TBase, bBindRef >(rt1, rt2, rt3, rt4)
    {
        SC_MOD()->RegisterRefWindow( static_cast<sal_uInt16>( static_cast<TDerived*>(this)->SLOTID ), this );
    }

    template<class T1, class T2, class T3, class T4, class T5>
    ScRefHdlrImpl( const T1 & rt1, const T2 & rt2, const T3& rt3, const T4& rt4, const T5& rt5 )
        : ScRefHdlrImplBase<TBase, bBindRef >(rt1, rt2, rt3, rt4, rt5)
    {
        SC_MOD()->RegisterRefWindow( static_cast<sal_uInt16>( static_cast<TDerived*>(this)->SLOTID ), this );
    }

    ~ScRefHdlrImpl()
    {
        SC_MOD()->UnregisterRefWindow( static_cast<sal_uInt16>( static_cast<TDerived*>(this)->SLOTID ), this );
    }
};
//============================================================================
struct ScAnyRefDlg : ::ScRefHdlrImpl< ScAnyRefDlg, SfxModelessDialog>
{
    template<class T1, class T2, class T3, class T4>
    ScAnyRefDlg( const T1 & rt1, const T2 & rt2, const T3& rt3, const T4& rt4 )
        : ScRefHdlrImpl< ScAnyRefDlg, SfxModelessDialog>(rt1, rt2, rt3, rt4)
    {
    }

    template<class T1, class T2, class T3, class T4, class T5>
    ScAnyRefDlg( const T1 & rt1, const T2 & rt2, const T3& rt3, const T4& rt4, const T5& rt5 )
        : ScRefHdlrImpl< ScAnyRefDlg, SfxModelessDialog>(rt1, rt2, rt3, rt4, rt5)
    {
    }
};
//============================================================================

inline bool ScRefHandler::CanInputStart( const formula::RefEdit *pEdit )
{
    return m_aHelper.CanInputStart( pEdit );
}

inline  bool ScRefHandler::CanInputDone( sal_Bool bForced )
{
    return m_aHelper.CanInputDone( bForced );
}

#endif // SC_ANYREFDG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
