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
class Accelerator;



class ScTextWndBase : public Window
{
public:
    ScTextWndBase( Window* pParent,  WinBits nStyle );
    virtual void            InsertAccessibleTextData( ScAccessibleEditLineTextData& rTextData ) = 0;
    virtual void            RemoveAccessibleTextData( ScAccessibleEditLineTextData& rTextData ) = 0;
    virtual void            SetTextString( const OUString& rString ) = 0;
    virtual const OUString& GetTextString() const = 0;
    virtual void            StartEditEngine() = 0;
    virtual void            StopEditEngine( bool bAll ) = 0;
    virtual EditView*       GetEditView() = 0;
    virtual void            MakeDialogEditView() = 0;
    virtual void            SetFormulaMode( bool bSet ) = 0;
    virtual bool            IsInputActive() = 0;
    virtual void            TextGrabFocus() = 0;
};

class ScTextWnd : public ScTextWndBase, public DragSourceHelper     // edit window
{
public:
                    ScTextWnd( Window* pParent, ScTabViewShell* pViewSh );
    virtual         ~ScTextWnd();

    virtual void            SetTextString( const OUString& rString ) SAL_OVERRIDE;
    virtual const OUString& GetTextString() const SAL_OVERRIDE;

    bool            IsInputActive() SAL_OVERRIDE;
    virtual EditView*       GetEditView() SAL_OVERRIDE;

                        // for function autopilots
    virtual void            MakeDialogEditView() SAL_OVERRIDE;

    virtual void            StartEditEngine() SAL_OVERRIDE;
    virtual void            StopEditEngine( bool bAll ) SAL_OVERRIDE;

    virtual void            TextGrabFocus() SAL_OVERRIDE;

    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) SAL_OVERRIDE;

    virtual void            SetFormulaMode( bool bSet ) SAL_OVERRIDE;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > CreateAccessible() SAL_OVERRIDE;

    virtual void            InsertAccessibleTextData( ScAccessibleEditLineTextData& rTextData ) SAL_OVERRIDE;
    virtual void            RemoveAccessibleTextData( ScAccessibleEditLineTextData& rTextData ) SAL_OVERRIDE;

    DECL_LINK( NotifyHdl, void* );

protected:
    virtual void    Paint( const Rectangle& rRect ) SAL_OVERRIDE;
    virtual void    Resize() SAL_OVERRIDE;

    virtual void    MouseMove( const MouseEvent& rMEvt ) SAL_OVERRIDE;
    virtual void    MouseButtonDown( const MouseEvent& rMEvt ) SAL_OVERRIDE;
    virtual void    MouseButtonUp( const MouseEvent& rMEvt ) SAL_OVERRIDE;
    virtual void    Command( const CommandEvent& rCEvt ) SAL_OVERRIDE;
    virtual void    KeyInput(const KeyEvent& rKEvt) SAL_OVERRIDE;
    virtual void    GetFocus() SAL_OVERRIDE;
    virtual void    LoseFocus() SAL_OVERRIDE;

    virtual void    StartDrag( sal_Int8 nAction, const Point& rPosPixel ) SAL_OVERRIDE;

    virtual OUString  GetText() const SAL_OVERRIDE;

    void            ImplInitSettings();
    void            UpdateAutoCorrFlag();

    ScTabViewShell* GetViewShell();

    typedef ::std::vector< ScAccessibleEditLineTextData* > AccTextDataVector;

    OUString    aString;
    Font        aTextFont;
    ScEditEngineDefaulter*  pEditEngine;            // only created when needed
    EditView*   pEditView;
    AccTextDataVector maAccTextDatas;   // #i105267# text datas may be cloned, remember all copies
    bool        bIsRTL;
    bool        bIsInsertMode;
    bool        bFormulaMode;

    // #102710#; this flag should be true if a key input or a command is handled
    // it prevents the call of InputChanged in the ModifyHandler of the EditEngine
    bool        bInputMode;

private:
    ScTabViewShell* mpViewShell;
};



class ScPosWnd : public ComboBox, public SfxListener        // Display position
{
private:
    OUString        aPosStr;
    Accelerator*    pAccel;
    sal_uLong       nTipVisible;
    bool            bFormulaMode;

public:
                    ScPosWnd( Window* pParent );
    virtual         ~ScPosWnd();

    void            SetPos( const OUString& rPosStr );        // Displayed Text
    void            SetFormulaMode( bool bSet );

protected:
    virtual void    Select() SAL_OVERRIDE;
    virtual void    Modify() SAL_OVERRIDE;

    virtual bool    Notify( NotifyEvent& rNEvt ) SAL_OVERRIDE;

    virtual void    Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) SAL_OVERRIDE;

private:
    void            FillRangeNames();
    void            FillFunctions();
    void            DoEnter();
    void            HideTip();

    void            ReleaseFocus_Impl();
};


class ScInputBarGroup;

class ScMultiTextWnd : public ScTextWnd
{
public:
    ScMultiTextWnd( ScInputBarGroup* pParent, ScTabViewShell* pViewSh );
    virtual ~ScMultiTextWnd();
    virtual void StartEditEngine() SAL_OVERRIDE;
    virtual void StopEditEngine( bool bAll ) SAL_OVERRIDE;
    virtual void Resize() SAL_OVERRIDE;
    virtual EditView*  GetEditView() SAL_OVERRIDE;
    long GetPixelHeightForLines( long nLines );
    long GetEditEngTxtHeight();

    void DoScroll();
    virtual void SetTextString( const OUString& rString ) SAL_OVERRIDE;
    void SetNumLines( long nLines );
    long GetNumLines() { return mnLines; }
    long GetLastNumExpandedLines() { return mnLastExpandedLines; }
protected:
    void SetScrollBarRange();
    void InitEditEngine();

    virtual void Paint( const Rectangle& rRect ) SAL_OVERRIDE;
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
    virtual void            InsertAccessibleTextData( ScAccessibleEditLineTextData& rTextData ) SAL_OVERRIDE;
    virtual void            RemoveAccessibleTextData( ScAccessibleEditLineTextData& rTextData ) SAL_OVERRIDE;
//    virtual void    Paint(const Rectangle& rRect );
    void            SetTextString( const OUString& rString ) SAL_OVERRIDE;
    void            StartEditEngine() SAL_OVERRIDE;
    EditView*       GetEditView() SAL_OVERRIDE;
    void            SetSize(Size aSize);
    virtual void    Resize() SAL_OVERRIDE;
    virtual const OUString&   GetTextString() const SAL_OVERRIDE;
    virtual void            StopEditEngine( bool bAll ) SAL_OVERRIDE;
    virtual void            TextGrabFocus() SAL_OVERRIDE;
    void            InitEditEngine(SfxObjectShell* pObjSh);
    void            SetFormulaMode( bool bSet ) SAL_OVERRIDE;
    bool            IsFocus();
    void            MakeDialogEditView() SAL_OVERRIDE;
    bool            IsInputActive() SAL_OVERRIDE;
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

    virtual void    Paint( const Rectangle& rRect ) SAL_OVERRIDE;
    virtual void    Resize() SAL_OVERRIDE;
    virtual void    Select() SAL_OVERRIDE;

    void            SetFuncString( const OUString& rString, bool bDoEdit = true );
    void            SetPosString( const OUString& rStr );
    void            SetTextString( const OUString& rString );

    void            SetOkCancelMode();
    void            SetSumAssignMode();
    void            EnableButtons( bool bEnable = true );

    void            SetFormulaMode( bool bSet );

    virtual bool            IsInputActive();
    EditView*       GetEditView();

    void            TextGrabFocus();
    void            TextInvalidate();
    void            SwitchToTextWin();

    void            PosGrabFocus();

    // For function autopilots
    void            MakeDialogEditView();

    void            StopEditEngine( bool bAll );

    void            SetInputHandler( ScInputHandler* pNew );

    ScInputHandler* GetInputHandler(){ return pInputHdl;}

    void            StateChanged( StateChangedType nType ) SAL_OVERRIDE;
    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) SAL_OVERRIDE;
    virtual void    MouseButtonUp( const MouseEvent& rMEvt ) SAL_OVERRIDE;
    virtual void    MouseButtonDown( const MouseEvent& rMEvt ) SAL_OVERRIDE;
    virtual void    MouseMove( const MouseEvent& rMEvt ) SAL_OVERRIDE;
    bool            IsMultiLineInput() { return mbIsMultiLine; }
protected:
    virtual void    SetText( const OUString& rString ) SAL_OVERRIDE;
    virtual OUString  GetText() const SAL_OVERRIDE;

    bool UseSubTotal( ScRangeList* pRangeList ) const;
    bool IsPointerAtResizePos();
private:
    ScPosWnd        aWndPos;
    std::auto_ptr<ScTextWndBase> pRuntimeWindow;
    ScTextWndBase&  aTextWindow;
    ScInputHandler* pInputHdl;
    OUString        aTextOk;
    OUString        aTextCancel;
    OUString        aTextSum;
    OUString        aTextEqual;
    long            mnMaxY;
    bool            bIsOkCancelMode;
    bool            bInResize;
    bool            mbIsMultiLine;
};



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
