/*************************************************************************
 *
 *  $RCSfile: charsets.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: fs $ $Date: 2000-10-11 11:31:54 $
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

#ifndef _DBAUI_CHARSETS_HXX_
#define _DBAUI_CHARSETS_HXX_

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _TOOLS_RC_HXX
#include <tools/rc.hxx>
#endif
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif

//.........................................................................
namespace dbaui
{
//.........................................................................

//=========================================================================
//= OCharsetCollection
//=========================================================================
class OCharsetCollection : public Resource
{
private:
    DECLARE_STL_VECTOR(String, StringVector);
    StringVector    m_aKeyList;
    StringVector    m_aNameList;

#ifdef DBG_UTIL
    sal_Int32       m_nLivingIterators;         /// just for debugging reasons, counts the living iterators
#endif

protected:
    String implLookUp(const String& _rKey, const StringVector& _rKeys, const StringVector& _rValues) const;

public:
    class CharsetIterator;
    friend class OCharsetCollection::CharsetIterator;

    OCharsetCollection();
    ~OCharsetCollection();

    String KeyToName(const String& _rKey) const { return implLookUp(_rKey, m_aKeyList, m_aNameList); }
    String NameToKey(const String& _rName) const { return implLookUp(_rName, m_aNameList, m_aKeyList); }

    /// get access to the first element of the charset collection
    CharsetIterator begin() const;
    /// get access to the (last + 1st) element of the charset collection
    CharsetIterator end() const;
};

//-------------------------------------------------------------------------
//- OCharsetCollection::CharsetIterator
//-------------------------------------------------------------------------
class OCharsetCollection::CharsetIterator
{
    friend class OCharsetCollection;

    friend bool operator==(const CharsetIterator& lhs, const CharsetIterator& rhs);
    friend bool operator!=(const CharsetIterator& lhs, const CharsetIterator& rhs) { return !(lhs == rhs); }

protected:
    const OCharsetCollection*   m_pContainer;
    sal_Int32                   m_nPosition;

public:
    CharsetIterator(const CharsetIterator& _rSource);
    ~CharsetIterator();

    String  getKey() const;
    String  getName() const;

    /// prefix increment
    const CharsetIterator&  operator++();
    /// postfix increment
    const CharsetIterator   operator++(int) { CharsetIterator hold(*this); ++*this; return hold; }

    /// prefix decrement
    const CharsetIterator&  operator--();
    /// postfix decrement
    const CharsetIterator   operator--(int) { CharsetIterator hold(*this); --*this; return hold; }

protected:
    CharsetIterator(const OCharsetCollection* _pContainer, sal_Int32 _nInitialPos = 0);
};

//.........................................................................
}   // namespace dbaui
//.........................................................................

#endif // _DBAUI_CHARSETS_HXX_

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.1  2000/10/05 10:07:57  fs
 *  initial checkin
 *
 *
 *  Revision 1.0 26.09.00 12:16:37  fs
 ************************************************************************/

