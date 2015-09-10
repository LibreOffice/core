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

#ifndef INCLUDED_SD_SOURCE_UI_INC_NAVIGATR_HXX
#define INCLUDED_SD_SOURCE_UI_INC_NAVIGATR_HXX

#include <vcl/window.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/toolbox.hxx>
#include <sfx2/ctrlitem.hxx>
#include "sdtreelb.hxx"
#include "pres.hxx"

#define NAVSTATE_NONE           0x00000000

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
}
class Menu;
class SdNavigatorControllerItem;
class SdPageNameControllerItem;

class NavDocInfo
{
public:
    NavDocInfo()
        : bName(false)
        , bActive(false)
        , mpDocShell(NULL)
    {
    }

    bool    HasName() { return bName; }
    bool    IsActive() { return bActive; }

    void    SetName( bool bOn = true ) { bName = bOn; }
    void    SetActive( bool bOn = true ) { bActive = bOn; }

private:
    friend class SdNavigatorWin;
    bool            bName   : 1;
    bool            bActive : 1;
    ::sd::DrawDocShell* mpDocShell;
};

class SdNavigatorWin
    : public vcl::Window
{
public:
    typedef ::std::function<void ()> UpdateRequestFunctor;

    /** Create a new instance of the navigator.
        @param bUseActiveUpdate
            When <TRUE/>, the default, then the SdNavigatorWin object
            will make a SID_NAVIGATOR_INIT call whenever it thinks an
            update is necessary.  When <FALSE/> the navigator will
            rely on others to trigger updates.
    */
    SdNavigatorWin(
        vcl::Window* pParent,
        ::sd::NavigatorChildWindow* pChildWinContext,
        const SdResId& rSdResId,
        SfxBindings* pBindings);
    void SetUpdateRequestFunctor(const UpdateRequestFunctor& rUpdateRequest);
    virtual ~SdNavigatorWin();
    virtual void                dispose() SAL_OVERRIDE;

    virtual void                KeyInput( const KeyEvent& rKEvt ) SAL_OVERRIDE;

    void                        InitTreeLB( const SdDrawDocument* pDoc );
    void                        RefreshDocumentLB( const OUString* pDocName = NULL );

    bool                        InsertFile(const OUString& rFileName);

    NavigatorDragType           GetNavigatorDragType();

protected:
    virtual void                Resize() SAL_OVERRIDE;
    virtual bool                Notify(NotifyEvent& rNEvt) SAL_OVERRIDE;

private:
    friend class ::sd::NavigatorChildWindow;
    friend class SdNavigatorControllerItem;
    friend class SdPageNameControllerItem;

    VclPtr<ToolBox>             maToolbox;
    VclPtr<SdPageObjsTLB>       maTlbObjects;
    VclPtr<ListBox>             maLbDocs;

    ::sd::NavigatorChildWindow*     mpChildWinContext;
    Size                        maSize;
    Size                        maMinSize;
    bool                        mbDocImported;
    OUString                    maDropFileName;
    NavigatorDragType           meDragType;
    std::vector<NavDocInfo>     maDocList;
    SfxBindings*                mpBindings;
    SdNavigatorControllerItem*  mpNavigatorCtrlItem;
    SdPageNameControllerItem*   mpPageNameCtrlItem;

    ImageList                   maImageList;

    /** This flag controls whether all shapes or only the named shapes are
        shown.
    */
    //    bool                        mbShowAllShapes;

    static sal_uInt16           GetDragTypeSdResId( NavigatorDragType eDT, bool bImage = false );
    NavDocInfo*                 GetDocInfo();

                                DECL_LINK( GetFocusObjectsHdl, void * );
                                DECL_LINK_TYPED( SelectToolboxHdl, ToolBox *, void );
                                DECL_LINK_TYPED( DropdownClickToolBoxHdl, ToolBox *, void );
                                DECL_LINK( ClickPageHdl, void * );
                                DECL_LINK_TYPED( ClickObjectHdl, SvTreeListBox*, bool );
                                DECL_LINK( SelectDocumentHdl, void * );
                                DECL_LINK_TYPED( MenuSelectHdl, Menu *, bool );
                                DECL_LINK_TYPED( ShapeFilterCallback, Menu *, bool );

    virtual void                DataChanged( const DataChangedEvent& rDCEvt ) SAL_OVERRIDE;
    void                        SetDragImage();
    void                        ApplyImageList();
public:
    //when object is marked , fresh the corresponding entry tree .
    static sd::DrawDocShell*    GetDrawDocShell(const SdDrawDocument*);
    void                        FreshTree ( const  SdDrawDocument* pDoc );
    void                        FreshEntry( );
};

/**
 * ControllerItem for Navigator
 */
class SdNavigatorControllerItem : public SfxControllerItem
{
public:
    SdNavigatorControllerItem( sal_uInt16, SdNavigatorWin*, SfxBindings*,
        const SdNavigatorWin::UpdateRequestFunctor& rUpdateRequest);

protected:
    virtual void StateChanged( sal_uInt16 nSId, SfxItemState eState,
                                const SfxPoolItem* pState ) SAL_OVERRIDE;

private:
    VclPtr<SdNavigatorWin> pNavigatorWin;
    const SdNavigatorWin::UpdateRequestFunctor maUpdateRequest;
};

/**
 * ControllerItem for Navigator to show the page in the TreeLB
 */
class SdPageNameControllerItem : public SfxControllerItem
{
public:
    SdPageNameControllerItem( sal_uInt16, SdNavigatorWin*, SfxBindings*,
        const SdNavigatorWin::UpdateRequestFunctor& rUpdateRequest);

protected:
    virtual void StateChanged( sal_uInt16 nSId, SfxItemState eState,
                                const SfxPoolItem* pState ) SAL_OVERRIDE;

private:
    VclPtr<SdNavigatorWin> pNavigatorWin;
    const SdNavigatorWin::UpdateRequestFunctor maUpdateRequest;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
