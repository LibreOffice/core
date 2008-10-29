/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: setnodeimpl.cxx,v $
 * $Revision: 1.26 $
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
#include <stdio.h>

#include "builddata.hxx"
#include "setnodeimpl.hxx"
#include "treefragment.hxx"
#include "viewfactory.hxx"
#include "configpath.hxx"
#include "tree.hxx"
#include "valuenodeimpl.hxx"
#include "nodechange.hxx"
#include "nodechangeimpl.hxx"
#include "nodevisitor.hxx"
#include "change.hxx"
#include "viewaccess.hxx"
#include "nodeconverter.hxx"
#include "treeactions.hxx"
#include "treechangefactory.hxx"
#include "collectchanges.hxx"
#include <osl/diagnose.h>

#ifndef INCLUDED_VECTOR
#include <vector>
#define INCLUDED_VECTOR
#endif

namespace configmgr
{
    namespace configuration
    {
//-------------------------------------------------------------------------
// initialization helpers
//-------------------------------------------------------------------------
namespace
{
    class CollectElementTrees : data::SetVisitor
    {
    public:
        CollectElementTrees(rtl::Reference<view::ViewStrategy> const& _xStrategy,
                            Tree* pParentTree, unsigned int nPos,
                            unsigned int nDepth,
                            rtl::Reference<Template> const& aTemplate,
                            TemplateProvider const& aTemplateProvider)
        : m_aTemplate(aTemplate)
        , m_aTemplateProvider(aTemplateProvider)
        , m_xStrategy(_xStrategy)
        , m_pParentTree(pParentTree)
        , m_nPos(nPos)
        , m_nDepth(nDepth)
        {
            OSL_ENSURE(m_aTemplate.is(),"WARNING: Collecting a set without a template");
        }

        void collect(sharable::SetNode * node)
        {
            visitElements(node);
        }

        ElementTreeData create(sharable::TreeFragment * elementTree)
        {
            OSL_ENSURE(collection.empty(),"warning: trying to reuse a full collection");

            collection.resize(1); // make an empty one for case of failure
            this->visitTree(elementTree);

            OSL_ENSURE(collection.size()==2,"warning: could not create an element");
            return collection.back();
        }

        std::vector<ElementTreeData> collection;

       private:
        using SetVisitor::handle;

        virtual bool handle(sharable::Node * node);

        virtual bool handle(sharable::ValueNode * node);

        virtual bool handle(sharable::TreeFragment * tree);

        void add(sharable::TreeFragment * node);

        rtl::Reference<Template>            m_aTemplate;
        TemplateProvider        m_aTemplateProvider;
        rtl::Reference<view::ViewStrategy>   m_xStrategy;
        Tree*   m_pParentTree;
        unsigned int    m_nPos;
        unsigned int    m_nDepth;
    };
    //-------------------------------------------------------------------------

    static
    rtl::OUString validatedName(ElementTreeData const& aTree)
    {
        OSL_ENSURE(aTree.isValid(), "INTERNAL ERROR: Unexpected null tree constructed in set node");
        if (!aTree.isValid()) throw Exception("INTERNAL ERROR: Unexpected null tree in set node");

        OSL_ENSURE(aTree->nodeCount(), "INTERNAL ERROR: Unexpected empty (!) tree constructed in set node");
        OSL_ENSURE(aTree->isValidNode(Tree::ROOT), "INTERNAL ERROR: Corrupt tree constructed in set node");

        return aTree->getSimpleRootName();
    }
    //-------------------------------------------------------------------------

    static
    bool isInDefault(SetEntry const& _anEntry)
    {
        if (!_anEntry.isValid()) return false;

        node::Attributes aAttributes = _anEntry.getTreeView().getRootAttributes();

        bool bReplaced = aAttributes.isReplacedForUser();

        return !bReplaced;
    }
    //-------------------------------------------------------------------------
    bool CollectElementTrees::handle(sharable::ValueNode * node)
    {
        if (m_aTemplate.is())
        {
            OSL_ENSURE(m_aTemplate->isInstanceTypeKnown(),"ERROR: Template must have a validated type when building a set.");
            OSL_ENSURE(m_aTemplate->isInstanceValue(),"ERROR: Found a value node in a Complex Template Set");

            if (!m_aTemplate->isInstanceValue())
                throw Exception("INTERNAL ERROR: Corrupt tree contains a value node within a template-set");

            com::sun::star::uno::Type aValueType = node->getValueType();
            com::sun::star::uno::Type aExpectedType = m_aTemplate->getInstanceType();

            if (aValueType.getTypeClass() != aExpectedType.getTypeClass() &&
                aExpectedType.getTypeClass() != uno::TypeClass_ANY &&
                aValueType.getTypeClass() != uno::TypeClass_VOID)
            {
                OSL_ENSURE(false, "WARNING: ValueType of set node does not match the template type");
            //  throw TypeMismatch(aValueType.getTypeName(),aExpectedType.getTypeName(), "INTERNAL ERROR: - Corrupt tree contains mistyped value node within a value-set")));
            }
        }
        return false;
    }
    //-------------------------------------------------------------------------
    bool CollectElementTrees::handle(sharable::Node * node)
    {
        (void) node; // avoid warnings
        OSL_ENSURE(!node->isValue(), "Unexpected: Value-node dispatched to wrong handler");
        if (m_aTemplate.is())
        {
            OSL_ENSURE(m_aTemplate->isInstanceTypeKnown(),"ERROR: Template must have a validated type when building a set.");
            OSL_ENSURE(!m_aTemplate->isInstanceValue(),"ERROR: Found a non-leaf node in a Value Set");

            if (m_aTemplate->isInstanceValue())
                throw Exception("INTERNAL ERROR: Corrupt tree contains a non-leaf node within a value-set");

        }
        return false;
    }
    //-------------------------------------------------------------------------
    bool CollectElementTrees::handle(sharable::TreeFragment * tree)
    {
        bool done = visitNode(tree->getRootNode());
        if (!done) {
            add(tree);
        }
        return done;
    }
    //-------------------------------------------------------------------------
    void CollectElementTrees::add(sharable::TreeFragment * tree)
    {
        node::Attributes const aAttributes = tree->getAttributes();

    bool bReadonly  = aAttributes.isReadonly();
    bool bInDefault = !aAttributes.isReplacedForUser();

        rtl::Reference<view::ViewStrategy> xStrategy = !bReadonly ? m_xStrategy : view::createReadOnlyStrategy();

        ElementTree * pNewTree;
        if (m_pParentTree)
            pNewTree = new ElementTree(xStrategy, *m_pParentTree, m_nPos, tree, m_nDepth, m_aTemplate, m_aTemplateProvider);

        else
            pNewTree = new ElementTree(xStrategy, tree, m_nDepth, m_aTemplate, m_aTemplateProvider);

        collection.push_back( ElementTreeData(pNewTree,bInDefault));
    }
}

//-------------------------------------------------------------------------
// class ElementSet
//-------------------------------------------------------------------------

bool ElementSet::hasElement(rtl::OUString const& aName) const
{
    return m_aData.find(aName) != m_aData.end();
}
//-------------------------------------------------------------------------

ElementTreeData* ElementSet::getElement(rtl::OUString const& aName)
{
    Data::iterator it = m_aData.find(aName);
    if (it != m_aData.end())
        return &it->second;
    else
        return 0;
}
//-------------------------------------------------------------------------


ElementTreeData const* ElementSet::getElement(rtl::OUString const& aName) const
{
    Data::const_iterator it = m_aData.find(aName);
    if (it != m_aData.end())
        return &it->second;
    else
        return 0;
}
//-------------------------------------------------------------------------

ElementTreeData ElementSet::findElement(rtl::OUString const& aName)
{
    ElementTreeData aRet;

    Data::iterator it = m_aData.find(aName);
    if (it != m_aData.end())
        aRet = it->second;

    return aRet;
}
//-------------------------------------------------------------------------

void ElementSet::insertElement(rtl::OUString const& aName, ElementTreeData const& aNewEntry)
{
    bool bInserted = m_aData.insert(Data::value_type(aName, aNewEntry)).second;

    OSL_ENSURE(bInserted,"INTERNAL ERROR: Inserted set Element was already present");
    if (!bInserted) throw Exception("INTERNAL ERROR: Inserted set Element was already present");
}
//-------------------------------------------------------------------------

ElementTreeData ElementSet::replaceElement(rtl::OUString const& aName, ElementTreeData const& aNewEntry)
{
    OSL_ENSURE(m_aData.find(aName) != m_aData.end(),"INTERNAL ERROR: Replaced set Element is not present");

    ElementTreeData& rElement = m_aData[aName];

    ElementTreeData aOld = rElement;
    rElement = aNewEntry;

    return aOld;
}
//-------------------------------------------------------------------------

ElementTreeData ElementSet::removeElement(rtl::OUString const& aName)
{
    Data::iterator it = m_aData.find(aName);
    OSL_ENSURE(it != m_aData.end(),"INTERNAL ERROR: Removed set Element is not present");

    ElementTreeData aOld;
    if (it != m_aData.end())
    {
        aOld = it->second;
        m_aData.erase(it);
    }
    return aOld;
}
//-------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// class SetEntry
//-----------------------------------------------------------------------------

SetEntry::SetEntry(ElementTree* pTree_)
: m_pTree(pTree_)
{
    OSL_ENSURE(pTree_ == 0 || pTree_->isValidNode(Tree::ROOT),
                "INTERNAL ERROR: Invalid empty tree used for SetEntry ");
}

//-----------------------------------------------------------------------------

view::ViewTreeAccess SetEntry::getTreeView() const
{
    OSL_ENSURE(isValid(), "Cannot get View Access for NULL SetEntry");
    return view::ViewTreeAccess(m_pTree);
}

//-----------------------------------------------------------------------------
//-------------------------------------------------------------------------

//-------------------------------------------------------------------------
// class SetNodeImpl
//-------------------------------------------------------------------------

SetNodeImpl::SetNodeImpl(sharable::SetNode * _pNodeRef,Template* pTemplate)
: NodeImpl(reinterpret_cast<sharable::Node *>(_pNodeRef))
,m_aTemplate(pTemplate)
,m_aTemplateProvider()
,m_pParentTree(0)
,m_nContextPos(0)
,m_aInit(0)
{
}
    // unbind the original
//-----------------------------------------------------------------------------

SetNodeImpl::~SetNodeImpl()
{
}
//-----------------------------------------------------------------------------

void SetNodeImpl::rebuildFrom(SetNodeImpl& rOldData, sharable::SetNode * newNode)
{
    m_aTemplate         = rOldData.m_aTemplate;
    m_aTemplateProvider = rOldData.m_aTemplateProvider;
    m_pParentTree       = rOldData.m_pParentTree;
    m_nContextPos       = rOldData.m_nContextPos;
    m_aInit             = rOldData.m_aInit;

    if (rOldData.implHasLoadedElements())
    {
        rOldData.doTransferElements(m_aDataSet);
        implRebuildElements(newNode);
        OSL_ASSERT(this->implHasLoadedElements());
    }
    else
        OSL_ASSERT(!this->implHasLoadedElements());

    rOldData.m_aTemplate.clear();
    rOldData.m_aTemplateProvider = TemplateProvider();
    rOldData.m_pParentTree = 0;
    rOldData.m_nContextPos = 0;

}

//-----------------------------------------------------------------------------
void SetNodeImpl::doTransferElements(ElementSet& rReplacement)
{
    m_aDataSet.swap(rReplacement);
}
//-----------------------------------------------------------------------------

void SetNodeImpl::implRebuildElements(sharable::SetNode * newNode)
{
    OSL_ENSURE(m_pParentTree,"Cannot rebuild set without context tree");
    rtl::Reference<view::ViewStrategy> xNewStrategy = m_pParentTree->getViewBehavior();

    for(ElementSet::Iterator it = m_aDataSet.begin(), stop = m_aDataSet.end();
    it != stop;
    ++it)
    {
        OSL_ASSERT(it->isValid());
        if (!it->isValid()) continue;

        ElementTreeData aElement = *it;
        rtl::OUString aName = aElement->getSimpleRootName();

        sharable::TreeFragment * aNewElementAccess = newNode->getElement(aName);
        OSL_ASSERT(
            aNewElementAccess != 0 && aElement->getOriginalTreeAccess() != 0);

    aElement->rebuild(xNewStrategy,aNewElementAccess);
    }
}
//-----------------------------------------------------------------------------

sharable::SetNode * SetNodeImpl::getDataAccess() const
{
    sharable::Node * node = getOriginalNodeAccess();
    OSL_ASSERT(node != 0 && node->isSet());
    return &node->set;
}
//-----------------------------------------------------------------------------

Tree*   SetNodeImpl::getParentTree() const
{
    OSL_ENSURE(m_pParentTree,"Set Node: Parent tree not set !");
    return m_pParentTree;
}
//-----------------------------------------------------------------------------

unsigned int    SetNodeImpl::getContextOffset() const
{
    OSL_ENSURE(m_nContextPos,"Set Node: Position within parent tree not set !");
    return m_nContextPos;
}
//-----------------------------------------------------------------------------

bool SetNodeImpl::doIsEmpty() const
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

ElementTree* SetNodeImpl::doFindElement(rtl::OUString const& aName)
{
    return m_aDataSet.findElement(aName).get();
}
//-------------------------------------------------------------------------
void SetNodeImpl::doDifferenceToDefaultState(SubtreeChange& _rChangeToDefault, ISubtree& _rDefaultTree)
{
    OSL_ENSURE(implHasLoadedElements(),"Should not query difference to default state for set that is not loaded");
    implDifferenceToDefaultState(_rChangeToDefault,_rDefaultTree);
}
//-----------------------------------------------------------------------------

SetElementChangeImpl* SetNodeImpl::doAdjustToAddedElement(rtl::OUString const& aName, AddNode const& aAddNodeChange, ElementTreeData const & aNewElement)
{
    return implAdjustToAddedElement(aName,aNewElement,aAddNodeChange.isReplacing());
}
//-------------------------------------------------------------------------

SetElementChangeImpl* SetNodeImpl::implAdjustToAddedElement(rtl::OUString const& aName, ElementTreeData const & aNewElement, bool _bReplacing)
{
    { (void)_bReplacing; }
    OSL_ENSURE( validatedName(aNewElement) == aName, "Unexpected name on new element" );

    if (hasStoredElement(aName))
    {
        OSL_ENSURE( _bReplacing, "Added Element already exists - replacing" );

        ElementTreeData aOldElement = this->replaceElement(aName,aNewElement);

        return implCreateReplace(aName,aNewElement,aOldElement);
    }
    else
    {
        OSL_ENSURE( !_bReplacing, "Replaced Element doesn't exist - simply adding" );
        this->insertElement(aName,aNewElement);

        return implCreateInsert(aName,aNewElement);
    }
}
//-------------------------------------------------------------------------

SetElementChangeImpl* SetNodeImpl::doAdjustToRemovedElement(rtl::OUString const& aName, RemoveNode const& /*aRemoveNodeChange*/)
{
    return implAdjustToRemovedElement(aName);
}
//-------------------------------------------------------------------------

SetElementChangeImpl* SetNodeImpl::implAdjustToRemovedElement(rtl::OUString const& aName)
{
    if (ElementTreeData* pOriginal = getStoredElement(aName))
    {
        ElementTreeData aOldElement = *pOriginal;
        this->removeElement(aName);

        return implCreateRemove(aName,aOldElement);
    }
    else
    {
        OSL_ENSURE( false, "Removed Element doesn't exist - ignoring" );
        return 0;
    }
}
//-------------------------------------------------------------------------

SetElementChangeImpl* SetNodeImpl::implCreateInsert(rtl::OUString const& aName, ElementTreeData const& aNewElement) const
{
    Path::Component aFullName = Path::makeCompositeName(aName, this->getElementTemplate()->getName());

    SetElementChangeImpl* pRet = new SetInsertImpl(aFullName, aNewElement.tree, true);
    pRet->setTarget( getParentTree(), getContextOffset() );
    return pRet;
}
//-------------------------------------------------------------------------

SetElementChangeImpl* SetNodeImpl::implCreateReplace(rtl::OUString const& aName, ElementTreeData const& aNewElement, ElementTreeData const& aOldElement) const
{
    Path::Component aFullName = Path::makeCompositeName(aName, this->getElementTemplate()->getName());

    SetElementChangeImpl* pRet = new SetReplaceImpl(aFullName, aNewElement.tree, aOldElement.tree);
    pRet->setTarget( getParentTree(), getContextOffset() );
    return pRet;
}
//-------------------------------------------------------------------------

SetElementChangeImpl* SetNodeImpl::implCreateRemove(rtl::OUString const& aName, ElementTreeData const& aOldElement) const
{
    Path::Component aFullName = Path::makeCompositeName(aName, this->getElementTemplate()->getName());

    SetElementChangeImpl* pRet = new SetRemoveImpl(aFullName, aOldElement.tree);
    pRet->setTarget( getParentTree(), getContextOffset() );
    return pRet;
}
//-------------------------------------------------------------------------

void SetNodeImpl::insertElement(rtl::OUString const& aName, ElementTreeData const& aNewElement)
{
    attach(aNewElement,aName);
    try
    {
        m_aDataSet.insertElement(aName,aNewElement);
    }
    catch (std::exception&)
    {
        detach(aNewElement);
        throw;
    }
}
//-------------------------------------------------------------------------

ElementTreeData SetNodeImpl::replaceElement(rtl::OUString const& aName, ElementTreeData const& aNewElement)
{
    attach(aNewElement,aName);
    try
    {
        ElementTreeData aOldElement = m_aDataSet.replaceElement(aName,aNewElement);
        detach(aOldElement);
        return aOldElement;
    }
    catch (std::exception&)
    {
        detach(aNewElement);
        throw;
    }
}
//-------------------------------------------------------------------------

ElementTreeData SetNodeImpl::removeElement(rtl::OUString const& aName)
{
    ElementTreeData aOldElement = m_aDataSet.removeElement(aName);
    detach(aOldElement);
    return aOldElement;
}
//-------------------------------------------------------------------------
SetNodeVisitor::Result  SetNodeImpl::doDispatchToElements(SetNodeVisitor& aVisitor)
{
    SetNodeVisitor::Result eRet = SetNodeVisitor::CONTINUE;
    for(ElementSet::Iterator it = m_aDataSet.begin(), stop = m_aDataSet.end();
        it != stop && eRet != SetNodeVisitor::DONE;
        ++it)
    {
        eRet = aVisitor.visit( SetEntry(it->get()) );
    }
    return eRet;
}
//-----------------------------------------------------------------------------

void SetNodeImpl::attach(ElementTreeData const& aNewElement, rtl::OUString const& aName)
{
    // check for name (this also reject NULLs, therefore it should go first)
    rtl::OUString aActualName = validatedName(aNewElement);

    Tree*   pParentContext  = getParentTree();
    unsigned int    nParentOffset   = getContextOffset();

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
    //  OSL_ENSURE(aActualName.isEmpty(), "WARNING: Wrongly named tree inserted in set node");
        aNewElement->renameTree(aName);

        aActualName = validatedName(aNewElement);
        if (aName !=aActualName )
        {
            OSL_ENSURE(false, "INTERNAL ERROR: Cannot rename tree in set node");
            throw Exception("INTERNAL ERROR: Cannot rename tree for insertion into set node");
        }
    }
}
//-------------------------------------------------------------------------

void SetNodeImpl::detach(ElementTreeData const& aOldElement)
{
    if (aOldElement.isValid())
    {
        aOldElement->detachTree();
    }
}
//-------------------------------------------------------------------------

ElementTreeData SetNodeImpl::entryToElement(SetEntry const& _anEntry)
{
    ElementTree * pTree = _anEntry.tree();
    return ElementTreeData(pTree, isInDefault(_anEntry));
}
//-------------------------------------------------------------------------


SetElementChangeImpl* SetNodeImpl::doAdjustChangedElement(NodeChangesInformation& rLocalChanges, rtl::OUString const& aName, Change const& _aElementChange)
{
    SetElementChangeImpl* pThisChange  = NULL;

    if (ElementTreeData* pElement = getStoredElement(aName))
    {
        OSL_ASSERT(pElement->isValid());

        if (SubtreeChange const * subtreeChange = dynamic_cast< SubtreeChange const * >(&_aElementChange))
        {
            // recurse to element tree
            Tree * elementTree = pElement->get();

            view::getViewBehavior(elementTree)->adjustToChanges(rLocalChanges, view::getRootNode(elementTree), *subtreeChange);
        }
        else if (ValueChange const * valueChange = dynamic_cast< ValueChange const * >(&_aElementChange))
        {
            // make an element for the old element
            std::auto_ptr<ValueNode> aOldNode = OTreeNodeConverter().createCorrespondingNode(*valueChange);
            aOldNode->setValue(valueChange->getOldValue());

            bool bWasDefault = (valueChange->getMode() == ValueChange::wasDefault);

            std::auto_ptr<INode> aBasePtr(aOldNode.release());
            rtl::OUString aElementTypeName = getElementTemplate()->getName();
            rtl::Reference< data::TreeSegment > aOldBaseTree = data::TreeSegment::create( aBasePtr, aElementTypeName );

            rtl::Reference<ElementTree> aOldElement = new ElementTree(aOldBaseTree, getElementTemplate(), getTemplateProvider());

            OSL_ASSERT(aOldBaseTree.is()); // the tree took ownership
            OSL_ASSERT(aOldElement->isFree()); // the tree is free-floating

            pThisChange = implCreateReplace(aName,*pElement,ElementTreeData(aOldElement,bWasDefault));
        }
        else
            OSL_ENSURE( false, "Unexpected kind of change to set element" );

    }
    else
    {
        // could be changed to do an insert instead (?)
        OSL_ENSURE( false, "Changed Element doesn't exist - (and not adding now)" );
    }
    return pThisChange;
}
//-------------------------------------------------------------------------

bool SetNodeImpl::implHasLoadedElements() const
{
    return m_aInit == 0; // cannot check whether init was called though ...
}
//-----------------------------------------------------------------------------

void SetNodeImpl::initElements(TemplateProvider const& aTemplateProvider,Tree& rParentTree,unsigned int nPos,unsigned int nDepth)
{
    OSL_ENSURE(m_pParentTree == 0 || m_pParentTree == &rParentTree, "WARNING: Set Node: Changing parent");
    OSL_ENSURE(m_nContextPos == 0 || m_nContextPos == nPos,         "WARNING: Set Node: Changing location within parent");
    m_pParentTree = &rParentTree;
    m_nContextPos = nPos;

    OSL_ENSURE(!m_aTemplateProvider.isValid() || !implHasLoadedElements(),"ERROR: Reinitializing set"); //doClearElements();
    OSL_ASSERT(doIsEmpty()); //doClearElements();

    OSL_ENSURE(!m_aTemplate.is() || m_aTemplate->isInstanceTypeKnown(),"ERROR: Need a type-validated template to fill a set");
    OSL_ENSURE(aTemplateProvider.isValid() || nDepth == 0 || m_aTemplate->isInstanceValue(), "ERROR: Need a template provider to fill a non-primitive set");

    if (nDepth > 0) // dont set a template provider for zero-depth sets
    {
        m_aInit = nDepth;
        m_aTemplateProvider = aTemplateProvider;
    }
}
//-----------------------------------------------------------------------------

bool SetNodeImpl::implLoadElements()
{
    if (m_aInit > 0)
    {
    OSL_ENSURE(!getElementTemplate().is() || getElementTemplate()->isInstanceTypeKnown(),"ERROR: Need a type-validated template to fill a set");
    OSL_ENSURE(getTemplateProvider().isValid() || getElementTemplate()->isInstanceValue(), "ERROR: Need a template provider to fill a non-primitive set");

        unsigned int nDepth = m_aInit;
    implInitElements(this->getDataAccess(),nDepth);
    m_aInit = 0;
    }
    OSL_ASSERT(implHasLoadedElements());

    return m_aInit == 0;
}
//-----------------------------------------------------------------------------

void SetNodeImpl::implEnsureElementsLoaded()
{
    if (!implLoadElements())
        throw ConstraintViolation("Trying to access set elements beyond the loaded nestíng level");
}
//-----------------------------------------------------------------------------

void SetNodeImpl::implInitElements(sharable::SetNode * node, unsigned int nDepth)
{
    Tree* pThisTree = getParentTree();

    OSL_ENSURE(pThisTree,"Cannot load elements of a set that has no parent tree");

    CollectElementTrees aCollector( pThisTree->getViewBehavior(), pThisTree, getContextOffset(),
                                    nDepth, getElementTemplate(), getTemplateProvider() );
    aCollector.collect(node);

    for(std::vector<ElementTreeData>::const_iterator it = aCollector.collection.begin(), stop = aCollector.collection.end();
        it != stop; ++it)
    {
        implInitElement(implValidateElement(*it));
    }
}
//-------------------------------------------------------------------------

void SetNodeImpl::implInitElement(ElementTreeData const& aNewElement)
{
    OSL_PRECOND(aNewElement.isValid(),"INTERNAL ERROR: Set element is NULL");
    OSL_ENSURE(!aNewElement->isFree(),"INTERNAL ERROR: Set element is free-floating");

    OSL_ENSURE(aNewElement->getContextTree() == getParentTree(),"INTERNAL ERROR: Set element has wrong context tree");
    OSL_ENSURE(aNewElement->getContextNode() == getContextOffset(),"INTERNAL ERROR: Set element has wrong context node");

    rtl::OUString aName = validatedName(aNewElement);

    OSL_ENSURE(aName.getLength() != 0,"INTERNAL ERROR: Unnamed element in set");
    OSL_ENSURE(m_aDataSet.getElement(aName) == 0,"INTERNAL ERROR: Duplicate element name in set");

    m_aDataSet.insertElement(aName,aNewElement);
}
//-------------------------------------------------------------------------

ElementTreeData SetNodeImpl::makeAdditionalElement(rtl::Reference<view::ViewStrategy> const& _xStrategy, AddNode const& aAddNodeChange, unsigned int nDepth)
{
    OSL_ENSURE(aAddNodeChange.wasInserted(), "Cannot integrate element that is not in tree yet");

    sharable::TreeFragment * aAddedTree = aAddNodeChange.getInsertedTree();
    // need 'unsafe', because ownership would be gone when notifications are sent
    if (aAddedTree != NULL)
    {
        CollectElementTrees aCollector( _xStrategy, getParentTree(), getContextOffset(),
                                        nDepth, getElementTemplate(), getTemplateProvider() );

        return implValidateElement(aCollector.create(aAddedTree));
    }

    return ElementTreeData();
}
//-------------------------------------------------------------------------

ElementTreeData SetNodeImpl::implValidateElement(ElementTreeData const& aNewElement)
{
    rtl::Reference<Template> aTemplate = getElementTemplate();
    OSL_ENSURE(aTemplate.is(),"INTERNAL ERROR: No template in set node");
    OSL_ENSURE(aTemplate->isInstanceTypeKnown(),"INTERNAL ERROR: Unspecifed template in set node");

    OSL_ENSURE(aNewElement.isValid(),"INTERNAL ERROR: Unexpected NULL element in set node");
    if (aNewElement.isValid())
    {
        if (aTemplate->isInstanceValue())
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

            view::Node aElementRoot = view::getRootNode(aNewElement.get());

            OSL_ENSURE(aElementRoot.isValueNode(),"INTERNAL ERROR: Inserting complex type into value set node");

            view::ValueNode aValueNode(aElementRoot);
            com::sun::star::uno::Type aValueType = aValueNode.get_impl()->getValueType();

            OSL_ENSURE( aValueType.getTypeClass() != uno::TypeClass_INTERFACE,
                        "INTERNAL ERROR: Inserting complex type into value set node");

            com::sun::star::uno::Type aElementType  = aTemplate->getInstanceType();

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
        else // a complete tree
        {
            // TODO: add some validation here
            if (!aNewElement->isTemplateInstance())
            {
                throw TypeMismatch( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("<Unknown> [Missing Template]")),
                                    aTemplate->getName(),
                                    " - Trying to insert element without template into set");
            }
            if (!aNewElement->isInstanceOf(aTemplate))
            {
                throw TypeMismatch( aNewElement->getTemplate()->getPathString(),
                                    aTemplate->getPathString(),
                                    " - Trying to insert element with wrong template into set");
            }
        }
    }
    return aNewElement;
}
//-------------------------------------------------------------------------

namespace
{
    //-------------------------------------------------------------------------
    class DiffToDefault : data::SetVisitor
    {
        SubtreeChange&          m_rChange;
        ISubtree&               m_rDefaultTree;
        OTreeChangeFactory&     m_rChangeFactory;
    public:
        explicit
        DiffToDefault(SubtreeChange& _rChange, ISubtree& _rDefaultTree)
        : m_rChange(_rChange)
        , m_rDefaultTree(_rDefaultTree)
        , m_rChangeFactory( getDefaultTreeChangeFactory() )
        {
        }

        void diff(sharable::SetNode * actualTree)
        {
            translate(m_rDefaultTree);
            visitElements(actualTree);
        }

    private:
        void translate(ISubtree& _rDefaultTree);
        void handleDefault(rtl::Reference< data::TreeSegment > const & _pDefaultElement);
        void handleActual(sharable::TreeFragment * element);

        using SetVisitor::handle;
        virtual bool handle(sharable::TreeFragment * tree)
        { handleActual(tree); return false; }
    };
    //-------------------------------------------------------------------------

    void DiffToDefault::translate(ISubtree& _rDefaultTree)
    {
        rtl::OUString aTypeName = _rDefaultTree.getElementTemplateName();
        OSL_ENSURE(aTypeName.getLength(),"Cannot get element type for default set");

        CollectNames aCollector;
        aCollector.applyToChildren(_rDefaultTree);

        std::vector<rtl::OUString> const& aNames = aCollector.list();

        for(std::vector<rtl::OUString>::const_iterator it = aNames.begin(); it != aNames.end(); ++it)
        {
            std::auto_ptr<INode> aChild = _rDefaultTree.removeChild(*it);
            handleDefault( data::TreeSegment::create(aChild,aTypeName) );
        }

    }
    //-------------------------------------------------------------------------

    void DiffToDefault::handleDefault(rtl::Reference< data::TreeSegment > const &_pDefaultElement)
    {
        OSL_PRECOND(_pDefaultElement.is(), "Unexpected NULL default node");

        rtl::OUString sName = _pDefaultElement->fragment->getName();

        OSL_ENSURE(_pDefaultElement->fragment->getAttributes().isDefault(), "Missing default state on default element tree");
        OSL_ENSURE(_pDefaultElement->fragment->nodes[0].isDefault(), "Missing default attribute on default node");

        std::auto_ptr<AddNode> pAddIt( m_rChangeFactory.createAddNodeChange(_pDefaultElement, sName,true) );

        m_rChange.addChange(base_ptr(pAddIt));
    }
    //-------------------------------------------------------------------------

// -----------------------------------------------------------------------------
    void DiffToDefault::handleActual(sharable::TreeFragment * element)
    {
        bool bDefaultElement = element->getRootNode()->isDefault();

        rtl::OUString sName = element->getName();

        if (Change* pDefaultNode = m_rChange.getChange(sName) )
        {

            if (AddNode * addNode = dynamic_cast< AddNode * >(pDefaultNode))
            {
                if (bDefaultElement)
                {
                    rtl::Reference< data::TreeSegment > aDefaultTree = addNode->getNewTree();
                    m_rDefaultTree.addChild(data::convertTree(aDefaultTree.is() ? aDefaultTree->fragment : 0, true));

                    // no change needed - remove the change and recover the default
                    m_rChange.removeChange(sName);
                }
            }
            else
            {
                // should never happen
                OSL_ASSERT(false);
                if (bDefaultElement) m_rChange.removeChange(sName);
            }
        }
        else
        {
            OSL_ENSURE(!bDefaultElement, "Node marked 'default' not found in actual default data");

            std::auto_ptr<RemoveNode> pRemoveIt( m_rChangeFactory.createRemoveNodeChange(sName,true) );
            // pRemoveIt->expectRemovedNode(pActualNode);
            m_rChange.addChange(base_ptr(pRemoveIt));
        }
    }
//-----------------------------------------------------------------------------
}
//-----------------------------------------------------------------------------
void SetNodeImpl::implDifferenceToDefaultState(SubtreeChange& _rChangeToDefault, ISubtree& _rDefaultTree) const
{
    DiffToDefault(_rChangeToDefault,_rDefaultTree).diff( getDataAccess() );
}
//-----------------------------------------------------------------------------
void SetNodeImpl::convertChanges(NodeChangesInformation& rLocalChanges, SubtreeChange const& rExternalChange,
                                 unsigned int nDepth)
{
    OSL_ASSERT(nDepth > 0);

    if (Tree* pParentTree = this->getParentTree())
    {
        unsigned int nNode = getContextOffset();

        OSL_ENSURE(pParentTree->isValidNode(nNode), "Invalid context node in Set");
        OSL_ENSURE(view::Node(pParentTree, nNode).get_impl() == this,
                    "Wrong context node in Set");

        CollectChanges aCollector(rLocalChanges, *pParentTree, nNode, getElementTemplate(), nDepth);

        aCollector.collectFromChildren(rExternalChange);
    }
    else
        OSL_ENSURE(false, "Missing context tree in Set");
}
//-----------------------------------------------------------------------------

    }
}

