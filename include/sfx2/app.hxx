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
class SfxMedium;
class SfxMenuCtrlFactArr_Impl;
class SfxNewFileDialog;
class SfxObjectShell;
class SfxObjectShellArr_Impl;
class SfxProgress;
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
class SfxModule;
class SfxModule;
typedef ::std::vector<SfxModule*> SfxModuleArr_Impl;
class Window;
class INetURLObject;
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
DECL_OBJHINT( SfxStringHint, OUString );

#ifndef SFX_DECL_OBJECTSHELL_DEFINED
#define SFX_DECL_OBJECTSHELL_DEFINED
SV_DECL_REF(SfxObjectShell)
#endif

class SfxObjectShellLock;

class SFX2_DLLPUBLIC SfxApplication: public SfxShell
{
    SAL_DLLPRIVATE static SfxApplication* pApp;

    SfxAppData_Impl*            pAppData_Impl;

    DECL_DLLPRIVATE_LINK(       GlobalBasicErrorHdl_Impl, StarBASIC* );

    static SfxApplication*      Create();
    void                        Init();
    void                        Exit();
    void                        SettingsChange( sal_uInt16, const AppSettings & );
    void                        Quit();
    void                        Deinitialize();

public:
                                TYPEINFO();
                                SFX_DECL_INTERFACE(SFX_INTERFACE_SFXAPP)

                                SfxApplication();
                                ~SfxApplication();
    static SfxApplication*      GetOrCreate();

    // Resource Manager
    ResMgr*                     GetSfxResManager();
    static ResMgr*              CreateResManager( const char *pPrefix );

    // DDE
#if defined( WNT )
    long                        DdeExecute( const OUString& rCmd );
#endif
    sal_Bool                    InitializeDde();
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
    sal_uIntPtr                       LoadTemplate( SfxObjectShellLock& xDoc, const OUString& rFileName, sal_Bool bCopy=sal_True, SfxItemSet* pArgs = 0 );
    SfxTemplateDialog*          GetTemplateDialog();
    Window*                     GetTopWindow() const;

    // TODO/CLEANUP: make currently selected family a view property and so we don't need to query the status from the "TemplateCommon"
    ISfxTemplateCommon*         GetCurrentTemplateCommon( SfxBindings& );

    // members
    SfxFilterMatcher&           GetFilterMatcher();
    SfxProgress*                GetProgress() const;
    const OUString&               GetLastSaveDirectory() const;
    sal_uInt16                  GetFreeIndex();
    void                        ReleaseIndex(sal_uInt16 i);

    // Basic/Scripting
    static sal_Bool             IsXScriptURL( const OUString& rScriptURL );
    static OUString      ChooseScript();
    static void                 MacroOrganizer( sal_Int16 nTabId );
    static ErrCode              CallBasic( const OUString&, BasicManager*, SbxArray *pArgs, SbxValue *pRet );
    static ErrCode              CallAppBasic( const OUString& i_macroName, SbxArray* i_args = NULL, SbxValue* i_ret = NULL )
                                { return CallBasic( i_macroName, SfxApplication::GetOrCreate()->GetBasicManager(), i_args, i_ret ); }
    BasicManager*               GetBasicManager();
    com::sun::star::uno::Reference< com::sun::star::script::XLibraryContainer >
                                GetDialogContainer();
    com::sun::star::uno::Reference< com::sun::star::script::XLibraryContainer >
                                GetBasicContainer();
    StarBASIC*                  GetBasic();
    sal_uInt16                  SaveBasicAndDialogContainer() const;

    // misc.
    sal_Bool                        GetOptions(SfxItemSet &);
    void                        SetOptions(const SfxItemSet &);
    virtual void                Invalidate(sal_uInt16 nId = 0);
    void                        NotifyEvent(const SfxEventHint& rEvent, bool bSynchron = true );
    sal_Bool                        IsDowning() const;
    sal_Bool                        IsSecureURL( const INetURLObject &rURL, const OUString *pReferer ) const;
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
    SAL_DLLPRIVATE const OUString& GetLastDir_Impl() const;
    SAL_DLLPRIVATE void         SetLastDir_Impl( const OUString & );

    SAL_DLLPRIVATE void         Registrations_Impl();
    SAL_DLLPRIVATE SfxWorkWindow* GetWorkWindow_Impl(const SfxViewFrame *pFrame=0) const;

    // TODO/CLEANUP: still needed? -- unclear whether this comment
    // refers to the GetDisabledSlotList_Impl() method which was
    // already removed, or the below methods?
    SAL_DLLPRIVATE SfxSlotPool& GetAppSlotPool_Impl() const;
    SAL_DLLPRIVATE SfxModule*   GetModule_Impl();
    SAL_DLLPRIVATE ResMgr*      GetOffResManager_Impl();

    static bool loadBrandSvg(const char *pName, BitmapEx &rBitmap, int nWidth);

    /** loads the application logo as used in the impress slideshow pause screen */
    static BitmapEx GetApplicationLogo(long nWidth);
};

#define SFX_APP() SfxGetpApp()

//--------------------------------------------------------------------

inline SfxApplication* SfxGetpApp()
{
    return SfxApplication::GetOrCreate();
}

#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
