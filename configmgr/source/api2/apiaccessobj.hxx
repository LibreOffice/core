/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: apiaccessobj.hxx,v $
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

#ifndef CONFIGMGR_API_ACCESSOBJECTS_HXX_
#define CONFIGMGR_API_ACCESSOBJECTS_HXX_

#include "apitreeaccess.hxx"
#include "apinodeaccess.hxx"
#include "apinodeupdate.hxx"

#include "apitreeimplobj.hxx"
#include "noderef.hxx"

#include "apiserviceinfo.hxx"

namespace configmgr
{
//-----------------------------------------------------------------------------
    namespace configapi
    {
//-----------------------------------------------------------------------------
        using configuration::Tree;
        using configuration::NodeRef;

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
            inline OInnerElement(UnoInterface*  pUnoThis,ApiTreeImpl& rTree, NodeRef const& aNode);
            inline ~OInnerElement();

            virtual inline NodeRef          doGetNode() const;
            virtual inline ApiTreeImpl&     getApiTree() const;

            virtual inline UnoInterface*        doGetUnoInstance() const;
            virtual inline ServiceInfo const*   doGetServiceInfo() const;

            static inline ServiceInfo const* getStaticServiceInfo();
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
            OSetElement(UnoInterface* pUnoThis, configuration::TreeRef const& aTree, ApiTreeImpl& rParentTree)
            : m_aTree(pUnoThis, aTree,rParentTree)
            {}
            OSetElement(UnoInterface* pUnoThis, configuration::TreeRef const& aTree, ApiProvider& rProvider, ApiTreeImpl* pParentTree = 0)
            : m_aTree(pUnoThis, rProvider,aTree,pParentTree)
            {}

            virtual inline NodeRef          doGetNode() const;
            virtual inline ApiTreeImpl&     getApiTree() const;

            virtual inline UnoInterface*        doGetUnoInstance() const;
            virtual inline ServiceInfo const*   doGetServiceInfo() const;

            static inline ServiceInfo const* getStaticServiceInfo();
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
            mutable ApiRootTreeImpl     m_aRootTree;
        public:
            OReadRootElement(UnoInterface* pUnoThis, ApiProvider& rProvider, configuration::Tree const& aTree, vos::ORef< OOptions >const& _xOptions)
                : m_aRootTree(pUnoThis, rProvider,aTree, _xOptions)
            {}

            virtual inline NodeRef          doGetNode() const;
            virtual inline ApiTreeImpl&     getApiTree() const;
            virtual inline ApiRootTreeImpl& getRootTree();

            virtual inline UnoInterface*        doGetUnoInstance() const;
            virtual inline ServiceInfo const*   doGetServiceInfo() const;

            static inline ServiceInfo const* getStaticServiceInfo();
        };
    //-------------------------------------------------------------------------

        template <class NodeClass>
        class OUpdateRootElement : public UpdateRootElement, public NodeClass
        {
            static ServiceInfo const*const s_pServiceInfo;

            mutable ApiRootTreeImpl     m_aRootTree;
        public:
            OUpdateRootElement(UnoInterface* pUnoThis, ApiProvider& rProvider, configuration::Tree const& aTree, vos::ORef< OOptions >const& _xOptions)
            : m_aRootTree(pUnoThis, rProvider,aTree,_xOptions)
            {}

            virtual inline NodeRef          doGetNode() const;
            virtual inline ApiTreeImpl&     getApiTree() const;
            virtual inline ApiRootTreeImpl& getRootTree();

            virtual inline UnoInterface*        doGetUnoInstance() const;
            virtual inline ServiceInfo const*   doGetServiceInfo() const;

            static inline ServiceInfo const* getStaticServiceInfo();
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
