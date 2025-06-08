/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <vcl/dllapi.h>
#include <rtl/ustring.hxx>
#include <version>
#if defined __cpp_lib_memory_resource
#include <memory_resource>
#endif

namespace rtl
{
class OStringBuffer;
}

class VCL_DLLPUBLIC CacheOwner
{
protected:
    CacheOwner();
    virtual ~CacheOwner();

public:
    // returns true if cache no longer uses GetMemoryResource
    virtual bool dropCaches() = 0;
    virtual void dumpState(rtl::OStringBuffer& rState) = 0;
    virtual OUString getCacheName() const = 0;
#if defined __cpp_lib_memory_resource
    static std::pmr::memory_resource& GetMemoryResource();
#endif
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
