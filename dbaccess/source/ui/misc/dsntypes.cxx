/*************************************************************************
 *
 *  $RCSfile: dsntypes.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: fs $ $Date: 2000-10-30 07:59:18 $
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

#ifndef _DBAUI_DSNTYPES_HXX_
#include "dsntypes.hxx"
#endif
#ifndef _DBU_RESOURCE_HRC_
#include "dbu_resource.hrc"
#endif
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif
#ifndef _DBU_MISCRES_HRC_
#include "dbumiscres.hrc"
#endif

//.........................................................................
namespace dbaui
{
//.........................................................................

//=========================================================================
//= ODsnTypeCollection
//=========================================================================
//-------------------------------------------------------------------------
ODsnTypeCollection::ODsnTypeCollection()
    :Resource(ModuleRes(RSC_DATASOURCE_TYPES))
#ifdef DBG_UTIL
    ,m_nLivingIterators(0)
#endif
{
    String sConnectionTypes = String(ResId(STR_CONNTYPES));
    String sConnectionTypeNames = String(ResId(STR_CONNUINAMES));
    DBG_ASSERT(sConnectionTypes.GetTokenCount(';') == sConnectionTypeNames.GetTokenCount(';'),
        "ODsnTypeCollection::ODsnTypeCollection : invalid resources !");
    String sCurrentType;
    for (sal_Int32 i=0; i<sConnectionTypeNames.GetTokenCount(); ++i)
    {
        m_aDsnTypesDisplayNames.push_back(sConnectionTypeNames.GetToken(i));

        sCurrentType = sConnectionTypes.GetToken(i);
        m_aDsnPrefixes.push_back(sCurrentType);
        m_aDsnTypes.push_back(implDetermineType(sCurrentType));
    }
    FreeResource();
}

//-------------------------------------------------------------------------
ODsnTypeCollection::~ODsnTypeCollection()
{
    DBG_ASSERT(0 == m_nLivingIterators, "ODsnTypeCollection::~ODsnTypeCollection : there are still living iterator objects!");
}

//-------------------------------------------------------------------------
DATASOURCE_TYPE ODsnTypeCollection::getType(const String& _rDsn)
{
    return implDetermineType(_rDsn);
}

//-------------------------------------------------------------------------
String ODsnTypeCollection::getTypeDisplayName(DATASOURCE_TYPE _eType)
{
    String sDisplayName;

    sal_Int32 nIndex = implDetermineTypeIndex(_eType);
    if ((nIndex >= 0) && (nIndex < m_aDsnTypesDisplayNames.size()))
        sDisplayName = m_aDsnTypesDisplayNames[nIndex];

    return sDisplayName;
}

//-------------------------------------------------------------------------
String ODsnTypeCollection::getDatasourcePrefix(DATASOURCE_TYPE _eType)
{
    String sPrefix;
    sal_Int32 nIndex = implDetermineTypeIndex(_eType);
    if ((nIndex >= 0) && (nIndex < m_aDsnPrefixes.size()))
        sPrefix = m_aDsnPrefixes[nIndex];

    return sPrefix;
}

//-------------------------------------------------------------------------
String ODsnTypeCollection::cutPrefix(const String& _rDsn)
{
    DATASOURCE_TYPE eType = getType(_rDsn);
    String sPrefix = getDatasourcePrefix(eType);
    return _rDsn.Copy(sPrefix.Len());
}

//-------------------------------------------------------------------------
String ODsnTypeCollection::getTypeDisplayName(const String& _rDsn)
{
    return getTypeDisplayName(implDetermineType(_rDsn));
}

//-------------------------------------------------------------------------
sal_Bool ODsnTypeCollection::hasAuthentication(DATASOURCE_TYPE _eType)
{
    switch (_eType)
    {
        case DST_ADABAS:
        case DST_JDBC:
        case DST_ODBC:
            return sal_True;
            break;
        case DST_DBASE:
        case DST_TEXT:
        default:
            return sal_False;
    }
}

//-------------------------------------------------------------------------
DATASOURCE_TYPE ODsnTypeCollection::implDetermineType(const String& _rDsn)
{
    sal_Int32 nSeparator = _rDsn.Search((sal_Unicode)':');
    if (STRING_NOTFOUND == nSeparator)
    {
        // there should be at least one such separator
        DBG_ERROR("ODsnTypeCollection::implDetermineType : missing the colon !");
        return DST_UNKNOWN;
    }

    if (_rDsn.EqualsIgnoreCaseAscii("jdbc", 0, nSeparator))
        return DST_JDBC;

    nSeparator = _rDsn.Search((sal_Unicode)':', nSeparator + 1);
    if (STRING_NOTFOUND == nSeparator)
    {
        // at the moment only jdbc is allowed to have just one separator
        DBG_ERROR("ODsnTypeCollection::implDetermineType : missing the second colon !");
        return DST_UNKNOWN;
    }

    if (_rDsn.EqualsIgnoreCaseAscii("sdbc:adabas", 0, nSeparator))
        return DST_ADABAS;
    if (_rDsn.EqualsIgnoreCaseAscii("sdbc:odbc", 0, nSeparator))
        return DST_ODBC;
    if (_rDsn.EqualsIgnoreCaseAscii("sdbc:dbase", 0, nSeparator))
        return DST_DBASE;

    nSeparator = _rDsn.Search((sal_Unicode)':', nSeparator + 1);
    if (STRING_NOTFOUND == nSeparator)
    {
        DBG_ERROR("ODsnTypeCollection::implDetermineType : missing the third colon !");
        return DST_UNKNOWN;
    }

    if (_rDsn.EqualsIgnoreCaseAscii("sdbc:flat:file", 0, nSeparator))
        return DST_TEXT;

    DBG_ERROR("ODsnTypeCollection::implDetermineType : unrecognized data source type !");
    return DST_UNKNOWN;
}

//-------------------------------------------------------------------------
sal_Int32 ODsnTypeCollection::implDetermineTypeIndex(DATASOURCE_TYPE _eType)
{
    DBG_ASSERT(
            (m_aDsnTypesDisplayNames.size() == m_aDsnPrefixes.size())
        &&  (m_aDsnTypesDisplayNames.size() == m_aDsnTypes.size()),
        "ODsnTypeCollection::implDetermineTypeIndex : inconsistent structures !");

    // the type of the datasource described by the DSN string
    if (DST_UNKNOWN == _eType)
    {
        DBG_ERROR("ODsnTypeCollection::implDetermineTypeIndex : invalid argument !");
        return -1;
    }

    // search this type in our arrays
    sal_Int32 nIndex = 0;
    ConstTypeVectorIterator aSearch = m_aDsnTypes.begin();

    for (; aSearch != m_aDsnTypes.end(); ++nIndex, ++aSearch)
        if (*aSearch == _eType)
            return nIndex;

    DBG_ERROR("ODsnTypeCollection::implDetermineTypeIndex : recognized the DSN schema, but did not find the type!");
    return -1;
}

//-------------------------------------------------------------------------
sal_Int32 ODsnTypeCollection::implDetermineTypeIndex(const String& _rDsn)
{
    return implDetermineTypeIndex(implDetermineType(_rDsn));
}

//-------------------------------------------------------------------------
ODsnTypeCollection::TypeIterator ODsnTypeCollection::begin() const
{
    return TypeIterator(this, 0);
}

//-------------------------------------------------------------------------
ODsnTypeCollection::TypeIterator ODsnTypeCollection::end() const
{
    return TypeIterator(this, m_aDsnTypes.size());
}

//=========================================================================
//= ODsnTypeCollection::TypeIterator
//=========================================================================
//-------------------------------------------------------------------------
ODsnTypeCollection::TypeIterator::TypeIterator(const ODsnTypeCollection* _pContainer, sal_Int32 _nInitialPos)
    :m_pContainer(_pContainer)
    ,m_nPosition(_nInitialPos)
{
    DBG_ASSERT(m_pContainer, "ODsnTypeCollection::TypeIterator::TypeIterator : invalid container!");
#ifdef DBG_UTIL
    ++const_cast<ODsnTypeCollection*>(m_pContainer)->m_nLivingIterators;
#endif
}

//-------------------------------------------------------------------------
ODsnTypeCollection::TypeIterator::TypeIterator(const TypeIterator& _rSource)
    :m_pContainer(_rSource.m_pContainer)
    ,m_nPosition(_rSource.m_nPosition)
{
#ifdef DBG_UTIL
    ++const_cast<ODsnTypeCollection*>(m_pContainer)->m_nLivingIterators;
#endif
}

//-------------------------------------------------------------------------
ODsnTypeCollection::TypeIterator::~TypeIterator()
{
#ifdef DBG_UTIL
    --const_cast<ODsnTypeCollection*>(m_pContainer)->m_nLivingIterators;
#endif
}

//-------------------------------------------------------------------------
DATASOURCE_TYPE ODsnTypeCollection::TypeIterator::getType() const
{
    DBG_ASSERT(m_nPosition < m_pContainer->m_aDsnTypes.size(), "ODsnTypeCollection::TypeIterator::getType : invalid position!");
    return m_pContainer->m_aDsnTypes[m_nPosition];
}

//-------------------------------------------------------------------------
String ODsnTypeCollection::TypeIterator::getPrefix() const
{
    DBG_ASSERT(m_nPosition < m_pContainer->m_aDsnPrefixes.size(), "ODsnTypeCollection::TypeIterator::getPrefix : invalid position!");
    return m_pContainer->m_aDsnPrefixes[m_nPosition];
}

//-------------------------------------------------------------------------
String ODsnTypeCollection::TypeIterator::getDisplayName() const
{
    DBG_ASSERT(m_nPosition < m_pContainer->m_aDsnTypesDisplayNames.size(), "ODsnTypeCollection::TypeIterator::getDisplayName : invalid position!");
    return m_pContainer->m_aDsnTypesDisplayNames[m_nPosition];
}

//-------------------------------------------------------------------------
const ODsnTypeCollection::TypeIterator& ODsnTypeCollection::TypeIterator::operator++()
{
    DBG_ASSERT(m_nPosition < m_pContainer->m_aDsnTypes.size(), "ODsnTypeCollection::TypeIterator::operator++ : invalid position!");
    if (m_nPosition < m_pContainer->m_aDsnTypes.size())
        ++m_nPosition;
    return *this;
}

//-------------------------------------------------------------------------
const ODsnTypeCollection::TypeIterator& ODsnTypeCollection::TypeIterator::operator--()
{
    DBG_ASSERT(m_nPosition >= 0, "ODsnTypeCollection::TypeIterator::operator-- : invalid position!");
    if (m_nPosition >= 0)
        --m_nPosition;
    return *this;
}

//-------------------------------------------------------------------------
bool operator==(const ODsnTypeCollection::TypeIterator& lhs, const ODsnTypeCollection::TypeIterator& rhs)
{
    return (lhs.m_pContainer == rhs.m_pContainer) && (lhs.m_nPosition == rhs.m_nPosition);
}

//=========================================================================
//= DbuTypeCollectionItem
//=========================================================================
TYPEINIT1(DbuTypeCollectionItem, SfxPoolItem);
//-------------------------------------------------------------------------
DbuTypeCollectionItem::DbuTypeCollectionItem(sal_Int16 _nWhich, ODsnTypeCollection* _pCollection)
    :SfxPoolItem(_nWhich)
    ,m_pCollection(_pCollection)
{
}

//-------------------------------------------------------------------------
DbuTypeCollectionItem::DbuTypeCollectionItem(const DbuTypeCollectionItem& _rSource)
    :SfxPoolItem(_rSource)
    ,m_pCollection(_rSource.getCollection())
{
}

//-------------------------------------------------------------------------
int DbuTypeCollectionItem::operator==(const SfxPoolItem& _rItem) const
{
    DbuTypeCollectionItem* pCompare = PTR_CAST(DbuTypeCollectionItem, &_rItem);
    return pCompare && (pCompare->getCollection() == getCollection());
}

//-------------------------------------------------------------------------
SfxPoolItem* DbuTypeCollectionItem::Clone(SfxItemPool* _pPool) const
{
    return new DbuTypeCollectionItem(*this);
}

//.........................................................................
}   // namespace dbaui
//.........................................................................

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.2  2000/10/20 07:01:39  fs
 *  sdbc:text -> sdbc:flat:file
 *
 *  Revision 1.1  2000/10/05 10:09:11  fs
 *  initial checkin
 *
 *
 *  Revision 1.0 26.09.00 08:05:35  fs
 ************************************************************************/

