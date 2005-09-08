/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: nodeimpl.hxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 04:31:49 $
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
#ifndef CONFIGMGR_NODEADDRESS_HXX
#include "nodeaddress.hxx"
#endif

#ifndef _SALHELPER_SIMPLEREFERENCEOBJECT_HXX_
#include <salhelper/simplereferenceobject.hxx>
#endif
#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif

namespace configmgr
{
//-----------------------------------------------------------------------------
    namespace memory { class Accessor; }
    namespace data { class NodeAccessRef; }
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
        typedef rtl::Reference<NodeImpl> NodeImplHolder;

        struct INodeHandler;

        // Almost an interface, but derives from concrete OReference
        class NodeImpl : public salhelper::SimpleReferenceObject
        {
            friend class view::ViewStrategy;
            data::NodeAddress m_aNodeRef_;
        public:
            NodeImpl(data::NodeAddress const & _aNodeRef)
            : m_aNodeRef_(_aNodeRef)
            {}

        public:
//          void directCommitChanges(memory::Accessor const& _aAccessor) { doCommitChanges(_aAccessor); }

            /// provide access to the address of the underlying node
            data::NodeAddress getOriginalNodeAddress() const
            { return m_aNodeRef_; }

            /// provide access to the data of the underlying node
            data::NodeAccessRef getOriginalNodeAccessRef(memory::Accessor const * _pAccessor) const;
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
