/*************************************************************************
 *
 *  $RCSfile: localizedtreeactions.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 16:19:38 $
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

#include "localizedtreeactions.hxx"

#ifndef _CONFIGMGR_TREEACTIONS_HXX_
#include "treeactions.hxx"
#endif
#ifndef CONFIGMGR_BUILDDATA_HXX
#include "builddata.hxx"
#endif
#ifndef CONFIGMGR_TREEACCESSOR_HXX
#include "treeaccessor.hxx"
#endif
#ifndef CONFIGMGR_SUBTREE_HXX
#include "subtree.hxx"
#endif
#ifndef CONFIGMGR_MATCHLOCALE_HXX
#include "matchlocale.hxx"
#endif
#ifndef CONFIGMGR_TYPECONVERTER_HXX
#include "typeconverter.hxx"
#endif
#ifndef CONFIGMGR_CHANGE_HXX
#include "change.hxx"
#endif
#ifndef CONFIGMGR_TREE_CHANGEFACTORY_HXX
#include "treechangefactory.hxx"
#endif
#ifndef CONFIGMGR_TREE_NODEFACTORY_HXX
#include "treenodefactory.hxx"
#endif
#ifndef _CONFIGMGR_STRDECL_HXX_
#include "strdecl.hxx"
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif


//..........................................................................
namespace configmgr
{

//--------------------------------------------------------------------------
namespace
{
    using localehelper::FindBestLocale;
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
        OCloneForLocale(OUString const& aLocale) : m_sTargetLocale(aLocale) {}
        std::auto_ptr<INode> getResult() { return m_pClone; }

    private:
        void handle(ValueNode const& _aValue);
        void handle(ISubtree const&  _aSubtree);
    };
//--------------------------------------------------------------------------
    struct OCloneChildrenForLocale : NodeAction
    {
        ISubtree&               m_rParent;
        FindBestLocale&         m_rLocaleMatcher;
    public:
        OCloneChildrenForLocale(ISubtree& _rParent, FindBestLocale& _rLocaleMatcher)
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
        FindBestLocale&     m_rLocaleMatcher;
    public:
        OSelectForLocale(FindBestLocale& _rLocaleMatcher)
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
    void OSelectForLocale::handle(ISubtree const&  _aSubtree)
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
    std::auto_ptr< INode > implReduceLocalizedSet(ISubtree const&  _aSubtree, FindBestLocale& _rLocaleMatcher)
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
        OSL_ENSURE( aValueType == parseTemplateName(_aSubtree.getElementTemplateName()),
                    "ERROR: Found Value Type doesn't match encoded value type in pseudo template name");
        OSL_POSTCOND( static_cast<ValueNode&>(*pResult).getValueType() == aValueType,
                      "ERROR: Resulting Value Type doesn't match original value type" );

        return pResult;
    }
//--------------------------------------------------------------------------
    std::auto_ptr< INode > implCloneForLocale(ISubtree const&  _aSubtree, FindBestLocale& _rLocaleMatcher)
    {
        std::auto_ptr< INode > pClone;

        if (isLocalizedValueSet(_aSubtree))
        {
            pClone = implReduceLocalizedSet(_aSubtree, _rLocaleMatcher);
        }
        else
        {
            // ISubtree should get a clone(NoChildCopy) member ...
            std::auto_ptr< Subtree > pCloneTree( new Subtree(_aSubtree, Subtree::NoChildCopy()) );

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
        FindBestLocale aLocaleMatcher( localehelper::makeLocale(m_sTargetLocale) );

        m_pClone = implCloneForLocale(_aSubtree,aLocaleMatcher);
    }
    //--------------------------------------------------------------------------

//--------------------------------------------------------------------------
} // anonymous namespace

//==========================================================================
// Helper function to invoke the previous ones properly

// convert to the given locale format, assuming the original representation was expanded
static std::auto_ptr<INode> impl_cloneExpandedForLocale(INode const* _pNode, OUString const& _sLocale)
{
    using namespace localehelper;

    OSL_ASSERT(_pNode != NULL);

    if ( designatesAllLocales(makeLocale(_sLocale)) ) // from expanded to expanded
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
static data::TreeSegment old_cloneExpandedForLocale(OUString const& _sName, INode const* _pNode, OUString const& _sLocale)
{
    using data::TreeSegment;

    if (_pNode == NULL)
        return TreeSegment();

    std::auto_ptr<INode> aResult;
    if ( !_pNode->ISA( ISubtree )  ) // simple value - nothing to reduce
        aResult = _pNode->clone();

    else
        aResult = impl_cloneExpandedForLocale(_pNode,_sLocale);

    return TreeSegment::createNew(_sName,aResult);
}
//--------------------------------------------------------------------------

// convert to the given locale format, no matter what the original representation
data::TreeSegment cloneForLocale(INode const* _pNode, OUString const& _sLocale)
{
    using namespace localehelper;
    OSL_ENSURE( !designatesAllLocales(makeLocale(_sLocale)),
                "WARNING: This function doesn't work from single values to  expanded sets so far");

    OUString sName = _pNode ? _pNode->getName() : OUString();
    return old_cloneExpandedForLocale(sName, _pNode,_sLocale);
}
//--------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//--------------------------------------------------------------------------
data::TreeSegment cloneExpandedForLocale(data::TreeAccessor const & _aTree, OUString const& _sLocale)
{
    std::auto_ptr<INode> aOldTree = data::convertTree(_aTree,true);

    return old_cloneExpandedForLocale(_aTree.getName().toString(), aOldTree.get(),_sLocale);
}
// -----------------------------------------------------------------------------
//--------------------------------------------------------------------------
/*
// convert to the given locale format, assuming the original representation was expanded
data::TreeSegment cloneExpandedForLocale(ISubtree const* _pNode, OUString const& _sLocale)
{
    using data::TreeSegment;

    if (_pNode == NULL)
        return TreeSegment();

    else
        return TreeSegment::createNew(impl_cloneExpandedForLocale(_pNode,_sLocale));
}
//--------------------------------------------------------------------------
*/
// convert to the given locale format, assuming the original representation was expanded
std::auto_ptr<INode> reduceExpandedForLocale(std::auto_ptr<ISubtree> _pNode, OUString const& _sLocale)
{
    using namespace localehelper;

    std::auto_ptr<INode> aResult;

    if ( _pNode.get() == NULL ||                           // nothing to reduce
         designatesAllLocales(makeLocale(_sLocale)) ) // from expanded to expanded
    {
        aResult.reset( _pNode.release() );
    }

    else // needs reduction
    {
        OUString const aTreeId = _pNode->getId();

        OCloneForLocale aCloner(_sLocale);
        aCloner.applyToNode(*_pNode);
        aResult = aCloner.getResult();

        OSL_ENSURE(aResult.get(),"Cloning a tree for a locale unexpectedly produced NOTHING");
        if (aResult.get())
            if (ISubtree* pResultTree = aResult->asISubtree())
                OIdPropagator::propagateIdToTree(aTreeId, *pResultTree);
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
        OUString const & m_aSourceLocale;

        ExpandTreeForLocale(ISubtree & _rParent,OUString const & _aSourceLocale)
        : m_rParent(_rParent)
        , m_aSourceLocale(_aSourceLocale)
        {}

        void handle(ISubtree& _rNode);
        void handle(ValueNode& _rNode);

        void substitute(std::auto_ptr<INode> _aExpanded);
    public:
        static void expand(ISubtree& _rTree, OUString const & _aSourceLocale)
        {
            ExpandTreeForLocale(_rTree,_aSourceLocale).applyToChildren(_rTree);
        }

        // returns NULL, if not a localized value
        static std::auto_ptr<ISubtree> expanded(ValueNode const& _aNode, OUString const & _aSourceLocale);
    };

//--------------------------------------------------------------------------
    class ExpandChangesForLocale : ChangeTreeModification
    {
        SubtreeChange & m_rParent;
        OUString const & m_aSourceLocale;

        ExpandChangesForLocale(SubtreeChange & _rParent,OUString const & _aSourceLocale)
        : m_rParent(_rParent)
        , m_aSourceLocale(_aSourceLocale)
        {}

        void handle(SubtreeChange& _rNode);
        void handle(ValueChange& _rNode);

        void handle(AddNode& _rNode);
        void handle(RemoveNode& _rNode);

        void substitute(std::auto_ptr<Change> _aExpanded);
    public:
        static void expand(SubtreeChange& _rTree, OUString const & _aSourceLocale)
        {
            ExpandChangesForLocale(_rTree,_aSourceLocale).applyToChildren(_rTree);
        }

        // returns NULL, if not a localized value
        static std::auto_ptr<SubtreeChange> expanded(ValueChange const& _aNode, OUString const & _aSourceLocale);
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

std::auto_ptr<ISubtree>  ExpandTreeForLocale::expanded(ValueNode const& _aNode, OUString const & _aSourceLocale)
{
    if (!_aNode.isLocalized()) return std::auto_ptr<ISubtree>();

    OTreeNodeFactory & rFactory = getDefaultTreeNodeFactory();

    node::Attributes aValueAttributes = _aNode.getAttributes();

    aValueAttributes.bLocalized = false;
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
    std::auto_ptr<INode> pAdded = _rNode.getNewTree().cloneData(false);
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
            data::TreeSegment aExpanded = data::TreeSegment::createNew( _rNode.getNodeName(),pExpanded);
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
std::auto_ptr<SubtreeChange> ExpandChangesForLocale::expanded(ValueChange const& _aNode, OUString const & _aSourceLocale)
{
    std::auto_ptr<SubtreeChange> aRet;
    if (_aNode.isLocalizedValue())
    {
        OTreeNodeFactory & rNodeFactory = getDefaultTreeNodeFactory();
        OTreeChangeFactory & rFactory = getDefaultTreeChangeFactory();

        node::Attributes aValueAttributes = _aNode.getAttributes();

        aValueAttributes.bLocalized = false;
        if (aValueAttributes.state() == node::isMerged)
            aValueAttributes.setState( node::isReplaced );


        OUString const sTemplateName = toTemplateName(_aNode.getValueType());

        std::auto_ptr<ValueNode> aValue = _aNode.getNewValue().hasValue()
                    ? rNodeFactory.createValueNode(sTemplateName,_aNode.getNewValue(),aValueAttributes)
                    : rNodeFactory.createNullValueNode(sTemplateName,_aNode.getValueType(),aValueAttributes);

        data::TreeSegment aValueSegment = data::TreeSegment::createNew(_aSourceLocale, base_ptr(aValue));

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

void expandForLocale(ISubtree& _rNode, OUString const& _sLocale)
{
    using namespace localehelper;

    if ( ! designatesAllLocales(makeLocale(_sLocale)) ) // nothing to reduce
        ExpandTreeForLocale::expand(_rNode,_sLocale);
}
//--------------------------------------------------------------------------

void expandForLocale(SubtreeChange& _rNode, OUString const& _sLocale)
{
    using namespace localehelper;

    if ( ! designatesAllLocales(makeLocale(_sLocale)) ) // nothing to reduce
        ExpandChangesForLocale::expand(_rNode,_sLocale);
}

//--------------------------------------------------------------------------
//..........................................................................
}   // namespace configmgr
//..........................................................................


