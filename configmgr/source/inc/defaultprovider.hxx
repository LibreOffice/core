/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: defaultprovider.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 14:18:20 $
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

#ifndef CONFIGMGR_DEFAULTPROVIDER_HXX
#define CONFIGMGR_DEFAULTPROVIDER_HXX

#ifndef _CONFIGMGR_TREE_VALUENODE_HXX
#include "valuenode.hxx"
#endif
#ifndef CONFIGMGR_UTILITY_HXX_
#include "utility.hxx"
#endif

#ifndef _COM_SUN_STAR_UNO_EXCEPTION_HPP_
#include <com/sun/star/uno/Exception.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_RUNTIMEEXCEPTION_HPP_
#include <com/sun/star/uno/RuntimeException.hpp>
#endif

#ifndef INCLUDED_MEMORY
#include <memory>
#define INCLUDED_MEMORY
#endif


namespace configmgr
{

    namespace uno = com::sun::star::uno;
    using ::rtl::OUString;

////////////////////////////////////////////////////////////////////////////////
    namespace configuration
    {
        class AbsolutePath;
    }
    //-------------------------
    class ISubtree;
    class RequestOptions;
    //==========================================================================
    //= IDefaultProvider
    //==========================================================================
    /* is an interface that can be implemented by an <type>ITreeProvider</type>
       or <type>ITreeManager</type>.
       <p>Supports functionality to fetch only the default data corresponding to a tree</p>
    */
    class SAL_NO_VTABLE IDefaultProvider
    {
    public:
        /** load the default version of the tree named by a path
            using certain options and requiring a specific loading depth
            @returns
                the default data tree, yielding ownership of it
                <NULL/>if no default data is available for the tree
        */
        virtual std::auto_ptr<ISubtree> requestDefaultData( configuration::AbsolutePath const& aSubtreePath,
                                                            const RequestOptions& _aOptions
                                                           ) CFG_UNO_THROW_ALL(  ) = 0;
    };

    //==========================================================================
    /// a refcounted <type>IDefaultProvider</type>.
    class SAL_NO_VTABLE IConfigDefaultProvider  : public Refcounted
                                                , public IDefaultProvider
    {
    };
    //==========================================================================
    //= IDefaultableTreeManager
    //==========================================================================
    /* is a supplementary interface for a <type>ITreeManager</type>.
       <p>Supports functionality to load default data into the managed tree</p>
    */
    class SAL_NO_VTABLE IDefaultableTreeManager
    {
    public:
        /** attempt to load default data into the tree named by a path using certain options
            and requiring a specific loading depth.

            @returns
                <TRUE/>,  if some default data is available within the tree
                <FALSE/>, if no default data is available for the tree
        */
        virtual sal_Bool fetchDefaultData(configuration::AbsolutePath const& aSubtreePath,
                                          const RequestOptions& _xOptions
                                          ) CFG_UNO_THROW_ALL(  ) = 0;

    };

////////////////////////////////////////////////////////////////////////////////

} // namespace configmgr

#endif

