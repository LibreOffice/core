/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: treechangelist.hxx,v $
 * $Revision: 1.6 $
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


#ifndef CONFIGMGR_TREECHANGELIST_HXX
#define CONFIGMGR_TREECHANGELIST_HXX

#include "change.hxx"
#include "attributes.hxx"
#include "configpath.hxx"
#include "requestoptions.hxx"

namespace configmgr
{

////////////////////////////////////////////////////////////////////////////////

    //==========================================================================
    //= TreeChangeList
    //==========================================================================
    struct TreeChangeList
    {
        TreeChangeList(const RequestOptions& _aOptions,
                        const configuration::AbsolutePath& _rRootPath,
                        const SubtreeChange& _aSubtree,
                        treeop::DeepChildCopy _doDeepCopy)
                : root(_aSubtree,_doDeepCopy)
                , m_aLocation(_rRootPath)
                , m_aOptions(_aOptions)
            {}

        /** ctor
        @param      _rRootPath      path to the root of the whole to-be-updated subtree
        */
        TreeChangeList( const RequestOptions& _aOptions,
                        const configuration::AbsolutePath& _rRootPath,
                        const node::Attributes& _rAttr = node::Attributes())
                : root(_rRootPath.getLocalName().getName(), _rAttr)
                , m_aLocation(_rRootPath)
                , m_aOptions(_aOptions)
        {}

        /** ctor
        @param      _rLocalName         relative path within the to-be-updated subtree
        */
        TreeChangeList( const RequestOptions& _aOptions,
                        const configuration::AbsolutePath& _rRootPath,
                        rtl::OUString const & _rChildTemplateName,
                        rtl::OUString const & _rChildTemplateModule,
                        const node::Attributes& _rAttr = node::Attributes())
                : root(_rRootPath.getLocalName().getName(), _rChildTemplateName, _rChildTemplateModule, _rAttr)
                , m_aLocation(_rRootPath)
                , m_aOptions(_aOptions)
        {}

        /** ctor
        @param      _rTreeList          list to initialize the path, no childs are copied
        */
        TreeChangeList( const TreeChangeList& _rTree, treeop::NoChildCopy _rNoCopy)
            : root(_rTree.root, _rNoCopy)
            , m_aLocation(_rTree.m_aLocation)
            , m_aOptions(_rTree.m_aOptions)
        {}

        /// is root a change for the module root
        bool isModuleRootChange() const { return m_aLocation.getDepth() <= 1; }

        /// get the module these changes belong to
        rtl::OUString getModuleName() const { return m_aLocation.getModuleName(); }

        /// get the full path to the root (location + root-name)
        void setRootPath(const configuration::AbsolutePath& _rRootPath)
        { m_aLocation = _rRootPath; }

        /// get the full path to the root (location + root-name)
        configuration::AbsolutePath const& getRootNodePath() const { return m_aLocation; }

        /// get the full path to the root (location)
        configuration::AbsolutePath getRootContextPath() const { return m_aLocation.getParentPath(); }

        RequestOptions const & getOptions() const { return m_aOptions; }

    public:
        SubtreeChange root;                      // the root of the whole tree of updates
    private:
        configuration::AbsolutePath            m_aLocation;  // absolute path to the parent of the node corresponding to this->root
        RequestOptions          m_aOptions;      // options for the tree that is concerned by these changes
    };
//----------------------------------------------------------------------------

} // namespace configmgr

#endif

