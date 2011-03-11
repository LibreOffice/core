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

#ifndef _RTTI_HXX
#define _RTTI_HXX

#include <string.h>
#include <tools/solar.h>

typedef void* (*TypeId)();

//-------------------------------------------------------------------------

#define TYPEINFO() \
        static  void*  CreateType(); \
        static  TypeId StaticType(); \
        static  sal_Bool   IsOf( TypeId aSameOrSuperType ); \
        virtual TypeId Type() const; \
        virtual sal_Bool   IsA( TypeId aSameOrSuperType ) const

#define TYPEINFO_VISIBILITY(visibility) \
        visibility static  void*  CreateType(); \
        visibility static  TypeId StaticType(); \
        visibility static  sal_Bool   IsOf( TypeId aSameOrSuperType ); \
        visibility virtual TypeId Type() const; \
        visibility virtual sal_Bool   IsA( TypeId aSameOrSuperType ) const

#define TYPEINIT_FACTORY(sType, Factory ) \
        void*  sType::CreateType() { return Factory; } \
        TypeId sType::StaticType() { return &CreateType; } \
        TypeId sType::Type() const { return &CreateType; } \
        sal_Bool sType::IsOf( TypeId aSameOrSuperType ) \
        { \
            if ( aSameOrSuperType == StaticType() ) \
                return sal_True

#define STATICTYPE(sType) (sType::StaticType())

//-------------------------------------------------------------------------

#define TYPEINIT_AUTOFACTORY(sType) TYPEINIT_FACTORY(sType, new sType)
#define TYPEINIT(sType) TYPEINIT_FACTORY(sType, 0)

#define SUPERTYPE(sSuper) \
            if ( sSuper::IsOf(aSameOrSuperType ) ) \
                return sal_True

#define TYPEINIT_END(sType) \
            return sal_False; \
        } \
        sal_Bool sType::IsA( TypeId aSameOrSuperType ) const \
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
#define TYPEINIT3_AUTOFACTORY(sType, sSuper1, sSuper2, sSuper3) \
            TYPEINIT3_FACTORY(sType, sSuper1, sSuper2, sSuper3, new sType)
#define TYPEINIT3(sType, sSuper1, sSuper2, sSuper3) \
            TYPEINIT3_FACTORY(sType, sSuper1, sSuper2, sSuper3, 0)

#define TYPE(sType) (sType::StaticType())
#define ISA(sType) IsA(sType::StaticType())
#define ISOF(sType) IsOf(sType::StaticType())
#define CREATE(TypeId) (TypeId())

//-------------------------------------------------------------------------
// On-Demand-faehige persistent-TypeId Version

#define TYPEINFO_ID(id) \
        static  TypeId StaticType() { return (TypeId) ( id | 0xF000000L ); } \
        static  sal_Bool   IsOf( TypeId aSameOrSuperType ); \
        virtual TypeId Type() const; \
        virtual sal_Bool   IsA( TypeId aSameOrSuperType ) const

#define TYPEINIT_ID(sType) \
        TypeId sType::Type() const { return StaticType(); } \
        sal_Bool   sType::IsOf( TypeId aSameOrSuperType ) \
        { \
            if ( aSameOrSuperType == StaticType() ) \
                return sal_True

#define TYPEINIT0_ID(sType) \
        TYPEINIT_ID(sType); \
        TYPEINIT_END(sType)

#define TYPEINIT1_ID(sType, sSuper) \
        TYPEINIT_ID(sType); \
            SUPERTYPE(sSuper); \
        TYPEINIT_END(sType)

#define TYPEINIT2_ID(sType, sSuper1, sSuper2) \
        TYPEINIT_ID(sType); \
            SUPERTYPE(sSuper1); \
            SUPERTYPE(sSuper2); \
        TYPEINIT_END(sType)

//-------------------------------------------------------------------------

//      Die (exemplarischen) Makros fuer die Anwendung ( hier fuer
//      Pointer, kann aber nach dem gleichen Strickmuster fuer
//      Referenzen erweitert werden.
//      PTR_CAST: sicheres Casten eines Pointers auf einen Pointer
//      einer abgeleiteten Klasse. Liefert im Fehlerfall einen
//      Nullpointer (wahrscheinlich die haeufigste Anwendung)
//
//      T: Typ, auf den gecastet werden soll
//      p: Pointer, der gecastet werden soll
#define PTR_CAST( T, pObj ) \
        ( pObj && (pObj)->IsA( TYPE(T) ) ? (T*)(pObj) : 0 )

//      Abfrage, ob ein Objekt eine bestimmte Klasse als
//      Basisklasse hat (oder genau von dieser Klasse ist)
#define HAS_BASE( T, pObj ) \
        ( pObj && (pObj)->IsA( TYPE(T) ) )

//      Abfrage, ob ein Pointer auf ein Objekt eines bestimmten
//      Typs zeigt
#define IS_TYPE(T,pObj) \
        ( pObj && (pObj)->Type() == TYPE(T) )

#endif // _RTTI_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
