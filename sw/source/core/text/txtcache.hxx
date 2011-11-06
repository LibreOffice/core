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


#ifndef _TXTCACHE_HXX
#define _TXTCACHE_HXX

#include <sal/types.h>
#include <tools/mempool.hxx>
#include "swcache.hxx"

class SwParaPortion;
class SwTxtFrm;

class SwTxtLine : public SwCacheObj
{
    SwParaPortion *pLine;

public:
    DECL_FIXEDMEMPOOL_NEWDEL(SwTxtLine)

    SwTxtLine( SwTxtFrm *pFrm, SwParaPortion *pNew = 0 );
    virtual ~SwTxtLine();

    inline       SwParaPortion *GetPara()       { return pLine; }
    inline const SwParaPortion *GetPara() const { return pLine; }

    inline void SetPara( SwParaPortion *pNew ) { pLine = pNew; }
};


class SwTxtLineAccess : public SwCacheAccess
{

protected:
    virtual SwCacheObj *NewObj();

public:
    SwTxtLineAccess( const SwTxtFrm *pOwner );

    SwParaPortion *GetPara();

    inline SwTxtLine &GetTxtLine();

    virtual sal_Bool IsAvailable() const;
};


inline SwTxtLine &SwTxtLineAccess::GetTxtLine()
{
    return *((SwTxtLine*)Get());
}

#endif
