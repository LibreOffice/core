/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sstring.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2006-08-01 09:20:15 $
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

#ifndef _SSTRING_HXX
#define _SSTRING_HXX

#include <string.hxx>
#include <list.hxx>

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
    ULONG       PutString( ByteString* );
    ByteString*     RemoveString( const ByteString& rName );

                // Position des ByteString in Liste, wenn nicht enthalten, dann
                // return = NOT_THERE
    ULONG       IsString( ByteString* );

                // Vorgaenger ermitteln ( auch wenn selbst noch nicht in
                // Liste enthalten
    ULONG       GetPrevString( ByteString* );
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
    ULONG       PutString( UniString* );
    UniString*  RemoveString( const UniString& rName );

                // Position des UniString in Liste, wenn nicht enthalten, dann
                // return = NOT_THERE
    ULONG       IsString( UniString* );

                // Vorgaenger ermitteln ( auch wenn selbst noch nicht in
                // Liste enthalten
    ULONG       GetPrevString( UniString* );
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
