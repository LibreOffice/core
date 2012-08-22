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

#ifndef SC_INPUTWIN_HXX
#define SC_INPUTWIN_HXX

#include <vector>
#include <vcl/toolbox.hxx>
#include <sfx2/childwin.hxx>
#include <svl/lstner.hxx>
#include <vcl/button.hxx>
#include <vcl/combobox.hxx>
#include <vcl/scrbar.hxx>
#include <vcl/window.hxx>
#include <svtools/transfer.hxx>

class ScEditEngineDefaulter;
class EditView;
struct ESelection;
class ScInputHandler;
class ScAccessibleEditLineTextData;
struct EENotify;
class ScRangeList;
class ScTabViewShell;

//========================================================================

class ScTextWndBase : public Window
{
public:
    ScTextWndBase( Window* pParent,  WinBits nStyle );
    virtual void            InsertAccessibleTextData( ScAccessibleEditLineTextData& rTextData ) = 0;
    virtual void            RemoveAccessibleTextData( ScAccessibleEditLineTextData& rTextData ) = 0;
    virtual void            SetTextString( const String& rString ) = 0;
    virtual const String&   GetTextString() const = 0;
    virtual void            StartEditEngine() = 0;
    virtual void            StopEditEngine( sal_Bool bAll ) = 0;
    virtual EditView*       GetEditView() = 0;
    virtual void            MakeDialogEditView() = 0;
    virtual void            SetFormulaMode( sal_Bool bSet ) = 0;
    virtual sal_Bool            IsInputActive() = 0;
    virtual void            TextGrabFocus() = 0;
};

class ScTextWnd : public ScTextWndBase, public DragSourceHelper     // edit window
{
public:
                    ScTextWnd( Window* pParent, ScTabViewShell* pViewSh );
    virtual         ~ScTextWnd();

    virtual void            SetTextString( const String& rString );
    virtual const String&   GetTextString() const;

    sal_Bool            IsInputActive();
    virtual EditView*       GetEditView();

                        // for function autopilots
    virtual void            MakeDialogEditView();

    virtual void            StartEditEngine();
    virtual void            StopEditEngine( sal_Bool bAll );

    virtual void            TextGrabFocus();

    virtual void    DataChanged( const DataChangedEvent& rDCEvt );

    virtual void            SetFormulaMode( sal_Bool bSet );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > CreateAccessible();

    virtual void            InsertAccessibleTextData( ScAccessibleEditLineTextData& rTextData );
    virtual void            RemoveAccessibleTextData( ScAccessibleEditLineTextData& rTextData );

    DECL_LINK( NotifyHdl, void* );

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

    void            ImplInitSettings();
    void            UpdateAutoCorrFlag();

    ScTabViewShell* GetViewShell();

    typedef ::std::vector< ScAccessibleEditLineTextData* > AccTextDataVector;

    String      aString;
    Font        aTextFont;
    ScEditEngineDefaulter*  pEditEngine;            // only created when needed
    EditView*   pEditView;
    AccTextDataVector maAccTextDatas;   // #i105267# text datas may be cloned, remember all copies
    sal_Bool        bIsRTL;
    sal_Bool        bIsInsertMode;
    sal_Bool        bFormulaMode;

    // #102710#; this flag should be true if a key input or a command is handled
    // it prevents the call of InputChanged in the ModifyHandler of the EditEngine
    sal_Bool        bInputMode;

private:
    ScTabViewShell* mpViewShell;
};

//========================================================================

class ScPosWnd : public ComboBox, public SfxListener        // Display position
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

    void            SetPos( const String& rPosStr );        // Displayed Text
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
class ScInputBarGroup;

class ScMultiTextWnd : public ScTextWnd
{
public:
    ScMultiTextWnd( ScInputBarGroup* pParent, ScTabViewShell* pViewSh );
    virtual ~ScMultiTextWnd();
    virtual void StartEditEngine();
    virtual void StopEditEngine( sal_Bool bAll );
    virtual void Resize();
    virtual EditView*  GetEditView();
    long GetPixelHeightForLines( long nLines );
    long GetEditEngTxtHeight();

    void DoScroll();
    virtual void SetTextString( const String& rString );
    void SetNumLines( long nLines );
    long GetNumLines() { return mnLines; }
    long GetLastNumExpandedLines() { return mnLastExpandedLines; }
protected:
    void SetScrollBarRange();
    void InitEditEngine();

    virtual void Paint( const Rectangle& rRec );
    DECL_LINK( NotifyHdl, EENotify* );
    DECL_LINK( ModifyHdl, EENotify* );
private:
    long GetPixelTextHeight();
    ScInputBarGroup& mrGroupBar;
    long mnLines;
    long mnLastExpandedLines;
    long mnBorderHeight;
    bool mbInvalidate;
};

class ScInputBarGroup : public ScTextWndBase
{

public:
                    ScInputBarGroup( Window* Parent, ScTabViewShell* pViewSh );
    virtual         ~ScInputBarGroup();
    virtual void            InsertAccessibleTextData( ScAccessibleEditLineTextData& rTextData );
    virtual void            RemoveAccessibleTextData( ScAccessibleEditLineTextData& rTextData );
//    virtual void    Paint(const Rectangle& rRec );
    void            SetTextString( const String& rString );
    void            StartEditEngine();
    EditView*       GetEditView();
    void            SetSize(Size aSize);
    virtual void    Resize();
    virtual const String&   GetTextString() const;
    virtual void            StopEditEngine( sal_Bool bAll );
    virtual void            TextGrabFocus();
    void            InitEditEngine(SfxObjectShell* pObjSh);
    void            SetFormulaMode( sal_Bool bSet );
    bool            IsFocus();
    void            MakeDialogEditView();
    sal_Bool            IsInputActive();
    ScrollBar&      GetScrollBar() { return aScrollBar; }
    void            IncrementVerticalSize();
    void            DecrementVerticalSize();
    long            GetNumLines() { return aMultiTextWnd.GetNumLines(); }
    long            GetVertOffset() { return  nVertOffset; }
private:
    void            TriggerToolboxLayout();
    ScMultiTextWnd  aMultiTextWnd;
    ImageButton     aButton;
    ScrollBar       aScrollBar;
    long            nVertOffset;
    DECL_LINK( ClickHdl, void* );
    DECL_LINK( Impl_ScrollHdl, void* );

};


class ScInputWindow : public ToolBox                        // Parent toolbox
{
public:
                    ScInputWindow( Window* pParent, SfxBindings* pBind );
    virtual         ~ScInputWindow();

    virtual void    Paint( const Rectangle& rRect );
    virtual void    Resize();
    virtual void    Select();

    void            SetFuncString( const String& rString, sal_Bool bDoEdit = sal_True );
    void            SetPosString( const String& rStr );
    void            SetTextString( const String& rString );

    void            SetOkCancelMode();
    void            SetSumAssignMode();
    void            EnableButtons( sal_Bool bEnable = sal_True );

    void            SetFormulaMode( sal_Bool bSet );

    virtual sal_Bool            IsInputActive();
    EditView*       GetEditView();

    void            TextGrabFocus();
    void            TextInvalidate();
    void            SwitchToTextWin();

    void            PosGrabFocus();

    // For function autopilots
    void            MakeDialogEditView();

    void            StopEditEngine( sal_Bool bAll );

    void            SetInputHandler( ScInputHandler* pNew );

    ScInputHandler* GetInputHandler(){ return pInputHdl;}

    void            StateChanged( StateChangedType nType );
    virtual void    DataChanged( const DataChangedEvent& rDCEvt );
    virtual void    MouseButtonUp( const MouseEvent& rMEvt );
    virtual void    MouseButtonDown( const MouseEvent& rMEvt );
    virtual void    MouseMove( const MouseEvent& rMEvt );
    bool            IsMultiLineInput() { return mbIsMultiLine; }
protected:
    virtual void    SetText( const String& rString );
    virtual String  GetText() const;

    bool UseSubTotal( ScRangeList* pRangeList ) const;
    bool IsPointerAtResizePos();
private:
    ScPosWnd        aWndPos;
    std::auto_ptr<ScTextWndBase> pRuntimeWindow;
    ScTextWndBase&  aTextWindow;
    ScInputHandler* pInputHdl;
    SfxBindings*    pBindings;
    String          aTextOk;
    String          aTextCancel;
    String          aTextSum;
    String          aTextEqual;
    long            mnMaxY;
    sal_Bool            bIsOkCancelMode;
    bool            bIsMultiLine;
    bool            bInResize;
    bool            mbIsMultiLine;
};

//==================================================================

class ScInputWindowWrapper : public SfxChildWindow
{
public:
            ScInputWindowWrapper( Window*           pParent,
                                  sal_uInt16            nId,
                                  SfxBindings*      pBindings,
                                  SfxChildWinInfo*  pInfo );

    SFX_DECL_CHILDWINDOW_WITHID(ScInputWindowWrapper);
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
