/*************************************************************************
 *
 *  $RCSfile: charsets.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: fs $ $Date: 2000-10-05 10:08:39 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _DBAUI_CHARSETS_HXX_
#include "charsets.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _DBU_MISCRES_HRC_
#include "dbumiscres.hrc"
#endif
#ifndef _DBU_RESOURCE_HRC_
#include "dbu_resource.hrc"
#endif

//.........................................................................
namespace dbaui
{
//.........................................................................

//=========================================================================
//= OCharsetCollection
//=========================================================================
//-------------------------------------------------------------------------
OCharsetCollection::OCharsetCollection()
    :Resource(RSC_CHARSETS)
#ifdef DBG_UTIL
    ,m_nLivingIterators(0)
#endif
{
    String aNameList = String(ResId(STR_CHARSETKEYS));
    String aKeyList = String(ResId(STR_CHARSETNAMES));

    DBG_ASSERT(aNameList.GetTokenCount(';') == aKeyList.GetTokenCount(';'),
        "OCharsetCollection::OCharsetCollection : invalid resources !");
    String sCurrentType;
    for (sal_Int32 i=0; i<aKeyList.GetTokenCount(); ++i)
    {
        m_aNameList.push_back(aNameList.GetToken(i));
        m_aKeyList.push_back(aKeyList.GetToken(i));
    }

    FreeResource();
}

//-------------------------------------------------------------------------
OCharsetCollection::~OCharsetCollection()
{
    DBG_ASSERT(0 == m_nLivingIterators, "OCharsetCollection::~OCharsetCollection : there are still living iterator objects!");
}

//-------------------------------------------------------------------------
OCharsetCollection::CharsetIterator OCharsetCollection::begin() const
{
    return CharsetIterator(this, 0);
}

//-------------------------------------------------------------------------
OCharsetCollection::CharsetIterator OCharsetCollection::end() const
{
    return CharsetIterator(this, m_aNameList.size());
}

// -----------------------------------------------------------------------
String OCharsetCollection::implLookUp(const String& _rKey, const StringVector& _rKeys, const StringVector& _rValues) const
{
    DBG_ASSERT(_rKeys.size() == _rValues.size(), "OCharsetCollection::implLookUp : invalid arrays!");
    ConstStringVectorIterator aKeySearch = _rKeys.begin();
    ConstStringVectorIterator aValue = _rValues.begin();
    for (;aKeySearch != _rKeys.end(); ++aKeySearch, ++aValue)
        if (aKeySearch->EqualsIgnoreCaseAscii(*aValue))
            return *aValue;

    return String();
}

//=========================================================================
//= OCharsetCollection::CharsetIterator
//=========================================================================
//-------------------------------------------------------------------------
OCharsetCollection::CharsetIterator::CharsetIterator(const OCharsetCollection* _pContainer, sal_Int32 _nInitialPos)
    :m_pContainer(_pContainer)
    ,m_nPosition(_nInitialPos)
{
    DBG_ASSERT(m_pContainer, "OCharsetCollection::CharsetIterator::CharsetIterator : invalid container!");
#ifdef DBG_UTIL
    ++const_cast<OCharsetCollection*>(m_pContainer)->m_nLivingIterators;
#endif
}

//-------------------------------------------------------------------------
OCharsetCollection::CharsetIterator::CharsetIterator(const CharsetIterator& _rSource)
    :m_pContainer(_rSource.m_pContainer)
    ,m_nPosition(_rSource.m_nPosition)
{
#ifdef DBG_UTIL
    ++const_cast<OCharsetCollection*>(m_pContainer)->m_nLivingIterators;
#endif
}

//-------------------------------------------------------------------------
OCharsetCollection::CharsetIterator::~CharsetIterator()
{
#ifdef DBG_UTIL
    --const_cast<OCharsetCollection*>(m_pContainer)->m_nLivingIterators;
#endif
}

//-------------------------------------------------------------------------
String OCharsetCollection::CharsetIterator::getKey() const
{
    DBG_ASSERT(m_nPosition < m_pContainer->m_aKeyList.size(), "OCharsetCollection::CharsetIterator::getKey : invalid position!");
    return m_pContainer->m_aKeyList[m_nPosition];
}

//-------------------------------------------------------------------------
String OCharsetCollection::CharsetIterator::getName() const
{
    DBG_ASSERT(m_nPosition < m_pContainer->m_aNameList.size(), "OCharsetCollection::CharsetIterator::getName : invalid position!");
    return m_pContainer->m_aNameList[m_nPosition];
}

//-------------------------------------------------------------------------
const OCharsetCollection::CharsetIterator&  OCharsetCollection::CharsetIterator::operator++()
{
    DBG_ASSERT(m_nPosition < m_pContainer->m_aKeyList.size(), "OCharsetCollection::CharsetIterator::operator++ : invalid position!");
    if (m_nPosition < m_pContainer->m_aKeyList.size())
        ++m_nPosition;
    return *this;
}

//-------------------------------------------------------------------------
const OCharsetCollection::CharsetIterator&  OCharsetCollection::CharsetIterator::operator--()
{
    DBG_ASSERT(m_nPosition >= 0, "OCharsetCollection::CharsetIterator::operator-- : invalid position!");
    if (m_nPosition >= 0)
        --m_nPosition;
    return *this;
}

//-------------------------------------------------------------------------
bool operator==(const OCharsetCollection::CharsetIterator& lhs, const OCharsetCollection::CharsetIterator& rhs)
{
    return (lhs.m_pContainer == rhs.m_pContainer) && (lhs.m_nPosition == rhs.m_nPosition);
}

//.........................................................................
}   // namespace dbaui
//.........................................................................

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *
 *  Revision 1.0 26.09.00 12:18:36  fs
 ************************************************************************/

