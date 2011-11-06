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


#ifndef _TXTFLCNT_HXX
#define _TXTFLCNT_HXX

#include <txatbase.hxx>

class SwFlyInCntFrm;
class SwFrm;
class SwTxtNode;
class SwDoc;

// SWTXT_FLYCNT ********************************
// Attribut fuer zeilengebundene Frames (ohne Endindex)

class SwTxtFlyCnt : public SwTxtAttr
{
    SwFlyInCntFrm  *_GetFlyFrm( const SwFrm *pCurrFrm );

public:
    SwTxtFlyCnt( SwFmtFlyCnt& rAttr, xub_StrLen nStart );

    // Setzt den Anker im pFmt und
    void    SetAnchor( const SwTxtNode *pNode );

    inline        SwFlyInCntFrm  *GetFlyFrm( const SwFrm *pCurrFrm );
    inline const  SwFlyInCntFrm  *GetFlyFrm( const SwFrm *pCurrFrm ) const;

    // erzeugt sich ein neues FlyFrameFormat
    void CopyFlyFmt( SwDoc* pDoc );
};

inline SwFlyInCntFrm *SwTxtFlyCnt::GetFlyFrm( const SwFrm *pCurrFrm )
{
    return _GetFlyFrm( pCurrFrm );
}

inline const SwFlyInCntFrm *SwTxtFlyCnt::GetFlyFrm( const SwFrm *pCurrFrm ) const
{
    return (const_cast<SwTxtFlyCnt*>(this))->_GetFlyFrm( pCurrFrm );
}

#endif

