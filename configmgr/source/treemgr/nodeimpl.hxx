/*************************************************************************
 *
 *  $RCSfile: nodeimpl.hxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-01 13:40:56 $
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
