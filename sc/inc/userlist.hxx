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

#ifndef SC_USERLIST_HXX
#define SC_USERLIST_HXX

#include <tools/stream.hxx>
#include "scdllapi.h"
#include "collect.hxx"

//------------------------------------------------------------------------
class SC_DLLPUBLIC ScUserListData : public ScDataObject
{
friend class ScUserList;
    String  aStr;
    sal_uInt16  nTokenCount;
    String* pSubStrings;
    String* pUpperSub;

    SC_DLLPRIVATE void  InitTokens();

public:
                    ScUserListData(const String& rStr);
                    ScUserListData(const ScUserListData& rData);
    virtual         ~ScUserListData();

    virtual ScDataObject*       Clone() const { return new ScUserListData(*this); }

    const   String&         GetString() const { return aStr; }
            void            SetString( const String& rStr);
            sal_uInt16          GetSubCount() const;
            sal_Bool            GetSubIndex(const String& rSubStr, sal_uInt16& rIndex) const;
            String          GetSubStr(sal_uInt16 nIndex) const;
            StringCompare   Compare(const String& rSubStr1, const String& rSubStr2) const;
            StringCompare   ICompare(const String& rSubStr1, const String& rSubStr2) const;
};

//------------------------------------------------------------------------
class SC_DLLPUBLIC ScUserList : public ScCollection
{
public:
                    ScUserList( sal_uInt16 nLim = 4, sal_uInt16 nDel = 4);
                    ScUserList( const ScUserList& rUserList ) : ScCollection ( rUserList ) {}

    virtual ScDataObject*       Clone() const;

            ScUserListData* GetData( const String& rSubStr ) const;
            /// If the list in rStr is already inserted
            sal_Bool            HasEntry( const String& rStr ) const;

    inline  ScUserListData* operator[]( const sal_uInt16 nIndex) const;
    inline  ScUserList&     operator= ( const ScUserList& r );
            sal_Bool            operator==( const ScUserList& r ) const;
    inline  sal_Bool            operator!=( const ScUserList& r ) const;
};

inline  ScUserList& ScUserList::operator=( const ScUserList& r )
    { return (ScUserList&)ScCollection::operator=( r ); }

inline ScUserListData* ScUserList::operator[]( const sal_uInt16 nIndex) const
    { return (ScUserListData*)At(nIndex); }

inline sal_Bool ScUserList::operator!=( const ScUserList& r ) const
    { return !operator==( r ); }

#endif

