/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: RowSetCacheIterator.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 02:36:52 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef DBACCESS_ROWSETCACHEITERATOR_HXX
#include "RowSetCacheIterator.hxx"
#endif
#ifndef DBACCESS_CORE_API_ROWSETCACHE_HXX
#include "RowSetCache.hxx"
#endif
#ifndef DBACCESS_CORE_API_ROWSETBASE_HXX
#include "RowSetBase.hxx"
#endif


using namespace dbaccess;
ORowSetCacheIterator::ORowSetCacheIterator(const ORowSetCacheIterator& _rRH)
: m_aIter(_rRH.m_aIter)
, m_pCache(_rRH.m_pCache)
,m_pRowSet(_rRH.m_pRowSet)
{
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
        m_pCache->moveToBookmark(m_aIter->second.aBookmark);
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
        m_pCache->moveToBookmark(m_aIter->second.aBookmark);
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
    m_aIter->second.aBookmark = _rBookmark;
}
// -----------------------------------------------------------------------------
sal_Bool ORowSetCacheIterator::isNull() const
{
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
    return m_pRowSet ? m_pRowSet->getMutex() : NULL;
}
