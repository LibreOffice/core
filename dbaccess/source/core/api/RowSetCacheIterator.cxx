/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: RowSetCacheIterator.cxx,v $
 * $Revision: 1.17 $
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
#include "precompiled_dbaccess.hxx"
#ifndef DBACCESS_ROWSETCACHEITERATOR_HXX
#include "RowSetCacheIterator.hxx"
#endif
#ifndef DBACCESS_CORE_API_ROWSETCACHE_HXX
#include "RowSetCache.hxx"
#endif
#ifndef DBACCESS_CORE_API_ROWSETBASE_HXX
#include "RowSetBase.hxx"
#endif
#include <rtl/logfile.hxx>

using namespace dbaccess;
ORowSetCacheIterator::ORowSetCacheIterator(const ORowSetCacheIterator& _rRH)
: m_aIter(_rRH.m_aIter)
, m_pCache(_rRH.m_pCache)
,m_pRowSet(_rRH.m_pRowSet)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "ORowSetCacheIterator::ORowSetCacheIterator" );
}
// -----------------------------------------------------------------------------
ORowSetCacheIterator::operator ORowSetMatrix::iterator()
{
    return m_aIter->second.aIterator;
}
// -----------------------------------------------------------------------------
ORowSetCacheIterator& ORowSetCacheIterator::operator =(const ORowSetCacheIterator& _rRH)
{
    if(this == &_rRH)
        return *this;

    m_pCache = _rRH.m_pCache;
    m_aIter  = _rRH.m_aIter;
    m_pRowSet = _rRH.m_pRowSet;

    return *this;
}
// -----------------------------------------------------------------------------
ORowSetCacheIterator& ORowSetCacheIterator::operator =(const ORowSetMatrix::iterator& _rIter)
{
    m_aIter->second.aIterator = _rIter;
    return *this;
}
// -----------------------------------------------------------------------------
ORowSetRow& ORowSetCacheIterator::operator *()
{
    return *m_aIter->second.aIterator;
}
// -----------------------------------------------------------------------------
const ORowSetRow& ORowSetCacheIterator::operator *() const
{
    if ( !m_pRowSet->isInsertRow() && m_aIter->second.aIterator == m_pCache->m_pMatrix->end() )
    {
        OSL_ENSURE(m_aIter->second.aBookmark.hasValue(),"bookmark has no value!");
        OSL_VERIFY(m_pCache->moveToBookmark(m_aIter->second.aBookmark));
        m_aIter->second.aIterator = m_pCache->m_aMatrixIter;
    }
    return *m_aIter->second.aIterator;
}
// -----------------------------------------------------------------------------
ORowSetMatrix::iterator& ORowSetCacheIterator::operator ->()
{
    return m_aIter->second.aIterator;
}
// -----------------------------------------------------------------------------
const ORowSetMatrix::iterator& ORowSetCacheIterator::operator ->() const
{
    if ( !m_pRowSet->isInsertRow() && m_aIter->second.aIterator == m_pCache->m_pMatrix->end() )
    {
        OSL_ENSURE(m_aIter->second.aBookmark.hasValue(),"bookmark has no value!");
        OSL_VERIFY(m_pCache->moveToBookmark(m_aIter->second.aBookmark));
        m_aIter->second.aIterator = m_pCache->m_aMatrixIter;
    }
    return m_aIter->second.aIterator;
}
// -----------------------------------------------------------------------------
bool ORowSetCacheIterator::operator <=(const ORowSetMatrix::iterator& _rRH) const
{
    return m_aIter->second.aIterator <= _rRH;
}
// -----------------------------------------------------------------------------
bool ORowSetCacheIterator::operator <(const ORowSetMatrix::iterator& _rRH) const
{
    return m_aIter->second.aIterator < _rRH;
}
// -----------------------------------------------------------------------------
bool ORowSetCacheIterator::operator !=(const ORowSetMatrix::iterator& _rRH) const
{
    return m_aIter->second.aIterator != _rRH;
}
// -----------------------------------------------------------------------------
bool ORowSetCacheIterator::operator ==(const ORowSetMatrix::iterator& _rRH) const
{
    return m_aIter->second.aIterator == _rRH;
}
// -----------------------------------------------------------------------------
void ORowSetCacheIterator::setBookmark(const ::com::sun::star::uno::Any&    _rBookmark)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "ORowSetCacheIterator::setBookmark" );
    m_aIter->second.aBookmark = _rBookmark;
}
// -----------------------------------------------------------------------------
sal_Bool ORowSetCacheIterator::isNull() const
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "ORowSetCacheIterator::isNull" );
    sal_Bool bRet = !m_pCache || !m_pRowSet || m_aIter == m_pCache->m_aCacheIterators.end();
    if ( !bRet )
    {
        ORowSetCacheIterator_Helper aHelper = m_aIter->second;
        bRet = ( m_pRowSet->isInsertRow()
            ?
            m_aIter->second.aIterator == m_pCache->m_pInsertMatrix->end()
            :
            m_aIter->second.aIterator == m_pCache->m_pMatrix->end()
        );
    }
    return  bRet;
}
// -----------------------------------------------------------------------------
::osl::Mutex* ORowSetCacheIterator::getMutex() const
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "ORowSetCacheIterator::getMutex" );
    return m_pRowSet ? m_pRowSet->getMutex() : NULL;
}
