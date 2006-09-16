/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: IconCache.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 19:21:41 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "tools/IconCache.hxx"

#include "sdresid.hxx"
#include <hash_map>
#ifndef INCLUDED_OSL_DOUBLECHECKEDLOCKING_H
#include <osl/doublecheckedlocking.h>
#endif
#ifndef INCLUDED_OSL_GETGLOBALMUTEX_HXX
#include <osl/getglobalmutex.hxx>
#endif

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

    typedef ::std::hash_map<USHORT,Image> ImageContainer;
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
        OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();

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
