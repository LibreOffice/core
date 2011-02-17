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

#ifndef SD_NAVIGATOR_HXX
#define SD_NAVIGATOR_HXX

#include <vcl/window.hxx>
#include <vcl/lstbox.hxx>
#ifndef _TOOLBOX_HXX //autogen
#include <vcl/toolbox.hxx>
#endif
#include <sfx2/ctrlitem.hxx>
#include "sdtreelb.hxx"
#include "pres.hxx"

#define NAVSTATE_NONE           0x00000000

#define NAVBTN_PEN_ENABLED      0x00000010
#define NAVBTN_PEN_DISABLED     0x00000020
#define NAVBTN_PEN_CHECKED      0x00000040
#define NAVBTN_PEN_UNCHECKED    0x00000080

#define NAVTLB_UPDATE           0x00000100

#define NAVBTN_FIRST_ENABLED    0x00001000
#define NAVBTN_FIRST_DISABLED   0x00002000
#define NAVBTN_PREV_ENABLED     0x00004000
#define NAVBTN_PREV_DISABLED    0x00008000

#define NAVBTN_LAST_ENABLED     0x00010000
#define NAVBTN_LAST_DISABLED    0x00020000
#define NAVBTN_NEXT_ENABLED     0x00040000
#define NAVBTN_NEXT_DISABLED    0x00080000

// forward
namespace sd {
class DrawDocShell;
class NavigatorChildWindow;
class View;
}
class Menu;
class SdNavigatorControllerItem;
class SdPageNameControllerItem;

//------------------------------------------------------------------------

class NavDocInfo
{
public:
            NavDocInfo() { mpDocShell = NULL; }

    sal_Bool    HasName() { return( (sal_Bool) bName ); }
    sal_Bool    IsActive() { return( (sal_Bool) bActive ); }

    void    SetName( sal_Bool bOn = sal_True ) { bName = bOn; }
    void    SetActive( sal_Bool bOn = sal_True ) { bActive = bOn; }

private:
    friend class SdNavigatorWin;
    sal_Bool            bName   : 1;
    sal_Bool            bActive : 1;
    ::sd::DrawDocShell* mpDocShell;
};

//------------------------------------------------------------------------

class SdNavigatorWin
    : public Window
{
public:
    SdNavigatorWin(
        ::Window* pParent,
        ::sd::NavigatorChildWindow* pChildWinContext,
        const SdResId& rSdResId,
        SfxBindings* pBindings );
    virtual ~SdNavigatorWin();

    virtual void                KeyInput( const KeyEvent& rKEvt );

    void                        InitTreeLB( const SdDrawDocument* pDoc );
    void                        RefreshDocumentLB( const String* pDocName = NULL );

    sal_Bool                        InsertFile(const String& rFileName);

    NavigatorDragType           GetNavigatorDragType();
    void                        SetNavigatorDragType(NavigatorDragType eType) { meDragType = eType; }

protected:
    virtual void                Resize();
    virtual long                Notify(NotifyEvent& rNEvt);


private:
    friend class ::sd::NavigatorChildWindow;
    friend class SdNavigatorControllerItem;
    friend class SdPageNameControllerItem;

    ToolBox                     maToolbox;
    SdPageObjsTLB               maTlbObjects;
    ListBox                     maLbDocs;

    ::sd::NavigatorChildWindow*     mpChildWinContext;
    Size                        maSize;
    Size                        maMinSize;
//  Size                        maFltWinSize;
    sal_Bool                        mbDocImported;
    String                      maDropFileName;
    NavigatorDragType           meDragType;
    List*                       mpDocList;
    SfxBindings*                mpBindings;
    SdNavigatorControllerItem*  mpNavigatorCtrlItem;
    SdPageNameControllerItem*   mpPageNameCtrlItem;

    ImageList                   maImageList;
    ImageList                   maImageListH;

    /** This flag controls whether all shapes or only the named shapes are
        shown.
    */
    bool                        mbShowAllShapes;

    sal_uInt16                      GetDragTypeSdResId( NavigatorDragType eDT, sal_Bool bImage = sal_False );
    NavDocInfo*                 GetDocInfo();

                                DECL_LINK( GetFocusObjectsHdl, void * );
                                DECL_LINK( SelectToolboxHdl, void * );
                                DECL_LINK( ClickToolboxHdl, ToolBox * );
                                DECL_LINK( DropdownClickToolBoxHdl, ToolBox * );
                                DECL_LINK( ClickPageHdl, void * );
                                DECL_LINK( ClickObjectHdl, void * );
                                DECL_LINK( SelectDocumentHdl, void * );
                                DECL_LINK( MenuSelectHdl, Menu * );
                                DECL_LINK( ShapeFilterCallback, Menu * );

    virtual void                DataChanged( const DataChangedEvent& rDCEvt );
    void                        SetDragImage();
    void                        ApplyImageList();
};




/*************************************************************************
|*
|* ControllerItem fuer Navigator
|*
\************************************************************************/

class SdNavigatorControllerItem : public SfxControllerItem
{
public:
    SdNavigatorControllerItem( sal_uInt16, SdNavigatorWin*, SfxBindings* );

protected:
    virtual void StateChanged( sal_uInt16 nSId, SfxItemState eState,
                                const SfxPoolItem* pState );

private:
    SdNavigatorWin* pNavigatorWin;
};



/*************************************************************************
|*
|* ControllerItem fuer Navigator zum Anzeigen der Seite in der TreeLB
|*
\************************************************************************/

class SdPageNameControllerItem : public SfxControllerItem
{
public:
    SdPageNameControllerItem( sal_uInt16, SdNavigatorWin*, SfxBindings* );

protected:
    virtual void StateChanged( sal_uInt16 nSId, SfxItemState eState,
                                const SfxPoolItem* pState );

private:
    SdNavigatorWin* pNavigatorWin;
};

#endif
