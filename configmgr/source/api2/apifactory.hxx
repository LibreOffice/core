/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: apifactory.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 14:02:34 $
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

#ifndef CONFIGMGR_API_FACTORY_HXX_
#define CONFIGMGR_API_FACTORY_HXX_

#ifndef CONFIGMGR_MISC_OPTIONS_HXX_
#include "options.hxx"
#endif
#ifndef CONFIGMGR_UTILITY_HXX_
#include "utility.hxx"
#endif

#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif
#ifndef _VOS_REF_HXX_
#include <vos/ref.hxx>
#endif

#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif

namespace configmgr
{
    namespace configuration
    {
        class Name;
        class NodeID;
        class NodeRef;
        class Tree;
        class ElementRef;
        class ElementTree;

        class Template;
        typedef rtl::Reference<Template> TemplateHolder;
    }
    namespace configapi
    {
        namespace uno = com::sun::star::uno;
        typedef uno::XInterface UnoInterface;
        typedef uno::Reference< uno::XInterface > UnoInterfaceRef;
        typedef uno::Any UnoAny;

        class ApiTreeImpl;
        class NodeElement;
        class InnerElement;
        class TreeElement;
        class SetElement;
        class RootElement;

    // used to register objects
        class ObjectRegistry;
        typedef rtl::Reference<ObjectRegistry> ObjectRegistryHolder;
        typedef cppu::OImplementationId UnoTunnelID;

    // used to create UNO objects
        class Factory : Noncopyable
        {
            ObjectRegistryHolder m_pRegistry;
            UnoTunnelID const m_aTunnelID;

        private:
            /// return the element _without_ acquiring it
            NodeElement*    implFind(configuration::NodeID const& aNode);

        public:
            Factory(ObjectRegistryHolder pRegistry);
            virtual ~Factory();

            UnoInterfaceRef makeUnoElement(configuration::Tree const& aTree, configuration::NodeRef const& aNode);
            UnoInterfaceRef findUnoElement(configuration::NodeID const& aNode);

            UnoInterfaceRef makeUnoGroupMember(configuration::Tree const& aTree, configuration::NodeRef const& aNode);
            UnoInterfaceRef makeUnoSetElement(configuration::ElementTree const& aTree);

            NodeElement* makeElement(configuration::Tree const& aTree, configuration::NodeRef const& aNode);
            NodeElement* findElement(configuration::NodeID const& aNode);

            NodeElement*    makeGroupMember(configuration::Tree const& aTree, configuration::NodeRef const& aNode);
            TreeElement*    makeAccessRoot(configuration::Tree const& aTree, RequestOptions const& _aOptions);
            SetElement*     makeSetElement(configuration::ElementTree const& aTree);

            SetElement*     findSetElement(configuration::ElementRef const& aElement);

            /// check for the existence of an element
            sal_Bool        hasElement(configuration::NodeID const& _rNode) { return NULL != implFind(_rNode); }

            void    revokeElement(configuration::NodeID const& aNode);
            void    revokeElement(configuration::NodeID const& aNode, NodeElement& rElement);

            SetElement* extractSetElement(UnoAny const& aElement);
            bool tunnelSetElement(sal_Int64& nSomething, SetElement& rElement, uno::Sequence< sal_Int8 > const& aTunnelID);
            // registry operations

            static ApiTreeImpl const* findDescendantTreeImpl(configuration::NodeID const& aNode, ApiTreeImpl const* pImpl);
        protected:
            void            doRegisterElement(configuration::NodeID const& aNode, NodeElement* pElement);
            void            doRevokeElement(configuration::NodeID const& aNode, NodeElement* pElement);

            uno::Sequence< sal_Int8 > doGetElementTunnelID() const { return m_aTunnelID.getImplementationId(); }

            virtual NodeElement*    doCreateGroupMember(configuration::Tree const& aTree, configuration::NodeRef const& aNode, configuration::Template* pSetElementTemplate) = 0;
            virtual TreeElement*    doCreateAccessRoot(configuration::Tree const& aTree, configuration::Template* pSetElementTemplate, vos::ORef< OOptions >const& _xOptions) = 0;
            virtual SetElement*     doCreateSetElement(configuration::ElementTree const& aTree, configuration::Template* pSetElementTemplate) = 0;

            static ApiTreeImpl& getImplementation(NodeElement& pElement);
        private:
            static configuration::TemplateHolder implGetSetElementTemplate(configuration::Tree const& aTree, configuration::NodeRef const& aNode);
            static UnoInterfaceRef implToUno(NodeElement* pNode);
            void implHaveNewElement(configuration::NodeID aNodeID, NodeElement* pNode);
        };

    }
}

#endif // CONFIGMGR_API_FACTORY_HXX_
