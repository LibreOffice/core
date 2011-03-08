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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "tools/IconCache.hxx"

#include "sdresid.hxx"
#include <boost/unordered_map.hpp>
#include <osl/doublecheckedlocking.h>
#include <osl/getglobalmutex.hxx>

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

    typedef ::boost::unordered_map<USHORT,Image> ImageContainer;
    ImageContainer maContainer;

    Image GetIcon (USHORT nResourceId);
};

IconCache* IconCache::Implementation::mpInstance = NULL;



Image IconCache::Implementation::GetIcon (USHORT nResourceId)
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
IconCache& IconCache::Instance (void)
{
    if (Implementation::mpInstance == NULL)
    {
        ::osl::GetGlobalMutex aMutexFunctor;
        ::osl::MutexGuard aGuard (aMutexFunctor());
        if (Implementation::mpInstance == NULL)
        {
            IconCache* pCache = new IconCache ();
            SdGlobalResourceContainer::Instance().AddResource (
                ::std::auto_ptr<SdGlobalResource>(pCache));
            OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
            Implementation::mpInstance = pCache;
        }
    }
    else
    {
        OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
    }

    DBG_ASSERT(Implementation::mpInstance!=NULL,
        "IconCache::Instance(): instance is NULL");
    return *Implementation::mpInstance;
}




Image IconCache::GetIcon (USHORT nResourceId)
{
    return mpImpl->GetIcon (nResourceId);
}




IconCache::IconCache (void)
    : mpImpl (new Implementation())
{
}




IconCache::~IconCache (void)
{
    // empty
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
