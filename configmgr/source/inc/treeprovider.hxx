/*************************************************************************
 *
 *  $RCSfile: treeprovider.hxx,v $
 *
 *  $Revision: 1.19 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 16:19:09 $
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

/* PLEASE DON'T DELETE ANY COMMENT LINES, ALSO IT'S UNNECESSARY. */


#ifndef CONFIGMGR_TREEPROVIDER_HXX
#define CONFIGMGR_TREEPROVIDER_HXX

#ifndef _CONFIGMGR_COMMONTYPES_HXX_
#include "commontypes.hxx"
#endif

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
        class Name;
        class AbsolutePath;
    }
    //-------------------------
    namespace memory
    {
        class Segment;
        class Accessor;
        class UpdateAccessor;
    }
    //-------------------------
    namespace data
    {
        class NodeAccess;
        class TreeAccessor;
    }
    //-------------------------
    class ISubtree;
    struct TreeChangeList;

    class RequestOptions;

    //==========================================================================
    //= ITreeManager
    //==========================================================================
    // a TreeProvider which can notify changes that were done, and manages the lifetime of subtrees

    class SAL_NO_VTABLE ITreeManager
    {
    public:
        typedef configuration::AbsolutePath AbsolutePath;

        /// get a data segment to host the given location
        virtual memory::Segment* getDataSegment(  AbsolutePath const& _rAccessor,
                                                    const RequestOptions& _aOptions) = 0;

       /** request that the tree named by a path is added to the collection of managed trees
            respecting certain options and requiring a specific loading depth.
            Return a reference to that managed tree.
            The reference must later be released by calling releaseSubtree with the same path and options.
        */
        virtual data::NodeAccess requestSubtree(AbsolutePath const& aSubtreePath,
                                                const RequestOptions& _aOptions) CFG_UNO_THROW_ALL(  ) = 0;

        /** request that the tree named by a path is added to the collection of managed trees
            respecting certain options and requiring a specific loading depth.
        */
        virtual void fetchSubtree(AbsolutePath const& aSubtreePath,
                                  const RequestOptions& _aOptions) CFG_NOTHROW() = 0;

        /// update the managed data according to a changes list - update the changes list accordingly with old values
        virtual void updateTree(memory::UpdateAccessor& _aAccessToken, TreeChangeList& aChanges) CFG_UNO_THROW_ALL(  ) = 0;

        // notification
        virtual void saveAndNotifyUpdate(memory::Accessor const& _aChangedDataAccessor, TreeChangeList const& aChanges ) CFG_UNO_THROW_ALL(  ) = 0;

        // bookkeeping support
        virtual void releaseSubtree(AbsolutePath const& aSubtreePath,
                                    const RequestOptions& _aOptions ) CFG_NOTHROW() = 0;

        /** data for the given options may not be used any more
            <p>all clients of such data must be disposed</p>
            <p>If the locale is not set, the whole user has become invalid</p>
        */
        virtual void disposeData(const RequestOptions& _aOptions) CFG_NOTHROW() = 0;


    };


    //==========================================================================
    //= ITemplateProvider
    //==========================================================================
    struct SAL_NO_VTABLE ITemplateProvider
    {
        typedef configuration::Name         Name;

        virtual ::std::auto_ptr<INode> loadTemplate(
                                            Name const& aName, Name const& aModule,
                                            const RequestOptions& _aOptions
                                        ) CFG_UNO_THROW_ALL( ) = 0;

    };

    //==========================================================================
    //= ITemplateManager
    //==========================================================================
    class SAL_NO_VTABLE ITemplateManager
    {
    public:
        typedef configuration::Name         Name;

        virtual data::TreeAccessor requestTemplate(
                                            memory::Accessor const& _aAccessor,
                                            Name const& aName, Name const& aModule
                                        ) CFG_UNO_THROW_ALL( ) = 0;

    };

////////////////////////////////////////////////////////////////////////////////

    //==========================================================================
    //= INotifyListener
    //==========================================================================
    /** a listener on configuration nodes. able to receive all changes in one or more
        specific registry sub trees.
    */
    struct SAL_NO_VTABLE INotifyListener : public IInterface
    {
        /** called whenever another session modified a node which the listener is registered for
            @param      _rChanges       The list of changes for a node.
        */
        virtual void    nodeUpdated(TreeChangeList& _rChanges) = 0;
    };

    //==========================================================================
    /// a refcounted TemplateManager
    struct SAL_NO_VTABLE IConfigTemplateManager
    : public Refcounted
    , public ITemplateManager
    {};

    //==========================================================================
    /// a complete combined TreeManager
    struct SAL_NO_VTABLE IConfigTreeManager
    : public IConfigTemplateManager
    , public ITreeManager
    {};


    //==========================================================================
} // namespace configmgr

#endif

