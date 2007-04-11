/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: minstack.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 21:23:42 $
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
#ifndef _SFXMINSTACK_HXX
#define _SFXMINSTACK_HXX

//ASDBG #ifndef _SFXMINARRAY_HXX
#ifndef _SFXVARARR_HXX
#include <sfx2/minarray.hxx>
#endif

#define DECL_OBJSTACK( ARR, T, nI, nG ) \
DECL_OBJARRAY( ARR##arr_, T, nI, nG ); \
class ARR: private ARR##arr_ \
{ \
public: \
    ARR( BYTE nInitSize = nI, BYTE nGrowSize = nG ): \
        ARR##arr_( nInitSize, nGrowSize ) \
    {} \
\
    ARR( const ARR& rOrig ): \
        ARR##arr_( rOrig ) \
    {} \
\
    USHORT      Count() const { return ARR##arr_::Count(); } \
    void        Push( const T& rElem ) { Append( rElem ); } \
    const T& Top( USHORT nLevel = 0 ) const \
                { return (*this)[Count()-nLevel-1]; } \
    const T& Bottom() const { return (*this)[0]; } \
    T        Pop(); \
    void        Clear() { ARR##arr_::Clear(); } \
    BOOL        Contains( const T& rItem ) const \
                { return ARR##arr_::Contains( rItem ); } \
}

#define IMPL_OBJSTACK( ARR, T ) \
IMPL_OBJARRAY( ARR##arr_, T ); \
\
T ARR::Pop() \
{   T aRet = (*this)[Count()-1]; \
    Remove( Count()-1, 1 ); \
    return aRet; \
}

#define DECL_PTRSTACK( ARR, T, nI, nG ) \
DECL_PTRARRAY( ARR##arr_, T, nI, nG ) \
class ARR: private ARR##arr_ \
{ \
public: \
    ARR( BYTE nInitSize = nI, BYTE nGrowSize = nG ): \
        ARR##arr_( nInitSize, nGrowSize ) \
    {} \
\
    ARR( const ARR& rOrig ): \
        ARR##arr_( rOrig ) \
    {} \
\
    USHORT      Count() const { return ARR##arr_::Count(); } \
    void        Push( T rElem ) { Append( rElem ); } \
    BOOL        Replace( T rOldElem, T rNewElem ) \
                { return ARR##arr_::Replace( rOldElem, rNewElem ); } \
    T           Top( USHORT nLevel = 0 ) const \
                { return (*this)[Count()-nLevel-1]; } \
    T           Bottom() const { return (*this)[0]; } \
    T           Pop() \
                {   T aRet = (*this)[Count()-1]; \
                    Remove( Count()-1, 1 ); \
                    return aRet; \
                } \
    T*       operator*() \
                { return &(*this)[Count()-1]; } \
    void        Clear() { ARR##arr_::Clear(); } \
    BOOL        Contains( const T pItem ) const \
                { return ARR##arr_::Contains( pItem ); } \
}

#endif

