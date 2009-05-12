/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: defaultprovider.hxx,v $
 * $Revision: 1.8 $
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

#ifndef CONFIGMGR_DEFAULTPROVIDER_HXX
#define CONFIGMGR_DEFAULTPROVIDER_HXX

#include "sal/config.h"

#include "com/sun/star/uno/Exception.hpp"
#include "sal/types.h"

#include "utility.hxx"

namespace configmgr
{
    namespace configuration
    {
        class AbsolutePath;
    }
    //-------------------------
    class RequestOptions;
    //==========================================================================
    //= IDefaultableTreeManager
    //==========================================================================
    /* is a supplementary interface for an <type>OProviderImpl</type>.
       <p>Supports functionality to load default data into the managed tree</p>
    */
    class SAL_NO_VTABLE IDefaultableTreeManager
    {
    public:
        /** attempt to load default data into the tree named by a path using certain options
            and requiring a specific loading depth.

            @returns
                <TRUE/>,  if some default data is available within the tree
                <FALSE/>, if no default data is available for the tree
        */
        virtual sal_Bool fetchDefaultData(configuration::AbsolutePath const& aSubtreePath,
                                          const RequestOptions& _xOptions
                                          ) SAL_THROW((com::sun::star::uno::Exception)) = 0;

    };

////////////////////////////////////////////////////////////////////////////////

} // namespace configmgr

#endif

