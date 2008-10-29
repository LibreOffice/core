/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: apifactory.cxx,v $
 * $Revision: 1.14 $
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

#include "apifactory.hxx"
#include "objectregistry.hxx"

#include "apitreeaccess.hxx"
#include "apitreeimplobj.hxx"

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/lang/XUnoTunnel.hpp>

#include "noderef.hxx"
#include "anynoderef.hxx"

#include "configexcept.hxx"
#include "configset.hxx"

namespace configmgr
{
//-----------------------------------------------------------------------------
    namespace css = ::com::sun::star;
    namespace uno   = css::uno;
    namespace lang  = css::lang;
//-----------------------------------------------------------------------------
    namespace configapi
    {
//-----------------------------------------------------------------------------
ObjectRegistry::~ObjectRegistry()
{
    OSL_ENSURE(m_aMap.empty(),"WARNING: Configuration Object Map: Some Objects were not revoked correctly");
}

//-----------------------------------------------------------------------------

Factory::Factory(rtl::Reference<ObjectRegistry> pRegistry)
: m_pRegistry(pRegistry)
, m_aTunnelID()
{
    OSL_ENSURE(pRegistry.is(), "ERROR: Factory requires a Object Registry");
}
//-----------------------------------------------------------------------------

Factory::~Factory()
{
}
//-----------------------------------------------------------------------------
inline
NodeElement* Factory::implFind(configuration::NodeID const& aNode)
{
    return m_pRegistry->findElement(aNode);
}
//-----------------------------------------------------------------------------
inline
void Factory::doRegisterElement(configuration::NodeID const& aNode, NodeElement* pElement)
{
    m_pRegistry->registerElement(aNode,pElement);
}
//-----------------------------------------------------------------------------
inline
void Factory::doRevokeElement(configuration::NodeID const& aNode, NodeElement* pElement)
{
    m_pRegistry->revokeElement(aNode,pElement);
}
//-----------------------------------------------------------------------------

ApiTreeImpl& Factory::getImplementation(NodeElement& rElement)
{
    return rElement.getApiTree();
}
//-----------------------------------------------------------------------------

inline
rtl::Reference<configuration::Template> Factory::implGetSetElementTemplate(rtl::Reference< configuration::Tree > const& aTree, configuration::NodeRef const& aNode)
{
    rtl::Reference<configuration::Template> aRet;
    if (configuration::isSetNode(aTree,aNode))
    {
        aRet = aTree->extractElementInfo(aNode);
    }
    else if (!configuration::isGroupNode(aTree,aNode))
    {
        OSL_ENSURE( !configuration::isStructuralNode(aTree,aNode), "ERROR: Configuration: unknown kind of object");
        throw configuration::Exception("INTERNAL ERROR: Cannot create template - Unexpected node type");
    }
    return aRet;
}
//-----------------------------------------------------------------------------
inline
uno::Reference< uno::XInterface > Factory::implToUno(NodeElement* pElement)
{
    if ( pElement )
        return uno::Reference< uno::XInterface >(pElement->getUnoInstance(), uno::UNO_REF_NO_ACQUIRE);
    else
        return uno::Reference< uno::XInterface >();
}
//-----------------------------------------------------------------------------
inline
void Factory::implHaveNewElement(configuration::NodeID aNodeID, NodeElement* pElement)
{
    OSL_ENSURE(pElement ,"WARNING: New API object could not be created");

    if (pElement)
    {
        doRegisterElement(aNodeID,pElement);
        OSL_ENSURE(implFind(aNodeID) == pElement,"WARNING: New API object could not be registered with its factory");
    }
}
//-----------------------------------------------------------------------------

uno::Reference< uno::XInterface > Factory::makeUnoElement(rtl::Reference< configuration::Tree > const& aTree, configuration::NodeRef const& aNode)
{
    return implToUno(makeElement(aTree,aNode));
}
//-----------------------------------------------------------------------------
NodeElement* Factory::makeElement(rtl::Reference< configuration::Tree > const& aTree, configuration::NodeRef const& aNode)
{
    OSL_PRECOND( !configuration::isEmpty(aTree.get()) == aNode.isValid(), "ERROR: Configuration: Making element from tree requires valid node");
    if (configuration::isEmpty(aTree.get()))
        return 0;

    OSL_PRECOND( aNode.isValid() && aTree->isValidNode(aNode.getOffset()), "ERROR: Configuration: NodeRef does not match Tree");
    OSL_PRECOND( configuration::isStructuralNode(aTree,aNode), "ERROR: Configuration: Cannot make object for value node");

    configuration::NodeID aNodeID(aTree,aNode);
    NodeElement* pRet = findElement(aNodeID);
    if (pRet == 0)
    {
        rtl::Reference<configuration::Template> aTemplate = implGetSetElementTemplate(aTree,aNode);

        if (!aTree->isRootNode(aNode))
        {
            pRet = doCreateGroupMember(aTree,aNode,aTemplate.get());
        }
        else
        {
            rtl::Reference< configuration::ElementTree > aElementTree(dynamic_cast< configuration::ElementTree * >(aTree.get()));
            if (aElementTree.is())
            {
                pRet = doCreateSetElement(aElementTree,aTemplate.get());
            }
            else
            {
                OSL_ENSURE(configuration::isEmpty(aTree->getContextTree()),"INTERNAL ERROR: Found tree (not a set element) with a parent tree.");
                pRet = doCreateAccessRoot(aTree,aTemplate.get(), vos::ORef< OOptions >());
            }
        }
        implHaveNewElement(aNodeID,pRet);
    }
    return pRet;
}
//-----------------------------------------------------------------------------

uno::Reference< uno::XInterface > Factory::findUnoElement(configuration::NodeID const& aNodeID)
{
    return implToUno(findElement(aNodeID));
}
//-----------------------------------------------------------------------------
NodeElement* Factory::findElement(configuration::NodeID const& aNodeID)
{
    NodeElement* pReturn = implFind(aNodeID);
    if (pReturn) pReturn->getUnoInstance()->acquire();
    return pReturn;
}
//-----------------------------------------------------------------------------

void Factory::revokeElement(configuration::NodeID const& aNodeID)
{
    if (NodeElement* pElement = implFind(aNodeID))
        doRevokeElement(aNodeID, pElement);
}
//-----------------------------------------------------------------------------

TreeElement* Factory::makeAccessRoot(rtl::Reference< configuration::Tree > const& aTree, RequestOptions const& _aOptions)
{
    OSL_PRECOND( !configuration::isEmpty(aTree.get()) , "ERROR: Configuration: Making element from tree requires valid tree");
    if (configuration::isEmpty(aTree.get())) return 0;

    OSL_ENSURE(configuration::isEmpty(aTree->getContextTree()),"INTERNAL ERROR: Tree with parent tree should not be used for an access root");
    OSL_ENSURE(dynamic_cast< configuration::ElementTree * >(aTree.get()) == 0, "INTERNAL ERROR: Element Tree should not be used for an access root");

    configuration::NodeRef aRoot = aTree->getRootNode();
    OSL_ENSURE(aRoot.isValid(),"INTERNAL ERROR: Tree has no root node");

    OSL_PRECOND( configuration::isStructuralNode(aTree,aRoot), "ERROR: Configuration: Cannot make object for value node");

    configuration::NodeID aNodeID(aTree,aRoot);
    // must be a tree element if it is a tree root
    TreeElement* pRet = static_cast<TreeElement*>(findElement(aNodeID));
    if (0 == pRet)
    {
        rtl::Reference<configuration::Template> aTemplate = implGetSetElementTemplate(aTree,aRoot);
        vos::ORef<OOptions> xOptions = new OOptions(_aOptions);
        pRet = doCreateAccessRoot(aTree,aTemplate.get(), xOptions);
        implHaveNewElement (aNodeID,pRet);
    }
    return pRet;
}
//-----------------------------------------------------------------------------

uno::Reference< uno::XInterface > Factory::makeUnoGroupMember(rtl::Reference< configuration::Tree > const& aTree, configuration::NodeRef const& aNode)
{
    return implToUno(makeGroupMember(aTree,aNode));
}
//-----------------------------------------------------------------------------
NodeElement* Factory::makeGroupMember(rtl::Reference< configuration::Tree > const& aTree, configuration::NodeRef const& aNode)
{
    OSL_PRECOND( !configuration::isEmpty(aTree.get()) , "ERROR: Configuration: Creating an object requires a valid tree");
    if (configuration::isEmpty(aTree.get())) return 0;

    OSL_PRECOND( aNode.isValid() , "ERROR: Configuration: Creating an object requires a valid node");
    OSL_PRECOND( aTree->isValidNode(aNode.getOffset()), "ERROR: Configuration: NodeRef does not match Tree");
    if (!aTree->isValidNode(aNode.getOffset())) return 0;

    OSL_PRECOND( configuration::isStructuralNode(aTree,aNode), "ERROR: Configuration: Cannot make object for value node");
    OSL_ENSURE(!aTree->isRootNode(aNode),"INTERNAL ERROR: Root of Tree should not be used for a group member object");

    configuration::NodeID aNodeID(aTree,aNode);
    NodeElement* pRet = findElement(aNodeID);
    if (0 == pRet)
    {
        rtl::Reference<configuration::Template> aTemplate = implGetSetElementTemplate(aTree,aNode);

        pRet = doCreateGroupMember(aTree,aNode,aTemplate.get());

        OSL_ENSURE( pRet,"WARNING: New API object could not be created");

        implHaveNewElement(aNodeID,pRet);
    }
    return pRet;
}
//-----------------------------------------------------------------------------

uno::Reference< uno::XInterface > Factory::makeUnoSetElement(rtl::Reference< configuration::ElementTree > const& aElementTree)
{
    uno::Reference< uno::XInterface > aRet = implToUno(makeSetElement(aElementTree));
    OSL_ENSURE( uno::Reference<lang::XUnoTunnel>::query(aRet).is(),"ERROR: API set element has no UnoTunnel");
    OSL_ENSURE( uno::Reference<lang::XUnoTunnel>::query(aRet).is() &&
                0 != uno::Reference<lang::XUnoTunnel>::query(aRet)->getSomething(doGetElementTunnelID()),
                "ERROR: API set element does not support the right tunnel ID");

    return aRet;
}
//-----------------------------------------------------------------------------

SetElement* Factory::makeSetElement(rtl::Reference< configuration::ElementTree > const& aElementTree)
{
    OSL_PRECOND( aElementTree.is() , "ERROR: Configuration: Making element from tree requires valid tree");
    if (!aElementTree.is()) return 0;

    rtl::Reference< configuration::Tree > aTree(aElementTree.get());
    OSL_ENSURE(!configuration::isEmpty(aTree.get()),"INTERNAL ERROR: Element Tree has no Tree");

    configuration::NodeRef aRoot = aTree->getRootNode();
    OSL_ENSURE(aRoot.isValid(),"INTERNAL ERROR: Tree has no root node");

    OSL_ENSURE( configuration::isStructuralNode(aTree,aRoot), "ERROR: Configuration: Cannot make object for value node");

    configuration::NodeID aNodeID(aTree,aRoot);
    // must be a set element if it wraps a ElementTree
    SetElement* pRet = static_cast<SetElement*>( findElement(aNodeID) );
    if (0 == pRet)
    {
        rtl::Reference<configuration::Template> aTemplate = implGetSetElementTemplate(aTree,aRoot);

        pRet = doCreateSetElement(aElementTree,aTemplate.get());

        implHaveNewElement(aNodeID,pRet);
    }
    return pRet;
}
//-----------------------------------------------------------------------------

SetElement* Factory::findSetElement(rtl::Reference< configuration::ElementTree > const& aElement)
{
    OSL_PRECOND( aElement.is() , "ERROR: Configuration: Making element from tree requires valid tree");
    if (!aElement.is()) return 0;

    rtl::Reference< configuration::Tree > aTree(aElement.get());
    OSL_ENSURE(!isEmpty(aTree.get()),"INTERNAL ERROR: Element Tree has no Tree");

    configuration::NodeRef aRoot = aTree->getRootNode();
    OSL_ENSURE(aRoot.isValid(),"INTERNAL ERROR: Tree has no root node");

    configuration::NodeID aNodeID(aTree,aRoot);
    // must be a set element if it wraps a ElementTree
    SetElement* pRet = static_cast<SetElement*>( findElement(aNodeID) );

    return pRet;
}
//-----------------------------------------------------------------------------

SetElement* Factory::extractSetElement(uno::Any const& aElement)
{
    SetElement* pTunneledImpl = 0;

    uno::Reference< lang::XUnoTunnel > xElementTunnel;
    if ( aElement.hasValue() && (aElement >>= xElementTunnel) )
    {
        OSL_ASSERT( xElementTunnel.is() );

        sal_Int64 nSomething = xElementTunnel->getSomething(doGetElementTunnelID());
        if (0 != nSomething)
        {
            void* pVoid = reinterpret_cast<void*>(nSomething);
            pTunneledImpl = static_cast<SetElement*>(pVoid);
        }
    }
    return pTunneledImpl;
}
//-----------------------------------------------------------------------------

bool Factory::tunnelSetElement(sal_Int64& nSomething, SetElement& rElement, uno::Sequence< sal_Int8 > const& aTunnelID)
{
    if (aTunnelID == doGetElementTunnelID())
    {
        void* pVoid = &rElement;
        nSomething = reinterpret_cast<sal_Int64>(pVoid);

        return true;
    }
    else
        return false;
}

//-----------------------------------------------------------------------------

ApiTreeImpl const* Factory::findDescendantTreeImpl(configuration::NodeID const& aNode, ApiTreeImpl const* pImpl)
{
    ApiTreeImpl* pRet = 0;
    if (pImpl)
    {
        if ( NodeElement* pElement = pImpl->getFactory().implFind( aNode ) )
            pRet = &pElement->getApiTree();
    }
    return pRet;
}

//-----------------------------------------------------------------------------
    }
}
