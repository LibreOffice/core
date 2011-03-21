/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _SFXAPP_HXX
#define _SFXAPP_HXX

#include "sal/config.h"
#include "sfx2/dllapi.h"
#include "sal/types.h"
#include <tools/solar.h>
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
#include <vector>

class Timer;
class ApplicationEvent;
class WorkWindow;
class ISfxTemplateCommon;
class BasicManager;
class DdeService;
class GenLink;
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
class SfxMacroConfig;
class SfxMedium;
class SfxMenuCtrlFactArr_Impl;
class SfxNewFileDialog;
class SfxObjectShell;
class SfxObjectShellArr_Impl;
class SfxOptions;
class SfxProgress;
class SfxResourceManager;
class SfxSlotPool;
class SfxStbCtrlFactArr_Impl;
class SfxTbxCtrlFactArr_Impl;
class SfxTemplateDialog;
class SfxViewFrame;
class SfxViewFrameArr_Impl;
class SfxViewShell;
class SfxViewShellArr_Impl;
class StarBASIC;
class SfxWorkWindow;
class SfxFilterMatcher;
class SvUShorts;
class SfxModule;
class SfxModuleArr_Impl;
class Window;
class INetURLObject;
struct SfxConstant;
struct SfxChildWinFactory;
struct SfxMenuCtrlFactory;
struct SfxStbCtrlFactory;
struct SfxTbxCtrlFactory;
class SimpleResMgr;
class ModalDialog;
class SbxArray;
class SbxValue;

typedef ::std::vector< SfxMedium* > SfxMediumList;

namespace sfx2
{
    class SvLinkSource;
}

//====================================================================

class SfxLinkItem : public SfxPoolItem
{
    Link aLink;
public:
    virtual SfxPoolItem*     Clone( SfxItemPool* = 0 ) const
    {   return new SfxLinkItem( *this ); }
    virtual int              operator==( const SfxPoolItem& rL) const
    {   return ((SfxLinkItem&)rL).aLink == aLink; }
    SfxLinkItem( sal_uInt16 nWhichId, const Link& rValue ) : SfxPoolItem( nWhichId )
    {   aLink = rValue; }
    const Link&              GetValue() const { return aLink; }
};

//TODO/CLEANUP
//is apparently used only in SfxPickList/SfxFrameLoader
DECL_OBJHINT( SfxStringHint, String );

/*
typedef SfxPoolItem* (*SfxItemCreateFunc)();
class SfxItemFactory_Impl;
class SfxItemFactoryList
{
    List aList;
public:
    ~SfxItemFactoryList();

    SfxItemFactory_Impl* GetFactory_Impl( const SvGlobalName& rName ) const;
    SfxItemFactory_Impl* GetFactory_Impl( TypeId ) const;
    const SvGlobalName* GetGlobalName( const SfxPoolItem* pItem ) const;
    SfxPoolItem* Create(
        const SvGlobalName& rName, sal_uInt16 nId, SvStream* pStrm = 0) const;
    void         RegisterItemFactory(
        const SvGlobalName& rName, SfxItemCreateFunc );
};

#define REGISTER_ITEM( ItemClass, aGlobName )                                 \
RegisterItemFactory( aGlobName, ( SfxItemCreateFunc) ItemClass::StaticType() );
*/

#ifndef SFX_DECL_OBJECTSHELL_DEFINED
#define SFX_DECL_OBJECTSHELL_DEFINED
SV_DECL_REF(SfxObjectShell)
#endif

class SfxObjectShellLock;

class SFX2_DLLPUBLIC SfxApplication: public SfxShell
{
    SAL_DLLPRIVATE static ::osl::Mutex gMutex;
    SAL_DLLPRIVATE static SfxApplication* pApp;

    SfxAppData_Impl*            pAppData_Impl;

    DECL_DLLPRIVATE_LINK(       GlobalBasicErrorHdl_Impl, StarBASIC* );
    SAL_DLLPRIVATE sal_Bool     SaveAll_Impl(sal_Bool bPrompt = sal_False, sal_Bool bAutoSave = sal_False);
    SAL_DLLPRIVATE short        QuerySave_Impl(SfxObjectShell &, sal_Bool bAutoSave = sal_False);

    static SfxApplication*      Create();
    void                        Init();
    void                        Exit();
    void                        SettingsChange( sal_uInt16, const AppSettings & );
    void                        Main( );
    void                        PreInit( );
    void                        Quit();
    void                        Deinitialize();

public:
                                TYPEINFO();
                                SFX_DECL_INTERFACE(SFX_INTERFACE_SFXAPP)

                                SfxApplication();
                                ~SfxApplication();
    static SfxApplication*      GetOrCreate();

    // Resource Manager
    SfxResourceManager&         GetResourceManager() const;
    ResMgr*                     GetSfxResManager();
    SimpleResMgr*               GetSimpleResManager();
    static ResMgr*              CreateResManager( const char *pPrefix );
    SimpleResMgr*               CreateSimpleResManager();

    // DDE
    long                        DdeExecute( const String& rCmd );
    long                        DdeGetData( const String& rItem,
                                            const String& rMimeType,
                                        ::com::sun::star::uno::Any & rValue );
    long                        DdeSetData( const String& rItem,
                                            const String& rMimeType,
                                const ::com::sun::star::uno::Any & rValue );
    ::sfx2::SvLinkSource*       DdeCreateLinkSource( const String& rItem );
    sal_Bool                        InitializeDde();
    const DdeService*           GetDdeService() const;
    DdeService*                 GetDdeService();
    void                        AddDdeTopic( SfxObjectShell* );
    void                        RemoveDdeTopic( SfxObjectShell* );

    // "static" methods
    sal_uIntPtr                       LoadTemplate( SfxObjectShellLock& xDoc, const String& rFileName, sal_Bool bCopy=sal_True, SfxItemSet* pArgs = 0 );
    ::com::sun::star::uno::Reference< ::com::sun::star::task::XStatusIndicator > GetStatusIndicator() const;
    SfxTemplateDialog*          GetTemplateDialog();
    Window*                     GetTopWindow() const;

    // TODO/CLEANUP: make currently selected family a view property and so we don't need to query the status from the "TemplateCommon"
    ISfxTemplateCommon*         GetCurrentTemplateCommon( SfxBindings& );

    // members
    SfxFilterMatcher&           GetFilterMatcher();
    SfxProgress*                GetProgress() const;
    const String&               GetLastSaveDirectory() const;
    sal_uInt16                  GetFreeIndex();
    void                        ReleaseIndex(sal_uInt16 i);

    // Basic/Scripting
    static sal_Bool             IsXScriptURL( const String& rScriptURL );
    static ::rtl::OUString      ChooseScript();
    static void                 MacroOrganizer( sal_Int16 nTabId );
    static ErrCode              CallBasic( const String&, BasicManager*, SbxArray *pArgs, SbxValue *pRet );
    static ErrCode              CallAppBasic( const String& i_macroName, SbxArray* i_args = NULL, SbxValue* i_ret = NULL )
                                { return CallBasic( i_macroName, SfxApplication::GetOrCreate()->GetBasicManager(), i_args, i_ret ); }
    BasicManager*               GetBasicManager();
    com::sun::star::uno::Reference< com::sun::star::script::XLibraryContainer >
                                GetDialogContainer();
    com::sun::star::uno::Reference< com::sun::star::script::XLibraryContainer >
                                GetBasicContainer();
    StarBASIC*                  GetBasic();
    sal_uInt16                  SaveBasicManager() const;
    sal_uInt16                  SaveBasicAndDialogContainer() const;

    // misc.
    sal_Bool                        GetOptions(SfxItemSet &);
    void                        SetOptions(const SfxItemSet &);
    virtual void                Invalidate(sal_uInt16 nId = 0);
    void                        NotifyEvent(const SfxEventHint& rEvent, bool bSynchron = true );
    sal_Bool                        IsDowning() const;
    sal_Bool                        IsSecureURL( const INetURLObject &rURL, const String *pReferer ) const;
    static SfxObjectShellRef    DocAlreadyLoaded( const String &rName,
                                                  sal_Bool bSilent,
                                                  sal_Bool bActivate,
                                                  sal_Bool bForbidVisible = sal_False,
                                                  const String* pPostStr = 0);
    void                        ResetLastDir();

    SAL_DLLPRIVATE static SfxApplication* Get() { return pApp;}
    SAL_DLLPRIVATE SfxDispatcher* GetAppDispatcher_Impl();
    SAL_DLLPRIVATE SfxDispatcher* GetDispatcher_Impl();

    SAL_DLLPRIVATE sal_Bool         QueryExit_Impl();
    SAL_DLLPRIVATE void         SetOptions_Impl(const SfxItemSet &);
    SAL_DLLPRIVATE bool         Initialize_Impl();

    SAL_DLLPRIVATE SfxAppData_Impl* Get_Impl() const { return pAppData_Impl; }

    // Object-Factories/global arrays
    SAL_DLLPRIVATE void         RegisterChildWindow_Impl(SfxModule*, SfxChildWinFactory*);
    SAL_DLLPRIVATE void         RegisterChildWindowContext_Impl(SfxModule*, sal_uInt16, SfxChildWinContextFactory*);
    SAL_DLLPRIVATE void         RegisterStatusBarControl_Impl(SfxModule*, SfxStbCtrlFactory*);
    SAL_DLLPRIVATE void         RegisterMenuControl_Impl(SfxModule*, SfxMenuCtrlFactory*);
    SAL_DLLPRIVATE void         RegisterToolBoxControl_Impl( SfxModule*, SfxTbxCtrlFactory*);
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
    SAL_DLLPRIVATE void         MiscExec_Impl(SfxRequest &);
    SAL_DLLPRIVATE void         MiscState_Impl(SfxItemSet &);
    SAL_DLLPRIVATE void         PropExec_Impl(SfxRequest &);
    SAL_DLLPRIVATE void         PropState_Impl(SfxItemSet &);
    SAL_DLLPRIVATE void         INetExecute_Impl(SfxRequest &);
    SAL_DLLPRIVATE void         INetState_Impl(SfxItemSet &);
    SAL_DLLPRIVATE void         OfaExec_Impl(SfxRequest &);
    SAL_DLLPRIVATE void         OfaState_Impl(SfxItemSet &);

    SAL_DLLPRIVATE void         SetProgress_Impl(SfxProgress *);
    SAL_DLLPRIVATE const String& GetLastDir_Impl() const;
    SAL_DLLPRIVATE void         SetLastDir_Impl( const String & );

    SAL_DLLPRIVATE void         EnterAsynchronCall_Impl();
    SAL_DLLPRIVATE bool         IsInAsynchronCall_Impl() const;
    SAL_DLLPRIVATE void         LeaveAsynchronCall_Impl();
    SAL_DLLPRIVATE void         Registrations_Impl();
    SAL_DLLPRIVATE SfxWorkWindow* GetWorkWindow_Impl(const SfxViewFrame *pFrame=0) const;

    // TODO/CLEANUP: still needed?
    SAL_DLLPRIVATE SvUShorts*   GetDisabledSlotList_Impl();
    SAL_DLLPRIVATE SfxSlotPool& GetAppSlotPool_Impl() const;
    SAL_DLLPRIVATE SfxModule*   GetModule_Impl();
    SAL_DLLPRIVATE ResMgr*      GetOffResManager_Impl();

    /* loads a branded bitmap - that can be overridden per locale */
    static bool  LoadBrandBitmap (const char* pName, BitmapEx &rBitmap);
    /** loads the application logo as used in the about dialog and impress slideshow pause screen */
    static Image GetApplicationLogo();
};

#define SFX_APP() SfxGetpApp()

//--------------------------------------------------------------------

inline SfxApplication* SfxGetpApp()
{
    return SfxApplication::GetOrCreate();
}

#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
