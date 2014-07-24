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

class SwFrmFmt;
class IntlWrapper;
class SwFmt;

 /** Header, for PageFormats
 Client of FrmFmt discribing the header. */

class SW_DLLPUBLIC SwFmtHeader: public SfxPoolItem, public SwClient
{
    bool bActive;       ///< Only for controlling (creation of content).

public:
    SwFmtHeader( bool bOn = false );
    SwFmtHeader( SwFrmFmt *pHeaderFmt );
    SwFmtHeader( const SwFmtHeader &rCpy );
    virtual ~SwFmtHeader();
    SwFmtHeader& operator=( const SwFmtHeader &rCpy );

    TYPEINFO_OVERRIDE();

    /// "pure virtual methods" of SfxPoolItem
    virtual bool            operator==( const SfxPoolItem& ) const SAL_OVERRIDE;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const SAL_OVERRIDE;
    virtual bool GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText,
                                    const IntlWrapper*    pIntl = 0 ) const SAL_OVERRIDE;

    const SwFrmFmt *GetHeaderFmt() const { return (SwFrmFmt*)GetRegisteredIn(); }
          SwFrmFmt *GetHeaderFmt()       { return (SwFrmFmt*)GetRegisteredIn(); }

    void RegisterToFormat( SwFmt& rFmt );
    bool IsActive() const { return bActive; }
    void SetActive( bool bNew = true ) { bActive = bNew; }
};

 /**Footer, for pageformats
 Client of FrmFmt describing the footer */

class SW_DLLPUBLIC SwFmtFooter: public SfxPoolItem, public SwClient
{
    bool bActive;       // Only for controlling (creation of content).

public:
    SwFmtFooter( bool bOn = false );
    SwFmtFooter( SwFrmFmt *pFooterFmt );
    SwFmtFooter( const SwFmtFooter &rCpy );
    virtual ~SwFmtFooter();
    SwFmtFooter& operator=( const SwFmtFooter &rCpy );

    TYPEINFO_OVERRIDE();

    /// "pure virtual methods" of SfxPoolItem
    virtual bool            operator==( const SfxPoolItem& ) const SAL_OVERRIDE;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const SAL_OVERRIDE;
    virtual bool GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText,
                                    const IntlWrapper*    pIntl = 0 ) const SAL_OVERRIDE;

    const SwFrmFmt *GetFooterFmt() const { return (SwFrmFmt*)GetRegisteredIn(); }
          SwFrmFmt *GetFooterFmt()       { return (SwFrmFmt*)GetRegisteredIn(); }

    void RegisterToFormat( SwFmt& rFmt );
    bool IsActive() const { return bActive; }
    void SetActive( bool bNew = true ) { bActive = bNew; }
};

inline const SwFmtHeader &SwAttrSet::GetHeader(bool bInP) const
    { return (const SwFmtHeader&)Get( RES_HEADER,bInP); }
inline const SwFmtFooter &SwAttrSet::GetFooter(bool bInP) const
    { return (const SwFmtFooter&)Get( RES_FOOTER,bInP); }

inline const SwFmtHeader &SwFmt::GetHeader(bool bInP) const
    { return aSet.GetHeader(bInP); }
inline const SwFmtFooter &SwFmt::GetFooter(bool bInP) const
    { return aSet.GetFooter(bInP); }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
