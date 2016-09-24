/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_DBACCESS_SOURCE_CORE_API_ROWSETCACHEITERATOR_HXX
#define INCLUDED_DBACCESS_SOURCE_CORE_API_ROWSETCACHEITERATOR_HXX

#include <sal/config.h>

#include <map>

#include "RowSetRow.hxx"

namespace dbaccess
{
    class ORowSetBase;
    typedef struct
    {
        ORowSetMatrix::iterator     aIterator;
        css::uno::Any               aBookmark;
        ORowSetBase*                pRowSet;
    } ORowSetCacheIterator_Helper;

    typedef std::map<sal_Int32, ORowSetCacheIterator_Helper> ORowSetCacheMap;

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
        ORowSetCacheIterator() :m_aIter(),m_pCache(nullptr),m_pRowSet(nullptr){}
        ORowSetCacheIterator(const ORowSetCacheIterator& _rRH);
        ORowSetCacheIterator& operator =(const ORowSetCacheIterator&);

        bool isNull() const;
        ORowSetCacheIterator& operator =(const ORowSetMatrix::iterator&);
        operator ORowSetMatrix::iterator const &();

        ORowSetRow& operator *();

        ORowSetMatrix::iterator& operator ->();

        bool operator <(const ORowSetMatrix::iterator& _rRH) const;
        bool operator !=(const ORowSetMatrix::iterator& _rRH) const;

        void setBookmark(const css::uno::Any&  _rBookmark);

        const ORowSetCacheMap::iterator& getIter() const { return m_aIter; }
    };
}
#endif // INCLUDED_DBACCESS_SOURCE_CORE_API_ROWSETCACHEITERATOR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
