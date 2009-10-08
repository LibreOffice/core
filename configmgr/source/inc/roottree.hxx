/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: roottree.hxx,v $
 * $Revision: 1.15 $
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

#ifndef CONFIGMGR_ROOTTREE_HXX_
#define CONFIGMGR_ROOTTREE_HXX_

#include "sal/config.h"

#include <memory>

#include "boost/utility.hpp"
#include "rtl/ref.hxx"

#include "utility.hxx"

namespace configmgr
{
//-----------------------------------------------------------------------------
    namespace sharable { union Node; }
    class SubtreeChange;
    struct TreeChangeList;

    namespace configuration
    {
//-----------------------------------------------------------------------------
        class Tree;
        class NodeRef;
        class NodeChangesInformation;
        class AbsolutePath;
        class TemplateProvider;

//-----------------------------------------------------------------------------

        rtl::Reference< Tree > createReadOnlyTree(  AbsolutePath const& aRootPath,
                                                    sharable::Node * _aCacheNode,
                                        unsigned int nDepth,
                                        TemplateProvider const& aTemplateProvider);

        rtl::Reference< Tree > createUpdatableTree( AbsolutePath const& aRootPath,
                                                    sharable::Node * _aCacheNode,
                                        unsigned int nDepth,
                                        TemplateProvider const& aTemplateProvider);

//-----------------------------------------------------------------------------
        class CommitHelper: private boost::noncopyable
        {
            struct Data;

            std::auto_ptr<Data> m_pData;
            Tree*           m_pTree;
        public:
            CommitHelper(rtl::Reference< Tree > const& aTree);
            ~CommitHelper();

            // collect all changes into rChangeList
            bool prepareCommit(TreeChangeList& rChangeList);

            // finish and clean up the changes in rChangeList after they are integrated
            void finishCommit(TreeChangeList& rChangeList);
            // throw away and clean up the changes in rChangeList after a commit failed
            void failedCommit(TreeChangeList& rChangeList);
        };

//-----------------------------------------------------------------------------
        /** adjusts <var>aTree</var> tree to the (externally produced) changes under <var>aExternalChanges</var>
            and collects the changes this induces locally.
            @param rLocalChanges
                a collection that will hold the changes induced by <var>aExternalChanges</var>.
            @param aExternalChanges
                a structured change that has already been applied to the master tree.
            @param aBaseTree
                the tree that contains (directly) the affected node of <var>aExternalChanges</var>.
            @param aBaseNode
                a NodeRef referring to the (directly) affected node of <var>aExternalChanges</var>.
            @return
                <TRUE/> if any changes occur in this tree (so rLocalChanges is not empty), <FALSE/> otherwise.

        */
        bool adjustToChanges(   NodeChangesInformation& rLocalChanges,
                                rtl::Reference< Tree > const& aBaseTree, NodeRef const& aBaseNode,
                                SubtreeChange const& aExternalChange) ;


//-----------------------------------------------------------------------------
    }
}

#endif // CONFIGMGR_ROOTTREE_HXX_
