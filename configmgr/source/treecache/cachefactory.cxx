/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cachefactory.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 14:36:15 $
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
#include "precompiled_configmgr.hxx"

#include "cachefactory.hxx"

#ifndef CONFIGMGR_TREEMANAGER_HXX_
#include "treemanager.hxx"
#endif
#ifndef CONFIGMGR_BACKEND_CACHECONTROLLER_HXX
#include "cachecontroller.hxx"
#endif
#ifndef CONFIGMGR_BACKENDFACTORY_HXX_
#include "backendfactory.hxx"
#endif

namespace configmgr
{
// -------------------------------------------------------------------------

    typedef rtl::Reference< backend::IMergedDataProvider > MergedBackendRef;
// -------------------------------------------------------------------------

    static
    rtl::Reference<TreeManager> buildCacheManager(MergedBackendRef const & _xBackend,
                                                  CacheFactory::CreationContext const & _xContext)
    {
        rtl::Reference< TreeManager > xCache;

        if (_xBackend.is())
        {
            rtl::Reference< backend::ICachedDataProvider > xLoader
                = new backend::CacheController(_xBackend.get(), _xContext);

            xCache.set( new TreeManager(xLoader.get()) );
        }

        return xCache;
    }
// -------------------------------------------------------------------------

    rtl::Reference<TreeManager>
        CacheFactory::createCacheManager(CreationContext const & _xContext)
    {
        MergedBackendRef xBackend = backend::BackendFactory::instance(_xContext).createBackend();

        return buildCacheManager(xBackend, _xContext);
    }

// -------------------------------------------------------------------------

    CacheFactory & CacheFactory::instance()
    {
        static CacheFactory aStaticFactory;
        return aStaticFactory;
    }

//-----------------------------------------------------------------------------
} // namespace
