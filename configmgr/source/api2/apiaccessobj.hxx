/*************************************************************************
 *
 *  $RCSfile: apiaccessobj.hxx,v $
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

#ifndef CONFIGMGR_API_ACCESSOBJECTS_HXX_
#define CONFIGMGR_API_ACCESSOBJECTS_HXX_

#include "apitreeaccess.hxx"
#include "apinodeaccess.hxx"
#include "apinodeupdate.hxx"

#include "apitreeimplobj.hxx"
#include "noderef.hxx"

namespace configmgr
{
//-----------------------------------------------------------------------------
    namespace configapi
    {
//-----------------------------------------------------------------------------
        using configuration::Tree;
        using configuration::NodeRef;

//-----------------------------------------------------------------------------
extern ServiceInfo const aRootElementUpdateAccessSI;
extern ServiceInfo const aRootElementAdminAccessSI;
extern ServiceInfo const aRootElementReadAccessSI;

//-----------------------------------------------------------------------------
// Inner Elements
//-----------------------------------------------------------------------------

        template <class NodeClass>
        class OInnerElement : public InnerElement, public NodeClass
        {
            static ServiceInfo const*const s_pServiceInfo;

            UnoInterface*   m_pUnoThis;
            ApiTreeImpl&    m_rTree;
            NodeRef         m_aNode;
        public:
            OInnerElement(UnoInterface* pUnoThis,ApiTreeImpl& rTree, NodeRef const& aNode)
            : m_pUnoThis(pUnoThis)
            , m_rTree(rTree)
            , m_aNode(aNode)
            {}

            virtual NodeRef             doGetNode() const;
            virtual ApiTreeImpl&        getApiTree() const;

            virtual UnoInterface*       doGetUnoInstance() const;
            virtual ServiceInfo const*  doGetServiceInfo() const;

            static ServiceInfo const* getStaticServiceInfo();
        };
    //-------------------------------------------------------------------------

        typedef OInnerElement<NodeGroupInfoAccess>  OInnerGroupInfoAccess;
        typedef OInnerElement<NodeGroupAccess>      OInnerGroupUpdateAccess;
        typedef OInnerElement<NodeSetInfoAccess>    OInnerSetInfoAccess;
        typedef OInnerElement<NodeTreeSetAccess>    OInnerTreeSetUpdateAccess;
        typedef OInnerElement<NodeValueSetAccess>   OInnerValueSetUpdateAccess;

//-----------------------------------------------------------------------------
// Set Elements
//-----------------------------------------------------------------------------

        template <class NodeClass>
        class OSetElement : public SetElement, public NodeClass
        {
            static ServiceInfo const*const s_pServiceInfo;

            mutable ApiTreeImpl     m_aTree;
        public:
            OSetElement(UnoInterface* pUnoThis, configuration::Tree const& aTree, ApiTreeImpl& rParentTree)
            : m_aTree(pUnoThis, aTree,rParentTree)
            {}
            OSetElement(UnoInterface* pUnoThis, configuration::Tree const& aTree, ApiProvider& rProvider, ApiTreeImpl* pParentTree = 0)
            : m_aTree(pUnoThis, rProvider,aTree,pParentTree)
            {}

            virtual NodeRef             doGetNode() const;
            virtual ApiTreeImpl&        getApiTree() const;

            virtual UnoInterface*       doGetUnoInstance() const;
            virtual ServiceInfo const*  doGetServiceInfo() const;

            static ServiceInfo const* getStaticServiceInfo();
        };

    // Set Elements
    //-------------------------------------------------------------------------

        typedef OSetElement<NodeGroupInfoAccess>    OSetElementGroupInfoAccess;
        typedef OSetElement<NodeGroupAccess>        OSetElementGroupUpdateAccess;
        typedef OSetElement<NodeSetInfoAccess>      OSetElementSetInfoAccess;
        typedef OSetElement<NodeTreeSetAccess>      OSetElementTreeSetUpdateAccess;
        typedef OSetElement<NodeValueSetAccess>     OSetElementValueSetUpdateAccess;

//-----------------------------------------------------------------------------
// Root Elements
//-----------------------------------------------------------------------------

        template <class NodeClass>
        class OReadRootElement : public RootElement, public NodeClass
        {
            static ServiceInfo const*const s_pServiceInfo;

            mutable ApiTreeImpl     m_aTree;
        public:
            OReadRootElement(UnoInterface* pUnoThis, ApiProvider& rProvider, configuration::Tree const& aTree)
            : m_aTree(pUnoThis, rProvider,aTree)
            {}

            virtual NodeRef             doGetNode() const;
            virtual ApiTreeImpl&        getApiTree() const;

            virtual UnoInterface*       doGetUnoInstance() const;
            virtual ServiceInfo const*  doGetServiceInfo() const;

            static ServiceInfo const* getStaticServiceInfo();
        };
    //-------------------------------------------------------------------------

        template <class NodeClass>
        class OUpdateRootElement : public UpdateRootElement, public NodeClass
        {
            static ServiceInfo const*const s_pServiceInfo;

            mutable ApiTreeImpl     m_aTree;
        public:
            OUpdateRootElement(UnoInterface* pUnoThis, ApiProvider& rProvider, configuration::Tree const& aTree)
            : m_aTree(pUnoThis, rProvider,aTree)
            {}

            virtual NodeRef             doGetNode() const;
            virtual ApiTreeImpl&        getApiTree() const;

            virtual UnoInterface*       doGetUnoInstance() const;
            virtual ServiceInfo const*  doGetServiceInfo() const;

            static ServiceInfo const* getStaticServiceInfo();
        };

    // Root Elements
    //-------------------------------------------------------------------------

        typedef OReadRootElement<NodeGroupInfoAccess>   ORootElementGroupInfoAccess;
        typedef OUpdateRootElement<NodeGroupAccess>     ORootElementGroupUpdateAccess;
        typedef OReadRootElement<NodeSetInfoAccess>     ORootElementSetInfoAccess;
        typedef OUpdateRootElement<NodeTreeSetAccess>   ORootElementTreeSetUpdateAccess;
        typedef OUpdateRootElement<NodeValueSetAccess>  ORootElementValueSetUpdateAccess;

//-----------------------------------------------------------------------------
    }
}
//-----------------------------------------------------------------------------
#include "apiaccessobj.inl"
//-----------------------------------------------------------------------------

#endif // CONFIGMGR_API_ACCESSOBJECTS_HXX_
