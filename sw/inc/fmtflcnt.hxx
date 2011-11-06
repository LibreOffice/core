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


#ifndef _FMTFLCNT_HXX
#define _FMTFLCNT_HXX


#include <svl/poolitem.hxx>

class SwFrmFmt;
class SwTxtFlyCnt;
// ATT_FLYCNT *******************************************************

class SwFmtFlyCnt : public SfxPoolItem
{
    friend class SwTxtFlyCnt;
    SwTxtFlyCnt* pTxtAttr;      // mein TextAttribut
    SwFrmFmt* pFmt;             // mein Fly/DrawFrame-Format
    // geschuetzter CopyCtor
    SwFmtFlyCnt& operator=(const SwFmtFlyCnt& rFlyCnt);

public:
    SwFmtFlyCnt( SwFrmFmt *pFrmFmt );
    // "pure virtual Methoden" vom SfxPoolItem
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const;

    inline SwFrmFmt *GetFrmFmt() const { return pFmt; }
    // fuer Undo: loesche "logisch" das FlyFrmFormat, wird sich im
    //          Undo-Object gemerkt.
    inline void SetFlyFmt( SwFrmFmt* pNew = 0 )   { pFmt = pNew; }

    const SwTxtFlyCnt *GetTxtFlyCnt() const { return pTxtAttr; }
          SwTxtFlyCnt *GetTxtFlyCnt()       { return pTxtAttr; }

    // OD 27.06.2003 #108784#
    bool Sw3ioExportAllowed() const;
};

#endif

