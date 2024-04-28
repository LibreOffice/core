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

#include <memory>
#include <sal/config.h>
#include <sfx2/dllapi.h>
#include <sal/types.h>
#include <comphelper/errcode.hxx>
#include <svl/poolitem.hxx>
#include <vcl/bitmapex.hxx>
#include <tools/link.hxx>
#include <com/sun/star/ui/XAcceleratorConfiguration.hpp>
#include <unordered_map>

#include <sfx2/shell.hxx>

namespace com::sun::star::frame { class XFrame; }
namespace com::sun::star::script { class XLibraryContainer; }

namespace weld { class Window; }

class BasicManager;
class DdeService;
class SfxAppData_Impl;
class SfxDispatcher;
class SfxEventHint;
class SfxItemSet;
class SfxObjectShell;
class SfxObjectShellLock;
class SfxProgress;
class SfxSlotPool;
class SfxViewFrame;
class StarBASIC;
class SfxWorkWindow;
class SfxFilterMatcher;
class SfxModule;
struct SfxChildWinFactory;
struct SfxStbCtrlFactory;
struct SfxTbxCtrlFactory;
class SbxArray;
class SbxValue;

namespace sfx2
{
    namespace sidebar {
        class Theme;
    }
}

enum class SfxToolsModule
{
    Math = 0,
    Calc = 1,
    Draw = 2,
    Writer = 3,
    Basic = 4,
    LAST = Basic
};

class SFX2_DLLPUBLIC SfxLinkItem final : public SfxPoolItem
{
    Link<SfxPoolItem const *, void> aLink;
public:
    SfxLinkItem( sal_uInt16 nWhichId, const Link<SfxPoolItem const *, void>& rValue ) : SfxPoolItem( nWhichId )
    {   aLink = rValue; }

    virtual SfxLinkItem*     Clone( SfxItemPool* = nullptr ) const override
    {   return new SfxLinkItem( *this ); }
    virtual bool             operator==( const SfxPoolItem& rL) const override
    {   return static_cast<const SfxLinkItem&>(rL).aLink == aLink; }
    const Link<SfxPoolItem const *, void>&
                             GetValue() const { return aLink; }
};

// This is a singleton class. Sad that there apparently is no other
// way than a comment like this to indicate that to the code reader.
class SFX2_DLLPUBLIC SfxApplication final : public SfxShell
{
    std::unique_ptr<SfxAppData_Impl>            pImpl;

    DECL_DLLPRIVATE_STATIC_LINK( SfxApplication, GlobalBasicErrorHdl_Impl, StarBASIC*, bool );

    SAL_DLLPRIVATE void        Deinitialize();

public:
                                SFX_DECL_INTERFACE(SFX_INTERFACE_SFXAPP)

private:
    /// SfxInterface initializer.
    SAL_DLLPRIVATE static void InitInterface_Impl();
    SAL_DLLPRIVATE SfxApplication();

public:
    SAL_DLLPRIVATE virtual ~SfxApplication() override;
    static SfxApplication*      GetOrCreate();
    static SfxApplication*      Get();

    // DDE
#if defined(_WIN32)
    static bool                 DdeExecute( const OUString& rCmd );
#endif
    SAL_DLLPRIVATE bool         InitializeDde();
    SAL_DLLPRIVATE const DdeService* GetDdeService() const;
    DdeService*                 GetDdeService();
#if defined(_WIN32)
    void                        AddDdeTopic( SfxObjectShell* );
#endif
    void                        RemoveDdeTopic( SfxObjectShell const * );

    // "static" methods
    /**
    * @param pArgs Takes ownership
    */
    ErrCodeMsg                  LoadTemplate( SfxObjectShellLock& xDoc, const OUString& rFileName, std::unique_ptr<SfxItemSet> pArgs );
    weld::Window*               GetTopWindow() const;

    // members
    SfxFilterMatcher&           GetFilterMatcher();
    SAL_DLLPRIVATE SfxProgress* GetProgress() const;
    SAL_DLLPRIVATE sal_uInt16   GetFreeIndex();
    SAL_DLLPRIVATE void         ReleaseIndex(sal_uInt16 i);

    // Basic/Scripting
    static bool                 IsXScriptURL( const OUString& rScriptURL );
    static OUString             ChooseScript(weld::Window *pParent);
    // if xDocFrame is present, then select that document in the macro organizer by default, otherwise it is typically "Application Macros"
    // that is preselected
    SAL_DLLPRIVATE static void  MacroOrganizer(weld::Window* pParent, const css::uno::Reference<css::frame::XFrame>& xDocFrame, sal_Int16 nTabId);
    static ErrCode              CallBasic( const OUString&, BasicManager*, SbxArray *pArgs, SbxValue *pRet );
    static ErrCode              CallAppBasic( const OUString& i_macroName )
                                { return CallBasic( i_macroName, SfxApplication::GetBasicManager(), nullptr, nullptr ); }
    static BasicManager*        GetBasicManager();
    css::script::XLibraryContainer * GetDialogContainer();
    css::script::XLibraryContainer * GetBasicContainer();
    static StarBASIC*           GetBasic();
    void                        SaveBasicAndDialogContainer() const;

    // misc.
    static void                 GetOptions(SfxItemSet &);
    static void                 SetOptions(const SfxItemSet &);
    SAL_DLLPRIVATE virtual void Invalidate(sal_uInt16 nId = 0) override;
    void                        NotifyEvent(const SfxEventHint& rEvent, bool bSynchron = true );
    bool                        IsDowning() const;
    void                        ResetLastDir();

    SAL_DLLPRIVATE SfxDispatcher* GetAppDispatcher_Impl();
    SAL_DLLPRIVATE SfxDispatcher* GetDispatcher_Impl();

    SAL_DLLPRIVATE void         Initialize_Impl();

    SAL_DLLPRIVATE SfxAppData_Impl* Get_Impl() const { return pImpl.get(); }

    // Object-Factories/global arrays
    SAL_DLLPRIVATE void         RegisterChildWindow_Impl(SfxModule*, const SfxChildWinFactory&);
    SAL_DLLPRIVATE void         RegisterStatusBarControl_Impl(SfxModule*, const SfxStbCtrlFactory&);
    SAL_DLLPRIVATE void         RegisterToolBoxControl_Impl( SfxModule*, const SfxTbxCtrlFactory&);
    SAL_DLLPRIVATE SfxTbxCtrlFactory* GetTbxCtrlFactory(const std::type_info& rSlotType, sal_uInt16 nSlotID) const;
    SAL_DLLPRIVATE SfxStbCtrlFactory* GetStbCtrlFactory(const std::type_info& rSlotType, sal_uInt16 nSlotID) const;
    SAL_DLLPRIVATE SfxChildWinFactory* GetChildWinFactoryById(sal_uInt16 nId) const;
    SAL_DLLPRIVATE std::vector<SfxViewFrame*>& GetViewFrames_Impl() const;
    SAL_DLLPRIVATE std::vector<SfxViewShell*>& GetViewShells_Impl() const;
    /* unordered_map<ModuleName+Language, acceleratorConfigurationClassInstance> */
    SAL_DLLPRIVATE std::unordered_map<OUString, css::uno::Reference<css::ui::XAcceleratorConfiguration>>& GetAcceleratorConfs_Impl() const;
    SAL_DLLPRIVATE std::vector<SfxObjectShell*>& GetObjectShells_Impl() const;
    SAL_DLLPRIVATE void         SetViewFrame_Impl(SfxViewFrame *pViewFrame);

    // Slot Methods
    // TODO/CLEANUP: still needed?
    SAL_DLLPRIVATE void         NewDocDirectExec_Impl(SfxRequest &);
    SAL_DLLPRIVATE static void  NewDocDirectState_Impl(SfxItemSet &);
    SAL_DLLPRIVATE void         NewDocExec_Impl(SfxRequest &);
    SAL_DLLPRIVATE void         OpenDocExec_Impl(SfxRequest &);
    SAL_DLLPRIVATE void         OpenRemoteExec_Impl(SfxRequest &);
    SAL_DLLPRIVATE void         SignPDFExec_Impl(SfxRequest&);
    SAL_DLLPRIVATE void         MiscExec_Impl(SfxRequest &);
    SAL_DLLPRIVATE void         MiscState_Impl(SfxItemSet &);
    SAL_DLLPRIVATE static void  PropExec_Impl(SfxRequest const &);
    SAL_DLLPRIVATE static void  PropState_Impl(SfxItemSet &);
    SAL_DLLPRIVATE void         OfaExec_Impl(SfxRequest &);
    SAL_DLLPRIVATE static void  OfaState_Impl(SfxItemSet &);

    SAL_DLLPRIVATE void         SetProgress_Impl(SfxProgress *);
    SAL_DLLPRIVATE const OUString& GetLastDir_Impl() const;
    SAL_DLLPRIVATE void         SetLastDir_Impl( const OUString & );

    SAL_DLLPRIVATE static void  Registrations_Impl();
    SAL_DLLPRIVATE SfxWorkWindow* GetWorkWindow_Impl(const SfxViewFrame *pFrame) const;

    // TODO/CLEANUP: still needed? -- unclear whether this comment
    // refers to the GetDisabledSlotList_Impl() method which was
    // already removed, or the below methods?
    SAL_DLLPRIVATE SfxSlotPool& GetAppSlotPool_Impl() const;
    SAL_DLLPRIVATE static SfxModule* GetModule_Impl();

    static void                 SetModule(SfxToolsModule nSharedLib, std::unique_ptr<SfxModule> pModule);
    static SfxModule*           GetModule(SfxToolsModule nSharedLib);

    static bool loadBrandSvg(const char *pName, BitmapEx &rBitmap, int nWidth);

    /** loads the application logo as used in the impress slideshow pause screen */
    static BitmapEx GetApplicationLogo(tools::Long nWidth);

    /** if true then dialog/infobar notifications like the tip of the day or
        version change infobar should be suppressed */
    static bool IsHeadlessOrUITest();

    static bool IsTipOfTheDayDue();

    /** this Theme contains Images so must be deleted before DeInitVCL */
    SAL_DLLPRIVATE sfx2::sidebar::Theme & GetSidebarTheme();
};

inline SfxApplication* SfxGetpApp()
{
    return SfxApplication::Get();
}

#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
