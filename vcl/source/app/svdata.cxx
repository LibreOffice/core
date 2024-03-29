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
#include <comphelper/diagnose_ex.hxx>
#include <o3tl/string_view.hxx>
#include <unotools/resmgr.hxx>
#include <sal/log.hxx>

#include <configsettings.hxx>
#include <vcl/QueueInfo.hxx>
#include <vcl/cvtgrf.hxx>
#include <vcl/dockwin.hxx>
#include <vcl/fieldvalues.hxx>
#include <vcl/menu.hxx>
#include <vcl/print.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <vcl/virdev.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/uitest/logger.hxx>
#include <salframe.hxx>
#include <scrwnd.hxx>
#include <helpwin.hxx>
#include <vcl/toolkit/dialog.hxx>
#include <salinst.hxx>
#include <salgdi.hxx>
#include <svdata.hxx>
#include <salsys.hxx>
#include <windowdev.hxx>
#include <units.hrc>
#include <print.h>

#include <com/sun/star/accessibility/MSAAService.hpp>

#include <config_features.h>
#include <basegfx/utils/systemdependentdata.hxx>
#include <mutex>

using namespace com::sun::star::uno;

namespace
{
    struct private_aImplSVData :
        public rtl::Static<ImplSVData, private_aImplSVData> {};
    /// Default instance ensures that ImplSVData::mpHelpData is never null.
    struct private_aImplSVHelpData :
        public rtl::Static<ImplSVHelpData, private_aImplSVHelpData> {};

    /// Default instance ensures that ImplSVData::mpWinData is never null.
    struct private_aImplSVWinData :
        public rtl::Static<ImplSVWinData, private_aImplSVWinData> {};

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

    class SystemDependentDataBuffer final : public basegfx::SystemDependentDataManager
    {
    private:
        std::mutex m_aMutex;
        std::unique_ptr<AutoTimer> maTimer;
        EntryMap maEntries;

        DECL_LINK(implTimeoutHdl, Timer *, void);

    public:
        SystemDependentDataBuffer(const char* pDebugName)
        :   maTimer(std::make_unique<AutoTimer>(pDebugName))
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
            std::unique_lock aGuard(m_aMutex);
            EntryMap::iterator aFound(maEntries.find(rData));

            if(aFound == maEntries.end())
            {
                if(maTimer && !maTimer->IsActive())
                {
                    maTimer->Start();
                }

                maEntries[rData] = rData->calculateCombinedHoldCyclesInSeconds();
            }
        }

        void endUsage(basegfx::SystemDependentData_SharedPtr& rData) override
        {
            std::unique_lock aGuard(m_aMutex);
            EntryMap::iterator aFound(maEntries.find(rData));

            if(aFound != maEntries.end())
            {
                maEntries.erase(aFound);
            }
        }

        void touchUsage(basegfx::SystemDependentData_SharedPtr& rData) override
        {
            std::unique_lock aGuard(m_aMutex);
            EntryMap::iterator aFound(maEntries.find(rData));

            if(aFound != maEntries.end())
            {
                aFound->second = rData->calculateCombinedHoldCyclesInSeconds();
            }
        }

        void flushAll() override
        {
            std::unique_lock aGuard(m_aMutex);

            if(maTimer)
            {
                maTimer->Stop();
                maTimer.reset();
            }

            maEntries.clear();
        }
    };

    IMPL_LINK_NOARG(SystemDependentDataBuffer, implTimeoutHdl, Timer *, void)
    {
        std::unique_lock aGuard(m_aMutex);
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
                aIter = maEntries.erase(aIter);
            }
        }

        if (maEntries.empty())
            maTimer->Stop();
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
    if (pSVData->maFrameData.mpAppWin)
        return pSVData->maFrameData.mpAppWin;
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

                pSVData->mpDefaultWin = VclPtr<WorkWindow>::Create(nullptr, WB_DEFAULTWIN);
                pSVData->mpDefaultWin->SetText( "VCL ImplGetDefaultWindow" );
            }
            catch (const css::uno::Exception&)
            {
                TOOLS_WARN_EXCEPTION("vcl", "unable to create Default Window");
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

OUString VclResId(TranslateId aId)
{
    return Translate::get(aId, ImplGetResLocale());
}

const FieldUnitStringList& ImplGetFieldUnits()
{
    ImplSVData* pSVData = ImplGetSVData();
    if( pSVData->maCtrlData.maFieldUnitStrings.empty() )
    {
        sal_uInt32 nUnits = std::size(SV_FUNIT_STRINGS);
        pSVData->maCtrlData.maFieldUnitStrings.reserve( nUnits );
        for (sal_uInt32 i = 0; i < nUnits; i++)
        {
            std::pair<OUString, FieldUnit> aElement(VclResId(SV_FUNIT_STRINGS[i].first), SV_FUNIT_STRINGS[i].second);
            pSVData->maCtrlData.maFieldUnitStrings.push_back( aElement );
        }
    }
    return pSVData->maCtrlData.maFieldUnitStrings;
}

namespace vcl
{
    FieldUnit EnglishStringToMetric(std::u16string_view rEnglishMetricString)
    {
        sal_uInt32 nUnits = std::size(SV_FUNIT_STRINGS);
        for (sal_uInt32 i = 0; i < nUnits; ++i)
        {
            if (o3tl::equalsAscii(rEnglishMetricString, SV_FUNIT_STRINGS[i].first.getId()))
                return SV_FUNIT_STRINGS[i].second;
        }
        return FieldUnit::NONE;
    }
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
             } catch (css::uno::DeploymentException &) {
                 TOOLS_WARN_EXCEPTION(
                    "vcl",
                    "got no IAccessible2 bridge");
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

ImplSVWinData* CreateSVWinData()
{
    if (!comphelper::LibreOfficeKit::isActive())
        return nullptr;

    ImplSVWinData* p = new ImplSVWinData;

    ImplSVData* pSVData = ImplGetSVData();
    assert(pSVData && pSVData->mpWinData);

    p->mpFocusWin = pSVData->mpWinData->mpFocusWin;
    return p;
}

void DestroySVWinData(ImplSVWinData* pData)
{
    delete pData;
}

void SetSVWinData(ImplSVWinData* pSVWinData)
{
    if (!comphelper::LibreOfficeKit::isActive())
        return;

    ImplSVData* pSVData = ImplGetSVData();
    assert(pSVData != nullptr);

    if (pSVData->mpWinData == pSVWinData)
        return;

    // If current one is the static, clean it up to avoid having lingering references.
    if (pSVData->mpWinData == &private_aImplSVWinData::get())
    {
        pSVData->mpWinData->mpFocusWin.reset();
    }

    pSVData->mpWinData = pSVWinData;
    if (pSVData->mpWinData == nullptr)
    {
        pSVData->mpWinData = &private_aImplSVWinData::get(); // Never leave it null.
    }
}

ImplSVData::ImplSVData()
{
    mpHelpData = &private_aImplSVHelpData::get();
    mpWinData = &private_aImplSVWinData::get();
}

void ImplSVData::dropCaches()
{
    // we are iterating over a map and doing erase while inside a loop which is doing erase
    // hence we can't use clear() here
    maGDIData.maScaleCache.remove_if([](const lru_scale_cache::key_value_pair_t&)
                                                { return true; });

    maGDIData.maThemeDrawCommandsCache.clear();
    maGDIData.maThemeImageCache.clear();
}

void ImplSVData::dumpState(rtl::OStringBuffer &rState)
{
    rState.append("\nScaleCache:\t");
    rState.append(static_cast<sal_Int32>(maGDIData.maScaleCache.size()));
    rState.append("\t items:");

    for (auto it = maGDIData.maScaleCache.begin();
         it != maGDIData.maScaleCache.begin(); ++it)
    {
        rState.append("\n\t");
        rState.append(static_cast<sal_Int32>(it->first.maDestSize.Width()));
        rState.append("x");
        rState.append(static_cast<sal_Int32>(it->first.maDestSize.Height()));
    }
}

ImplSVHelpData* CreateSVHelpData()
{
    if (!comphelper::LibreOfficeKit::isActive())
        return nullptr;

    ImplSVHelpData* pNewData = new ImplSVHelpData;

    // Set options set globally
    ImplSVHelpData& aStaticHelpData = private_aImplSVHelpData::get();
    pNewData->mbContextHelp = aStaticHelpData.mbContextHelp;
    pNewData->mbExtHelp = aStaticHelpData.mbExtHelp;
    pNewData->mbExtHelpMode = aStaticHelpData.mbExtHelpMode;
    pNewData->mbOldBalloonMode = aStaticHelpData.mbOldBalloonMode;
    pNewData->mbBalloonHelp = aStaticHelpData.mbBalloonHelp;
    pNewData->mbQuickHelp = aStaticHelpData.mbQuickHelp;

    return pNewData;
}

void DestroySVHelpData(ImplSVHelpData* pSVHelpData)
{
    if (!comphelper::LibreOfficeKit::isActive())
        return;

    // Change the SVData's help date if necessary
    if(ImplGetSVData()->mpHelpData == pSVHelpData)
    {
        ImplGetSVData()->mpHelpData = &private_aImplSVHelpData::get();
    }

    if(pSVHelpData)
    {
        ImplDestroyHelpWindow(*pSVHelpData, false);
        delete pSVHelpData;
    }
}

void SetSVHelpData(ImplSVHelpData* pSVHelpData)
{
    if (!comphelper::LibreOfficeKit::isActive())
        return;

    ImplSVData* pSVData = ImplGetSVData();
    if (pSVData->mpHelpData == pSVHelpData)
        return;

    // If current one is the static, clean it up to avoid having lingering references.
    if (pSVData->mpHelpData == &private_aImplSVHelpData::get())
    {
        pSVData->mpHelpData->mpHelpWin.reset();
    }

    pSVData->mpHelpData = pSVHelpData;
    if (pSVData->mpHelpData == nullptr)
    {
        pSVData->mpHelpData = &private_aImplSVHelpData::get(); // Never leave it null.
    }
}

ImplSVHelpData& ImplGetSVHelpData()
{
    ImplSVData* pSVData = ImplGetSVData();
    if(pSVData->mpHelpData)
    {
        return *pSVData->mpHelpData;
    }
    else
    {
        return private_aImplSVHelpData::get();
    }
}

ImplSVData::~ImplSVData() {}

ImplSVAppData::ImplSVAppData()
{
    m_bUseSystemLoop = getenv("SAL_USE_SYSTEM_LOOP") != nullptr;
    SAL_WARN_IF(m_bUseSystemLoop, "vcl.schedule", "Overriding to run LO on system event loop!");
}

ImplSVAppData::~ImplSVAppData() {}
ImplSVGDIData::~ImplSVGDIData() {}
ImplSVFrameData::~ImplSVFrameData() {}
ImplSVWinData::~ImplSVWinData() {}
ImplSVHelpData::~ImplSVHelpData() {}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
