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


#ifndef _FTNIDX_HXX
#define _FTNIDX_HXX


#define _SVSTDARR_USHORTS
#include <svl/svstdarr.hxx>

class SwTxtFtn;
class SwNodeIndex;
class SwSectionNode;

// ueberall, wo der NodeIndex gebraucht wird, werden die hier fehlenden
// Headerfiles schon includes. Darum hier nur als define und nicht als
// inline Methode (spart Compile-Zeit)
#define _SwTxtFtn_GetIndex( pFIdx ) (pFIdx->GetTxtNode().GetIndex())


typedef SwTxtFtn* SwTxtFtnPtr;
SV_DECL_PTRARR_SORT( _SwFtnIdxs, SwTxtFtnPtr, 0, 10 )

class SwFtnIdxs : public _SwFtnIdxs
{
public:
    SwFtnIdxs() {}

    void UpdateFtn( const SwNodeIndex& rStt );      // ab Pos. alle Updaten
    void UpdateAllFtn();                    // alle Fussnoten updaten

    SwTxtFtn* SeekEntry( const SwNodeIndex& rIdx, sal_uInt16* pPos = 0 ) const;
};


class SwUpdFtnEndNtAtEnd
{
    SvPtrarr aFtnSects, aEndSects;
    SvUShorts aFtnNums, aEndNums;

public:
    SwUpdFtnEndNtAtEnd() : aFtnSects( 0, 4 ), aEndSects( 0, 4 ),
                           aFtnNums( 0, 4 ), aEndNums( 0, 4 )
        {}

    static const SwSectionNode* FindSectNdWithEndAttr(
                                            const SwTxtFtn& rTxtFtn );

    sal_uInt16 GetNumber( const SwTxtFtn& rTxtFtn, const SwSectionNode& rNd );
    sal_uInt16 ChkNumber( const SwTxtFtn& rTxtFtn );
};



#endif // _FTNIDX_HXX

