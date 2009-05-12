/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: apifactoryimpl.cxx,v $
 * $Revision: 1.13 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_configmgr.hxx"
#include <stdio.h>
#include "apifactoryimpl.hxx"

#include "setobjects.hxx"
#include "groupobjects.hxx"

#include "configset.hxx"
#include "configpath.hxx"
#include "template.hxx"
#include "noderef.hxx"
#include "objectregistry.hxx"
#include "attributes.hxx"

namespace configmgr
{
    namespace configapi
    {
//-----------------------------------------------------------------------------
// class ReadOnlyObjectFactory
//-----------------------------------------------------------------------------

ReadOnlyObjectFactory::ReadOnlyObjectFactory(ApiProvider& rProvider,rtl::Reference<ObjectRegistry> pRegistry)
: Factory(pRegistry)
, m_rProvider(rProvider)
{
}
//-----------------------------------------------------------------------------

ReadOnlyObjectFactory::~ReadOnlyObjectFactory()
{
}
//-----------------------------------------------------------------------------

NodeElement* ReadOnlyObjectFactory::doCreateGroupMember(rtl::Reference< configuration::Tree > const& aTree, configuration::NodeRef const& aNode, configuration::Template* pSetElementTemplate)
{
    OSL_ENSURE(!configuration::isEmpty(aTree.get()), "ERROR: trying to create a group member without a tree");
    OSL_ENSURE(aNode.isValid(), "ERROR: trying to create a group member without a node");
    OSL_ENSURE(aTree->isValidNode(aNode.getOffset()), "ERROR: node does not match tree , while trying to create a group member");
    OSL_ENSURE(!aTree->isRootNode(aNode), "ERROR: trying to create a group member on a root node");
    if (aTree->isRootNode(aNode))
        return 0;

    NodeElement* pRootElement = makeElement(aTree,aTree->getRootNode());
    OSL_ENSURE(pRootElement, "Could not create root element of tree - cannot create group member object");
    if (!pRootElement)
        return 0;

    uno::Reference<uno::XInterface> aRootRelease(pRootElement->getUnoInstance(), uno::UNO_REF_NO_ACQUIRE);
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

TreeElement* ReadOnlyObjectFactory::doCreateAccessRoot(rtl::Reference< configuration::Tree > const& aTree, configuration::Template* pSetElementTemplate, vos::ORef< OOptions >const& _xOptions)
{
    OSL_ENSURE(!configuration::isEmpty(aTree.get()), "ERROR: trying to create a root object without a tree");

    TreeElement * pResult = 0;
    if (!pSetElementTemplate)
    {
         ORootElementGroupInfo * pNewObject = new ORootElementGroupInfo(m_rProvider, aTree, _xOptions);
         pNewObject->acquire();
         pResult = &pNewObject->getElementClass();
    }
    else
    {
         ORootElementSetInfo * pNewObject = new ORootElementSetInfo(m_rProvider, aTree, _xOptions);
         pNewObject->acquire();
         pResult = &pNewObject->getElementClass();
    }
    return pResult;
}

//-----------------------------------------------------------------------------
SetElement* ReadOnlyObjectFactory::doCreateSetElement(rtl::Reference< configuration::ElementTree > const& aElementTree, configuration::Template* pSetElementTemplate)
{
    OSL_ENSURE(aElementTree.is(), "ERROR: trying to create a set element object without a tree");

    rtl::Reference< configuration::Tree > aTree( aElementTree.get() );
    OSL_ENSURE(!configuration::isEmpty(aTree.get()), "ERROR: trying to create a set element object without a tree");

    ApiTreeImpl * pParentContext = 0;
    uno::Reference<uno::XInterface> aParentRelease;

    rtl::Reference< configuration::Tree > aParentTree = aTree->getContextTree();
    if (!configuration::isEmpty(aParentTree.get()))
    {
        //configuration::NodeRef aParentNode = aTree.getContextNode();
        configuration::NodeRef aParentRoot = aParentTree->getRootNode();
        if (NodeElement* pParentRootElement = makeElement(aParentTree,aParentRoot) )
        {
            aParentRelease = uno::Reference<uno::XInterface>(pParentRootElement->getUnoInstance(), uno::UNO_REF_NO_ACQUIRE);
            pParentContext = &getImplementation(*pParentRootElement);
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

UpdateObjectFactory::UpdateObjectFactory(ApiProvider& rProvider,rtl::Reference<ObjectRegistry> pRegistry)
: Factory(pRegistry)
, m_rProvider(rProvider)
{
}
//-----------------------------------------------------------------------------

UpdateObjectFactory::~UpdateObjectFactory()
{
}
//-----------------------------------------------------------------------------

bool UpdateObjectFactory::implIsReadOnly(rtl::Reference< configuration::Tree > const& aTree, configuration::NodeRef const& aNode)
{
    OSL_ENSURE(!configuration::isEmpty(aTree.get()), "ERROR: trying to create an object without a tree");
    OSL_ENSURE(aNode.isValid(), "ERROR: trying to create an object without a node");
    OSL_ENSURE(aTree->isValidNode(aNode.getOffset()), "ERROR: node does not match tree , while trying to create an object");

    return aTree->getAttributes(aNode).isReadonly();
}
//-----------------------------------------------------------------------------

NodeElement* UpdateObjectFactory::doCreateGroupMember(rtl::Reference< configuration::Tree > const& aTree, configuration::NodeRef const& aNode, configuration::Template* pSetElementTemplate)
{
    OSL_ENSURE(!configuration::isEmpty(aTree.get()), "ERROR: trying to create a group member without a tree");
    OSL_ENSURE(aNode.isValid(), "ERROR: trying to create a group member without a node");
    OSL_ENSURE(aTree->isValidNode(aNode.getOffset()), "ERROR: node does not match tree , while trying to create a group");

    NodeElement* pRootElement = makeElement(aTree,aTree->getRootNode());
    OSL_ENSURE(pRootElement, "Could not create root element of tree - cannot create group member object");
    if (!pRootElement)
        return 0;

    uno::Reference<uno::XInterface> aRootRelease(pRootElement->getUnoInstance(), uno::UNO_REF_NO_ACQUIRE);
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

TreeElement* UpdateObjectFactory::doCreateAccessRoot(rtl::Reference< configuration::Tree > const& aTree, configuration::Template* pSetElementTemplate, vos::ORef< OOptions >const& _xOptions)
{
    OSL_ENSURE(!configuration::isEmpty(aTree.get()), "ERROR: trying to create a root object without a tree");

    TreeElement * pResult = 0;
    if (implIsReadOnly(aTree,aTree->getRootNode()))
    {
        OSL_ENSURE(false, "WARNING: Trying to create an 'Update Access' on a read-only tree/node");
        if (!pSetElementTemplate)
        {
             ORootElementGroupInfo * pNewObject = new ORootElementGroupInfo(m_rProvider, aTree, _xOptions);
             pNewObject->acquire();
             pResult = &pNewObject->getElementClass();
        }
        else
        {
             ORootElementSetInfo * pNewObject = new ORootElementSetInfo(m_rProvider, aTree, _xOptions);
             pNewObject->acquire();
             pResult = &pNewObject->getElementClass();
        }
    }
    else
    {
        if (!pSetElementTemplate)
        {
             ORootElementGroupUpdate * pNewObject = new ORootElementGroupUpdate(m_rProvider, aTree, _xOptions);
             pNewObject->acquire();
             pResult = &pNewObject->getElementClass();
        }
        else if (pSetElementTemplate->isInstanceValue())
        {
             ORootElementValueSetUpdate * pNewObject = new ORootElementValueSetUpdate(m_rProvider, aTree, _xOptions);
             pNewObject->acquire();
             pResult = &pNewObject->getElementClass();
        }
        else
        {
             ORootElementTreeSetUpdate * pNewObject = new ORootElementTreeSetUpdate(m_rProvider, aTree, _xOptions);
             pNewObject->acquire();
             pResult = &pNewObject->getElementClass();
        }
    }

    return pResult;
}

//-----------------------------------------------------------------------------
SetElement* UpdateObjectFactory::doCreateSetElement(rtl::Reference< configuration::ElementTree > const& aElementTree, configuration::Template* pSetElementTemplate)
{
    OSL_ENSURE(aElementTree.is(), "ERROR: trying to create a set element object without a tree");

    rtl::Reference< configuration::Tree > aTree( aElementTree.get() );
    OSL_ENSURE(!configuration::isEmpty(aTree.get()), "ERROR: trying to create a set element object without a tree");

    ApiTreeImpl * pParentContext = 0;
    uno::Reference<uno::XInterface> aParentRelease;

    rtl::Reference< configuration::Tree > aParentTree = aTree->getContextTree();
    if (!configuration::isEmpty(aParentTree.get()))
    {
        //configuration::NodeRef aParentNode = aTree.getContextNode();
        configuration::NodeRef aParentRoot = aParentTree->getRootNode();
        if (NodeElement* pParentRootElement = makeElement(aParentTree,aParentRoot) )
        {
            aParentRelease = uno::Reference<uno::XInterface>(pParentRootElement->getUnoInstance(), uno::UNO_REF_NO_ACQUIRE);
            pParentContext = &getImplementation(*pParentRootElement);
        }
    }

    SetElement * pResult = 0;
    if (implIsReadOnly(aTree,aTree->getRootNode()))
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

