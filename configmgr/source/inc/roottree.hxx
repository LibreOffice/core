/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: roottree.hxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 14:23:34 $
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

#ifndef CONFIGMGR_ROOTTREE_HXX_
#define CONFIGMGR_ROOTTREE_HXX_

#ifndef CONFIGMGR_UTILITY_HXX_
#include "utility.hxx"
#endif

#ifndef INCLUDED_MEMORY
#include <memory>
#define INCLUDED_MEMORY
#endif

namespace configmgr
{
//-----------------------------------------------------------------------------

    class SubtreeChange;
    struct TreeChangeList;
//-----------------------------------------------------------------------------

    namespace data
    {
        class NodeAccess;
    }
//-----------------------------------------------------------------------------

    namespace configuration
    {
//-----------------------------------------------------------------------------
        class Tree; typedef Tree RootTree;
        class TreeRef;
        class TreeImpl;
        class NodeRef;
        class NodeChangesInformation;
        class AbsolutePath;
        class TemplateProvider;
        typedef unsigned int NodeOffset;
        typedef unsigned int TreeDepth;

//-----------------------------------------------------------------------------

        RootTree createReadOnlyTree(    AbsolutePath const& aRootPath,
                                        data::NodeAccess const& _aCacheNode,
                                        TreeDepth nDepth,
                                        TemplateProvider const& aTemplateProvider);

        RootTree createUpdatableTree(   AbsolutePath const& aRootPath,
                                        data::NodeAccess const& _aCacheNode,
                                        TreeDepth nDepth,
                                        TemplateProvider const& aTemplateProvider);

//-----------------------------------------------------------------------------
        class CommitHelper : Noncopyable
        {
            struct Data;

            std::auto_ptr<Data> m_pData;
            TreeImpl*           m_pTree;
        public:
            CommitHelper(TreeRef const& aTree);
            ~CommitHelper();

            // collect all changes into rChangeList
            bool prepareCommit(TreeChangeList& rChangeList);

            // finish and clean up the changes in rChangeList after they are integrated
            void finishCommit(TreeChangeList& rChangeList);
            // restore the changes in rChangeList as pending
            void revertCommit(TreeChangeList& rChangeList);
            // throw away and clean up the changes in rChangeList after a commit failed
            void failedCommit(TreeChangeList& rChangeList);

            // dispose of auxiliary data for a commit operation
            void reset();
        };

//-----------------------------------------------------------------------------
        /** adjusts <var>aTree</var> tree to the (externally produced) changes under <var>aExternalChanges</var>
            and collects the changes this induces locally.
            @param rLocalChanges
                a collection that will hold the changes induced by <var>aExternalChanges</var>.
            @param aExternalChanges
                a structured change that has already been applied to the master tree.
            @param aBaseTree
                the Tree that contains (directly) the affected node of <var>aExternalChanges</var>.
            @param aBaseNode
                a NodeRef referring to the (directly) affected node of <var>aExternalChanges</var>.
            @return
                <TRUE/> if any changes occur in this tree (so rLocalChanges is not empty), <FALSE/> otherwise.

        */
        bool adjustToChanges(   NodeChangesInformation& rLocalChanges,
                                Tree const& aBaseTree, NodeRef const& aBaseNode,
                                SubtreeChange const& aExternalChange) ;


//-----------------------------------------------------------------------------
    }
}

#endif // CONFIGMGR_ROOTTREE_HXX_
