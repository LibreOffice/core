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

#include <precomp.h>
#include <cosv/string.hxx>

// NOT FULLY DECLARED SERVICES
#include <string.h>
#include <cosv/comfunc.hxx>




namespace csv
{


inline const char *
str_from_StringOffset( const String &     i_rStr,
                       str::size       i_nOffset )
{
     return i_nOffset < i_rStr.size()
                ?   i_rStr.c_str() + i_nOffset
                :   "";
}

inline const char *
str_from_ptr( const char * i_str )
{

    return valid_str(i_str);
}


//*********************     String::S_Data    **********************//

inline String::
S_Data::S_Data()
    :   nCount(1)
{
}

String::
S_Data::S_Data( const char *        i_sData,
                size_type           i_nValidLength )
    :   aStr( str_from_ptr(i_sData),
              (i_nValidLength != str::maxsize
                    ?   i_nValidLength
                    :   strlen(i_sData)) ),
        nCount(1)
{
}

String::
S_Data::~S_Data()
{
    csv_assert( nCount == 0 );
}

const String::S_Data *
String::
S_Data::Acquire() const
{
#ifdef CSV_NO_MUTABLE
    ++ (const_cast< uintt& >(nCount));
#else
    ++nCount;
#endif
    return this;
}

void
String::
S_Data::Release() const
{
#ifdef CSV_NO_MUTABLE
    -- (const_cast< uintt& >(nCount));
#else
    --nCount;
#endif
    if (nCount == 0)
        delete (const_cast< S_Data* >(this));
}


//**************************     String    **************************//


String::String()
    :   pd( String::Null_().pd->Acquire() )
{
}

String::String( const char * i_str )
    :   pd( new S_Data(i_str) )
{
}

String::String( const char *        i_str,
                size_type           i_nLength )
    :   pd( new S_Data(i_str, i_nLength) )
{
}

String::String( const_iterator i_itBegin,
                const_iterator i_itEnd )
    :   pd( new S_Data(i_itBegin, size_type(i_itEnd - i_itBegin)) )
{
}

String::String( const self & i_rStr )
    :   pd( i_rStr.pd->Acquire() )
{
}

String::~String()
{
    pd->Release();
}


String &
String::operator=( const self & i_rStr )
{
    i_rStr.pd->Acquire();
    pd->Release();
    pd = i_rStr.pd;

    return *this;
}

String &
String::operator=( const char * i_str )
{
    const S_Data *
        pTemp = new S_Data(i_str);
    pd->Release();
    pd = pTemp;

    return *this;
}

void
String::assign( const char *        i_str,
                size_type           i_nLength )
{
    const S_Data *
        pTemp = new S_Data( i_str, i_nLength );
    pd->Release();
    pd = pTemp;
}

int
String::compare( const self & i_rStr ) const
{
    return strcmp( c_str(), i_rStr.c_str() );
}

int
String::compare( const CharOrder_Table & i_rOrder,
                 const self &            i_rStr ) const
{
    return csv::compare( i_rOrder, c_str(), i_rStr.c_str() );
}

const String &
String::Null_()
{
    // Must not use the default constructor! Because that one calls
    //   this function, which would create a circular dependency.
    static const String aNull_("");
    return aNull_;
}

const char &
String::Nulch_()
{
    static const char cNull_ = '\0';
    return cNull_;
}


int
compare( const String &      i_s1,
         csv::str::position        i_nStartPosition1,
         const char *              i_s2,
         csv::str::size            i_nLength )
{
    const char * pS1 = str_from_StringOffset( i_s1, i_nStartPosition1 );

    if ( i_nLength != csv::str::maxsize )
        return strncmp( pS1,
                        i_s2,
                        i_nLength );
    else
        return strcmp( pS1,
                       i_s2 );
}

int
compare( const CharOrder_Table &            i_rOrder,
         const char *                       i_s1,
         const char *                       i_s2 )
{
    const char * it1 = i_s1;
    const char * it2 = i_s2;
    for ( ; i_rOrder(*it1) == i_rOrder(*it2) AND *it1 != '\0'; ++it1, ++it2 )
    {}
    return int( i_rOrder(*it1) - i_rOrder(*it2) );
}

}   // namespace csv

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
