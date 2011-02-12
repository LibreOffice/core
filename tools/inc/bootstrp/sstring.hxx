/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include <vector>

#define NOT_THERE       LIST_ENTRY_NOTFOUND

#define  StringList UniStringList

typedef ::std::vector< ByteString* > ByteStringList;
DECLARE_LIST( UniStringList, UniString* )

class SvStream;

// ---------------------
// - class SStringList -
// ---------------------

class SByteStringList
{
private:
    ByteStringList  maList;

public:
                SByteStringList();
                ~SByteStringList();

                // neuen ByteString in Liste einfuegen
    size_t      PutString( ByteString* );
    ByteString* RemoveString( const ByteString& rName );

                // Position des ByteString in Liste, wenn nicht enthalten, dann
                // return = NOT_THERE
    size_t      IsString( ByteString* );

                // Vorgaenger ermitteln ( auch wenn selbst noch nicht in
                // Liste enthalten
    size_t      GetPrevString( ByteString* );
    void        CleanUp();
    size_t      size() const;
    ByteString* erase( size_t i );

    SByteStringList&    operator<<  ( SvStream& rStream );
    SByteStringList&    operator>>  ( SvStream& rStream );
    ByteString*         operator[]( size_t i ) const;
    ByteString*         at( size_t i ) const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
