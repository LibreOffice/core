/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_BASEGFX_SYSTEMDEPENDENTDATA_HXX
#define INCLUDED_BASEGFX_SYSTEMDEPENDENTDATA_HXX

#include <sal/types.h>
#include <basegfx/basegfxdllapi.h>
#include <memory>
#include <map>
#include <set>

namespace basegfx
{
    class SystemDependentData;
    typedef std::shared_ptr<SystemDependentData> SystemDependentData_SharedPtr;
    typedef std::weak_ptr<SystemDependentData> SystemDependentData_WeakPtr;
} // end of namespace basegfx

namespace basegfx
{
    class BASEGFX_DLLPUBLIC SystemDependentDataManager
    {
    private:
        // noncopyable
        SystemDependentDataManager(const SystemDependentDataManager&) = delete;
        SystemDependentDataManager& operator=(const SystemDependentDataManager&) = delete;

    public:
        SystemDependentDataManager();
        virtual ~SystemDependentDataManager();

        virtual void startUsage(basegfx::SystemDependentData_SharedPtr& rData) = 0;
        virtual void endUsage(basegfx::SystemDependentData_SharedPtr& rData) = 0;
        virtual void touchUsage(basegfx::SystemDependentData_SharedPtr& rData) = 0;
    };
} // end of namespace basegfx

namespace basegfx
{
    class BASEGFX_DLLPUBLIC MinimalSystemDependentDataManager : public SystemDependentDataManager
    {
    private:
        // example of a minimal SystemDependentDataManager. It *needs to hold*
        // a SystemDependentData_SharedPtr while SystemDependentDataHolder's will
        // use a SystemDependentData_WeakPtr. When the held SystemDependentData_SharedPtr
        // is deleted, the corresponding SystemDependentData_WeakPtr will get void.
        // To make this work, a minimal SystemDependentDataManager *has* to hold at
        // least that one SystemDependentData_SharedPtr.
        // That SystemDependentData_SharedPtr may be (e.g. Timer-based or ressource-based)
        // be freed then. This minimal implementation does never free it, so all stay valid.
        // The instances may still be removed by endUsage calls, but there is no
        // caching/buffering mechanism involved here at all. It's an example, but
        // not used - better use an advanced derivation of SystemDependentDataManager
        std::set< SystemDependentData_SharedPtr >   maSystemDependentDataReferences;

    public:
        MinimalSystemDependentDataManager();
        virtual ~MinimalSystemDependentDataManager();

        virtual void startUsage(basegfx::SystemDependentData_SharedPtr& rData);
        virtual void endUsage(basegfx::SystemDependentData_SharedPtr& rData);
        virtual void touchUsage(basegfx::SystemDependentData_SharedPtr& rData);
    };
} // end of namespace basegfx

namespace basegfx
{
    class BASEGFX_DLLPUBLIC SystemDependentData
    {
    private:
        // noncopyable
        SystemDependentData(const SystemDependentData&) = delete;
        SystemDependentData& operator=(const SystemDependentData&) = delete;

        // reference to a SystemDependentDataManager, probably
        // a single, globally used one, but not necessarily
        SystemDependentDataManager&     mrSystemDependentDataManager;

        // number of cycles a SystemDependentDataManager should/might
        // hold this instance - does not have to be used, but should be
        sal_uInt32                      mnHoldCycles;

    public:
        SystemDependentData(
            SystemDependentDataManager& rSystemDependentDataManager,
            sal_uInt32 nHoldCycles = 60);

        // CAUTION! It is VERY important to keep this base class
        // virtual, else typeid(class).hash_code() from derived classes
        // will NOT work what is ESSENTIAL for the SystemDependentData
        // mechanism to work properly. So DO NOT REMOVE virtual here, please.
        virtual ~SystemDependentData();

        // allow access to call startUsage/endUsage/touchUsage
        // using getSystemDependentDataManager()
        SystemDependentDataManager& getSystemDependentDataManager() { return mrSystemDependentDataManager; }

        // number of cycles to hold data
        sal_uInt32 getHoldCycles() const { return mnHoldCycles; }
    };
} // end of namespace basegfx

namespace basegfx
{
    class BASEGFX_DLLPUBLIC SystemDependentDataHolder
    {
    private:
        // Possibility to hold System-Dependent B2DPolygon-Representations
        std::map< size_t, SystemDependentData_WeakPtr > maSystemDependentReferences;

        // noncopyable
        SystemDependentDataHolder(const SystemDependentDataHolder&) = delete;
        SystemDependentDataHolder& operator=(const SystemDependentDataHolder&) = delete;

    public:
        SystemDependentDataHolder();
        ~SystemDependentDataHolder();

        bool addOrReplaceSystemDependentData(SystemDependentData_SharedPtr& rData);
        SystemDependentData_SharedPtr getSystemDependentData(size_t hash_code) const;
    };
} // end of namespace basegfx

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
