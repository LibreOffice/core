/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#ifndef DBACCESS_ROWSETCACHEITERATOR_HXX
#define DBACCESS_ROWSETCACHEITERATOR_HXX

#include "RowSetRow.hxx"
#include <comphelper/stl_types.hxx>

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
        bool operator <(const ORowSetMatrix::iterator& _rRH) const;
        bool operator !=(const ORowSetMatrix::iterator& _rRH) const;
        bool operator ==(const ORowSetMatrix::iterator& _rRH) const;

        void setBookmark(const ::com::sun::star::uno::Any&  _rBookmark);
        ::com::sun::star::uno::Any getBookmark() const { return m_aIter->second.aBookmark; }
        ::osl::Mutex*   getMutex() const;

        ORowSetCacheMap::iterator getIter() const { return m_aIter; }
    };
}
#endif // DBACCESS_ROWSETCACHEITERATOR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
