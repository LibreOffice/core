/*************************************************************************
 *
 *  $RCSfile: localizedtreeactions.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: jb $ $Date: 2001-04-05 14:46:29 $
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

#include "treeactions.hxx"
#include "cmtree.hxx"
#include "matchlocale.hxx"
#include "typeconverter.hxx"

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
        _rLocaleMatcher.reset();

        OSelectForLocale aSelector(_rLocaleMatcher);

        aSelector.applyToChildren(_aSubtree);

        std::auto_ptr< INode > pResult;
        if (aSelector.hasResult())
        {
            ValueNode const& rSelected = *aSelector.getResult();
            OSL_ENSURE( rSelected.getValueType() == parseTemplateName(_aSubtree.getElementTemplateName()),
                        "ERROR: Found Value Type doesn't match encoded value type in pseudo template name" );

            std::auto_ptr< ValueNode > pClonedValue(
                new ValueNode(  _aSubtree.getName(),
                                rSelected.getValue(), rSelected.getDefault(),
                                _aSubtree.getAttributes() ) );

            pResult.reset( pClonedValue.release() );
        }
        else // create a NULL
        {
            uno::Type aValueType = parseTemplateName(_aSubtree.getElementTemplateName());

            std::auto_ptr< ValueNode > pClonedValue(
                new ValueNode(  _aSubtree.getName(),
                                aValueType,
                                _aSubtree.getAttributes() ) );

            pResult.reset( pClonedValue.release() );
        }
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

} // anonymous namespace

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

//==========================================================================
// Helper function to invoke the previous ones properly

// convert to the given locale format, no matter what the original representation
std::auto_ptr<INode> cloneForLocale(INode const* _pNode, OUString const& _sLocale)
{
    using namespace localehelper;
    OSL_ENSURE( !designatesAllLocales(makeLocale(_sLocale)),
                "WARNING: This function doesn't work from single values to  expanded sets so far");

    return cloneExpandedForLocale(_pNode,_sLocale);
}

// convert to the given locale format, assuming the original representation was expanded
std::auto_ptr<INode> cloneExpandedForLocale(INode const* _pNode, OUString const& _sLocale)
{
    using namespace localehelper;

    if (_pNode == NULL)
        return std::auto_ptr< INode >();

    else if ( designatesAllLocales(makeLocale(_sLocale)) ) // from expanded to expanded
        return _pNode->clone();

    else // needs reduction
    {
        OCloneForLocale aCloner(_sLocale);
        aCloner.applyToNode(*_pNode);
        return aCloner.getResult();
    }
}


//--------------------------------------------------------------------------

//..........................................................................
}   // namespace configmgr
//..........................................................................


