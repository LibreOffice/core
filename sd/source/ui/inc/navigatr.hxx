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

#pragma once

#include <sfx2/ctrlitem.hxx>
#include <sfx2/sidebar/PanelLayout.hxx>
#include <sfx2/navigat.hxx>
#include "sdtreelb.hxx"
#include <pres.hxx>

#include <sfx2/sidebar/IContextChangeReceiver.hxx>
#include <vcl/EnumContext.hxx>

// forward
namespace vcl { class Window; }

namespace sd {
class DrawDocShell;
class SdNavigatorFloat;
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

    bool    HasName() const { return bName; }
    bool    IsActive() const { return bActive; }

    void    SetName( bool bOn ) { bName = bOn; }
    void    SetActive( bool bOn ) { bActive = bOn; }

    ::sd::DrawDocShell* GetDrawDocShell() {return mpDocShell;}

private:
    friend class SdNavigatorWin;
    bool            bName   : 1;
    bool            bActive : 1;
    ::sd::DrawDocShell* mpDocShell;
};

namespace sd {

class SdNavigatorFloat final : public SfxNavigator
{
private:
    std::unique_ptr<SdNavigatorWin> m_xNavWin;
    bool m_bSetInitialFocusOnActivate;

public:
    SdNavigatorFloat(SfxBindings* _pBindings, SfxChildWindow* pMgr,
                     vcl::Window* pParent, SfxChildWinInfo* pInfo);
    void InitTreeLB(const SdDrawDocument* pDoc);
    void FreshTree(const SdDrawDocument* pDoc);
    virtual void Activate() override;
    virtual void dispose() override;
    virtual ~SdNavigatorFloat() override;
};

}

class SAL_DLLPUBLIC_RTTI SdNavigatorWin : public PanelLayout, public sfx2::sidebar::IContextChangeReceiver
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
    SD_DLLPUBLIC SdNavigatorWin(weld::Widget* pParent, SfxBindings* pBindings, SfxNavigator* pNavigatorDlg);
    void SetUpdateRequestFunctor(const UpdateRequestFunctor& rUpdateRequest);
    virtual ~SdNavigatorWin() override;

    SD_DLLPUBLIC void           InitTreeLB( const SdDrawDocument* pDoc );
    void                        RefreshDocumentLB( const OUString* pDocName = nullptr );
    void                        FirstFocus();

    bool                        InsertFile(const OUString& rFileName);

    NavigatorDragType           GetNavigatorDragType();
    SD_DLLPUBLIC SdPageObjsTLV& GetObjects();

    virtual void HandleContextChange(const vcl::EnumContext& eContext) override;

private:
    friend class SdNavigatorFloat;
    friend class SdNavigatorControllerItem;
    friend class SdPageNameControllerItem;

    std::unique_ptr<weld::Toolbar> mxToolbox;
    std::unique_ptr<SdPageObjsTLV> mxTlbObjects;
    std::unique_ptr<weld::ComboBox> mxLbDocs;
    std::unique_ptr<weld::Menu> mxDragModeMenu;
    std::unique_ptr<weld::Menu> mxShapeMenu;

    VclPtr<SfxNavigator> mxNavigatorDlg;

    bool                        mbDocImported;
    OUString                    maDropFileName;
    NavigatorDragType           meDragType;
    std::vector<NavDocInfo>     maDocList;
    SfxBindings*                mpBindings;
    std::unique_ptr<SdNavigatorControllerItem>  mpNavigatorCtrlItem;
    std::unique_ptr<SdPageNameControllerItem>   mpPageNameCtrlItem;

    /** This flag controls whether all shapes or only the named shapes are
        shown.
    */
    //    bool                        mbShowAllShapes;

    static OUString             GetDragTypeSdBmpId(NavigatorDragType eDT);
    NavDocInfo*                 GetDocInfo();

                                DECL_DLLPRIVATE_LINK( SelectToolboxHdl, const OUString&, void );
                                DECL_DLLPRIVATE_LINK( DropdownClickToolBoxHdl, const OUString&, void );
                                DECL_DLLPRIVATE_LINK( ClickObjectHdl, weld::TreeView&, bool );
                                DECL_DLLPRIVATE_LINK( SelectDocumentHdl, weld::ComboBox&, void );
                                DECL_DLLPRIVATE_LINK( MenuSelectHdl, const OUString&, void );
                                DECL_DLLPRIVATE_LINK( ShapeFilterCallback, const OUString&, void );
                                DECL_DLLPRIVATE_LINK( KeyInputHdl, const KeyEvent&, bool );
    DECL_DLLPRIVATE_STATIC_LINK(SdNavigatorWin, MouseReleaseHdl, const MouseEvent&, bool);
    DECL_LINK(CommandHdl, const CommandEvent&, bool);

    void                        SetDragImage();

    void ExecuteContextMenuAction(std::u16string_view rSelectedPopupEntry);

public:
    //when object is marked , fresh the corresponding entry tree .
    void                        FreshTree ( const  SdDrawDocument* pDoc );

    virtual weld::Window* GetFrameWeld() const override;
};

/**
 * ControllerItem for Navigator
 */
class SdNavigatorControllerItem final : public SfxControllerItem
{
public:
    SdNavigatorControllerItem( sal_uInt16, SdNavigatorWin*, SfxBindings*,
        SdNavigatorWin::UpdateRequestFunctor aUpdateRequest);

protected:
    virtual void StateChangedAtToolBoxControl( sal_uInt16 nSId, SfxItemState eState,
                                const SfxPoolItem* pState ) override;

private:
    SdNavigatorWin* pNavigatorWin;
    const SdNavigatorWin::UpdateRequestFunctor maUpdateRequest;
};

/**
 * ControllerItem for Navigator to show the page in the TreeLB
 */
class SdPageNameControllerItem final : public SfxControllerItem
{
public:
    SdPageNameControllerItem( sal_uInt16, SdNavigatorWin*, SfxBindings*);

protected:
    virtual void StateChangedAtToolBoxControl( sal_uInt16 nSId, SfxItemState eState,
                                const SfxPoolItem* pState ) override;

private:
    SdNavigatorWin* pNavigatorWin;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
