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



#ifndef _NOTEURL_HXX
#define _NOTEURL_HXX


#include <svl/svarray.hxx>
#include <tools/string.hxx>
#include "swrect.hxx"

class ImageMap;
class MapMode;

class SwURLNote
{
    String aURL;
    String aTarget;
    SwRect aRect;
public:
    SwURLNote( const String& rURL, const String& rTarget, const SwRect& rRect )
    : aURL( rURL ), aTarget( rTarget ), aRect( rRect )
    {}
    const String& GetURL()      const { return aURL; }
    const String& GetTarget()   const { return aTarget; }
    const SwRect& GetRect()     const { return aRect; }
    sal_Bool operator==( const SwURLNote& rSwURLNote ) const
    { return aRect == rSwURLNote.aRect; }
};

typedef SwURLNote* SwURLNotePtr;
SV_DECL_PTRARR_DEL(SwURLNoteList, SwURLNotePtr, 0, 5)

class SwNoteURL
{
    SwURLNoteList aList;
public:
    SwNoteURL() {}
    sal_uInt16 Count() const { return aList.Count(); }
    void InsertURLNote( const String& rURL, const String& rTarget,
                 const SwRect& rRect );
    const SwURLNote& GetURLNote( sal_uInt16 nPos ) const
        { return *aList.GetObject( nPos ); }
    void FillImageMap( ImageMap* pMap, const Point& rPos, const MapMode& rMap );
};

// globale Variable, in NoteURL.Cxx angelegt
extern SwNoteURL *pNoteURL;


#endif

