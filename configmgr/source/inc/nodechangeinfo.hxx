/*************************************************************************
 *
 *  $RCSfile: nodechangeinfo.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 16:19:00 $
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

#ifndef CONFIGMGR_CONFIGCHANGEINFO_HXX_
#define CONFIGMGR_CONFIGCHANGEINFO_HXX_

#ifndef CONFIGMGR_CONFIGPATH_HXX_
#include "configpath.hxx"
#endif
#ifndef CONFIGMGR_CONFIGNODE_HXX_
#include "noderef.hxx"
#endif
#ifndef CONFIGMGR_CONFIGVALUEREF_HXX_
#include "valueref.hxx"
#endif

#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif

#ifndef INCLUDED_VECTOR
#include <vector>
#define INCLUDED_VECTOR
#endif

namespace configmgr
{
    namespace configuration
    {
//-----------------------------------------------------------------------------
        typedef com::sun::star::uno::Type       UnoType;
        typedef com::sun::star::uno::Any        UnoAny;
//-----------------------------------------------------------------------------
        class Tree;
        class NodeRef;
        class ValueRef;
        class NodeID;
        class SubNodeID;

        class ElementTreeImpl;
        typedef rtl::Reference<ElementTreeImpl> ElementTreeHolder;

//-----------------------------------------------------------------------------
        /// captures the values of something changing
        template <class DataT>
        struct DataChange
        {
            DataT newValue;
            DataT oldValue;

            DataChange()
            : newValue(), oldValue()
            {}

            DataChange(DataT const& newValue_, DataT const& oldValue_)
            : newValue(newValue_), oldValue(oldValue_)
            {}

            // note: maybe we should support a comparison object
            bool isDataChange() const
            { return !(oldValue == newValue); } // not using != to avoid conversion warning
        };

//-----------------------------------------------------------------------------
        /// information about what changed (but close to no context)
        class NodeChangeData
        {
        public:
        //-------------------------------------------------
            enum Type
            {
                eNoChange,

            // Changes to value nodes
                eSetValue,
                eSetDefault,

            // Changes to set nodes
                eInsertElement,
                eReplaceElement,
                eRemoveElement,

                eRenameElementTree, // not fully supported yet

                eResetSetDefault
            };
        //-------------------------------------------------

            bool isEmptyChange()    const { return eNoChange == type; }
            bool isValueChange()    const { return eSetValue <= type && type <= eSetDefault; }
            bool isSetChange()      const { return eInsertElement <= type && type <= eRemoveElement; }

        //-------------------------------------------------
            bool isDataChange() const;

        //-------------------------------------------------
        // wrapper object creation
            Tree    getNewElementTree(data::Accessor const& aAccessor) const;
            Tree    getOldElementTree(data::Accessor const& aAccessor) const;

            NodeRef getNewElementNodeRef() const;
            NodeRef getOldElementNodeRef() const;

            NodeID  getNewElementNodeID() const;
            NodeID  getOldElementNodeID() const;
        //-------------------------------------------------

        //-- Compiler barrier for element tree ------------
            NodeChangeData();
            NodeChangeData(NodeChangeData const& aOther);
            NodeChangeData& operator=(NodeChangeData const& aOther);
            ~NodeChangeData();
        //-------------------------------------------------
            Type type;

            // Value change: old/new value; Set change: new/old api element (if known); Rename: old/new name
            DataChange< UnoAny              > unoData;
            // Value change: NULL,NULL; Set change: new/old tree element; Rename: the affected element-tree (twice)
            DataChange< ElementTreeHolder   > element;
        //-------------------------------------------------
        };

        //-------------------------------------------------
        // Identify the location of a change. Interpretation of members may depend upon
        class NodeChangeLocation
        {
        public:
        //-------------------------------------------------
            // checks whether the base has been properly set up.
            // Does not check for existence of the affected node

            /// check whether the location has been initialized properly
            bool isValidData() const;

            /// check whether the location is for a valid object
            bool isValidLocation(data::Accessor const& aAccessor) const;

        //-------------------------------------------------
            /// retrieve the path from the base node to the changed node (which might be a child of the affected node)
            RelativePath getAccessor() const { return m_path; }

            /// retrieve the tree where the change is actually initiated/reported
            Tree getBaseTree(data::Accessor const& aAccessor) const;
            /// retrieve the node where the change is actually initiated/reported
            NodeRef getBaseNode() const;

            /// retrieve the tree where the change is actually taking place (may be Empty, if the tree has never been accessed)
            Tree getAffectedTree(data::Accessor const& aAccessor) const;
            /// retrieve the tree where the change is actually taking place (may be Empty, if the tree has never been accessed)
            TreeRef getAffectedTreeRef() const;
            /// retrieve the node where the change is actually taking place (if the affected Tree is not empty)
            NodeRef getAffectedNode() const;
            /// identify the node where the change is actually taking place
            NodeID getAffectedNodeID() const;

            /// identify the node (within the affected tree), that actually is changed (this one may be a value node)
            SubNodeID getChangingValueID() const;

        //-------------------------------------------------
            void setAccessor( RelativePath const& aAccessor );

            void setBase( NodeID const& aBaseID );
            void setBase( Tree const& aBaseTree, NodeRef const& aBaseNode )
            { setBase( NodeID(aBaseTree,aBaseNode) ); }

            void setAffected( NodeID const& aTargetID );
            void setAffected( Tree const& aTargetTree, NodeRef const& aTargetNode )
            { setAffected( NodeID(aTargetTree,aTargetNode) ); }

            void setChangingSubnode( bool bSubnode = true );
        //-------------------------------------------------
            NodeChangeLocation();
        //  NodeChangeLocation(NodeChangeLocation const& aOther);
        //  NodeChangeLocation& operator=(NodeChangeLocation const& aOther);
        //  ~NodeChangeLocation();
        //-------------------------------------------------
        private:
            RelativePath    m_path;     // path from baseNode to changing node
            NodeID          m_base;     // a (non-empty) node
            NodeID          m_affected; // identifies the affected node (if available)
            bool            m_bSubNodeChanging; // do we change a value ?
        //-------------------------------------------------
        };
//-----------------------------------------------------------------------------
        class NodeChangeInformation
        {
        public:
        //-------------------------------------------------
            explicit
            NodeChangeInformation(data::Accessor const& _accessor)
            : accessor(_accessor)
            , change()
            , location()
            {
            }
        //-------------------------------------------------
            data::Accessor      accessor;
            NodeChangeData      change;
            NodeChangeLocation  location;

        //-------------------------------------------------
            bool hasValidLocation() const { return location.isValidLocation(accessor); }
            bool isDataChange()     const { return change.isDataChange(); }

            bool isEmptyChange()    const { return change.isEmptyChange(); }
            bool isValueChange()    const { return change.isValueChange(); }
            bool isSetChange()      const { return change.isSetChange(); }
        //-------------------------------------------------
        };
//-----------------------------------------------------------------------------

        class NodeChangesInformation
        {
            typedef std::vector< NodeChangeInformation > Rep;
        public:
            typedef Rep::const_iterator Iterator;

            Rep::size_type size() const { return m_data.size(); }
            bool empty() const { return m_data.empty(); }

            void reserve(Rep::size_type sz_)    { m_data.reserve(sz_); }
            void clear() { m_data.clear(); }
            void swap(NodeChangesInformation& aOther) throw() { m_data.swap(aOther.m_data); }

            void push_back(NodeChangeInformation const& aChange_)
            { m_data.push_back(aChange_); }

            Iterator begin() const { return m_data.begin(); }
            Iterator end() const { return m_data.end(); }
        private:
            Rep m_data;
        };
//-----------------------------------------------------------------------------
    }
}

namespace std
{
    template <>
    inline
    void swap(configmgr::configuration::NodeChangesInformation& lhs, configmgr::configuration::NodeChangesInformation& rhs)
    { lhs.swap(rhs); }
}

#endif // CONFIGMGR_CONFIGCHANGEINFO_HXX_
