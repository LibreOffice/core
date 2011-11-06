/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
