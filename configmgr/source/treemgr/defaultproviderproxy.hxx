/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: defaultproviderproxy.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 14:41:12 $
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

#ifndef CONFIGMGR_DEFAULTPROVIDER_PROXY_HXX_
#define CONFIGMGR_DEFAULTPROVIDER_PROXY_HXX_

#ifndef CONFIGMGR_CONFIGPATH_HXX_
#include "configpath.hxx"
#endif
#ifndef CONFIGMGR_UTILITY_HXX_
#include "utility.hxx"
#endif
#ifndef CONFIGMGR_MISC_REQUESTOPTIONS_HXX_
#include "requestoptions.hxx"
#endif
#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif

#ifndef INCLUDED_MEMORY
#include <memory>
#define INCLUDED_MEMORY
#endif

namespace configmgr
{
//-----------------------------------------------------------------------------
    class ISubtree;
    class IConfigDefaultProvider;
    class IDefaultableTreeManager;
    class OOptions;
//-----------------------------------------------------------------------------
    namespace configuration
    {
//-----------------------------------------------------------------------------

        /// provides access to the defaults for a given request
        class DefaultProviderProxy
        : public configmgr::SimpleReferenceObject
        {
            // the data defining a request
            AbsolutePath            m_aBaseLocation;
            RequestOptions          m_aOptions;

            // the object(s) that provide the defaults
            rtl::Reference< IConfigDefaultProvider >    m_xDefaultTreeProvider;
            IDefaultableTreeManager *                   m_pDefaultTreeManager;
        public:
            explicit
            DefaultProviderProxy(
                    rtl::Reference< IConfigDefaultProvider > const & _xDefaultTreeProvider,
                    IDefaultableTreeManager *   _pDefaultTreeManager,
                    AbsolutePath        const&  _aBaseLocation,
                    RequestOptions      const&  _aOptions
                );

            ~DefaultProviderProxy();

        /// tries to load a default instance of the specified node (which must be within the request range owned)
            std::auto_ptr<ISubtree> getDefaultTree(AbsolutePath const& _aLocation) const CFG_UNO_THROW_ALL();

            /// tries to load default data into the owned tree - call only outside of any locks
            bool fetchDefaultData() CFG_UNO_THROW_ALL();
        };
//-----------------------------------------------------------------------------
    }
}

#endif // CONFIGMGR_DEFAULTPROVIDER_PROXY_HXX_
