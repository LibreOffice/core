/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cntnrsrt.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 09:15:21 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

#ifndef _CONTNR_HXX //autogen
#include <tools/contnr.hxx>
#endif

#define DECLARE_CONTAINER_SORT_COMMON( ClassName, Type )                        \
    ClassName( const ClassName& );                                          \
    ClassName& operator =( const ClassName& );                              \
public:                                                                     \
    Container::Count;                                                       \
                                                                            \
    ClassName( USHORT nInitSize, USHORT nReSize ) :                         \
        Container( CONTAINER_MAXBLOCKSIZE, nInitSize, nReSize ) {}          \
                                                                            \
    BOOL Insert( Type* pObj );                                              \
                                                                               \
    Type *Remove( ULONG nPos )                                              \
        { return (Type *)Container::Remove( nPos ); }                       \
                                                                            \
    Type *Remove( Type* pObj );                                             \
                                                                               \
    void DeleteAndDestroy( ULONG nPos )                                     \
    {                                                                       \
        Type *pObj = Remove( nPos );                                        \
        if( pObj )                                                          \
            delete pObj;                                                    \
    }                                                                       \
                                                                               \
    void DeleteAndDestroy()                                                 \
        { while( Count() ) DeleteAndDestroy( 0 ); }                         \
                                                                            \
    Type* GetObject( ULONG nPos ) const                                     \
        { return (Type *)Container::GetObject( nPos ); }                    \
                                                                            \
    Type* operator[]( ULONG nPos ) const                                    \
        { return GetObject(nPos); }                                         \
                                                                            \
    BOOL Seek_Entry( const Type *pObj, ULONG* pPos ) const;                 \
                                                                            \
    ULONG GetPos( const Type* pObj ) const;                                 \


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
BOOL ClassName::Insert( Type *pObj )                                        \
{                                                                           \
    ULONG nPos;                                                             \
    BOOL bExist;                                                            \
    if( ! ( bExist = Seek_Entry( pObj, &nPos ) ) )                          \
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
            int nCmp = SortFunc( *GetObject(nM), *pObj );                   \
                                                                            \
            if( 0 == nCmp )                                                 \
            {                                                               \
                if( pPos ) *pPos = nM;                                      \
                return TRUE;                                                \
            }                                                               \
            else if( nCmp < 0 )                                             \
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

#endif
