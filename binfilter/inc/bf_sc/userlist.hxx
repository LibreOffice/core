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


#ifndef SC_COLLECT_HXX
#include "collect.hxx"
#endif
namespace binfilter {

//------------------------------------------------------------------------
class ScUserListData : public DataObject
{
friend class ScUserList;
    String	aStr;
    USHORT	nTokenCount;
    String* pSubStrings;
    String* pUpperSub;

    void	InitTokens();

public:
                    ScUserListData(const String& rStr);
                    ScUserListData(const ScUserListData& rData);
/*N*/ 					ScUserListData( SvStream& rStream );
    virtual			~ScUserListData();

    virtual	DataObject*		Clone() const { return new ScUserListData(*this); }
/*N*/ 			BOOL			Store( SvStream& rStream ) const;
    const	String&			GetString() const { return aStr; }
            BOOL			GetSubIndex(const String& rSubStr, USHORT& rIndex) const;
};

//------------------------------------------------------------------------
class ScUserList : public Collection
{
public:
                    ScUserList( USHORT nLim = 4, USHORT nDel = 4);
                    ScUserList( const ScUserList& rUserList ) : Collection ( rUserList ) {}

    virtual	DataObject*		Clone() const;

            ScUserListData*	GetData( const String& rSubStr ) const;
/*N*/ 			BOOL			Load( SvStream& rStream );
/*N*/ 			BOOL			Store( SvStream& rStream ) const;
            /// If the list in rStr is already inserted
            BOOL            HasEntry( const String& rStr ) const;

    inline	ScUserListData*	operator[]( const USHORT nIndex) const;
    inline	ScUserList&		operator= ( const ScUserList& r );
};

inline	ScUserList& ScUserList::operator=( const ScUserList& r )
    { return (ScUserList&)Collection::operator=( r ); }

inline ScUserListData* ScUserList::operator[]( const USHORT nIndex) const
    { return (ScUserListData*)At(nIndex); }


} //namespace binfilter
#endif

