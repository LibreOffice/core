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


#ifndef _FLYPOS_HXX
#define _FLYPOS_HXX


#include <svl/svarray.hxx>
#include <swdllapi.h>

class SwFrmFmt;
class SwNodeIndex;

// Struktur zum Erfragen der akt. freifliegenden Rahmen am Dokument.
class SW_DLLPUBLIC SwPosFlyFrm
{
    const SwFrmFmt* pFrmFmt;    // das FlyFrmFmt
//  SwPosition* pPos;           // Position in den ContentNode
    SwNodeIndex* pNdIdx;        // es reicht ein Index auf den Node
    sal_uInt32 nOrdNum;
public:
    SwPosFlyFrm( const SwNodeIndex& , const SwFrmFmt*, sal_uInt16 nArrPos );
    virtual ~SwPosFlyFrm(); // virtual fuer die Writer (DLL !!)

    // operatoren fuer das Sort-Array
    sal_Bool operator==( const SwPosFlyFrm& );
    sal_Bool operator<( const SwPosFlyFrm& );

    const SwFrmFmt& GetFmt() const { return *pFrmFmt; }
    const SwNodeIndex& GetNdIndex() const { return *pNdIdx; }
    sal_uInt32 GetOrdNum() const { return nOrdNum; }
};

typedef SwPosFlyFrm* SwPosFlyFrmPtr;
SV_DECL_PTRARR_SORT_VISIBILITY( SwPosFlyFrms, SwPosFlyFrmPtr, 0, 40, SW_DLLPUBLIC )

#endif // _FLYPOS_HXX
