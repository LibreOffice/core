/*************************************************************************
 *
 *  $RCSfile: string.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2005-03-23 09:07:31 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

/*  For efficiency see the previous c'tor.
*/
String::String( const self &        i_rStr,
                position_type       i_nStartPosition,
                size_type           i_nLength )
    :   pd( new S_Data(str_from_StringOffset(i_rStr, i_nStartPosition), i_nLength) )
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
String::swap( self & i_rStr )
{
    const S_Data * pTemp = pd;
    pd = i_rStr.pd;
    i_rStr.pd = pTemp;
}

void
String::assign( const self &        i_rStr,
                position_type       i_nStartPosition,
                size_type           i_nLength )
{
    const S_Data *
        pTemp = new S_Data( str_from_StringOffset(i_rStr, i_nStartPosition),
                            i_nLength );
    pd->Release();
    pd = pTemp;
}

void
String::assign( const char *        i_str )
{
    const S_Data *
        pTemp = new S_Data( i_str );
    pd->Release();
    pd = pTemp;
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

void
String::assign( const_iterator      i_itBegin,
                const_iterator      i_itEnd )
{
    const S_Data *
        pTemp = new S_Data( i_itBegin,
                            size_type(i_itEnd - i_itBegin) );
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

String
String::substr( position_type       i_nStartPosition,
                size_type           i_nLength ) const
{
    size_type nSize = size();

    if ( i_nStartPosition < nSize )
    {
        if ( i_nLength == str::maxsize
             OR i_nLength >= nSize - i_nStartPosition )
             return String( c_str() + i_nStartPosition );
        else
            return String( c_str() + i_nStartPosition,
                                 i_nLength );
    }

    return Null_();
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
compare( const char *              i_s1,
         const String &      i_s2,
         csv::str::position        i_nStartPosition2,
         csv::str::size            i_nLength )
{
    const char * pS2 = str_from_StringOffset( i_s2, i_nStartPosition2 );

    if ( i_nLength != csv::str::maxsize )
        return strncmp( i_s1,
                        pS2,
                        i_nLength );
    else
        return strcmp( i_s1,
                       pS2 );
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

int
compare( const CharOrder_Table &   i_rOrder,
         const String &      i_s1,
         csv::str::position        i_nStartPosition1,
         const char *              i_s2,
         csv::str::size            i_nLength )
{
    const char * pS1 = str_from_StringOffset( i_s1, i_nStartPosition1 );

    if ( i_nLength != csv::str::maxsize )
        return compare( i_rOrder,
                        pS1,
                        i_s2,
                        i_nLength );
    else
        return compare( i_rOrder,
                        pS1,
                        i_s2 );
}

int
compare( const CharOrder_Table &   i_rOrder,
         const char *              i_s1,
         const String &            i_s2,
         csv::str::position        i_nStartPosition2,
         csv::str::size                 i_nLength )
{
    const char * pS2 = str_from_StringOffset( i_s2, i_nStartPosition2 );

    if ( i_nLength != csv::str::maxsize )
        return compare( i_rOrder,
                        i_s1,
                        pS2,
                        i_nLength );
    else
        return compare( i_rOrder,
                        i_s1,
                        pS2 );
}

int
compare( const CharOrder_Table &            i_rOrder,
         const char *                       i_s1,
         const char *                       i_s2,
         csv::str::size                     i_nLength )
{
    const char * sEnd = i_s1 + i_nLength;

    const char * it1 = i_s1;
    const char * it2 = i_s2;
    for ( ; i_rOrder(*it1) == i_rOrder(*it2) AND *it1 != '\0' AND it1 != sEnd; ++it1, ++it2 )
    {}

    if ( it1 != sEnd )
        return int( i_rOrder(*it1) - i_rOrder(*it2) );
    else
        return 0;
}


}   // namespace csv







