/*************************************************************************
 *
 *  $RCSfile: treechangelist.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 16:19:08 $
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
                : m_aOptions(_aOptions),
                  m_aLocation(_rRootPath),
                  root(_aSubtree,_doDeepCopy)
            {}

        /** ctor
        @param      _rRootPath      path to the root of the whole to-be-updated subtree
        */
        TreeChangeList( const RequestOptions& _aOptions,
                        const AbsolutePath& _rRootPath,
                        const NodeAttributes& _rAttr = NodeAttributes())
                : m_aOptions(_aOptions)
                , m_aLocation(_rRootPath)
                , root(_rRootPath.getLocalName().getName().toString(), _rAttr)
        {}

        /** ctor
        @param      _rLocalName         relative path within the to-be-updated subtree
        */
        TreeChangeList( const RequestOptions& _aOptions,
                        const AbsolutePath& _rRootPath,
                        const Name& _rChildTemplateName,
                        const Name& _rChildTemplateModule,
                        const NodeAttributes& _rAttr = NodeAttributes())
                : m_aOptions(_aOptions)
                , m_aLocation(_rRootPath)
                , root(_rRootPath.getLocalName().getName().toString(), _rChildTemplateName.toString(), _rChildTemplateModule.toString(), _rAttr)
        {}

        /** ctor
        @param      _rTreeList          list to initialize the path, no childs are copied
        */
        TreeChangeList( const TreeChangeList& _rTree, SubtreeChange::NoChildCopy _rNoCopy)
            : m_aOptions(_rTree.m_aOptions)
            , m_aLocation(_rTree.m_aLocation)
            , root(_rTree.root, _rNoCopy)
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

