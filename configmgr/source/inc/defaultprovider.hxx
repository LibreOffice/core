/*************************************************************************
 *
 *  $RCSfile: defaultprovider.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 16:18:58 $
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
    namespace memory
    {
        class Segment;
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
        /// get a data segment to host the given location - also available in ITreeManager
        virtual memory::Segment* getDataSegment(configuration::AbsolutePath const& _rAccessor,
                                                const RequestOptions& _aOptions) = 0;

        /** attempt to load default data into the tree named by a path using certain options
            and requiring a specific loading depth.

            @returns
                <TRUE/>,  if some default data is available within the tree
                <FALSE/>, if no default data is available for the tree
        */
        virtual sal_Bool fetchDefaultData(  memory::UpdateAccessor& _aAccessToken,
                                            configuration::AbsolutePath const& aSubtreePath,
                                            const RequestOptions& _xOptions
                                         ) CFG_UNO_THROW_ALL(  ) = 0;

    };

////////////////////////////////////////////////////////////////////////////////

} // namespace configmgr

#endif

