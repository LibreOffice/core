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
#ifndef _UNQIDX_HXX
#define _UNQIDX_HXX

#include "tools/toolsdllapi.h"
#include <tools/solar.h>
#include <tools/contnr.hxx>
#include <map>

#define UNIQUEINDEX_ENTRY_NOTFOUND   CONTAINER_ENTRY_NOTFOUND

class TOOLS_DLLPUBLIC SAL_WARN_UNUSED UniqueIndexImpl : public std::map<sal_uInt32, void*>
{
private:
    sal_uIntPtr   nStartIndex;
    sal_uIntPtr   nUniqIndex;
    sal_uIntPtr   nCount;

public:
    UniqueIndexImpl( sal_uIntPtr _nStartIndex = 0 )
        : std::map<sal_uInt32, void*>(),
          nStartIndex(_nStartIndex), nUniqIndex(_nStartIndex), nCount(0) {}

    sal_uIntPtr   Insert( void* p );
    // insert value with key, replacing existing entry if necessary
    void          Insert( sal_uIntPtr aIndex, void* p );
    void*         Remove( sal_uIntPtr aIndex );
    void*         Get( sal_uIntPtr aIndex ) const;

    sal_uIntPtr   GetIndexOf( void* p ) const;
    sal_uIntPtr   FirstIndex() const;
    sal_uIntPtr   LastIndex() const;
    sal_uIntPtr   NextIndex( sal_uIntPtr aCurrIndex ) const;
};

template<typename T>
class UniqueIndex : private UniqueIndexImpl
{
public:
    UniqueIndex<T>( sal_uIntPtr _nStartIndex = 0 ) : UniqueIndexImpl(_nStartIndex) {}

    sal_uIntPtr Insert(T* p) { return UniqueIndexImpl::Insert(p); }
    void        Insert(sal_uIntPtr aIdx, T* p) { return UniqueIndexImpl::Insert(aIdx, p); }
    T*          Get(sal_uIntPtr idx) const { return static_cast<T*>( UniqueIndexImpl::Get(idx) ); }
    T*          Remove(sal_uIntPtr idx) { return static_cast<T*>( UniqueIndexImpl::Remove(idx) ); }
    sal_uIntPtr Count() const { return UniqueIndexImpl::size(); }
    sal_uIntPtr GetIndexOf(T* p) const { return UniqueIndexImpl::GetIndexOf(p); }

    using UniqueIndexImpl::FirstIndex;
    using UniqueIndexImpl::LastIndex;
    using UniqueIndexImpl::NextIndex;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
