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

#ifndef TD_NEWDESKTOP
#define TF_NEWDESKTOP 1
#endif

#include <tools/solar.h>

#if _SOLAR__PRIVATE
#include <tools/time.hxx>
#include <vcl/timer.hxx>
#endif

#include <vcl/svapp.hxx>
#include <bf_svtools/smplhint.hxx>
#include <bf_svtools/poolitem.hxx>
#include <tools/errcode.hxx>
#include <tools/ref.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/script/XLibraryContainer.hpp>

// whatwever will be today's name for explorer/frameset-docs
#define SFX_DESKTOP_HELPFILE_NAME "desktop"

#include <bf_sfx2/shell.hxx>
class ModalDialog;
class OutputDevice;
class KeyEvent;
class Point;
class Rectangle;
class Menu;
class Window;
class Config;
class INetURLObject;
class SimpleResMgr;
namespace binfilter {
class SvPseudoObject;
class SvStringsDtor;
class SvLinkSource;
class SvUShorts;
class SfxCancelManager;
class SfxHint;
class SfxItemSet;
class SfxEventConfigItem_Impl;

class StarBASIC;
class BasicManager;

#define PLUG_BROWSEHACK     42

#define SFX_DEMOKIND_FULL      1
#define SFX_DEMOKIND_COMPACT   2
#define SFX_DEMOKIND_TRYBUY    3
#define SFX_DEMOKIND_DEMO      4
#define SFX_DEMOKIND_INVALID   5
#define SFX_DEMOKIND_OLDTRYBUY 6
#define SFX_DEMOKIND_RUNTIME   7

enum SfxDBNameConvert {INI2NATIONAL, NATIONAL2INI };

#define SFX_BASICLIB_ADDMODE_NORMAL 	0
#define SFX_BASICLIB_ADDMODE_REFERENCE 	1
#define SFX_BASICLIB_ADDMODE_EXTERNAL 	2

class GenLink;
class DropEvent;
class AppSettings;
class SfxAppData_Impl;
class SfxApplication;
class SfxBasicTestWin;
class SfxConfigManager;
class SfxEventConfiguration;
class SfxEventHint;
class SfxFilter;
class SfxInterface;
class SfxMacroConfig;
class SfxMedium;
class SfxMediumList;
class SfxObjectFactory;
class SfxObjectFactoryArr_Impl;
class SfxObjectShell;
class SfxObjectShellArr_Impl;
class SfxOptions;
class SfxProgress;
class SfxResourceManager;
class SfxShellObject;
class SfxSlot;
class SfxSlotPool;
class SvData;
class SfxFilterMatcher;
class SfxModule;
class SfxModuleArr_Impl;
class ISfxModule;
class SfxObjectShellLock;
class SfxMiscCfg;
struct SfxConstant;

#define STARAPP_VERB "STARAPP"
#define PRESENTATION_LIVEMODE       ((USHORT)0x1000)
#define DYNAMIC_USERID_OFFSET	1000

//--------------------------------------------------------------------


// class SfxLoadEnvironment ------------------------------------------

class LoadEnvironment_Impl;
class SfxLoadEnvironment : public SvRefBase
{
    LoadEnvironment_Impl* _pImpl;
public:
    SfxLoadEnvironment( LoadEnvironment_Impl* pImpl );
    ~SfxLoadEnvironment();
};

// class SfxSysChangeHint --------------------------------------------

class SfxSysChangeHint : public SfxHint
{
    USHORT              _nType;
    AppSettings&        _rAppSettings;
public:
                        TYPEINFO();
                        SfxSysChangeHint(USHORT nType, const AppSettings &rSet )
                        : _nType(nType), _rAppSettings((AppSettings&) rSet) {}
    USHORT              GetChangeType() const { return _nType; }
    const AppSettings&  GetAppSettings() const { return _rAppSettings; }
};

//--------------------------------------------------------------------

/*  [Beschreibung]

    Kurzschreibweise f"ur SfxGetpApp(). Liefert den Pointer auf die
    SfxApplication Instanz.
*/

#define SFX_APP() SfxGetpApp()

//====================================================================

#ifndef SFX_DECL_OBJECTSHELL_DEFINED
#define SFX_DECL_OBJECTSHELL_DEFINED
SV_DECL_REF(SfxObjectShell)
SV_DECL_LOCK(SfxObjectShell)
#endif

void RegisterServices();

struct SfxApplication_Impl;
class SfxApplication: public SfxShell

/*  [Beschreibung]

    Die Klasse SfxApplication bietet eine erweiterte Funktionalit"at
    der StarView-Klasse MDIApplication und mu?als Basisklasse fÅr
    Applikationen, die das StarFramework verwenden wollen,
    eingesetzt werden.

    Sie bietet verschiedene Factories und Hooks in Form von
    virtuellen Methoden, um das default-Verhalten zu modifizieren.

    [Anmerkung]
    Von dieser Klasse mu?jede StarFramework-Applikation genau
    eine Instanz anlegen.
*/

{
    static SfxApplication*		pApp;
    struct SfxApplication_Impl* pImp;

    ULONG                       _nFeatures;
    SfxResourceManager*         pResMgr;
    SfxConfigManager*           pCfgMgr;    	// bin"are Config, "ubergreifend
    SfxOptions*                 pOptions;
    SfxAppData_Impl*            pAppData_Impl;

    BOOL                        bInInit:1;    // TRUE solange in Initialisierung
    BOOL                        bInExit:1;    // TRUE solange in Deinit.
    BOOL                        bDowning:1;   // TRUE ab Exit und danach
    BOOL						bCreatedExternal : 1;

    static SfxApplication*      Create();

protected:
    void                		SettingsChange( USHORT, const AppSettings & );

    virtual void                Init();
    virtual void                Exit();

    static void					SetApp( SfxApplication *pDerived );

public:
                                TYPEINFO();

                                SfxApplication();
                                ~SfxApplication();

    virtual void                Main( );

    void                		PreInit( );
    static SfxApplication*		GetOrCreate();
    void                		Quit();
    void						InitLabelResMgr( const char* );
    void						HandleAppEvent( const ApplicationEvent& rAppEvent );

    BOOL                        HasFeature( ULONG nFeature ) const;
    void 						SetFeatures( ULONG nFeatures )
                                {
                                    _nFeatures = nFeatures;
                                }

    String&                     GetSaveAsTargetURLHack();
    SfxFilterMatcher&           GetFilterMatcher();

    SfxResourceManager&         GetResourceManager() const
                                { return *pResMgr; }

    BasicManager*               GetBasicManager();
    StarBASIC*                  GetBasic();
    void                        EnterBasicCall();
    void                        LeaveBasicCall();
    SfxCancelManager*           GetCancelManager() const;

    BOOL                        IsInInit() const { return bInInit; }
    BOOL                        IsInExit() const { return bInExit; }
    BOOL                        IsDowning() const { return bDowning; }

    SfxMacroConfig*             GetMacroConfig() const;

    ResMgr*                     CreateResManager( const char *pPrefix );
    ResMgr*                     GetSfxResManager();

    const SfxFilter*            GetFilter(const SfxObjectFactory &rFact, const String &rFilterName) const;

    SfxProgress*                GetProgress() const;

    SfxOptions&                 GetOptions() { return *pOptions; }
    const USHORT*               GetOptionsRanges() const;
    const SfxOptions&           GetOptions() const { return *pOptions; }
    BOOL                        GetOptions(SfxItemSet &);
    void                        SetOptions(const SfxItemSet &);

    void                        ReleaseIndex(USHORT i);

    void                        NotifyEvent(const SfxEventHint& rEvent, FASTBOOL bSynchron = TRUE );
    SfxEventConfiguration*      GetEventConfig() const;

    SfxMiscCfg*					GetMiscConfig();
    void                        Deinitialize();

#if _SOLAR__PRIVATE
    SfxConfigManager*           GetConfigManager_Impl() const { return pCfgMgr; }
    USHORT                      ParseCommandLine_Impl();

    SfxObjectShellLock          NewDoc_Impl( const String& rFactory, const SfxItemSet* pSet = NULL );

    BOOL                        IsReadOnly_Impl(const String &rFile) const;
    FASTBOOL                    Initialize_Impl();

    SfxAppData_Impl*            Get_Impl() const { return pAppData_Impl; }

    // Object-Factories
    SfxObjectShellArr_Impl&     GetObjectShells_Impl() const;
    const String&               GetLastDir_Impl() const;
    void                        SetLastDir_Impl( const String & );
    void                        SetLastFilter_Impl( const String & );
    void                        BuildBasicData_Impl();
    StarBASIC*					GetBasic_Impl() const;

    FASTBOOL                    IsInAsynchronCall_Impl() const;
    void                        Registrations_Impl();
    void                        InvalidateWinSlots_Impl();

    static SfxObjectShellLock   CreateObject_Impl( SfxMedium* pMedium, const String& rUrl );
    SfxEventConfigItem_Impl*	GetEventConfig_Impl();
#endif
};

//--------------------------------------------------------------------

inline SfxApplication* SfxGetpApp()
{
    return SfxApplication::GetOrCreate();
}

//--------------------------------------------------------------------

//! Dummies f"ur zu implementierende Items
#define SfxApplicationItem SfxVoidItem
#define SfxDocumentListItem SfxVoidItem

}//end of namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
