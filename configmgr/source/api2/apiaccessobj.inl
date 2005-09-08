/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: apiaccessobj.inl,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 03:05:44 $
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
OInnerElement<NodeClass>::OInnerElement(UnoInterface*   pUnoThis,ApiTreeImpl& rTree, NodeRef const& aNode)
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
NodeRef OInnerElement<NodeClass>::doGetNode() const
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
UnoInterface* OInnerElement<NodeClass>::doGetUnoInstance() const
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
NodeRef OSetElement<NodeClass>::doGetNode() const
{
    return m_aTree.getTree().getRootNode();
}
//-----------------------------------------------------------------------------

template <class NodeClass>
ApiTreeImpl& OSetElement<NodeClass>::getApiTree() const
{
    return m_aTree;
}
//-----------------------------------------------------------------------------

template <class NodeClass>
UnoInterface* OSetElement<NodeClass>::doGetUnoInstance() const
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
NodeRef OReadRootElement<NodeClass>::doGetNode() const
{
    return m_aRootTree.getApiTree().getTree().getRootNode();
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
UnoInterface* OReadRootElement<NodeClass>::doGetUnoInstance() const
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
NodeRef OUpdateRootElement<NodeClass>::doGetNode() const
{
    return m_aRootTree.getApiTree().getTree().getRootNode();
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
UnoInterface* OUpdateRootElement<NodeClass>::doGetUnoInstance() const
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
