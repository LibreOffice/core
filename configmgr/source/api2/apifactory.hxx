/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: apifactory.hxx,v $
 * $Revision: 1.11 $
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

#ifndef CONFIGMGR_API_FACTORY_HXX_
#define CONFIGMGR_API_FACTORY_HXX_

#include "options.hxx"
#include "utility.hxx"
#include <boost/utility.hpp>
#include <rtl/ref.hxx>
#include <vos/ref.hxx>
#include <cppuhelper/typeprovider.hxx>

namespace configmgr
{
    namespace configuration
    {
        class NodeID;
        class NodeRef;
        class ElementTree;
        class Template;
        class Tree;
    }
    namespace configapi
    {
        namespace uno = com::sun::star::uno;

        class ApiTreeImpl;
        class NodeElement;
        class InnerElement;
        class TreeElement;
        class SetElement;
        class RootElement;

    // used to register objects
        class ObjectRegistry;

    // used to create UNO objects
        class Factory : private boost::noncopyable
        {
            rtl::Reference<ObjectRegistry> m_pRegistry;
            cppu::OImplementationId const m_aTunnelID;

        private:
            /// return the element _without_ acquiring it
            NodeElement*    implFind(configuration::NodeID const& aNode);

        public:
            Factory(rtl::Reference<ObjectRegistry> pRegistry);
            virtual ~Factory();

            uno::Reference< uno::XInterface > makeUnoElement(rtl::Reference< configuration::Tree > const& aTree, configuration::NodeRef const& aNode);
            uno::Reference< uno::XInterface > findUnoElement(configuration::NodeID const& aNode);

            uno::Reference< uno::XInterface > makeUnoGroupMember(rtl::Reference< configuration::Tree > const& aTree, configuration::NodeRef const& aNode);
            uno::Reference< uno::XInterface > makeUnoSetElement(rtl::Reference< configuration::ElementTree > const& aTree);

            NodeElement* makeElement(rtl::Reference< configuration::Tree > const& aTree, configuration::NodeRef const& aNode);
            NodeElement* findElement(configuration::NodeID const& aNode);

            NodeElement*    makeGroupMember(rtl::Reference< configuration::Tree > const& aTree, configuration::NodeRef const& aNode);
            TreeElement*    makeAccessRoot(rtl::Reference< configuration::Tree > const& aTree, RequestOptions const& _aOptions);
            SetElement*     makeSetElement(rtl::Reference< configuration::ElementTree > const& aTree);

            SetElement*     findSetElement(rtl::Reference< configuration::ElementTree > const& aElement);

            /// check for the existence of an element
            sal_Bool        hasElement(configuration::NodeID const& _rNode) { return NULL != implFind(_rNode); }

            void    revokeElement(configuration::NodeID const& aNode);

            SetElement* extractSetElement(uno::Any const& aElement);
            bool tunnelSetElement(sal_Int64& nSomething, SetElement& rElement, uno::Sequence< sal_Int8 > const& aTunnelID);
            // registry operations

            static ApiTreeImpl const* findDescendantTreeImpl(configuration::NodeID const& aNode, ApiTreeImpl const* pImpl);
        protected:
            void            doRegisterElement(configuration::NodeID const& aNode, NodeElement* pElement);
            void            doRevokeElement(configuration::NodeID const& aNode, NodeElement* pElement);

            uno::Sequence< sal_Int8 > doGetElementTunnelID() const { return m_aTunnelID.getImplementationId(); }

            virtual NodeElement*    doCreateGroupMember(rtl::Reference< configuration::Tree > const& aTree, configuration::NodeRef const& aNode, configuration::Template* pSetElementTemplate) = 0;
            virtual TreeElement*    doCreateAccessRoot(rtl::Reference< configuration::Tree > const& aTree, configuration::Template* pSetElementTemplate, vos::ORef< OOptions >const& _xOptions) = 0;
            virtual SetElement*     doCreateSetElement(rtl::Reference< configuration::ElementTree > const& aTree, configuration::Template* pSetElementTemplate) = 0;

            static ApiTreeImpl& getImplementation(NodeElement& pElement);
        private:
            static rtl::Reference<configuration::Template> implGetSetElementTemplate(rtl::Reference< configuration::Tree > const& aTree, configuration::NodeRef const& aNode);
            static uno::Reference< uno::XInterface > implToUno(NodeElement* pNode);
            void implHaveNewElement(configuration::NodeID aNodeID, NodeElement* pNode);
        };

    }
}

#endif // CONFIGMGR_API_FACTORY_HXX_
