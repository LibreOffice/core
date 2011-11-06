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



#ifndef _NAVICONT_HXX
#define _NAVICONT_HXX

#include <tools/string.hxx>

class SwDocShell;
class TransferDataContainer;
class TransferableDataHelper;

/*  [Beschreibung]
    Navigator-Bookmark zur eindeutigen Identifizierung im Sw
*/

class NaviContentBookmark
{
    String          aUrl;       // URL inkl. Sprungmarke
    String          aDescr;     // Description
    long            nDocSh;     // Adresse der DocShell
    sal_uInt16          nDefDrag;   // Description enthaelt defaultDragType

public:
    NaviContentBookmark();
    NaviContentBookmark( const String &rUrl, const String& rDesc,
                            sal_uInt16 nDragType, const SwDocShell* );

    const String&   GetURL() const              { return aUrl; }
    const String&   GetDescription() const      { return aDescr; }
    sal_uInt16          GetDefaultDragType() const  { return nDefDrag; }
    long            GetDocShell() const         { return nDocSh; }
    void            Copy( TransferDataContainer& rData ) const;
    sal_Bool            Paste( TransferableDataHelper& rData );
};

#endif
