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

#ifndef _CONTNR_HXX //autogen
#include <tools/contnr.hxx>
#endif

namespace binfilter {

#define DECLARE_CONTAINER_SORT_COMMON( ClassName, Type )						\
    ClassName( const ClassName& );											\
    ClassName& operator =( const ClassName& );								\
public:                                       								\
    using Container::Count;                    								\
                                                                            \
    ClassName( USHORT  InitSize, USHORT  ReSize ) :							\
        Container( CONTAINER_MAXBLOCKSIZE, InitSize, ReSize )	{}			\
                                                                            \
    BOOL Insert( Type* pObj );												\
                                                                               \
    Type *Remove( ULONG nPos ) 												\
        { return (Type *)Container::Remove( nPos ); }						\
                                                                            \
    Type *Remove( Type* pObj );												\
                                                                               \
    void DeleteAndDestroy( ULONG nPos )										\
    {                                  										\
        Type *pObj = Remove( nPos );   										\
        if( pObj )                     										\
            delete pObj;               										\
    }                                  										\
                                                                               \
    void DeleteAndDestroy()													\
        { while( Count() ) DeleteAndDestroy( 0 ); }							\
                                                                            \
    Type* GetObject( ULONG nPos ) const										\
        { return (Type *)Container::GetObject( nPos ); }					\
                                                                            \
    Type* operator[]( ULONG nPos ) const 									\
        { return GetObject(nPos); }											\
                                                                            \
    BOOL Seek_Entry( const Type *pObj, ULONG* pPos ) const;					\
                                                                            \
    ULONG GetPos( const Type* pObj ) const;									\


#define DECLARE_CONTAINER_SORT( ClassName, Type )							\
class ClassName : private Container											\
{																			\
    DECLARE_CONTAINER_SORT_COMMON( ClassName, Type )						\
    ~ClassName() {} 														\
};																			\


#define DECLARE_CONTAINER_SORT_DEL( ClassName, Type )							\
class ClassName : private Container											\
{																			\
    DECLARE_CONTAINER_SORT_COMMON( ClassName, Type )							\
    ~ClassName() { DeleteAndDestroy(); }									\
};																			\


#define IMPL_CONTAINER_SORT( ClassName, Type, SortFunc )					\
BOOL ClassName::Insert( Type *pObj )                                        \
{                                                                           \
    ULONG nPos;                                                             \
    BOOL bExist = Seek_Entry( pObj, &nPos );                                \
    if( !bExist )                                                           \
        Container::Insert( pObj, nPos );                                    \
    return !bExist;                                                         \
}                                                                           \
                                                                            \
Type *ClassName::Remove( Type* pObj )                                       \
{                                                                           \
    ULONG nPos;                                                             \
    if( Seek_Entry( pObj, &nPos ) )                                         \
        return Remove( nPos );                                              \
    else                                                                    \
        return 0;                                                           \
}                                                                           \
                                                                            \
ULONG ClassName::GetPos( const Type* pObj ) const                           \
{                                                                           \
    ULONG nPos;                                                             \
    if( Seek_Entry( pObj, &nPos ) )                                         \
        return nPos;                                                        \
    else                                                                    \
        return CONTAINER_ENTRY_NOTFOUND;                                    \
}                                                                           \
                                                                            \
BOOL ClassName::Seek_Entry( const Type* pObj, ULONG* pPos ) const           \
{                                                                           \
    register ULONG nO  = Count(),                                           \
            nM,                                                             \
            nU = 0;                                                         \
    if( nO > 0 )                                                            \
    {                                                                       \
        nO--;                                                               \
        while( nU <= nO )                                                   \
        {                                                                   \
            nM = nU + ( nO - nU ) / 2;                                      \
            int nCmp = SortFunc( *GetObject(nM), *pObj );				    \
                                                                            \
            if( 0 == nCmp )                              					\
            {                                                               \
                if( pPos ) *pPos = nM;                                      \
                return TRUE;                                                \
            }                                                               \
            else if( nCmp < 0 )                       						\
                nU = nM + 1;                                                \
            else if( nM == 0 )                                              \
            {                                                               \
                if( pPos ) *pPos = nU;                                      \
                return FALSE;                                               \
            }                                                               \
            else                                                            \
                nO = nM - 1;                                                \
        }                                                                   \
    }                                                                       \
    if( pPos ) *pPos = nU;                                                  \
    return FALSE;                                                           \
}                                                                           \

}

#endif
