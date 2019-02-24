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

#include "RowSetCacheIterator.hxx"
#include "RowSetCache.hxx"
#include "RowSetBase.hxx"

using namespace dbaccess;

ORowSetCacheIterator::ORowSetCacheIterator(const ORowSetCacheIterator& _rRH)
: m_aIter(_rRH.m_aIter)
, m_pCache(_rRH.m_pCache)
,m_pRowSet(_rRH.m_pRowSet)
{
}

ORowSetCacheIterator::operator ORowSetMatrix::iterator const &()
{
    return m_aIter->second.aIterator;
}

ORowSetCacheIterator& ORowSetCacheIterator::operator =(const ORowSetCacheIterator& _rRH)
{
    if(this == &_rRH)
        return *this;

    m_pCache = _rRH.m_pCache;
    m_aIter  = _rRH.m_aIter;
    m_pRowSet = _rRH.m_pRowSet;

    return *this;
}

ORowSetCacheIterator& ORowSetCacheIterator::operator =(const ORowSetMatrix::iterator& _rIter)
{
    m_aIter->second.aIterator = _rIter;
    return *this;
}

ORowSetRow& ORowSetCacheIterator::operator *()
{
    return *m_aIter->second.aIterator;
}

ORowSetMatrix::iterator& ORowSetCacheIterator::operator ->()
{
    return m_aIter->second.aIterator;
}

bool ORowSetCacheIterator::operator <(const ORowSetMatrix::iterator& _rRH) const
{
    return m_aIter->second.aIterator < _rRH;
}

bool ORowSetCacheIterator::operator !=(const ORowSetMatrix::iterator& _rRH) const
{
    return m_aIter->second.aIterator != _rRH;
}

bool ORowSetCacheIterator::isNull() const
{
    bool bRet = !m_pCache || !m_pRowSet || m_aIter == m_pCache->m_aCacheIterators.end();
    if ( !bRet )
    {
        bRet = ( m_pRowSet->isInsertRow()
            ?
            m_aIter->second.aIterator == m_pCache->m_pInsertMatrix->end()
            :
            m_aIter->second.aIterator == m_pCache->m_pMatrix->end()
        );
    }
    return  bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
