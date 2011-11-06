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



#ifndef SC_RFINDLST_HXX
#define SC_RFINDLST_HXX

#include <tools/color.hxx>
#include "global.hxx"
#include "address.hxx"

//==================================================================

struct ScRangeFindData
{
    ScRange     aRef;
    sal_uInt16      nFlags;
    xub_StrLen  nSelStart;
    xub_StrLen  nSelEnd;

    ScRangeFindData( const ScRange& rR, sal_uInt16 nF, xub_StrLen nS, xub_StrLen nE ) :
        aRef(rR), nFlags(nF), nSelStart(nS), nSelEnd(nE) {}
};

class ScRangeFindList
{
    List        aEntries;
    String      aDocName;
    sal_Bool        bHidden;

public:
            ScRangeFindList(const String& rName);
            ~ScRangeFindList();

    sal_uLong   Count() const                       { return aEntries.Count(); }
    void    Insert( ScRangeFindData* pNew )     { aEntries.Insert(pNew, LIST_APPEND); }
    ScRangeFindData* GetObject( sal_uLong nIndex ) const
                        { return (ScRangeFindData*)aEntries.GetObject(nIndex); }

    void    SetHidden( sal_Bool bSet )              { bHidden = bSet; }

    const String&   GetDocName() const          { return aDocName; }
    sal_Bool            IsHidden() const            { return bHidden; }

    static ColorData GetColorName( sal_uInt16 nIndex );
};



#endif


