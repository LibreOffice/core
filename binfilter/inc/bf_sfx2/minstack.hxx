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
#ifndef _SFXMINSTACK_HXX
#define _SFXMINSTACK_HXX

//ASDBG #ifndef _SFXMINARRAY_HXX
namespace binfilter {

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
    USHORT		Count() const { return ARR##arr_::Count(); } \
    void		Push( const T& rElem ) { Append( rElem ); } \
    const T& Top( USHORT nLevel = 0 ) const \
                { return (*this)[Count()-nLevel-1]; } \
    const T& Bottom() const { return (*this)[0]; } \
    T		 Pop(); \
    void		Clear() { ARR##arr_::Clear(); } \
    BOOL		Contains( const T& rItem ) const \
                { return ARR##arr_::Contains( rItem ); } \
}

#define IMPL_OBJSTACK( ARR, T ) \
IMPL_OBJARRAY( ARR##arr_, T ); \
\
T ARR::Pop() \
{	T aRet = (*this)[Count()-1]; \
    Remove( Count()-1, 1 ); \
    return aRet; \
}
//STRIP008 the following "DECL_PTRARRAY( ARR##arr_, T, nI, nG );\" should be "DECL_PTRARRAY( ARR##arr_, T, nI, nG )\"
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
    USHORT		Count() const { return ARR##arr_::Count(); } \
    void		Push( T rElem ) { Append( rElem ); } \
    BOOL        Replace( T rOldElem, T rNewElem ) \
                { return ARR##arr_::Replace( rOldElem, rNewElem ); } \
    T			Top( USHORT nLevel = 0 ) const \
                { return (*this)[Count()-nLevel-1]; } \
    T			Bottom() const { return (*this)[0]; } \
    T			Pop() \
                {	T aRet = (*this)[Count()-1]; \
                    Remove( Count()-1, 1 ); \
                    return aRet; \
                } \
    T*		 operator*() \
                { return &(*this)[Count()-1]; } \
    void		Clear() { ARR##arr_::Clear(); } \
    BOOL		Contains( const T pItem ) const \
                { return ARR##arr_::Contains( pItem ); } \
}

}//end of namespace binfilter
#endif

