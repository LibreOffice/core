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

#ifndef SC_INPUTWIN_HXX
#define SC_INPUTWIN_HXX

#include <vector>
#include <vcl/toolbox.hxx>
#include <sfx2/childwin.hxx>
#include <svl/lstner.hxx>
#include <vcl/combobox.hxx>
#include <vcl/window.hxx>
#include <svtools/transfer.hxx>

class ScEditEngineDefaulter;
class EditView;
struct ESelection;
class ScInputHandler;
class ScAccessibleEditLineTextData;
struct EENotify;
class ScRangeList;

//========================================================================

class ScTextWnd : public Window, public DragSourceHelper        // edit window
{
public:
                    ScTextWnd( Window* pParent );
    virtual         ~ScTextWnd();

    void            SetTextString( const String& rString );
    const String&   GetTextString() const;

    sal_Bool            IsInputActive();
    EditView*       GetEditView();

                        // fuer FunktionsAutopiloten
    void            MakeDialogEditView();

    void            StartEditEngine();
    void            StopEditEngine( sal_Bool bAll );

    virtual void    DataChanged( const DataChangedEvent& rDCEvt );

    void            SetFormulaMode( sal_Bool bSet );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > CreateAccessible();

    void            InsertAccessibleTextData( ScAccessibleEditLineTextData& rTextData );
    void            RemoveAccessibleTextData( ScAccessibleEditLineTextData& rTextData );

    DECL_LINK( NotifyHdl, EENotify* );

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

    virtual void    StartDrag( sal_Int8 nAction, const Point& rPosPixel );

    virtual String  GetText() const;

private:
    void            ImplInitSettings();
    void            UpdateAutoCorrFlag();

private:
    typedef ::std::vector< ScAccessibleEditLineTextData* > AccTextDataVector;

    String      aString;
    Font        aTextFont;
    ScEditEngineDefaulter*  pEditEngine;            // erst bei Bedarf angelegt
    EditView*   pEditView;
    AccTextDataVector maAccTextDatas;   // #i105267# text datas may be cloned, remember all copies
    sal_Bool        bIsRTL;
    sal_Bool        bIsInsertMode;
    sal_Bool        bFormulaMode;

    // #102710#; this flag should be true if a key input or a command is handled
    // it prevents the call of InputChanged in the ModifyHandler of the EditEngine
    sal_Bool        bInputMode;
};

//========================================================================

class ScPosWnd : public ComboBox, public SfxListener        // Positionsanzeige
{
private:
    String          aPosStr;
    Accelerator*    pAccel;
    sal_uLong           nTipVisible;
    sal_Bool            bFormulaMode;
    sal_Bool            bTopHadFocus;

public:
                    ScPosWnd( Window* pParent );
    virtual         ~ScPosWnd();

    void            SetPos( const String& rPosStr );        // angezeigter Text
    void            SetFormulaMode( sal_Bool bSet );

protected:
    virtual void    Select();
    virtual void    Modify();

    virtual long    Notify( NotifyEvent& rNEvt );

    virtual void    Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

private:
    void            FillRangeNames();
    void            FillFunctions();
    void            DoEnter();
    void            HideTip();

    void            ReleaseFocus_Impl();
};

//========================================================================

class ScInputWindow : public ToolBox                        // Parent-Toolbox
{
public:
                    ScInputWindow( Window* pParent, SfxBindings* pBind );
    virtual         ~ScInputWindow();

    virtual void    Resize();
    virtual void    Select();

    void            SetFuncString( const String& rString, sal_Bool bDoEdit = sal_True );
    void            SetPosString( const String& rStr );
    void            SetTextString( const String& rString );

    void            SetOkCancelMode();
    void            SetSumAssignMode();
    void            EnableButtons( sal_Bool bEnable = sal_True );

    void            SetFormulaMode( sal_Bool bSet );

    sal_Bool            IsInputActive();
    EditView*       GetEditView();
//UNUSED2008-05  EditView*      ActivateEdit( const String&     rText,
//UNUSED2008-05                                const ESelection& rSel );

    void            TextGrabFocus();
    void            TextInvalidate();
    void            SwitchToTextWin();

    void            PosGrabFocus();

    // Fuer FunktionsAutopiloten
    void            MakeDialogEditView();

    void            StopEditEngine( sal_Bool bAll );

    void            SetInputHandler( ScInputHandler* pNew );

    ScInputHandler* GetInputHandler(){ return pInputHdl;}

    void            StateChanged( StateChangedType nType );
    virtual void    DataChanged( const DataChangedEvent& rDCEvt );


protected:
    virtual void    SetText( const String& rString );
    virtual String  GetText() const;

    sal_Bool        UseSubTotal( ScRangeList* pRangeList ) const;

private:
    ScPosWnd        aWndPos;
    ScTextWnd       aTextWindow;
    ScInputHandler* pInputHdl;
    SfxBindings*    pBindings;
    String          aTextOk;
    String          aTextCancel;
    String          aTextSum;
    String          aTextEqual;
    sal_Bool            bIsOkCancelMode;
};

//==================================================================

class ScInputWindowWrapper : public SfxChildWindow
{
public:
            ScInputWindowWrapper( Window*           pParent,
                                  sal_uInt16            nId,
                                  SfxBindings*      pBindings,
                                  SfxChildWinInfo*  pInfo );

    SFX_DECL_CHILDWINDOW(ScInputWindowWrapper);
};


#endif

