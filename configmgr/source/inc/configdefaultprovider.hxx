/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: configdefaultprovider.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 14:17:12 $
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

#ifndef CONFIGMGR_CONFIG_DEFAULTPROVIDER_HXX_
#define CONFIGMGR_CONFIG_DEFAULTPROVIDER_HXX_

#ifndef CONFIGMGR_UTILITY_HXX_
#include "utility.hxx"
#endif

#ifndef _COM_SUN_STAR_UNO_EXCEPTION_HPP_
#include <com/sun/star/uno/Exception.hpp>
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
    class RequestOptions;

    namespace uno = com::sun::star::uno;
//-----------------------------------------------------------------------------
    namespace configuration
    {
//-----------------------------------------------------------------------------

        class Tree;
        class TreeRef;
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
            static DefaultProvider create(Tree const& _aRootTree,
                                          RequestOptions const& _xOptions,
                                          rtl::Reference< IConfigDefaultProvider > const & _xDefaultProvider,
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
            std::auto_ptr<ISubtree> getDefaultTree(Tree const& _aTree, NodeRef const& _aNode) const CFG_UNO_THROW_ALL();

        /// tries to load default data into the specified tree
            bool fetchDefaultData(TreeRef const& _aTreeRef) const CFG_UNO_THROW_ALL();
        };

//-----------------------------------------------------------------------------

    }
}

#endif // CONFIGMGR_CONFIG_DEFAULTPROVIDER_HXX_
