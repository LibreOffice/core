/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_SW_SOURCE_CORE_INC_LAYOUTER_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_LAYOUTER_HXX

#include "swtypes.hxx"
#include <unordered_map>

class SwEndnoter;
class SwDoc;
class SwSectionFrame;
class SwFootnoteFrame;
class SwPageFrame;
class SwLooping;

class SwMovedFwdFramesByObjPos;
class SwTextFrame;
class SwRowFrame;
class SwObjsMarkedAsTmpConsiderWrapInfluence;
class SwAnchoredObject;
class SwFlowFrame;
class SwLayoutFrame;

#define LOOP_PAGE 1

class SwLayouter
{
    SwEndnoter* mpEndnoter;
    SwLooping* mpLooping;
    void _CollectEndnotes( SwSectionFrame* pSect );
    bool StartLooping( SwPageFrame* pPage );

    // --> #i28701#
    SwMovedFwdFramesByObjPos* mpMovedFwdFrames;
    // --> #i35911#
    SwObjsMarkedAsTmpConsiderWrapInfluence* mpObjsTmpConsiderWrapInfl;

public:
    // --> #i65250#
    // - data structure to collect moving backward layout information
    struct tMoveBwdLayoutInfoKey
    {
        // frame ID of flow frame
        sal_uInt32 mnFrameId;
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
            return p_key.mnFrameId;
        }
    };
    struct fMoveBwdLayoutInfoKeyEq
    {
        bool operator()( const tMoveBwdLayoutInfoKey& p_key1,
                         const tMoveBwdLayoutInfoKey& p_key2 ) const
        {
            return p_key1.mnFrameId == p_key2.mnFrameId &&
                   p_key1.mnNewUpperPosX == p_key2.mnNewUpperPosX &&
                   p_key1.mnNewUpperPosY == p_key2.mnNewUpperPosY &&
                   p_key1.mnNewUpperWidth == p_key2.mnNewUpperWidth &&
                   p_key1.mnNewUpperHeight == p_key2.mnNewUpperHeight &&
                   p_key1.mnFreeSpaceInNewUpper == p_key2.mnFreeSpaceInNewUpper;
        }
    };
    std::unordered_map< tMoveBwdLayoutInfoKey, sal_uInt16,
                   fMoveBwdLayoutInfoKeyHash,
                   fMoveBwdLayoutInfoKeyEq > maMoveBwdLayoutInfo;
public:
    SwLayouter();
    ~SwLayouter();
    void InsertEndnotes( SwSectionFrame* pSect );
    void CollectEndnote( SwFootnoteFrame* pFootnote );
    bool HasEndnotes() const;

    void LoopControl( SwPageFrame* pPage, sal_uInt8 nLoop );
    void EndLoopControl();
    void LoopingLouieLight( const SwDoc& rDoc, const SwTextFrame& rFrame );

    static void CollectEndnotes( SwDoc* pDoc, SwSectionFrame* pSect );
    static bool Collecting( SwDoc* pDoc, SwSectionFrame* pSect, SwFootnoteFrame* pFootnote );
    static bool StartLoopControl( SwDoc* pDoc, SwPageFrame *pPage );

    // --> #i28701#
    static void ClearMovedFwdFrames( const SwDoc& _rDoc );
    static void InsertMovedFwdFrame( const SwDoc& _rDoc,
                                   const SwTextFrame& _rMovedFwdFrameByObjPos,
                                   const sal_uInt32 _nToPageNum );
    static bool FrameMovedFwdByObjPos( const SwDoc& _rDoc,
                                     const SwTextFrame& _rTextFrame,
                                     sal_uInt32& _ornToPageNum );
    // --> #i40155# - unmark given frame as to be moved forward.
    static void RemoveMovedFwdFrame( const SwDoc& _rDoc,
                                   const SwTextFrame& _rTextFrame );
    // --> #i26945#
    static bool DoesRowContainMovedFwdFrame( const SwDoc& _rDoc,
                                           const SwRowFrame& _rRowFrame );

    // --> #i35911#
    static void ClearObjsTmpConsiderWrapInfluence( const SwDoc& _rDoc );
    static void InsertObjForTmpConsiderWrapInfluence(
                                        const SwDoc& _rDoc,
                                        SwAnchoredObject& _rAnchoredObj );
    // --> #i65250#
    static bool MoveBwdSuppressed( const SwDoc& p_rDoc,
                                   const SwFlowFrame& p_rFlowFrame,
                                   const SwLayoutFrame& p_rNewUpperFrame );
    static void ClearMoveBwdLayoutInfo( const SwDoc& p_rDoc );
};

extern void LOOPING_LOUIE_LIGHT( bool bCondition, const SwTextFrame& rTextFrame );

#endif // INCLUDED_SW_SOURCE_CORE_INC_LAYOUTER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
