/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: navigatr.hxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: vg $ $Date: 2007-08-28 13:38:13 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef SD_NAVIGATOR_HXX
#define SD_NAVIGATOR_HXX

#ifndef _SV_WINDOW_HXX
#include <vcl/window.hxx>
#endif
#ifndef _SV_LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif
#ifndef _TOOLBOX_HXX //autogen
#include <vcl/toolbox.hxx>
#endif
#ifndef _SFXCTRLITEM_HXX //autogen
#include <sfx2/ctrlitem.hxx>
#endif
#ifndef _SDTREELB_HXX
#include "sdtreelb.hxx"
#endif
#ifndef _PRESENTATION_HXX
#include "pres.hxx"
#endif

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

    BOOL    HasName() { return( (BOOL) bName ); }
    BOOL    IsActive() { return( (BOOL) bActive ); }

    void    SetName( BOOL bOn = TRUE ) { bName = bOn; }
    void    SetActive( BOOL bOn = TRUE ) { bActive = bOn; }

private:
    friend class SdNavigatorWin;
    BOOL            bName   : 1;
    BOOL            bActive : 1;
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

    BOOL                        InsertFile(const String& rFileName);

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
    BOOL                        mbDocImported;
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

    USHORT                      GetDragTypeSdResId( NavigatorDragType eDT, BOOL bImage = FALSE );
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
    SdNavigatorControllerItem( USHORT, SdNavigatorWin*, SfxBindings* );

protected:
    virtual void StateChanged( USHORT nSId, SfxItemState eState,
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
    SdPageNameControllerItem( USHORT, SdNavigatorWin*, SfxBindings* );

protected:
    virtual void StateChanged( USHORT nSId, SfxItemState eState,
                                const SfxPoolItem* pState );

private:
    SdNavigatorWin* pNavigatorWin;
};

#endif
