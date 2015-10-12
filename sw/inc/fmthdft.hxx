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
#ifndef INCLUDED_SW_INC_FMTHDFT_HXX
#define INCLUDED_SW_INC_FMTHDFT_HXX

#include <hintids.hxx>
#include <format.hxx>
#include <svl/poolitem.hxx>
#include <calbck.hxx>
#include <frmfmt.hxx>

class IntlWrapper;
class SwFormat;

 /** Header, for PageFormats
 Client of FrameFormat describing the header. */

class SW_DLLPUBLIC SwFormatHeader: public SfxPoolItem, public SwClient
{
    bool bActive;       ///< Only for controlling (creation of content).

public:
    SwFormatHeader( bool bOn = false );
    SwFormatHeader( SwFrameFormat *pHeaderFormat );
    SwFormatHeader( const SwFormatHeader &rCpy );
    virtual ~SwFormatHeader();
    SwFormatHeader& operator=( const SwFormatHeader &rCpy );

    TYPEINFO_OVERRIDE();

    /// "pure virtual methods" of SfxPoolItem
    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const override;
    virtual bool GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText,
                                    const IntlWrapper*    pIntl = 0 ) const override;

    const SwFrameFormat *GetHeaderFormat() const { return static_cast<const SwFrameFormat*>(GetRegisteredIn()); }
          SwFrameFormat *GetHeaderFormat()       { return static_cast<SwFrameFormat*>(GetRegisteredIn()); }

    void RegisterToFormat( SwFormat& rFormat );
    bool IsActive() const { return bActive; }
};

 /**Footer, for pageformats
 Client of FrameFormat describing the footer */

class SW_DLLPUBLIC SwFormatFooter: public SfxPoolItem, public SwClient
{
    bool bActive;       // Only for controlling (creation of content).

public:
    SwFormatFooter( bool bOn = false );
    SwFormatFooter( SwFrameFormat *pFooterFormat );
    SwFormatFooter( const SwFormatFooter &rCpy );
    virtual ~SwFormatFooter();
    SwFormatFooter& operator=( const SwFormatFooter &rCpy );

    TYPEINFO_OVERRIDE();

    /// "pure virtual methods" of SfxPoolItem
    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const override;
    virtual bool GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText,
                                    const IntlWrapper*    pIntl = 0 ) const override;

    const SwFrameFormat *GetFooterFormat() const { return static_cast<const SwFrameFormat*>(GetRegisteredIn()); }
          SwFrameFormat *GetFooterFormat()       { return static_cast<SwFrameFormat*>(GetRegisteredIn()); }

    void RegisterToFormat( SwFormat& rFormat );
    bool IsActive() const { return bActive; }
};

inline const SwFormatHeader &SwAttrSet::GetHeader(bool bInP) const
    { return static_cast<const SwFormatHeader&>(Get( RES_HEADER,bInP)); }
inline const SwFormatFooter &SwAttrSet::GetFooter(bool bInP) const
    { return static_cast<const SwFormatFooter&>(Get( RES_FOOTER,bInP)); }

inline const SwFormatHeader &SwFormat::GetHeader(bool bInP) const
    { return m_aSet.GetHeader(bInP); }
inline const SwFormatFooter &SwFormat::GetFooter(bool bInP) const
    { return m_aSet.GetFooter(bInP); }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
