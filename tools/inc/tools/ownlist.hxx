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

#ifndef _TOOLS_OWNLIST_HXX
#define _TOOLS_OWNLIST_HXX

#include <tools/list.hxx>
/*************************************************************************
*************************************************************************/

#define PRV_SV_DECL_OWNER_LIST(ClassName,Type)                            \
    List  aTypes;                                                         \
public:                                                                   \
                        ClassName( USHORT nInitSize = 16,                 \
                                   USHORT nReSize = 16 )                  \
                            : aTypes( nInitSize, nReSize ) {}             \
                        ClassName( const ClassName & rObj )               \
                        { *this = rObj; }                                 \
    ClassName &         operator = ( const ClassName & );                 \
                        ~ClassName()                                      \
                        { Clear(); }                                      \
    void                Clear();                                          \
    void                Remove()                                          \
                        { delete (Type *)aTypes.Remove(); }               \
    void                Remove( Type * pObj )                             \
                        { delete (Type *)aTypes.Remove( pObj ); }         \
    void                Remove( ULONG nPos )                              \
                        { delete (Type *)aTypes.Remove( nPos ); }         \
    Type &              Insert( const Type &, ULONG nPos );               \
    Type &              Insert( const Type & rType )                      \
                        { return Insert( rType, aTypes.GetCurPos() ); }   \
    Type &              Append( const Type & rType )                      \
                        { return Insert( rType, LIST_APPEND ); }          \
    Type &              GetObject( ULONG nPos ) const                     \
                        { return *(Type *)aTypes.GetObject( nPos ); }     \
    Type &              operator []( ULONG nPos ) const                   \
                        { return *(Type *)aTypes.GetObject( nPos ); }     \
    ULONG               Count() const { return aTypes.Count(); }

#define PRV_SV_IMPL_OWNER_LIST(ClassName,Type)                          \
ClassName & ClassName::operator = ( const ClassName & rObj )            \
{                                                                       \
    if( this != &rObj )                                                 \
    {                                                                   \
        Clear();                                                        \
        for( ULONG i = 0; i < rObj.Count(); i++ )                       \
            Append( rObj.GetObject( i ) );                              \
    }                                                                   \
    return *this;                                                       \
}                                                                       \
void ClassName::Clear()                                                 \
{                                                                       \
    Type * p = (Type *)aTypes.First();                                  \
    while( p )                                                          \
    {                                                                   \
        delete p;                                                       \
        p = (Type *)aTypes.Next();                                      \
    }                                                                   \
    aTypes.Clear();                                                     \
}                                                                       \
Type & ClassName::Insert( const Type & rType, ULONG nPos )              \
{                                                                       \
    Type * pType = new Type( rType );                                   \
    aTypes.Insert( pType, nPos );                                       \
    return *pType;                                                      \
}

#endif // _TOOLS_OWNLIST_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
