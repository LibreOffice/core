/*************************************************************************
 *
 *  $RCSfile: dbcharset.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 16:38:11 $
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
#ifndef _RTL_TENCINFO_H
#include <rtl/tencinfo.h>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

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
            Have a look at <A href="http://www.iana.org/assignments/character-sets">this document</A> for
            more details</li>
            <li><b>rtl_TextEncoding</b></li>
        </ul>
        </p>
    */
    class OCharsetMap
    {
    protected:
        DECLARE_STL_STDKEY_SET( rtl_TextEncoding, TextEncBag );

        TextEncBag  m_aEncodings;

    public:
        class CharsetIterator;
        friend class OCharsetMap::CharsetIterator;
        typedef CharsetIterator iterator;
        typedef CharsetIterator const_iterator;

        OCharsetMap();
        ~OCharsetMap();

        struct IANA { };

        /** find the given text encoding in the map.
            @return the <em>end</em> iterator if the encoding could not be found.
        */
        CharsetIterator find(const rtl_TextEncoding _eEncoding) const;
        /** find the given IANA name in the map.
            @return the <em>end</em> iterator if the IANA name could not be found.
        */
        CharsetIterator find(const ::rtl::OUString& _rIanaName, const IANA&) const;

        sal_Int32   size() const { ensureConstructed( ); return m_aEncodings.size(); }

        /// get access to the first element of the charset collection
        CharsetIterator begin() const;
        /// get access to the (last + 1st) element of the charset collection
        CharsetIterator end() const;

    protected:
        // needed because we want to call a virtual method during construction
                void lateConstruct();
        inline  void ensureConstructed( ) const { if ( m_aEncodings.empty() ) const_cast< OCharsetMap* >( this )->lateConstruct(); }

        virtual sal_Bool approveEncoding( const rtl_TextEncoding _eEncoding, const rtl_TextEncodingInfo& _rInfo ) const;
    };

    //-------------------------------------------------------------------------
    //- CharsetIteratorDerefHelper
    //-------------------------------------------------------------------------
    class CharsetIteratorDerefHelper
    {
        friend class OCharsetMap::CharsetIterator;

        rtl_TextEncoding    m_eEncoding;
        ::rtl::OUString     m_aIanaName;

    public:
        CharsetIteratorDerefHelper(const CharsetIteratorDerefHelper& _rSource);

        rtl_TextEncoding    getEncoding() const { return m_eEncoding; }
        ::rtl::OUString     getIanaName() const { return m_aIanaName; }

    protected:
        CharsetIteratorDerefHelper();
        CharsetIteratorDerefHelper( const rtl_TextEncoding _eEncoding, const ::rtl::OUString& _rIanaName );

    };


    //-------------------------------------------------------------------------
    //- OCharsetMap::CharsetIterator
    //-------------------------------------------------------------------------
    class OCharsetMap::CharsetIterator
    {
        friend class OCharsetMap;

        friend bool operator==(const CharsetIterator& lhs, const CharsetIterator& rhs);
        friend bool operator!=(const CharsetIterator& lhs, const CharsetIterator& rhs) { return !(lhs == rhs); }

//      friend sal_Int32 operator-(const CharsetIterator& lhs, const CharsetIterator& rhs);

    protected:
        const OCharsetMap*                      m_pContainer;
        OCharsetMap::TextEncBag::const_iterator m_aPos;

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
        CharsetIterator(const OCharsetMap* _pContainer, OCharsetMap::TextEncBag::const_iterator _aPos );
    };

//.........................................................................
}   // namespace dbtools
//.........................................................................

#endif // _DBHELPER_DBCHARSET_HXX_

