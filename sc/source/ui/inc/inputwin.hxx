/*************************************************************************
 *
 *  $RCSfile: inputwin.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:44:59 $
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

#ifndef SC_INPUTWIN_HXX
#define SC_INPUTWIN_HXX


#ifndef _TOOLBOX_HXX //autogen
#include <vcl/toolbox.hxx>
#endif
#ifndef _SFX_CHILDWIN_HXX //autogen
#include <sfx2/childwin.hxx>
#endif
#ifndef _SFXLSTNER_HXX //autogen
#include <svtools/lstner.hxx>
#endif
#ifndef _COMBOBOX_HXX //autogen
#include <vcl/combobox.hxx>
#endif
#ifndef _WINDOW_HXX //autogen
#include <vcl/window.hxx>
#endif

class ScEditEngineDefaulter;
class EditView;
struct ESelection;
class ScInputHandler;

//========================================================================

class ScTextWnd : public Window                         // Edit-Fenster
{
public:
                    ScTextWnd( Window* pParent );
    virtual         ~ScTextWnd();

    void            SetTextString( const String& rString );
    const String&   GetTextString() const;

    BOOL            IsActive();
    EditView*       GetEditView();

                        // fuer FunktionsAutopiloten
    void            MakeDialogEditView();

    void            StartEditEngine();
    void            StopEditEngine();

    virtual void    DataChanged( const DataChangedEvent& rDCEvt );

    void            SetFormulaMode( BOOL bSet );

protected:
    virtual void    Paint( const Rectangle& rRec );
    virtual void    Resize();

    virtual void    MouseMove( const MouseEvent& rMEvt );
    virtual void    MouseButtonDown( const MouseEvent& rMEvt );
    virtual void    MouseButtonUp( const MouseEvent& rMEvt );
    virtual void    Command( const CommandEvent& rCEvt );
    virtual void    KeyInput(const KeyEvent& rKEvt);
    virtual void    GetFocus();
    virtual void    LoseFocus();

    virtual String  GetText() const;

private:
    void            ImplInitSettings();
    void            UpdateAutoCorrFlag();

private:
    String      aString;
    Font        aTextFont;
    ScEditEngineDefaulter*  pEditEngine;            // erst bei Bedarf angelegt
    EditView*   pEditView;
    BOOL        bIsInsertMode;
    BOOL        bFormulaMode;
};

//========================================================================

class ScPosWnd : public ComboBox, public SfxListener        // Positionsanzeige
{
private:
    String          aPosStr;
    Accelerator*    pAccel;
    BOOL            bFormulaMode;
    BOOL            bTopHadFocus;

public:
                    ScPosWnd( Window* pParent );
    virtual         ~ScPosWnd();

    void            SetPos( const String& rPosStr );        // angezeigter Text
    void            SetFormulaMode( BOOL bSet );

protected:
    virtual void    Select();

#ifdef VCL
    virtual long    Notify( NotifyEvent& rNEvt );
#else
    virtual void    GetFocus();
    virtual void    LoseFocus();
#endif

    virtual void    SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                            const SfxHint& rHint, const TypeId& rHintType );

private:
    void            FillRangeNames();
    void            FillFunctions();
    void            DoEnter();

    void            ReleaseFocus_Impl();
    DECL_LINK( AccelSelectHdl, Accelerator * );
};

//========================================================================

class ScInputWindow : public ToolBox                        // Parent-Toolbox
{
public:
                    ScInputWindow( Window* pParent );
    virtual         ~ScInputWindow();

    virtual void    Resize();
    virtual void    Select();

    void            SetFuncString( const String& rString, BOOL bDoEdit = TRUE );
    void            SetPosString( const String& rStr );
    void            SetTextString( const String& rString );
    const String&   GetTextString();

    void            SetOkCancelMode();
    void            SetSumAssignMode();
    void            EnableButtons( BOOL bEnable = TRUE );

    void            SetFormulaMode( BOOL bSet );

    BOOL            IsActive();
    EditView*       GetEditView();
    EditView*       ActivateEdit( const String&     rText,
                                  const ESelection& rSel );

    void            TextGrabFocus();
    void            TextInvalidate();

    void            PosGrabFocus();

    // Fuer FunktionsAutopiloten
    void            MakeDialogEditView();

    void            StopEditEngine();

    void            SetInputHandler( ScInputHandler* pNew );

    ScInputHandler* GetInputHandler(){ return pInputHdl;}

    void            StateChanged( StateChangedType nType );


protected:
    virtual void    SetText( const String& rString );
    virtual String  GetText() const;

private:
    ScPosWnd        aWndPos;
    ScTextWnd       aTextWindow;
    ScInputHandler* pInputHdl;
    String          aTextOk;
    String          aTextCancel;
    String          aTextSum;
    String          aTextEqual;
    BOOL            bIsOkCancelMode;
};

//==================================================================

class ScInputWindowWrapper : public SfxChildWindow
{
public:
            ScInputWindowWrapper( Window*           pParent,
                                  USHORT            nId,
                                  SfxBindings*      pBindings,
                                  SfxChildWinInfo*  pInfo );

    SFX_DECL_CHILDWINDOW(ScInputWindowWrapper);
};


#endif

