/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: nametree.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:13:46 $
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

#include <precomp.h>
#include <nametree.hxx>


// NOT FULLY DECLARED SERVICES



namespace ary
{


/** Lexigraphical sequence is: AaBbCc ... Zz_0123456789.
*/
int cCompareValues[128] =
{
    0,  0,  0,  0,      0,  0,  0,  0,      0,  0,  0,  0,      0,  0,  0,  0,
    0,  0,  0,  0,      0,  0,  0,  0,      0,  0,  0,  0,      0,  0,  0,  0,
    0,  0,  0,  0,      0,  0,  0,  0,      0,  0,  0,  0,      0,  0,  0,  0,
   54, 55, 56, 57,     58, 59, 60, 61,     62, 63,  0,  0,      0,  0,  0,  0,

    0,  1,  3,  5,      7,  9, 11, 13,     15, 17, 19, 21,     23, 25, 27, 29,
   31, 33, 35, 37,     39, 41, 43, 45,     47, 49, 51,  0,      0,  0,  0, 53,

    0,  2,  4,  6,      8, 10, 12, 14,     16, 18, 20, 22,     24, 26, 28, 30,
   32, 34, 36, 38,     40, 42, 44, 46,     48, 50, 52,  0,      0,  0,  0,  0
};


#if 0
#ifdef WNT
#define strcmp_nocase   stricmp
#elif (UNX)
#define strcmp_nocase   strcasecmp
#else
#error  For running Autodoc, 'WNT' or 'UNX' must be defined.
#endif

bool
NameTree::
Less_Name::operator()( const udmstri &     i_r1,
                       const udmstri &     i_r2 ) const
{
    int result = strcmp_nocase(i_r1.c_str(),i_r2.c_str());
    if (result != 0)
        return result < 0;

    const unsigned char *
        p1 = reinterpret_cast< const unsigned char* >( i_r1.c_str() );
    const unsigned char *
        p2 = reinterpret_cast< const unsigned char* >( i_r2.c_str() );

    int cp = 0;
    do {
        cp = cCompareValues[*p1] - cCompareValues[*p2++];
        if ( cp < 0 )
            return true;
        if ( cp > 0 )
            return false;
    } while (*p1++ != 0);
    return false;
}
#endif // 0


NameTree::NameTree()
{
}

NameTree::~NameTree()
{
}

const InstanceList &
NameTree::operator[]( const udmstri & i_rName ) const
{
    static InstanceList aNull_;

    const_iterator itFound = aNames.find(i_rName);
    if ( itFound != aNames.end() )
        return (*itFound).second;
    return aNull_;
}

void
NameTree::insert( const udmstri &     i_rName,
                  ary::Rid            i_nId )
{


    aNames[i_rName].push_back(i_nId);
}

NameTree::const_iterator
NameTree::find( const udmstri & i_rName )
{
    return aNames.find( i_rName );
}

NameTree::const_iterator
NameTree::lower_bound( const udmstri & i_rName ) const
{
     return aNames.lower_bound(i_rName);
}

NameTree::const_iterator
NameTree::begin() const
{
    return aNames.begin();
}

NameTree::const_iterator
NameTree::end() const
{
    return aNames.end();
}

NameTree::iterator
NameTree::begin()
{
    return aNames.begin();
}

NameTree::iterator
NameTree::end()
{
    return aNames.end();
}






}   // namespace ary



