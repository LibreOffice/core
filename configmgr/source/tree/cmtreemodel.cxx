/*************************************************************************
 *
 *  $RCSfile: cmtreemodel.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jb $ $Date: 2000-11-10 12:13:43 $
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
#ifndef CONFIGMGR_CMTREEMODEL_HXX
#include "cmtreemodel.hxx"
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#include "configexcept.hxx"

#include <stl/algorithm>

//..........................................................................
namespace configmgr
{
//..........................................................................
//==========================================================================
//= Change
//==========================================================================
void Change::swap(Change& aOther)
{
    std::swap(m_aName, aOther.m_aName);
}

//==========================================================================
//= SubtreeChange
//==========================================================================
//--------------------------------------------------------------------------
SubtreeChange::SubtreeChange(OUString const& _rName)
    :Change(_rName)
{
}
//--------------------------------------------------------------------------

void SubtreeChange::swap(SubtreeChange& aOther)
{
    Change::swap(aOther);
    m_mChanges.swap(aOther.m_mChanges);
    std::swap(m_sTemplateName, aOther.m_sTemplateName);
}

//--------------------------------------------------------------------------
SubtreeChange::~SubtreeChange()
{
    for(Children::iterator aIter = m_mChanges.begin();
        aIter != m_mChanges.end();
        ++aIter)
    {
        // Change* pChange = aIter->second;
        delete aIter->second;
    }
}

//--------------------------------------------------------------------------
void SubtreeChange::addChange(std::auto_ptr<Change> aChange)
{
    OUString aNodeName(aChange->getNodeName());
    OSL_ENSHURE(m_mChanges.end() == m_mChanges.find(aNodeName),
        "SubtreeChange::addChange : overwriting an existent change !");
    delete m_mChanges[aNodeName];
    m_mChanges[aNodeName] = aChange.release();
}

//--------------------------------------------------------------------------
::std::auto_ptr<Change> SubtreeChange::removeChange(OUString const& _rName)
{
    Children::iterator aIter = m_mChanges.find(_rName);

    ::std::auto_ptr<Change> aReturn;
    if (m_mChanges.end() != aIter)
    {
        aReturn = ::std::auto_ptr<Change>(aIter->second);
        m_mChanges.erase(aIter);
    }
    return aReturn;
}

//--------------------------------------------------------------------------
Change* SubtreeChange::getChange(OUString const& _rName)
{
    return doGetChild(_rName);
}

//--------------------------------------------------------------------------
Change const* SubtreeChange::getChange(OUString const& _rName) const
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
    ::std::map< ::rtl::OUString,Change* >::const_iterator aIter = m_mChanges.begin();
    for(;aIter != m_mChanges.end();)
    {
        ::std::map< ::rtl::OUString,Change* >::const_iterator aNextIter = aIter;
        ++aNextIter;
        aIter->second->dispatch(_anAction);
        aIter = aNextIter;
    }
}

//--------------------------------------------------------------------------
void SubtreeChange::forEachChange(ChangeTreeModification& _anAction)
{
    ::std::map< ::rtl::OUString,Change* >::const_iterator aIter = m_mChanges.begin();
    for(;aIter != m_mChanges.end();++aIter)
        aIter->second->dispatch(_anAction);
}

//--------------------------------------------------------------------------
Change* SubtreeChange::doGetChild(OUString const& _rName) const
{
    Children::const_iterator aIter = m_mChanges.find(_rName);
    return (aIter != m_mChanges.end()) ? aIter->second : NULL;
}

//--------------------------------------------------------------------------
uno::Sequence< OUString > SubtreeChange::elementNames() const
{
    uno::Sequence< OUString > aReturn(size());
    OUString* pReturn = aReturn.getArray();

    for (   Children::const_iterator aCollector = m_mChanges.begin();
            aCollector != m_mChanges.end();
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
    return MutatingChildIterator(m_mChanges.begin());
}

//--------------------------------------------------------------------------
SubtreeChange::MutatingChildIterator SubtreeChange::end_changes() throw()
{
    return MutatingChildIterator(m_mChanges.end());
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
    :m_pTree(_pTree)
    ,m_aNames(_pTree->elementNames())
    ,m_nPos(0)
{
}

//--------------------------------------------------------------------------
SubtreeChange::ChildIterator::ChildIterator(const SubtreeChange* _pTree, struct EndPos)
    :m_pTree(_pTree)
    ,m_aNames(_pTree->elementNames())
    ,m_nPos(_pTree->size())
{
}

//--------------------------------------------------------------------------
Change const & SubtreeChange::ChildIterator::operator*() const
{
    OSL_ENSHURE(isValid(), "SubtreeChange::ChildIterator::operator* : invalid iterator !");

    if (!isValid())
        throw configuration::Exception("INTERNAL ERROR: Invalid SubtreeChange::ChildIterator dereferenced");

    return *m_pTree->getChange(m_aNames[m_nPos]);
}

//--------------------------------------------------------------------------
Change const * SubtreeChange::ChildIterator::operator->() const
{
    if (isValid())
        return m_pTree->getChange(m_aNames[m_nPos]);
    OSL_ENSHURE(sal_False, "SubtreeChange::ChildIterator::operator-> : invalid iterator !");
    return NULL;
}

//--------------------------------------------------------------------------
SubtreeChange::ChildIterator& SubtreeChange::ChildIterator::operator++()
{
    OSL_ENSHURE(m_nPos < m_aNames.getLength(), "SubtreeChange::ChildIterator : can't increment the end iterator !");
    if (m_nPos < m_aNames.getLength())
        ++m_nPos;
    return *this;
}

//--------------------------------------------------------------------------
SubtreeChange::ChildIterator& SubtreeChange::ChildIterator::operator--()
{
    OSL_ENSHURE(m_nPos > 0, "SubtreeChange::ChildIterator : can't decrement the begin iterator !");
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
    :SubtreeChange(_rSource.getNodeName())
{
    ChildIterator aSourceChildren = _rSource.begin();
    while (aSourceChildren != _rSource.end())
    {
        const Change* pChange = &*aSourceChildren;
        OSL_ENSHURE(pChange, "SubtreeChangeReferrer::SubtreeChangeReferrer : invalid change !");
        if  (   pChange->isA(ValueChange::getStaticType())
            ||  pChange->isA(RemoveNode::getStaticType())
            ||  pChange->isA(AddNode::getStaticType())
            )
            SubtreeChange::addChange(::std::auto_ptr<Change>(const_cast<Change*>(pChange)));
        else if (   pChange->isA(SubtreeChange::getStaticType())
                ||  pChange->isA(SubtreeChangeReferrer::getStaticType())
                )
        {
            SubtreeChange::addChange(::std::auto_ptr<Change>(new SubtreeChangeReferrer(*static_cast<const SubtreeChange*>(pChange))));
        }
        else
            OSL_ENSHURE(sal_False, "SubtreeChangeReferrer::SubtreeChangeReferrer : unknown changes type !");

        ++aSourceChildren;
    }
}

//--------------------------------------------------------------------------
SubtreeChangeReferrer::~SubtreeChangeReferrer()
{
    for (   Children::iterator aChildren = m_mChanges.begin();
            aChildren != m_mChanges.end();
            ++aChildren
        )
    {
        const Change* pChange = aChildren->second;
        if  (   pChange->isA(ValueChange::getStaticType())
            ||  pChange->isA(RemoveNode::getStaticType())
            ||  pChange->isA(AddNode::getStaticType())
            )
        {
            // we just hold references to the non-SubtreeChange-objects, so don't delete them
            m_mChanges.erase(aChildren);
        }
        else if (   pChange->isA(SubtreeChange::getStaticType())
                ||  pChange->isA(SubtreeChangeReferrer::getStaticType())
                )
        {
            // nothing to do
        }
        else
            OSL_ENSHURE(sal_False, "SubtreeChangeReferrer::~SubtreeChangeReferrer : unknown changes type !");
    }

    // the base class will remove the remaining SubtreeChanges, which are SubtreeChangeReferrer's in real
}

//..........................................................................
}   // namespace configmgr
//..........................................................................


