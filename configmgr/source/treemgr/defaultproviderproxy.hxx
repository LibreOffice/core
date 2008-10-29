/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: defaultproviderproxy.hxx,v $
 * $Revision: 1.9 $
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

#ifndef CONFIGMGR_DEFAULTPROVIDER_PROXY_HXX_
#define CONFIGMGR_DEFAULTPROVIDER_PROXY_HXX_

#include "configpath.hxx"
#include "utility.hxx"
#include "requestoptions.hxx"
#include <rtl/ref.hxx>
#include <salhelper/simplereferenceobject.hxx>

#ifndef INCLUDED_MEMORY
#include <memory>
#define INCLUDED_MEMORY
#endif

namespace configmgr
{
//-----------------------------------------------------------------------------
    class ISubtree;
    class IDefaultableTreeManager;
    class OOptions;
    class TreeManager;
//-----------------------------------------------------------------------------
    namespace configuration
    {
//-----------------------------------------------------------------------------

        /// provides access to the defaults for a given request
        class DefaultProviderProxy
        : public salhelper::SimpleReferenceObject
        {
            // the data defining a request
            AbsolutePath            m_aBaseLocation;
            RequestOptions          m_aOptions;

            // the object(s) that provide the defaults
            rtl::Reference< TreeManager >    m_xDefaultTreeProvider;
            IDefaultableTreeManager *                   m_pDefaultTreeManager;
        public:
            explicit
            DefaultProviderProxy(
                    rtl::Reference< TreeManager > const & _xDefaultTreeProvider,
                    IDefaultableTreeManager *   _pDefaultTreeManager,
                    AbsolutePath        const&  _aBaseLocation,
                    RequestOptions      const&  _aOptions
                );

            ~DefaultProviderProxy();

        /// tries to load a default instance of the specified node (which must be within the request range owned)
            std::auto_ptr<ISubtree> getDefaultTree(AbsolutePath const& _aLocation) const SAL_THROW((com::sun::star::uno::Exception));

            /// tries to load default data into the owned tree - call only outside of any locks
            bool fetchDefaultData() SAL_THROW((com::sun::star::uno::Exception));
        };
//-----------------------------------------------------------------------------
    }
}

#endif // CONFIGMGR_DEFAULTPROVIDER_PROXY_HXX_
