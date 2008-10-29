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
// Inner Elements
//-----------------------------------------------------------------------------

        template <class NodeClass>
        class OInnerElement : public InnerElement, public NodeClass
        {
            static ServiceImplementationInfo const*const s_pServiceInfo;

            uno::XInterface*    m_pUnoThis;
            ApiTreeImpl&    m_rTree;
            configuration::NodeRef          m_aNode;
        public:
            inline OInnerElement(uno::XInterface*   pUnoThis,ApiTreeImpl& rTree, configuration::NodeRef const& aNode);
            inline ~OInnerElement();

            virtual inline configuration::NodeRef           doGetNode() const;
            virtual inline ApiTreeImpl&     getApiTree() const;

            virtual inline uno::XInterface*     doGetUnoInstance() const;
            virtual inline ServiceImplementationInfo const* doGetServiceInfo() const;

            static inline ServiceImplementationInfo const* getStaticServiceInfo();
        };

//-----------------------------------------------------------------------------
// Set Elements
//-----------------------------------------------------------------------------

        template <class NodeClass>
        class OSetElement : public SetElement, public NodeClass
        {
            static ServiceImplementationInfo const*const s_pServiceInfo;

            mutable ApiTreeImpl     m_aTree;
        public:
            OSetElement(uno::XInterface* pUnoThis, rtl::Reference< configuration::Tree > const& aTree, ApiTreeImpl& rParentTree)
            : m_aTree(pUnoThis, aTree,rParentTree)
            {}
            OSetElement(uno::XInterface* pUnoThis, rtl::Reference< configuration::Tree > const& aTree, ApiProvider& rProvider, ApiTreeImpl* pParentTree = 0)
            : m_aTree(pUnoThis, rProvider,aTree,pParentTree)
            {}

            virtual inline configuration::NodeRef           doGetNode() const;
            virtual inline ApiTreeImpl&     getApiTree() const;

            virtual inline uno::XInterface*     doGetUnoInstance() const;
            virtual inline ServiceImplementationInfo const* doGetServiceInfo() const;

            static inline ServiceImplementationInfo const* getStaticServiceInfo();
        };

//-----------------------------------------------------------------------------
// Root Elements
//-----------------------------------------------------------------------------

        template <class NodeClass>
        class OReadRootElement : public RootElement, public NodeClass
        {
            static ServiceImplementationInfo const*const s_pServiceInfo;
            mutable ApiRootTreeImpl     m_aRootTree;
        public:
            OReadRootElement(uno::XInterface* pUnoThis, ApiProvider& rProvider, rtl::Reference< configuration::Tree > const& aTree, vos::ORef< OOptions >const& _xOptions)
                : m_aRootTree(pUnoThis, rProvider,aTree, _xOptions)
            {}

            virtual inline configuration::NodeRef           doGetNode() const;
            virtual inline ApiTreeImpl&     getApiTree() const;
            virtual inline ApiRootTreeImpl& getRootTree();

            virtual inline uno::XInterface*     doGetUnoInstance() const;
            virtual inline ServiceImplementationInfo const* doGetServiceInfo() const;

            static inline ServiceImplementationInfo const* getStaticServiceInfo();
        };
    //-------------------------------------------------------------------------

        template <class NodeClass>
        class OUpdateRootElement : public UpdateRootElement, public NodeClass
        {
            static ServiceImplementationInfo const*const s_pServiceInfo;

            mutable ApiRootTreeImpl     m_aRootTree;
        public:
            OUpdateRootElement(uno::XInterface* pUnoThis, ApiProvider& rProvider, rtl::Reference< configuration::Tree > const& aTree, vos::ORef< OOptions >const& _xOptions)
            : m_aRootTree(pUnoThis, rProvider,aTree,_xOptions)
            {}

            virtual inline configuration::NodeRef           doGetNode() const;
            virtual inline ApiTreeImpl&     getApiTree() const;
            virtual inline ApiRootTreeImpl& getRootTree();

            virtual inline uno::XInterface*     doGetUnoInstance() const;
            virtual inline ServiceImplementationInfo const* doGetServiceInfo() const;

            static inline ServiceImplementationInfo const* getStaticServiceInfo();
        };
    }
}
//-----------------------------------------------------------------------------
#include "apiaccessobj.inl"
//-----------------------------------------------------------------------------

#endif // CONFIGMGR_API_ACCESSOBJECTS_HXX_
