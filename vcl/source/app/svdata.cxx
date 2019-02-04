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

#include <string.h>

#include <comphelper/lok.hxx>
#include <comphelper/processfactory.hxx>
#include <rtl/process.h>
#include <tools/gen.hxx>
#include <unotools/resmgr.hxx>
#include <uno/current_context.hxx>
#include <sal/log.hxx>

#include <vcl/button.hxx>
#include <vcl/configsettings.hxx>
#include <vcl/dockwin.hxx>
#include <vcl/layout.hxx>
#include <vcl/menu.hxx>
#include <vcl/print.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <vcl/virdev.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/uitest/logger.hxx>
#include <scrwnd.hxx>
#include <helpwin.hxx>
#include <vcl/dialog.hxx>
#include <salinst.hxx>
#include <salframe.hxx>
#include <salgdi.hxx>
#include <svdata.hxx>
#include <window.h>
#include <salimestatus.hxx>
#include <salsys.hxx>
#include <strings.hrc>
#include <units.hrc>
#include <print.h>

#include <com/sun/star/accessibility/MSAAService.hpp>

#include <officecfg/Office/Common.hxx>

#include <config_folders.h>
#include <config_features.h>
#if HAVE_FEATURE_OPENGL
#include <vcl/opengl/OpenGLContext.hxx>
#endif
#include <basegfx/utils/systemdependentdata.hxx>
#include <cppuhelper/basemutex.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::awt;

namespace
{
    struct private_aImplSVData :
        public rtl::Static<ImplSVData, private_aImplSVData> {};
}

ImplSVData* ImplGetSVData() {
    return &private_aImplSVData::get();
}

SalSystem* ImplGetSalSystem()
{
    ImplSVData* pSVData = ImplGetSVData();
    if( ! pSVData->mpSalSystem )
        pSVData->mpSalSystem.reset( pSVData->mpDefInst->CreateSalSystem() );
    return pSVData->mpSalSystem.get();
}

void ImplDeInitSVData()
{
    ImplSVData* pSVData = ImplGetSVData();

    // delete global instance data
    pSVData->mpSettingsConfigItem.reset();

    pSVData->mpDockingManager.reset();

    pSVData->maCtrlData.maFieldUnitStrings.clear();
    pSVData->maCtrlData.maCleanUnitStrings.clear();
    pSVData->maPaperNames.clear();
}

namespace
{
    typedef ::std::map< basegfx::SystemDependentData_SharedPtr, sal_uInt32 > EntryMap;

    class SystemDependentDataBuffer : public basegfx::SystemDependentDataManager, protected cppu::BaseMutex
    {
    private:
        std::unique_ptr<Timer>  maTimer;
        EntryMap                maEntries;

        DECL_LINK(implTimeoutHdl, Timer *, void);

    public:
        SystemDependentDataBuffer(const sal_Char* pDebugName)
        :   basegfx::SystemDependentDataManager(),
            maTimer(std::make_unique<Timer>(pDebugName)),
            maEntries()
        {
            maTimer->SetTimeout(1000);
            maTimer->SetInvokeHandler(LINK(this, SystemDependentDataBuffer, implTimeoutHdl));
        }

        virtual ~SystemDependentDataBuffer() override
        {
            flushAll();
        }

        void startUsage(basegfx::SystemDependentData_SharedPtr& rData) override
        {
            ::osl::MutexGuard aGuard(m_aMutex);
            EntryMap::iterator aFound(maEntries.find(rData));

            if(aFound == maEntries.end())
            {
                if(maEntries.empty() && maTimer)
                {
                    maTimer->Start();
                }

                maEntries[rData] = rData->calculateCombinedHoldCyclesInSeconds();
            }
        }

        void endUsage(basegfx::SystemDependentData_SharedPtr& rData) override
        {
            ::osl::MutexGuard aGuard(m_aMutex);
            EntryMap::iterator aFound(maEntries.find(rData));

            if(aFound != maEntries.end())
            {
                maEntries.erase(aFound);

                if(maEntries.empty() && maTimer)
                {
                    maTimer->Stop();
                }
            }
        }

        void touchUsage(basegfx::SystemDependentData_SharedPtr& rData) override
        {
            ::osl::MutexGuard aGuard(m_aMutex);
            EntryMap::iterator aFound(maEntries.find(rData));

            if(aFound != maEntries.end())
            {
                aFound->second = rData->calculateCombinedHoldCyclesInSeconds();
            }
        }

        void flushAll() override
        {
            ::osl::MutexGuard aGuard(m_aMutex);
            EntryMap::iterator aIter(maEntries.begin());

            if(maTimer)
            {
                maTimer->Stop();
                maTimer.reset();
            }

            while(aIter != maEntries.end())
            {
                EntryMap::iterator aDelete(aIter);
                ++aIter;
                maEntries.erase(aDelete);
            }
        }
    };

    IMPL_LINK_NOARG(SystemDependentDataBuffer, implTimeoutHdl, Timer *, void)
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        EntryMap::iterator aIter(maEntries.begin());

        while(aIter != maEntries.end())
        {
            if(aIter->second)
            {
                aIter->second--;
                ++aIter;
            }
            else
            {
                EntryMap::iterator aDelete(aIter);
                ++aIter;
                maEntries.erase(aDelete);

                if(maEntries.empty() && maTimer)
                {
                    maTimer->Stop();
                }
            }
        }

        if(!maEntries.empty() && maTimer)
        {
            maTimer->Start();
        }
    }
}

basegfx::SystemDependentDataManager& ImplGetSystemDependentDataManager()
{
    static SystemDependentDataBuffer aSystemDependentDataBuffer("vcl SystemDependentDataBuffer aSystemDependentDataBuffer");

    return aSystemDependentDataBuffer;
}

/// Returns either the application window, or the default GL context window
vcl::Window* ImplGetDefaultWindow()
{
    ImplSVData* pSVData = ImplGetSVData();
    if ( pSVData->maWinData.mpAppWin )
        return pSVData->maWinData.mpAppWin;
    else
        return ImplGetDefaultContextWindow();
}

/// returns the default window created to hold the persistent VCL GL context.
vcl::Window *ImplGetDefaultContextWindow()
{
    ImplSVData* pSVData = ImplGetSVData();

    // Double check locking on mpDefaultWin.
    if ( !pSVData->mpDefaultWin )
    {
        SolarMutexGuard aGuard;

        if (!pSVData->mpDefaultWin && !pSVData->mbDeInit)
        {
            try
            {
                SAL_INFO( "vcl", "ImplGetDefaultWindow(): No AppWindow" );

                pSVData->mpDefaultWin = VclPtr<WorkWindow>::Create( nullptr, WB_DEFAULTWIN );
                pSVData->mpDefaultWin->SetText( "VCL ImplGetDefaultWindow" );

#if HAVE_FEATURE_OPENGL
                // Add a reference to the default context so it never gets deleted
                rtl::Reference<OpenGLContext> pContext = pSVData->mpDefaultWin->GetGraphics()->GetOpenGLContext();
                if( pContext.is() )
                    pContext->acquire();
#endif
            }
            catch (const css::uno::Exception& e)
            {
                 SAL_WARN("vcl", "unable to create Default Window: " << e);
            }
        }
    }

    return pSVData->mpDefaultWin;
}

const std::locale& ImplGetResLocale()
{
    ImplSVData* pSVData = ImplGetSVData();
    if (!pSVData->mbResLocaleSet || comphelper::LibreOfficeKit::isActive())
    {
        pSVData->maResLocale = Translate::Create("vcl");
        pSVData->mbResLocaleSet = true;
    }
    return pSVData->maResLocale;
}

OUString VclResId(const char* pId)
{
    return Translate::get(pId, ImplGetResLocale());
}

const FieldUnitStringList& ImplGetFieldUnits()
{
    ImplSVData* pSVData = ImplGetSVData();
    if( pSVData->maCtrlData.maFieldUnitStrings.empty() )
    {
        sal_uInt32 nUnits = SAL_N_ELEMENTS(SV_FUNIT_STRINGS);
        pSVData->maCtrlData.maFieldUnitStrings.reserve( nUnits );
        for (sal_uInt32 i = 0; i < nUnits; i++)
        {
            std::pair<OUString, FieldUnit> aElement(VclResId(SV_FUNIT_STRINGS[i].first), SV_FUNIT_STRINGS[i].second);
            pSVData->maCtrlData.maFieldUnitStrings.push_back( aElement );
        }
    }
    return pSVData->maCtrlData.maFieldUnitStrings;
}

const FieldUnitStringList& ImplGetCleanedFieldUnits()
{
    ImplSVData* pSVData = ImplGetSVData();
    if( pSVData->maCtrlData.maCleanUnitStrings.empty() )
    {
        const FieldUnitStringList& rUnits = ImplGetFieldUnits();
        size_t nUnits = rUnits.size();
        pSVData->maCtrlData.maCleanUnitStrings.reserve(nUnits);
        for (size_t i = 0; i < nUnits; ++i)
        {
            OUString aUnit(rUnits[i].first);
            aUnit = aUnit.replaceAll(" ", "");
            aUnit = aUnit.toAsciiLowerCase();
            std::pair<OUString, FieldUnit> aElement(aUnit, rUnits[i].second);
            pSVData->maCtrlData.maCleanUnitStrings.push_back(aElement);
        }
    }
    return pSVData->maCtrlData.maCleanUnitStrings;
}

DockingManager* ImplGetDockingManager()
{
    ImplSVData* pSVData = ImplGetSVData();
    if ( !pSVData->mpDockingManager )
        pSVData->mpDockingManager.reset(new DockingManager());

    return pSVData->mpDockingManager.get();
}

BlendFrameCache* ImplGetBlendFrameCache()
{
    ImplSVData* pSVData = ImplGetSVData();
    if ( !pSVData->mpBlendFrameCache)
        pSVData->mpBlendFrameCache.reset( new BlendFrameCache() );

    return pSVData->mpBlendFrameCache.get();
}

#ifdef _WIN32
bool ImplInitAccessBridge()
{
    ImplSVData* pSVData = ImplGetSVData();
    if( ! pSVData->mxAccessBridge.is() )
    {
        css::uno::Reference< XComponentContext > xContext(comphelper::getProcessComponentContext());

        if (!HasAtHook() && !getenv("SAL_FORCE_IACCESSIBLE2"))
        {
            SAL_INFO("vcl", "Apparently no running AT -> "
                     "not enabling IAccessible2 integration");
        }
        else
        {
            try {
                 pSVData->mxAccessBridge
                     = css::accessibility::MSAAService::create(xContext);
                 SAL_INFO("vcl", "got IAccessible2 bridge");
                 return true;
             } catch (css::uno::DeploymentException & e) {
                 SAL_WARN(
                    "vcl",
                    "got no IAccessible2 bridge" << e);
                 return false;
             }
        }
    }

    return true;
}
#endif

void LocaleConfigurationListener::ConfigurationChanged( utl::ConfigurationBroadcaster*, ConfigurationHints nHint )
{
    AllSettings::LocaleSettingsChanged( nHint );
}


ImplSVData::~ImplSVData() {}
ImplSVAppData::~ImplSVAppData() {}
ImplSVGDIData::~ImplSVGDIData() {}
ImplSVWinData::~ImplSVWinData() {}
ImplSVHelpData::~ImplSVHelpData() {}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
