/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: nodeimpl.hxx,v $
 * $Revision: 1.16 $
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

#ifndef CONFIGMGR_CONFIGNODEBEHAVIOR_HXX_
#define CONFIGMGR_CONFIGNODEBEHAVIOR_HXX_

#include "attributes.hxx"
#include "node.hxx"
#include "utility.hxx"
#include <rtl/ref.hxx>
#include <salhelper/simplereferenceobject.hxx>

namespace configmgr
{
//-----------------------------------------------------------------------------
    namespace view { class ViewStrategy; }
//-----------------------------------------------------------------------------
    namespace configuration
    {
    //-----------------------------------------------------------------------------
    class Tree;
    class NodeChange;
    class NodeChanges;
    class NodeChangesInformation;

//-----------------------------------------------------------------------------
// Specific types of nodes
//-----------------------------------------------------------------------------

    class NodeImpl;
    struct INodeHandler;

    // Almost an interface, but derives from concrete OReference
    class NodeImpl : public salhelper::SimpleReferenceObject
    {
        friend class view::ViewStrategy;
            sharable::Node * m_pNodeRef;
    public:
            NodeImpl(sharable::Node * _pNodeRef)
        : m_pNodeRef(_pNodeRef) {}

    public:
        /// provide access to the data of the underlying node
        sharable::Node * getOriginalNodeAccess() const
        { return m_pNodeRef; }
    };

//-----------------------------------------------------------------------------
        class ValueElementNodeImpl;
        class GroupNodeImpl;
        class SetNodeImpl;
//-----------------------------------------------------------------------------

        struct INodeHandler
        {
            virtual void handle( ValueElementNodeImpl& rNode) = 0;
            virtual void handle( GroupNodeImpl& rNode) = 0;
            virtual void handle( SetNodeImpl& rNode) = 0;
        };

//-----------------------------------------------------------------------------
    }
}

#endif // CONFIGMGR_CONFIGNODEBEHAVIOR_HXX_
