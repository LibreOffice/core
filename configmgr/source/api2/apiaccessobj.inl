/*************************************************************************
 *
 *  $RCSfile: apiaccessobj.inl,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: jb $ $Date: 2000-11-07 14:37:25 $
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
ServiceInfo const* OInnerElement<NodeClass>::doGetServiceInfo() const
{
    return s_pServiceInfo;
}
//-----------------------------------------------------------------------------

template <class NodeClass>
ServiceInfo const* OInnerElement<NodeClass>::getStaticServiceInfo()
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
ServiceInfo const* OSetElement<NodeClass>::doGetServiceInfo() const
{
    return s_pServiceInfo;
}
//-----------------------------------------------------------------------------

template <class NodeClass>
ServiceInfo const* OSetElement<NodeClass>::getStaticServiceInfo()
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
    return m_aTree.getTree().getRootNode();
}
//-----------------------------------------------------------------------------

template <class NodeClass>
ApiTreeImpl& OReadRootElement<NodeClass>::getApiTree() const
{
    return m_aTree;
}
//-----------------------------------------------------------------------------

template <class NodeClass>
UnoInterface* OReadRootElement<NodeClass>::doGetUnoInstance() const
{
    return m_aTree.getUnoInstance();
}
//-----------------------------------------------------------------------------

template <class NodeClass>
ServiceInfo const* OReadRootElement<NodeClass>::doGetServiceInfo() const
{
    return s_pServiceInfo;
}
//-----------------------------------------------------------------------------

template <class NodeClass>
ServiceInfo const* OReadRootElement<NodeClass>::getStaticServiceInfo()
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
    return m_aTree.getTree().getRootNode();
}
//-----------------------------------------------------------------------------

template <class NodeClass>
ApiTreeImpl& OUpdateRootElement<NodeClass>::getApiTree() const
{
    return m_aTree;
}
//-----------------------------------------------------------------------------

template <class NodeClass>
UnoInterface* OUpdateRootElement<NodeClass>::doGetUnoInstance() const
{
    return m_aTree.getUnoInstance();
}
//-----------------------------------------------------------------------------

template <class NodeClass>
ServiceInfo const* OUpdateRootElement<NodeClass>::doGetServiceInfo() const
{
    return s_pServiceInfo;
}
//-----------------------------------------------------------------------------

template <class NodeClass>
ServiceInfo const* OUpdateRootElement<NodeClass>::getStaticServiceInfo()
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
