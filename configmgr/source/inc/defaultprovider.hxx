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

#include "valuenode.hxx"
#include "utility.hxx"
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>

#ifndef INCLUDED_MEMORY
#include <memory>
#define INCLUDED_MEMORY
#endif


namespace configmgr
{

    namespace uno = com::sun::star::uno;
    using ::rtl::OUString;

////////////////////////////////////////////////////////////////////////////////
    namespace configuration
    {
        class AbsolutePath;
    }
    //-------------------------
    class ISubtree;
    class RequestOptions;
    //==========================================================================
    //= IDefaultProvider
    //==========================================================================
    /* is an interface that can be implemented by an <type>ITreeProvider</type>
       or <type>ITreeManager</type>.
       <p>Supports functionality to fetch only the default data corresponding to a tree</p>
    */
    class SAL_NO_VTABLE IDefaultProvider
    {
    public:
        /** load the default version of the tree named by a path
            using certain options and requiring a specific loading depth
            @returns
                the default data tree, yielding ownership of it
                <NULL/>if no default data is available for the tree
        */
        virtual std::auto_ptr<ISubtree> requestDefaultData( configuration::AbsolutePath const& aSubtreePath,
                                                            const RequestOptions& _aOptions
                                                           ) CFG_UNO_THROW_ALL(  ) = 0;
    };

    //==========================================================================
    /// a refcounted <type>IDefaultProvider</type>.
    class SAL_NO_VTABLE IConfigDefaultProvider  : public Refcounted
                                                , public IDefaultProvider
    {
    };
    //==========================================================================
    //= IDefaultableTreeManager
    //==========================================================================
    /* is a supplementary interface for a <type>ITreeManager</type>.
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
                                          ) CFG_UNO_THROW_ALL(  ) = 0;

    };

////////////////////////////////////////////////////////////////////////////////

} // namespace configmgr

#endif

