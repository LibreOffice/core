/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <basegfx/utils/systemdependentdata.hxx>
#include <config_fuzzers.h>
#include <math.h>

namespace basegfx
{
    SystemDependentDataManager::SystemDependentDataManager()
    {
    }

    SystemDependentDataManager::~SystemDependentDataManager()
    {
    }
} // namespace basegfx

namespace basegfx
{
    SystemDependentData::SystemDependentData(
        SystemDependentDataManager& rSystemDependentDataManager,
        SDD_Type aSystemDependentDataType)
    :   mrSystemDependentDataManager(rSystemDependentDataManager)
    , maSystemDependentDataType(aSystemDependentDataType)
    , mnCalculatedCycles(0)
    {
    }

    SystemDependentData::~SystemDependentData()
    {
    }

    sal_uInt32 SystemDependentData::calculateCombinedHoldCyclesInSeconds() const
    {
#if ENABLE_FUZZERS
        return 0;
#endif

        // already set, use it
        if(0 != mnCalculatedCycles)
            return mnCalculatedCycles;

        // get size in bytes as base for estimation
        const sal_Int64 nBytes(estimateUsageInBytes());

        // tdf#129845 as indicator for no need to buffer trivial data, stay at and
        // return zero. As border, use 450 bytes. For polygons, this means to buffer
        // starting with ca. 50 points (GDIPLUS uses 9 bytes per coordinate). For
        // Bitmap data this means to more or less always buffer (as it was before).
        // Note that this is the method for all Buffered data, independent of
        // graphic target, so it is possible to refine that if needed in the
        // system-dependent parts of impl (see getOrCreateCairoSurfaceHelper). It
        // is also possible to make calculateCombinedHoldCyclesInSeconds virtual
        // and override for your needs.
        if (nBytes < 450)
            return mnCalculatedCycles;

        // We have seen that for very huge images the hold time was too short, e.g.
        // for a very huge bitmap (see fHugeDataInBytes below) it was about 20s. This
        // is not enough, we have learned that we need to hold huge data longer.
        // In the impl before I took the memory aspect more into account, so I used
        // sqrt() to have less HoldTime the bigger the nBytes gets by using a flattening
        // curve. With having identified that remain time is important for those cases,
        // we can go back to linear. This means that we can explicitely choose a
        // HoldTime, get a factor and simply apply that to all buffered data.
        // Setup:
        //  fHugeDataInBytes -> 300000000
        //  fHugeTimeToHoldInSeconds -> 1200
        //  fTimeToHoldInSecondsPerByte -> 0.000004
        // Example outcome (for implied raw BitmapData):
        //  320x200 -> 1.024s
        //  800x600 -> 7.68s
        //  1600x1200 -> 30.72s
        //  15000x5000 -> 1200s (as targeted)
        constexpr double fHugeDataInBytes(15000*5000*4);
        constexpr double fHugeTimeToHoldInSeconds(20 * 60);
        constexpr double fTimeToHoldInSecondsPerByte(fHugeTimeToHoldInSeconds/fHugeDataInBytes);

        // set locally (once, on-demand created, non-zero)
        mnCalculatedCycles = static_cast<sal_uInt32>(1.0 + nBytes * fTimeToHoldInSecondsPerByte);

        return mnCalculatedCycles;
    }

    sal_Int64 SystemDependentData::estimateUsageInBytes() const
    {
        // default implementation has no idea
        return 0;
    }
} // namespace basegfx

namespace basegfx
{
    SystemDependentDataHolder::SystemDependentDataHolder()
    {
    }

    SystemDependentDataHolder::~SystemDependentDataHolder()
    {
        for(const auto& candidate : maSystemDependentReferences)
        {
            basegfx::SystemDependentData_SharedPtr aData(candidate.second.lock());

            if(aData)
            {
                aData->getSystemDependentDataManager().endUsage(aData);
            }
        }
    }

    void SystemDependentDataHolder::addOrReplaceSystemDependentData(basegfx::SystemDependentData_SharedPtr& rData)
    {
        auto result(maSystemDependentReferences.find(rData->getSystemDependentDataType()));

        if(result != maSystemDependentReferences.end())
        {
            basegfx::SystemDependentData_SharedPtr aData(result->second.lock());

            if(aData)
            {
                aData->getSystemDependentDataManager().endUsage(aData);
            }

            maSystemDependentReferences.erase(result);
            result = maSystemDependentReferences.end();
        }

        maSystemDependentReferences[rData->getSystemDependentDataType()] = rData;
        rData->getSystemDependentDataManager().startUsage(rData);
    }

    SystemDependentData_SharedPtr SystemDependentDataHolder::getSystemDependentData(SDD_Type aType) const
    {
        basegfx::SystemDependentData_SharedPtr aRetval;
        auto result(maSystemDependentReferences.find(aType));

        if(result != maSystemDependentReferences.end())
        {
            aRetval = result->second.lock();

            if(aRetval)
            {
                aRetval->getSystemDependentDataManager().touchUsage(aRetval);
            }
            else
            {
                const_cast< SystemDependentDataHolder* >(this)->maSystemDependentReferences.erase(result);
            }
        }

        return aRetval;
    }
} // namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
