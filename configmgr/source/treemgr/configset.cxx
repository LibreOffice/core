/*************************************************************************
 *
 *  $RCSfile: configset.cxx,v $
 *
 *  $Revision: 1.24 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-01 13:39:53 $
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
#include <stdio.h> // needed for Solaris 8
#include "configset.hxx"

#ifndef CONFIGMGR_CONFIGCHANGE_HXX_
#include "nodechange.hxx"
#endif
#ifndef CONFIGMGR_CONFIGCHANGEIMPL_HXX_
#include "nodechangeimpl.hxx"
#endif
#ifndef CONFIGMGR_CONFIGNODEIMPL_HXX_
#include "treeimpl.hxx"
#endif
#ifndef CONFIGMGR_CONFIGTEMPLATE_HXX_
#include "template.hxx"
#endif
#ifndef CONFIGMGR_TEMPLATEIMPL_HXX_
#include "templateimpl.hxx"
#endif
#ifndef CONFIGMGR_CONFIGGROUP_HXX_
#include "configgroup.hxx"
#endif

#ifndef _CONFIGMGR_TREE_VALUENODE_HXX
#include "valuenode.hxx"
#endif
#ifndef CONFIGMGR_VALUENODEBEHAVIOR_HXX_
#include "valuenodeimpl.hxx"
#endif
#ifndef CONFIGMGR_SETNODEBEHAVIOR_HXX_
#include "setnodeimpl.hxx"
#endif
#ifndef CONFIGMGR_UPDATEACCESSOR_HXX
#include "updateaccessor.hxx"
#endif

#ifndef _VOS_REFERNCE_HXX_
#include <vos/refernce.hxx>
#endif

namespace configmgr
{
    namespace configuration
    {
//-----------------------------------------------------------------------------
typedef SetInsertImpl  SetInsertTreeImpl;
typedef SetRemoveImpl  SetRemoveTreeImpl;
typedef SetReplaceImpl SetReplaceTreeImpl;
typedef SetInsertImpl  SetInsertValueImpl;
typedef SetRemoveImpl  SetRemoveValueImpl;
typedef SetReplaceImpl SetReplaceValueImpl;
//-----------------------------------------------------------------------------
// class ElementRef
//-----------------------------------------------------------------------------

ElementRef::ElementRef(ElementTreeImpl* pTree)
: m_aTreeHolder(pTree)
{
}
//-----------------------------------------------------------------------------

ElementRef::ElementRef(ElementRef const& aOther)
: m_aTreeHolder(aOther.m_aTreeHolder)
{
}
//-----------------------------------------------------------------------------

ElementRef& ElementRef::operator=(ElementRef const& aOther)
{
    m_aTreeHolder = aOther.m_aTreeHolder;
    return *this;
}
//-----------------------------------------------------------------------------

ElementRef::~ElementRef()
{
}
//-----------------------------------------------------------------------------

bool ElementRef::isValid() const
{
    return !!m_aTreeHolder.is();
}

//-----------------------------------------------------------------------------

ElementTree ElementRef::getElementTree(data::Accessor const& _accessor) const
{
   return ElementTree(_accessor, m_aTreeHolder);
}
//-----------------------------------------------------------------------------

Path::Component ElementRef::getFullName() const
{
    if (!isValid()) return Path::makeEmptyComponent();

    return m_aTreeHolder->getExtendedRootName();
}
//-----------------------------------------------------------------------------

Name ElementRef::getName() const
{
    if (!isValid()) return Name();

    return m_aTreeHolder->getSimpleRootName();
}
//-----------------------------------------------------------------------------

TemplateHolder ElementRef::getTemplate() const
{
    if (!isValid()) return TemplateHolder();

    return m_aTreeHolder->getTemplate();
}
//-----------------------------------------------------------------------------

TreeRef ElementRef::getTreeRef() const
{
    return TreeRef(m_aTreeHolder.get());
}
//-----------------------------------------------------------------------------

ElementRef ElementRef::extract(TreeRef const& aTree)
{
    TreeImpl* pTree = TreeImplHelper::impl(aTree);
    ElementTreeImpl* pImpl = pTree ? pTree->asElementTree() : 0;
    return ElementRef(pImpl);
}
//-----------------------------------------------------------------------------

osl::Mutex& ElementRef::getTreeLock() const
{
    OSL_ENSURE(isValid(),"ERROR: Trying to get NULL lock for tree");

    return m_aTreeHolder->getRootLock();
}
//-----------------------------------------------------------------------------
// class ElementTree
//-----------------------------------------------------------------------------

ElementTree::ElementTree()
: m_accessor(NULL)
, m_aTreeHolder()
{
}
//-----------------------------------------------------------------------------

ElementTree::ElementTree(data::Accessor const& _accessor, ElementTreeImpl* pTree)
: m_accessor(_accessor)
, m_aTreeHolder(pTree)
{
}
//-----------------------------------------------------------------------------

ElementTree::ElementTree(data::Accessor const& _accessor, ElementTreeHolder const& pTree)
: m_accessor(_accessor)
, m_aTreeHolder(pTree)
{
}
//-----------------------------------------------------------------------------

ElementTree::ElementTree(ElementTree const& aOther)
: m_accessor(aOther.m_accessor)
, m_aTreeHolder(aOther.m_aTreeHolder)
{
}
//-----------------------------------------------------------------------------

ElementTree& ElementTree::operator=(ElementTree const& aOther)
{
    m_accessor = aOther.m_accessor;
    m_aTreeHolder = aOther.m_aTreeHolder;
    return *this;
}
//-----------------------------------------------------------------------------

ElementTree::~ElementTree()
{
}
//-----------------------------------------------------------------------------

bool ElementTree::isValid() const
{
    return !!m_aTreeHolder.is();
}
//-----------------------------------------------------------------------------

ElementTreeHolder ElementTree::get() const
{
    return m_aTreeHolder;
}
//-----------------------------------------------------------------------------

ElementTreeImpl* ElementTree::getImpl() const
{
    return m_aTreeHolder.get();
}
//-----------------------------------------------------------------------------

TemplateHolder ElementTree::getTemplate() const
{
    OSL_PRECOND(isValid(),"ERROR: Trying to get the template of a NULL element tree");

    if (m_aTreeHolder,isValid())
        return m_aTreeHolder->getTemplate();
    else
        return TemplateHolder();
}
//-----------------------------------------------------------------------------

ElementTreeImpl* ElementTree::operator->() const
{
    return m_aTreeHolder.operator->();
}
//-----------------------------------------------------------------------------

ElementTreeImpl& ElementTree::operator*() const
{
    return m_aTreeHolder.operator*();
}
//-----------------------------------------------------------------------------

Tree ElementTree::getTree() const
{
    return Tree(m_accessor,m_aTreeHolder.get());
}
//-----------------------------------------------------------------------------

ElementTree ElementTree::extract(Tree const& aTree)
{
    return ElementRef::extract(aTree.getRef()).getElementTree(aTree.getDataAccessor());
}
//-----------------------------------------------------------------------------

data::TreeSegment ElementTree::getOwnedElement(ElementTree const& aElementTree)
{
    OSL_PRECOND(aElementTree.isValid(),"ERROR: Trying to take over the content of a NULL element tree");
    OSL_PRECOND(aElementTree->isFree(),"Trying to take over the content of a owned element tree - returning NULL");

    return aElementTree->getOwnedTree();
}

//-----------------------------------------------------------------------------
// class TemplateInfo
//-----------------------------------------------------------------------------

TemplateInfo::TemplateInfo(TemplateHolder const& aTemplate)
: m_aTemplate(aTemplate)
{
    OSL_ENSURE(m_aTemplate.is(), "ERROR: Configuration: Creating TemplateInfo without template information");
    if (!m_aTemplate.is())
        throw configuration::Exception("Missing template information");
}
//-----------------------------------------------------------------------------

TemplateHolder TemplateInfo::getTemplate() const
{
    return m_aTemplate;
}
//-----------------------------------------------------------------------------

UnoType TemplateInfo::getType() const
{
    return m_aTemplate->getInstanceType();
}

//-----------------------------------------------------------------------------

Name TemplateInfo::getTemplateName() const
{
    return m_aTemplate->getName();
}
//-----------------------------------------------------------------------------

Name TemplateInfo::getTemplatePackage() const
{
    return m_aTemplate->getModule();
}
//-----------------------------------------------------------------------------

OUString TemplateInfo::getTemplatePathString() const
{
    return m_aTemplate->getPathString();
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// class SetElementInfo
//-----------------------------------------------------------------------------

SetElementInfo::SetElementInfo(data::Accessor const& _aSetAccessor, TemplateHolder const& aTemplate)
: m_aTemplateInfo(aTemplate)
, m_aSetAccessor(_aSetAccessor)
{
}
//-----------------------------------------------------------------------------

TemplateHolder SetElementInfo::getTemplate() const
{
    return m_aTemplateInfo.getTemplate();
}
//-----------------------------------------------------------------------------

TemplateInfo SetElementInfo::getTemplateInfo() const
{
    return m_aTemplateInfo;
}
//-----------------------------------------------------------------------------

TemplateHolder SetElementInfo::extractElementInfo(Tree const& aTree, NodeRef const& aNode)
{
    OSL_ENSURE(!aTree.isEmpty(), "ERROR: Getting Element Info requires a valid tree");
    OSL_ENSURE(aNode.isValid(), "ERROR: Getting Element Info requires a valid node");
    OSL_ENSURE(aTree.isValidNode(aNode), "ERROR: Tree/Node mismatch");
    if (aNode.isValid() )
    {
        view::ViewTreeAccess aView = aTree.getView();

        OSL_ENSURE (aView.isSetNode(aNode), "WARNING: Getting Element Info requires a SET node");
        if (aView.isSetNode(aNode))
            return aView.getElementTemplate(aView.toSetNode(aNode));
    }
    return TemplateHolder();
}

//-----------------------------------------------------------------------------
// class SetElementFactory
//-----------------------------------------------------------------------------

SetElementFactory::SetElementFactory(data::Accessor const& _aDataAccessor, TemplateProvider const& aProvider)
: m_aProvider(aProvider)
, m_aDataAccessor(_aDataAccessor)
{
    OSL_ENSURE(aProvider.m_aImpl.is(), "WARNING: Template Instance Factory created without template provider - cannot instantiate elements");
}
//-----------------------------------------------------------------------------

SetElementFactory::SetElementFactory(SetElementFactory const& aOther)
: m_aProvider(aOther.m_aProvider)
, m_aDataAccessor(aOther.m_aDataAccessor)
{
}
//-----------------------------------------------------------------------------

SetElementFactory& SetElementFactory::operator=(SetElementFactory const& aOther)
{
    m_aProvider = aOther.m_aProvider;
    m_aDataAccessor = aOther.m_aDataAccessor;
    return *this;
}
//-----------------------------------------------------------------------------

SetElementFactory::~SetElementFactory()
{
}

//-----------------------------------------------------------------------------
ElementTree SetElementFactory::instantiateTemplate(TemplateHolder const& aTemplate)
{
    OSL_ENSURE(m_aProvider.m_aImpl.is(), "ERROR: Template Instance Factory has no template provider - cannot instantiate element");
    OSL_ENSURE(aTemplate.is(), "ERROR: Template is NULL - cannot instantiate element");

    if (!m_aProvider.m_aImpl.is()) return ElementTree::emptyElement();

    if (!aTemplate.is()) return ElementTree::emptyElement();

    data::TreeSegment aInstanceTree( m_aProvider.m_aImpl->instantiate(this->getDataAccessor(), aTemplate) );
    OSL_ENSURE(aInstanceTree.is(), "ERROR: Cannot create Element Instance: Provider could not instantiate template");


    if (!aInstanceTree.is()) return ElementTree::emptyElement();
    //set removable state
    aInstanceTree.markRemovable();

    ElementTree aRet( this->getDataAccessor(), new ElementTreeImpl( aInstanceTree, aTemplate, m_aProvider ) );

    return aRet;
}
//-----------------------------------------------------------------------------
ElementTree SetElementFactory::instantiateOnDefault(data::TreeSegment const& _aElementData, TemplateHolder const& aDummyTemplate)
{
//  OSL_ENSURE(m_aProvider.m_aImpl(), "ERROR: Template Instance Factory has no template provider - cannot instantiate element");
    OSL_ENSURE(_aElementData.is(), "ERROR: Tree is NULL - cannot instantiate element");
    OSL_ENSURE(aDummyTemplate.is(), "ERROR: Template is NULL - cannot instantiate element");

    if (!_aElementData.is()) return ElementTree::emptyElement();

    ElementTree aRet( this->getDataAccessor(), new ElementTreeImpl( _aElementData, aDummyTemplate, m_aProvider ) );
    // ElementTreeImpl* pNewTree = new ElementTreeImpl( NodeType::getDeferredChangeFactory(),*aTree, c_TreeDepthAll, aDummyTemplate, m_aProvider );
    // pNewTree->takeNodeFrom(aTree);

    // return ElementTree( pNewTree );

    return aRet;
}
//-----------------------------------------------------------------------------

TemplateProvider SetElementFactory::findTemplateProvider(Tree const& aTree, NodeRef const& aNode)
{
    OSL_ENSURE(!aTree.isEmpty(), "ERROR: Getting Element Factory requires a valid tree");
    OSL_ENSURE(aNode.isValid(), "ERROR: Getting Element Factory requires a valid node");
    OSL_ENSURE(aTree.isValidNode(aNode), "ERROR: Tree/Node mismatch");
    if (aNode.isValid() )
    {
        view::ViewTreeAccess aView = aTree.getView();

        OSL_ENSURE (aView.isSetNode(aNode), "WARNING: Getting Element Factory requires a SET node");
        if (aView.isSetNode(aNode))
            return aView.getTemplateProvider(aView.toSetNode(aNode));
    }
    return TemplateProvider();
}

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// class TreeSetUpdater and ValueSetUpdater
//-----------------------------------------------------------------------------

static node::Attributes getNewElementAttributes(bool bInserting)
{
    node::Attributes aResult;
    aResult.setState( node::isReplaced );
    //Check if you are inserting new dynamic property
    if(bInserting)
    {
        aResult.markRemovable();
    }
    return aResult;
}

// Value Element Factory methods
//-----------------------------------------------------------------------------

ElementTreeHolder ValueSetUpdater::makeValueElement(Name const& aName, UnoAny const& aValue, bool bInserting)
{

    const node::Attributes aNewValueAttributes = getNewElementAttributes(bInserting); // TODO: get real value

    UnoType aType = m_aTemplate->getInstanceType();

    OUString aTypeName = m_aTemplate->getName().toString();

    std::auto_ptr<INode> pNode;
    if (aValue.hasValue())
        pNode.reset( new ValueNode(aTypeName, aValue, aNewValueAttributes) );
    else
        pNode.reset( new ValueNode(aTypeName, aType, aNewValueAttributes) );

    data::TreeSegment aValueTree = data::TreeSegment::createNew(aName.toString(), pNode);

    return new ElementTreeImpl(aValueTree, m_aTemplate, TemplateProvider() );
}
//-----------------------------------------------------------------------------

ElementTreeHolder ValueSetUpdater::makeValueElement(Name const& aName, ElementNodeRef const& , UnoAny const& aValue, bool bInserting)
{
    // for now ignoring the node.
    // TODO: merge attributes etc. from that node's value
    return makeValueElement(aName, aValue,bInserting);
}
//-----------------------------------------------------------------------------


TreeSetUpdater::TreeSetUpdater(Tree const& aParentTree, NodeRef const& aSetNode, SetElementInfo const& aInfo)
: m_aParentTree(aParentTree)
, m_aSetNode(aSetNode)
, m_aTemplate(aInfo.getTemplate())
{
    implValidateSet();
}
//-----------------------------------------------------------------------------

ValueSetUpdater::ValueSetUpdater(Tree const& aParentTree, NodeRef const& aSetNode,
                                 SetElementInfo const& aInfo, UnoTypeConverter const& xConverter)
: m_aParentTree(aParentTree)
, m_aSetNode(aSetNode)
, m_aTemplate(aInfo.getTemplate())
, m_xTypeConverter(xConverter)
{
    implValidateSet();
}
//-----------------------------------------------------------------------------

SetDefaulter::SetDefaulter(Tree const& aParentTree, NodeRef const& aSetNode,
                            DefaultProvider const& aDefaultProvider)
: m_aParentTree(aParentTree)
, m_aSetNode(aSetNode)
, m_aDefaultProvider(aDefaultProvider)
{
    implValidateSet();
}
//-----------------------------------------------------------------------------

/// validates that a actual set and an updater's construction parameters match
static void doValidateSet(Tree const& aParentTree, NodeRef const& aSetNode)
{
    if (aParentTree.isEmpty())
        throw Exception("INTERNAL ERROR: Set Update: Unexpected NULL tree");

    if (!aSetNode.isValid())
        throw Exception("INTERNAL ERROR: Set Update: Unexpected NULL node");

    if (!aParentTree.isValidNode(aSetNode))
        throw Exception("INTERNAL ERROR: Set Update: node does not match tree");

    if (! aParentTree.getView().isSetNode(aSetNode))
        throw Exception("INTERNAL ERROR: Set Update: node is not a set");

    if (aParentTree.getAttributes(aSetNode).isReadonly())
        throw ConstraintViolation( "Set Update: Set is read-only !" );
}
//-----------------------------------------------------------------------------

/// validates that the actual set and the construction parameters match
void TreeSetUpdater::implValidateSet()
{
    doValidateSet(m_aParentTree,m_aSetNode);

    if (!m_aTemplate.is())
        throw Exception("INTERNAL ERROR: No template available for tree set update");

    if (m_aTemplate->isInstanceValue())
        throw Exception("INTERNAL ERROR: Tree set update invoked on a value-set");

    view::ViewTreeAccess aParentView = m_aParentTree.getView();

    if ( aParentView.getElementTemplate(aParentView.toSetNode(m_aSetNode)) != m_aTemplate)
        throw Exception("INTERNAL ERROR: Set Update: template mismatch");
}
//-----------------------------------------------------------------------------

/// validates that the actual set and the construction parameters match
void ValueSetUpdater::implValidateSet()
{
    doValidateSet(m_aParentTree,m_aSetNode);

    UnoType aThisType = m_aTemplate->getInstanceType();

    switch ( aThisType.getTypeClass())
    {
    case uno::TypeClass_VOID:       throw Exception("INTERNAL ERROR: Value set element type is void");
    case uno::TypeClass_INTERFACE:  throw Exception("INTERNAL ERROR: Value update invoked on a complex set");

    case uno::TypeClass_STRUCT:
    case uno::TypeClass_EXCEPTION:  throw Exception("INTERNAL ERROR: Unexpected/Invalid type for set elements");

    default: break;
    }

    view::ViewTreeAccess aParentView = m_aParentTree.getView();

    if ( aParentView.getElementTemplate(aParentView.toSetNode(m_aSetNode))->getInstanceType() != aThisType)
        throw Exception("INTERNAL ERROR: Set Update: element type mismatch");
}
//-----------------------------------------------------------------------------

/// validates that the actual set and the construction parameters match
void SetDefaulter::implValidateSet()
{
    doValidateSet(m_aParentTree,m_aSetNode);

    if (!m_aDefaultProvider.isValid())
        throw Exception("INTERNAL ERROR: No default provider available for restoring set default state");
}
//-----------------------------------------------------------------------------

static void doValidateElement(ElementRef const& aElement, bool bReqRemovable,Tree const& aTree)
{
    if (!aElement.isValid())
        throw Exception("INTERNAL ERROR: Set Update: Unexpected NULL element");

// DISABLED: replaceable/removable != writable
//  if (!aElement.getAttributes().writable)
//      throw ConstraintViolation( "Set Update: Existing element is read-only !" );

    if ( bReqRemovable)
    {
        Tree aElementTree = aElement.getElementTree(aTree.getDataAccessor()).getTree();

        if(!aElementTree.getAttributes(aElementTree.getRootNode()).isRemovable())
            throw ConstraintViolation( "New Set Update: Existing element cannot be removed (or replaced) !" );
    }
}
//-----------------------------------------------------------------------------

/// validates that the given element is valid in this context and returns its name
Path::Component TreeSetUpdater::implValidateElement(ElementRef const& aElement, bool bReqRemovable)
{
    doValidateElement(aElement,bReqRemovable,m_aParentTree);

#if 0 // maybe reeanable for DEBUG ?
    ElementTreeImpl* pElement = TreeImplHelper::elementImpl(aTree)->isTemplateInstance();
    OSL_ENSURE( pElement, "INTERNAL ERROR: Set Element has wrong type of tree");
    OSL_ENSURE( !pElement || pElement->isTemplateInstance(), "INTERNAL ERROR: Set Element without associated template found");
    OSL_ENSURE( !pElement || pElement->isInstanceOf(m_aTemplate), "INTERNAL ERROR: Set Update: existing element does not match template");
#endif

    return aElement.getFullName();
}
//-----------------------------------------------------------------------------

/// validates that the given element is valid and can be replaced in this context and returns its name
Path::Component ValueSetUpdater::implValidateElement(ElementRef const& aElement, bool mReqRemovable)
{
    doValidateElement(aElement,mReqRemovable,m_aParentTree);

#ifdef _DEBUG
    UnoType aNodeType = ElementHelper::getUnoType(aElement.getElementTree(m_aParentTree.getDataAccessor()));

    OSL_ENSURE(aNodeType.getTypeClass() != uno::TypeClass_VOID, "INTERNAL ERROR: Set Element without associated type found");
    OSL_ENSURE(aNodeType.getTypeClass() != uno::TypeClass_INTERFACE,"INTERNAL ERROR: Set Element with complex type found");

    OSL_ENSURE(aNodeType == m_aTemplate->getInstanceType() ||
               uno::TypeClass_ANY == m_aTemplate->getInstanceType().getTypeClass(),
               "INTERNAL ERROR: Set Update: existing element does not match template type");
#endif

    return aElement.getFullName();
}
//-----------------------------------------------------------------------------
static void checkEligibleChild(ElementTree const& aElementTree, Tree const& aParentTree)
{
    ElementTreeImpl const * const pElement = aElementTree.getImpl();    OSL_ASSERT(pElement);

    if (pElement->getContextTree() != NULL)
        throw ConstraintViolation( "Set Update: cannot insert an element that already has a parent." );

    TreeImpl const* pAncestor = TreeImplHelper::impl(aParentTree);
    while (pAncestor != NULL)
    {
        if (pElement == pAncestor)
            throw ConstraintViolation( "Set Update: Circular insertion - trying to insert an element into self or descendant" );

        pAncestor = pAncestor->getContextTree();
        OSL_ENSURE(pAncestor != TreeImplHelper::impl(aParentTree), "ERROR: Circular tree found");
    }
}

//-----------------------------------------------------------------------------

void TreeSetUpdater::implValidateTree(ElementTree const& aElementTree)
{
    if (!aElementTree.isValid())
        throw ConstraintViolation( "Set Update: cannot replace element of complex set with NULL node. Remove the element instead !" );

    checkEligibleChild(aElementTree,m_aParentTree);

    if (! aElementTree->isTemplateInstance())
    {
        throw TypeMismatch(OUString(RTL_CONSTASCII_USTRINGPARAM("<Unnamed> (Template missing)")),
                                    m_aTemplate->getName().toString(), " - new element without template in Set Update");
    }

    if (!aElementTree->isInstanceOf(m_aTemplate))
    {
        throw TypeMismatch( aElementTree->getTemplate()->getPathString(),
                            m_aTemplate->getPathString(), " - new element without template in Set Update");
    }
}
//-----------------------------------------------------------------------------

UnoAny ValueSetUpdater::implValidateValue(UnoAny const& aValue)
{
    UnoType const aThisType = m_aTemplate->getInstanceType();

    OSL_ENSURE( aThisType.getTypeClass() == uno::TypeClass_ANY || isPossibleValueType(aThisType),
                "Invalid element type for value set" );

    UnoAny aRet;
    if (aValue.hasValue())
    {
        UnoType const aValType = aValue.getValueType();

        if (aValType.getTypeClass() == uno::TypeClass_INTERFACE)
            throw TypeMismatch(aValType.getTypeName(), aThisType.getTypeName(), " - cannot replace value by complex tree in Set update");

        if (aValType == aThisType)
        {
            aRet = aValue;
        }

        else if ( uno::TypeClass_ANY == aThisType.getTypeClass() )
        {
            if ( ! isPossibleValueType(aValType) )
                throw TypeMismatch(aValType.getTypeName(), aThisType.getTypeName(), " - new element has no legal configuration data type");

            aRet = aValue;
        }

        else
        {
            if (!convertCompatibleValue(m_xTypeConverter, aRet, aValue, aThisType))
                throw TypeMismatch(aValType.getTypeName(), aThisType.getTypeName(), " - new element does not match template type in SetUpdate");
        }

        OSL_ASSERT( isPossibleValueType(aRet.getValueType()) );
    }
    else
    {
        //  cannot do anything about null values here
        OSL_ASSERT(aValue.getValueTypeClass() == uno::TypeClass_VOID);

    }
    return aRet;
}
//-----------------------------------------------------------------------------

UnoAny ValueSetUpdater::implValidateValue(ElementNodeRef const& aElementTree, UnoAny const& aValue)
{
    node::Attributes aAttributes = aElementTree.getAttributes(aElementTree.getRootNode());
    // Here we assume writable == removable/replaceable
    if (aAttributes.isReadonly())
        throw ConstraintViolation( "Set Update: Existing element is read-only !" );

    // Here we assume nullable != removable
    if (!aValue.hasValue())
    {
        if (!aAttributes.isNullable())
            throw ConstraintViolation( "Set Update: Value is not nullable !" );
    }
    return implValidateValue( aValue);
}
//-----------------------------------------------------------------------------

NodeChange TreeSetUpdater::validateInsertElement (Name const& aName, ElementTree const& aNewElement)
{
    view::ViewTreeAccess aParentView = m_aParentTree.getView();

    SetEntry anEntry = aParentView.findElement(aParentView.toSetNode(m_aSetNode),aName);
    if (anEntry.isValid())
        throw Exception("INTERNAL ERROR: Set Update: Element to be inserted already exists");

    implValidateTree(aNewElement);

    std::auto_ptr<SetElementChangeImpl> pChange( new SetInsertTreeImpl(aNewElement->makeExtendedName(aName), aNewElement.get()) );

    pChange->setTarget(aParentView.makeNode(m_aSetNode));

    return NodeChange(pChange.release());
}
//-----------------------------------------------------------------------------

NodeChange ValueSetUpdater::validateInsertElement (Name const& aName, UnoAny const& aNewValue)
{
    view::ViewTreeAccess aParentView = m_aParentTree.getView();

    SetEntry anEntry = aParentView.findElement(aParentView.toSetNode(m_aSetNode),aName);
    if (anEntry.isValid())
        throw Exception("INTERNAL ERROR: Set Update: Element to be inserted already exists");

    UnoAny aValidValue = implValidateValue(aNewValue);

    ElementTreeHolder aNewElement = makeValueElement(aName, aValidValue,true);

    std::auto_ptr<SetElementChangeImpl> pChange( new SetInsertValueImpl(aNewElement->makeExtendedName(aName), aNewElement) );

    pChange->setTarget(aParentView.makeNode(m_aSetNode));

    return NodeChange(pChange.release());
}
//-----------------------------------------------------------------------------

NodeChange TreeSetUpdater::validateReplaceElement(ElementRef const& aElement, ElementTree const& aNewElement)
{
    Path::Component aName = implValidateElement(aElement,true);

    implValidateTree(aNewElement);

    std::auto_ptr<SetElementChangeImpl> pChange( new SetReplaceTreeImpl(aName, aNewElement.get()) );

    pChange->setTarget(m_aParentTree.getView().makeNode(m_aSetNode));

    return NodeChange(pChange.release());
}
//-----------------------------------------------------------------------------

NodeChange ValueSetUpdater::validateReplaceElement(ElementRef const& aElement, UnoAny const& aNewValue)
{
    Path::Component aName = implValidateElement(aElement,false);

    ElementNodeRef aElementNode = extractElementNode(aElement);

    UnoAny aValidValue = implValidateValue(aElementNode, aNewValue);

    ElementTreeHolder aNewElement = makeValueElement(aName.getName(), aElementNode, aValidValue,false);

    std::auto_ptr<SetElementChangeImpl> pChange( new SetReplaceValueImpl(aName, aNewElement) );

    pChange->setTarget(m_aParentTree.getView().makeNode(m_aSetNode));

    return NodeChange(pChange.release());
}
//-----------------------------------------------------------------------------

NodeChange TreeSetUpdater::validateRemoveElement (ElementRef const& aElement)
{
    Path::Component aName = implValidateElement(aElement,true);

    std::auto_ptr<SetElementChangeImpl> pChange( new SetRemoveTreeImpl(aName) );

    pChange->setTarget(m_aParentTree.getView().makeNode(m_aSetNode));

    return NodeChange(pChange.release());
}

//-----------------------------------------------------------------------------

NodeChange ValueSetUpdater::validateRemoveElement (ElementRef const& aElement)
{
    Path::Component aName = implValidateElement(aElement,true);

    std::auto_ptr<SetElementChangeImpl> pChange( new SetRemoveValueImpl(aName) );

    pChange->setTarget(m_aParentTree.getView().makeNode(m_aSetNode));

    return NodeChange(pChange.release());
}

//-----------------------------------------------------------------------------

NodeChange SetDefaulter::validateSetToDefaultState()
{
    memory::UpdateAccessor _aTargetSpace(NULL); // to do: get from right place
    std::auto_ptr< ISubtree > aDefault = m_aDefaultProvider.getDefaultTree(_aTargetSpace,m_aParentTree,m_aSetNode);

    // now build the specific change
    std::auto_ptr<SetChangeImpl> pChange;

    if (aDefault.get())
    {
        TemplateProvider aProvider = SetElementFactory::findTemplateProvider(m_aParentTree,m_aSetNode);

        configmgr::configuration::SetElementFactory aTmp(m_aParentTree.getDataAccessor(), aProvider);
        pChange.reset( new SetResetImpl(aTmp, aDefault) );
        pChange->setTarget(m_aParentTree.getView().makeNode(m_aSetNode));
    }
    return NodeChange(pChange.release());
}
//-----------------------------------------------------------------------------

ValueSetUpdater::ElementNodeRef ValueSetUpdater::extractElementNode (ElementRef const& aElement)
{
    return aElement.getElementTree(m_aParentTree.getDataAccessor()).getTree();
}
//-----------------------------------------------------------------------------

UnoType ElementHelper::getUnoType(ElementTree const& aElement)
{
    OSL_PRECOND( aElement.isValid(), "ERROR: Configuration: ElementRef operation requires valid node" );
    if (!aElement.isValid()) return getVoidCppuType();

    Tree aElementTree = aElement.getTree();

    NodeRef aNode = aElementTree.getRootNode();
    OSL_ASSERT( aNode.isValid() );

    view::ViewTreeAccess aElementView = aElementTree.getView();

    if ( aElementView.isValueNode(aNode) )
    {
        return aElementView.getValueType(aElementView.toValueNode(aNode));
    }
    else
    {
        uno::Reference< uno::XInterface > const * const selectInterface=0;
        return ::getCppuType(selectInterface);
    }
}
//-----------------------------------------------------------------------------
    }
}

