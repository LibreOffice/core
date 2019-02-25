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

#ifndef INCLUDED_IDL_INC_BASOBJ_HXX
#define INCLUDED_IDL_INC_BASOBJ_HXX

#include <tools/ref.hxx>
#include "bastype.hxx"
#include <functional>
#include <vector>

class SvTokenStream;
class SvIdlDataBase;

template<typename T>
class SvRefMemberList : private std::vector<T>
{
private:
    typedef typename std::vector<T> base_t;

public:
    using base_t::size;
    using base_t::front;
    using base_t::back;
    using base_t::operator[];
    using base_t::begin;
    using base_t::end;
    using typename base_t::iterator;
    using typename base_t::const_iterator;
    using base_t::rbegin;
    using base_t::rend;
    using typename base_t::reverse_iterator;
    using base_t::empty;

    ~SvRefMemberList() { clear(); }
    void clear()
    {
        for( typename base_t::const_iterator it = base_t::begin(); it != base_t::end(); ++it )
        {
              T p = *it;
              if( p )
                  p->ReleaseRef();
        }
        base_t::clear();
    }

    void push_back( T p )
    {
        base_t::push_back( p );
        p->AddFirstRef();
    }

    void insert( typename base_t::iterator it, T p )
    {
        base_t::insert( it, p );
        p->AddFirstRef();
    }

    void pop_back()
    {
        T p = base_t::back();
        base_t::pop_back();
        if( p )
            p->ReleaseRef();
    }
};

class SvMetaObject : public SvRefBase
{
    OString      aName;

protected:
    bool         ReadNameSvIdl( SvTokenStream & rInStm );
            void DoReadContextSvIdl( SvIdlDataBase &, SvTokenStream & rInStm );
    virtual void ReadContextSvIdl( SvIdlDataBase &, SvTokenStream & rInStm );
    virtual void ReadAttributesSvIdl( SvIdlDataBase & rBase,
                                      SvTokenStream & rInStm );
public:
            SvMetaObject();

    static void         WriteTab( SvStream & rOutStm, sal_uInt16 nTab );
    static void         Back2Delimiter( SvStream & );
    static void         WriteStars( SvStream & );

    void                      SetName( const OString& rName );
    virtual const OString &  GetName() const { return aName; }

    virtual bool        ReadSvIdl( SvIdlDataBase &, SvTokenStream & rInStm );
};

class SvMetaReference : public SvMetaObject
{
    tools::SvRef<SvMetaReference>  aRef;
public:
            SvMetaReference();

    const OString &     GetName() const override
                        {
                            return ( !aRef.is()
                                    || !SvMetaObject::GetName().isEmpty() )
                                ? SvMetaObject::GetName()
                                : aRef->GetName();
                        }

    SvMetaReference *   GetRef() const { return aRef.get(); }
    void                SetRef( SvMetaReference * pRef  )
                        { aRef = pRef; }
};

#endif // INCLUDED_IDL_INC_BASOBJ_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
