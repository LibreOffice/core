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

#ifndef INCLUDED_TOOLS_RTTI_HXX
#define INCLUDED_TOOLS_RTTI_HXX

#include <string.h>
#include <tools/solar.h>

typedef void* (*TypeId)();

#define TYPEINFO() \
        static  void*  CreateType(); \
        static  TypeId StaticType(); \
        static  bool   IsOf( TypeId aSameOrSuperType ); \
        virtual TypeId Type() const; \
        virtual bool   IsA( TypeId aSameOrSuperType ) const

#define TYPEINFO_OVERRIDE() \
        static  void*  CreateType(); \
        static  TypeId StaticType(); \
        static  bool   IsOf( TypeId aSameOrSuperType ); \
        virtual TypeId Type() const SAL_OVERRIDE; \
        virtual bool   IsA( TypeId aSameOrSuperType ) const SAL_OVERRIDE

#define TYPEINFO_VISIBILITY(visibility) \
        visibility static  void*  CreateType(); \
        visibility static  TypeId StaticType(); \
        visibility static  bool   IsOf( TypeId aSameOrSuperType ); \
        visibility virtual TypeId Type() const SAL_OVERRIDE; \
        visibility virtual bool   IsA( TypeId aSameOrSuperType ) const SAL_OVERRIDE

#define TYPEINFO_VISIBILITY_OVERRIDE(visibility) \
        visibility static  void*  CreateType(); \
        visibility static  TypeId StaticType(); \
        visibility static  bool   IsOf( TypeId aSameOrSuperType ); \
        visibility virtual TypeId Type() const SAL_OVERRIDE; \
        visibility virtual bool   IsA( TypeId aSameOrSuperType ) const SAL_OVERRIDE

#define TYPEINIT_FACTORY(sType, Factory ) \
        void*  sType::CreateType() { return Factory; } \
        TypeId sType::StaticType() { return &CreateType; } \
        TypeId sType::Type() const { return &CreateType; } \
        bool sType::IsOf( TypeId aSameOrSuperType ) \
        { \
            if ( aSameOrSuperType == StaticType() ) \
                return true

#define STATICTYPE(sType) (sType::StaticType())

#define SUPERTYPE(sSuper) \
            if ( sSuper::IsOf(aSameOrSuperType ) ) \
                return true

#define TYPEINIT_END(sType) \
            return false; \
        } \
        bool sType::IsA( TypeId aSameOrSuperType ) const \
        { return IsOf( aSameOrSuperType ); }

#define TYPEINIT0_FACTORY(sType, Factory) \
        TYPEINIT_FACTORY(sType, Factory); \
        TYPEINIT_END(sType)
#define TYPEINIT0_AUTOFACTORY(sType) TYPEINIT0_FACTORY(sType, new sType)
#define TYPEINIT0(sType) TYPEINIT0_FACTORY(sType, 0)

#define TYPEINIT1_FACTORY(sType, sSuper, Factory) \
        TYPEINIT_FACTORY(sType, Factory); \
            SUPERTYPE(sSuper); \
        TYPEINIT_END(sType)
#define TYPEINIT1_AUTOFACTORY(sType, sSuper) \
            TYPEINIT1_FACTORY(sType, sSuper, new sType)
#define TYPEINIT1(sType, sSuper) \
            TYPEINIT1_FACTORY(sType, sSuper, 0)

#define TYPEINIT2_FACTORY(sType, sSuper1, sSuper2, Factory) \
        TYPEINIT_FACTORY(sType, Factory); \
            SUPERTYPE(sSuper1); \
            SUPERTYPE(sSuper2); \
        TYPEINIT_END(sType)
#define TYPEINIT2_AUTOFACTORY(sType, sSuper1, sSuper2) \
            TYPEINIT2_FACTORY(sType, sSuper1, sSuper2, new sType)
#define TYPEINIT2(sType, sSuper1, sSuper2) \
            TYPEINIT2_FACTORY(sType, sSuper1, sSuper2, 0)

#define TYPEINIT3_FACTORY(sType, sSuper1, sSuper2, sSuper3, Factory) \
        TYPEINIT_FACTORY(sType, Factory); \
            SUPERTYPE(sSuper1); \
            SUPERTYPE(sSuper2); \
            SUPERTYPE(sSuper3); \
        TYPEINIT_END(sType)
#define TYPEINIT3(sType, sSuper1, sSuper2, sSuper3) \
            TYPEINIT3_FACTORY(sType, sSuper1, sSuper2, sSuper3, 0)

#define TYPE(sType) (sType::StaticType())
#define ISA(sType) IsA(sType::StaticType())
#define ISOF(sType) IsOf(sType::StaticType())
#define CREATE(TypeId) (TypeId())

/** Exemplary application macros for pointers
    (can be extended for use with references)

    PTR_CAST: Safe pointer casting to a derived class.
              Returns NULL pointer on cast error

    T: Target type to cast into
    p: Pointer to be cast into T
*/
#define PTR_CAST( T, pObj ) rttiCast<T>(pObj, TYPE(T))

template<class T1, class T2>
inline T1* rttiCast(T2* pObj, const TypeId& rTypeId) {
    return (pObj && pObj->IsA( rTypeId )) ? static_cast<T1*>(pObj) : 0;
};

template<class T1, class T2>
inline const T1* rttiCast(const T2* pObj, const TypeId& rTypeId) {
    return (pObj && pObj->IsA( rTypeId )) ? static_cast<const T1*>(pObj) : 0;
};

/** Check whether object pObj has a Base Class T
    (or if pObj is an instance of T) */
#define HAS_BASE( T, pObj ) \
        ( pObj && (pObj)->IsA( TYPE(T) ) )

/** Check whether a pointer is targeting an object of type T. */
#define IS_TYPE(T,pObj) \
        ( pObj && (pObj)->Type() == TYPE(T) )

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
