/*************************************************************************
 *
 *  $RCSfile: setnodeimpl.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: jb $ $Date: 2000-11-07 14:35:59 $
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

#include "setnodeimpl.hxx"
#include "nodefactory.hxx"

#include "configpath.hxx"
#include "treeimpl.hxx"

#include "cmtreemodel.hxx"

#include <osl/diagnose.h>
#include <stl/vector>

namespace configmgr
{
    namespace configuration
    {

//-------------------------------------------------------------------------
// initialization helpers
//-------------------------------------------------------------------------
namespace
{
    class CollectElementTrees : NodeModification
    {
    public:
        CollectElementTrees(NodeFactory& rFactory, ISubtree& aSet,
                            TreeImpl* pParentTree, NodeOffset nPos,
                            TreeDepth nDepth, TemplateHolder const& aTemplate)
        : m_rFactory(rFactory)
        , m_aTemplate(aTemplate)
        , m_pParentTree(pParentTree)
        , m_nPos(nPos)
        , m_nDepth(nDepth)
        {
            OSL_ENSURE(m_aTemplate.isValid(),"WARNING: Collecting a set without a template");
            NodeModification::applyToChildren(aSet);
        }

        typedef vector<ElementTreeHolder> Collection;
        Collection collection;
    private:
        void handle(ValueNode& rValue);
        void handle(ISubtree& rTree);

        void add(INode& rNode);
        NodeFactory& m_rFactory;
        TemplateHolder m_aTemplate;
        TreeImpl*   m_pParentTree;
        NodeOffset  m_nPos;
        TreeDepth   m_nDepth;
    };

    static
    Name validatedName(ElementTreeHolder const& aTree)
    {
        OSL_ENSURE(aTree.isValid(), "INTERNAL ERROR: Unexpected null tree constructed in set node");
        if (!aTree.isValid()) throw Exception("INTERNAL ERROR: Unexpected null tree in set node");

        OSL_ENSURE(aTree->nodeCount(), "INTERNAL ERROR: Unexpected empty (!) tree constructed in set node");
        OSL_ENSURE(aTree->isValidNode(aTree->root()), "INTERNAL ERROR: Corrupt tree constructed in set node");

        return aTree->name( aTree->root() );
    }
    void CollectElementTrees::handle(ValueNode& rValue)
    {
        if (m_aTemplate.isValid())
        {
            OSL_ENSURE(m_aTemplate->isInstanceValue(),"ERROR: Found a value node in a Complex Template Set");
            if (!m_aTemplate->isInstanceValue())
                throw Exception("INTERNAL ERROR: Corrupt tree contains a value node within a template-set");

            UnoType aValueType = rValue.getValueType();
            UnoType aExpectedType = m_aTemplate->getInstanceType();

            if (aValueType.getTypeClass() != aExpectedType.getTypeClass() &&
                aExpectedType.getTypeClass() != uno::TypeClass_ANY &&
                aValueType.getTypeClass() != uno::TypeClass_VOID)
            {
                OSL_ENSURE(false, "WARNING: ValueType of set node does not match the template type");
            //  throw TypeMismatch(aValueType.getTypeName(),aExpectedType.getTypeName(), "INTERNAL ERROR: - Corrupt tree contains mistyped value node within a value-set")));
            }
        }
        add(rValue);
    }
    void CollectElementTrees::handle(ISubtree& rTree)
    {
        if (m_aTemplate.isValid())
        {
            OSL_ENSURE(!m_aTemplate->isInstanceValue(),"ERROR: Found a non-leaf node in a Value Set");
            if (m_aTemplate->isInstanceValue())
                throw Exception("INTERNAL ERROR: Corrupt tree contains a non-leaf node within a value-set");

        }
        add(rTree);
    }
    void CollectElementTrees::add(INode& rNode)
    {
        bool bWritable = !rNode.getAttributes().writable;

        NodeFactory& rNodeFactory = bWritable ? m_rFactory : NodeType::getReadAccessFactory();

        ElementTreeImpl * pNewTree;
        if (m_pParentTree)
            pNewTree = new ElementTreeImpl(rNodeFactory, *m_pParentTree, m_nPos, rNode,m_nDepth, m_aTemplate);

        else
            pNewTree = new ElementTreeImpl(rNodeFactory, rNode,m_nDepth, m_aTemplate);

        collection.push_back(pNewTree);
    }
}

//-------------------------------------------------------------------------
// class ElementSet
//-------------------------------------------------------------------------

bool ElementSet::hasElement(Name const& aName) const
{
    return m_aData.find(aName) != m_aData.end();
}
//-------------------------------------------------------------------------

ElementSet::Element* ElementSet::getElement(Name const& aName)
{
    Data::iterator it = m_aData.find(aName);
    if (it != m_aData.end())
        return &it->second;
    else
        return 0;
}
//-------------------------------------------------------------------------


ElementSet::Element const* ElementSet::getElement(Name const& aName) const
{
    Data::const_iterator it = m_aData.find(aName);
    if (it != m_aData.end())
        return &it->second;
    else
        return 0;
}
//-------------------------------------------------------------------------

ElementSet::Element ElementSet::findElement(Name const& aName)
{
    Element aRet;

    Data::iterator it = m_aData.find(aName);
    if (it != m_aData.end())
        aRet = it->second;

    return aRet;
}
//-------------------------------------------------------------------------

void ElementSet::insertElement(Name const& aName, Element const& aNewEntry)
{
    bool bInserted = m_aData.insert(Data::value_type(aName, aNewEntry)).second;

    OSL_ENSURE(bInserted,"INTERNAL ERROR: Inserted set Element was already present");
    if (!bInserted) throw Exception("INTERNAL ERROR: Inserted set Element was already present");
}
//-------------------------------------------------------------------------

ElementSet::Element ElementSet::replaceElement(Name const& aName, Element const& aNewEntry)
{
    OSL_ENSURE(m_aData.find(aName) != m_aData.end(),"INTERNAL ERROR: Replaced set Element is not present");

    Element& rElement = m_aData[aName];

    Element aOld = rElement;
    rElement = aNewEntry;

    return aOld;
}
//-------------------------------------------------------------------------

ElementSet::Element ElementSet::removeElement(Name const& aName)
{
    Data::iterator it = m_aData.find(aName);
    OSL_ENSURE(it != m_aData.end(),"INTERNAL ERROR: Removed set Element is not present");

    Element aOld;
    if (it != m_aData.end())
    {
        aOld = it->second;
        m_aData.erase(it);
    }
    return aOld;
}
//-------------------------------------------------------------------------

//-------------------------------------------------------------------------
// class AbstractSetNodeImpl
//-------------------------------------------------------------------------

AbstractSetNodeImpl::AbstractSetNodeImpl(ISubtree& rOriginal, Template* pTemplate)
: SetNodeImpl(rOriginal,pTemplate)
{
}
//-------------------------------------------------------------------------

AbstractSetNodeImpl::AbstractSetNodeImpl(AbstractSetNodeImpl& rOriginal)
: SetNodeImpl(rOriginal)
{
    // steal the data (losing pending changes)
    m_aDataSet.swap(rOriginal.m_aDataSet);
}
//-------------------------------------------------------------------------

bool AbstractSetNodeImpl::doIsEmpty() const
{
    /*
    for(ElementSet::Iterator it = m_aDataSet.begin(), stop = m_aDataSet.end();
        it != stop;
        ++it)
    {
        if (!it->isEmpty())
            return false;
    }
    return true;
    */
    return m_aDataSet.isEmpty();
}
//-------------------------------------------------------------------------

SetEntry AbstractSetNodeImpl::doFindElement(Name const& aName)
{
    return SetEntry( m_aDataSet.findElement(aName).getBodyPtr() );
}
//-------------------------------------------------------------------------

void AbstractSetNodeImpl::doClearElements()
{
    m_aDataSet.clearElements();
}
//-------------------------------------------------------------------------

void AbstractSetNodeImpl::implInitElement(Element const& aNewElement)
{
    OSL_PRECOND(aNewElement.isValid(),"INTERNAL ERROR: Set element is NULL");
    OSL_ENSURE(!aNewElement->isFree(),"INTERNAL ERROR: Set element is free-floating");

    OSL_ENSURE(aNewElement->getContextTree() == getParentTree(),"INTERNAL ERROR: Set element has wrong context tree");
    OSL_ENSURE(aNewElement->getContextNode() == getContextOffset(),"INTERNAL ERROR: Set element has wrong context node");

    Name aName = validatedName(aNewElement);

    OSL_ENSURE(!aName.isEmpty(),"INTERNAL ERROR: Unnamed element in set");
    OSL_ENSURE(m_aDataSet.getElement(aName) == 0,"INTERNAL ERROR: Duplicate element name in set");

    m_aDataSet.insertElement(aName,aNewElement);
}
//-------------------------------------------------------------------------

void AbstractSetNodeImpl::implInsertElement(Name const& aName, Element const& aNewElement)
{
    attach(aNewElement,aName,true);
    try
    {
        m_aDataSet.insertElement(aName,aNewElement);
    }
    catch (std::exception&)
    {
        detach(aNewElement,true);
        throw;
    }
}
//-------------------------------------------------------------------------

void    AbstractSetNodeImpl::implReplaceElement(Name const& aName, Element const& aNewElement)
{
    attach(aNewElement,aName,true);
    try
    {
        detach(m_aDataSet.replaceElement(aName,aNewElement),true);
    }
    catch (std::exception&)
    {
        detach(aNewElement,true);
        throw;
    }
}
//-------------------------------------------------------------------------

void    AbstractSetNodeImpl::implRemoveElement(Name const& aName)
{
    detach(m_aDataSet.removeElement(aName),true);
}
//-------------------------------------------------------------------------

void    AbstractSetNodeImpl::implMakeIndirect(bool bIndirect)
{
    for(ElementSet::Iterator it = m_aDataSet.begin(), stop = m_aDataSet.end();
        it != stop;
        ++it)
    {
        (*it)->makeIndirect(bIndirect);
    }
}
//-------------------------------------------------------------------------

SetNodeVisitor::Result  AbstractSetNodeImpl::doDispatchToElements(SetNodeVisitor& aVisitor)
{
    SetNodeVisitor::Result eRet = SetNodeVisitor::CONTINUE;
    for(ElementSet::Iterator it = m_aDataSet.begin(), stop = m_aDataSet.end();
        it != stop && eRet != SetNodeVisitor::DONE;
        ++it)
    {
        eRet = aVisitor.visit( SetEntry(it->getBodyPtr()) );
    }
    return eRet;
}
//-----------------------------------------------------------------------------

void AbstractSetNodeImpl::attach(Element const& aNewElement, Name const& aName, bool bCommit)
{
    // check for name (this also reject NULLs, therefore it should go first)
    Name aActualName = validatedName(aNewElement);

    TreeImpl*   pParentContext  = getParentTree();
    NodeOffset  nParentOffset   = getContextOffset();

    OSL_ENSURE(nParentOffset != 0 && pParentContext != 0,"INTERNAL ERROR: Set has no context");

    bool bHasContext = (aNewElement->getContextTree() != 0);

    if (bHasContext)
    {
        if (aNewElement->getContextTree() != pParentContext)
        {
            OSL_ENSURE(false,"INTERNAL ERROR: New set element belongs to another context tree" );
            throw Exception("INTERNAL ERROR: New set element belongs to another context tree" );
        }
        if (aNewElement->getContextNode() != nParentOffset)
        {
            OSL_ENSURE(false,"INTERNAL ERROR: New set element belongs to another context node" );
            throw Exception("INTERNAL ERROR: New set element belongs to another context node" );
        }
    }
    else
    {
        OSL_ENSURE(aNewElement->getContextNode() == 0, "INTERNAL ERROR: New element has context position without a parent");
        aNewElement->moveTree(pParentContext,nParentOffset);
    }

    // check for and correct a misnomer - do only after parenthood is assured (else we don't own it anyways)
    if (aName != aActualName)
    {
        OSL_ENSURE(aActualName.isEmpty(), "WARNING: Wrongly named tree inserted in set node");
        aNewElement->renameTree(aName);

        aActualName = validatedName(aNewElement);
        if (aName !=aActualName )
        {
            OSL_ENSURE(false, "INTERNAL ERROR: Cannot rename tree in set node");
            throw Exception("INTERNAL ERROR: Cannot rename tree for insertion into set node");
        }
    }

    if (bCommit) aNewElement->attachTo( getOriginalSetNode(), aName );
}
//-------------------------------------------------------------------------

void AbstractSetNodeImpl::detach(Element const& aOldElement, bool bCommit)
{
    if (aOldElement.isValid())
    {
        aOldElement->detachFrom(getOriginalSetNode(), validatedName(aOldElement));
        if (bCommit) aOldElement->detachTree();
    }
}
//-------------------------------------------------------------------------


//-------------------------------------------------------------------------
// class TreeSetNodeImpl/ValueSetNodeImpl
//-------------------------------------------------------------------------

void TreeSetNodeImpl::doInsertElement(Name const& aName, SetEntry const& aNewEntry)
{
    AbstractSetNodeImpl::implInsertElement( aName, implMakeElement(aNewEntry.tree()));
}
//-------------------------------------------------------------------------

void ValueSetNodeImpl::doInsertElement(Name const& aName, SetEntry const& aNewEntry)
{
    AbstractSetNodeImpl::implInsertElement( aName, implMakeElement(aNewEntry.tree()));
}
//-------------------------------------------------------------------------

void TreeSetNodeImpl::doRemoveElement(Name const& aName)
{
    AbstractSetNodeImpl::implRemoveElement( aName );
}
//-------------------------------------------------------------------------

void ValueSetNodeImpl::doRemoveElement(Name const& aName)
{
    AbstractSetNodeImpl::implRemoveElement( aName );
}
//-------------------------------------------------------------------------

void TreeSetNodeImpl::initHelper(NodeFactory& rFactory, ISubtree& rTree, TreeDepth nDepth)
{
    CollectElementTrees aCollector( rFactory, rTree,
                                    getParentTree(), getContextOffset(),
                                    nDepth, getElementTemplate() );

    typedef CollectElementTrees::Collection::const_iterator Iter;
    for(Iter it = aCollector.collection.begin(), stop = aCollector.collection.end();
        it != stop; ++it)
    {
        implInitElement(implMakeElement(*it));
    }
}
//-------------------------------------------------------------------------

void ValueSetNodeImpl::initHelper(NodeFactory& rFactory, ISubtree& rSet)
{
    CollectElementTrees aCollector( rFactory, rSet,
                                    getParentTree(), getContextOffset(),
                                    0, getElementTemplate() );

    typedef CollectElementTrees::Collection::const_iterator Iter;
    for(Iter it = aCollector.collection.begin(), stop = aCollector.collection.end();
        it != stop; ++it)
    {
        implInitElement(implMakeElement(*it));
    }
}
//-------------------------------------------------------------------------

NodeType::Enum TreeSetNodeImpl::getType() const
{
    return NodeType::eTREESET;
}
//-------------------------------------------------------------------------

NodeType::Enum  ValueSetNodeImpl::getType() const
{
    return NodeType::eVALUESET;
}
//-------------------------------------------------------------------------

ElementTreeHolder TreeSetNodeImpl::implMakeElement(ElementTreeHolder const& aNewElement)
{
    TemplateHolder aTemplate = getElementTemplate();
    OSL_ENSURE(aTemplate.isValid(),"INTERNAL ERROR: No template in set node");

    OSL_ENSURE(aNewElement.isValid(),"INTERNAL ERROR: Unexpected NULL element in set node");
    if (aNewElement.isValid())
    {
        // TODO: add some validation here
        if (!aNewElement->isTemplateInstance())
        {
            throw TypeMismatch( OUString(RTL_CONSTASCII_USTRINGPARAM("<Unknown> [Missing Template]")),
                                aTemplate->getName().toString(),
                                " - Trying to insert element without template into set");
        }
        if (!aNewElement->isInstanceOf(aTemplate))
        {
            throw TypeMismatch( aNewElement->getTemplate()->getPath().toString(),
                                aTemplate->getPath().toString(),
                                " - Trying to insert element with wrong template into set");
        }
    }
    return aNewElement;
}
//-------------------------------------------------------------------------

ElementTreeHolder ValueSetNodeImpl::implMakeElement(ElementTreeHolder const& aNewElement)
{
    TemplateHolder aTemplate = getElementTemplate();
    OSL_ENSURE(aTemplate.isValid(),"INTERNAL ERROR: No template in set node");

    OSL_ENSURE(aNewElement.isValid(),"INTERNAL ERROR: Unexpected NULL element in set node");
    if (aNewElement.isValid())
    {
        if (aNewElement->nodeCount() == 0)
        {
            OSL_ENSURE(false,"INTERNAL ERROR: Invalid (empty) element tree in value set");
            throw Exception("INTERNAL ERROR: Invalid (empty) element tree in value set");
        }
        if (aNewElement->nodeCount() > 1)
        {
            OSL_ENSURE(false,"INTERNAL ERROR: Complex element tree in value set");
            throw Exception("INTERNAL ERROR: Complex element tree in value set");
        }

        // checks that this is a value
        ValueNodeImpl& rNode = aNewElement->node(aNewElement->root())->valueImpl();

        UnoType aElementType    = aTemplate->getInstanceType();
        UnoType aValueType      = rNode.getValueType();

        OSL_ENSURE( aValueType.getTypeClass() != uno::TypeClass_INTERFACE,
                    "INTERNAL ERROR: Inserting complex type into value set node");

        if (aValueType != aElementType)
        {
            // handle 'Any'
            if (aElementType.getTypeClass() != uno::TypeClass_ANY)
            {
                OSL_ENSURE(false,"INTERNAL ERROR:  ´Wrong value type inserting into value set");
                throw TypeMismatch(aValueType.getTypeName(), aElementType.getTypeName(),
                                    "- INTERNAL ERROR: Mistyped element in value set");
            }
        }
    }

    return aNewElement;
}
//-----------------------------------------------------------------------------

    }
}

