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

#include <sal/config.h>

#include <sfx2/dllapi.h>
#include <sal/types.h>
#include <o3tl/typed_flags_set.hxx>
#include <vcl/window.hxx>
#include <com/sun/star/frame/XFrame.hpp>

#include <sfx2/shell.hxx>
#include <sfx2/chalign.hxx>

#include <sfx2/bindings.hxx>
    // complete SfxBindings for complete SfxChildWinCtor, SfxChildWinContextCtor
    // under -fsanitize=function

class SfxWorkWindow;
class SfxModule;
class SfxShell;
class SfxChildWindow;
class SfxChildWindowContext;

enum class SfxChildWindowFlags
{
    NONE            = 0x00,
    ZOOMIN          = 0x01, // Fully retracted Float
    FORCEDOCK       = 0x04, // Float forbidden
    TASK            = 0x10, // ChildWindow inside the Task
    CANTGETFOCUS    = 0x20, // ChildWindow can not get focus
    ALWAYSAVAILABLE = 0x40, // ChildWindow is never disabled
    NEVERHIDE       = 0x80  // ChildWindow is can always made
                            // visible/is visible
};

namespace o3tl
{
    template<> struct typed_flags<SfxChildWindowFlags> : is_typed_flags<SfxChildWindowFlags, 0xf5> {};
}


#define CHILDWIN_NOPOS            USHRT_MAX

// ChildWindow Configuration
struct SAL_DLLPUBLIC_RTTI SfxChildWinInfo
{
    bool                bVisible;
    Point               aPos;
    Size                aSize;
    SfxChildWindowFlags nFlags;
    OUString            aExtraString;
    OUString            aModule;
    OString             aWinState;

                        SfxChildWinInfo()
                        {
                            bVisible = false;
                            nFlags = SfxChildWindowFlags::NONE;
                        }
    bool                GetExtraData_Impl( SfxChildAlignment    *pAlign,
                                           SfxChildAlignment    *pLastAlign = nullptr,
                                           Size                 *pSize = nullptr,
                                           sal_uInt16               *pLine = nullptr,
                                           sal_uInt16               *pPos = nullptr ) const;
};

// ChildWindow factory methods
typedef SfxChildWindow* (*SfxChildWinCtor)( vcl::Window *pParentWindow,
                                            sal_uInt16 nId,
                                            SfxBindings *pBindings,
                                            SfxChildWinInfo *pInfo);

// ChildWindowsContexts factory methods
typedef SfxChildWindowContext* (*SfxChildWinContextCtor)( vcl::Window *pParentWindow,
                                            SfxBindings *pBindings,
                                            SfxChildWinInfo *pInfo);
struct SfxChildWinContextFactory
{
    SfxChildWinContextCtor  pCtor;      // Factory method
    sal_uInt16              nContextId; // Idenifier for SfxInterface

    SfxChildWinContextFactory( SfxChildWinContextCtor pTheCtor, sal_uInt16 nID )
        : pCtor(pTheCtor)
        , nContextId(nID)
    {}
};

class SfxChildWinContextArr_Impl;

struct SFX2_DLLPUBLIC SfxChildWinFactory
{
    SfxChildWinCtor             pCtor;  // Factory method
    sal_uInt16                  nId;    // ChildWindow-Id ( SlotId )
    SfxChildWinInfo             aInfo;  // Configuration
    sal_uInt16                  nPos;   // Position in UI
    SfxChildWinContextArr_Impl *pArr;   // Array for Contexts

    SfxChildWinFactory( SfxChildWinCtor pTheCtor, sal_uInt16 nID, sal_uInt16 n );
    ~SfxChildWinFactory();
};

class FloatingWindow;
struct SfxChildWindow_Impl;
class SFX2_DLLPUBLIC SfxChildWindowContext
{
friend class SfxChildWindow;
    VclPtr<vcl::Window> pWindow;
    sal_uInt16          nContextId;

protected:
                        SfxChildWindowContext( sal_uInt16 nId );

public:
    virtual             ~SfxChildWindowContext();

    void                SetWindow( vcl::Window* pWin )
                        { pWindow=pWin; }
    vcl::Window*        GetWindow() const
                        { return pWindow; }
    sal_uInt16          GetContextId() const
                        { return nContextId; }

    FloatingWindow*     GetFloatingWindow() const;

    virtual void        Resizing( Size& rSize );
    static void         RegisterChildWindowContext(SfxModule*, sal_uInt16, SfxChildWinContextFactory*);
};

class SFX2_DLLPUBLIC SfxChildWindow
{
    VclPtr<vcl::Window>        pParent;        // parent window ( Topwindow )
    sal_uInt16                 nType;          // ChildWindow-Id
    VclPtr<vcl::Window>        pWindow;         // actual contents
    SfxChildAlignment          eChildAlignment; // Current css::drawing::Alignment
    SfxChildWindow_Impl*       pImp;            // Implementation data
    SfxChildWindowContext*     pContext;        // With context-sensitive ChildWindows:
                                                 // Another window in pWindow
    SAL_DLLPRIVATE void ClearWorkwin();

protected:
    void                SetWindow(const VclPtr<vcl::Window>& p) { pWindow = p; }

                        SfxChildWindow(vcl::Window *pParentWindow, sal_uInt16 nId);

public:
    virtual             ~SfxChildWindow();
    void                Destroy();
    vcl::Window*        GetWindow() const
                        { return pWindow; }
    vcl::Window*        GetParent() const
                        { return pParent; }
    SfxChildAlignment   GetAlignment() const
                        { return eChildAlignment; }
    void                SetAlignment(SfxChildAlignment eAlign);
    Size                GetSizePixel() const
                        { return pWindow->GetSizePixel(); }
    virtual void        Hide();
    virtual void        Show( ShowFlags nFlags );
    bool                CanGetFocus() const;
    sal_uInt16          GetPosition();
    sal_uInt16          GetType()
                        { return nType; }

    void                CreateContext( sal_uInt16 nContextId, SfxBindings& );
    sal_uInt16          GetContextId() const
                        { return pContext ? pContext->GetContextId(): 0; }

    vcl::Window*        GetContextWindow() const
                        { return pContext ? pContext->GetWindow(): nullptr; }

    vcl::Window*        GetContextWindow( SfxModule *pModule ) const;

    virtual SfxChildWinInfo GetInfo() const;
    void                SaveStatus(const SfxChildWinInfo& rInfo);

    static void         RegisterChildWindow(SfxModule*, SfxChildWinFactory*);

    static SfxChildWindow* CreateChildWindow( sal_uInt16, vcl::Window*, SfxBindings*, SfxChildWinInfo&);
    void                SetHideNotDelete( bool bOn );
    bool                IsHideNotDelete() const;
    bool                IsHideAtToggle() const;
    bool                IsVisible() const;
    void                SetWantsFocus( bool );
    bool                WantsFocus() const;

    virtual bool        QueryClose();
    css::uno::Reference< css::frame::XFrame >           GetFrame();
    void                SetFrame( const css::uno::Reference< css::frame::XFrame > & );

    SAL_DLLPRIVATE static void InitializeChildWinFactory_Impl(sal_uInt16, SfxChildWinInfo&);
    void                SetVisible_Impl( bool bVis );
    SAL_DLLPRIVATE void SetWorkWindow_Impl( SfxWorkWindow* );
    SAL_DLLPRIVATE void Activate_Impl();
    SAL_DLLPRIVATE void Deactivate_Impl();

    SAL_DLLPRIVATE SfxChildWindowContext*
                        GetContext_Impl() const
                        { return pContext; }
    SAL_DLLPRIVATE void SetFactory_Impl( SfxChildWinFactory* );
};


//! soon obsolete !
#define SFX_DECL_CHILDWINDOW_CONTEXT(Class) \
        static  SfxChildWindowContext* CreateImpl(vcl::Window *pParent, \
                    SfxBindings *pBindings, SfxChildWinInfo* pInfo ); \
        static  void RegisterChildWindowContext(SfxModule *pMod=nullptr); \

//! The Macro of the future ...
#define SFX_DECL_CHILDWINDOWCONTEXT(Class) \
        static  SfxChildWindowContext* CreateImpl(vcl::Window *pParent, \
                    SfxBindings *pBindings, SfxChildWinInfo* pInfo ); \
        static  void RegisterChildWindowContext(sal_uInt16, SfxModule *pMod=nullptr); \

//! soon obsolete !
#define SFX_IMPL_CHILDWINDOW_CONTEXT(Class, MyID, ShellClass) \
        SfxChildWindowContext* Class::CreateImpl( vcl::Window *pParent, \
                SfxBindings *pBindings, SfxChildWinInfo* pInfo ) \
        {   \
            SfxChildWindowContext *pContext = new Class(pParent, \
                    /* cast is safe here! */static_cast< sal_uInt16 >(ShellClass::GetInterfaceId()), \
                    pBindings,pInfo); \
            return pContext; \
        } \
        void    Class::RegisterChildWindowContext(SfxModule* pMod)   \
        {   \
            SfxChildWinContextFactory *pFact = new SfxChildWinContextFactory( \
                Class::CreateImpl, \
                /* cast is safe here! */static_cast< sal_uInt16 >(ShellClass::GetInterfaceId()) );   \
            SfxChildWindowContext::RegisterChildWindowContext(pMod, MyID, pFact); \
        }

//! The Macro of the future ...
// As a parameter and because of ContextId, CreateImpl must be handed the
// factory. As long as Id is set to 0 and patched in
// SfxChildWindow::CreateContext
#define SFX_IMPL_CHILDWINDOWCONTEXT(Class, MyID) \
        SfxChildWindowContext* Class::CreateImpl( vcl::Window *pParent, \
                SfxBindings *pBindings, SfxChildWinInfo* pInfo ) \
        {   \
            SfxChildWindowContext *pContext = new Class(pParent,0,pBindings,pInfo);\
            return pContext; \
        } \
        void    Class::RegisterChildWindowContext(sal_uInt16 nId, SfxModule* pMod)   \
        {   \
            SfxChildWinContextFactory *pFact = new SfxChildWinContextFactory( \
                Class::CreateImpl, nId );   \
            SfxChildWindowContext::RegisterChildWindowContext(pMod, MyID, pFact); \
        }

#define SFX_DECL_CHILDWINDOW(Class) \
    public  :   \
        static  SfxChildWindow* CreateImpl(vcl::Window *pParent, sal_uInt16 nId, \
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
        SfxChildWindow* Class::CreateImpl( vcl::Window *pParent, \
                sal_uInt16 nId, SfxBindings *pBindings, SfxChildWinInfo* pInfo ) \
                {   \
                    SfxChildWindow *pWin = new Class(pParent, nId, pBindings, pInfo);\
                    return pWin; \
                } \
        void    Class::RegisterChildWindow (bool bVis, SfxModule *pMod, SfxChildWindowFlags nFlags)   \
                {   \
                    SfxChildWinFactory *pFact = new SfxChildWinFactory( \
                        Class::CreateImpl, MyID, Pos );   \
                    pFact->aInfo.nFlags |= nFlags;  \
                    pFact->aInfo.bVisible = bVis;         \
                    SfxChildWindow::RegisterChildWindow(pMod, pFact); \
                }

#define SFX_IMPL_POS_CHILDWINDOW_WITHID(Class, MyID, Pos) \
        SFX_IMPL_POS_CHILDWINDOW(Class, MyID, Pos) \
        sal_uInt16 Class::GetChildWindowId () \
                { return MyID; } \

#define SFX_IMPL_FLOATINGWINDOW(Class, MyID)    \
        SFX_IMPL_CHILDWINDOW(Class, MyID)       \
        SfxChildWinInfo Class::GetInfo() const \
        {                                       \
            SfxChildWinInfo aInfo = SfxChildWindow::GetInfo();     \
            static_cast<SfxFloatingWindow*>(GetWindow())->FillInfo( aInfo );  \
            return aInfo; }

#define SFX_IMPL_FLOATINGWINDOW_WITHID(Class, MyID)    \
        SFX_IMPL_CHILDWINDOW_WITHID(Class, MyID)       \
        SfxChildWinInfo Class::GetInfo() const \
        {                                       \
            SfxChildWinInfo aInfo = SfxChildWindow::GetInfo();     \
            static_cast<SfxFloatingWindow*>(GetWindow())->FillInfo( aInfo );  \
            return aInfo; }

#define SFX_IMPL_MODELESSDIALOG_WITHID(Class, MyID)    \
        SFX_IMPL_CHILDWINDOW_WITHID(Class, MyID)       \
        SfxChildWinInfo Class::GetInfo() const \
        {                                       \
            SfxChildWinInfo aInfo = SfxChildWindow::GetInfo();     \
            static_cast<SfxModelessDialog*>(GetWindow())->FillInfo( aInfo );  \
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

bool GetPosSizeFromString( const OUString& rStr, Point& rPos, Size& rSize );

bool GetSplitSizeFromString( const OUString& rStr, Size& rSize );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
