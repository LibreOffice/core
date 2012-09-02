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
                                    String &rText,
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
};

inline const SwFmtFrmSize &SwAttrSet::GetFrmSize(sal_Bool bInP) const
    { return (const SwFmtFrmSize&)Get( RES_FRM_SIZE,bInP); }

inline const SwFmtFrmSize &SwFmt::GetFrmSize(sal_Bool bInP) const
    { return aSet.GetFrmSize(bInP); }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
