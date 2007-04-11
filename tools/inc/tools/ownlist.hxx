/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ownlist.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 20:14:27 $
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

#ifndef _TOOLS_OWNLIST_HXX
#define _TOOLS_OWNLIST_HXX

#ifndef _TOOLS_LIST_HXX //autogen
#include <tools/list.hxx>
#endif
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
