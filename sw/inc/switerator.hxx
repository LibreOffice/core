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
#ifndef INCLUDED_SW_INC_SWITERATOR_HXX
#define INCLUDED_SW_INC_SWITERATOR_HXX

#include <calbck.hxx>
#include <sal/log.hxx>

template< class TElementType, class TSource > class SwIterator SAL_FINAL
{
    SwClientIter aClientIter;
public:

    SwIterator( const TSource& rSrc ) : aClientIter(rSrc) { assert(TElementType::IsOf( TYPE(SwClient) )); }
    TElementType* First()
    {
        aClientIter.m_aSearchType = TYPE(TElementType);
        aClientIter.GoStart();
        if(!aClientIter.m_pPosition)
            return nullptr;
        aClientIter.m_pCurrent = nullptr;
        return PTR_CAST(TElementType,aClientIter.Next());
    }
    TElementType* Last()
    {
        aClientIter.m_aSearchType = TYPE(TElementType);
        GoEnd();
        if(!aClientIter.m_pPosition)
            return nullptr;
        if(aClientIter.m_pPosition->IsA(TYPE(TElementType))
            return PTR_CAST(TElementType,aClientIter.m_pPosition);
        return PTR_CAST(TElementType,aClientIter.Previous());
    }
    TElementType* Next()
    {
        if( aClientIter.m_pPosition == aClientIter.m_pCurrent )
            aClientIter.m_pPosition = static_cast<SwClient*>(aClientIter.m_pPosition->m_pRight);
        while(aClientIter.m_pPosition && !aClientIter.m_pPosition->IsA( TYPE(TElementType) ) )
            aClientIter.m_pPosition = static_cast<SwClient*>(aClientIter.m_pPosition->m_pRight);
        return PTR_CAST(TElementType,aClientIter.m_pCurrent = aClientIter.m_pPosition);
    }
    TElementType* Previous()
    {
        aClientIter.m_pPosition = static_cast<SwClient*>(aClientIter.m_pPosition->m_pLeft);
        while(aClientIter.m_pPosition && !aClientIter.m_pPosition->IsA( TYPE(TElementType) ) )
            aClientIter.m_pPosition = static_cast<SwClient*>(aClientIter.m_pPosition->m_pLeft);
        return PTR_CAST(TElementType,aClientIter.m_pCurrent = aClientIter.m_pPosition);
    }
    static TElementType* FirstElement( const TSource& rMod ) { SwClient* p = SwClientIter(rMod).First(TYPE(TElementType)); return PTR_CAST(TElementType,p); }
    bool IsChanged()          { return aClientIter.IsChanged(); }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
