/*************************************************************************
 *
 *  $RCSfile: apifactoryimpl.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jb $ $Date: 2000-11-10 12:22:55 $
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

#include "apifactoryimpl.hxx"

#include "setobjects.hxx"
#include "groupobjects.hxx"

#include "configset.hxx"
#include "configpath.hxx"
#include "template.hxx"
#include "noderef.hxx"
#include "objectregistry.hxx"

namespace configmgr
{
    namespace configapi
    {
        using configuration::Template;
        using configuration::NodeID;
//-----------------------------------------------------------------------------
// class ReadOnlyObjectFactory
//-----------------------------------------------------------------------------

ReadOnlyObjectFactory::ReadOnlyObjectFactory(ApiProvider& rProvider,ObjectRegistryHolder pRegistry)
: Factory(pRegistry)
, m_rProvider(rProvider)
{
}
//-----------------------------------------------------------------------------

ReadOnlyObjectFactory::~ReadOnlyObjectFactory()
{
}
//-----------------------------------------------------------------------------

NodeElement* ReadOnlyObjectFactory::doCreateGroupMember(configuration::Tree const& aTree, configuration::NodeRef const& aNode, Template* pSetElementTemplate)
{
    OSL_ENSURE(!aTree.isEmpty(), "ERROR: trying to create a group member without a tree");
    OSL_ENSURE(aNode.isValid(), "ERROR: trying to create a group member without a node");
    OSL_ENSURE(aTree.isValidNode(aNode), "ERROR: node does not match tree , while trying to create a group member");
    OSL_ENSURE(!aTree.isRootNode(aNode), "ERROR: trying to create a group member on a root node");
    if (aTree.isRootNode(aNode))
        return 0;

    NodeElement* pRootElement = makeElement(aTree,aTree.getRootNode());
    OSL_ENSURE(pRootElement, "Could not create root element of tree - cannot create group member object");
    if (!pRootElement)
        return 0;

    UnoInterfaceRef aRootRelease(pRootElement->getUnoInstance(), uno::UNO_REF_NO_ACQUIRE);
    ApiTreeImpl& rRootContext = getImplementation(*pRootElement);

    NodeElement * pResult = 0;
    if (!pSetElementTemplate)
    {
         OInnerGroupInfo * pNewObject = new OInnerGroupInfo(rRootContext, aNode);
         pNewObject->acquire();
         pResult = &pNewObject->getElementClass();
    }
    else
    {
         OInnerSetInfo * pNewObject = new OInnerSetInfo(rRootContext, aNode);
         pNewObject->acquire();
         pResult = &pNewObject->getElementClass();
    }

    return pResult;
}
//-----------------------------------------------------------------------------

TreeElement* ReadOnlyObjectFactory::doCreateAccessRoot(configuration::Tree const& aTree, Template* pSetElementTemplate)
{
    OSL_ENSURE(!aTree.isEmpty(), "ERROR: trying to create a root object without a tree");

    TreeElement * pResult = 0;
    if (!pSetElementTemplate)
    {
         ORootElementGroupInfo * pNewObject = new ORootElementGroupInfo(m_rProvider, aTree);
         pNewObject->acquire();
         pResult = &pNewObject->getElementClass();
    }
    else
    {
         ORootElementSetInfo * pNewObject = new ORootElementSetInfo(m_rProvider, aTree);
         pNewObject->acquire();
         pResult = &pNewObject->getElementClass();
    }
    return pResult;
}

//-----------------------------------------------------------------------------
SetElement* ReadOnlyObjectFactory::doCreateSetElement(configuration::ElementTree const& aElementTree, Template* pSetElementTemplate)
{
    OSL_ENSURE(aElementTree.isValid(), "ERROR: trying to create a set element object without a tree");

    Tree aTree( aElementTree.getTree() );
    OSL_ENSURE(!aTree.isEmpty(), "ERROR: trying to create a set element object without a tree");

    ApiTreeImpl * pParentContext = 0;
    UnoInterfaceRef aParentRelease;

    configuration::Tree aParentTree = aTree.getContextTree();
    if (!aParentTree.isEmpty())
    {
        NodeRef aParentNode = aTree.getContextNode();
        if (NodeElement* pParentElement = makeElement(aParentTree,aParentNode) )
        {
            aParentRelease = UnoInterfaceRef(pParentElement->getUnoInstance(), uno::UNO_REF_NO_ACQUIRE);
            pParentContext = &getImplementation(*pParentElement);
        }
    }

    SetElement * pResult = 0;
    if (!pSetElementTemplate)
    {
         OSetElementGroupInfo * pNewObject = new OSetElementGroupInfo(aTree,m_rProvider,pParentContext);
         pNewObject->acquire();
         pResult = &pNewObject->getElementClass();
    }
    else
    {
         OSetElementSetInfo * pNewObject = new OSetElementSetInfo(aTree,m_rProvider,pParentContext);
         pNewObject->acquire();
         pResult = &pNewObject->getElementClass();
    }
    return pResult;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// class UpdateObjectFactory
//-----------------------------------------------------------------------------

UpdateObjectFactory::UpdateObjectFactory(ApiProvider& rProvider,ObjectRegistryHolder pRegistry)
: Factory(pRegistry)
, m_rProvider(rProvider)
{
}
//-----------------------------------------------------------------------------

UpdateObjectFactory::~UpdateObjectFactory()
{
}
//-----------------------------------------------------------------------------

bool UpdateObjectFactory::implIsReadOnly(configuration::Tree const& aTree, configuration::NodeRef const& aNode)
{
    OSL_ENSURE(!aTree.isEmpty(), "ERROR: trying to create an object without a tree");
    OSL_ENSURE(aNode.isValid(), "ERROR: trying to create an object without a node");
    OSL_ENSURE(aTree.isValidNode(aNode), "ERROR: node does not match tree , while trying to create an object");

    return !aNode.getAttributes().writable;
}
//-----------------------------------------------------------------------------

NodeElement* UpdateObjectFactory::doCreateGroupMember(configuration::Tree const& aTree, configuration::NodeRef const& aNode, Template* pSetElementTemplate)
{
    OSL_ENSURE(!aTree.isEmpty(), "ERROR: trying to create a group member without a tree");
    OSL_ENSURE(aNode.isValid(), "ERROR: trying to create a group member without a node");
    OSL_ENSURE(aTree.isValidNode(aNode), "ERROR: node does not match tree , while trying to create a group");

    NodeElement* pRootElement = makeElement(aTree,aTree.getRootNode());
    OSL_ENSURE(pRootElement, "Could not create root element of tree - cannot create group member object");
    if (!pRootElement)
        return 0;

    UnoInterfaceRef aRootRelease(pRootElement->getUnoInstance(), uno::UNO_REF_NO_ACQUIRE);
    ApiTreeImpl& rRootContext = getImplementation(*pRootElement);

    NodeElement * pResult = 0;

    if (implIsReadOnly(aTree,aNode))
    {
        if (!pSetElementTemplate)
        {
             OInnerGroupInfo * pNewObject = new OInnerGroupInfo(rRootContext, aNode);
             pNewObject->acquire();
             pResult = &pNewObject->getElementClass();
        }
        else
        {
             OInnerSetInfo * pNewObject = new OInnerSetInfo(rRootContext, aNode);
             pNewObject->acquire();
             pResult = &pNewObject->getElementClass();
        }
    }
    else
    {
        if (!pSetElementTemplate)
        {
             OInnerGroupUpdate * pNewObject = new OInnerGroupUpdate(rRootContext, aNode);
             pNewObject->acquire();
             pResult = &pNewObject->getElementClass();
        }
        else if (pSetElementTemplate->isInstanceValue())
        {
             OInnerValueSetUpdate * pNewObject = new OInnerValueSetUpdate(rRootContext, aNode);
             pNewObject->acquire();
             pResult = &pNewObject->getElementClass();
        }
        else
        {
             OInnerTreeSetUpdate * pNewObject = new OInnerTreeSetUpdate(rRootContext, aNode);
             pNewObject->acquire();
             pResult = &pNewObject->getElementClass();
        }
    }

    return pResult;
}
//-----------------------------------------------------------------------------

TreeElement* UpdateObjectFactory::doCreateAccessRoot(configuration::Tree const& aTree, Template* pSetElementTemplate)
{
    OSL_ENSURE(!aTree.isEmpty(), "ERROR: trying to create a root object without a tree");

    TreeElement * pResult = 0;
    if (implIsReadOnly(aTree,aTree.getRootNode()))
    {
        OSL_ENSURE(false, "WARNING: Trying to create an 'Update Access' on a read-only tree/node");
        if (!pSetElementTemplate)
        {
             ORootElementGroupInfo * pNewObject = new ORootElementGroupInfo(m_rProvider, aTree);
             pNewObject->acquire();
             pResult = &pNewObject->getElementClass();
        }
        else
        {
             ORootElementSetInfo * pNewObject = new ORootElementSetInfo(m_rProvider, aTree);
             pNewObject->acquire();
             pResult = &pNewObject->getElementClass();
        }
    }
    else
    {
        if (!pSetElementTemplate)
        {
             ORootElementGroupUpdate * pNewObject = new ORootElementGroupUpdate(m_rProvider, aTree);
             pNewObject->acquire();
             pResult = &pNewObject->getElementClass();
        }
        else if (pSetElementTemplate->isInstanceValue())
        {
             ORootElementValueSetUpdate * pNewObject = new ORootElementValueSetUpdate(m_rProvider, aTree);
             pNewObject->acquire();
             pResult = &pNewObject->getElementClass();
        }
        else
        {
             ORootElementTreeSetUpdate * pNewObject = new ORootElementTreeSetUpdate(m_rProvider, aTree);
             pNewObject->acquire();
             pResult = &pNewObject->getElementClass();
        }
    }

    return pResult;
}

//-----------------------------------------------------------------------------
SetElement* UpdateObjectFactory::doCreateSetElement(configuration::ElementTree const& aElementTree, Template* pSetElementTemplate)
{
    OSL_ENSURE(aElementTree.isValid(), "ERROR: trying to create a set element object without a tree");

    Tree aTree( aElementTree.getTree() );
    OSL_ENSURE(!aTree.isEmpty(), "ERROR: trying to create a set element object without a tree");

    ApiTreeImpl * pParentContext = 0;
    UnoInterfaceRef aParentRelease;

    configuration::Tree aParentTree = aTree.getContextTree();
    if (!aParentTree.isEmpty())
    {
        NodeRef aParentNode = aTree.getContextNode();
        if (NodeElement* pParentElement = makeElement(aParentTree,aParentNode) )
        {
            aParentRelease = UnoInterfaceRef(pParentElement->getUnoInstance(), uno::UNO_REF_NO_ACQUIRE);
            pParentContext = &getImplementation(*pParentElement);
        }
    }

    SetElement * pResult = 0;
    if (implIsReadOnly(aTree,aTree.getRootNode()))
    {
        if (!pSetElementTemplate)
        {
             OSetElementGroupInfo * pNewObject = new OSetElementGroupInfo(aTree,m_rProvider,pParentContext);
             pNewObject->acquire();
             pResult = &pNewObject->getElementClass();
        }
        else
        {
             OSetElementSetInfo * pNewObject = new OSetElementSetInfo(aTree,m_rProvider,pParentContext);
             pNewObject->acquire();
             pResult = &pNewObject->getElementClass();
        }
    }
    else
    {
        if (!pSetElementTemplate)
        {
             OSetElementGroupUpdate * pNewObject = new OSetElementGroupUpdate(aTree,m_rProvider,pParentContext);
             pNewObject->acquire();
             pResult = &pNewObject->getElementClass();
        }
        else if (pSetElementTemplate->isInstanceValue())
        {
             OSetElementValueSetUpdate * pNewObject = new OSetElementValueSetUpdate(aTree,m_rProvider,pParentContext);
             pNewObject->acquire();
             pResult = &pNewObject->getElementClass();
        }
        else
        {
             OSetElementTreeSetUpdate * pNewObject = new OSetElementTreeSetUpdate(aTree,m_rProvider,pParentContext);
             pNewObject->acquire();
             pResult = &pNewObject->getElementClass();
        }
    }
    return pResult;
}
//-----------------------------------------------------------------------------
    }
}

