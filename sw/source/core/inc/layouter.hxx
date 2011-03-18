/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
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

// --> #i28701#
class SwMovedFwdFrmsByObjPos;
class SwTxtFrm;
// <--
// --> #i26945#
class SwRowFrm;
// <--
// --> #i35911#
class SwObjsMarkedAsTmpConsiderWrapInfluence;
class SwAnchoredObject;
// <--
// --> #i40155#
#include <vector>
class SwFrm;
// <--
// --> #i65250#
#include <swtypes.hxx>
#include <boost/unordered_map.hpp>
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

    // --> #i28701#
    SwMovedFwdFrmsByObjPos* mpMovedFwdFrms;
    // <--
    // --> #i35911#
    SwObjsMarkedAsTmpConsiderWrapInfluence* mpObjsTmpConsiderWrapInfl;
    // <--
    // --> #i40155# - data structure to collect frames, which are
    // marked not to wrap around objects.
    std::vector< const SwFrm* > maFrmsNotToWrap;
    // <--

public:
    // --> #i65250#
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
    boost::unordered_map< const tMoveBwdLayoutInfoKey, sal_uInt16,
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

    // --> #i28701#
    static void ClearMovedFwdFrms( const SwDoc& _rDoc );
    static void InsertMovedFwdFrm( const SwDoc& _rDoc,
                                   const SwTxtFrm& _rMovedFwdFrmByObjPos,
                                   const sal_uInt32 _nToPageNum );
    static bool FrmMovedFwdByObjPos( const SwDoc& _rDoc,
                                     const SwTxtFrm& _rTxtFrm,
                                     sal_uInt32& _ornToPageNum );
    // <--
    // --> #i40155# - ummark given frame as to be moved forward.
    static void RemoveMovedFwdFrm( const SwDoc& _rDoc,
                                   const SwTxtFrm& _rTxtFrm );
    // <--
    // --> #i26945#
    static bool DoesRowContainMovedFwdFrm( const SwDoc& _rDoc,
                                           const SwRowFrm& _rRowFrm );
    // <--

    // --> #i35911#
    static void ClearObjsTmpConsiderWrapInfluence( const SwDoc& _rDoc );
    static void InsertObjForTmpConsiderWrapInfluence(
                                        const SwDoc& _rDoc,
                                        SwAnchoredObject& _rAnchoredObj );
    // <--
    // --> #i40155#
    static void ClearFrmsNotToWrap( const SwDoc& _rDoc );
    static void InsertFrmNotToWrap( const SwDoc& _rDoc,
                                    const SwFrm& _rFrm );
    static bool FrmNotToWrap( const IDocumentLayoutAccess& _rIDLA,
                              const SwFrm& _rFrm );
    // <--
    // --> #i65250#
    static bool MoveBwdSuppressed( const SwDoc& p_rDoc,
                                   const SwFlowFrm& p_rFlowFrm,
                                   const SwLayoutFrm& p_rNewUpperFrm );
    static void ClearMoveBwdLayoutInfo( const SwDoc& p_rDoc );
    // <--
};


extern void LOOPING_LOUIE_LIGHT( bool bCondition, const SwTxtFrm& rTxtFrm );

#endif  //_LAYOUTER_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
