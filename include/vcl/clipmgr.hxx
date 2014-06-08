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

#ifndef INCLUDED_VCL_INC_CLIPMGR_HXX
#define INCLUDED_VCL_INC_CLIPMGR_HXX

#include <vcl/window.hxx>
#include <vcl/virdev.hxx>

#include <sal/types.h>


class VCL_DLLPUBLIC ClipManager
{
private:
    static bool instanceFlag;
    static ClipManager *single;
    ClipManager() {};

    void clipSiblings( Window* pWindow, Region& rRegion );
    void intersectClipRegion( Window* pWindow, Region& rRegion );
    void intersectAndUnionOverlapWindows(Window* pWindow, const Region& rInterRegion, Region& rRegion);
    void excludeOverlapWindows( Window *pWindow, Region& rRegion );
    void initChildClipRegion( Window *pWindow );

    bool clipSysObject( Window *pWindow, const Region* pOldRegion );
    void updateSysObjChildrenClip( Window *pWindow );
    void updateSysObjOverlapsClip( Window *pWindow );

public:
    ~ClipManager() { instanceFlag = false; }
    static ClipManager *GetInstance();

    void Init( Window *pWindow );
    void InitClipRegion( Window *pWindow );

    Region GetActiveClipRegion( const Window *pWindow ) const;
    void ClipToPaintRegion( Window *pWindow, Rectangle& rDstRect );

    bool SetClipFlag( Window *pWindow, bool bSysObjOnlySmaller = false );
    bool SetClipFlagChildren( Window *pWindow, bool bSysObjOnlySmaller = false );
    bool SetClipFlagOverlapWindows( Window *pWindow, bool bSysObjOnlySmaller = false );

    void EnableClipSiblings( Window *pWindow, bool bClipSiblings = true );
    void SetParentClipMode( Window* pWindow, sal_uInt16 nMode );
    sal_uInt16 GetParentClipMode( Window* pWindow ) const;

    bool IsOverlapWindow( Window* pWindow ) const;
    void CalcOverlapRegionOverlaps( Window* pWindow, const Region& rInterRegion, Region& rRegion );
    void CalcOverlapRegion( Window* pWindow, const Rectangle& rSourceRect, Region& rRegion,
                            bool bChildren, bool bParent, bool bSiblings );

    void ClipBoundaries( Window* pWindow, Region& rRegion, bool bThis, bool bOverlaps );
    bool ClipChildren( Window *pWindow, Region& rRegion );
    void ClipAllChildren( Window *pWindow, Region& rRegion );
    Region* GetChildClipRegion( Window* pWindow );
    bool ClipCoversWholeWindow( Window *pWindow );

    void SaveBackground( Window *pWindow, const Point& rPos, const Size& rSize,
                         const Point& rDestOff, VirtualDevice& rSaveDevice );
    void SaveOverlapBackground( Window *pWindow );
    bool RestoreOverlapBackground( Window *pWindow, Region& rInvRegion );
    void DeleteOverlapBackground( Window *pWindow );
    void InvalidateAllOverlapBackgrounds( Window *pWindow );

    void UpdateSysObjClip( Window *pWindow );

    void Intersect( Window* pWindow, Region& rRegion );
    void Exclude( Window *pWindow, Region& rRegion );

    /** Adds the submitted region to the paint clip region so you can
        paint additional parts of your window if necessary.
     */
    void ExpandPaintClipRegion( Window *pWindow, const Region& rRegion );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
