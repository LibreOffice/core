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
#ifndef INCLUDED_SW_INC_FMTFSIZE_HXX
#define INCLUDED_SW_INC_FMTFSIZE_HXX

#include <sal/config.h>

#include <editeng/sizeitem.hxx>
#include <svl/poolitem.hxx>
#include "swdllapi.h"
#include "hintids.hxx"
#include "swtypes.hxx"
#include "format.hxx"

class IntlWrapper;

//Frame size.

enum SwFrameSize
{
    ATT_VAR_SIZE,       ///< Frame is variable in Var-direction.
    ATT_FIX_SIZE,       ///< Frame cannot be moved in Var-direction.
    ATT_MIN_SIZE        /**< Value in Var-direction gives minimum
                         (can be exceeded but not be less). */
};

class SW_DLLPUBLIC SwFormatFrameSize: public SvxSizeItem
{
    SwFrameSize m_eFrameHeightType;
    SwFrameSize m_eFrameWidthType;
    sal_uInt8 m_nWidthPercent;
    sal_Int16 m_eWidthPercentRelation;
    sal_uInt8 m_nHeightPercent;
    sal_Int16 m_eHeightPercentRelation;

    // For tables: width can be given in percent.

    // For frames: height and/or width may be given in percent.
    // If only one of these percentage values is given, the value 0xFF
    // used instead of the missing percentage value indicates this side
    // being proportional to the given one.
    // The calculation in this case is based upon the values in Size.
    // Percentages are always related to the environment in which
    // the object is placed (PrtArea) and to the screen width
    // minus borders in BrowseView if the environment is the page.

    void ScaleMetrics(long lMult, long lDiv) override;
    bool HasMetrics() const override;

public:
    SwFormatFrameSize( SwFrameSize eSize = ATT_VAR_SIZE,
                  SwTwips nWidth = 0, SwTwips nHeight = 0 );

    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = nullptr ) const override;
    virtual bool GetPresentation( SfxItemPresentation ePres,
                                  MapUnit eCoreMetric,
                                  MapUnit ePresMetric,
                                  OUString &rText,
                                  const IntlWrapper& rIntl ) const override;
    virtual bool QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    SwFrameSize GetHeightSizeType() const { return m_eFrameHeightType; }
    void SetHeightSizeType( SwFrameSize eSize ) { m_eFrameHeightType = eSize; }

    SwFrameSize GetWidthSizeType() const { return m_eFrameWidthType; }
    void SetWidthSizeType( SwFrameSize eSize ) { m_eFrameWidthType = eSize; }

    enum PercentFlags { SYNCED = 0xff };
    //0xff is reserved to indicate height is synced to width
    sal_uInt8   GetHeightPercent() const{ return m_nHeightPercent; }
    sal_Int16   GetHeightPercentRelation() const { return m_eHeightPercentRelation;  }
    //0xff is reserved to indicate width is synced to height
    sal_uInt8   GetWidthPercent() const { return m_nWidthPercent;  }
    sal_Int16   GetWidthPercentRelation() const { return m_eWidthPercentRelation;  }
    void    SetHeightPercent( sal_uInt8 n ) { m_nHeightPercent = n; }
    void    SetHeightPercentRelation ( sal_Int16 n ) { m_eHeightPercentRelation  = n; }
    void    SetWidthPercent ( sal_uInt8 n ) { m_nWidthPercent  = n; }
    void    SetWidthPercentRelation ( sal_Int16 n ) { m_eWidthPercentRelation  = n; }

    void dumpAsXml(struct _xmlTextWriter* pWriter) const override;
};

inline const SwFormatFrameSize &SwAttrSet::GetFrameSize(bool bInP) const
    { return Get( RES_FRM_SIZE,bInP); }

inline const SwFormatFrameSize &SwFormat::GetFrameSize(bool bInP) const
    { return m_aSet.GetFrameSize(bInP); }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
