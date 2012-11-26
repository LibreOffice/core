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


#ifndef _CHARFMT_HXX
#define _CHARFMT_HXX

#include <format.hxx>

class SwCharFmt : public SwFmt
{
    friend class SwDoc;
    friend class SwTxtFmtColl;

    SwCharFmt( SwAttrPool& rPool, const sal_Char* pFmtName,
                SwCharFmt *pDerivedFrom )
          : SwFmt( rPool, pFmtName, aCharFmtSetRange, pDerivedFrom, RES_CHRFMT )
    {}
    SwCharFmt( SwAttrPool& rPool, const String &rFmtName,
                SwCharFmt *pDerivedFrom )
          : SwFmt( rPool, rFmtName, aCharFmtSetRange, pDerivedFrom, RES_CHRFMT )
    {}


public:
};

namespace CharFmt
{
    extern const SfxItemSet* GetItemSet( const SfxPoolItem& rAttr );
    extern const SfxPoolItem* GetItem( const SwTxtAttr& rAttr, sal_uInt16 nWhich );
    extern sal_Bool IsItemIncluded( const sal_uInt16 nWhich, const SwTxtAttr *pAttr );
}

#endif



