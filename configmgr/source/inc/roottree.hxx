/*************************************************************************
 *
 *  $RCSfile: roottree.hxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 16:19:03 $
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

    namespace memory
    {
        class Segment;
        class Accessor;
    }
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
                                        memory::Segment const* _pDataSegment,
                                        data::NodeAccess const& _aCacheNode,
                                        TreeDepth nDepth,
                                        TemplateProvider const& aTemplateProvider);

        RootTree createUpdatableTree(   AbsolutePath const& aRootPath,
                                        memory::Segment const* _pDataSegment,
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
            bool prepareCommit(memory::Accessor const & _aAccessor, TreeChangeList& rChangeList);

            // finish and clean up the changes in rChangeList after they are integrated
            void finishCommit(memory::Accessor const & _aAccessor, TreeChangeList& rChangeList);
            // restore the changes in rChangeList as pending
            void revertCommit(memory::Accessor const & _aAccessor, TreeChangeList& rChangeList);
            // throw away and clean up the changes in rChangeList after a commit failed
            void failedCommit(memory::Accessor const & _aAccessor, TreeChangeList& rChangeList);

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
