/*************************************************************************
 *
 *  $RCSfile: configset.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: dg $ $Date: 2000-11-10 22:41:30 $
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

#ifndef CONFIGMGR_CONFIGSET_HXX_
#define CONFIGMGR_CONFIGSET_HXX_

#include "apitypes.hxx"
#include "configexcept.hxx"
#include "noderef.hxx"
#include "template.hxx"

#include <vos/ref.hxx>
#include <stl/memory>

namespace com { namespace sun { namespace star {
    namespace script { class XTypeConverter; }
} } }

namespace configmgr
{
    class IRefCountedTemplateProvider;
    class INode;

    namespace configuration
    {
//-----------------------------------------------------------------------------

        //---------------------------------------------------------------------
        class Name;
        class AbsolutePath;
        //---------------------------------------------------------------------

        typedef com::sun::star::uno::Type       UnoType;
        typedef com::sun::star::uno::Any        UnoAny;
        typedef com::sun::star::uno::Reference<com::sun::star::script::XTypeConverter>  UnoTypeConverter;
        //---------------------------------------------------------------------

        class NodeRef;
        class Tree;
        class ElementTreeImpl;
        typedef vos::ORef<ElementTreeImpl> ElementTreeHolder;

        class NodeChange;
        class Template;
        typedef vos::ORef<Template> TemplateHolder;
//-----------------------------------------------------------------------------

        struct TemplateProvider
        {
            typedef vos::ORef<IRefCountedTemplateProvider> Holder;
            Holder m_aProvider;
        public:
            explicit TemplateProvider(IRefCountedTemplateProvider* pProvider);
            explicit TemplateProvider(Holder const& aProvider);
            TemplateProvider(TemplateProvider const& aOther);
            ~TemplateProvider();

            Holder const& get() const { return m_aProvider; }
            IRefCountedTemplateProvider* getBodyPtr()   const { return m_aProvider.getBodyPtr(); }
            IRefCountedTemplateProvider& getBody()      const { return m_aProvider.getBody(); }
            IRefCountedTemplateProvider* operator->()   const { return m_aProvider.operator->(); }
            IRefCountedTemplateProvider& operator*()    const { return m_aProvider.operator *(); }
        };
//-----------------------------------------------------------------------------
        /// provides information about a <type>Node</type> that is a element of a Container ("set").
        class ElementTree
        {
            ElementTreeHolder m_aTreeHolder;
        public:
            explicit ElementTree(ElementTreeImpl* pTree);
            explicit ElementTree(ElementTreeHolder const& pTree);
            ElementTree(ElementTree const& aOther);
            ElementTree& operator=(ElementTree const& aOther);
            ~ElementTree();

            bool isValid() const;
            ElementTreeHolder get() const;
            ElementTreeImpl* getImpl() const;

            TemplateHolder getTemplate() const;

            ElementTreeImpl* operator->() const;
            ElementTreeImpl& operator*() const;

            Tree getTree() const;
            ISynchronizedData* getTreeLock() const;

            static ElementTree extract(Tree const& aTree);
            /** if the element tree owns it's node tree, ownership is given to the caller.
                <p>WARNING: Irresponsible use of this feature produces crashes</p>
            */
            static void releaseOwnedElement(std::auto_ptr<INode>& rNewOwner, ElementTree const& aElementTree);
            /** if the element tree owns it's node tree, ownership is given to the caller
                and the root of the tree is assigned the given name.
                <p>WARNING: Irresponsible use of this feature produces crashes</p>
            */
            static void releaseOwnedElementAs(std::auto_ptr<INode>& rNewOwner, ElementTree const& aElementTree, Name const& aNewName);
        };
//-----------------------------------------------------------------------------

        /// provides information about the elements of a <type>Node</type> that is a Container ("set").
        class SetElementInfo
        {
            TemplateHolder  m_aTemplate;
        public:
            explicit SetElementInfo(TemplateHolder const& aTemplate);
            explicit SetElementInfo(UnoType const& aElementType);

            TemplateHolder getTemplate() const;

            UnoType  getElementType() const;
            Name     getTemplateName() const;
            Name     getTemplatePackage() const;
            RelativePath getTemplatePath() const;

            static TemplateHolder extractElementInfo(Tree const& aTree, NodeRef const& aNode);
        };
//-----------------------------------------------------------------------------

        class SetElementFactory
        {
            TemplateProvider m_aProvider;
        public:
            explicit SetElementFactory(TemplateProvider const& aProvider);
            SetElementFactory(SetElementFactory const& aOther);
            SetElementFactory& operator=(SetElementFactory const& aOther);
            ~SetElementFactory();

            ElementTree instantiateTemplate(TemplateHolder const& aTemplate);
            ElementTree instantiateOnDefault(std::auto_ptr<INode> aTree, TemplateHolder const& aDummyTemplate);
        };

//-----------------------------------------------------------------------------

        /// allows to insert,remove and replace an element of a <type>Node</type> that is a Container ("set") of full-fledged trees.
        class TreeSetUpdater
        {
            Tree    m_aParentTree;
            NodeRef m_aSetNode;
            TemplateHolder  m_aTemplate;
        public:
            TreeSetUpdater(Tree const& aParentTree, NodeRef const& aSetNode, SetElementInfo const& aInfo);

            NodeChange validateInsertElement (Name const& aName, ElementTree const& aNewElement);

            NodeChange validateReplaceElement(Tree const& aElementTree, NodeRef const& aElementNode, ElementTree const& aNewElement);

            NodeChange validateRemoveElement (Tree const& aElementTree, NodeRef const& aElementNode);
        private:
            void        implValidateSet();
            Name        implValidateElement(Tree const& aTree, NodeRef const& aNode);
            void        implValidateTree(ElementTree const& aElementTree);
        };
//-----------------------------------------------------------------------------
        /// allows to insert,remove and replace an element of a <type>Node</type> that is a Container ("set") of simple values.
        class ValueSetUpdater
        {
            Tree    m_aParentTree;
            NodeRef m_aSetNode;
            TemplateHolder  m_aTemplate;
            UnoTypeConverter m_xTypeConverter;
        public:
            ValueSetUpdater(Tree const& aParentTree, NodeRef const& aSetNode,
                            SetElementInfo const& aInfo, UnoTypeConverter const& xConverter);

            NodeChange validateInsertElement (Name const& aName, UnoAny const& aNewValue);

            NodeChange validateReplaceElement(Tree const& aElementTree, NodeRef const& aElementNode, UnoAny const& aNewValue);

            NodeChange validateRemoveElement (Tree const& aElementTree, NodeRef const& aElementNode);
        private:
            void implValidateSet();
            Name implValidateElement(Tree const& aTree, NodeRef const& aNode);
            UnoAny implValidateValue(NodeRef const& aElementNode, UnoAny const& aValue);
            UnoAny implValidateValue(UnoAny const& aValue);

            ElementTreeHolder makeValueElement(Name const& aName, NodeRef const& aElementNode, UnoAny const& aValue);
            ElementTreeHolder makeValueElement(Name const& aName, UnoAny const& aValue);
        };
//-----------------------------------------------------------------------------
    }
}

#endif // CONFIGMGR_CONFIGSET_HXX_
