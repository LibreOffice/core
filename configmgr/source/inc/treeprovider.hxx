/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: treeprovider.hxx,v $
 * $Revision: 1.23 $
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

/* PLEASE DON'T DELETE ANY COMMENT LINES, ALSO IT'S UNNECESSARY. */


#ifndef CONFIGMGR_TREEPROVIDER_HXX
#define CONFIGMGR_TREEPROVIDER_HXX

#include "commontypes.hxx"
#include "valuenode.hxx"
#include "utility.hxx"
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>

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
        virtual void updateTree(TreeChangeList& aChanges) CFG_UNO_THROW_ALL(  ) = 0;

        // notification
        virtual void saveAndNotifyUpdate(TreeChangeList const& aChanges ) CFG_UNO_THROW_ALL(  ) = 0;

        // bookkeeping support
        virtual void releaseSubtree(AbsolutePath const& aSubtreePath,
                                    const RequestOptions& _aOptions ) CFG_NOTHROW() = 0;

        /** data for the given options may not be used any more
            <p>all clients of such data must be disposed</p>
            <p>If the locale is not set, the whole user has become invalid</p>
        */
        virtual void disposeData(const RequestOptions& _aOptions) CFG_NOTHROW() = 0;

        //Refresh all components in the cache
        virtual void refreshAll()CFG_UNO_THROW_ALL() = 0;

        //Flush all components in the cache
        virtual void flushAll()CFG_NOTHROW() = 0;

        //Enable/Disable Asynchronous write-back to cache
        virtual void enableAsync(const sal_Bool& bEnableAsync) CFG_NOTHROW() = 0;
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

