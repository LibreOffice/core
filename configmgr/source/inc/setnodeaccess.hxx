/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: setnodeaccess.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 14:24:44 $
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

#ifndef CONFIGMGR_SETNODEACCESS_HXX
#define CONFIGMGR_SETNODEACCESS_HXX

#ifndef CONFIGMGR_NODEACCESS_HXX
#include "nodeaccess.hxx"
#endif
#ifndef INCLUDED_SHARABLE_TREEFRAGMENT_HXX
#include "treefragment.hxx"
#endif

#include "treeaccessor.hxx"

namespace configmgr
{
// -----------------------------------------------------------------------------
    namespace data
    {
    // -------------------------------------------------------------------------
        class TreeAccessor;
    // -------------------------------------------------------------------------
        /** class that mediates access to the data of a Set node
            <p>Is a handle class with reference copy semantics.</p>
        */
        class SetNodeAccess
        {
        public:
            typedef TreeAddress    ElementAddress;
            typedef TreeAccessor   ElementAccess;

            SetNodeAccess(const sharable::SetNode *_pNodeRef)
                : m_pData((SetNodeAddress)_pNodeRef)
            {}

            SetNodeAccess(const sharable::Node *_pNodeRef)
                : m_pData(check(_pNodeRef))
            {}

            explicit
            SetNodeAccess(NodeAccess const & _aNode)
                : m_pData(check(_aNode))
            {}

            static bool isInstance(NodeAccess const & _aNode)
                { return check(_aNode) != NULL; }

            bool isValid() const { return m_pData != NULL; }

            configuration::Name getName() const;
            node::Attributes getAttributes() const;

            bool isDefault()   const;

            bool isLocalizedValueSetNode() const;

            configuration::Name getElementTemplateName()   const;
            configuration::Name getElementTemplateModule() const;

            bool            hasElement      (configuration::Name const& _aName) const
                { return SetNodeAccess::implGetElement(_aName) != NULL; }
            ElementAccess   getElementTree  (configuration::Name const& _aName) const
                { return TreeAccessor(implGetElement(_aName)); }

            operator NodeAccess() const { return NodeAccess(NodeAddress(m_pData)); }

            sharable::SetNode & data() const { return *m_pData; }
            operator SetNodeAddress () const { return (SetNodeAddress)m_pData; }
            operator NodeAddress () const { return (NodeAddress)m_pData; }

            static void addElement(SetNodeAddress _aSetAddress, ElementAddress _aNewElement);
            static ElementAddress removeElement(SetNodeAddress _aSetAddress,
                        configuration::Name const & _aName);
        private:
            static SetNodeAddress check(sharable::Node *pNode)
                { return pNode ? const_cast<SetNodeAddress>(pNode->setData()) : NULL; }
            static SetNodeAddress check(NodeAccess const&aRef)
                { return check(static_cast<sharable::Node *>(aRef)); }

            ElementAddress implGetElement(configuration::Name const& _aName) const;

            SetNodeAddress m_pData;
        };

        SetNodeAddress toSetNodeAddress(NodeAddress const & _aNodeAddr);
    // -------------------------------------------------------------------------
        inline
        configuration::Name SetNodeAccess::getName() const
        { return NodeAccess::wrapName( data().info.getName() ); }

        inline
        configuration::Name SetNodeAccess::getElementTemplateName()   const
        { return NodeAccess::wrapName( data().getElementTemplateName() ); }

        inline
        configuration::Name SetNodeAccess::getElementTemplateModule() const
        { return NodeAccess::wrapName( data().getElementTemplateModule() ); }

        inline
        node::Attributes SetNodeAccess::getAttributes() const
        { return sharable::node(data()).getAttributes(); }

        inline
        bool SetNodeAccess::isDefault()   const
        { return data().info.isDefault(); }

        inline
        bool SetNodeAccess::isLocalizedValueSetNode() const
        { return data().isLocalizedValue(); }
    // -------------------------------------------------------------------------
    }
// -----------------------------------------------------------------------------
} // namespace configmgr

#endif // CONFIGMGR_SETNODEACCESS_HXX

