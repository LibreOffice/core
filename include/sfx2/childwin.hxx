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
#ifndef INCLUDED_SFX2_CHILDWIN_HXX
#define INCLUDED_SFX2_CHILDWIN_HXX

#include <memory>
#include <sal/config.h>

#include <sfx2/dllapi.h>
#include <sal/types.h>
#include <o3tl/typed_flags_set.hxx>
#include <vcl/window.hxx>

#include <sfx2/chalign.hxx>
#include <sfx2/basedlgs.hxx>

namespace com::sun::star::frame { class XFrame; }

class SfxWorkWindow;
class SfxModule;
class SfxChildWindow;
class SfxBindings;
class SfxModelessDialogController;

enum class SfxChildWindowFlags
{
    NONE            = 0x000,
    FORCEDOCK       = 0x004, // Float forbidden
    TASK            = 0x010, // ChildWindow inside the Task
    CANTGETFOCUS    = 0x020, // ChildWindow can not get focus
    ALWAYSAVAILABLE = 0x040, // ChildWindow is never disabled
    NEVERHIDE       = 0x080, // ChildWindow is always visible
    NEVERCLONE      = 0x100, // ChildWindow is not recreated in new view
};

namespace o3tl
{
    template<> struct typed_flags<SfxChildWindowFlags> : is_typed_flags<SfxChildWindowFlags, 0x1f4> {};
}


#define CHILDWIN_NOPOS            USHRT_MAX

// ChildWindow Configuration
struct SAL_DLLPUBLIC_RTTI SfxChildWinInfo
{
    OUString            aExtraString;
    OUString            aModule;
    OString             aWinState;
    Point               aPos;
    Size                aSize;
    SfxChildWindowFlags nFlags;
    bool                bVisible;

                        SfxChildWinInfo()
                        {
                            bVisible = false;
                            nFlags = SfxChildWindowFlags::NONE;
                        }
    bool                GetExtraData_Impl( SfxChildAlignment    *pAlign ) const;
};

// ChildWindow factory methods
typedef std::unique_ptr<SfxChildWindow> (*SfxChildWinCtor)( vcl::Window *pParentWindow,
                                            sal_uInt16 nId,
                                            SfxBindings *pBindings,
                                            SfxChildWinInfo *pInfo);

struct SFX2_DLLPUBLIC SfxChildWinFactory
{
    SfxChildWinCtor             pCtor;  // Factory method
    SfxChildWinInfo             aInfo;  // Configuration
    sal_uInt16                  nId;    // ChildWindow-Id ( SlotId )
    sal_uInt16                  nPos;   // Position in UI

    SfxChildWinFactory( SfxChildWinCtor pTheCtor, sal_uInt16 nID, sal_uInt16 n );
};

struct SfxChildWindow_Impl;

extern SFX2_DLLPUBLIC bool ParentIsFloatingWindow(const vcl::Window *pParent);

class SFX2_DLLPUBLIC SfxChildWindow
{
    VclPtr<vcl::Window>        pParent;         // parent window ( Topwindow )
    VclPtr<vcl::Window>        pWindow;         // actual contents
    std::unique_ptr< SfxChildWindow_Impl>       pImpl;            // Implementation data
    std::shared_ptr<SfxDialogController> xController;     // actual contents
    SfxChildAlignment          eChildAlignment; // Current css::drawing::Alignment
                                                 // Another window in pWindow
    sal_uInt16                 nType;           // ChildWindow-Id
    SAL_DLLPRIVATE void ClearWorkwin();

protected:
    void                SetWindow(const VclPtr<vcl::Window>& p) { pWindow = p; }
                        SfxChildWindow(vcl::Window *pParentWindow, sal_uInt16 nId);

public:
    virtual             ~SfxChildWindow();
    void                Destroy();
    vcl::Window*        GetWindow() const
                        { return pWindow; }
    void                SetController(std::shared_ptr<SfxDialogController> controller) { xController = controller; }
    void                ClearController() { xController.reset(); }
    std::shared_ptr<SfxDialogController>& GetController() { return xController; }
    const std::shared_ptr<SfxDialogController>& GetController() const { return xController; }
    vcl::Window*        GetParent() const
                        { return pParent; }
    SfxChildAlignment   GetAlignment() const
                        { return eChildAlignment; }
    void                SetAlignment(SfxChildAlignment eAlign);
    virtual void        Hide();
    virtual void        Show( ShowFlags nFlags );
    sal_uInt16          GetPosition() const;
    sal_uInt16          GetType() const
                        { return nType; }

    virtual SfxChildWinInfo GetInfo() const;
    void                SaveStatus(const SfxChildWinInfo& rInfo);

    static void         RegisterChildWindow(SfxModule*, const SfxChildWinFactory&);

    static std::unique_ptr<SfxChildWindow> CreateChildWindow( sal_uInt16, vcl::Window*, SfxBindings*, SfxChildWinInfo const &);
    void                SetHideNotDelete( bool bOn );
    bool                IsHideNotDelete() const;
    bool                IsVisible() const;
    void                SetWantsFocus( bool );
    bool                WantsFocus() const;

    virtual bool        QueryClose();
    const css::uno::Reference< css::frame::XFrame >&   GetFrame() const;
    void                SetFrame( const css::uno::Reference< css::frame::XFrame > & );

    SAL_DLLPRIVATE static void InitializeChildWinFactory_Impl(sal_uInt16, SfxChildWinInfo&);
    void                SetVisible_Impl( bool bVis );
    SAL_DLLPRIVATE void SetWorkWindow_Impl( SfxWorkWindow* );
    SAL_DLLPRIVATE void Activate_Impl();

    SAL_DLLPRIVATE void SetFactory_Impl( const SfxChildWinFactory* );
};

const int nCloseResponseToJustHide = -42;

#define SFX_DECL_CHILDWINDOW(Class) \
    public  :   \
        static  std::unique_ptr<SfxChildWindow> CreateImpl(vcl::Window *pParent, sal_uInt16 nId, \
                    SfxBindings *pBindings, SfxChildWinInfo* pInfo ); \
        static  void RegisterChildWindow (bool bVisible=false, SfxModule *pMod=nullptr, SfxChildWindowFlags nFlags=SfxChildWindowFlags::NONE); \
        virtual SfxChildWinInfo GetInfo() const override

#define SFX_DECL_CHILDWINDOW_WITHID(Class) \
        SFX_DECL_CHILDWINDOW(Class); \
        static  sal_uInt16 GetChildWindowId ()\

#define SFX_IMPL_CHILDWINDOW(Class, MyID) \
        SFX_IMPL_POS_CHILDWINDOW(Class, MyID, CHILDWIN_NOPOS)

#define SFX_IMPL_CHILDWINDOW_WITHID(Class, MyID) \
        SFX_IMPL_POS_CHILDWINDOW_WITHID(Class, MyID, CHILDWIN_NOPOS)

#define SFX_IMPL_POS_CHILDWINDOW(Class, MyID, Pos) \
        std::unique_ptr<SfxChildWindow> Class::CreateImpl( vcl::Window *pParent, \
                sal_uInt16 nId, SfxBindings *pBindings, SfxChildWinInfo* pInfo ) \
                {   \
                    return std::make_unique<Class>(pParent, nId, pBindings, pInfo);\
                } \
        void    Class::RegisterChildWindow (bool bVis, SfxModule *pMod, SfxChildWindowFlags nFlags)   \
                {   \
                    SfxChildWinFactory aFact( \
                        Class::CreateImpl, MyID, Pos );   \
                    aFact.aInfo.nFlags |= nFlags;  \
                    aFact.aInfo.bVisible = bVis;         \
                    SfxChildWindow::RegisterChildWindow(pMod, aFact); \
                }

#define SFX_IMPL_POS_CHILDWINDOW_WITHID(Class, MyID, Pos) \
        SFX_IMPL_POS_CHILDWINDOW(Class, MyID, Pos) \
        sal_uInt16 Class::GetChildWindowId () \
                { return MyID; } \

#define SFX_IMPL_MODELESSDIALOGCONTOLLER(Class, MyID)    \
        SFX_IMPL_CHILDWINDOW(Class, MyID)       \
        SfxChildWinInfo Class::GetInfo() const \
        {                                       \
            SfxChildWinInfo aInfo = SfxChildWindow::GetInfo();     \
            static_cast<const SfxModelessDialogController*>(GetController().get())->FillInfo( aInfo );  \
            return aInfo; }

#define SFX_IMPL_MODELESSDIALOGCONTOLLER_WITHID(Class, MyID)    \
        SFX_IMPL_CHILDWINDOW_WITHID(Class, MyID)       \
        SfxChildWinInfo Class::GetInfo() const \
        {                                       \
            SfxChildWinInfo aInfo = SfxChildWindow::GetInfo();     \
            static_cast<const SfxModelessDialogController*>(GetController().get())->FillInfo( aInfo );  \
            return aInfo; }

#define SFX_IMPL_DOCKINGWINDOW(Class, MyID) \
        SFX_IMPL_CHILDWINDOW(Class, MyID)       \
        SfxChildWinInfo Class::GetInfo() const \
        {                                       \
            SfxChildWinInfo aInfo = SfxChildWindow::GetInfo();     \
            static_cast<SfxDockingWindow*>(GetWindow())->FillInfo( aInfo );  \
            return aInfo; }

#define SFX_IMPL_DOCKINGWINDOW_WITHID(Class, MyID) \
        SFX_IMPL_CHILDWINDOW_WITHID(Class, MyID)       \
        SfxChildWinInfo Class::GetInfo() const \
        {                                       \
            SfxChildWinInfo aInfo = SfxChildWindow::GetInfo();     \
            static_cast<SfxDockingWindow*>(GetWindow())->FillInfo( aInfo );  \
            return aInfo; }

bool GetPosSizeFromString( std::u16string_view rStr, Point& rPos, Size& rSize );

bool GetSplitSizeFromString( std::u16string_view rStr, Size& rSize );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
