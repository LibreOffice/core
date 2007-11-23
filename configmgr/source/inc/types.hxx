/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: types.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 14:26:37 $
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

#ifndef INCLUDED_SHARABLE_BASETYPES_HXX
#define INCLUDED_SHARABLE_BASETYPES_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

//-----------------------------------------------------------------------------
namespace rtl { class OUString; }
//-----------------------------------------------------------------------------

namespace configmgr
{
    namespace sharable
    {
    //-----------------------------------------------------------------------------
    // some base types
        typedef sal_uInt16 Offset;  // Offset relative to 'this' in array of nodes

    // some derived types
        typedef rtl_uString *  Name;
        typedef rtl_uString *  String;
        typedef struct TreeFragment * List; // singly linked intrusive, used for set elements
        typedef sal_uInt8 *           Vector;   // points to counted sequence of some type

    //-----------------------------------------------------------------------------

        inline String allocString(::rtl::OUString const & _sString)
        {
            rtl_uString_acquire(_sString.pData);
            return _sString.pData;
        }
        inline void freeString(String _aString)
        {   rtl_uString_release(_aString);    }
        inline ::rtl::OUString readString(String _aString)
        {   return rtl::OUString(_aString);   }

    //-----------------------------------------------------------------------------

        inline Name allocName(::rtl::OUString const & _aName)
        {   return allocString(_aName); }
        inline void freeName(Name _aName)
        {   freeString(_aName); }
        inline ::rtl::OUString readName(Name _aName)
        {   return readString(_aName); }

    //-----------------------------------------------------------------------------
    }
//-----------------------------------------------------------------------------
}

#endif // INCLUDED_SHARABLE_BASETYPES_HXX
