/*************************************************************************
 *
 *  $RCSfile: view.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:57:25 $
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
#ifndef VIEW_HXX
#define VIEW_HXX


#ifndef _SFXDOCKWIN_HXX //autogen
#include <sfx2/dockwin.hxx>
#endif
#ifndef _SFXVIEWSH_HXX //autogen
#include <sfx2/viewsh.hxx>
#endif
#ifndef _SCRWIN_HXX //autogen
#include <svtools/scrwin.hxx>
#endif
#ifndef _SFXCTRLITEM_HXX //autogen
#include <sfx2/ctrlitem.hxx>
#endif
#ifndef _SFX_SHELL_HXX //autogen
#include <sfx2/shell.hxx>
#endif
#ifndef _VIEWFAC_HXX //autogen
#include <sfx2/viewfac.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX //autogen
#include <sfx2/viewfrm.hxx>
#endif

#ifndef EDIT_HXX
#include "edit.hxx"
#endif

#ifndef NODE_HXX
#include "node.hxx"
#endif

class Menu;
class DataChangedEvent;
class SmDocShell;
class SmViewShell;

/**************************************************************************/

class SmGraphicWindow: public ScrollableWindow
{
    Point           aFormulaDrawPos;
    Rectangle       aCursorRect;
    SmViewShell    *pViewShell;
    USHORT          nZoom;
    short           nModifyCount;
    BOOL            bIsCursorVisible;

protected:
    void        SetFormulaDrawPos(const Point &rPos) { aFormulaDrawPos = rPos; }
    Point       GetFormulaDrawPos() const { return aFormulaDrawPos; }
    void        SetIsCursorVisible(BOOL bVis) { bIsCursorVisible = bVis; }
    void        SetCursor(const SmNode *pNode);
    void        SetCursor(const Rectangle &rRect);

    virtual void DataChanged( const DataChangedEvent& );
    virtual void Paint(const Rectangle&);
    virtual void KeyInput(const KeyEvent& rKEvt);
    virtual void Command(const CommandEvent& rCEvt);
    virtual void StateChanged( StateChangedType eChanged );
    DECL_LINK(MenuSelectHdl, Menu *);

public:
    SmGraphicWindow(SmViewShell* pShell);

    virtual void MouseButtonDown(const MouseEvent &rMEvt);

    void   SetZoom(USHORT Factor);
    USHORT GetZoom() const { return nZoom; }

    void ZoomToFitInWindow();
    void SetTotalSize();

    BOOL IsCursorVisible() const { return bIsCursorVisible; }
    void ShowCursor(BOOL bShow);
    const SmNode * SetCursorPos(USHORT nRow, USHORT nCol);
};

/**************************************************************************/

class SmGraphicController: public SfxControllerItem
{
protected:
    SmGraphicWindow &rGraphic;
public:
    SmGraphicController(SmGraphicWindow &, USHORT, SfxBindings & );
    virtual void StateChanged(USHORT             nSID,
                              SfxItemState       eState,
                              const SfxPoolItem* pState);
};

/**************************************************************************/

class SmEditController: public SfxControllerItem
{
protected:
    SmEditWindow &rEdit;

public:
    SmEditController(SmEditWindow &, USHORT, SfxBindings  & );
    virtual void StateChanged(USHORT             nSID,
                              SfxItemState       eState,
                              const SfxPoolItem* pState);
};

/**************************************************************************/

class SmCmdBoxWindow : public SfxDockingWindow
{
    SmEditWindow        aEdit;
    SmEditController    aController;
    Timer               aGrabTimer;

    virtual void Resize();
    virtual void Paint(const Rectangle& rRect);
    virtual Size CalcDockingSize(SfxChildAlignment eAlign);
    virtual SfxChildAlignment CheckAlignment(SfxChildAlignment eActual,
                                             SfxChildAlignment eWish);

protected :
    DECL_LINK(UpdateTimeoutHdl, Timer *);

public:
    SmCmdBoxWindow(SfxBindings    *pBindings,
                   SfxChildWindow *pChildWindow,
                   Window         *pParent);

    virtual ~SmCmdBoxWindow ();

    void AdjustPosition();

    SmEditWindow *GetEditWindow() { return (&aEdit); }

    void ShowWindows ();

    void RestartFocusTimer ()
    {
        aGrabTimer.SetTimeout (100);
        aGrabTimer.Start ();
    }

    virtual void    ToggleFloatingMode();

    // Window
    virtual void    StateChanged( StateChangedType nStateChange );

    void Grab ();

};

/**************************************************************************/

class SmCmdBoxWrapper : public SfxChildWindow
{
    SFX_DECL_CHILDWINDOW(SmCmdBoxWrapper);

protected:
    SmCmdBoxWrapper(Window          *pParentWindow,
                    USHORT           nId,
                    SfxBindings     *pBindings,
                    SfxChildWinInfo *pInfo);

public:
    Size    aSize;

    SmEditWindow *GetEditWindow()
    {
        return (((SmCmdBoxWindow *)pWindow)->GetEditWindow());
    }

    void RestartFocusTimer ()
    {
        ((SmCmdBoxWindow *)pWindow)->RestartFocusTimer ();
    }

//    virtual void FillInfo(SfxChildWinInfo &rInfo) const;

    void Grab ();
};

/**************************************************************************/

class SmViewShell: public SfxViewShell
{
    SmGraphicWindow     aGraphic;
    SmGraphicController aGraphicController;
    String              StatusText;
    Window             *pViewFrame;

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
                  USHORT        MaxWidth);

    virtual USHORT Print(SfxProgress &rProgress,
                         PrintDialog *pPrintDialog = 0);
    virtual SfxPrinter *GetPrinter(BOOL bCreate = FALSE);
    virtual USHORT SetPrinter(SfxPrinter *pNewPrinter,
                              USHORT     nDiffFlags = SFX_PRINTER_ALL);

    virtual SfxTabPage *CreatePrintOptionsPage(Window           *pParent,
                                               const SfxItemSet &rOptions);
    virtual void Deactivate(BOOL IsMDIActivate);
    virtual void Activate(BOOL IsMDIActivate);
    virtual Size GetOptimalSizePixel() const;
    virtual void AdjustPosSizePixel(const Point &rPos, const Size &rSize);
    virtual void InnerResizePixel(const Point &rOfs, const Size  &rSize);
    virtual void OuterResizePixel(const Point &rOfs, const Size  &rSize);
    virtual void QueryObjAreaPixel( Rectangle& rRect ) const;
    virtual void SetZoomFactor( const Fraction &rX, const Fraction &rY );

public:
    TYPEINFO();

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

    SFX_DECL_INTERFACE(SFX_INTERFACE_SMA_START+2);
    SFX_DECL_VIEWFACTORY(SmViewShell);

    virtual void Execute( SfxRequest& rReq );
    virtual void GetState(SfxItemSet &);

    SmViewShell(SfxViewFrame *pFrame, SfxViewShell *pOldSh);
    ~SmViewShell();
};

#endif

