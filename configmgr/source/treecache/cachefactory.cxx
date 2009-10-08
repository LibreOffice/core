/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: cachefactory.cxx,v $
 * $Revision: 1.10 $
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
#include "precompiled_configmgr.hxx"

#include "cachefactory.hxx"
#include "treemanager.hxx"
#include "cachecontroller.hxx"
#include "backendfactory.hxx"

namespace configmgr
{
// -------------------------------------------------------------------------

    static
    rtl::Reference<TreeManager> buildCacheManager(rtl::Reference< backend::IMergedDataProvider > const & _xBackend,
                                                  ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & _xContext)
    {
        rtl::Reference< TreeManager > xCache;

        if (_xBackend.is())
        {
            rtl::Reference< backend::CacheController > xLoader
                = new backend::CacheController(_xBackend.get(), _xContext);

            xCache.set( new TreeManager(xLoader.get()) );
        }

        return xCache;
    }
// -------------------------------------------------------------------------

    rtl::Reference<TreeManager>
        CacheFactory::createCacheManager(::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & _xContext)
    {
        rtl::Reference< backend::IMergedDataProvider > xBackend = backend::BackendFactory::instance(_xContext).createBackend();

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
