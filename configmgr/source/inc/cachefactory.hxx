/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: cachefactory.hxx,v $
 * $Revision: 1.7 $
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

#ifndef CONFIGMGR_CACHEFACTORY_HXX_
#define CONFIGMGR_CACHEFACTORY_HXX_

#include <rtl/ref.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>

namespace configmgr
{
//-----------------------------------------------------------------------------
    class TreeManager;
//-----------------------------------------------------------------------------
    struct CacheFactory
    {
        rtl::Reference<TreeManager>
            createCacheManager(::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & _xContext);

        static CacheFactory & instance();
    };
//-----------------------------------------------------------------------------
}


#endif // CONFIGMGR_CACHEFACTORY_HXX_

