/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: configdefaultprovider.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 14:39:52 $
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

#include "configdefaultprovider.hxx"

#ifndef CONFIGMGR_DEFAULTPROVIDER_PROXY_HXX_
#include "defaultproviderproxy.hxx"
#endif

#ifndef CONFIGMGR_CONFIGNODE_HXX_
#include "noderef.hxx"
#endif
#ifndef _CONFIGMGR_TREE_VALUENODE_HXX
#include "valuenode.hxx"
#endif
#ifndef CONFIGMGR_CONFIGNODEIMPL_HXX_
#include "treeimpl.hxx"
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
// class DefaultProvider
//-----------------------------------------------------------------------------

// standard c/d-tors to make compiler barrier
DefaultProvider DefaultProvider::createEmpty()
{
    return DefaultProvider(NULL);
}
//-----------------------------------------------------------------------------

DefaultProvider DefaultProvider::create(Tree const& _aRootTree, RequestOptions const& _aOptions,
                                          rtl::Reference< IConfigDefaultProvider > const &  _xDefaultProvider,
                                          IDefaultableTreeManager* _pDefaultableTree)
{
    OSL_PRECOND( !_aRootTree.isEmpty(), "ERROR: Cannot create DefaultProvider for NULL tree");

    rtl::Reference< DefaultProviderProxy > xNewProxy;

    if (!_aRootTree.isEmpty())
    {
        xNewProxy = new DefaultProviderProxy(_xDefaultProvider,_pDefaultableTree,
                                             _aRootTree.getRootPath(), _aOptions );
    }

    return DefaultProvider( xNewProxy );
}
//-----------------------------------------------------------------------------

DefaultProvider::DefaultProvider(DefaultProvider const& _aOther)
: m_aProxy(_aOther.m_aProxy)
{
}
//-----------------------------------------------------------------------------

DefaultProvider& DefaultProvider::operator=(DefaultProvider const& _aOther)
{
    m_aProxy = _aOther.m_aProxy;
    return *this;
}
//-----------------------------------------------------------------------------

DefaultProvider::~DefaultProvider()
{
}
//-----------------------------------------------------------------------------

DefaultProvider::DefaultProvider(rtl::Reference< DefaultProviderProxy > const& _xProviderProxy)
: m_aProxy(_xProviderProxy)
{
}
//-----------------------------------------------------------------------------

/// tries to load a default instance of the specified node
std::auto_ptr<ISubtree> DefaultProvider::getDefaultTree(
            Tree const& _aTree, NodeRef const& _aNode
     ) const CFG_UNO_THROW_ALL()
{
    std::auto_ptr<ISubtree> aRet;

    node::Attributes aAttributes = _aTree.getAttributes(_aNode);

//    if (aAttributes.bDefaulted)
//        clone the ISubtree (no interface for that) :-(

    if (m_aProxy.is() && aAttributes.existsInDefault())
        aRet = m_aProxy->getDefaultTree(_aTree.getAbsolutePath(_aNode));

    return aRet;
}

//-----------------------------------------------------------------------------
/// tries to load default data into the specified tree
static bool shouldFetchDefaultData(TreeRef const& _aTreeRef, bool & _rbHasDefaults)
{
    bool bShouldFetch = false;

    Tree aTempTree(_aTreeRef);

    node::Attributes aAttributes = aTempTree.getAttributes(aTempTree.getRootNode());

    if (aAttributes.isDefault())
        _rbHasDefaults = true;

    // in replaced/added parts, defaults are considered non-existing
    else if (!aAttributes.isReplacedForUser())
        _rbHasDefaults = false;

    else
        bShouldFetch = true;

    return bShouldFetch;
}

//-----------------------------------------------------------------------------
/// tries to load default data into the specified tree
bool DefaultProvider::fetchDefaultData(TreeRef const& _aTreeRef) const CFG_UNO_THROW_ALL()
{
    bool bHasDefaults = false;

    if (shouldFetchDefaultData(_aTreeRef,bHasDefaults) && m_aProxy.is() )
        bHasDefaults = m_aProxy->fetchDefaultData();

    return bHasDefaults;
}

//-----------------------------------------------------------------------------
    }
}

