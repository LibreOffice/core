/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: defaultproviderproxy.cxx,v $
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

#include "defaultproviderproxy.hxx"
#include "defaultprovider.hxx"
#include "treemanager.hxx"
#include "valuenode.hxx"
#include "options.hxx"

namespace configmgr
{
//-----------------------------------------------------------------------------
    namespace configuration
    {
//-----------------------------------------------------------------------------

DefaultProviderProxy::DefaultProviderProxy(
        rtl::Reference< TreeManager > const & _xDefaultTreeProvider,
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
                            AbsolutePath const& _aLocation
                       ) const SAL_THROW((com::sun::star::uno::Exception))
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
bool DefaultProviderProxy::fetchDefaultData()  SAL_THROW((com::sun::star::uno::Exception))
{
    OSL_PRECOND(m_pDefaultTreeManager, "No tree to fetch defaults into");
    if (!m_pDefaultTreeManager) return false;

    return !! m_pDefaultTreeManager->fetchDefaultData(m_aBaseLocation,m_aOptions);
}
//-----------------------------------------------------------------------------
    }
}

