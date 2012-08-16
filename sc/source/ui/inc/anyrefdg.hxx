/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef SC_ANYREFDG_HXX
#define SC_ANYREFDG_HXX

#include <vcl/button.hxx>
#include <vcl/edit.hxx>
#include <vcl/accel.hxx>
#include <sfx2/basedlgs.hxx>
#include <sfx2/tabdlg.hxx>
#include "address.hxx"
#include "cell.hxx"
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
    ::std::auto_ptr<ScFormulaCell>      pRefCell;
    ::std::auto_ptr<ScCompiler>         pRefComp;
    formula::RefEdit*    pRefEdit;               // active input field
    formula::RefButton*  pRefBtn;                // associated button
    Window*             m_pWindow;
    SfxBindings*        m_pBindings;
    ::std::auto_ptr<Accelerator>
                        pAccel;                 // for Enter/Escape
    sal_Bool*               pHiddenMarks;           // Mark field for hidden Controls
    SCTAB               nRefTab;                // used for ShowReference

    String              sOldDialogText;         // Original title of the dialog window
    Size                aOldDialogSize;         // Original size of the dialog window
    Point               aOldEditPos;            // Original position of the input field
    Size                aOldEditSize;           // Original size of the input field
    Point               aOldButtonPos;          // Original position of the button

    sal_Bool                bEnableColorRef;
    sal_Bool                bHighLightRef;
    sal_Bool                bAccInserted;

    DECL_LINK( AccelSelectHdl, Accelerator* );

public:
    ScFormulaReferenceHelper(IAnyRefDialog* _pDlg,SfxBindings* _pBindings);
    ~ScFormulaReferenceHelper();

    void                ShowSimpleReference( const XubString& rStr );
    void                ShowFormulaReference( const XubString& rStr );
    bool                ParseWithNames( ScRangeList& rRanges, const String& rStr, ScDocument* pDoc );
    void                Init();

    void                ShowReference( const XubString& rStr );
    void                ReleaseFocus( formula::RefEdit* pEdit, formula::RefButton* pButton = NULL );
    void                HideReference( sal_Bool bDoneRefMode = sal_True );
    void                RefInputStart( formula::RefEdit* pEdit, formula::RefButton* pButton = NULL );
    void                RefInputDone( sal_Bool bForced = false );
    void                ToggleCollapsed( formula::RefEdit* pEdit, formula::RefButton* pButton = NULL );

    inline void         SetWindow(Window* _pWindow) { m_pWindow = _pWindow; }
    sal_Bool                DoClose( sal_uInt16 nId );
    void                SetDispatcherLock( sal_Bool bLock );
    void                EnableSpreadsheets( sal_Bool bFlag = sal_True, sal_Bool bChildren = sal_True );
    void                ViewShellChanged( ScTabViewShell* pScViewShell );

    static              void enableInput(sal_Bool _bInput);

protected:
    Window      *       GetWindow(){ return m_pWindow; }

public:
    bool                CanInputStart( const formula::RefEdit *pEdit ){ return !!pEdit; }
    bool                CanInputDone( sal_Bool bForced ){   return pRefEdit && (bForced || !pRefBtn);   }
};

//============================================================================

class SC_DLLPUBLIC ScRefHandler : //public SfxModelessDialog,
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

    void                SetDispatcherLock( sal_Bool bLock );

    //Overwrite TWindow will implemented by ScRefHdlrImplBase
    //virtual long        PreNotify( NotifyEvent& rNEvt );

    virtual void        RefInputStart( formula::RefEdit* pEdit, formula::RefButton* pButton = NULL );
    virtual void        RefInputDone( sal_Bool bForced = false );
    void                ShowSimpleReference( const XubString& rStr );
    void                ShowFormulaReference( const XubString& rStr );

    bool                ParseWithNames( ScRangeList& rRanges, const String& rStr, ScDocument* pDoc );

    void preNotify(const NotifyEvent& rEvent, const bool bBindRef);
    void stateChanged(const StateChangedType nStateChange, const bool bBindRef);

public:
                        ScRefHandler( Window &rWindow, SfxBindings* pB/*, SfxChildWindow* pCW,
                                     Window* pParent, sal_uInt16 nResId*/, bool bBindRef );
    virtual             ~ScRefHandler();

    virtual void        SetReference( const ScRange& rRef, ScDocument* pDoc ) = 0;
    virtual void        AddRefEntry();

    virtual sal_Bool        IsRefInputMode() const;
    virtual sal_Bool        IsTableLocked() const;
    virtual sal_Bool        IsDocAllowed( SfxObjectShell* pDocSh ) const;

    virtual void        ShowReference( const XubString& rStr );
    virtual void        HideReference( sal_Bool bDoneRefMode = sal_True );

    virtual void        ToggleCollapsed( formula::RefEdit* pEdit, formula::RefButton* pButton = NULL );
    virtual void        ReleaseFocus( formula::RefEdit* pEdit, formula::RefButton* pButton = NULL );

    virtual void        ViewShellChanged( ScTabViewShell* pScViewShell );
    void                SwitchToDocument();
    //SfxBindings&        GetBindings();

    virtual void        SetActive() = 0;
//  virtual sal_Bool        Close();
    //Overwrite TWindow will implemented by ScRefHdlrImplBase
    //virtual void        StateChanged( StateChangedType nStateChange );

public:
    bool                EnterRefMode();
    bool                LeaveRefMode();
    inline  bool        CanInputStart( const formula::RefEdit *pEdit );
    inline  bool        CanInputDone( sal_Bool bForced );
};

//============================================================================

template<  class TWindow, bool bBindRef = true >
class ScRefHdlrImplBase:public TWindow, public ScRefHandler
{
public:
    //Overwrite TWindow
    virtual long        PreNotify( NotifyEvent& rNEvt );
    virtual void        StateChanged( StateChangedType nStateChange );

private:
    template<class TBindings, class TChildWindow, class TParentWindow, class TResId>
    ScRefHdlrImplBase( TBindings* pB, TChildWindow* pCW,
        TParentWindow* pParent, TResId nResId);

    template<class TParentWindow, class TResId, class TArg>
    ScRefHdlrImplBase( TParentWindow* pParent, TResId nResId, const TArg &rArg, SfxBindings *pB = NULL );

    ~ScRefHdlrImplBase();

    template<class, class, bool> friend struct ScRefHdlrImpl;
};

template<class TWindow, bool bBindRef>
template<class TBindings, class TChildWindow, class TParentWindow, class TResId>
ScRefHdlrImplBase<TWindow, bBindRef>::ScRefHdlrImplBase( TBindings* pB, TChildWindow* pCW,
                 TParentWindow* pParent, TResId nResId):TWindow(pB, pCW, pParent, ScResId(static_cast<sal_uInt16>( nResId ) ) ), ScRefHandler( *static_cast<TWindow*>(this), pB, bBindRef ){}

template<class TWindow, bool bBindRef >
template<class TParentWindow, class TResId, class TArg>
ScRefHdlrImplBase<TWindow,bBindRef>::ScRefHdlrImplBase( TParentWindow* pParent, TResId nResIdP, const TArg &rArg, SfxBindings *pB /*= NULL*/ )
:TWindow( pParent, ScResId(static_cast<sal_uInt16>( nResIdP )), rArg ), ScRefHandler( *static_cast<TWindow*>(this), pB, bBindRef ){}

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

//============================================================================
template<class TDerived, class TBase, bool bBindRef = true>
struct ScRefHdlrImpl: ScRefHdlrImplBase<TBase, bBindRef >
{
    enum { UNKNOWN_SLOTID = 0U, SLOTID = UNKNOWN_SLOTID };

    template<class T1, class T2, class T3, class T4>
    ScRefHdlrImpl( const T1 & rt1, const T2 & rt2, const T3 & rt3, const T4 & rt4 ):ScRefHdlrImplBase<TBase, bBindRef >(rt1, rt2, rt3, rt4 )
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
    ScAnyRefDlg( const T1 & rt1, const T2 & rt2, const T3 & rt3, const T4 & rt4 ):ScRefHdlrImpl< ScAnyRefDlg, SfxModelessDialog>(rt1, rt2, rt3, rt4){}
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
