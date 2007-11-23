/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: nodeaccess.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 14:32:25 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_configmgr.hxx"

#include "nodeaccess.hxx"

#ifndef CONFIGMGR_TREEACCESSOR_HXX
#include "treeaccessor.hxx"
#endif
#ifndef CONFIGMGR_SETNODEACCESS_HXX
#include "setnodeaccess.hxx"
#endif
#ifndef CONFIGMGR_GROUPNODEACCESS_HXX
#include "groupnodeaccess.hxx"
#endif
#ifndef CONFIGMGR_VALUENODEACCESS_HXX
#include "valuenodeaccess.hxx"
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

// -----------------------------------------------------------------------------

namespace configmgr
{
// -----------------------------------------------------------------------------
    namespace data
    {
        // -------------------------------------------------------------------------

        NodeAccess getSubnode(NodeAccess const & _aParent, configuration::Name const & _aName)
        {
            if (GroupNodeAccess::isInstance(_aParent))
            {
                return GroupNodeAccess(_aParent).getChildNode(_aName);
            }
            else if (SetNodeAccess::isInstance(_aParent))
            {
                TreeAccessor aElement = SetNodeAccess(_aParent).getElementTree(_aName);
                return aElement != NULL ? aElement.getRootNode() : NodeAccess(NULL);
            }
            else
            {
                OSL_ENSURE( ValueNodeAccess::isInstance(_aParent),"ERROR: Unknown node type");
                OSL_ENSURE(!ValueNodeAccess::isInstance(_aParent),"ERROR: Trying to access child of value node");
                return NULL;
            }
        }
        // -------------------------------------------------------------------------

        NodeAddress getSubnodeAddress(NodeAddress const & _aNodeAddress, configuration::Name const & _aName)
        {
            return getSubnode( NodeAccess(_aNodeAddress), _aName );
        }
        // -------------------------------------------------------------------------

        SetNodeAddress toSetNodeAddress(NodeAddress const & _aNodeAddr)
        {
            SetNodeAccess aNodeAccess(_aNodeAddr);
            return aNodeAccess;
        }
        // -------------------------------------------------------------------------

        GroupNodeAddress toGroupNodeAddress(NodeAddress const & _aNodeAddr)
        {
            GroupNodeAccess aNodeAccess( _aNodeAddr );
            return aNodeAccess;
        }

        // -------------------------------------------------------------------------
        // GroupNodeAccess ...
        // -------------------------------------------------------------------------

        NodeAddress GroupNodeAccess::implGetChild(configuration::Name const& _aName) const
        {
            using namespace sharable;
            rtl::OUString aNodeName = _aName.toString();

            GroupNode const  & aNode = data();
            for (Node const * pChild = aNode.getFirstChild();
                              pChild != NULL;
                              pChild = aNode.getNextChild(pChild))
            {
                if (pChild->isNamed(aNodeName))
                {
                    NodeAccess aChildNode(pChild);
                    return aChildNode;
                }
            }
            return NodeAddress();
        }

        // -------------------------------------------------------------------------
        // SetNodeAccess ...
        // -------------------------------------------------------------------------

        TreeAddress SetNodeAccess::implGetElement(configuration::Name const& _aName) const
        {
            using namespace sharable;
            SetNode const  & aNode = data();
            for (TreeFragment const * pElement = aNode.getFirstElement();
                 pElement  != NULL;
                 pElement  = aNode.getNextElement(pElement))
            {
                if (pElement->isNamed(_aName.toString()))
                    return (TreeAddress)pElement;
            }
            return NULL;
        }
    // -------------------------------------------------------------------------*/

        void SetNodeAccess::addElement(SetNodeAddress _aSetAddress,
                       ElementAddress _aNewElement)
        {
            using namespace sharable;

            SetNode * pNode = _aSetAddress;
            OSL_ENSURE(pNode, "ERROR: Trying to add an element to a NULL set node");

            // to do(?): insert sorted - find location here
            TreeFragment * pElement = _aNewElement;
            OSL_ENSURE(pElement, "ERROR: Trying to add a NULL element to a set node");

            pElement->header.next   = pNode->elements;
            pElement->header.parent = reinterpret_cast<Node *>(pNode);

            pNode->elements = _aNewElement;
        }
    // -------------------------------------------------------------------------*/

        TreeAddress SetNodeAccess::removeElement(SetNodeAddress _aSetAddress,
                         configuration::Name const & _aName)
        {
            using namespace sharable;

            SetNode * pNode = _aSetAddress;
            OSL_ENSURE(pNode, "ERROR: Trying to add an element to a NULL set node");

            TreeAddress aRemoved = NULL;

            List * pLink = & pNode->elements;
            while( TreeFragment * pElement = reinterpret_cast<TreeFragment *>(*pLink) )
            {
                if (pElement->isNamed(_aName.toString()))
                {
                    aRemoved = *pLink;

                    *pLink = pElement->header.next;
                    pElement->header.next   = 0;
                    pElement->header.parent = 0;

                    break;
                }

                pLink = & pElement->header.next;
            }

            return aRemoved;
        }

    }
// -----------------------------------------------------------------------------
} // namespace configmgr


