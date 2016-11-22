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
#ifndef INCLUDED_SFX2_APP_HXX
#define INCLUDED_SFX2_APP_HXX

#include <sal/config.h>
#include <sfx2/dllapi.h>
#include <sal/types.h>
#include <tools/errcode.hxx>
#include <svl/smplhint.hxx>
#include <svl/poolitem.hxx>
#include <vcl/image.hxx>
#include <tools/ref.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/script/XLibraryContainer.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>

// too many files including sfx2/app.hxx use VCL Application class but don't include the
// header file because in former times SfxApplication was derived from it
#include <vcl/svapp.hxx>

#include <sfx2/shell.hxx>

class Timer;
class WorkWindow;
class ISfxTemplateCommon;
class BasicManager;
class DdeService;
class PrinterDialog;
class Point;
class Rectangle;
class AppSettings;
struct SfxChildWinContextFactory;
class SfxAppData_Impl;
class SfxBindings;
class SfxChildWinFactArr_Impl;
class SfxChildWindow;
class SfxDispatcher;
class SfxEventConfiguration;
class SfxEventHint;
class SfxItemSet;
class SfxMedium;
class SfxMenuCtrlFactArr_Impl;
class SfxNewFileDialog;
class SfxObjectShell;
class SfxObjectShellArr_Impl;
class SfxProgress;
class SfxSlotPool;
class SfxStbCtrlFactArr_Impl;
class SfxTbxCtrlFactArr_Impl;
class SfxViewFrame;
class SfxViewFrameArr_Impl;
class SfxViewShell;
class SfxViewShellArr_Impl;
class StarBASIC;
class SfxWorkWindow;
class SfxFilterMatcher;
class SfxModule;
class SfxModule;
namespace vcl { class Window; }
struct SfxChildWinFactory;
struct SfxMenuCtrlFactory;
struct SfxStbCtrlFactory;
struct SfxTbxCtrlFactory;
class SimpleResMgr;
class ModalDialog;
class SbxArray;
class SbxValue;

namespace sfx2
{
    class SvLinkSource;
    namespace sidebar {
        class Theme;
    }
}

class SFX2_DLLPUBLIC SfxLinkItem : public SfxPoolItem
{
    Link<SfxPoolItem*, void> aLink;
public:
    SfxLinkItem( sal_uInt16 nWhichId, const Link<SfxPoolItem*, void>& rValue ) : SfxPoolItem( nWhichId )
    {   aLink = rValue; }

    virtual SfxPoolItem*     Clone( SfxItemPool* = nullptr ) const override
    {   return new SfxLinkItem( *this ); }
    virtual bool             operator==( const SfxPoolItem& rL) const override
    {   return static_cast<const SfxLinkItem&>(rL).aLink == aLink; }
    const Link<SfxPoolItem*, void>&
                             GetValue() const { return aLink; }
};

#ifndef SFX_DECL_OBJECTSHELL_DEFINED
#define SFX_DECL_OBJECTSHELL_DEFINED
typedef tools::SvRef<SfxObjectShell> SfxObjectShellRef;
#endif

class SfxObjectShellLock;

class SFX2_DLLPUBLIC SfxApplication: public SfxShell
{
    SfxAppData_Impl*            pAppData_Impl;

    DECL_DLLPRIVATE_LINK_TYPED( GlobalBasicErrorHdl_Impl, StarBASIC*, bool );

    void                        Deinitialize();

public:
                                SFX_DECL_INTERFACE(SFX_INTERFACE_SFXAPP)

private:
    /// SfxInterface initializer.
    static void InitInterface_Impl();

public:

                                SfxApplication();
                                virtual ~SfxApplication();
    static SfxApplication*      GetOrCreate();
    static SfxApplication*      Get();

    // Resource Manager
    static ResMgr*              GetSfxResManager();

    // DDE
#if defined( WNT )
    long                        DdeExecute( const OUString& rCmd );
#endif
    bool                        InitializeDde();
    const DdeService*           GetDdeService() const;
    DdeService*                 GetDdeService();
#if defined( WNT )
    void                        AddDdeTopic( SfxObjectShell* );
#endif
    void                        RemoveDdeTopic( SfxObjectShell* );

    // "static" methods
    /**
    * @param pArgs Takes ownership
    */
    sal_uIntPtr                       LoadTemplate( SfxObjectShellLock& xDoc, const OUString& rFileName, bool bCopy=true, SfxItemSet* pArgs = nullptr );
    vcl::Window*                     GetTopWindow() const;

    // members
    SfxFilterMatcher&           GetFilterMatcher();
    SfxProgress*                GetProgress() const;
    const OUString&               GetLastSaveDirectory() const;
    sal_uInt16                  GetFreeIndex();
    void                        ReleaseIndex(sal_uInt16 i);

    // Basic/Scripting
    static bool                 IsXScriptURL( const OUString& rScriptURL );
    static OUString             ChooseScript();
    static void                 MacroOrganizer( sal_Int16 nTabId );
    static ErrCode              CallBasic( const OUString&, BasicManager*, SbxArray *pArgs, SbxValue *pRet );
    static ErrCode              CallAppBasic( const OUString& i_macroName, SbxArray* i_args = nullptr, SbxValue* i_ret = nullptr )
                                { return CallBasic( i_macroName, SfxApplication::GetBasicManager(), i_args, i_ret ); }
    static BasicManager*        GetBasicManager();
    css::script::XLibraryContainer * GetDialogContainer();
    css::script::XLibraryContainer * GetBasicContainer();
    static StarBASIC*           GetBasic();
    sal_uInt16                  SaveBasicAndDialogContainer() const;

    // misc.
    bool                        GetOptions(SfxItemSet &);
    void                        SetOptions(const SfxItemSet &);
    virtual void                Invalidate(sal_uInt16 nId = 0) override;
    void                        NotifyEvent(const SfxEventHint& rEvent, bool bSynchron = true );
    bool                        IsDowning() const;
    void                        ResetLastDir();

    SAL_DLLPRIVATE SfxDispatcher* GetAppDispatcher_Impl();
    SAL_DLLPRIVATE SfxDispatcher* GetDispatcher_Impl();

    SAL_DLLPRIVATE void         SetOptions_Impl(const SfxItemSet &);
    SAL_DLLPRIVATE bool         Initialize_Impl();

    SAL_DLLPRIVATE SfxAppData_Impl* Get_Impl() const { return pAppData_Impl; }

    // Object-Factories/global arrays
    SAL_DLLPRIVATE void         RegisterChildWindow_Impl(SfxModule*, SfxChildWinFactory*);
    SAL_DLLPRIVATE void         RegisterChildWindowContext_Impl(SfxModule*, sal_uInt16, SfxChildWinContextFactory*);
    SAL_DLLPRIVATE void         RegisterStatusBarControl_Impl(SfxModule*, const SfxStbCtrlFactory&);
    SAL_DLLPRIVATE void         RegisterMenuControl_Impl(SfxModule*, const SfxMenuCtrlFactory&);
    SAL_DLLPRIVATE void         RegisterToolBoxControl_Impl( SfxModule*, const SfxTbxCtrlFactory&);
    SAL_DLLPRIVATE SfxTbxCtrlFactArr_Impl& GetTbxCtrlFactories_Impl() const;
    SAL_DLLPRIVATE SfxStbCtrlFactArr_Impl& GetStbCtrlFactories_Impl() const;
    SAL_DLLPRIVATE SfxMenuCtrlFactArr_Impl& GetMenuCtrlFactories_Impl() const;
    SAL_DLLPRIVATE SfxChildWinFactArr_Impl& GetChildWinFactories_Impl() const;
    SAL_DLLPRIVATE SfxViewFrameArr_Impl& GetViewFrames_Impl() const;
    SAL_DLLPRIVATE SfxViewShellArr_Impl& GetViewShells_Impl() const;
    SAL_DLLPRIVATE SfxObjectShellArr_Impl& GetObjectShells_Impl() const;
    SAL_DLLPRIVATE void         SetViewFrame_Impl(SfxViewFrame *pViewFrame);

    // Slot Methods
    // TODO/CLEANUP: still needed?
    SAL_DLLPRIVATE void         NewDocDirectExec_Impl(SfxRequest &);
    SAL_DLLPRIVATE void         NewDocExec_Impl(SfxRequest &);
    SAL_DLLPRIVATE void         OpenDocExec_Impl(SfxRequest &);
    SAL_DLLPRIVATE void         OpenRemoteExec_Impl(SfxRequest &);
    SAL_DLLPRIVATE void         MiscExec_Impl(SfxRequest &);
    SAL_DLLPRIVATE void         MiscState_Impl(SfxItemSet &);
    SAL_DLLPRIVATE void         PropExec_Impl(SfxRequest &);
    SAL_DLLPRIVATE void         PropState_Impl(SfxItemSet &);
    SAL_DLLPRIVATE void         OfaExec_Impl(SfxRequest &);
    SAL_DLLPRIVATE void         OfaState_Impl(SfxItemSet &);

    SAL_DLLPRIVATE void         SetProgress_Impl(SfxProgress *);
    SAL_DLLPRIVATE const OUString& GetLastDir_Impl() const;
    SAL_DLLPRIVATE void         SetLastDir_Impl( const OUString & );

    SAL_DLLPRIVATE void         Registrations_Impl();
    SAL_DLLPRIVATE SfxWorkWindow* GetWorkWindow_Impl(const SfxViewFrame *pFrame=nullptr) const;

    // TODO/CLEANUP: still needed? -- unclear whether this comment
    // refers to the GetDisabledSlotList_Impl() method which was
    // already removed, or the below methods?
    SAL_DLLPRIVATE SfxSlotPool& GetAppSlotPool_Impl() const;
    SAL_DLLPRIVATE SfxModule*   GetModule_Impl();

    static bool loadBrandSvg(const char *pName, BitmapEx &rBitmap, int nWidth);

    /** loads the application logo as used in the impress slideshow pause screen */
    static BitmapEx GetApplicationLogo(long nWidth);

    /** this Theme contains Images so must be deleted before DeInitVCL */
    sfx2::sidebar::Theme & GetSidebarTheme();
};

inline SfxApplication* SfxGetpApp()
{
    return SfxApplication::GetOrCreate();
}

#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
