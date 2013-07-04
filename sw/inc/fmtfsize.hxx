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
#ifndef _FMTFSIZE_HXX
#define _FMTFSIZE_HXX

#include <tools/gen.hxx>
#include <svl/poolitem.hxx>
#include "swdllapi.h"
#include <hintids.hxx>
#include <swtypes.hxx>
#include <format.hxx>

class IntlWrapper;

//Frame size.

enum SwFrmSize
{
    ATT_VAR_SIZE,       ///< Frame is variable in Var-direction.
    ATT_FIX_SIZE,       ///< Frame cannot be moved in Var-direction.
    ATT_MIN_SIZE        /**< Value in Var-direction gives minimum
                         (can be exceeded but not be less). */
};

class SW_DLLPUBLIC SwFmtFrmSize: public SfxPoolItem
{
    Size      aSize;
    SwFrmSize eFrmHeightType;
    SwFrmSize eFrmWidthType;
    sal_uInt8     nWidthPercent;
    sal_uInt8     nHeightPercent;
    OUString strMarginLeft;
    OUString strMarginTop;
    OUString strMsoWidth;
    OUString strMsoHeight;
    OUString strZIndex;
    OUString strMsoWrapStyle;
    OUString strMsoTopPercent;
    OUString strMsoWrapDistanceLeft;
    OUString strMsoWrapDistanceTop;
    OUString strMsoWrapDistanceRight;
    OUString strMsoWrapDistanceBottom;
    OUString strMsoPositionHorizontalRelative;
    OUString strMsoPositionVerticalRelative;
    OUString strMsoWidthRelative;
    OUString strMsoHeightRelative;
    OUString strVTextAnchor;
    sal_Bool bOLockAspectRatio;
    OUString strOLockVExt;

    // For tables: width can be given in percent.

    // For frames: height and/or width may be given in percent.
    // If only one of these percentage values is given, the value 0xFF
    // used instead of the missing percentage value indicates this side
    // being proportional to the given one.
    // The calculation in this case is based upon the values in Size.
    // Percentages are always related to the environment in which
    // the object is placed (PrtArea) and to the screen width
    // minus borders in BrowseView if the environment is the page.



public:
    SwFmtFrmSize( SwFrmSize eSize = ATT_VAR_SIZE,
                  SwTwips nWidth = 0, SwTwips nHeight = 0 );
    SwFmtFrmSize& operator=( const SwFmtFrmSize& rCpy );

    /// "Pure virtual methods" of SfxPoolItem.
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText,
                                    const IntlWrapper*    pIntl = 0 ) const;
    virtual bool QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual bool PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

    SwFrmSize GetHeightSizeType() const { return eFrmHeightType; }
    void SetHeightSizeType( SwFrmSize eSize ) { eFrmHeightType = eSize; }

    SwFrmSize GetWidthSizeType() const { return eFrmWidthType; }
    void SetWidthSizeType( SwFrmSize eSize ) { eFrmWidthType = eSize; }

    const Size& GetSize() const { return aSize; }
          void  SetSize( const Size &rNew ) { aSize = rNew; }

    SwTwips GetHeight() const { return aSize.Height(); }
    SwTwips GetWidth()  const { return aSize.Width();  }
    void    SetHeight( const SwTwips nNew ) { aSize.Height() = nNew; }
    void    SetWidth ( const SwTwips nNew ) { aSize.Width()  = nNew; }

    sal_uInt8    GetHeightPercent() const{ return nHeightPercent; }
    sal_uInt8   GetWidthPercent() const { return nWidthPercent;  }
    void    SetHeightPercent( sal_uInt8 n ) { nHeightPercent = n; }
    void    SetWidthPercent ( sal_uInt8 n ) { nWidthPercent  = n; }

    OUString GetStrMarginLeft() const { return strMarginLeft; }
    OUString GetStrMarginTop() const { return strMarginTop; }
    OUString GetStrMsoWidth() const { return strMsoWidth; }
    OUString GetStrMsoHeight() const { return strMsoHeight; }
    OUString GetStrZIndex() const { return strZIndex; }
    OUString GetStrMsoWrapStyle() const { return strMsoWrapStyle; }
    OUString GetStrMsoTopPercent() const { return strMsoTopPercent; }
    OUString GetStrMsoWrapDistanceLeft() const { return strMsoWrapDistanceLeft; }
    OUString GetStrMsoWrapDistanceTop() const { return strMsoWrapDistanceTop; }
    OUString GetStrMsoWrapDistanceRight() const { return strMsoWrapDistanceRight; }
    OUString GetStrMsoWrapDistanceBottom() const { return strMsoWrapDistanceBottom; }
    OUString GetStrMsoPositionHorizontalRelative() const { return strMsoPositionHorizontalRelative; }
    OUString GetStrMsoPositionVerticalRelative() const { return strMsoPositionVerticalRelative; }
    OUString GetStrMsoWidthRelative() const { return strMsoWidthRelative; }
    OUString GetStrMsoHeightRelative() const { return strMsoHeightRelative; }
    OUString GetStrVTextAnchor() const { return strVTextAnchor; }
    sal_Bool GetOLockAspectRatio() const { return bOLockAspectRatio; }
    OUString GetStrOLockVExt() const { return strOLockVExt; }
};

inline const SwFmtFrmSize &SwAttrSet::GetFrmSize(sal_Bool bInP) const
    { return (const SwFmtFrmSize&)Get( RES_FRM_SIZE,bInP); }

inline const SwFmtFrmSize &SwFmt::GetFrmSize(sal_Bool bInP) const
    { return aSet.GetFrmSize(bInP); }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
