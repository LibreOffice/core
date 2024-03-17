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
#include <rtl/strbuf.hxx>
#include <vcl/timer.hxx>

#include <memory>
#include <mutex>
#include <chrono>
#include <o3tl/sorted_vector.hxx>

namespace vcl::graphic
{
class VCL_DLLPUBLIC MemoryManaged
{
private:
    sal_Int64 mnCurrentSizeBytes = 0;
    bool mbIsRegistered = false;

protected:
    // Copy
    MemoryManaged(MemoryManaged const& rMemoryManaged)
        : mbIsRegistered(rMemoryManaged.mbIsRegistered)
    {
        // if the original is registered we need to register as well
        if (mbIsRegistered)
            registerIntoManager();
    }

    // Move
    MemoryManaged(MemoryManaged&& rMemoryManaged)
        : mbIsRegistered(rMemoryManaged.mbIsRegistered)
    {
        // if the original is registered we need to register as well
        if (mbIsRegistered)
            registerIntoManager();
    }

    MemoryManaged(bool bRegister)
    {
        if (bRegister)
            registerIntoManager();
    }

    virtual ~MemoryManaged() { unregisterFromManager(); }

    void updateCurrentSizeInBytes(sal_Int64 nNewSize)
    {
        if (mbIsRegistered)
        {
            auto& rManager = vcl::graphic::MemoryManager::get();
            rManager.changeExisting(this, nNewSize);
        }
        else
        {
            mnCurrentSizeBytes = nNewSize;
        }
    }

    bool isRegistered() const { return mbIsRegistered; }

    void changeExisting(sal_Int64 nNewSize)
    {
        if (mbIsRegistered)
        {
            auto& rManager = vcl::graphic::MemoryManager::get();
            rManager.changeExisting(this, nNewSize);
        }
        else
        {
            mnCurrentSizeBytes = nNewSize;
        }
    }

    void swappedIn(sal_Int64 nNewSize)
    {
        if (mbIsRegistered)
        {
            auto& rManager = vcl::graphic::MemoryManager::get();
            rManager.swappedIn(this, nNewSize);
        }
        else
        {
            mnCurrentSizeBytes = nNewSize;
        }
    }

    void swappedOut(sal_Int64 nNewSize)
    {
        if (mbIsRegistered)
        {
            auto& rManager = vcl::graphic::MemoryManager::get();
            rManager.swappedOut(this, nNewSize);
        }
        else
        {
            mnCurrentSizeBytes = nNewSize;
        }
    }

public:
    sal_Int64 getCurrentSizeInBytes() const { return mnCurrentSizeBytes; }

    void setCurrentSizeInBytes(sal_Int64 nSize) { mnCurrentSizeBytes = nSize; }

protected:
    void registerIntoManager()
    {
        if (!mbIsRegistered)
        {
            auto& rManager = vcl::graphic::MemoryManager::get();
            rManager.registerObject(this);
            mbIsRegistered = true;
        }
    }

    void unregisterFromManager()
    {
        if (mbIsRegistered)
        {
            auto& rManager = vcl::graphic::MemoryManager::get();
            rManager.unregisterObject(this);
            mbIsRegistered = false;
        }
    }

public:
    virtual bool canReduceMemory() const = 0;
    virtual bool reduceMemory() = 0;
    virtual std::chrono::high_resolution_clock::time_point getLastUsed() const = 0;
    virtual void dumpState(rtl::OStringBuffer& rState) = 0;
};

} // end namespace vcl::graphic

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
