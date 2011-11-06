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


#ifndef _LAYOUTER_HXX
#define _LAYOUTER_HXX

#include "swtypes.hxx"

class SwEndnoter;
class SwDoc;
class SwSectionFrm;
class SwFtnFrm;
class SwPageFrm;
class SwLooping;
class IDocumentLayoutAccess;

// --> OD 2004-06-23 #i28701#
class SwMovedFwdFrmsByObjPos;
class SwTxtFrm;
// <--
// --> OD 2004-10-05 #i26945#
class SwRowFrm;
// <--
// --> OD 2004-10-22 #i35911#
class SwObjsMarkedAsTmpConsiderWrapInfluence;
class SwAnchoredObject;
// <--
// --> OD 2005-01-12 #i40155#
#include <vector>
class SwFrm;
// <--
// --> OD 2006-05-10 #i65250#
#include <swtypes.hxx>
#include <hash_map>
class SwFlowFrm;
class SwLayoutFrm;
// <--

#define LOOP_PAGE 1

class SwLayouter
{
    SwEndnoter* pEndnoter;
    SwLooping* pLooping;
    void _CollectEndnotes( SwSectionFrm* pSect );
    sal_Bool StartLooping( SwPageFrm* pPage );

    // --> OD 2004-06-23 #i28701#
    SwMovedFwdFrmsByObjPos* mpMovedFwdFrms;
    // <--
    // --> OD 2004-10-22 #i35911#
    SwObjsMarkedAsTmpConsiderWrapInfluence* mpObjsTmpConsiderWrapInfl;
    // <--
    // --> OD 2005-01-12 #i40155# - data structure to collect frames, which are
    // marked not to wrap around objects.
    std::vector< const SwFrm* > maFrmsNotToWrap;
    // <--

public:
    // --> OD 2006-05-10 #i65250#
    // - data structure to collect moving backward layout information
    struct tMoveBwdLayoutInfoKey
    {
        // frame ID of flow frame
        sal_uInt32 mnFrmId;
        // position of new upper frame
        SwTwips mnNewUpperPosX;
        SwTwips mnNewUpperPosY;
        // size of new upper frame
        SwTwips mnNewUpperWidth;
        SwTwips mnNewUpperHeight;
        // free space in new upper frame
        SwTwips mnFreeSpaceInNewUpper;

    };
private:
    struct fMoveBwdLayoutInfoKeyHash
    {
        size_t operator()( const tMoveBwdLayoutInfoKey& p_key ) const
        {
            return p_key.mnFrmId;
        }
    };
    struct fMoveBwdLayoutInfoKeyEq
    {
        bool operator()( const tMoveBwdLayoutInfoKey& p_key1,
                         const tMoveBwdLayoutInfoKey& p_key2 ) const
        {
            return p_key1.mnFrmId == p_key2.mnFrmId &&
                   p_key1.mnNewUpperPosX == p_key2.mnNewUpperPosX &&
                   p_key1.mnNewUpperPosY == p_key2.mnNewUpperPosY &&
                   p_key1.mnNewUpperWidth == p_key2.mnNewUpperWidth &&
                   p_key1.mnNewUpperHeight == p_key2.mnNewUpperHeight &&
                   p_key1.mnFreeSpaceInNewUpper == p_key2.mnFreeSpaceInNewUpper;
        }
    };
    std::hash_map< const tMoveBwdLayoutInfoKey, sal_uInt16,
                   fMoveBwdLayoutInfoKeyHash,
                   fMoveBwdLayoutInfoKeyEq > maMoveBwdLayoutInfo;
    // <--
public:
    SwLayouter();
    ~SwLayouter();
    void InsertEndnotes( SwSectionFrm* pSect );
    void CollectEndnote( SwFtnFrm* pFtn );
    sal_Bool HasEndnotes() const;

    void LoopControl( SwPageFrm* pPage, sal_uInt8 nLoop );
    void EndLoopControl();
    void LoopingLouieLight( const SwDoc& rDoc, const SwTxtFrm& rFrm );

    static void CollectEndnotes( SwDoc* pDoc, SwSectionFrm* pSect );
    static sal_Bool Collecting( SwDoc* pDoc, SwSectionFrm* pSect, SwFtnFrm* pFtn );
    static sal_Bool StartLoopControl( SwDoc* pDoc, SwPageFrm *pPage );

    // --> OD 2004-06-23 #i28701#
    static void ClearMovedFwdFrms( const SwDoc& _rDoc );
    static void InsertMovedFwdFrm( const SwDoc& _rDoc,
                                   const SwTxtFrm& _rMovedFwdFrmByObjPos,
                                   const sal_uInt32 _nToPageNum );
    static bool FrmMovedFwdByObjPos( const SwDoc& _rDoc,
                                     const SwTxtFrm& _rTxtFrm,
                                     sal_uInt32& _ornToPageNum );
    // <--
    // --> OD 2005-01-12 #i40155# - ummark given frame as to be moved forward.
    static void RemoveMovedFwdFrm( const SwDoc& _rDoc,
                                   const SwTxtFrm& _rTxtFrm );
    // <--
    // --> OD 2004-10-05 #i26945#
    static bool DoesRowContainMovedFwdFrm( const SwDoc& _rDoc,
                                           const SwRowFrm& _rRowFrm );
    // <--

    // --> OD 2004-10-22 #i35911#
    static void ClearObjsTmpConsiderWrapInfluence( const SwDoc& _rDoc );
    static void InsertObjForTmpConsiderWrapInfluence(
                                        const SwDoc& _rDoc,
                                        SwAnchoredObject& _rAnchoredObj );
    // <--
    // --> OD 2005-01-12 #i40155#
    static void ClearFrmsNotToWrap( const SwDoc& _rDoc );
    static void InsertFrmNotToWrap( const SwDoc& _rDoc,
                                    const SwFrm& _rFrm );
    static bool FrmNotToWrap( const IDocumentLayoutAccess& _rIDLA,
                              const SwFrm& _rFrm );
    // <--
    // --> OD 2006-05-10 #i65250#
    static bool MoveBwdSuppressed( const SwDoc& p_rDoc,
                                   const SwFlowFrm& p_rFlowFrm,
                                   const SwLayoutFrm& p_rNewUpperFrm );
    static void ClearMoveBwdLayoutInfo( const SwDoc& p_rDoc );
    // <--
};


extern void LOOPING_LOUIE_LIGHT( bool bCondition, const SwTxtFrm& rTxtFrm );

#endif  //_LAYOUTER_HXX


