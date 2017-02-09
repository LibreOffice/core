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
#ifndef INCLUDED_TOOLS_UNQIDX_HXX
#define INCLUDED_TOOLS_UNQIDX_HXX

#include <sal/types.h>
#include <tools/toolsdllapi.h>
#include <o3tl/strong_int_typedef.hxx>
#include <map>

class SAL_WARN_UNUSED TOOLS_DLLPUBLIC UniqueIndexImpl
{
public:
    struct IndexTagType {};
    typedef o3tl::strong_int_typedef<sal_uInt32, IndexTagType> Index;
    static Index const IndexNotFound;// = Index(SAL_MAX_UINT32);

private:
    std::map<Index, void*> maMap;
    Index nUniqIndex;

public:
    UniqueIndexImpl( Index nStartIndex = 0 )
        : maMap(), nUniqIndex(nStartIndex) {}

    Index Insert( void* p );
    // insert value with key, replacing existing entry if necessary
    void* Remove( Index aIndex );
    void* Get( Index aIndex ) const;

    Index GetIndexOf( void const* p ) const;
    Index FirstIndex() const;
    Index LastIndex() const;
    Index NextIndex( Index aCurrIndex ) const;
};

template<typename T>
class SAL_WARN_UNUSED UniqueIndex : private UniqueIndexImpl
{
public:
    using UniqueIndexImpl::Index;
    using UniqueIndexImpl::IndexNotFound;

    UniqueIndex<T>( Index _nStartIndex = 0 ) : UniqueIndexImpl(_nStartIndex) {}

    Index Insert(T* p) { return UniqueIndexImpl::Insert(p); }
    T*    Get(Index idx) const { return static_cast<T*>( UniqueIndexImpl::Get(idx) ); }
    T*    Remove(Index idx) { return static_cast<T*>( UniqueIndexImpl::Remove(idx) ); }
    Index GetIndexOf(T* p) const { return UniqueIndexImpl::GetIndexOf(p); }

    using UniqueIndexImpl::FirstIndex;
    using UniqueIndexImpl::LastIndex;
    using UniqueIndexImpl::NextIndex;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
