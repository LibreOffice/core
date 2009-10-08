/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: nodechangeinfo.hxx,v $
 * $Revision: 1.12 $
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

#ifndef CONFIGMGR_CONFIGCHANGEINFO_HXX_
#define CONFIGMGR_CONFIGCHANGEINFO_HXX_

#include "configpath.hxx"
#include "noderef.hxx"
#include "valueref.hxx"
#include <rtl/ref.hxx>

#ifndef INCLUDED_VECTOR
#include <vector>
#define INCLUDED_VECTOR
#endif

namespace configmgr
{
    namespace configuration
    {
//-----------------------------------------------------------------------------
        class NodeRef;
        class ValueRef;
        class NodeID;
        class SubNodeID;

        class ElementTree;

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
            bool isRemoveSetChange() const { return eRemoveElement == type;}
            bool isReplaceSetChange() const { return eReplaceElement == type;}
        //-------------------------------------------------
            bool isDataChange() const;

        //-------------------------------------------------
        // wrapper object creation
            rtl::Reference< Tree > getNewElementTree() const;
            rtl::Reference< Tree > getOldElementTree() const;

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
            DataChange< com::sun::star::uno::Any                > unoData;
            // Value change: NULL,NULL; Set change: new/old tree element; Rename: the affected element-tree (twice)
            DataChange< rtl::Reference<ElementTree> > element;
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
#if OSL_DEBUG_LEVEL > 0
            /// check whether the location has been initialized properly
            bool isValidData() const;
#endif
            /// check whether the location is for a valid object
            bool isValidLocation() const;

        //-------------------------------------------------
            /// retrieve the path from the base node to the changed node (which might be a child of the affected node)
            RelativePath getAccessor() const { return m_path; }

            /// retrieve the tree where the change is actually initiated/reported
            rtl::Reference< Tree > getBaseTree() const;
            /// retrieve the node where the change is actually initiated/reported
            NodeRef getBaseNode() const;

            /// retrieve the tree where the change is actually taking place (may be Empty, if the tree has never been accessed)
            rtl::Reference< Tree > getAffectedTreeRef() const;
            /// identify the node where the change is actually taking place
            NodeID getAffectedNodeID() const;

            /// identify the node (within the affected tree), that actually is changed (this one may be a value node)
            SubNodeID getChangingValueID() const;

        //-------------------------------------------------
            void setAccessor( RelativePath const& aAccessor );

            void setBase( NodeID const& aBaseID );
            void setBase( rtl::Reference< Tree > const& aBaseTree, NodeRef const& aBaseNode )
            { setBase( NodeID(aBaseTree,aBaseNode) ); }

            void setAffected( NodeID const& aTargetID );
            void setAffected( rtl::Reference< Tree > const& aTargetTree, NodeRef const& aTargetNode )
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
            NodeChangeInformation()
            : change()
            , location()
            {
            }
        //-------------------------------------------------
            NodeChangeData      change;
            NodeChangeLocation  location;

        //-------------------------------------------------
            bool hasValidLocation() const { return location.isValidLocation(); }
            bool isDataChange()     const { return change.isDataChange(); }

            bool isEmptyChange()    const { return change.isEmptyChange(); }
            bool isValueChange()    const { return change.isValueChange(); }
            bool isSetChange()      const { return change.isSetChange(); }
        //-------------------------------------------------
        };
//-----------------------------------------------------------------------------

        class NodeChangesInformation
        {
        public:
            std::vector< NodeChangeInformation >::size_type size() const { return m_data.size(); }
            bool empty() const { return m_data.empty(); }

            void reserve(std::vector< NodeChangeInformation >::size_type sz_)   { m_data.reserve(sz_); }
            void clear() { m_data.clear(); }
            void swap(NodeChangesInformation& aOther) throw() { m_data.swap(aOther.m_data); }

            void push_back(NodeChangeInformation const& aChange_)
            { m_data.push_back(aChange_); }

            std::vector< NodeChangeInformation >::const_iterator begin() const { return m_data.begin(); }
            std::vector< NodeChangeInformation >::const_iterator end() const { return m_data.end(); }
        private:
            std::vector< NodeChangeInformation > m_data;
        };
//-----------------------------------------------------------------------------
    }
}

#if !defined(WNT) || (defined(WNT) && _MSC_VER < 1400)
namespace std
{
    template <>
    inline
    void swap(configmgr::configuration::NodeChangesInformation& lhs, configmgr::configuration::NodeChangesInformation& rhs)
    { lhs.swap(rhs); }
}
#endif

#endif // CONFIGMGR_CONFIGCHANGEINFO_HXX_
