/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: treechangelist.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 03:59:14 $
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

/* PLEASE DON'T DELETE ANY COMMENT LINES, ALSO IT'S UNNECESSARY. */


#ifndef CONFIGMGR_TREECHANGELIST_HXX
#define CONFIGMGR_TREECHANGELIST_HXX

#ifndef CONFIGMGR_CHANGE_HXX
#include "change.hxx"
#endif
#ifndef CONFIGMGR_CONFIGURATION_ATTRIBUTES_HXX_
#include "attributes.hxx"
#endif
#ifndef CONFIGMGR_CONFIGPATH_HXX_
#include "configpath.hxx"
#endif
#ifndef CONFIGMGR_MISC_REQUESTOPTIONS_HXX_
#include "requestoptions.hxx"
#endif

namespace configmgr
{

////////////////////////////////////////////////////////////////////////////////

    //==========================================================================
    //= TreeChangeList
    //==========================================================================
    struct TreeChangeList
    {
        typedef configuration::AbsolutePath   AbsolutePath;
        typedef configuration::Name           Name;
        typedef node::Attributes              NodeAttributes;

        TreeChangeList(const RequestOptions& _aOptions,
                        const AbsolutePath& _rRootPath,
                        const SubtreeChange& _aSubtree,
                        SubtreeChange::DeepChildCopy _doDeepCopy)
                : root(_aSubtree,_doDeepCopy)
                , m_aLocation(_rRootPath)
                , m_aOptions(_aOptions)
            {}

        /** ctor
        @param      _rRootPath      path to the root of the whole to-be-updated subtree
        */
        TreeChangeList( const RequestOptions& _aOptions,
                        const AbsolutePath& _rRootPath,
                        const NodeAttributes& _rAttr = NodeAttributes())
                : root(_rRootPath.getLocalName().getName().toString(), _rAttr)
                , m_aLocation(_rRootPath)
                , m_aOptions(_aOptions)
        {}

        /** ctor
        @param      _rLocalName         relative path within the to-be-updated subtree
        */
        TreeChangeList( const RequestOptions& _aOptions,
                        const AbsolutePath& _rRootPath,
                        const Name& _rChildTemplateName,
                        const Name& _rChildTemplateModule,
                        const NodeAttributes& _rAttr = NodeAttributes())
                : root(_rRootPath.getLocalName().getName().toString(), _rChildTemplateName.toString(), _rChildTemplateModule.toString(), _rAttr)
                , m_aLocation(_rRootPath)
                , m_aOptions(_aOptions)
        {}

        /** ctor
        @param      _rTreeList          list to initialize the path, no childs are copied
        */
        TreeChangeList( const TreeChangeList& _rTree, SubtreeChange::NoChildCopy _rNoCopy)
            : root(_rTree.root, _rNoCopy)
            , m_aLocation(_rTree.m_aLocation)
            , m_aOptions(_rTree.m_aOptions)
        {}

        /// is root a change for the module root
        bool isModuleRootChange() const { return m_aLocation.getDepth() <= 1; }

        /// get the module these changes belong to
        Name getModuleName() const { return m_aLocation.getModuleName(); }

        /// get the full path to the root (location + root-name)
        void setRootPath(const AbsolutePath& _rRootPath)
        { m_aLocation = _rRootPath; }

        /// get the full path to the root (location + root-name)
        AbsolutePath const& getRootNodePath() const { return m_aLocation; }

        /// get the full path to the root (location)
        AbsolutePath getRootContextPath() const { return m_aLocation.getParentPath(); }

        RequestOptions const & getOptions() const { return m_aOptions; }

    public:
        SubtreeChange root;                      // the root of the whole tree of updates
    private:
        AbsolutePath            m_aLocation;     // absolute path to the parent of the node corresponding to this->root
        RequestOptions          m_aOptions;      // options for the tree that is concerned by these changes
    };
//----------------------------------------------------------------------------

} // namespace configmgr

#endif

