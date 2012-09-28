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
#ifndef VIEW_HXX
#define VIEW_HXX

#include <sfx2/dockwin.hxx>
#include <sfx2/viewsh.hxx>
#include <svtools/scrwin.hxx>
#include <sfx2/ctrlitem.hxx>
#include <sfx2/shell.hxx>
#include <sfx2/viewfac.hxx>
#include <sfx2/viewfrm.hxx>
#include <vcl/timer.hxx>
#include <svtools/colorcfg.hxx>
#include "edit.hxx"
#include "node.hxx"

class Menu;
class DataChangedEvent;
class SmClipboardChangeListener;
class SmDocShell;
class SmViewShell;
class SmPrintUIOptions;
class SmGraphicAccessible;

/**************************************************************************/

class SmGraphicWindow : public ScrollableWindow
{
    Point     aFormulaDrawPos;

    // old style editing pieces
    Rectangle aCursorRect;
    bool      bIsCursorVisible;
    bool      bIsLineVisible;
    AutoTimer aCaretBlinkTimer;
public:
    bool IsCursorVisible() const { return bIsCursorVisible; }
    void ShowCursor(bool bShow);
    bool IsLineVisible() const { return bIsLineVisible; }
    void ShowLine(bool bShow);
    const SmNode * SetCursorPos(sal_uInt16 nRow, sal_uInt16 nCol);
protected:
    void        SetIsCursorVisible(bool bVis) { bIsCursorVisible = bVis; }
    using   Window::SetCursor;
    void        SetCursor(const SmNode *pNode);
    void        SetCursor(const Rectangle &rRect);
    bool        IsInlineEditEnabled() const;

private:
    ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible >  xAccessible;
    SmGraphicAccessible *                                       pAccessible;

    SmViewShell    *pViewShell;
    sal_uInt16          nZoom;

protected:
    void        SetFormulaDrawPos(const Point &rPos) { aFormulaDrawPos = rPos; }

    virtual void DataChanged( const DataChangedEvent& );
    virtual void Paint(const Rectangle&);
    virtual void KeyInput(const KeyEvent& rKEvt);
    virtual void Command(const CommandEvent& rCEvt);
    virtual void StateChanged( StateChangedType eChanged );
    DECL_LINK(MenuSelectHdl, Menu *);

private:
    void RepaintViewShellDoc();
    DECL_LINK(CaretBlinkTimerHdl, void *);
    void CaretBlinkInit();
    void CaretBlinkStart();
    void CaretBlinkStop();
public:
    SmGraphicWindow(SmViewShell* pShell);
    ~SmGraphicWindow();

    // Window
    virtual void    MouseButtonDown(const MouseEvent &rMEvt);
    virtual void    MouseMove(const MouseEvent &rMEvt);
    virtual void    GetFocus();
    virtual void    LoseFocus();

    SmViewShell *   GetView()   { return pViewShell; }

    using   Window::SetZoom;
    void   SetZoom(sal_uInt16 Factor);
    using   Window::GetZoom;
    sal_uInt16 GetZoom() const { return nZoom; }

    const Point &   GetFormulaDrawPos() const { return aFormulaDrawPos; }

    void ZoomToFitInWindow();
    using   ScrollableWindow::SetTotalSize;
    void SetTotalSize();

    void ApplyColorConfigValues( const svtools::ColorConfig &rColorCfg );

    // for Accessibility
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > CreateAccessible();

    using   Window::GetAccessible;
    SmGraphicAccessible *   GetAccessible_Impl()  { return pAccessible; }
};

/**************************************************************************/

class SmGraphicController: public SfxControllerItem
{
protected:
    SmGraphicWindow &rGraphic;
public:
    SmGraphicController(SmGraphicWindow &, sal_uInt16, SfxBindings & );
    virtual void StateChanged(sal_uInt16             nSID,
                              SfxItemState       eState,
                              const SfxPoolItem* pState);
};

/**************************************************************************/

class SmEditController: public SfxControllerItem
{
protected:
    SmEditWindow &rEdit;

public:
    SmEditController(SmEditWindow &, sal_uInt16, SfxBindings  & );
#if OSL_DEBUG_LEVEL > 1
    virtual ~SmEditController();
#endif

    virtual void StateChanged(sal_uInt16             nSID,
                              SfxItemState       eState,
                              const SfxPoolItem* pState);
};

/**************************************************************************/

class SmCmdBoxWindow : public SfxDockingWindow
{
    SmEditWindow        aEdit;
    SmEditController    aController;
    bool                bExiting;

    Timer               aInitialFocusTimer;

    DECL_LINK(InitialFocusTimerHdl, Timer *);

protected :

    // Window
    virtual void    GetFocus();
    virtual void Resize();
    virtual void Paint(const Rectangle& rRect);
    virtual void StateChanged( StateChangedType nStateChange );

    virtual Size CalcDockingSize(SfxChildAlignment eAlign);
    virtual SfxChildAlignment CheckAlignment(SfxChildAlignment eActual,
                                             SfxChildAlignment eWish);

    virtual void    ToggleFloatingMode();

public:
    SmCmdBoxWindow(SfxBindings    *pBindings,
                   SfxChildWindow *pChildWindow,
                   Window         *pParent);

    virtual ~SmCmdBoxWindow ();

    void AdjustPosition();

    SmEditWindow *GetEditWindow() { return (&aEdit); }
    SmViewShell  *GetView();
};

/**************************************************************************/

class SmCmdBoxWrapper : public SfxChildWindow
{
    SFX_DECL_CHILDWINDOW_WITHID(SmCmdBoxWrapper);

protected:
    SmCmdBoxWrapper(Window          *pParentWindow,
                    sal_uInt16           nId,
                    SfxBindings     *pBindings,
                    SfxChildWinInfo *pInfo);

#if OSL_DEBUG_LEVEL > 1
    virtual ~SmCmdBoxWrapper();
#endif

public:

    SmEditWindow *GetEditWindow()
    {
        return (((SmCmdBoxWindow *)pWindow)->GetEditWindow());
    }

};

/**************************************************************************/

namespace sfx2 { class FileDialogHelper; }
struct SmViewShell_Impl;

class SmViewShell: public SfxViewShell
{
    // for handling the PasteClipboardState
    friend class SmClipboardChangeListener;

    SmViewShell_Impl*   pImpl;

    SmGraphicWindow     aGraphic;
    SmGraphicController aGraphicController;
    String              StatusText;

    ::com::sun::star::uno:: Reference <
            ::com::sun::star::lang:: XEventListener > xClipEvtLstnr;
    SmClipboardChangeListener*  pClipEvtLstnr;
    bool                bPasteState;

    DECL_LINK( DialogClosedHdl, sfx2::FileDialogHelper* );
    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    /** Used to determine whether insertions using SID_INSERTSYMBOL and SID_INSERTCOMMAND
     * should be inserted into SmEditWindow or directly into the SmDocShell as done if the
     * visual editor was last to have focus.
     */
    bool bInsertIntoEditWindow;
protected:

    Size GetTextLineSize(OutputDevice& rDevice,
                         const String& rLine);
    Size GetTextSize(OutputDevice& rDevice,
                     const String& rText,
                     long          MaxWidth);
    void DrawTextLine(OutputDevice& rDevice,
                      const Point&  rPosition,
                      const String& rLine);
    void DrawText(OutputDevice& rDevice,
                  const Point&  rPosition,
                  const String& rText,
                  sal_uInt16        MaxWidth);

    virtual sal_uInt16 Print(SfxProgress &rProgress, sal_Bool bIsAPI);
    virtual SfxPrinter *GetPrinter(sal_Bool bCreate = sal_False);
    virtual sal_uInt16 SetPrinter(SfxPrinter *pNewPrinter,
                              sal_uInt16     nDiffFlags = SFX_PRINTER_ALL, bool bIsAPI=false);

    bool        Insert( SfxMedium& rMedium );
    bool        InsertFrom(SfxMedium &rMedium);

    virtual SfxTabPage *CreatePrintOptionsPage(Window           *pParent,
                                               const SfxItemSet &rOptions);
    virtual void Deactivate(sal_Bool IsMDIActivate);
    virtual void Activate(sal_Bool IsMDIActivate);
    virtual Size GetOptimalSizePixel() const;
    virtual void AdjustPosSizePixel(const Point &rPos, const Size &rSize);
    virtual void InnerResizePixel(const Point &rOfs, const Size  &rSize);
    virtual void OuterResizePixel(const Point &rOfs, const Size  &rSize);
    virtual void QueryObjAreaPixel( Rectangle& rRect ) const;
    virtual void SetZoomFactor( const Fraction &rX, const Fraction &rY );

public:
    TYPEINFO();

    SmViewShell(SfxViewFrame *pFrame, SfxViewShell *pOldSh);
    ~SmViewShell();

    SmDocShell * GetDoc()
    {
        return (SmDocShell *) GetViewFrame()->GetObjectShell();
    }

    SmEditWindow * GetEditWindow();
          SmGraphicWindow & GetGraphicWindow()       { return aGraphic; }
    const SmGraphicWindow & GetGraphicWindow() const { return aGraphic; }

    void        SetStatusText(const String& Text);

    void        ShowError( const SmErrorDesc *pErrorDesc );
    void        NextError();
    void        PrevError();

    SFX_DECL_INTERFACE(SFX_INTERFACE_SMA_START+2)
    SFX_DECL_VIEWFACTORY(SmViewShell);

    virtual void Execute( SfxRequest& rReq );
    virtual void GetState(SfxItemSet &);

    void Impl_Print( OutputDevice &rOutDev, const SmPrintUIOptions &rPrintUIOptions,
            Rectangle aOutRect, Point aZeroPoint );

    /** Set bInsertIntoEditWindow so we know where to insert
     *
     * This method is called whenever SmGraphicWindow or SmEditWindow gets focus,
     * so that when text is inserted from catalog or elsewhere we know whether to
     * insert for the visual editor, or the text editor.
     */
    void SetInsertIntoEditWindow(bool bEditWindowHadFocusLast = true){
        bInsertIntoEditWindow = bEditWindowHadFocusLast;
    }
    bool IsInlineEditEnabled() const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
