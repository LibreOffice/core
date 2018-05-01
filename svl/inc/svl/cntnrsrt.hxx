/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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
    sal_uLong nO  = Count(),                                           \
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
