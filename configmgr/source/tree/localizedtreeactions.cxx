/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: localizedtreeactions.cxx,v $
 * $Revision: 1.19 $
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

#include "localizedtreeactions.hxx"
#include "treeactions.hxx"
#include "builddata.hxx"
#include "subtree.hxx"
#include "matchlocale.hxx"
#include "typeconverter.hxx"
#include "change.hxx"
#include "treechangefactory.hxx"
#include "treenodefactory.hxx"
#include "strdecl.hxx"
#include <osl/diagnose.h>


//..........................................................................
namespace configmgr
{

//--------------------------------------------------------------------------
namespace
{
//--------------------------------------------------------------------------
    //==========================================================================
    //= OCloneForLocale
    //==========================================================================
    //= clones a subtree , in the process selecting only the best match locale
    //= from the set representation of localized values
    //==========================================================================
    class OCloneForLocale : public NodeAction
    {
        rtl::OUString           m_sTargetLocale;
        std::auto_ptr<INode>    m_pClone;
    public:
        OCloneForLocale(rtl::OUString const& aLocale) : m_sTargetLocale(aLocale) {}
        std::auto_ptr<INode> getResult() { return m_pClone; }

    private:
        void handle(ValueNode const& _aValue);
        void handle(ISubtree const&  _aSubtree);
    };
//--------------------------------------------------------------------------
    struct OCloneChildrenForLocale : NodeAction
    {
        ISubtree&               m_rParent;
        localehelper::FindBestLocale&           m_rLocaleMatcher;
    public:
        OCloneChildrenForLocale(ISubtree& _rParent, localehelper::FindBestLocale& _rLocaleMatcher)
            : m_rParent(_rParent)
            , m_rLocaleMatcher(_rLocaleMatcher)
        {}

        virtual void handle(ValueNode const& _aValue);
        virtual void handle(ISubtree const&  _aSubtree);
    };

//--------------------------------------------------------------------------
    struct OSelectForLocale : NodeAction
    {
        ValueNode const*    m_pFound;
        localehelper::FindBestLocale&       m_rLocaleMatcher;
    public:
        OSelectForLocale(localehelper::FindBestLocale& _rLocaleMatcher)
            : m_pFound(NULL)
            , m_rLocaleMatcher(_rLocaleMatcher)
        {}


        bool hasResult() const
        { return m_pFound != NULL; }

        ValueNode const* getResult() const
        { return m_pFound; }

    private:
        virtual void handle(ValueNode const& _aValue);
        virtual void handle(ISubtree const&  _aSubtree);

        void maybeSelect(ValueNode const& _aNode);
    };

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
    void OSelectForLocale::handle(ValueNode const& _aValue)
    {
        maybeSelect(_aValue);
    }
//--------------------------------------------------------------------------
       void OSelectForLocale::handle(ISubtree const&  /*_aSubtree*/)
    {
        OSL_ENSURE(false, "INTERNAL ERROR: Inconsistent data: Found a Subtree in a set of localized values");
    }
//--------------------------------------------------------------------------
    void OSelectForLocale::maybeSelect(ValueNode const& _aNode)
    {
        if (m_rLocaleMatcher.accept( localehelper::makeLocale(_aNode.getName()) ) )
            m_pFound = &_aNode;

        else
            OSL_ENSURE(m_pFound, "WARNING: Node Locale wasn't accepted, but no node had been found before");
    }

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
    std::auto_ptr< INode > implReduceLocalizedSet(ISubtree const&  _aSubtree, localehelper::FindBestLocale& _rLocaleMatcher)
    {
    // -- find the best-match locale -----------------------------
        _rLocaleMatcher.reset();

        OSelectForLocale aSelector(_rLocaleMatcher);

        aSelector.applyToChildren(_aSubtree);

        std::auto_ptr< INode > pResult;

    // -- look for a non-NULL value -----------------------------
        uno::Type aValueType;
        if (aSelector.hasResult())
        {
            ValueNode const& rSelected = *aSelector.getResult();

            aValueType = rSelected.getValueType();

            if (!rSelected.isNull()) // values are present - clone it from the values
            {
                pResult.reset ( new ValueNode(  _aSubtree.getName(),
                                                rSelected.getValue(), rSelected.getDefault(),
                                                _aSubtree.getAttributes()
                              ) );

            }
        }
        else // no entry - exract the type to be used from the template name
            aValueType = parseTemplateName(_aSubtree.getElementTemplateName());

    // --  create NULL value, if none was found -----------------------------
        // create a NULL of the found type
        if (pResult.get() == NULL)
        {
            pResult.reset( new ValueNode(  _aSubtree.getName(),
                                            aValueType,
                                            _aSubtree.getAttributes()
                         ) );
        }

    // -- -----------------------------
        OSL_ENSURE( aValueType != uno::Type(), "VOID result type found");
#if OSL_DEBUG_LEVEL > 0
        uno::Type aTemplateType = parseTemplateName( _aSubtree.getElementTemplateName() );
        OSL_ENSURE( ( aValueType == aTemplateType ) || ( aTemplateType.getTypeClass() == uno::TypeClass_ANY ),
                    "ERROR: Found Value Type doesn't match encoded value type in pseudo template name");
#endif
        OSL_POSTCOND( static_cast<ValueNode&>(*pResult).getValueType() == aValueType,
                      "ERROR: Resulting Value Type doesn't match original value type" );

        return pResult;
    }
//--------------------------------------------------------------------------
    std::auto_ptr< INode > implCloneForLocale(ISubtree const&  _aSubtree, localehelper::FindBestLocale& _rLocaleMatcher)
    {
        std::auto_ptr< INode > pClone;

        if (isLocalizedValueSet(_aSubtree))
        {
            pClone = implReduceLocalizedSet(_aSubtree, _rLocaleMatcher);
        }
        else
        {
            // ISubtree should get a clone(NoChildCopy) member ...
            std::auto_ptr< Subtree > pCloneTree( new Subtree(_aSubtree, treeop::NoChildCopy()) );

            OCloneChildrenForLocale aSubCloner(*pCloneTree,_rLocaleMatcher);

            aSubCloner.applyToChildren(_aSubtree);

            pClone.reset( pCloneTree.release() );
        }

        return pClone;
    }
//--------------------------------------------------------------------------
//--- OCloneChildrenForLocale:-----------------------------------------------------------------------

    void OCloneChildrenForLocale::handle(ValueNode const& _aValue)
    {
        // just a single value - nothing to do
        std::auto_ptr< INode > pClone( _aValue.clone() );

        m_rParent.addChild(pClone);

    }
//--------------------------------------------------------------------------
    void OCloneChildrenForLocale::handle(ISubtree const&  _aSubtree)
    {
        std::auto_ptr< INode > pClone = implCloneForLocale(_aSubtree,m_rLocaleMatcher);

        m_rParent.addChild(pClone);
    }

    //--------------------------------------------------------------------------
    //= OCloneForLocale

    //      rtl::OUString           m_sTargetLocale;
    //      std::auto_ptr<INode>    m_pClone;

    void OCloneForLocale::handle(ValueNode const& _aValue)
    {
        // just a single value - nothing to do
        std::auto_ptr< INode > pClone( _aValue.clone() );

        m_pClone = pClone;
    }
    //--------------------------------------------------------------------------
    void OCloneForLocale::handle(ISubtree const&  _aSubtree)
    {
        localehelper::FindBestLocale aLocaleMatcher( localehelper::makeLocale(m_sTargetLocale) );

        m_pClone = implCloneForLocale(_aSubtree,aLocaleMatcher);
    }
    //--------------------------------------------------------------------------

//--------------------------------------------------------------------------
} // anonymous namespace

//==========================================================================
// Helper function to invoke the previous ones properly

// convert to the given locale format, assuming the original representation was expanded
static std::auto_ptr<INode> impl_cloneExpandedForLocale(INode const* _pNode, rtl::OUString const& _sLocale)
{
    OSL_ASSERT(_pNode != NULL);

    if ( localehelper::designatesAllLocales(localehelper::makeLocale(_sLocale)) ) // from expanded to expanded
    {
        return _pNode->clone();
    }

    else // needs reduction
    {
        OCloneForLocale aCloner(_sLocale);
        aCloner.applyToNode(*_pNode);
        return aCloner.getResult();
    }
}
//--------------------------------------------------------------------------

// convert to the given locale format, assuming the original representation was expanded
static rtl::Reference< data::TreeSegment > old_cloneExpandedForLocale(rtl::OUString const& _sName, INode const* _pNode, rtl::OUString const& _sLocale)
{
    if (_pNode == NULL)
        return rtl::Reference< data::TreeSegment >();

    std::auto_ptr<INode> aResult;
    if (dynamic_cast< ISubtree const * >(_pNode) == 0) // simple value - nothing to reduce
        aResult = _pNode->clone();

    else
        aResult = impl_cloneExpandedForLocale(_pNode,_sLocale);

    return data::TreeSegment::create(_sName,aResult);
}
//--------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//--------------------------------------------------------------------------
rtl::Reference< data::TreeSegment > cloneExpandedForLocale(sharable::TreeFragment * tree, rtl::OUString const& _sLocale)
{
    std::auto_ptr<INode> aOldTree = data::convertTree(tree, true);

    return old_cloneExpandedForLocale(tree->getName(), aOldTree.get(), _sLocale);
}
//--------------------------------------------------------------------------
// convert to the given locale format, assuming the original representation was expanded
std::auto_ptr<INode> reduceExpandedForLocale(std::auto_ptr<ISubtree> _pNode, rtl::OUString const& _sLocale)
{
    std::auto_ptr<INode> aResult;

    if ( _pNode.get() == NULL ||                           // nothing to reduce
         localehelper::designatesAllLocales(localehelper::makeLocale(_sLocale)) ) // from expanded to expanded
    {
        aResult.reset( _pNode.release() );
    }

    else // needs reduction
    {
        OCloneForLocale aCloner(_sLocale);
        aCloner.applyToNode(*_pNode);
        aResult = aCloner.getResult();

        OSL_ENSURE(aResult.get(),"Cloning a tree for a locale unexpectedly produced NOTHING");
    }
    return aResult;
}
//--------------------------------------------------------------------------
namespace
{
//--------------------------------------------------------------------------
    class ExpandTreeForLocale : NodeModification
    {
        ISubtree & m_rParent;
        rtl::OUString const & m_aSourceLocale;

        ExpandTreeForLocale(ISubtree & _rParent,rtl::OUString const & _aSourceLocale)
        : m_rParent(_rParent)
        , m_aSourceLocale(_aSourceLocale)
        {}

        void handle(ISubtree& _rNode);
        void handle(ValueNode& _rNode);

        void substitute(std::auto_ptr<INode> _aExpanded);
    public:
        static void expand(ISubtree& _rTree, rtl::OUString const & _aSourceLocale)
        {
            ExpandTreeForLocale(_rTree,_aSourceLocale).applyToChildren(_rTree);
        }

        // returns NULL, if not a localized value
        static std::auto_ptr<ISubtree> expanded(ValueNode const& _aNode, rtl::OUString const & _aSourceLocale);
    };

//--------------------------------------------------------------------------
    class ExpandChangesForLocale : ChangeTreeModification
    {
        SubtreeChange & m_rParent;
        rtl::OUString const & m_aSourceLocale;

        ExpandChangesForLocale(SubtreeChange & _rParent,rtl::OUString const & _aSourceLocale)
        : m_rParent(_rParent)
        , m_aSourceLocale(_aSourceLocale)
        {}

        void handle(SubtreeChange& _rNode);
        void handle(ValueChange& _rNode);

        void handle(AddNode& _rNode);
        void handle(RemoveNode& _rNode);

        void substitute(std::auto_ptr<Change> _aExpanded);
    public:
        static void expand(SubtreeChange& _rTree, rtl::OUString const & _aSourceLocale)
        {
            ExpandChangesForLocale(_rTree,_aSourceLocale).applyToChildren(_rTree);
        }

        // returns NULL, if not a localized value
        static std::auto_ptr<SubtreeChange> expanded(ValueChange const& _aNode, rtl::OUString const & _aSourceLocale);
    };
//--------------------------------------------------------------------------
inline
void ExpandTreeForLocale::substitute(std::auto_ptr<INode> _aExpanded)
{
    m_rParent.removeChild(_aExpanded->getName());
    m_rParent.addChild(_aExpanded);
}

//--------------------------------------------------------------------------
void ExpandTreeForLocale::handle(ISubtree& _rNode)
{
    expand(_rNode,m_aSourceLocale);
}

//--------------------------------------------------------------------------
void ExpandTreeForLocale::handle(ValueNode& _rNode)
{
    std::auto_ptr<ISubtree>  aExpanded = expanded(_rNode,m_aSourceLocale);

    if (aExpanded.get())
        substitute( base_ptr(aExpanded) );
}

//--------------------------------------------------------------------------

std::auto_ptr<ISubtree>  ExpandTreeForLocale::expanded(ValueNode const& _aNode, rtl::OUString const & _aSourceLocale)
{
    if (!_aNode.isLocalized()) return std::auto_ptr<ISubtree>();

    OTreeNodeFactory & rFactory = getDefaultTreeNodeFactory();

    node::Attributes aValueAttributes = _aNode.getAttributes();

    aValueAttributes.setLocalized(false);
    if (aValueAttributes.state() == node::isMerged)
        aValueAttributes.setState( node::isReplaced );


    std::auto_ptr<ValueNode> aValue = _aNode.isNull()
                ? rFactory.createNullValueNode(_aSourceLocale,_aNode.getValueType(),aValueAttributes)
                : rFactory.createValueNode(_aSourceLocale,_aNode.getValue(),aValueAttributes);

    std::auto_ptr<ISubtree> aRet = rFactory.createSetNode(  _aNode.getName(),
                                                            toTemplateName(_aNode.getValueType()),
                                                            TEMPLATE_MODULE_LOCALIZED_VALUE,
                                                            _aNode.getAttributes() );

    aRet->addChild(base_ptr(aValue));

    return aRet;
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
inline
void ExpandChangesForLocale::substitute(std::auto_ptr<Change> _aExpanded)
{
    m_rParent.removeChange(_aExpanded->getNodeName());
    m_rParent.addChange(_aExpanded);
}

//--------------------------------------------------------------------------

void ExpandChangesForLocale::handle(SubtreeChange& _rNode)
{
    expand(_rNode,m_aSourceLocale);
}

//--------------------------------------------------------------------------
void ExpandChangesForLocale::handle(ValueChange& _rNode)
{
    std::auto_ptr<SubtreeChange> aExpanded = expanded(_rNode,m_aSourceLocale);

    if (aExpanded.get())
        substitute( base_ptr( aExpanded ) );
}

//--------------------------------------------------------------------------
void ExpandChangesForLocale::handle(AddNode& _rNode)
{
    rtl::Reference< data::TreeSegment > seg(_rNode.getNewTree());
    std::auto_ptr<INode> pAdded(data::convertTree(seg.is() ? seg->fragment : 0, false));
    if (pAdded.get() != NULL)
    {
        std::auto_ptr<INode> pExpanded;
        if (ISubtree * pAddedTree = pAdded->asISubtree())
        {
            ExpandTreeForLocale::expand(*pAddedTree,m_aSourceLocale);
            pExpanded = pAdded;
        }
        else if(ValueNode * pAddedValue = pAdded->asValueNode())
        {
            pExpanded = base_ptr(ExpandTreeForLocale::expanded(*pAddedValue,m_aSourceLocale));
        }
        else
            OSL_ENSURE(false, "Cannot expand unknown Node type (found in AddNode)");


        if (pExpanded.get())
        {
            rtl::Reference< data::TreeSegment > aExpanded = data::TreeSegment::create( _rNode.getNodeName(),pExpanded);
            std::auto_ptr<AddNode> aExpandedAdd( new AddNode( aExpanded, _rNode.getNodeName(), _rNode.isToDefault() ) );

            if (_rNode.isReplacing()) aExpandedAdd->setReplacing();

            substitute( base_ptr( aExpandedAdd ) );
        }
    }
    else
        OSL_ENSURE(false, "Cannot expand AddNode without content");
}

//--------------------------------------------------------------------------
void ExpandChangesForLocale::handle(RemoveNode& )
{
    // nothing to do
}

//--------------------------------------------------------------------------
std::auto_ptr<SubtreeChange> ExpandChangesForLocale::expanded(ValueChange const& _aNode, rtl::OUString const & _aSourceLocale)
{
    std::auto_ptr<SubtreeChange> aRet;
    if (_aNode.isLocalizedValue())
    {
        OTreeNodeFactory & rNodeFactory = getDefaultTreeNodeFactory();
        OTreeChangeFactory & rFactory = getDefaultTreeChangeFactory();

        node::Attributes aValueAttributes = _aNode.getAttributes();

        aValueAttributes.setLocalized(false);
        if (aValueAttributes.state() == node::isMerged)
            aValueAttributes.setState( node::isReplaced );


        rtl::OUString const sTemplateName = toTemplateName(_aNode.getValueType());

        std::auto_ptr<ValueNode> aValue = _aNode.getNewValue().hasValue()
                    ? rNodeFactory.createValueNode(sTemplateName,_aNode.getNewValue(),aValueAttributes)
                    : rNodeFactory.createNullValueNode(sTemplateName,_aNode.getValueType(),aValueAttributes);

        rtl::Reference< data::TreeSegment > aValueSegment = data::TreeSegment::create(_aSourceLocale, base_ptr(aValue));

        std::auto_ptr<AddNode> aAddValue = rFactory.createAddNodeChange(aValueSegment,_aSourceLocale,_aNode.isToDefault());
        aAddValue->setReplacing();

        aRet = rFactory.createSetNodeChange( _aNode.getNodeName(),
                                            sTemplateName,
                                            TEMPLATE_MODULE_LOCALIZED_VALUE,
                                            _aNode.getAttributes() );

        aRet->addChange(base_ptr(aAddValue));
    }
    return aRet;
}

//--------------------------------------------------------------------------
} // anonymous namespace
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
//..........................................................................
}   // namespace configmgr
//..........................................................................


