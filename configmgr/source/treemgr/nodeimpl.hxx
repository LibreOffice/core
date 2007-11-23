/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: nodeimpl.hxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 14:44:39 $
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

#ifndef CONFIGMGR_CONFIGNODEBEHAVIOR_HXX_
#define CONFIGMGR_CONFIGNODEBEHAVIOR_HXX_

#ifndef CONFIGMGR_CONFIGURATION_ATTRIBUTES_HXX_
#include "attributes.hxx"
#endif
#ifndef INCLUDED_SHARABLE_NODE_HXX
#include "node.hxx"
#endif
#ifndef CONFIGMGR_UTILITY_HXX_
#include "utility.hxx"
#endif
#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif

#include "nodeaccess.hxx"

namespace configmgr
{
//-----------------------------------------------------------------------------
    namespace data { class NodeAccess; }
    namespace view { class ViewStrategy; }
//-----------------------------------------------------------------------------
    namespace configuration
    {
    //-----------------------------------------------------------------------------
    typedef unsigned int NodeOffset;

    class TreeImpl;

    class Name;

    class NodeChange;
    class NodeChanges;
    class NodeChangesInformation;

//-----------------------------------------------------------------------------
// Specific types of nodes
//-----------------------------------------------------------------------------

    class NodeImpl;
    struct INodeHandler;

    // Almost an interface, but derives from concrete OReference
    class NodeImpl : public configmgr::SimpleReferenceObject
    {
        friend class view::ViewStrategy;
            data::NodeAddress m_pNodeRef;
    public:
            NodeImpl(data::NodeAddress _pNodeRef)
        : m_pNodeRef(_pNodeRef) {}

    public:
        /// provide access to the address of the underlying node
        data::NodeAddress getOriginalNodeAddress() const
            { return m_pNodeRef; }

        /// provide access to the data of the underlying node
        data::NodeAccess getOriginalNodeAccess() const
        { return data::NodeAccess( m_pNodeRef ); }
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
