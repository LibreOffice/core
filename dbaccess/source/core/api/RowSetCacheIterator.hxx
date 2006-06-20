/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: RowSetCacheIterator.hxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 02:37:06 $
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
#define DBACCESS_ROWSETCACHEITERATOR_HXX

#ifndef DBACCESS_CORE_API_ROWSETROW_HXX
#include "RowSetRow.hxx"
#endif
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif

namespace dbaccess
{
    class ORowSetBase;
    typedef struct
    {
        ORowSetMatrix::iterator     aIterator;
        ::com::sun::star::uno::Any  aBookmark;
        ORowSetBase*                pRowSet;
    } ORowSetCacheIterator_Helper;

    DECLARE_STL_STDKEY_MAP(sal_Int32,ORowSetCacheIterator_Helper,ORowSetCacheMap);

    class ORowSetCache;
    class ORowSetCacheIterator
    {
        friend class ORowSetCache;
        ORowSetCacheMap::iterator   m_aIter;
        ORowSetCache*               m_pCache;
        ORowSetBase*                m_pRowSet;
    protected:
        ORowSetCacheIterator(const ORowSetCacheMap::iterator& _rIter,ORowSetCache* _pCache,ORowSetBase* _pRowSet)
            : m_aIter(_rIter)
            ,m_pCache(_pCache)
            ,m_pRowSet(_pRowSet)
        {
        }
    public:
        ORowSetCacheIterator() :m_aIter(),m_pCache(NULL),m_pRowSet(NULL){}
        ORowSetCacheIterator(const ORowSetCacheIterator& _rRH);
        ORowSetCacheIterator& operator =(const ORowSetCacheIterator&);

        sal_Bool isNull() const;
        ORowSetCacheIterator& operator =(const ORowSetMatrix::iterator&);
        operator ORowSetMatrix::iterator();

        ORowSetRow& operator *();
        const ORowSetRow& operator *() const;

        ORowSetMatrix::iterator& operator ->();
        const ORowSetMatrix::iterator& operator ->() const;

        bool operator <=(const ORowSetMatrix::iterator& _rRH) const;
        bool operator !=(const ORowSetMatrix::iterator& _rRH) const;
        bool operator ==(const ORowSetMatrix::iterator& _rRH) const;

        void setBookmark(const ::com::sun::star::uno::Any&  _rBookmark);
        ::com::sun::star::uno::Any getBookmark() const { return m_aIter->second.aBookmark; }
        ::osl::Mutex*   getMutex() const;
    };
}
#endif // DBACCESS_ROWSETCACHEITERATOR_HXX


