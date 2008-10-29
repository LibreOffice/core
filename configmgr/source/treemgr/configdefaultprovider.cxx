/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: configdefaultprovider.cxx,v $
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

#include "configdefaultprovider.hxx"
#include "defaultproviderproxy.hxx"
#include "noderef.hxx"
#include "valuenode.hxx"
#include "tree.hxx"
#include "options.hxx"

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

DefaultProvider DefaultProvider::create(rtl::Reference< Tree > const& _aRootTree, RequestOptions const& _aOptions,
                                          rtl::Reference< TreeManager > const &  _xDefaultProvider,
                                          IDefaultableTreeManager* _pDefaultableTree)
{
    OSL_PRECOND( !isEmpty(_aRootTree.get()), "ERROR: Cannot create DefaultProvider for NULL tree");

    rtl::Reference< DefaultProviderProxy > xNewProxy;

    if (!isEmpty(_aRootTree.get()))
    {
        xNewProxy = new DefaultProviderProxy(_xDefaultProvider,_pDefaultableTree,
                                             _aRootTree->getRootPath(), _aOptions );
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
            rtl::Reference< Tree > const& _aTree, NodeRef const& _aNode
     ) const SAL_THROW((com::sun::star::uno::Exception))
{
    std::auto_ptr<ISubtree> aRet;

    node::Attributes aAttributes = _aTree->getAttributes(_aNode);

//    if (aAttributes.bDefaulted)
//        clone the ISubtree (no interface for that) :-(

    if (m_aProxy.is() && aAttributes.existsInDefault())
        aRet = m_aProxy->getDefaultTree(_aTree->getAbsolutePath(_aNode));

    return aRet;
}

//-----------------------------------------------------------------------------
/// tries to load default data into the specified tree
static bool shouldFetchDefaultData(rtl::Reference< Tree > const& _aTreeRef, bool & _rbHasDefaults)
{
    bool bShouldFetch = false;

    node::Attributes aAttributes = _aTreeRef->getAttributes(_aTreeRef->getRootNode());

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
bool DefaultProvider::fetchDefaultData(rtl::Reference< Tree > const& _aTreeRef) const SAL_THROW((com::sun::star::uno::Exception))
{
    bool bHasDefaults = false;

    if (shouldFetchDefaultData(_aTreeRef,bHasDefaults) && m_aProxy.is() )
        bHasDefaults = m_aProxy->fetchDefaultData();

    return bHasDefaults;
}

//-----------------------------------------------------------------------------
    }
}

