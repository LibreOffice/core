/*************************************************************************
 *
 *  $RCSfile: cmtreemodel.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: jb $ $Date: 2001-03-12 15:04:10 $
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

#include <algorithm>

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
void SubtreeChange::swap(SubtreeChange& aOther)
{
    Change::swap(aOther);
    m_aChanges.swap(aOther.m_aChanges);
    std::swap(m_sTemplateName, aOther.m_sTemplateName);
    std::swap(m_sTemplateModule, aOther.m_sTemplateModule);
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
SubtreeChange::SubtreeChange(const SubtreeChange& _aObj)
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
        Children::value_type aCopy(aIter->first, aIter->second->clone());
        m_aChanges.insert(m_aChanges.end(), aCopy);
    }
}

// -----------------------------------------------------------------------------
Change* SubtreeChange::clone() const
{
    return new SubtreeChange(*this);
}
//--------------------------------------------------------------------------
void SubtreeChange::addChange(std::auto_ptr<Change> aChange)
{
    OUString aNodeName(aChange->getNodeName());
    m_aChanges.find(aNodeName);
    OSL_ENSHURE(m_aChanges.end() == m_aChanges.find(aNodeName),
        "SubtreeChange::addChange : overwriting an existent change !");
    delete m_aChanges[aNodeName];
    m_aChanges[aNodeName] = aChange.release();
}

//--------------------------------------------------------------------------
::std::auto_ptr<Change> SubtreeChange::removeChange(OUString const& _rName)
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
    ::std::map< ::rtl::OUString,Change* >::const_iterator aIter = m_aChanges.begin();
    for(;aIter != m_aChanges.end();)
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
    ::std::map< ::rtl::OUString,Change* >::const_iterator aIter = m_aChanges.begin();
    for(;aIter != m_aChanges.end();++aIter)
        aIter->second->dispatch(_anAction);
}

//--------------------------------------------------------------------------
Change* SubtreeChange::doGetChild(OUString const& _rName) const
{
    Children::const_iterator aIter = m_aChanges.find(_rName);
    return (aIter != m_aChanges.end()) ? aIter->second : NULL;
}

//--------------------------------------------------------------------------
uno::Sequence< OUString > SubtreeChange::elementNames() const
{
    uno::Sequence< OUString > aReturn(size());
    OUString* pReturn = aReturn.getArray();

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
    :SubtreeChange(_rSource, SubtreeChange::NoChildCopy())
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
    for (   Children::iterator aChildren = m_aChanges.begin();
            aChildren != m_aChanges.end();

        )
    {
        const Change* pChange = aChildren->second;
        Children::iterator aCurrent = aChildren++;

        if  (   pChange->isA(ValueChange::getStaticType())
            ||  pChange->isA(RemoveNode::getStaticType())
            ||  pChange->isA(AddNode::getStaticType())
            )
        {
            // we just hold references to the non-SubtreeChange-objects, so don't delete them
            m_aChanges.erase(aCurrent);
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


