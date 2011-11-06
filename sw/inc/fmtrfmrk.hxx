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


#ifndef _FMTRFMRK_HXX
#define _FMTRFMRK_HXX


#include <tools/string.hxx>
#include <svl/poolitem.hxx>

class SwTxtRefMark;

// ATT_REFMARK *******************************************************

class SwFmtRefMark : public SfxPoolItem
{
    friend class SwTxtRefMark;
    SwTxtRefMark* pTxtAttr;     // mein TextAttribut

    // geschuetzter CopyCtor
    SwFmtRefMark& operator=(const SwFmtRefMark& rRefMark);
    String aRefName;

public:
    SwFmtRefMark( const String& rTxt );
    SwFmtRefMark( const SwFmtRefMark& rRefMark );
    ~SwFmtRefMark( );

    // "pure virtual Methoden" vom SfxPoolItem
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const;

    const SwTxtRefMark *GetTxtRefMark() const   { return pTxtAttr; }
    SwTxtRefMark *GetTxtRefMark()               { return pTxtAttr; }

    inline       String &GetRefName()       { return aRefName; }
    inline const String &GetRefName() const { return aRefName; }
};

#endif

