/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: configdefaultprovider.hxx,v $
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

#ifndef CONFIGMGR_CONFIG_DEFAULTPROVIDER_HXX_
#define CONFIGMGR_CONFIG_DEFAULTPROVIDER_HXX_

#include "utility.hxx"
#include <com/sun/star/uno/Exception.hpp>
#include <rtl/ref.hxx>

#ifndef INCLUDED_MEMORY
#include <memory>
#define INCLUDED_MEMORY
#endif

namespace configmgr
{
//-----------------------------------------------------------------------------
    class ISubtree;
    class IDefaultableTreeManager;
    class RequestOptions;
    class TreeManager;

    namespace uno = com::sun::star::uno;
//-----------------------------------------------------------------------------
    namespace configuration
    {
//-----------------------------------------------------------------------------

        class Tree;
        class NodeRef;
//-----------------------------------------------------------------------------

        /// provides access to the default for a given request
        class DefaultProviderProxy;

        class DefaultProvider
        {
            rtl::Reference< DefaultProviderProxy > m_aProxy;
        public:
            // factory methods
            static DefaultProvider createEmpty();
            static DefaultProvider create(rtl::Reference< Tree > const& _aRootTree,
                                          RequestOptions const& _xOptions,
                                          rtl::Reference< TreeManager > const & _xDefaultProvider,
                                          IDefaultableTreeManager* _pDefaultableTree);

            // actual c'tor
            explicit
            DefaultProvider(rtl::Reference< DefaultProviderProxy > const& _xProviderProxy);

        // standard c/d-tors to make compiler barrier
            DefaultProvider(DefaultProvider const& _aOther);
            DefaultProvider& operator=(DefaultProvider const& _aOther);
            ~DefaultProvider();

            bool isValid() const { return !! m_aProxy.is(); }

        /// tries to load a default instance of the specified node
            std::auto_ptr<ISubtree> getDefaultTree(rtl::Reference< Tree > const& _aTree, NodeRef const& _aNode) const SAL_THROW((com::sun::star::uno::Exception));

        /// tries to load default data into the specified tree
            bool fetchDefaultData(rtl::Reference< Tree > const& _aTreeRef) const SAL_THROW((com::sun::star::uno::Exception));
        };

//-----------------------------------------------------------------------------

    }
}

#endif // CONFIGMGR_CONFIG_DEFAULTPROVIDER_HXX_
