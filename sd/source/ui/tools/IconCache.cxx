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

#include "tools/IconCache.hxx"

#include "sdresid.hxx"
#include <osl/doublecheckedlocking.h>
#include <osl/getglobalmutex.hxx>
#include <unordered_map>

namespace sd {

//===== IconCache::Implementation =============================================

class IconCache::Implementation
{
private:
    friend class IconCache;

    /** This pointer holds a valid reference from first time that
        IconCache::Instance() is called to the end of the sd module when the
        cache is destroyed from SdGlobalResourceContainer.
    */
    static IconCache* mpInstance;

    typedef std::unordered_map<sal_uInt16,Image> ImageContainer;
    ImageContainer maContainer;

    Image GetIcon (sal_uInt16 nResourceId);
};

IconCache* IconCache::Implementation::mpInstance = nullptr;

Image IconCache::Implementation::GetIcon (sal_uInt16 nResourceId)
{
    Image aResult;
    ImageContainer::iterator iImage;
    iImage = maContainer.find (nResourceId);
    if (iImage == maContainer.end())
    {
        aResult = Image(BitmapEx(SdResId(nResourceId)));
        maContainer[nResourceId] = aResult;
    }
    else
        aResult = iImage->second;
    return aResult;
}

//===== IconCache =============================================================

//static
IconCache& IconCache::Instance()
{
    if (Implementation::mpInstance == nullptr)
    {
        ::osl::GetGlobalMutex aMutexFunctor;
        ::osl::MutexGuard aGuard (aMutexFunctor());
        if (Implementation::mpInstance == nullptr)
        {
            IconCache* pCache = new IconCache ();
            SdGlobalResourceContainer::Instance().AddResource (
                ::std::unique_ptr<SdGlobalResource>(pCache));
            OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
            Implementation::mpInstance = pCache;
        }
    }
    else
    {
        OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
    }

    DBG_ASSERT(Implementation::mpInstance!=nullptr,
        "IconCache::Instance(): instance is NULL");
    return *Implementation::mpInstance;
}

Image IconCache::GetIcon (sal_uInt16 nResourceId)
{
    return mpImpl->GetIcon (nResourceId);
}

IconCache::IconCache()
    : mpImpl (new Implementation())
{
}

IconCache::~IconCache()
{
    // empty
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
