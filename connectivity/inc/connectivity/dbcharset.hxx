/*************************************************************************
 *
 *  $RCSfile: dbcharset.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: fs $ $Date: 2001-04-09 06:09:44 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
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

#ifndef _DBHELPER_DBCHARSET_HXX_
#define _DBHELPER_DBCHARSET_HXX_

#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif
#ifndef _RTL_TEXTENC_H
#include <rtl/textenc.h>
#endif
#include <rtl/ustring.hxx>

//.........................................................................
namespace dbtools
{
//.........................................................................

    //=========================================================================
    //= OCharsetMap
    //=========================================================================
    /** is a class which translates between different charset representations.

        <p>The set of recognized charsets is very limited: only the ones which are database relevant are
        implemented at the moment</p>

        <p>Possible representations are:
        <ul>
            <li><b>IANA names.</b>
            Have a look at <A href="ftp://ftp.isi.edu/in-notes/iana/assignments/character-sets">this document</A> for
            more details</li>
            <li><b>rtl_TextEncoding</b></li>
            <li><b>logical names</b> This are strings without a deeper meaning. They're introduced because different
            logical names can be matched to the same IANA name, so the opposite direction is ambiguous. That's why if
            you want to make a name persisten, you probably should use the logical name.</li>
        </ul>
        </p>
    */
    class OCharsetMap
    {
    protected:
        DECLARE_STL_VECTOR(rtl_TextEncoding, TextEncVector);
        TextEncVector   m_aEncodings;
        DECLARE_STL_VECTOR(::rtl::OUString, StringVector);
        StringVector    m_aIanaNames;
            // IANA names for the charsets
        StringVector    m_aNames;
            // logical names


    #ifdef _DEBUG
        sal_Int32       m_nLivingIterators;         /// just for debugging reasons, counts the living iterators
    #endif

    public:
        class CharsetIterator;
        friend class OCharsetMap::CharsetIterator;
        typedef CharsetIterator iterator;
        typedef CharsetIterator const_iterator;

        OCharsetMap();
        ~OCharsetMap();

        struct Logical { };
        struct IANA { };

        /** find the given text encoding in the map.
            @return the <em>end</em> iterator if the encoding could not be found.
        */
        CharsetIterator find(const rtl_TextEncoding _eEncoding) const;
        /** find the given IANA name in the map.
            @return the <em>end</em> iterator if the IANA name could not be found.
        */
        CharsetIterator find(const ::rtl::OUString& _rIanaName, const IANA&) const;
        /** find the given logical name in the map.
            @return the <em>end</em> iterator if the logical name could not be found.
        */
        CharsetIterator find(const ::rtl::OUString& _rLogicalName, const Logical&) const;

        sal_Int32   size() const { return m_aNames.size(); }

        /// get access to the first element of the charset collection
        CharsetIterator begin() const;
        /// get access to the (last + 1st) element of the charset collection
        CharsetIterator end() const;
    };

    //-------------------------------------------------------------------------
    //- CharsetIteratorDerefHelper
    //-------------------------------------------------------------------------
    class CharsetIteratorDerefHelper
    {
        friend class OCharsetMap::CharsetIterator;

        rtl_TextEncoding    m_eEncoding;
        ::rtl::OUString     m_aIanaName;
        ::rtl::OUString     m_aName;

    public:
        CharsetIteratorDerefHelper(const CharsetIteratorDerefHelper& _rSource);

        rtl_TextEncoding    getEncoding() const { return m_eEncoding; }
        ::rtl::OUString     getIanaName() const { return m_aIanaName; }
        ::rtl::OUString     getName() const     { return m_aName; }

    protected:
        CharsetIteratorDerefHelper();
        CharsetIteratorDerefHelper(const rtl_TextEncoding _eEncoding, const ::rtl::OUString& _rIanaName, const ::rtl::OUString& _rName);

    };


    //-------------------------------------------------------------------------
    //- OCharsetMap::CharsetIterator
    //-------------------------------------------------------------------------
    class OCharsetMap::CharsetIterator
    {
        friend class OCharsetMap;

        friend bool operator==(const CharsetIterator& lhs, const CharsetIterator& rhs);
        friend bool operator!=(const CharsetIterator& lhs, const CharsetIterator& rhs) { return !(lhs == rhs); }

        friend sal_Int32 operator-(const CharsetIterator& lhs, const CharsetIterator& rhs);

    protected:
        const OCharsetMap*  m_pContainer;
        sal_Int32           m_nPosition;

    public:
        CharsetIterator(const CharsetIterator& _rSource);
        ~CharsetIterator();

        CharsetIteratorDerefHelper operator*() const;
        // no -> operator
        // this would require us to a) store CharsetIteratorDerefHelper instances ourself so that we
        // can return a pointer or b) introduce a -> operator on the CharsetIteratorDerefHelper, too.

        /// prefix increment
        const CharsetIterator&  operator++();
        /// postfix increment
        const CharsetIterator   operator++(int) { CharsetIterator hold(*this); ++*this; return hold; }

        /// prefix decrement
        const CharsetIterator&  operator--();
        /// postfix decrement
        const CharsetIterator   operator--(int) { CharsetIterator hold(*this); --*this; return hold; }

    protected:
        CharsetIterator(const OCharsetMap* _pContainer, sal_Int32 _nInitialPos = 0);
    };

//.........................................................................
}   // namespace dbtools
//.........................................................................

#endif // _DBHELPER_DBCHARSET_HXX_

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.1  2000/11/29 22:21:00  fs
 *  initial checkin - helper class for translating charset representations
 *
 *
 *  Revision 1.0 29.11.00 18:29:26  fs
 ************************************************************************/

