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



#ifndef _SWCONT_HXX
#define _SWCONT_HXX

#include <tools/string.hxx>

class SwContentType;

//Reihenfolge und Anzahl mit ResIds abgleichen!!
#define CONTENT_TYPE_OUTLINE        0
#define CONTENT_TYPE_TABLE          1
#define CONTENT_TYPE_FRAME          2
#define CONTENT_TYPE_GRAPHIC        3
#define CONTENT_TYPE_OLE            4
#define CONTENT_TYPE_BOOKMARK       5
#define CONTENT_TYPE_REGION         6
#define CONTENT_TYPE_URLFIELD       7
#define CONTENT_TYPE_REFERENCE      8
#define CONTENT_TYPE_INDEX          9
#define CONTENT_TYPE_POSTIT         10
#define CONTENT_TYPE_DRAWOBJECT     11
#define CONTENT_TYPE_MAX            CONTENT_TYPE_DRAWOBJECT +1


// Typen fuer das Globaldokument
#define GLOBAL_CONTENT_REGION       100
#define GLOBAL_CONTENT_INDEX        101
#define GLOBAL_CONTENT_TEXT         102
#define GLOBAL_CONTENT_MAX          3

// Strings fuer Kontextmenue
#define CONTEXT_COUNT   12
#define GLOBAL_CONTEXT_COUNT 14

// Modi fuer Drag 'n Drop
#define REGION_MODE_NONE        0
#define REGION_MODE_LINK        1
#define REGION_MODE_EMBEDDED    2

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

//mini rtti
class SwTypeNumber
{
    sal_uInt8 nTypeId;

    public:
        SwTypeNumber(sal_uInt8 nId) :nTypeId(nId){}
        virtual ~SwTypeNumber();

        virtual sal_uInt8   GetTypeId();
};
//----------------------------------------------------------------------------

class SwContent : public SwTypeNumber
{
    const SwContentType*    pParent;
    String                  sContentName;
    long                    nYPosition;
    sal_Bool                    bInvisible;
public:
        SwContent(const SwContentType* pCnt, const String& rName, long nYPos );

    virtual sal_Bool            IsProtect() const;
    const SwContentType*    GetParent() const {return pParent;}
    const String&           GetName()   const {return sContentName;}
    int                     operator==(const SwContent& /*rCont*/) const
                                {
                                    //gleich sind sie nie, sonst fallen sie aus dem Array
                                    return sal_False;
                                }
    int                     operator<(const SwContent& rCont) const
                                {
                                    //zuerst nach Position dann nach Name sortieren
                                    return nYPosition != rCont.nYPosition ?
                                        nYPosition < rCont.nYPosition :
                                            sContentName < rCont.sContentName;;
                                }

    long        GetYPos() const {return nYPosition;}

    sal_Bool        IsInvisible() const {return bInvisible;}
    void        SetInvisible(){ bInvisible = sal_True;}
};

#endif
