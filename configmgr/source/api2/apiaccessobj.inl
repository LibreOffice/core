/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: apiaccessobj.inl,v $
 * $Revision: 1.6 $
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

namespace configmgr
{
//-----------------------------------------------------------------------------
    namespace configapi
    {

//========================================================================
//= member functions
//========================================================================

//-----------------------------------------------------------------------------
// Inner Elements
//-----------------------------------------------------------------------------

template <class NodeClass>
OInnerElement<NodeClass>::OInnerElement(uno::XInterface*    pUnoThis,ApiTreeImpl& rTree, configuration::NodeRef const& aNode)
: m_pUnoThis(pUnoThis)
, m_rTree(rTree)
, m_aNode(aNode)
{
    m_rTree.setNodeInstance(aNode,pUnoThis);
    m_rTree.getUnoInstance()->acquire();
}

template <class NodeClass>
OInnerElement<NodeClass>::~OInnerElement()
{
    m_rTree.getUnoInstance()->release();
}
//-----------------------------------------------------------------------------

template <class NodeClass>
configuration::NodeRef OInnerElement<NodeClass>::doGetNode() const
{
    return m_aNode;
}
//-----------------------------------------------------------------------------

template <class NodeClass>
ApiTreeImpl& OInnerElement<NodeClass>::getApiTree() const
{
    return m_rTree;
}
//-----------------------------------------------------------------------------

template <class NodeClass>
uno::XInterface* OInnerElement<NodeClass>::doGetUnoInstance() const
{
    return m_pUnoThis;
}
//-----------------------------------------------------------------------------

template <class NodeClass>
ServiceImplementationInfo const* OInnerElement<NodeClass>::doGetServiceInfo() const
{
    return s_pServiceInfo;
}
//-----------------------------------------------------------------------------

template <class NodeClass>
ServiceImplementationInfo const* OInnerElement<NodeClass>::getStaticServiceInfo()
{
    return s_pServiceInfo;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Set Elements
//-----------------------------------------------------------------------------

template <class NodeClass>
configuration::NodeRef OSetElement<NodeClass>::doGetNode() const
{
    return m_aTree.getTree()->getRootNode();
}
//-----------------------------------------------------------------------------

template <class NodeClass>
ApiTreeImpl& OSetElement<NodeClass>::getApiTree() const
{
    return m_aTree;
}
//-----------------------------------------------------------------------------

template <class NodeClass>
uno::XInterface* OSetElement<NodeClass>::doGetUnoInstance() const
{
    return m_aTree.getUnoInstance();
}
//-----------------------------------------------------------------------------

template <class NodeClass>
ServiceImplementationInfo const* OSetElement<NodeClass>::doGetServiceInfo() const
{
    return s_pServiceInfo;
}
//-----------------------------------------------------------------------------

template <class NodeClass>
ServiceImplementationInfo const* OSetElement<NodeClass>::getStaticServiceInfo()
{
    return s_pServiceInfo;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Read-only Root Elements
//-----------------------------------------------------------------------------

template <class NodeClass>
configuration::NodeRef OReadRootElement<NodeClass>::doGetNode() const
{
    return m_aRootTree.getApiTree().getTree()->getRootNode();
}
//-----------------------------------------------------------------------------

template <class NodeClass>
ApiTreeImpl& OReadRootElement<NodeClass>::getApiTree() const
{
    return m_aRootTree.getApiTree();
}
//-----------------------------------------------------------------------------

template <class NodeClass>
ApiRootTreeImpl& OReadRootElement<NodeClass>::getRootTree()
{
    return m_aRootTree;
}
//-----------------------------------------------------------------------------

template <class NodeClass>
uno::XInterface* OReadRootElement<NodeClass>::doGetUnoInstance() const
{
    return m_aRootTree.getApiTree().getUnoInstance();
}
//-----------------------------------------------------------------------------

template <class NodeClass>
ServiceImplementationInfo const* OReadRootElement<NodeClass>::doGetServiceInfo() const
{
    return s_pServiceInfo;
}
//-----------------------------------------------------------------------------

template <class NodeClass>
ServiceImplementationInfo const* OReadRootElement<NodeClass>::getStaticServiceInfo()
{
    return s_pServiceInfo;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Update Root Elements
//-----------------------------------------------------------------------------

template <class NodeClass>
configuration::NodeRef OUpdateRootElement<NodeClass>::doGetNode() const
{
    return m_aRootTree.getApiTree().getTree()->getRootNode();
}
//-----------------------------------------------------------------------------

template <class NodeClass>
ApiTreeImpl& OUpdateRootElement<NodeClass>::getApiTree() const
{
    return m_aRootTree.getApiTree();
}
//-----------------------------------------------------------------------------

template <class NodeClass>
ApiRootTreeImpl& OUpdateRootElement<NodeClass>::getRootTree()
{
    return m_aRootTree;
}
//-----------------------------------------------------------------------------

template <class NodeClass>
uno::XInterface* OUpdateRootElement<NodeClass>::doGetUnoInstance() const
{
    return m_aRootTree.getApiTree().getUnoInstance();
}
//-----------------------------------------------------------------------------

template <class NodeClass>
ServiceImplementationInfo const* OUpdateRootElement<NodeClass>::doGetServiceInfo() const
{
    return s_pServiceInfo;
}
//-----------------------------------------------------------------------------

template <class NodeClass>
ServiceImplementationInfo const* OUpdateRootElement<NodeClass>::getStaticServiceInfo()
{
    return s_pServiceInfo;
}
//-----------------------------------------------------------------------------

//========================================================================
//= Instantiations
//========================================================================
/*
//-----------------------------------------------------------------------------
// Inner Elements
//-----------------------------------------------------------------------------

template class OInnerElement<NodeGroupInfoAccess>;  // OInnerGroupInfoAccess
template class OInnerElement<NodeGroupAccess>;      // OInnerGroupUpdateAccess
template class OInnerElement<NodeSetInfoAccess>;    // OInnerSetInfoAccess
template class OInnerElement<NodeTreeSetAccess>;    // OInnerTreeSetUpdateAccess
template class OInnerElement<NodeValueSetAccess>;   // OInnerValueSetUpdateAccess

//-----------------------------------------------------------------------------
// Set Elements
//-----------------------------------------------------------------------------
template class OSetElement<NodeGroupInfoAccess>;    // OSetElementGroupInfoAccess
template class OSetElement<NodeGroupAccess>;        // OSetElementGroupUpdateAccess
template class OSetElement<NodeSetInfoAccess>;      // OSetElementSetInfoAccess
template class OSetElement<NodeTreeSetAccess>;      // OSetElementTreeSetUpdateAccess
template class OSetElement<NodeValueSetAccess>;     // OSetElementValueSetUpdateAccess

//-----------------------------------------------------------------------------
// Root Elements
//-----------------------------------------------------------------------------

template class OReadRootElement<NodeGroupInfoAccess>;   // ORootElementGroupInfoAccess
template class OUpdateRootElement<NodeGroupAccess>;     // ORootElementGroupUpdateAccess
template class OReadRootElement<NodeSetInfoAccess>;     // ORootElementSetInfoAccess
template class OUpdateRootElement<NodeTreeSetAccess>;   // ORootElementTreeSetUpdateAccess
template class OUpdateRootElement<NodeValueSetAccess>;  // ORootElementValueSetUpdateAccess
*/
//-----------------------------------------------------------------------------
    }
}
