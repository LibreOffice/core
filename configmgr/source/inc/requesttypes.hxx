/*************************************************************************
 *
 *  $RCSfile: requesttypes.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: jb $ $Date: 2002-03-12 14:10:33 $
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

#ifndef CONFIGMGR_BACKEND_REQUESTTYPES_HXX_
#define CONFIGMGR_BACKEND_REQUESTTYPES_HXX_

#ifndef _CONFIGMGR_TREE_VALUENODE_HXX
#include "valuenode.hxx"
#endif
#ifndef CONFIGMGR_TREECHANGELIST_HXX
#include "treechangelist.hxx"
#endif
#ifndef CONFIGMGR_CONFIGPATH_HXX_
#include "configpath.hxx"
#endif

#ifndef INCLUDED_MEMORY
#include <memory>
#define INCLUDED_MEMORY
#endif

namespace configmgr
{
// ---------------------------------------------------------------------------
    namespace backend
    {
// ---------------------------------------------------------------------------
        using configuration::AbsolutePath;
        using configuration::Name;
// ---------------------------------------------------------------------------
        class NodePath
        {
            AbsolutePath m_path;
        public:
            NodePath(AbsolutePath const & _path) : m_path(_path) {};

            AbsolutePath const & location() const { return m_path; }
            AbsolutePath context()          const { return m_path.getParentPath(); }

            bool isEmpty()              const { return m_path.isRoot(); }
            bool isModuleRoot()         const { return m_path.getDepth() == 1; }
            Name getModuleName()        const { return m_path.getModuleName(); }
            rtl::OUString toString()    const { return m_path.toString(); }
        };
// ---------------------------------------------------------------------------
        struct NodeInstance
        {
            typedef std::auto_ptr<ISubtree> Data;

            explicit
            NodeInstance(AbsolutePath const & _rootpath, Data & _node)
            : node(_node)
            , root(_rootpath)
            {
            }

            Data     node;
            NodePath root;
        };
// ---------------------------------------------------------------------------
        struct TemplateInstance
        {
            typedef std::auto_ptr<INode> Data;

            explicit
            TemplateInstance(Data & _node, Name const & _component)
            : node(_node)
            , component(_component)
            {
            }

            Data node;
            Name name; // if empty, this is a complete set of component templates
            Name component;
        };
// ---------------------------------------------------------------------------
        struct UpdateInstance
        {
            typedef std::auto_ptr<SubtreeChange> Data;

            explicit
            UpdateInstance(AbsolutePath const & _rootpath, Data & _update)
            : update(_update)
            , root(_rootpath)
            {
            }

            Data        update;
            NodePath    root;
        };
// ---------------------------------------------------------------------------
    }
// ---------------------------------------------------------------------------
} // namespace

#endif
