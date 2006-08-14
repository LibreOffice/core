/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: layouter.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2006-08-14 16:20:39 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
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
    BOOL StartLooping( SwPageFrm* pPage );

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
    BOOL HasEndnotes() const;

    void LoopControl( SwPageFrm* pPage, BYTE nLoop );
    void EndLoopControl();
    void LoopingLouieLight( const SwDoc& rDoc, const SwTxtFrm& rFrm );

    static void CollectEndnotes( SwDoc* pDoc, SwSectionFrm* pSect );
    static BOOL Collecting( SwDoc* pDoc, SwSectionFrm* pSect, SwFtnFrm* pFtn );
    static BOOL StartLoopControl( SwDoc* pDoc, SwPageFrm *pPage );

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


