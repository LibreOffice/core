/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: configset.cxx,v $
 * $Revision: 1.32 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_configmgr.hxx"
#include <stdio.h> // needed for Solaris 8
#include "configset.hxx"
#include "nodechange.hxx"
#include "nodechangeimpl.hxx"
#include "tree.hxx"
#include "treefragment.hxx"
#include "template.hxx"
#include "templateimpl.hxx"
#include "configgroup.hxx"
#include "valuenode.hxx"
#include "valuenodeimpl.hxx"
#include "setnodeimpl.hxx"
#include <vos/refernce.hxx>

namespace configmgr
{
    namespace configuration
    {
//-----------------------------------------------------------------------------
// class SetElementFactory
//-----------------------------------------------------------------------------

SetElementFactory::SetElementFactory(TemplateProvider const& aProvider)
: m_aProvider(aProvider)
{
    OSL_ENSURE(aProvider.m_aImpl.is(), "WARNING: Template Instance Factory created without template provider - cannot instantiate elements");
}
//-----------------------------------------------------------------------------

SetElementFactory::SetElementFactory(SetElementFactory const& aOther)
: m_aProvider(aOther.m_aProvider)
{
}
//-----------------------------------------------------------------------------

SetElementFactory& SetElementFactory::operator=(SetElementFactory const& aOther)
{
    m_aProvider = aOther.m_aProvider;
    return *this;
}
//-----------------------------------------------------------------------------

SetElementFactory::~SetElementFactory()
{
}

//-----------------------------------------------------------------------------
rtl::Reference< ElementTree > SetElementFactory::instantiateTemplate(rtl::Reference<Template> const& aTemplate)
{
    OSL_ENSURE(m_aProvider.m_aImpl.is(), "ERROR: Template Instance Factory has no template provider - cannot instantiate element");
    OSL_ENSURE(aTemplate.is(), "ERROR: Template is NULL - cannot instantiate element");

    rtl::Reference< data::TreeSegment > aInstanceTree( m_aProvider.m_aImpl->instantiate(aTemplate) );
    OSL_ENSURE(aInstanceTree.is(), "ERROR: Cannot create Element Instance: Provider could not instantiate template");

    //set removable state
    aInstanceTree->fragment->header.state |= data::State::flag_removable;

    return new ElementTree( aInstanceTree, aTemplate, m_aProvider );
}
//-----------------------------------------------------------------------------
rtl::Reference< ElementTree > SetElementFactory::instantiateOnDefault(rtl::Reference< data::TreeSegment > const& _aElementData, rtl::Reference<Template> const& aDummyTemplate)
{
//  OSL_ENSURE(m_aProvider.m_aImpl(), "ERROR: Template Instance Factory has no template provider - cannot instantiate element");
    OSL_ENSURE(_aElementData.is(), "ERROR: Tree is NULL - cannot instantiate element");
    OSL_ENSURE(aDummyTemplate.is(), "ERROR: Template is NULL - cannot instantiate element");

    return new ElementTree( _aElementData, aDummyTemplate, m_aProvider );
}
//-----------------------------------------------------------------------------

TemplateProvider SetElementFactory::findTemplateProvider(rtl::Reference< Tree > const& aTree, NodeRef const& aNode)
{
    OSL_ENSURE(!isEmpty(aTree.get()), "ERROR: Getting Element Factory requires a valid tree");
    OSL_ENSURE(aNode.isValid(), "ERROR: Getting Element Factory requires a valid node");
    OSL_ENSURE(aTree->isValidNode(aNode.getOffset()), "ERROR: Tree/Node mismatch");
    if (aNode.isValid() )
    {
      view::ViewTreeAccess aView(aTree.get());

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

rtl::Reference<ElementTree> ValueSetUpdater::makeValueElement(rtl::OUString const& aName, com::sun::star::uno::Any const& aValue, bool bInserting)
{

    const node::Attributes aNewValueAttributes = getNewElementAttributes(bInserting); // TODO: get real value

    com::sun::star::uno::Type aType = m_aTemplate->getInstanceType();

    rtl::OUString aTypeName = m_aTemplate->getName();

    std::auto_ptr<INode> pNode;
    if (aValue.hasValue())
        pNode.reset( new ValueNode(aTypeName, aValue, aNewValueAttributes) );
    else
        pNode.reset( new ValueNode(aTypeName, aType, aNewValueAttributes) );

    rtl::Reference< data::TreeSegment > aValueTree = data::TreeSegment::create(aName, pNode);

    return new ElementTree(aValueTree, m_aTemplate, TemplateProvider() );
}
//-----------------------------------------------------------------------------

rtl::Reference<ElementTree> ValueSetUpdater::makeValueElement(rtl::OUString const& aName, rtl::Reference< Tree > const& , com::sun::star::uno::Any const& aValue, bool bInserting)
{
    // for now ignoring the node.
    // TODO: merge attributes etc. from that node's value
    return makeValueElement(aName, aValue,bInserting);
}
//-----------------------------------------------------------------------------


TreeSetUpdater::TreeSetUpdater(rtl::Reference< Tree > const& aParentTree, NodeRef const& aSetNode, rtl::Reference< Template > const& aTemplate)
: m_aParentTree(aParentTree)
, m_aSetNode(aSetNode)
, m_aTemplate(aTemplate)
{
    implValidateSet();
}
//-----------------------------------------------------------------------------

ValueSetUpdater::ValueSetUpdater(rtl::Reference< Tree > const& aParentTree, NodeRef const& aSetNode,
                                 rtl::Reference< Template > const& aTemplate, com::sun::star::uno::Reference<com::sun::star::script::XTypeConverter> const& xConverter)
: m_aParentTree(aParentTree)
, m_aSetNode(aSetNode)
, m_aTemplate(aTemplate)
, m_xTypeConverter(xConverter)
{
    implValidateSet();
}
//-----------------------------------------------------------------------------

SetDefaulter::SetDefaulter(rtl::Reference< Tree > const& aParentTree, NodeRef const& aSetNode,
                            DefaultProvider const& aDefaultProvider)
: m_aParentTree(aParentTree)
, m_aSetNode(aSetNode)
, m_aDefaultProvider(aDefaultProvider)
{
    implValidateSet();
}
//-----------------------------------------------------------------------------

/// validates that a actual set and an updater's construction parameters match
static void doValidateSet(rtl::Reference< Tree > const& aParentTree, NodeRef const& aSetNode)
{
    if (isEmpty(aParentTree.get()))
        throw Exception("INTERNAL ERROR: Set Update: Unexpected NULL tree");

    if (!aSetNode.isValid())
        throw Exception("INTERNAL ERROR: Set Update: Unexpected NULL node");

    if (!aParentTree->isValidNode(aSetNode.getOffset()))
        throw Exception("INTERNAL ERROR: Set Update: node does not match tree");

    if (! view::ViewTreeAccess(aParentTree.get()).isSetNode(aSetNode))
        throw Exception("INTERNAL ERROR: Set Update: node is not a set");

    if (aParentTree->getAttributes(aSetNode).isReadonly())
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

    view::ViewTreeAccess aParentView(m_aParentTree.get());

    if ( aParentView.getElementTemplate(aParentView.toSetNode(m_aSetNode)) != m_aTemplate)
        throw Exception("INTERNAL ERROR: Set Update: template mismatch");
}
//-----------------------------------------------------------------------------

/// validates that the actual set and the construction parameters match
void ValueSetUpdater::implValidateSet()
{
    doValidateSet(m_aParentTree,m_aSetNode);

    com::sun::star::uno::Type aThisType = m_aTemplate->getInstanceType();

    switch ( aThisType.getTypeClass())
    {
    case uno::TypeClass_VOID:       throw Exception("INTERNAL ERROR: Value set element type is void");
    case uno::TypeClass_INTERFACE:  throw Exception("INTERNAL ERROR: Value update invoked on a complex set");

    case uno::TypeClass_STRUCT:
    case uno::TypeClass_EXCEPTION:  throw Exception("INTERNAL ERROR: Unexpected/Invalid type for set elements");

    default: break;
    }

    view::ViewTreeAccess aParentView(m_aParentTree.get());

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

static void doValidateElement(rtl::Reference< ElementTree > const& aElement, bool bReqRemovable)
{
    if (!aElement.is())
        throw Exception("INTERNAL ERROR: Set Update: Unexpected NULL element");

    if ( bReqRemovable)
    {
        rtl::Reference< Tree > aElementTree = aElement.get();

        if(!aElementTree->getAttributes(aElementTree->getRootNode()).isRemovable())
            throw ConstraintViolation( "New Set Update: Existing element cannot be removed (or replaced) !" );
    }
}
//-----------------------------------------------------------------------------

/// validates that the given element is valid in this context and returns its name
Path::Component TreeSetUpdater::implValidateElement(rtl::Reference< ElementTree > const& aElement, bool bReqRemovable)
{
    doValidateElement(aElement,bReqRemovable);
    return aElement->getExtendedRootName();
}
//-----------------------------------------------------------------------------

/// validates that the given element is valid and can be replaced in this context and returns its name
Path::Component ValueSetUpdater::implValidateElement(rtl::Reference< ElementTree > const& aElement, bool mReqRemovable)
{
    doValidateElement(aElement,mReqRemovable);

#if OSL_DEBUG_LEVEL > 0
    com::sun::star::uno::Type aNodeType = ElementHelper::getUnoType(aElement);

    OSL_ENSURE(aNodeType.getTypeClass() != uno::TypeClass_VOID, "INTERNAL ERROR: Set Element without associated type found");
    OSL_ENSURE(aNodeType.getTypeClass() != uno::TypeClass_INTERFACE,"INTERNAL ERROR: Set Element with complex type found");

    OSL_ENSURE(aNodeType == m_aTemplate->getInstanceType() ||
               uno::TypeClass_ANY == m_aTemplate->getInstanceType().getTypeClass(),
               "INTERNAL ERROR: Set Update: existing element does not match template type");
#endif

    return aElement->getExtendedRootName();
}
//-----------------------------------------------------------------------------
static void checkEligibleChild(rtl::Reference< ElementTree > const& aElementTree, rtl::Reference< Tree > const& aParentTree)
{
    ElementTree const * const pElement = aElementTree.get();    OSL_ASSERT(pElement);

    if (pElement->getContextTree() != NULL)
        throw ConstraintViolation( "Set Update: cannot insert an element that already has a parent." );

    Tree const* pAncestor = aParentTree.get();
    while (pAncestor != NULL)
    {
        if (pElement == pAncestor)
            throw ConstraintViolation( "Set Update: Circular insertion - trying to insert an element into self or descendant" );

        pAncestor = pAncestor->getContextTree();
        OSL_ENSURE(pAncestor != aParentTree.get(), "ERROR: Circular tree found");
    }
}

//-----------------------------------------------------------------------------

void TreeSetUpdater::implValidateTree(rtl::Reference< ElementTree > const& aElementTree)
{
    if (!aElementTree.is())
        throw ConstraintViolation( "Set Update: cannot replace element of complex set with NULL node. Remove the element instead !" );

    checkEligibleChild(aElementTree,m_aParentTree);

    if (! aElementTree->isTemplateInstance())
    {
        throw TypeMismatch(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("<Unnamed> (Template missing)")),
                                    m_aTemplate->getName(), " - new element without template in Set Update");
    }

    if (!aElementTree->isInstanceOf(m_aTemplate))
    {
        throw TypeMismatch( aElementTree->getTemplate()->getPathString(),
                            m_aTemplate->getPathString(), " - new element without template in Set Update");
    }
}
//-----------------------------------------------------------------------------

com::sun::star::uno::Any ValueSetUpdater::implValidateValue(com::sun::star::uno::Any const& aValue)
{
    com::sun::star::uno::Type const aThisType = m_aTemplate->getInstanceType();

    OSL_ENSURE( aThisType.getTypeClass() == uno::TypeClass_ANY || isPossibleValueType(aThisType),
                "Invalid element type for value set" );

    com::sun::star::uno::Any aRet;
    if (aValue.hasValue())
    {
        com::sun::star::uno::Type const aValType = aValue.getValueType();

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

com::sun::star::uno::Any ValueSetUpdater::implValidateValue(rtl::Reference< Tree > const& aElementTree, com::sun::star::uno::Any const& aValue)
{
    node::Attributes aAttributes = aElementTree->getAttributes(aElementTree->getRootNode());
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

NodeChange TreeSetUpdater::validateInsertElement (rtl::OUString const& aName, rtl::Reference< ElementTree > const& aNewElement)
{
    view::ViewTreeAccess aParentView(m_aParentTree.get());

    SetEntry anEntry = aParentView.findElement(aParentView.toSetNode(m_aSetNode),aName);
    if (anEntry.isValid())
        throw Exception("INTERNAL ERROR: Set Update: Element to be inserted already exists");

    implValidateTree(aNewElement);

    std::auto_ptr<SetElementChangeImpl> pChange( new SetInsertImpl(aNewElement->makeExtendedName(aName), aNewElement) );

    pChange->setTarget(aParentView.makeNode(m_aSetNode));

    return NodeChange(pChange.release());
}
//-----------------------------------------------------------------------------

NodeChange ValueSetUpdater::validateInsertElement (rtl::OUString const& aName, com::sun::star::uno::Any const& aNewValue)
{
    view::ViewTreeAccess aParentView(m_aParentTree.get());

    SetEntry anEntry = aParentView.findElement(aParentView.toSetNode(m_aSetNode),aName);
    if (anEntry.isValid())
        throw Exception("INTERNAL ERROR: Set Update: Element to be inserted already exists");

    com::sun::star::uno::Any aValidValue = implValidateValue(aNewValue);

    rtl::Reference<ElementTree> aNewElement = makeValueElement(aName, aValidValue,true);

    std::auto_ptr<SetElementChangeImpl> pChange( new SetInsertImpl(aNewElement->makeExtendedName(aName), aNewElement) );

    pChange->setTarget(aParentView.makeNode(m_aSetNode));

    return NodeChange(pChange.release());
}
//-----------------------------------------------------------------------------

NodeChange TreeSetUpdater::validateReplaceElement(rtl::Reference< ElementTree > const& aElement, rtl::Reference< ElementTree > const& aNewElement)
{
    Path::Component aName = implValidateElement(aElement,true);

    implValidateTree(aNewElement);

    std::auto_ptr<SetElementChangeImpl> pChange( new SetReplaceImpl(aName, aNewElement) );

    pChange->setTarget(view::ViewTreeAccess(m_aParentTree.get()).makeNode(m_aSetNode));

    return NodeChange(pChange.release());
}
//-----------------------------------------------------------------------------

NodeChange ValueSetUpdater::validateReplaceElement(rtl::Reference< ElementTree > const& aElement, com::sun::star::uno::Any const& aNewValue)
{
    Path::Component aName = implValidateElement(aElement,false);

    rtl::Reference< Tree > aElementNode = extractElementNode(aElement);

    com::sun::star::uno::Any aValidValue = implValidateValue(aElementNode, aNewValue);

    rtl::Reference< Tree > aElementTree = aElement.get();

    rtl::Reference<ElementTree> aNewElement;
    if(aElementTree->getAttributes(aElementTree->getRootNode()).isRemovable())
    {
        aNewElement = makeValueElement(aName.getName(), aElementNode, aValidValue,true);
    }
    else
    {
        aNewElement = makeValueElement(aName.getName(), aElementNode, aValidValue,false);
    }

    std::auto_ptr<SetElementChangeImpl> pChange( new SetReplaceImpl(aName, aNewElement) );

    pChange->setTarget(view::ViewTreeAccess(m_aParentTree.get()).makeNode(m_aSetNode));

    return NodeChange(pChange.release());
}
//-----------------------------------------------------------------------------

NodeChange TreeSetUpdater::validateRemoveElement (rtl::Reference< ElementTree > const& aElement)
{
    Path::Component aName = implValidateElement(aElement,true);

    std::auto_ptr<SetElementChangeImpl> pChange( new SetRemoveImpl(aName) );

    pChange->setTarget(view::ViewTreeAccess(m_aParentTree.get()).makeNode(m_aSetNode));

    return NodeChange(pChange.release());
}

//-----------------------------------------------------------------------------

NodeChange ValueSetUpdater::validateRemoveElement (rtl::Reference< ElementTree > const& aElement)
{
    Path::Component aName = implValidateElement(aElement,true);

    std::auto_ptr<SetElementChangeImpl> pChange( new SetRemoveImpl(aName) );

    pChange->setTarget(view::ViewTreeAccess(m_aParentTree.get()).makeNode(m_aSetNode));

    return NodeChange(pChange.release());
}

//-----------------------------------------------------------------------------

NodeChange SetDefaulter::validateSetToDefaultState()
{
    std::auto_ptr< ISubtree > aDefault = m_aDefaultProvider.getDefaultTree(m_aParentTree,m_aSetNode);

    // now build the specific change
    std::auto_ptr<SetChangeImpl> pChange;

    if (aDefault.get())
    {
        TemplateProvider aProvider = SetElementFactory::findTemplateProvider(m_aParentTree,m_aSetNode);

        configmgr::configuration::SetElementFactory aTmp(aProvider);
        pChange.reset( new SetResetImpl(aTmp, aDefault) );
    pChange->setTarget(view::ViewTreeAccess(m_aParentTree.get()).makeNode(m_aSetNode));
    }
    return NodeChange(pChange.release());
}
//-----------------------------------------------------------------------------

rtl::Reference< Tree > ValueSetUpdater::extractElementNode (rtl::Reference< ElementTree > const& aElement)
{
    return aElement.get();
}
//-----------------------------------------------------------------------------

#if OSL_DEBUG_LEVEL > 0
com::sun::star::uno::Type   ElementHelper::getUnoType(rtl::Reference< ElementTree > const& aElement)
{
    OSL_PRECOND( aElement.is(), "ERROR: Configuration: ElementTree operation requires valid node" );

    rtl::Reference< Tree > aElementTree(aElement.get());

    NodeRef aNode = aElementTree->getRootNode();
    OSL_ASSERT( aNode.isValid() );

    view::ViewTreeAccess aElementView(aElementTree.get());

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
#endif
//-----------------------------------------------------------------------------
    }
}

