/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <basegfx/utils/systemdependentdata.hxx>

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
    MinimalSystemDependentDataManager::MinimalSystemDependentDataManager()
    :   SystemDependentDataManager(),
        maSystemDependentDataReferences()
    {
    }

    MinimalSystemDependentDataManager::~MinimalSystemDependentDataManager()
    {
    }

    void MinimalSystemDependentDataManager::startUsage(basegfx::SystemDependentData_SharedPtr& rData)
    {
        if(rData)
        {
            maSystemDependentDataReferences.insert(rData);
        }
    }

    void MinimalSystemDependentDataManager::endUsage(basegfx::SystemDependentData_SharedPtr& rData)
    {
        if(rData)
        {
            maSystemDependentDataReferences.erase(rData);
        }
    }

    void MinimalSystemDependentDataManager::touchUsage(basegfx::SystemDependentData_SharedPtr& /* rData */)
    {
    }

} // namespace basegfx

namespace basegfx
{
    SystemDependentData::SystemDependentData(
        SystemDependentDataManager& rSystemDependentDataManager,
        sal_uInt32 nHoldCycles)
    :   mrSystemDependentDataManager(rSystemDependentDataManager),
        mnHoldCycles(nHoldCycles)
    {
    }

    SystemDependentData::~SystemDependentData()
    {
    }
} // namespace basegfx

namespace basegfx
{
    SystemDependentDataHolder::SystemDependentDataHolder()
    :   maSystemDependentReferences()
    {
    }

    SystemDependentDataHolder::~SystemDependentDataHolder()
    {
        for(auto& candidate : maSystemDependentReferences)
        {
            basegfx::SystemDependentData_SharedPtr aData(candidate.second.lock());

            if(aData)
            {
                aData->getSystemDependentDataManager().endUsage(aData);
            }
        }
    }

    bool SystemDependentDataHolder::addOrReplaceSystemDependentData(basegfx::SystemDependentData_SharedPtr& rData)
    {
        if(!rData)
        {
            return false;
        }

        const size_t hash_code(typeid(*rData.get()).hash_code());
        auto result(maSystemDependentReferences.find(hash_code));

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

        if(result == maSystemDependentReferences.end())
        {
            maSystemDependentReferences[hash_code] = rData;
            rData->getSystemDependentDataManager().startUsage(rData);

            return true;
        }

        return false;
    }

    SystemDependentData_SharedPtr SystemDependentDataHolder::getSystemDependentData(size_t hash_code) const
    {
        basegfx::SystemDependentData_SharedPtr aRetval;
        auto result(maSystemDependentReferences.find(hash_code));

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
