/*************************************************************************
 *
 *  $RCSfile: anyrefdg.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:44:57 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef SC_ANYREFDG_HXX
#define SC_ANYREFDG_HXX

#ifndef _IMAGEBTN_HXX //autogen
#include <vcl/imagebtn.hxx>
#endif
#ifndef _EDIT_HXX //autogen
#include <vcl/edit.hxx>
#endif
#ifndef _ACCEL_HXX //autogen
#include <vcl/accel.hxx>
#endif
#ifndef _BASEDLGS_HXX //autogen
#include <sfx2/basedlgs.hxx>
#endif

class SfxObjectShell;
class ScRange;
class ScDocument;
class ScTabViewShell;
class ScAnyRefDlg;
class ScFormulaCell;
class ScCompiler;

//============================================================================

class ScRefEdit : public Edit
{
private:

    Timer           aTimer;
    ScAnyRefDlg*    pAnyRefDlg;         // Dialog

    DECL_LINK( UpdateHdl, Timer*);

public:
                    ScRefEdit( ScAnyRefDlg* pParent, const ResId& rResId);
                    ScRefEdit( Window *pParent, const ResId& rResId);
                    ~ScRefEdit();

    void            SetRefString( const XubString& rStr );
    virtual void    SetText( const XubString& rStr );

    virtual void    GetFocus();
    virtual void    LoseFocus();
    virtual void    Modify();
    void            StartUpdateData();

    void            SetRefDialog(ScAnyRefDlg *pDlg);
    ScAnyRefDlg*    GetRefDialog() {return pAnyRefDlg;}

};

class ScRefButton : public ImageButton
{
public:
                    ScRefButton( ScAnyRefDlg* pParent, const ResId& rResId, ScRefEdit* pEdit );
                    ScRefButton( Window *pParent, const ResId& rResId);

    void            SetReferences( ScAnyRefDlg* pRefDlg,ScRefEdit* pEdit );

    void            SetStartImage();
    void            SetEndImage();
    void            DoRef() { Click(); }
    virtual void    GetFocus();
    virtual void    LoseFocus();


protected:
    virtual void    Click();

private:
    Image           aImgRefStart;   // Symbol fuer Ref.-Eingabe starten
    Image           aImgRefDone;    // Symbol fuer Ref.-Eingabe beenden
    ScAnyRefDlg*    pDlg;           // Dialog
    ScRefEdit*      pRefEdit;       // zugeordnetes Edit-Control
};


//============================================================================
class ScAnyRefDlg : public SfxModelessDialog
{
friend  ScRefButton;

private:
    Edit*           pRefEdit;               // aktives Eingabefeld
    ScRefButton*    pRefBtn;                // Button dazu
    String          sOldDialogText;         // Originaltitel des Dialogfensters
    Size            aOldDialogSize;         // Originalgroesse Dialogfenster
    Point           aOldEditPos;            // Originalposition des Eingabefeldes
    Size            aOldEditSize;           // Originalgroesse des Eingabefeldes
    Point           aOldButtonPos;          // Originalpositiuon des Buttons
    BOOL*           pHiddenMarks;           // Merkfeld fuer versteckte Controls
    Accelerator*    pAccel;                 // fuer Enter/Escape
    BOOL            bAccInserted;
    BOOL            bHighLightRef;
    BOOL            bEnableColorRef;
    ScFormulaCell*  pRefCell;
    ScCompiler*     pRefComp;
    Window*         pActiveWin;
    Timer           aTimer;
    String          aDocName;               // document on which the dialog was opened

    DECL_LINK( UpdateFocusHdl, Timer*);
    DECL_LINK( AccelSelectHdl, Accelerator * );

protected:
    BOOL            DoClose( USHORT nId );

    void            EnableSpreadsheets(BOOL bFlag=TRUE, BOOL bChilds=TRUE);

    virtual long    PreNotify( NotifyEvent& rNEvt );

    virtual void    RefInputStart( ScRefEdit* pEdit, ScRefButton* pButton = NULL );
    virtual void    RefInputDone( BOOL bForced = FALSE );
    void            ShowSimpleReference( const XubString& rStr );
    void            ShowFormulaReference( const XubString& rStr );


public:
                    ScAnyRefDlg( SfxBindings* pB, SfxChildWindow* pCW,
                                 Window* pParent, USHORT nResId);
    virtual         ~ScAnyRefDlg();

    virtual void    SetReference( const ScRange& rRef, ScDocument* pDoc ) = 0;
    virtual void    AddRefEntry();

    virtual BOOL    IsRefInputMode() const;
    virtual BOOL    IsTableLocked() const;
    virtual BOOL    IsDocAllowed(SfxObjectShell* pDocSh) const;

    void            ShowReference( const XubString& rStr );
    void            HideReference();

    void            ViewShellChanged(ScTabViewShell* pScViewShell);
    void            SwitchToDocument();

    virtual void    SetActive() = 0;
//  virtual BOOL    Close();
    virtual void    StateChanged( StateChangedType nStateChange );
};



#endif // SC_ANYREFDG_HXX

