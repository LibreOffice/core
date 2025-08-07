/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/types.h>
#include <basegfx/basegfxdllapi.h>
#include <memory>
#include <unordered_map>

namespace basegfx
{
    class SystemDependentData;
    typedef std::shared_ptr<SystemDependentData> SystemDependentData_SharedPtr;
    typedef std::weak_ptr<SystemDependentData> SystemDependentData_WeakPtr;

    class BASEGFX_DLLPUBLIC SystemDependentDataManager
    {
    private:
        // noncopyable
        SystemDependentDataManager(const SystemDependentDataManager&) = delete;
        SystemDependentDataManager& operator=(const SystemDependentDataManager&) = delete;

    public:
        SystemDependentDataManager();
        virtual ~SystemDependentDataManager();

        // call from (and with) SystemDependentData objects when start/end/touch
        // usage is needed
        virtual void startUsage(basegfx::SystemDependentData_SharedPtr& rData) = 0;
        virtual void endUsage(basegfx::SystemDependentData_SharedPtr& rData) = 0;
        virtual void touchUsage(basegfx::SystemDependentData_SharedPtr& rData) = 0;

        // flush all buffered data (e.g. cleanup/shutdown)
        virtual void flushAll() = 0;
    };

    // (S)ystem(D)ependent(D)ata_Type
    enum class BASEGFX_DLLPUBLIC SDD_Type : sal_uInt16 {
        SDDType_CairoPathGeometry,
        SDDType_CairoSurface,
        SDDType_ID2D1PathGeometry,
        SDDType_ID2D1Bitmap,
        SDDType_BitmapHelper,
        SDDType_MaskHelper,
        SDDType_CairoPath,
        SDDType_ModifiedBitmapEx,
        SDDType_ModifiedBitmap,
        SDDType_GraphicsPath,
        SDDType_GdiPlusBitmap
    };

    class BASEGFX_DLLPUBLIC SystemDependentData
    {
    private:
        // noncopyable
        SystemDependentData(const SystemDependentData&) = delete;
        SystemDependentData& operator=(const SystemDependentData&) = delete;

        // reference to a SystemDependentDataManager, probably
        // a single, globally used one, but not necessarily
        SystemDependentDataManager&     mrSystemDependentDataManager;

        // Type identifier
        SDD_Type                        maSystemDependentDataType;

        // Buffered CalculatedCycles, result of estimations using
        // getHoldCyclesInSeconds and estimateUsageInBytes, executed
        // using getHoldCyclesInSeconds. StartValue is 0 to detect
        // not-yet-calculated state
        sal_uInt32                      mnCalculatedCycles;

    public:
        SystemDependentData(
            SystemDependentDataManager& rSystemDependentDataManager,
            SDD_Type aSystemDependentDataType);
        virtual ~SystemDependentData();

        // allow access to call startUsage/endUsage/touchUsage
        // using getSystemDependentDataManager()
        SystemDependentDataManager& getSystemDependentDataManager() { return mrSystemDependentDataManager; }

        // read access to SDD_Type
        SDD_Type getSystemDependentDataType() const { return maSystemDependentDataType; }

        // Calculate HoldCyclesInSeconds based on using
        // getHoldCyclesInSeconds and estimateUsageInBytes, the
        // result is created once on-demand and buffered in
        // mnCalculatedCycles
        sal_uInt32 calculateCombinedHoldCyclesInSeconds() const;

        // Allow read access to the calculated cycles in seconds, this
        // can be e.g. used to determine if this instance got added
        sal_uInt32 getCombinedHoldCyclesInSeconds() const { return mnCalculatedCycles; }

        // Size estimation of the entry in bytes - does not have to
        // be used, but should be. Default returns zero what
        // means there is no size estimation available. Override to
        // offer useful data if you want to have better caching.
        virtual sal_Int64 estimateUsageInBytes() const;
    };

    class BASEGFX_DLLPUBLIC SystemDependentDataHolder
    {
    private:
        // Possibility to hold System-Dependent B2DPolygon-Representations
        std::unordered_map< SDD_Type, SystemDependentData_WeakPtr > maSystemDependentReferences;

        // noncopyable
        SystemDependentDataHolder(const SystemDependentDataHolder&) = delete;
        SystemDependentDataHolder& operator=(const SystemDependentDataHolder&) = delete;

    public:
        SystemDependentDataHolder();
        virtual ~SystemDependentDataHolder();

        void addOrReplaceSystemDependentData(SystemDependentData_SharedPtr& rData);
        SystemDependentData_SharedPtr getSystemDependentData(SDD_Type aType) const;
    };
} // end of namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
