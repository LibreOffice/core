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

#ifndef _SSTRING_HXX
#define _SSTRING_HXX

#include <tools/string.hxx>
#include <tools/list.hxx>

#define NOT_THERE       LIST_ENTRY_NOTFOUND

#define  SStringList SUniStringList
#define  StringList UniStringList

DECLARE_LIST( ByteStringList, ByteString* )
DECLARE_LIST( UniStringList, UniString* )

class SvStream;

// ---------------------
// - class SStringList -
// ---------------------

class SByteStringList : public ByteStringList
{
public:
                SByteStringList();
                ~SByteStringList();

                // neuen ByteString in Liste einfuegen
    sal_uIntPtr     PutString( ByteString* );
    ByteString*     RemoveString( const ByteString& rName );

                // Position des ByteString in Liste, wenn nicht enthalten, dann
                // return = NOT_THERE
    sal_uIntPtr     IsString( ByteString* );

                // Vorgaenger ermitteln ( auch wenn selbst noch nicht in
                // Liste enthalten
    sal_uIntPtr     GetPrevString( ByteString* );
    void        CleanUp();

    SByteStringList& operator<<  ( SvStream& rStream );
    SByteStringList& operator>>  ( SvStream& rStream );
};

// ---------------------
// - class SUniStringList -
// ---------------------

class SUniStringList : public UniStringList
{
public:
                SUniStringList();
                ~SUniStringList();

                // neuen UniString in Liste einfuegen
    sal_uIntPtr     PutString( UniString* );
    UniString*  RemoveString( const UniString& rName );

                // Position des UniString in Liste, wenn nicht enthalten, dann
                // return = NOT_THERE
    sal_uIntPtr     IsString( UniString* );

                // Vorgaenger ermitteln ( auch wenn selbst noch nicht in
                // Liste enthalten
    sal_uIntPtr     GetPrevString( UniString* );
};

class Text
{
protected:
    String      aString;

public:
                Text( char* pChar );
                Text( String &rStr ) { aString = rStr; }
    void        Stderr();
};

#endif
