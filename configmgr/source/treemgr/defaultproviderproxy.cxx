/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: defaultproviderproxy.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 14:40:55 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_configmgr.hxx"

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

    return !! m_pDefaultTreeManager->fetchDefaultData(m_aBaseLocation,m_aOptions);
}
//-----------------------------------------------------------------------------
    }
}

