/*************************************************************************
 *
 *  $RCSfile: defaultproviderproxy.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 16:19:46 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

#ifndef _SALHELPER_SIMPLEREFERENCEOBJECT_HXX_
#include <salhelper/simplereferenceobject.hxx>
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
    namespace memory
    {
        class UpdateAccessor;
    }
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
            std::auto_ptr<ISubtree> getDefaultTree(memory::UpdateAccessor& _aDestinationSpace, AbsolutePath const& _aLocation) const CFG_UNO_THROW_ALL();

            /// tries to load default data into the owned tree - call only outside of any locks
            bool fetchDefaultData() CFG_UNO_THROW_ALL();
        };
//-----------------------------------------------------------------------------
    }
}

#endif // CONFIGMGR_DEFAULTPROVIDER_PROXY_HXX_
