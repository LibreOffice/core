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
#ifndef _CNTRSRT_HXX
#define _CNTRSRT_HXX

#if 0
***********************************************************************
*
*   Hier folgt die Beschreibung fuer die exportierten Makros:
*
*       DECLARE_CONTAINER_SORT( ClassName, Type )
*       IMPL_CONTAINER_SORT( ClassName, Type, SortFunc )
*
*       Definiert eine von Container abgeleitete Klasse "ClassName",
*       in der die Elemente des Typs "Type" sortiert enthalten sind.
*       Dazu muss einer Funktion "SortFunc" definiert sein, die als
*       Paramter zwei "const Type&" erwartet und 0 zurueckgibt, wenn
*       beide gleich sind, -1 wenn der erste Paramter kleiner ist als
*       der zweite und +1 wenn der erste Paramter groesser ist als
*       der zweite.
*
*       Die Zugriffs-Methoden entsprechen in etwa denen der Container-
*       Klasse, mit Ausnahme von Insert, DeleteAndDestroy und Seek_Entry,
*       der den SV-Pointer-Arrays entsprechen.
*
*       DECLARE_CONTAINER_SORT_DEL( ClassName, Type )
*       IMPL_CONTAINER_SORT( ClassName, Type, SortFunc )
*
*       Wie DECLARE_CONTAINER_SORT, nur dass beim Aufruf des Destruktors
*       alle im Conatiner vorhandenen Objekte geloescht werden.
*
#endif

#include <tools/contnr.hxx>

#define DECLARE_CONTAINER_SORT_COMMON( ClassName, Type )                        \
    ClassName( const ClassName& );                                          \
    ClassName& operator =( const ClassName& );                              \
public:                                                                     \
    using Container::Count;                                                 \
                                                                            \
    ClassName( sal_uInt16  InitSize, sal_uInt16  ReSize ) :                         \
        Container( CONTAINER_MAXBLOCKSIZE, InitSize, ReSize )   {}          \
                                                                            \
    sal_Bool Insert( Type* pObj );                                              \
                                                                               \
    Type *Remove( sal_uLong nPos )                                              \
        { return (Type *)Container::Remove( nPos ); }                       \
                                                                            \
    Type *Remove( Type* pObj );                                             \
                                                                               \
    void DeleteAndDestroy( sal_uLong nPos )                                     \
    {                                                                       \
        Type *pObj = Remove( nPos );                                        \
        if( pObj )                                                          \
            delete pObj;                                                    \
    }                                                                       \
                                                                               \
    void DeleteAndDestroy()                                                 \
        { while( Count() ) DeleteAndDestroy( 0 ); }                         \
                                                                            \
    Type* GetObject( sal_uLong nPos ) const                                     \
        { return (Type *)Container::GetObject( nPos ); }                    \
                                                                            \
    Type* operator[]( sal_uLong nPos ) const                                    \
        { return GetObject(nPos); }                                         \
                                                                            \
    sal_Bool Seek_Entry( const Type *pObj, sal_uLong* pPos ) const;                 \
                                                                            \
    sal_uLong GetPos( const Type* pObj ) const;                                 \


#define DECLARE_CONTAINER_SORT( ClassName, Type )                           \
class ClassName : private Container                                         \
{                                                                           \
    DECLARE_CONTAINER_SORT_COMMON( ClassName, Type )                        \
    ~ClassName() {}                                                         \
};                                                                          \


#define DECLARE_CONTAINER_SORT_DEL( ClassName, Type )                           \
class ClassName : private Container                                         \
{                                                                           \
    DECLARE_CONTAINER_SORT_COMMON( ClassName, Type )                            \
    ~ClassName() { DeleteAndDestroy(); }                                    \
};                                                                          \


#define IMPL_CONTAINER_SORT( ClassName, Type, SortFunc )                    \
sal_Bool ClassName::Insert( Type *pObj )                                        \
{                                                                           \
    sal_uLong nPos;                                                             \
    sal_Bool bExist = Seek_Entry( pObj, &nPos );                                \
    if( !bExist )                                                           \
        Container::Insert( pObj, nPos );                                    \
    return !bExist;                                                         \
}                                                                           \
                                                                            \
Type *ClassName::Remove( Type* pObj )                                       \
{                                                                           \
    sal_uLong nPos;                                                             \
    if( Seek_Entry( pObj, &nPos ) )                                         \
        return Remove( nPos );                                              \
    else                                                                    \
        return 0;                                                           \
}                                                                           \
                                                                            \
sal_uLong ClassName::GetPos( const Type* pObj ) const                           \
{                                                                           \
    sal_uLong nPos;                                                             \
    if( Seek_Entry( pObj, &nPos ) )                                         \
        return nPos;                                                        \
    else                                                                    \
        return CONTAINER_ENTRY_NOTFOUND;                                    \
}                                                                           \
                                                                            \
sal_Bool ClassName::Seek_Entry( const Type* pObj, sal_uLong* pPos ) const           \
{                                                                           \
    register sal_uLong nO  = Count(),                                           \
            nM,                                                             \
            nU = 0;                                                         \
    if( nO > 0 )                                                            \
    {                                                                       \
        nO--;                                                               \
        while( nU <= nO )                                                   \
        {                                                                   \
            nM = nU + ( nO - nU ) / 2;                                      \
            int nCmp = SortFunc( *GetObject(nM), *pObj );                   \
                                                                            \
            if( 0 == nCmp )                                                 \
            {                                                               \
                if( pPos ) *pPos = nM;                                      \
                return sal_True;                                                \
            }                                                               \
            else if( nCmp < 0 )                                             \
                nU = nM + 1;                                                \
            else if( nM == 0 )                                              \
            {                                                               \
                if( pPos ) *pPos = nU;                                      \
                return sal_False;                                               \
            }                                                               \
            else                                                            \
                nO = nM - 1;                                                \
        }                                                                   \
    }                                                                       \
    if( pPos ) *pPos = nU;                                                  \
    return sal_False;                                                           \
}                                                                           \

#endif
