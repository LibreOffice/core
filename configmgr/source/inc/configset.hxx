/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: configset.hxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 14:17:27 $
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

#ifndef CONFIGMGR_CONFIGSET_HXX_
#define CONFIGMGR_CONFIGSET_HXX_

#ifndef CONFIGMGR_CONFIGEXCEPT_HXX_
#include "configexcept.hxx"
#endif
#ifndef CONFIGMGR_CONFIG_DEFAULTPROVIDER_HXX_
#include "configdefaultprovider.hxx"
#endif
#ifndef CONFIGMGR_CONFIGTEMPLATE_HXX_
#include "template.hxx"
#endif
#ifndef CONFIGMGR_CONFIGNODE_HXX_
#include "noderef.hxx"
#endif

#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif

#ifndef INCLUDED_MEMORY
#include <memory>
#define INCLUDED_MEMORY
#endif

namespace com { namespace sun { namespace star {
    namespace script { class XTypeConverter; }
} } }

namespace configmgr
{
    namespace data { class TreeSegment; }

    namespace configuration
    {
//-----------------------------------------------------------------------------

        class ElementRef;
        class ElementTree;
        //---------------------------------------------------------------------
        class Name;
        class AbsolutePath;
        namespace Path { class Component; }
        //---------------------------------------------------------------------

        typedef com::sun::star::uno::Type       UnoType;
        typedef com::sun::star::uno::Any        UnoAny;
        typedef com::sun::star::uno::Reference<com::sun::star::script::XTypeConverter>  UnoTypeConverter;
        //---------------------------------------------------------------------

        class ElementTreeImpl;
        typedef rtl::Reference<ElementTreeImpl> ElementTreeHolder;

        class NodeChange;
        class Template;
        typedef rtl::Reference<Template> TemplateHolder;
//-----------------------------------------------------------------------------
        /// identifies a <type>Node</type> that is a element of a Container ("set").
        class ElementRef
        {
            ElementTreeHolder   m_aTreeHolder;
        public:
            explicit ElementRef(ElementTreeImpl* pTree);
            ElementRef(ElementRef const& aOther);
            ElementRef& operator=(ElementRef const& aOther);
            ~ElementRef();

            bool isValid() const;

            Path::Component getFullName() const;
            Name            getName() const;

            TemplateHolder getTemplate() const;

            ElementTree getElementTree() const;

            TreeRef getTreeRef() const;

            static ElementRef extract(TreeRef const& aTree);
        };
//-----------------------------------------------------------------------------

        /// provides information about a <type>Node</type> that is a element of a Container ("set").
        class ElementTree
        {
            ElementTreeHolder m_aTreeHolder;
        public:
            static ElementTree emptyElement() { return ElementTree(); }

            ElementTree(ElementTreeImpl* pTree);
            ElementTree(ElementTreeHolder const& pTree);
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

            static ElementTree extract(Tree const& aTree);

            /** if the element tree owns it's node tree, access to the segment is given to the caller.
            */
            static data::TreeSegment getOwnedElement(ElementTree const& aElementTree);
        private:
            ElementTree();
        };
//-----------------------------------------------------------------------------

        /// provides information about the elements of a <type>Node</type> that is a Container ("set").
        class TemplateInfo
        {
            TemplateHolder  m_aTemplate;
        public:
            explicit TemplateInfo(TemplateHolder const& aTemplate);

            TemplateHolder getTemplate() const;

            UnoType  getType() const;
            Name     getTemplateName() const;
            Name     getTemplatePackage() const;
            OUString getTemplatePathString() const;
        };
//-----------------------------------------------------------------------------

        /// provides information about the elements of a <type>Node</type> that is a Container ("set").
        class SetElementInfo
        {
            TemplateInfo    m_aTemplateInfo;
        public:
            SetElementInfo(TemplateHolder const& aTemplate);

            TemplateHolder getTemplate()        const;
            TemplateInfo   getTemplateInfo()    const;

            UnoType  getElementType() const { return m_aTemplateInfo.getType(); }

            static TemplateHolder extractElementInfo(Tree const& aTree, NodeRef const& aNode);
        };
//-----------------------------------------------------------------------------

        class SetElementFactory
        {
            TemplateProvider m_aProvider;
        public:
            SetElementFactory(TemplateProvider const& aProvider);
            SetElementFactory(SetElementFactory const& aOther);
            SetElementFactory& operator=(SetElementFactory const& aOther);
            ~SetElementFactory();

            ElementTree instantiateTemplate(TemplateHolder const& aTemplate);
            ElementTree instantiateOnDefault(data::TreeSegment const& _aTree, TemplateHolder const& aDummyTemplate);

            static TemplateProvider findTemplateProvider(Tree const& aTree, NodeRef const& aNode);
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

            NodeChange validateReplaceElement(ElementRef const& aElement, ElementTree const& aNewElement);

            NodeChange validateRemoveElement (ElementRef const& aElement);
        private:
            void                implValidateSet();
            Path::Component     implValidateElement(ElementRef const& aElement, bool bReqRemovable);
            void                implValidateTree(ElementTree const& aElementTree);
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

            NodeChange validateReplaceElement(ElementRef const& aElement, UnoAny const& aNewValue);

            NodeChange validateRemoveElement (ElementRef const& aElement);
        private:
            typedef Tree ElementNodeRef;
            void implValidateSet();
            Path::Component     implValidateElement(ElementRef const& aElement, bool bReqRemovable);
            UnoAny implValidateValue(ElementNodeRef const& aElementTree, UnoAny const& aValue);
            UnoAny implValidateValue(UnoAny const& aValue);

            ElementTreeHolder makeValueElement(Name const& aName, ElementNodeRef const& aElementTree, UnoAny const& aValue, bool bInserting);
            ElementTreeHolder makeValueElement(Name const& aName, UnoAny const& aValue, bool bInserting);
            ElementNodeRef extractElementNode(ElementRef const& aElement);
        };
//-----------------------------------------------------------------------------

        /// allows to restore to its default state a <type>Node</type> that is a Container ("set") of full-fledged trees.
        class SetDefaulter
        {
            Tree    m_aParentTree;
            NodeRef m_aSetNode;
            DefaultProvider m_aDefaultProvider;
        public:
            SetDefaulter(Tree const& aParentTree, NodeRef const& aSetNode, DefaultProvider const& aDefaultProvider);

            NodeChange validateSetToDefaultState();

        private:
            void implValidateSet();
        };
//-----------------------------------------------------------------------------
    }
}

#endif // CONFIGMGR_CONFIGSET_HXX_
