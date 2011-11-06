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



#ifndef _URLBMK_HXX
#define _URLBMK_HXX


#include <tools/string.hxx>

//=========================================================================

class INetBookmark

/*  [Beschreibung]

    Diese Klasse stellt ein Bookmark dar, welches aus einer URL und
    einem dazuge"horigen Beschreibungstext besteht.

    Es gibt ein eigenes Clipboardformat und Hilfsmethoden zum Kopieren
    und Einf"ugen in und aus Clipboard und DragServer.
*/

{
    String          aUrl;
    String          aDescr;

protected:

    void            SetURL( const String& rS )          { aUrl = rS; }
    void            SetDescription( const String& rS )  { aDescr = rS; }

public:
                    INetBookmark( const String &rUrl, const String &rDescr )
                        : aUrl( rUrl ), aDescr( rDescr )
                    {}
                    INetBookmark()
                    {}

    const String&   GetURL() const          { return aUrl; }
    const String&   GetDescription() const  { return aDescr; }
};


#endif

