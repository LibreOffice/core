/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

    bool    HasName() { return( bName ); }
    bool    IsActive() { return( bActive ); }

    void    SetName( bool bOn = true ) { bName = bOn; }
    void    SetActive( bool bOn = true ) { bActive = bOn; }

private:
    friend class SdNavigatorWin;
    bool            bName   : 1;
    bool            bActive : 1;
    ::sd::DrawDocShell* mpDocShell;
};

//------------------------------------------------------------------------

class SdNavigatorWin
    : public Window
{
public:
    typedef ::boost::function<void(void)> UpdateRequestFunctor;

    /** Create a new instance of the navigator.
        @param bUseActiveUpdate
            When <TRUE/>, the default, then the SdNavigatorWin object
            will make a SID_NAVIGATOR_INIT call whenever it thinks an
            update is necessary.  When <FALSE/> the navigator will
            rely on others to trigger updates.
    */
    SdNavigatorWin(
        ::Window* pParent,
        ::sd::NavigatorChildWindow* pChildWinContext,
        const SdResId& rSdResId,
        SfxBindings* pBindings,
        const UpdateRequestFunctor& rUpdateRequest);
    virtual ~SdNavigatorWin();

    virtual void                KeyInput( const KeyEvent& rKEvt );

    void                        InitTreeLB( const SdDrawDocument* pDoc );
    void                        RefreshDocumentLB( const String* pDocName = NULL );

    bool                        InsertFile(const String& rFileName);

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
    bool                        mbDocImported;
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
    //    bool                        mbShowAllShapes;

    sal_uInt16                  GetDragTypeSdResId( NavigatorDragType eDT, bool bImage = false );
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
//IAccessibility2 Implementation 2009-----
public:
    //Solution: when object is marked , fresh the corresponding entry tree .
    sd::DrawDocShell*             GetDrawDocShell(const SdDrawDocument*);
    void                        FreshTree ( const  SdDrawDocument* pDoc );
    void                        FreshEntry( );
//-----IAccessibility2 Implementation 2009
};




/*************************************************************************
|*
|* ControllerItem fuer Navigator
|*
\************************************************************************/

class SdNavigatorControllerItem : public SfxControllerItem
{
public:
    SdNavigatorControllerItem( sal_uInt16, SdNavigatorWin*, SfxBindings*,
        const SdNavigatorWin::UpdateRequestFunctor& rUpdateRequest);

protected:
    virtual void StateChanged( sal_uInt16 nSId, SfxItemState eState,
                                const SfxPoolItem* pState );

private:
    SdNavigatorWin* pNavigatorWin;
    const SdNavigatorWin::UpdateRequestFunctor maUpdateRequest;
};



/*************************************************************************
|*
|* ControllerItem fuer Navigator zum Anzeigen der Seite in der TreeLB
|*
\************************************************************************/

class SdPageNameControllerItem : public SfxControllerItem
{
public:
    SdPageNameControllerItem( sal_uInt16, SdNavigatorWin*, SfxBindings*,
        const SdNavigatorWin::UpdateRequestFunctor& rUpdateRequest);

protected:
    virtual void StateChanged( sal_uInt16 nSId, SfxItemState eState,
                                const SfxPoolItem* pState );

private:
    SdNavigatorWin* pNavigatorWin;
    const SdNavigatorWin::UpdateRequestFunctor maUpdateRequest;
};

#endif
