/*************************************************************************
 *
 *  $RCSfile: defaultproviderproxy.cxx,v $
 *
 *  $Revision: 1.5 $
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

#include "defaultproviderproxy.hxx"

#ifndef CONFIGMGR_DEFAULTPROVIDER_HXX
#include "defaultprovider.hxx"
#endif
#ifndef CONFIGMGR_TREEPROVIDER_HXX
#include "treeprovider.hxx"
#endif
#ifndef _CONFIGMGR_TREE_VALUENODE_HXX
#include "valuenode.hxx"
#endif
#ifndef CONFIGMGR_MISC_OPTIONS_HXX_
#include "options.hxx"
#endif
#ifndef CONFIGMGR_UPDATEACCESSOR_HXX
#include "updateaccessor.hxx"
#endif

namespace configmgr
{
//-----------------------------------------------------------------------------
    namespace configuration
    {
//-----------------------------------------------------------------------------

DefaultProviderProxy::DefaultProviderProxy(
        rtl::Reference< IConfigDefaultProvider > const & _xDefaultTreeProvider,
        IDefaultableTreeManager *   _pDefaultTreeManager,
        AbsolutePath        const&  _aBaseLocation,
        RequestOptions      const&  _aOptions
    )
: m_aBaseLocation(_aBaseLocation)
, m_aOptions(_aOptions)
, m_xDefaultTreeProvider(_xDefaultTreeProvider)
, m_pDefaultTreeManager(_pDefaultTreeManager)
{
}
//-----------------------------------------------------------------------------

DefaultProviderProxy::~DefaultProviderProxy()
{
}
//-----------------------------------------------------------------------------

/// tries to load a default instance of the specified node (which must be within the request range owned)
std::auto_ptr<ISubtree> DefaultProviderProxy::getDefaultTree(
                            memory::UpdateAccessor& _aDestinationSpace,
                            AbsolutePath const& _aLocation
                       ) const CFG_UNO_THROW_ALL()
{
    OSL_ENSURE( Path::hasPrefix(_aLocation,m_aBaseLocation),
                "ERROR: DefaultProviderProxy called for out-of-scope location" );

    std::auto_ptr<ISubtree> aRet;

    if (m_xDefaultTreeProvider.is())
        aRet = m_xDefaultTreeProvider->requestDefaultData(_aLocation, m_aOptions);

    return aRet;
}

//-----------------------------------------------------------------------------
/// tries to load default data into the specified tree
bool DefaultProviderProxy::fetchDefaultData()  CFG_UNO_THROW_ALL()
{
    OSL_PRECOND(m_pDefaultTreeManager, "No tree to fetch defaults into");
    if (!m_pDefaultTreeManager) return false;

    memory::UpdateAccessor anAccessToken(m_pDefaultTreeManager->getDataSegment(m_aBaseLocation,m_aOptions));

    return !! m_pDefaultTreeManager->fetchDefaultData(anAccessToken,m_aBaseLocation,m_aOptions);
}
//-----------------------------------------------------------------------------
    }
}

