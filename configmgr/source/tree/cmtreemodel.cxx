/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: cmtreemodel.cxx,v $
 * $Revision: 1.24 $
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

#include "valuenode.hxx"
#include "change.hxx"
#include "configexcept.hxx"
#include "strdecl.hxx"
#include <osl/diagnose.h>

#ifndef INCLUDED_ALGORITHM
#define INCLUDED_ALGORITHM
#include <algorithm>
#endif

//..........................................................................
namespace configmgr
{
//==========================================================================

bool isLocalizedValueSet(ISubtree const& _aSubtree)
{
    if ( !_aSubtree.isSetNode())    return false;
    if ( !_aSubtree.isLocalized())  return false;
    if ( !_aSubtree.getElementTemplateModule().equals(TEMPLATE_MODULE_LOCALIZED_VALUE) )    return false;
    return true;
}
// -----------------------------------------------------------------------------

bool isLocalizedValueSet(SubtreeChange const& _aSubtree)
{
    if ( !_aSubtree.isSetNodeChange())  return false;
    if ( !_aSubtree.isLocalizedContainer()) return false;
    if ( !_aSubtree.getElementTemplateModule().equals(TEMPLATE_MODULE_LOCALIZED_VALUE) )    return false;
    return true;
}

// -----------------------------------------------------------------------------

bool isValueSet(SubtreeChange const& _aSubtree)
{
    if ( !_aSubtree.isSetNodeChange())  return false;
    if ( !_aSubtree.getElementTemplateModule().equals(TEMPLATE_MODULE_NATIVE_VALUE) )   return false;
    return true;
}
//==========================================================================

//==========================================================================
//= Change
//==========================================================================
void Change::swap(Change& aOther)
{
    std::swap(m_aName, aOther.m_aName);
    std::swap(m_bIsToDefault, aOther.m_bIsToDefault);
}

//==========================================================================
//= SubtreeChange
//==========================================================================
//--------------------------------------------------------------------------
void SubtreeChange::swap(SubtreeChange& aOther)
{
    Change::swap(aOther);
    m_aChanges.swap(aOther.m_aChanges);
    std::swap(m_sTemplateName, aOther.m_sTemplateName);
    std::swap(m_sTemplateModule, aOther.m_sTemplateModule);
    std::swap(m_aAttributes, aOther.m_aAttributes);
}

//--------------------------------------------------------------------------
SubtreeChange::~SubtreeChange()
{
    for(Children::iterator aIter = m_aChanges.begin();
        aIter != m_aChanges.end();
        ++aIter)
    {
        // Change* pChange = aIter->second;
        delete aIter->second;
    }
}

// -----------------------------------------------------------------------------
SubtreeChange::SubtreeChange(const SubtreeChange& _aObj, treeop::DeepChildCopy)
        :Change(_aObj),
         m_sTemplateName(_aObj.m_sTemplateName),
         m_sTemplateModule(_aObj.m_sTemplateModule),
         m_aAttributes(_aObj.m_aAttributes)
{
    for(Children::const_iterator aIter = _aObj.m_aChanges.begin();
        aIter != _aObj.m_aChanges.end();
        ++aIter)
    {
        OSL_ASSERT(aIter->second);
        Children::value_type aCopy(aIter->first, aIter->second->clone().release());
        m_aChanges.insert(m_aChanges.end(), aCopy);
    }
}

// -----------------------------------------------------------------------------
std::auto_ptr<Change> SubtreeChange::clone() const
{
    return std::auto_ptr<Change>(new SubtreeChange(*this, treeop::DeepChildCopy()));
}
//--------------------------------------------------------------------------
void SubtreeChange::addChange(std::auto_ptr<Change> aChange)
{
    rtl::OUString aNodeName(aChange->getNodeName());
    m_aChanges.find(aNodeName);
    OSL_ENSURE(m_aChanges.end() == m_aChanges.find(aNodeName),
        "SubtreeChange::addChange : overwriting an existent change !");
    delete m_aChanges[aNodeName];
    m_aChanges[aNodeName] = aChange.release();
}

//--------------------------------------------------------------------------
::std::auto_ptr<Change> SubtreeChange::removeChange(rtl::OUString const& _rName)
{
    Children::iterator aIter = m_aChanges.find(_rName);

    ::std::auto_ptr<Change> aReturn;
    if (m_aChanges.end() != aIter)
    {
        aReturn = ::std::auto_ptr<Change>(aIter->second);
        m_aChanges.erase(aIter);
    }
    return aReturn;
}

//--------------------------------------------------------------------------
Change* SubtreeChange::getChange(rtl::OUString const& _rName)
{
    return doGetChild(_rName);
}

//--------------------------------------------------------------------------
Change const* SubtreeChange::getChange(rtl::OUString const& _rName) const
{
    return doGetChild(_rName);
}

//--------------------------------------------------------------------------
void SubtreeChange::dispatch(ChangeTreeAction& _anAction) const
{
    _anAction.handle(*this);
}

//--------------------------------------------------------------------------
void SubtreeChange::dispatch(ChangeTreeModification& _anAction)
{
    _anAction.handle(*this);
}

//--------------------------------------------------------------------------
void SubtreeChange::forEachChange(ChangeTreeAction& _anAction) const
{
    ::std::map< ::rtl::OUString,Change* >::const_iterator aIter = m_aChanges.begin();
    while (aIter != m_aChanges.end())
    {
        (aIter++)->second->dispatch(_anAction);
    }
}

//--------------------------------------------------------------------------
void SubtreeChange::forEachChange(ChangeTreeModification& _anAction)
{
    ::std::map< ::rtl::OUString,Change* >::const_iterator aIter = m_aChanges.begin();
    while (aIter != m_aChanges.end())
    {
        (aIter++)->second->dispatch(_anAction);
    }
}

//--------------------------------------------------------------------------
Change* SubtreeChange::doGetChild(rtl::OUString const& _rName) const
{
    Children::const_iterator aIter = m_aChanges.find(_rName);
    return (aIter != m_aChanges.end()) ? aIter->second : NULL;
}

//--------------------------------------------------------------------------
uno::Sequence< rtl::OUString > SubtreeChange::elementNames() const
{
    uno::Sequence< rtl::OUString > aReturn(size());
    rtl::OUString* pReturn = aReturn.getArray();

    for (   Children::const_iterator aCollector = m_aChanges.begin();
            aCollector != m_aChanges.end();
            ++aCollector, ++pReturn
        )
    {
        *pReturn = aCollector->first;
    }

    return aReturn;
}

//--------------------------------------------------------------------------
SubtreeChange::MutatingChildIterator SubtreeChange::begin_changes() throw()
{
    return MutatingChildIterator(m_aChanges.begin());
}

//--------------------------------------------------------------------------
SubtreeChange::MutatingChildIterator SubtreeChange::end_changes() throw()
{
    return MutatingChildIterator(m_aChanges.end());
}

//--------------------------------------------------------------------------
SubtreeChange::ChildIterator SubtreeChange::begin() const throw()
{
    return ChildIterator(this);
}

//--------------------------------------------------------------------------
SubtreeChange::ChildIterator SubtreeChange::end() const throw()
{
    return ChildIterator(this, ChildIterator::EndPos());
}

//--------------------------------------------------------------------------
SubtreeChange::ChildIterator::ChildIterator(const SubtreeChange* _pTree)
    :m_aNames(_pTree->elementNames())
    ,m_pTree(_pTree)
    ,m_nPos(0)
{
}

//--------------------------------------------------------------------------
SubtreeChange::ChildIterator::ChildIterator(const SubtreeChange* _pTree, struct EndPos)
    :m_aNames(_pTree->elementNames())
    ,m_pTree(_pTree)
    ,m_nPos(_pTree->size())
{
}

//--------------------------------------------------------------------------
Change const & SubtreeChange::ChildIterator::operator*() const
{
    OSL_ENSURE(isValid(), "SubtreeChange::ChildIterator::operator* : invalid iterator !");

    if (!isValid())
        throw configuration::Exception("INTERNAL ERROR: Invalid SubtreeChange::ChildIterator dereferenced");

    return *m_pTree->getChange(m_aNames[m_nPos]);
}

//--------------------------------------------------------------------------
Change const * SubtreeChange::ChildIterator::operator->() const
{
    if (isValid())
        return m_pTree->getChange(m_aNames[m_nPos]);
    OSL_ENSURE(sal_False, "SubtreeChange::ChildIterator::operator-> : invalid iterator !");
    return NULL;
}

//--------------------------------------------------------------------------
SubtreeChange::ChildIterator& SubtreeChange::ChildIterator::operator++()
{
    OSL_ENSURE(m_nPos < m_aNames.getLength(), "SubtreeChange::ChildIterator : can't increment the end iterator !");
    if (m_nPos < m_aNames.getLength())
        ++m_nPos;
    return *this;
}

//--------------------------------------------------------------------------
SubtreeChange::ChildIterator& SubtreeChange::ChildIterator::operator--()
{
    OSL_ENSURE(m_nPos > 0, "SubtreeChange::ChildIterator : can't decrement the begin iterator !");
    if (m_nPos > 0)
        --m_nPos;
    return *this;
}

//--------------------------------------------------------------------------
bool operator==(SubtreeChange::ChildIterator const& lhs, SubtreeChange::ChildIterator const& rhs)
{
    return (lhs.m_pTree == rhs.m_pTree) && (lhs.m_nPos == rhs.m_nPos);
}

//==========================================================================
//= SubtreeChangeReferrer
//==========================================================================
//--------------------------------------------------------------------------
SubtreeChangeReferrer::SubtreeChangeReferrer(const SubtreeChange& _rSource)
    :SubtreeChange(_rSource, treeop::NoChildCopy())
{
    ChildIterator aSourceChildren = _rSource.begin();
    while (aSourceChildren != _rSource.end())
    {
        const Change* pChange = &*aSourceChildren;
        OSL_ENSURE(pChange, "SubtreeChangeReferrer::SubtreeChangeReferrer : invalid change !");
        if  (dynamic_cast< ValueChange const * >(pChange) != 0 ||
             dynamic_cast< RemoveNode const * >(pChange) != 0 ||
             dynamic_cast< AddNode const * >(pChange) != 0)
            SubtreeChange::addChange(::std::auto_ptr<Change>(const_cast<Change*>(pChange)));
        else if (dynamic_cast< SubtreeChange const * >(pChange) != 0 ||
                 dynamic_cast< SubtreeChangeReferrer const * >(pChange) != 0)
        {
            SubtreeChange::addChange(::std::auto_ptr<Change>(new SubtreeChangeReferrer(*static_cast<const SubtreeChange*>(pChange))));
        }
        else
            OSL_ENSURE(sal_False, "SubtreeChangeReferrer::SubtreeChangeReferrer : unknown changes type !");

        ++aSourceChildren;
    }
}

//--------------------------------------------------------------------------
SubtreeChangeReferrer::~SubtreeChangeReferrer()
{
    for (   Children::iterator aChildren = m_aChanges.begin();
            aChildren != m_aChanges.end();

        )
    {
        const Change* pChange = aChildren->second;
        Children::iterator aCurrent = aChildren++;

        if  (dynamic_cast< ValueChange const * >(pChange) != 0 ||
             dynamic_cast< RemoveNode const * >(pChange) != 0 ||
             dynamic_cast< AddNode const * >(pChange) != 0)
        {
            // we just hold references to the non-SubtreeChange-objects, so don't delete them
            m_aChanges.erase(aCurrent);
        }
        else if (dynamic_cast< SubtreeChange const * >(pChange) != 0 ||
                 dynamic_cast< SubtreeChangeReferrer const * >(pChange) != 0)
        {
            // nothing to do
        }
        else
            OSL_ENSURE(sal_False, "SubtreeChangeReferrer::~SubtreeChangeReferrer : unknown changes type !");
    }

    // the base class will remove the remaining SubtreeChanges, which are SubtreeChangeReferrer's in real
}

//..........................................................................
}   // namespace configmgr
//..........................................................................


