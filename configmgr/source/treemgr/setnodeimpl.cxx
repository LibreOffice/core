/*************************************************************************
 *
 *  $RCSfile: setnodeimpl.cxx,v $
 *
 *  $Revision: 1.20 $
 *
 *  last change: $Author: hr $ $Date: 2003-04-04 16:12:19 $
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
#include <stdio.h>

#include "setnodeimpl.hxx"

#ifndef CONFIGMGR_VIEWBEHAVIORFACTORY_HXX_
#include "viewfactory.hxx"
#endif

#ifndef CONFIGMGR_CONFIGPATH_HXX_
#include "configpath.hxx"
#endif
#ifndef CONFIGMGR_CONFIGNODEIMPL_HXX_
#include "treeimpl.hxx"
#endif
#ifndef CONFIGMGR_VALUENODEBEHAVIOR_HXX_
#include "valuenodeimpl.hxx"
#endif

#ifndef CONFIGMGR_CONFIGCHANGE_HXX_
#include "nodechange.hxx"
#endif
#ifndef CONFIGMGR_CONFIGCHANGEIMPL_HXX_
#include "nodechangeimpl.hxx"
#endif

#ifndef CONFIGMGR_NODEACCESS_HXX
#include "nodeaccess.hxx"
#endif
#ifndef CONFIGMGR_VALUENODEACCESS_HXX
#include "valuenodeaccess.hxx"
#endif
#ifndef CONFIGMGR_GROUPNODEACCESS_HXX
#include "groupnodeaccess.hxx"
#endif
#ifndef CONFIGMGR_SETNODEACCESS_HXX
#include "setnodeaccess.hxx"
#endif
#ifndef CONFIGMGR_NODEVISITOR_HXX
#include "nodevisitor.hxx"
#endif
#ifndef CONFIGMGR_CHANGE_HXX
#include "change.hxx"
#endif
#ifndef CONFIGMGR_VIEWACCESS_HXX_
#include "viewaccess.hxx"
#endif

#ifndef CONFIGMGR_NODECONVERTER_HXX
#include "nodeconverter.hxx"
#endif
#ifndef _CONFIGMGR_TREEACTIONS_HXX_
#include "treeactions.hxx"
#endif
#ifndef CONFIGMGR_TREE_CHANGEFACTORY_HXX
#include "treechangefactory.hxx"
#endif
#ifndef CONFIGMGR_COLLECTCHANGES_HXX_
#include "collectchanges.hxx"
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

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
    using namespace data;

    typedef SetNodeImpl::Element Element;

    class CollectElementTrees : SetVisitor
    {
    public:
        CollectElementTrees(view::ViewStrategyRef const& _xStrategy,
                            TreeImpl* pParentTree, NodeOffset nPos,
                            TreeDepth nDepth,
                            TemplateHolder const& aTemplate,
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

        void collect(SetNodeAccess const& _aNode)
        {
            this->visitElements(_aNode);
        }

        Element create(TreeAccessor const & _aElementTree)
        {
            OSL_ENSURE(collection.empty(),"warning: trying to reuse a full collection");

            collection.resize(1); // make an empty one for case of failure
            this->visitTree(_aElementTree);

            OSL_ENSURE(collection.size()==2,"warning: could not create an element");
            return collection.back();
        }

        typedef std::vector<Element> Collection;
        Collection collection;
    private:
        Result handle(TreeAccessor const& _aElement);

        Result handle(NodeAccessRef const& _aNonValue);
        Result handle(ValueNodeAccess const& _aValue);

        void add(TreeAccessor const& _aNode);

        TemplateHolder          m_aTemplate;
        TemplateProvider        m_aTemplateProvider;
        view::ViewStrategyRef   m_xStrategy;
        TreeImpl*   m_pParentTree;
        NodeOffset  m_nPos;
        TreeDepth   m_nDepth;
    };
    //-------------------------------------------------------------------------

    static
    Name validatedName(Element const& aTree)
    {
        OSL_ENSURE(aTree.isValid(), "INTERNAL ERROR: Unexpected null tree constructed in set node");
        if (!aTree.isValid()) throw Exception("INTERNAL ERROR: Unexpected null tree in set node");

        OSL_ENSURE(aTree->nodeCount(), "INTERNAL ERROR: Unexpected empty (!) tree constructed in set node");
        OSL_ENSURE(aTree->isValidNode(aTree->root_()), "INTERNAL ERROR: Corrupt tree constructed in set node");

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
    CollectElementTrees::Result CollectElementTrees::handle(ValueNodeAccess const& _aValue)
    {
        if (m_aTemplate.is())
        {
            OSL_ENSURE(m_aTemplate->isInstanceTypeKnown(),"ERROR: Template must have a validated type when building a set.");
            OSL_ENSURE(m_aTemplate->isInstanceValue(),"ERROR: Found a value node in a Complex Template Set");

            if (!m_aTemplate->isInstanceValue())
                throw Exception("INTERNAL ERROR: Corrupt tree contains a value node within a template-set");

            UnoType aValueType = _aValue.getValueType();
            UnoType aExpectedType = m_aTemplate->getInstanceType();

            if (aValueType.getTypeClass() != aExpectedType.getTypeClass() &&
                aExpectedType.getTypeClass() != uno::TypeClass_ANY &&
                aValueType.getTypeClass() != uno::TypeClass_VOID)
            {
                OSL_ENSURE(false, "WARNING: ValueType of set node does not match the template type");
            //  throw TypeMismatch(aValueType.getTypeName(),aExpectedType.getTypeName(), "INTERNAL ERROR: - Corrupt tree contains mistyped value node within a value-set")));
            }
        }
        return CONTINUE;
    }
    //-------------------------------------------------------------------------
    CollectElementTrees::Result CollectElementTrees::handle(NodeAccessRef const& _aNonValue)
    {
        OSL_ENSURE(!ValueNodeAccess::isInstance(_aNonValue),"Unexpected: Value-node dispatched to wrong handler");
        if (m_aTemplate.is())
        {
            OSL_ENSURE(m_aTemplate->isInstanceTypeKnown(),"ERROR: Template must have a validated type when building a set.");
            OSL_ENSURE(!m_aTemplate->isInstanceValue(),"ERROR: Found a non-leaf node in a Value Set");

            if (m_aTemplate->isInstanceValue())
                throw Exception("INTERNAL ERROR: Corrupt tree contains a non-leaf node within a value-set");

        }
        return CONTINUE;
    }
    //-------------------------------------------------------------------------
    CollectElementTrees::Result CollectElementTrees::handle(TreeAccessor const& _aTree)
    {
        // check the node type
        Result aResult = this->visitNode( _aTree.getRootNode() );

        if (aResult == CONTINUE) this->add(_aTree);

        return aResult;
    }
    //-------------------------------------------------------------------------
    void CollectElementTrees::add(TreeAccessor const& _aTree)
    {
        node::Attributes const aAttributes = _aTree.getAttributes();

        bool bReadonly  = aAttributes.isReadonly();
        bool bInDefault = !aAttributes.isReplacedForUser();

        view::ViewStrategyRef xStrategy = !bReadonly ? m_xStrategy : view::createReadOnlyStrategy(m_pParentTree ? m_pParentTree->getDataSegment() : NULL);

        ElementTreeImpl * pNewTree;
        if (m_pParentTree)
            pNewTree = new ElementTreeImpl(xStrategy, *m_pParentTree, m_nPos, _aTree,m_nDepth, m_aTemplate, m_aTemplateProvider);

        else
            pNewTree = new ElementTreeImpl(xStrategy, _aTree, m_nDepth, m_aTemplate, m_aTemplateProvider);

        collection.push_back( Element(pNewTree,bInDefault));
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

//-----------------------------------------------------------------------------
// class SetEntry
//-----------------------------------------------------------------------------

SetEntry::SetEntry(data::Accessor const& _aAccessor, ElementTreeImpl* pTree_)
: m_aAccessor(_aAccessor), m_pTree(pTree_)
{
    OSL_ENSURE(pTree_ == 0 || pTree_->isValidNode(pTree_->root_()),
                "INTERNAL ERROR: Invalid empty tree used for SetEntry ");
}

//-----------------------------------------------------------------------------

view::ViewTreeAccess SetEntry::getTreeView() const
{
    OSL_ENSURE(isValid(), "Cannot get View Access for NULL SetEntry");
    return view::ViewTreeAccess(m_aAccessor,*m_pTree);
}

//-----------------------------------------------------------------------------
//-------------------------------------------------------------------------

//-------------------------------------------------------------------------
// class SetNodeImpl
//-------------------------------------------------------------------------

SetNodeImpl::SetNodeImpl(data::SetNodeAddress const& _aNodeRef,Template* pTemplate)
: NodeImpl(_aNodeRef)
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

void SetNodeImpl::rebuildFrom(SetNodeImpl& rOldData,data::SetNodeAccess const& _aNewNode,data::SetNodeAccess const& _aOldNode)
{
    m_aTemplate         = rOldData.m_aTemplate;
    m_aTemplateProvider = rOldData.m_aTemplateProvider;
    m_pParentTree       = rOldData.m_pParentTree;
    m_nContextPos       = rOldData.m_nContextPos;
    m_aInit             = rOldData.m_aInit;

    if (rOldData.implHasLoadedElements())
    {
        rOldData.doTransferElements(m_aDataSet);
        implRebuildElements(_aNewNode,_aOldNode);
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

void SetNodeImpl::implRebuildElements(data::SetNodeAccess const& _aNewNode,data::SetNodeAccess const& _aOldNode)
{
    OSL_ENSURE(m_pParentTree,"Cannot rebuild set without context tree");
    rtl::Reference<view::ViewStrategy> xNewStrategy = m_pParentTree->getViewBehavior();

    for(ElementSet::Iterator it = m_aDataSet.begin(), stop = m_aDataSet.end();
        it != stop;
        ++it)
    {
        OSL_ASSERT(it->isValid());
        if (!it->isValid()) continue;

        Element aElement = *it;
        Name aName = aElement->getSimpleRootName();

        data::TreeAccessor const& aNewElementAccess = _aNewNode.getElementTree(aName);
        data::TreeAccessor const& aOldElementAccess = aElement->getOriginalTreeAccess(_aOldNode.accessor());
        OSL_ASSERT(aNewElementAccess.isValid());
        OSL_ASSERT(aOldElementAccess.isValid());

        aElement->rebuild(xNewStrategy,aNewElementAccess,aOldElementAccess.accessor());
    }
}
//-----------------------------------------------------------------------------

data::SetNodeAccess SetNodeImpl::getDataAccess(data::Accessor const& _aAccessor) const
{
    using namespace data;

    NodeAccessRef aNodeAccess = getOriginalNodeAccessRef(&_aAccessor);
    OSL_ASSERT(SetNodeAccess::isInstance(aNodeAccess));

    SetNodeAccess aSetAccess(aNodeAccess);
    OSL_ASSERT(aSetAccess.isValid());

    return aSetAccess;
}
//-----------------------------------------------------------------------------

TreeImpl*   SetNodeImpl::getParentTree() const
{
    OSL_ENSURE(m_pParentTree,"Set Node: Parent tree not set !");
    return m_pParentTree;
}
//-----------------------------------------------------------------------------

NodeOffset  SetNodeImpl::getContextOffset() const
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

ElementTreeImpl* SetNodeImpl::doFindElement(Name const& aName)
{
    return m_aDataSet.findElement(aName).get();
}
//-------------------------------------------------------------------------
void SetNodeImpl::doDifferenceToDefaultState(data::Accessor const& _aAccessor, SubtreeChange& _rChangeToDefault, ISubtree& _rDefaultTree)
{
    OSL_ENSURE(implHasLoadedElements(),"Should not query difference to default state for set that is not loaded");
    implDifferenceToDefaultState(_aAccessor,_rChangeToDefault,_rDefaultTree);
}
//-----------------------------------------------------------------------------

SetElementChangeImpl* SetNodeImpl::doAdjustToAddedElement(data::Accessor const& _aAccessor, Name const& aName, AddNode const& aAddNodeChange, Element const & aNewElement)
{
    return implAdjustToAddedElement(_aAccessor,aName,aNewElement,aAddNodeChange.isReplacing());
}
//-------------------------------------------------------------------------

SetElementChangeImpl* SetNodeImpl::implAdjustToAddedElement(data::Accessor const& _aAccessor, Name const& aName, Element const & aNewElement, bool _bReplacing)
{
    OSL_ENSURE( validatedName(aNewElement) == aName, "Unexpected Name on new element" );

    if (Element* pOriginal = getStoredElement(aName))
    {
        OSL_ENSURE( _bReplacing, "Added Element already exists - replacing" );

        Element aOldElement = this->replaceElement(aName,aNewElement);

        return implCreateReplace(_aAccessor, aName,aNewElement,aOldElement);
    }
    else
    {
        OSL_ENSURE( !_bReplacing, "Replaced Element doesn't exist - simply adding" );
        this->insertElement(aName,aNewElement);

        return implCreateInsert(_aAccessor, aName,aNewElement);
    }
}
//-------------------------------------------------------------------------

SetElementChangeImpl* SetNodeImpl::doAdjustToRemovedElement(data::Accessor const& _aAccessor, Name const& aName, RemoveNode const& aRemoveNodeChange)
{
    return implAdjustToRemovedElement(_aAccessor,aName);
}
//-------------------------------------------------------------------------

SetElementChangeImpl* SetNodeImpl::implAdjustToRemovedElement(data::Accessor const& _aAccessor, Name const& aName)
{
    if (Element* pOriginal = getStoredElement(aName))
    {
        Element aOldElement = *pOriginal;
        this->removeElement(aName);

        return implCreateRemove(_aAccessor, aName,aOldElement);
    }
    else
    {
        OSL_ENSURE( false, "Removed Element doesn't exist - ignoring" );
        return 0;
    }
}
//-------------------------------------------------------------------------

SetElementChangeImpl* SetNodeImpl::implCreateInsert(data::Accessor const& _aAccessor, Name const& aName, Element const& aNewElement) const
{
    Path::Component aFullName = Path::makeCompositeName(aName, this->getElementTemplate()->getName());

    SetElementChangeImpl* pRet = new SetInsertImpl(aFullName, aNewElement.tree, true);
    pRet->setTarget( _aAccessor, getParentTree(), getContextOffset() );
    return pRet;
}
//-------------------------------------------------------------------------

SetElementChangeImpl* SetNodeImpl::implCreateReplace(data::Accessor const& _aAccessor, Name const& aName, Element const& aNewElement, Element const& aOldElement) const
{
    Path::Component aFullName = Path::makeCompositeName(aName, this->getElementTemplate()->getName());

    SetElementChangeImpl* pRet = new SetReplaceImpl(aFullName, aNewElement.tree, aOldElement.tree);
    pRet->setTarget( _aAccessor, getParentTree(), getContextOffset() );
    return pRet;
}
//-------------------------------------------------------------------------

SetElementChangeImpl* SetNodeImpl::implCreateRemove(data::Accessor const& _aAccessor, Name const& aName, Element const& aOldElement) const
{
    Path::Component aFullName = Path::makeCompositeName(aName, this->getElementTemplate()->getName());

    SetElementChangeImpl* pRet = new SetRemoveImpl(aFullName, aOldElement.tree);
    pRet->setTarget( _aAccessor, getParentTree(), getContextOffset() );
    return pRet;
}
//-------------------------------------------------------------------------

void SetNodeImpl::insertElement(Name const& aName, Element const& aNewElement)
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

Element SetNodeImpl::replaceElement(Name const& aName, Element const& aNewElement)
{
    attach(aNewElement,aName);
    try
    {
        Element aOldElement = m_aDataSet.replaceElement(aName,aNewElement);
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

Element SetNodeImpl::removeElement(Name const& aName)
{
    Element aOldElement = m_aDataSet.removeElement(aName);
    detach(aOldElement);
    return aOldElement;
}
//-------------------------------------------------------------------------
/*
void    SetNodeImpl::implMakeIndirect(bool bIndirect)
{
    for(ElementSet::Iterator it = m_aDataSet.begin(), stop = m_aDataSet.end();
        it != stop;
        ++it)
    {
        view::ViewTreeAccess accessor = (*it)->getAccess();
        if (bIndirect)
            accessor.makeIndirect();

        else
            accessor.makeDirect();

    }
}
//-------------------------------------------------------------------------
*/
SetNodeVisitor::Result  SetNodeImpl::doDispatchToElements(data::Accessor const& _aAccessor, SetNodeVisitor& aVisitor)
{
    SetNodeVisitor::Result eRet = SetNodeVisitor::CONTINUE;
    for(ElementSet::Iterator it = m_aDataSet.begin(), stop = m_aDataSet.end();
        it != stop && eRet != SetNodeVisitor::DONE;
        ++it)
    {
        eRet = aVisitor.visit( SetEntry(_aAccessor, it->get()) );
    }
    return eRet;
}
//-----------------------------------------------------------------------------

void SetNodeImpl::attach(Element const& aNewElement, Name const& aName)
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
    //  OSL_ENSURE(aActualName.isEmpty(), "WARNING: Wrongly named tree inserted in set node");
        aNewElement->renameTree(aName);

        aActualName = validatedName(aNewElement);
        if (aName !=aActualName )
        {
            OSL_ENSURE(false, "INTERNAL ERROR: Cannot rename tree in set node");
            throw Exception("INTERNAL ERROR: Cannot rename tree for insertion into set node");
        }
    }

//  if (bCommit) aNewElement->attachTo( directSetAccess_hack(getOriginalNodeAddress()), aName );
}
//-------------------------------------------------------------------------

void SetNodeImpl::detach(Element const& aOldElement)
{
    if (aOldElement.isValid())
    {
        aOldElement->detachTree();
//      if (bCommit) aOldElement->detachFrom( directSetAccess_hack(getOriginalNodeAddress()), validatedName(aOldElement));
    }
}
//-------------------------------------------------------------------------

Element SetNodeImpl::entryToElement(SetEntry const& _anEntry)
{
    ElementTreeImpl * pTree = _anEntry.tree();
    return Element(pTree, isInDefault(_anEntry));
}
//-------------------------------------------------------------------------


SetElementChangeImpl* SetNodeImpl::doAdjustChangedElement(data::Accessor const & _aAccessor, NodeChangesInformation& rLocalChanges, Name const& aName, Change const& _aElementChange)
{
    SetElementChangeImpl* pThisChange  = NULL;

    if (Element* pElement = getStoredElement(aName))
    {
        OSL_ASSERT(pElement->isValid());

        if (_aElementChange.ISA(SubtreeChange))
        {
            //OSL_ENSURE( !containsValues(), "Unexpected kind of change: Tree change applied to value set element" );

            SubtreeChange const& aSubtreeChange = static_cast<SubtreeChange const&>(_aElementChange);
            // recurse to element tree
            view::Tree aElementTree(_aAccessor,**pElement);

            view::getViewBehavior(aElementTree)->adjustToChanges(rLocalChanges, view::getRootNode(aElementTree), aSubtreeChange);
        }
        else if ( _aElementChange.ISA(ValueChange) )
        {
            //OSL_ENSURE( containsValues(), "Unexpected kind of change: Value change applied to tree set element" );

            ValueChange const& aValueChange = static_cast<ValueChange const&>(_aElementChange);

            // make an element for the old element
            std::auto_ptr<ValueNode> aOldNode = OTreeNodeConverter().createCorrespondingNode(aValueChange);
            aOldNode->setValue(aValueChange.getOldValue());

            bool bWasDefault = (aValueChange.getMode() == ValueChange::wasDefault);

            std::auto_ptr<INode> aBasePtr(aOldNode.release());
            Name aElementTypeName = getElementTemplate()->getName();
            data::TreeSegment aOldBaseTree = data::TreeSegment::createNew( aBasePtr, aElementTypeName.toString() );

            ElementTreeHolder aOldElement = new ElementTreeImpl(aOldBaseTree, getElementTemplate(), getTemplateProvider());

            OSL_ASSERT(aOldBaseTree.is()); // the tree took ownership
            OSL_ASSERT(aOldElement->isFree()); // the tree is free-floating

            pThisChange = implCreateReplace(_aAccessor,aName,*pElement,Element(aOldElement,bWasDefault));
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

void SetNodeImpl::initElements(TemplateProvider const& aTemplateProvider,TreeImpl& rParentTree,NodeOffset nPos,TreeDepth nDepth)
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

bool SetNodeImpl::implLoadElements(data::Accessor const& _aAccessor)
{
    if (m_aInit > 0)
    {
        OSL_ENSURE(!getElementTemplate().is() || getElementTemplate()->isInstanceTypeKnown(),"ERROR: Need a type-validated template to fill a set");
        OSL_ENSURE(getTemplateProvider().isValid() || getElementTemplate()->isInstanceValue(), "ERROR: Need a template provider to fill a non-primitive set");

        TreeDepth nDepth = m_aInit;
        implInitElements(this->getDataAccess(_aAccessor),nDepth);
        m_aInit = 0;

    }
    OSL_ASSERT(implHasLoadedElements());

    return m_aInit == 0;
}
//-----------------------------------------------------------------------------

void SetNodeImpl::implEnsureElementsLoaded(data::Accessor const& _aAccessor)
{
    if (!implLoadElements(_aAccessor))
        throw ConstraintViolation("Trying to access set elements beyond the loaded nestíng level");
}
//-----------------------------------------------------------------------------

void SetNodeImpl::implInitElements( data::SetNodeAccess const& _aNode, TreeDepth nDepth)
{
    TreeImpl* pThisTree = getParentTree();

    OSL_ENSURE(pThisTree,"Cannot load elements of a set that has no parent tree");

    CollectElementTrees aCollector( pThisTree->getViewBehavior(), pThisTree, getContextOffset(),
                                    nDepth, getElementTemplate(), getTemplateProvider() );
    aCollector.collect(_aNode);

    typedef CollectElementTrees::Collection::const_iterator Iter;
    for(Iter it = aCollector.collection.begin(), stop = aCollector.collection.end();
        it != stop; ++it)
    {
        implInitElement(implValidateElement(_aNode.accessor(),*it));
    }
}
//-------------------------------------------------------------------------

void SetNodeImpl::implInitElement(Element const& aNewElement)
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

Element SetNodeImpl::makeAdditionalElement(data::Accessor const& _aAccessor, rtl::Reference<view::ViewStrategy> const& _xStrategy, AddNode const& aAddNodeChange, TreeDepth nDepth)
{
    OSL_ENSURE(aAddNodeChange.wasInserted(), "Cannot integrate element that is not in tree yet");

    data::TreeAddress aAddedTree = aAddNodeChange.getInsertedTree();
    // need 'unsafe', because ownership would be gone when notifications are sent
    if (aAddedTree.is())
    {
        // OSL_ENSURE( pNode->ISA(ISubtree), "Type mismatch when adjusting to update: value element found in tree set");

        CollectElementTrees aCollector( _xStrategy, getParentTree(), getContextOffset(),
                                        nDepth, getElementTemplate(), getTemplateProvider() );

        data::TreeAccessor aElementAccess( _aAccessor, aAddedTree);

        return implValidateElement(_aAccessor, aCollector.create(aElementAccess));
    }

    return Element();
}
//-------------------------------------------------------------------------

Element SetNodeImpl::implValidateElement(data::Accessor const& _aAccessor, Element const& aNewElement)
{
    TemplateHolder aTemplate = getElementTemplate();
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

            view::Node aElementRoot = view::getRootNode( view::Tree(_aAccessor, *aNewElement) );

            OSL_ENSURE(aElementRoot.isValueNode(),"INTERNAL ERROR: Inserting complex type into value set node");

            view::ValueNode aValueNode(aElementRoot);
            UnoType aValueType = aValueNode.get_impl()->getValueType(aValueNode.accessor());

            OSL_ENSURE( aValueType.getTypeClass() != uno::TypeClass_INTERFACE,
                        "INTERNAL ERROR: Inserting complex type into value set node");

            UnoType aElementType    = aTemplate->getInstanceType();

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
                throw TypeMismatch( OUString(RTL_CONSTASCII_USTRINGPARAM("<Unknown> [Missing Template]")),
                                    aTemplate->getName().toString(),
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

        void diff(data::SetNodeAccess const& _aActualTree)
        {
            translate(m_rDefaultTree);
            visitElements(_aActualTree);
        }

    private:
        void translate(ISubtree& _rDefaultTree);
        void handleDefault(data::TreeSegment const & _pDefaultElement);
        void handleActual(data::TreeAccessor const& _aElement);

        virtual Result handle(data::TreeAccessor const& _aElement)
        { handleActual(_aElement); return CONTINUE; }
    };
    //-------------------------------------------------------------------------

    void DiffToDefault::translate(ISubtree& _rDefaultTree)
    {
        typedef CollectNames::NameList::const_iterator NameIter;

        OUString aTypeName = _rDefaultTree.getElementTemplateName();
        OSL_ENSURE(aTypeName.getLength(),"Cannot get element type for default set");

        CollectNames aCollector;
        aCollector.applyToChildren(_rDefaultTree);

        CollectNames::NameList const& aNames = aCollector.list();

        for(NameIter it = aNames.begin(); it != aNames.end(); ++it)
        {
            std::auto_ptr<INode> aChild = _rDefaultTree.removeChild(*it);
            handleDefault( data::TreeSegment::createNew(aChild,aTypeName) );
        }

    }
    //-------------------------------------------------------------------------

    void DiffToDefault::handleDefault(data::TreeSegment const &_pDefaultElement)
    {
        OSL_PRECOND(_pDefaultElement.is(), "Unexpected NULL default node");
        if (!_pDefaultElement.is()) return;

        OUString sName = _pDefaultElement.getName().toString();

        OSL_ENSURE(_pDefaultElement.getTreeData()->getAttributes().isDefault(), "Missing default state on default element tree");
        OSL_ENSURE(_pDefaultElement.getSegmentRootNode()->isDefault(), "Missing default attribute on default node");

        std::auto_ptr<AddNode> pAddIt( m_rChangeFactory.createAddNodeChange(_pDefaultElement, sName,true) );

        m_rChange.addChange(base_ptr(pAddIt));
    }
    //-------------------------------------------------------------------------

// -----------------------------------------------------------------------------
    void DiffToDefault::handleActual(data::TreeAccessor const& _aElement)
    {
        bool bDefaultElement = _aElement.getRootNode().isDefault();

        OUString sName = _aElement.getName().toString();

        if (Change* pDefaultNode = m_rChange.getChange(sName) )
        {

            if (pDefaultNode->ISA(AddNode))
            {
                AddNode* pAddIt = static_cast<AddNode*>(pDefaultNode);
                if (bDefaultElement)
                {
                    data::TreeSegment aDefaultTree = pAddIt->getNewTree();
                    m_rDefaultTree.addChild( aDefaultTree.cloneData(true) );

                    // no change needed - remove the change and recover the default
                    m_rChange.removeChange(sName);
                }
                else
                {
                   // OSL_ENSURE(!pAddIt->getReplacedNode().is(), "Duplicate node name in actual tree");

                   // pAddIt->expectReplacedNode(pActualNode);
                }
            }
            else
            {
                // should never happen
                OSL_ENSURE(pDefaultNode->ISA(RemoveNode), "Unexpected node type found in translated default tree");
                OSL_ENSURE(!pDefaultNode->ISA(RemoveNode), "Duplicate node name in actual tree");

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
void SetNodeImpl::implDifferenceToDefaultState(data::Accessor const& _aAccessor, SubtreeChange& _rChangeToDefault, ISubtree& _rDefaultTree) const
{
    DiffToDefault(_rChangeToDefault,_rDefaultTree).diff( getDataAccess(_aAccessor) );
}
//-----------------------------------------------------------------------------
void SetNodeImpl::convertChanges(NodeChangesInformation& rLocalChanges, data::Accessor const& _accessor, SubtreeChange const& rExternalChange,
                                     TreeDepth nDepth)
{
    OSL_ASSERT(nDepth > 0);

    if (TreeImpl* pParentTree = this->getParentTree())
    {
        NodeOffset nNode = getContextOffset();

        OSL_ENSURE(pParentTree->isValidNode(nNode), "Invalid context node in Set");
        OSL_ENSURE(view::Node(_accessor, *pParentTree, nNode).get_impl() == this,
                    "Wrong context node in Set");

        CollectChanges aCollector(rLocalChanges, _accessor, *pParentTree, nNode, getElementTemplate(), nDepth);

        aCollector.collectFromChildren(rExternalChange);
    }
    else
        OSL_ENSURE(false, "Missing context tree in Set");
}
//-----------------------------------------------------------------------------

    }
}

