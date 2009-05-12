/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: anyrefdg.hxx,v $
 * $Revision: 1.11.32.2 $
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

#ifndef _IMAGEBTN_HXX
#include <vcl/imagebtn.hxx>
#endif
#ifndef _EDIT_HXX
#include <vcl/edit.hxx>
#endif
#ifndef _ACCEL_HXX
#include <vcl/accel.hxx>
#endif
#include <sfx2/basedlgs.hxx>
#include "address.hxx"
#include "formula/funcutl.hxx"
#include "IAnyRefDialog.hxx"
#include <memory>

class SfxObjectShell;
class ScRange;
class ScDocument;
class ScTabViewShell;
class ScAnyRefDlg;
class ScFormulaCell;
class ScCompiler;
class ScRangeList;

class ScFormulaReferenceHelper
{
    IAnyRefDialog*      m_pDlg;
    ::std::auto_ptr<ScFormulaCell>      pRefCell;
    ::std::auto_ptr<ScCompiler>         pRefComp;
    formula::RefEdit*    pRefEdit;               // aktives Eingabefeld
    formula::RefButton*  pRefBtn;                // Button dazu
    Window*             m_pWindow;
    SfxBindings*        m_pBindings;
    ::std::auto_ptr<Accelerator>
                        pAccel;                 // fuer Enter/Escape
    BOOL*               pHiddenMarks;           // Merkfeld fuer versteckte Controls
    SCTAB               nRefTab;                // used for ShowReference

    String              sOldDialogText;         // Originaltitel des Dialogfensters
    Size                aOldDialogSize;         // Originalgroesse Dialogfenster
    Point               aOldEditPos;            // Originalposition des Eingabefeldes
    Size                aOldEditSize;           // Originalgroesse des Eingabefeldes
    Point               aOldButtonPos;          // Originalpositiuon des Buttons

    BOOL                bEnableColorRef;
    BOOL                bHighLightRef;
    BOOL                bAccInserted;

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
    void                HideReference( BOOL bDoneRefMode = TRUE );
    void                RefInputStart( formula::RefEdit* pEdit, formula::RefButton* pButton = NULL );
    void                RefInputDone( BOOL bForced = FALSE );
    void                ToggleCollapsed( formula::RefEdit* pEdit, formula::RefButton* pButton = NULL );

    inline void         SetWindow(Window* _pWindow) { m_pWindow = _pWindow; }
    BOOL                DoClose( USHORT nId );
    void                SetDispatcherLock( BOOL bLock );
    void                EnableSpreadsheets( BOOL bFlag = TRUE, BOOL bChilds = TRUE );
    void                ViewShellChanged( ScTabViewShell* pScViewShell );

    static              void enableInput(BOOL _bInput);
};
//============================================================================

class ScAnyRefDlg : public SfxModelessDialog,
                    public IAnyRefDialog
{
    friend class        formula::RefButton;
    friend class        formula::RefEdit;

private:
    ScFormulaReferenceHelper
                        m_aHelper;
    SfxBindings*        pMyBindings;

    Window*             pActiveWin;
    Timer               aTimer;
    String              aDocName;               // document on which the dialog was opened

    DECL_LINK( UpdateFocusHdl, Timer* );


protected:
    virtual BOOL        DoClose( USHORT nId );

    void                EnableSpreadsheets( BOOL bFlag = TRUE, BOOL bChilds = TRUE );
    void                SetDispatcherLock( BOOL bLock );

    virtual long        PreNotify( NotifyEvent& rNEvt );

    virtual void        RefInputStart( formula::RefEdit* pEdit, formula::RefButton* pButton = NULL );
    virtual void        RefInputDone( BOOL bForced = FALSE );
    void                ShowSimpleReference( const XubString& rStr );
    void                ShowFormulaReference( const XubString& rStr );

    bool                ParseWithNames( ScRangeList& rRanges, const String& rStr, ScDocument* pDoc );

public:
                        ScAnyRefDlg( SfxBindings* pB, SfxChildWindow* pCW,
                                     Window* pParent, USHORT nResId);
    virtual             ~ScAnyRefDlg();

    virtual void        SetReference( const ScRange& rRef, ScDocument* pDoc ) = 0;
    virtual void        AddRefEntry();

    virtual BOOL        IsRefInputMode() const;
    virtual BOOL        IsTableLocked() const;
    virtual BOOL        IsDocAllowed( SfxObjectShell* pDocSh ) const;

    virtual void        ShowReference( const XubString& rStr );
    virtual void        HideReference( BOOL bDoneRefMode = TRUE );

    virtual void        ToggleCollapsed( formula::RefEdit* pEdit, formula::RefButton* pButton = NULL );
    virtual void        ReleaseFocus( formula::RefEdit* pEdit, formula::RefButton* pButton = NULL );

    virtual void        ViewShellChanged( ScTabViewShell* pScViewShell );
    void                SwitchToDocument();
    SfxBindings&        GetBindings();

    virtual void        SetActive() = 0;
//  virtual BOOL        Close();
    virtual void        StateChanged( StateChangedType nStateChange );
};


//============================================================================

#endif // SC_ANYREFDG_HXX

