/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "tools/IconCache.hxx"

#include "sdresid.hxx"
#include <hash_map>
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

    typedef ::std::hash_map<sal_uInt16,Image> ImageContainer;
    ImageContainer maContainer;

    Image GetIcon (sal_uInt16 nResourceId);
};

IconCache* IconCache::Implementation::mpInstance = NULL;



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




Image IconCache::GetIcon (sal_uInt16 nResourceId)
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
