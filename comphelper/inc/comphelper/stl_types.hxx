/*************************************************************************
 *
 *  $RCSfile: stl_types.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: fs $ $Date: 2000-10-06 14:01:11 $
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
 *  WITHOUT WARRUNTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRUNTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _COMPHELPER_STLTYPES_HXX_
#define _COMPHELPER_STLTYPES_HXX_

#if !defined(__SGI_STL_VECTOR_H) || !defined(__SGI_STL_MAP_H) || !defined(__SGI_STL_MULTIMAP_H)

#include <math.h> // prevent conflict between exception and std::exception
#include <vector>
#include <map>
#include <hash_map>
#include <stack>

using namespace std;

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _RTL_CHAR_H_
#include <rtl/char.h>
#endif

//... namespace comphelper ................................................
namespace comphelper
{
//.........................................................................

//========================================================================
// comparisation functions

//------------------------------------------------------------------------
struct UStringLess : public binary_function< ::rtl::OUString, ::rtl::OUString, bool>
{
    bool operator() (const ::rtl::OUString& x, const ::rtl::OUString& y) const { return x < y ? true : false;}      // construct prevents a MSVC6 warning
};
//------------------------------------------------------------------------
struct UStringMixLess : public binary_function< ::rtl::OUString, ::rtl::OUString, bool>
{
    bool m_bCaseSensitive;
public:
    UStringMixLess(bool bCaseSensitive = true):m_bCaseSensitive(bCaseSensitive){}
    bool operator() (const ::rtl::OUString& x, const ::rtl::OUString& y) const
    {
        if (m_bCaseSensitive)
            return x < y ? true : false;    // construct prevents a MSVC6 warning

        // unfortunally the ::rtl::OUString does not have something like "compareIgnoreCase" ..
        sal_Int32 nXLen = x.getLength();
        sal_Int32 nYLen = y.getLength();
        sal_Int32 nMinLen = nXLen < nYLen ? nXLen : nYLen;
        const sal_Unicode* pX = x.getStr();
        const sal_Unicode* pY = y.getStr();

        sal_Unicode cX;
        sal_Unicode cY;
        for (sal_Int32 i=0; i<nMinLen; ++i, ++pX, ++pY)
        {
            cX = rtl_char_toUpperCase(*pX);
            cY = rtl_char_toUpperCase(*pY);
            if (cX < cY)
                return sal_True;
            if (cX > cY)
                return sal_False;
        }
        // the first nMinLen characters are equal
        return nXLen < nYLen;
    }
    bool isCaseSensitive() const {return m_bCaseSensitive;}
};
//------------------------------------------------------------------------
struct UStringEqual
{
    sal_Bool operator() (const ::rtl::OUString& lhs, const ::rtl::OUString& rhs) const { return lhs.equals( rhs );}
};

//------------------------------------------------------------------------
struct UStringIEqual
{
    sal_Bool operator() (const ::rtl::OUString& lhs, const ::rtl::OUString& rhs) const { return lhs.equalsIgnoreCase( rhs );}
};

//------------------------------------------------------------------------
struct UStringHash
{
    size_t operator() (const ::rtl::OUString& rStr) const {return rStr.hashCode();}
};

//------------------------------------------------------------------------
class UStringMixEqual
{
    sal_Bool m_bCaseSensitive;

public:
    UStringMixEqual(sal_Bool bCaseSensitive = sal_True):m_bCaseSensitive(bCaseSensitive){}
    sal_Bool operator() (const ::rtl::OUString& lhs, const ::rtl::OUString& rhs) const
    {
        return m_bCaseSensitive ? lhs.equals( rhs ) : lhs.equalsIgnoreCase( rhs );
    }
    sal_Bool isCaseSensitive() const {return m_bCaseSensitive;}
};

//------------------------------------------------------------------------
class UStringMixHash
{
    sal_Bool m_bCaseSensitive;

public:
    UStringMixHash(sal_Bool bCaseSensitive = sal_True):m_bCaseSensitive(bCaseSensitive){}
    size_t operator() (const ::rtl::OUString& rStr) const
    {
        return m_bCaseSensitive ? rStr.hashCode() : rStr.toUpperCase().hashCode();
    }
    sal_Bool isCaseSensitive() const {return m_bCaseSensitive;}
};

//.........................................................................
}
//... namespace comphelper ................................................

//==================================================================
// consistently defining stl-types
//==================================================================

#define DECLARE_STL_ITERATORS(classname)                            \
    typedef classname::iterator         classname##Iterator;        \
    typedef classname::const_iterator   Const##classname##Iterator  \

#define DECLARE_STL_MAP(keytype, valuetype, comparefct, classname)  \
    typedef std::map< keytype, valuetype, comparefct >  classname;  \
    DECLARE_STL_ITERATORS(classname)                                \

#define DECLARE_STL_STDKEY_MAP(keytype, valuetype, classname)               \
    DECLARE_STL_MAP(keytype, valuetype, std::less< keytype >, classname)    \

#define DECLARE_STL_VECTOR(valuetyp, classname)     \
    typedef std::vector< valuetyp >     classname;  \
    DECLARE_STL_ITERATORS(classname)                \

#define DECLARE_STL_USTRINGACCESS_MAP(valuetype, classname)                 \
    DECLARE_STL_MAP(::rtl::OUString, valuetype, ::comphelper::UStringLess, classname)   \

#endif

#endif  // _COMPHELPER_STLTYPES_HXX_

