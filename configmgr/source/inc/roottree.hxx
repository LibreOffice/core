/*************************************************************************
 *
 *  $RCSfile: roottree.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: jb $ $Date: 2000-11-20 01:38:19 $
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

namespace configmgr
{
    class ISubtree;
    class Change;
    struct TreeChangeList;

    namespace configuration
    {
//-----------------------------------------------------------------------------
        class Tree; typedef Tree RootTree;
        class TreeImpl;
        class NodeRef;
        class NodeChanges;
        class AbsolutePath;
        class TemplateProvider;
        typedef unsigned int NodeOffset;
        typedef unsigned int TreeDepth;

//-----------------------------------------------------------------------------

        RootTree createReadOnlyTree(    AbsolutePath const& aContextPath,
                                        ISubtree& rCacheNode, TreeDepth nDepth,
                                        TemplateProvider const& aTemplateProvider);

        RootTree createUpdatableTree(   AbsolutePath const& aContextPath,
                                        ISubtree& rCacheNode, TreeDepth nDepth,
                                        TemplateProvider const& aTemplateProvider);

//-----------------------------------------------------------------------------
        class CommitHelper
        {
            TreeImpl* m_pTree;
        public:
            CommitHelper(Tree const& aTree);

            // collect all changes into rChangeList
            bool prepareCommit(TreeChangeList& rChangeList);
            // finish and clean up the changes in rChangeList after they are integrated
            void finishCommit(TreeChangeList& rChangeList);
            // restore the changes in rChangeList as pending
            void revertCommit(TreeChangeList& rChangeList);
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
        bool adjustToChanges(   NodeChanges& rLocalChanges,
                                Tree const& aBaseTree, NodeRef const& aBaseNode,
                                Change const& aExternalChange,
                                TemplateProvider const& aTemplateProvider) ;


//-----------------------------------------------------------------------------
    }
}

#endif // CONFIGMGR_ROOTTREE_HXX_
