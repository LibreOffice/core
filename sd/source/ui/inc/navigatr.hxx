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
#include <svx/sidebar/PanelLayout.hxx>
#include "sdtreelb.hxx"
#include "pres.hxx"

// forward
namespace sd {
class DrawDocShell;
class NavigatorChildWindow;
}
class Menu;
class SdNavigatorControllerItem;
class SdPageNameControllerItem;

enum class NavState {
    NONE               = 0x000000,
    TableUpdate        = 0x000100,
    BtnFirstEnabled    = 0x001000,
    BtnFirstDisabled   = 0x002000,
    BtnPrevEnabled     = 0x004000,
    BtnPrevDisabled    = 0x008000,
    BtnLastEnabled     = 0x010000,
    BtnLastDisabled    = 0x020000,
    BtnNextEnabled     = 0x040000,
    BtnNextDisabled    = 0x080000,
};
namespace o3tl {
    template<> struct typed_flags<NavState> : is_typed_flags<NavState, 0x0ff100> {};
}

class NavDocInfo
{
public:
    NavDocInfo()
        : bName(false)
        , bActive(false)
        , mpDocShell(nullptr)
    {
    }

    bool    HasName() { return bName; }
    bool    IsActive() { return bActive; }

    void    SetName( bool bOn ) { bName = bOn; }
    void    SetActive( bool bOn ) { bActive = bOn; }

private:
    friend class SdNavigatorWin;
    bool            bName   : 1;
    bool            bActive : 1;
    ::sd::DrawDocShell* mpDocShell;
};

class SdNavigatorWin : public PanelLayout
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
    SdNavigatorWin(vcl::Window* pParent, SfxBindings* pBindings);
    void SetUpdateRequestFunctor(const UpdateRequestFunctor& rUpdateRequest);
    virtual ~SdNavigatorWin() override;
    virtual void                dispose() override;

    virtual void                KeyInput( const KeyEvent& rKEvt ) override;

    void                        InitTreeLB( const SdDrawDocument* pDoc );
    void                        RefreshDocumentLB( const OUString* pDocName = nullptr );

    bool                        InsertFile(const OUString& rFileName);

    NavigatorDragType           GetNavigatorDragType();

protected:
    virtual bool                EventNotify(NotifyEvent& rNEvt) override;

private:
    friend class ::sd::NavigatorChildWindow;
    friend class SdNavigatorControllerItem;
    friend class SdPageNameControllerItem;

    VclPtr<ToolBox>             maToolbox;
    VclPtr<SdPageObjsTLB>       maTlbObjects;
    VclPtr<ListBox>             maLbDocs;

    bool                        mbDocImported;
    OUString                    maDropFileName;
    NavigatorDragType           meDragType;
    std::vector<NavDocInfo>     maDocList;
    SfxBindings*                mpBindings;
    SdNavigatorControllerItem*  mpNavigatorCtrlItem;
    SdPageNameControllerItem*   mpPageNameCtrlItem;

    /** This flag controls whether all shapes or only the named shapes are
        shown.
    */
    //    bool                        mbShowAllShapes;

    static const char*          GetDragTypeSdStrId(NavigatorDragType eDT);
    static OUString             GetDragTypeSdBmpId(NavigatorDragType eDT);
    NavDocInfo*                 GetDocInfo();

                                DECL_LINK( SelectToolboxHdl, ToolBox *, void );
                                DECL_LINK( DropdownClickToolBoxHdl, ToolBox *, void );
                                DECL_LINK( ClickObjectHdl, SvTreeListBox*, bool );
                                DECL_LINK( SelectDocumentHdl, ListBox&, void );
                                DECL_LINK( MenuSelectHdl, Menu *, bool );
                                DECL_LINK( ShapeFilterCallback, Menu *, bool );

    void                        SetDragImage();

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
                                const SfxPoolItem* pState ) override;

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
    SdPageNameControllerItem( sal_uInt16, SdNavigatorWin*, SfxBindings*);

protected:
    virtual void StateChanged( sal_uInt16 nSId, SfxItemState eState,
                                const SfxPoolItem* pState ) override;

private:
    VclPtr<SdNavigatorWin> pNavigatorWin;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
