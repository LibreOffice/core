/*************************************************************************
 *
 *  $RCSfile: apifactory.hxx,v $
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

#ifndef CONFIGMGR_API_FACTORY_HXX_
#define CONFIGMGR_API_FACTORY_HXX_

#include "apitypes.hxx"
#include <vos/ref.hxx>
#include <cppuhelper/typeprovider.hxx>

namespace osl { class Mutex; }
namespace configmgr
{
    namespace configuration
    {
        class Name;
        class NodeID;
        class NodeRef;
        class Tree;
        class ElementTree;

        class Template;
        typedef vos::ORef<Template> TemplateHolder;
    }
    namespace configapi
    {
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
        typedef vos::ORef<ObjectRegistry> ObjectRegistryHolder;
        typedef cppu::OImplementationId UnoTunnelID;

    // used to create UNO objects
        class Factory : NotCopyable
        {
            ObjectRegistryHolder m_pRegistry;
            UnoTunnelID const m_aTunnelID;
        public:
            Factory(ObjectRegistryHolder pRegistry);
            virtual ~Factory();

            UnoInterfaceRef makeUnoElement(configuration::Tree const& aTree, configuration::NodeRef const& aNode);
            UnoInterfaceRef findUnoElement(configuration::NodeID const& aNode);

            UnoInterfaceRef makeUnoAccessRoot(configuration::Tree const& aTree);
            UnoInterfaceRef makeUnoGroupMember(configuration::Tree const& aTree, configuration::NodeRef const& aNode);
            UnoInterfaceRef makeUnoSetElement(configuration::ElementTree const& aTree);

            NodeElement* makeElement(configuration::Tree const& aTree, configuration::NodeRef const& aNode);
            NodeElement* findElement(configuration::NodeID const& aNode);

            NodeElement*    makeGroupMember(configuration::Tree const& aTree, configuration::NodeRef const& aNode);
            NodeElement*    makeAccessRoot(configuration::Tree const& aTree);
            NodeElement*    makeSetElement(configuration::ElementTree const& aTree);

            void    registerElement(configuration::NodeID const& aNode, NodeElement& rElement);
            void    revokeElement(configuration::NodeID const& aNode);
            void    revokeElement(configuration::NodeID const& aNode, NodeElement& rElement);

            SetElement* extractSetElement(UnoAny const& aElement);
            bool tunnelSetElement(sal_Int64& nSomething, SetElement& rElement, uno::Sequence< sal_Int8 > const& aTunnelID);
            // registry operations
        protected:
            NodeElement*    doFindElement(configuration::NodeID const& aNode);
            void            doRegisterElement(configuration::NodeID const& aNode, NodeElement* pElement);
            void            doRevokeElement(configuration::NodeID const& aNode, NodeElement* pElement);

            uno::Sequence< sal_Int8 > doGetElementTunnelID() const { return m_aTunnelID.getImplementationId(); }

            virtual NodeElement*    doCreateGroupMember(configuration::Tree const& aTree, configuration::NodeRef const& aNode, configuration::Template* pSetElementTemplate) = 0;
            virtual NodeElement*    doCreateAccessRoot(configuration::Tree const& aTree, configuration::Template* pSetElementTemplate) = 0;
            virtual NodeElement*    doCreateSetElement(configuration::ElementTree const& aTree, configuration::Template* pSetElementTemplate) = 0;

            static ApiTreeImpl& getImplementation(NodeElement& pElement);
        private:
            osl::Mutex&     doGetMutex();

            static configuration::TemplateHolder implGetSetElementTemplate(configuration::Tree const& aTree, configuration::NodeRef const& aNode);
            static UnoInterfaceRef implToUno(NodeElement* pNode);
            void implHaveNewElement(configuration::NodeID aNodeID, NodeElement* pNode);
        };

    }
}

#endif // CONFIGMGR_API_FACTORY_HXX_
