/*************************************************************************
 *
 *  $RCSfile: navigatr.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:48:40 $
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


#ifndef _SD_NAVIGATR_HXX
#define _SD_NAVIGATR_HXX


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

#define NAVSTATE_NONE           0x00000000

#define NAVBTN_LIVE_ENABLED     0x00000001
#define NAVBTN_LIVE_DISABLED    0x00000002
#define NAVBTN_LIVE_CHECKED     0x00000004
#define NAVBTN_LIVE_UNCHECKED   0x00000008

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
class SdView;
class SdDrawDocShell;
class Menu;

enum NavigatorDragType
{
    NAVIGATOR_DRAGTYPE_NONE,
    NAVIGATOR_DRAGTYPE_URL,
    NAVIGATOR_DRAGTYPE_LINK,
    NAVIGATOR_DRAGTYPE_EMBEDDED
};
// Bitte mitpflegen !!!
#define NAVIGATOR_DRAGTYPE_COUNT 4

//------------------------------------------------------------------------

class NavDocInfo
{
 friend class SdNavigatorWin;

private:
    BOOL            bName   : 1;
    BOOL            bActive : 1;
    SdDrawDocShell* pDocShell;


public:
            NavDocInfo() { pDocShell = NULL; }

    BOOL    HasName() { return( (BOOL) bName ); }
    BOOL    IsActive() { return( (BOOL) bActive ); }

    void    SetName( BOOL bOn = TRUE ) { bName = bOn; }
    void    SetActive( BOOL bOn = TRUE ) { bActive = bOn; }
};

//------------------------------------------------------------------------

class SdNavigatorWin : public Window
{
 friend class SdNavigatorChildWindow;
 friend class SdNavigatorControllerItem;
 friend class SdPageNameControllerItem;

private:
    ToolBox         aToolbox;
    SdPageObjsTLB   aTlbObjects;
    ListBox         aLbDocs;

    SdNavigatorChildWindow* pChildWinContext;
    Size            aSize;
    Size            aMinSize;
    Size            aFltWinSize;
    BOOL            bDocImported;

    String                      aDropFileName;
    NavigatorDragType           eDragType;
    List*                       pDocList;

    SfxBindings*                pBindings;
    SdNavigatorControllerItem*  pNavigatorCtrlItem;
    SdPageNameControllerItem*   pPageNameCtrlItem;

    //------------------------------------

    DECL_LINK( SelectToolboxHdl, void * );
    DECL_LINK( ClickToolboxHdl, ToolBox * );
    DECL_LINK( ClickPageHdl, void * );
    DECL_LINK( ClickObjectHdl, void * );
    DECL_LINK( SelectDocumentHdl, void * );
    DECL_LINK( MenuSelectHdl, Menu * );

    USHORT GetDragTypeSdResId( NavigatorDragType eDT, BOOL bImage = FALSE );
    NavDocInfo* GetDocInfo();

protected:
    virtual void    Resize();
    virtual long    ParentNotify(NotifyEvent& rNEvt);

public:
            SdNavigatorWin( Window* pParent,
                            SdNavigatorChildWindow* pChildWinContext,
                            const SdResId& rSdResId,
                            SfxBindings* pBindings );
            ~SdNavigatorWin();

    virtual void    KeyInput( const KeyEvent& rKEvt );
    void    InitTreeLB( const SdDrawDocument* pDoc );
    void    RefreshDocumentLB( const String* pDocName = NULL );
    BOOL    InsertFile(const String& rFileName);
    NavigatorDragType GetNavigatorDragType();
    void    SetNavigatorDragType(NavigatorDragType eType) { eDragType = eType; }
    SdDrawDocShell* GetDropDocSh();
};

/*************************************************************************
|*
|* ControllerItem fuer Navigator
|*
\************************************************************************/

class SdNavigatorControllerItem : public SfxControllerItem
{
    SdNavigatorWin* pNavigatorWin;

 protected:
    virtual void StateChanged( USHORT nSId, SfxItemState eState,
                                const SfxPoolItem* pState );

 public:
    SdNavigatorControllerItem( USHORT, SdNavigatorWin*, SfxBindings* );
};

/*************************************************************************
|*
|* ControllerItem fuer Navigator zum Anzeigen der Seite in der TreeLB
|*
\************************************************************************/

class SdPageNameControllerItem : public SfxControllerItem
{
    SdNavigatorWin* pNavigatorWin;

 protected:
    virtual void StateChanged( USHORT nSId, SfxItemState eState,
                                const SfxPoolItem* pState );

 public:
    SdPageNameControllerItem( USHORT, SdNavigatorWin*, SfxBindings* );
};

#endif      // _SD_NAVIGATR_HXX

