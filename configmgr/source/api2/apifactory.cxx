/*************************************************************************
 *
 *  $RCSfile: apifactory.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: jb $ $Date: 2000-11-07 14:34:32 $
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

#include "apifactory.hxx"
#include "objectregistry.hxx"

#include "apitreeaccess.hxx"

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/lang/XUnoTunnel.hpp>

#include "noderef.hxx"
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
        typedef uno::XInterface UnoInterface;
        typedef uno::Reference< uno::XInterface > UnoInterfaceRef;
        typedef uno::Any UnoAny;

        using uno::Reference;
        using lang::XUnoTunnel;
        using configuration::Tree;
        using configuration::ElementTree;
        using configuration::TemplateHolder;
        using configuration::NodeRef;
        using configuration::NodeID;

//-----------------------------------------------------------------------------
ObjectRegistry::~ObjectRegistry()
{
    OSL_ENSURE(m_aMap.empty(),"WARNING: Configuration Object Map: Some Objects were not revoked correctly");
}

//-----------------------------------------------------------------------------

Factory::Factory(ObjectRegistryHolder pRegistry)
: m_pRegistry(pRegistry)
, m_aTunnelID()
{
    OSL_ENSURE(pRegistry.isValid(), "ERROR: Factory requires a Object Registry");
}
//-----------------------------------------------------------------------------

Factory::~Factory()
{
}
//-----------------------------------------------------------------------------
inline
osl::Mutex& Factory::doGetMutex()
{
    return m_pRegistry->mutex();
}
//-----------------------------------------------------------------------------
inline
NodeElement* Factory::doFindElement(configuration::NodeID const& aNode)
{
    NodeElement* pFound = m_pRegistry->findElement(aNode);
    if (pFound) pFound->getUnoInstance()->acquire();
    return pFound;
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
TemplateHolder Factory::implGetSetElementTemplate(Tree const& aTree, NodeRef const& aNode)
{
    TemplateHolder aRet;
    if (configuration::isSetNode(aTree,aNode))
    {
        aRet = configuration::SetElementInfo::extractElementInfo(aTree,aNode);
    }
    else if (!configuration::isGroupNode(aTree,aNode))
    {
        OSL_ENSURE( configuration::isSimpleValue(aTree,aNode), "ERROR: Configuration: unknown kind of object");
        throw configuration::Exception("INTERNAL ERROR: Cannot create object - Illegal node type");
    }
    return aRet;
}
//-----------------------------------------------------------------------------
inline
UnoInterfaceRef Factory::implToUno(NodeElement* pElement)
{
    if ( pElement )
        return UnoInterfaceRef(pElement->getUnoInstance(), uno::UNO_REF_NO_ACQUIRE);
    else
        return UnoInterfaceRef();
}
//-----------------------------------------------------------------------------
inline
void Factory::implHaveNewElement(NodeID aNodeID, NodeElement* pElement)
{
    OSL_ENSURE(pElement ,"WARNING: New API object could not be created");

    if (pElement)
    {
        doRegisterElement(aNodeID,pElement);
        OSL_ENSURE(doFindElement(aNodeID) == pElement,"WARNING: New API object could not be registered with its factory");
    }
}
//-----------------------------------------------------------------------------

UnoInterfaceRef Factory::makeUnoElement(Tree const& aTree, NodeRef const& aNode)
{
    return implToUno(makeElement(aTree,aNode));
}
//-----------------------------------------------------------------------------
NodeElement* Factory::makeElement(Tree const& aTree, NodeRef const& aNode)
{
    OSL_PRECOND( !aTree.isEmpty() == aNode.isValid(), "ERROR: Configuration: Making element from tree requires valid node");
    if (aTree.isEmpty())
        return 0;

    OSL_PRECOND( aTree.isValidNode(aNode), "ERROR: Configuration: NodeRef does not match Tree");
    OSL_PRECOND( !configuration::isSimpleValue(aTree,aNode), "ERROR: Configuration: Cannot make object for value node");

    osl::MutexGuard aLock(this->doGetMutex());

    NodeID aNodeID(aTree,aNode);
    NodeElement* pRet = doFindElement(aNodeID);
    if (pRet == 0)
    {
        TemplateHolder aTemplate = implGetSetElementTemplate(aTree,aNode);

        if (!aTree.isRootNode(aNode))
        {
            pRet = doCreateGroupMember(aTree,aNode,aTemplate.getBodyPtr());
        }
        else
        {
            ElementTree aElementTree = ElementTree::extract(aTree);
            if (aElementTree.isValid())
            {
                pRet = doCreateSetElement(aElementTree,aTemplate.getBodyPtr());
            }
            else
            {
                OSL_ENSURE(aTree.getContextTree().isEmpty(),"INTERNAL ERROR: Found tree (not a set element) with a parent tree.");
                pRet = doCreateAccessRoot(aTree,aTemplate.getBodyPtr());
            }
        }
        implHaveNewElement(aNodeID,pRet);
    }
    return pRet;
}
//-----------------------------------------------------------------------------

UnoInterfaceRef Factory::findUnoElement(NodeID const& aNodeID)
{
    return implToUno(findElement(aNodeID));
}
//-----------------------------------------------------------------------------
NodeElement* Factory::findElement(NodeID const& aNodeID)
{
    osl::MutexGuard aLock(this->doGetMutex());
    return doFindElement(aNodeID);
}
//-----------------------------------------------------------------------------

void Factory::registerElement(NodeID const& aNodeID, NodeElement& rElement)
{
    osl::MutexGuard aLock(this->doGetMutex());
    if (NodeElement* pExist = doFindElement(aNodeID))
    {
        OSL_ENSURE(pExist == &rElement,"ERROR: A different Configuration Element was already registered for the same node");
        OSL_ENSURE(false ,"WARNING: Configuration Element was already registered");
    }
    else
    {
        doRegisterElement(aNodeID, &rElement);
    }
}
//-----------------------------------------------------------------------------

void Factory::revokeElement(NodeID const& aNodeID)
{
    osl::MutexGuard aLock(this->doGetMutex());
    if (NodeElement* pElement = doFindElement(aNodeID))
        doRevokeElement(aNodeID, pElement);
}
//-----------------------------------------------------------------------------

void Factory::revokeElement(NodeID const& aNodeID, NodeElement& rElement)
{
    osl::MutexGuard aLock(this->doGetMutex());
    if (doFindElement(aNodeID) == &rElement)
        doRevokeElement(aNodeID, &rElement);
}
//-----------------------------------------------------------------------------

UnoInterfaceRef Factory::makeUnoAccessRoot(Tree const& aTree)
{
    return implToUno(makeAccessRoot(aTree));
}
//-----------------------------------------------------------------------------
NodeElement* Factory::makeAccessRoot(Tree const& aTree)
{
    OSL_PRECOND( !aTree.isEmpty() , "ERROR: Configuration: Making element from tree requires valid tree");
    if (aTree.isEmpty()) return 0;

    OSL_ENSURE(aTree.getContextTree().isEmpty(),"INTERNAL ERROR: Tree with parent tree should not be used for an access root");
    OSL_ENSURE(!ElementTree::extract(aTree).isValid(),"INTERNAL ERROR: Element Tree should not be used for an access root");

    NodeRef aRoot = aTree.getRootNode();
    OSL_ENSURE(aRoot.isValid(),"INTERNAL ERROR: Tree has no root node");

    OSL_PRECOND( !configuration::isSimpleValue(aTree,aRoot), "ERROR: Configuration: Cannot make object for value node");

    osl::MutexGuard aLock(this->doGetMutex());

    NodeID aNodeID(aTree,aRoot);
    NodeElement* pRet = doFindElement(aNodeID);
    if (0 == pRet)
    {
        TemplateHolder aTemplate = implGetSetElementTemplate(aTree,aRoot);

        pRet = doCreateAccessRoot(aTree,aTemplate.getBodyPtr());

        implHaveNewElement (aNodeID,pRet);
    }
    return pRet;
}
//-----------------------------------------------------------------------------

UnoInterfaceRef Factory::makeUnoGroupMember(Tree const& aTree, NodeRef const& aNode)
{
    return implToUno(makeGroupMember(aTree,aNode));
}
//-----------------------------------------------------------------------------
NodeElement* Factory::makeGroupMember(Tree const& aTree, NodeRef const& aNode)
{
    OSL_PRECOND( !aTree.isEmpty() , "ERROR: Configuration: Creating an object requires a valid tree");
    if (aTree.isEmpty()) return 0;

    OSL_PRECOND( aNode.isValid() , "ERROR: Configuration: Creating an object requires a valid node");
    OSL_PRECOND( aTree.isValidNode(aNode), "ERROR: Configuration: NodeRef does not match Tree");
    if (!aTree.isValidNode(aNode)) return 0;

    OSL_PRECOND( !configuration::isSimpleValue(aTree,aNode), "ERROR: Configuration: Cannot make object for value node");
    OSL_ENSURE(!aTree.isRootNode(aNode),"INTERNAL ERROR: Root of Tree should not be used for a group member object");

    osl::MutexGuard aLock(this->doGetMutex());

    NodeID aNodeID(aTree,aNode);
    NodeElement* pRet = doFindElement(aNodeID);
    if (0 == pRet)
    {
        TemplateHolder aTemplate = implGetSetElementTemplate(aTree,aNode);

        pRet = doCreateGroupMember(aTree,aNode,aTemplate.getBodyPtr());

        OSL_ENSURE( pRet,"WARNING: New API object could not be created");

        implHaveNewElement(aNodeID,pRet);
    }
    return pRet;
}
//-----------------------------------------------------------------------------

UnoInterfaceRef Factory::makeUnoSetElement(ElementTree const& aElementTree)
{
    UnoInterfaceRef aRet = implToUno(makeSetElement(aElementTree));
    OSL_ENSURE( uno::Reference<XUnoTunnel>::query(aRet).is(),"ERROR: API set element has no UnoTunnel");
    OSL_ENSURE( uno::Reference<XUnoTunnel>::query(aRet).is() &&
                0 != uno::Reference<XUnoTunnel>::query(aRet)->getSomething(doGetElementTunnelID()),
                "ERROR: API set element does not support the right tunnel ID");

    return aRet;
}
//-----------------------------------------------------------------------------

NodeElement* Factory::makeSetElement(ElementTree const& aElementTree)
{
    OSL_PRECOND( aElementTree.isValid() , "ERROR: Configuration: Making element from tree requires valid tree");
    if (!aElementTree.isValid()) return 0;

    Tree aTree = aElementTree.getTree();
    OSL_ENSURE(!aTree.isEmpty(),"INTERNAL ERROR: Element Tree has no Tree");

    NodeRef aRoot = aTree.getRootNode();
    OSL_ENSURE(aRoot.isValid(),"INTERNAL ERROR: Tree has no root node");

    OSL_ENSURE( !configuration::isSimpleValue(aTree,aRoot), "ERROR: Configuration: Cannot make object for value node");

    osl::MutexGuard aLock(this->doGetMutex());

    NodeID aNodeID(aTree,aRoot);
    NodeElement* pRet = doFindElement(aNodeID);
    if (0 == pRet)
    {
        TemplateHolder aTemplate = implGetSetElementTemplate(aTree,aRoot);

        pRet = doCreateSetElement(aElementTree,aTemplate.getBodyPtr());

        implHaveNewElement(aNodeID,pRet);
    }
    return pRet;
}
//-----------------------------------------------------------------------------

SetElement* Factory::extractSetElement(UnoAny const& aElement)
{
    using configuration::ElementTreeImpl;
    SetElement* pTunneledImpl = 0;

    Reference< XUnoTunnel > xElementTunnel;
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
    }
}
