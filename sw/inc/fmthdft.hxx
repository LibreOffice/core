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
#ifndef _FMTHDFT_HXX
#define _FMTHDFT_HXX

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
    sal_Bool bActive;       ///< Only for controlling (creation of content).

public:
    SwFmtHeader( sal_Bool bOn = sal_False );
    SwFmtHeader( SwFrmFmt *pHeaderFmt );
    SwFmtHeader( const SwFmtHeader &rCpy );
    ~SwFmtHeader();
    SwFmtHeader& operator=( const SwFmtHeader &rCpy );

    TYPEINFO();

    /// "pure virtual methods" of SfxPoolItem
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText,
                                    const IntlWrapper*    pIntl = 0 ) const;

    const SwFrmFmt *GetHeaderFmt() const { return (SwFrmFmt*)GetRegisteredIn(); }
          SwFrmFmt *GetHeaderFmt()       { return (SwFrmFmt*)GetRegisteredIn(); }

    void RegisterToFormat( SwFmt& rFmt );
    sal_Bool IsActive() const { return bActive; }
    void SetActive( sal_Bool bNew = sal_True ) { bActive = bNew; }
};

 /**Footer, for pageformats
 Client of FrmFmt describing the footer */

class SW_DLLPUBLIC SwFmtFooter: public SfxPoolItem, public SwClient
{
    sal_Bool bActive;       // Only for controlling (creation of content).

public:
    SwFmtFooter( sal_Bool bOn = sal_False );
    SwFmtFooter( SwFrmFmt *pFooterFmt );
    SwFmtFooter( const SwFmtFooter &rCpy );
    ~SwFmtFooter();
    SwFmtFooter& operator=( const SwFmtFooter &rCpy );

    TYPEINFO();

    /// "pure virtual methods" of SfxPoolItem
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText,
                                    const IntlWrapper*    pIntl = 0 ) const;

    const SwFrmFmt *GetFooterFmt() const { return (SwFrmFmt*)GetRegisteredIn(); }
          SwFrmFmt *GetFooterFmt()       { return (SwFrmFmt*)GetRegisteredIn(); }

    void RegisterToFormat( SwFmt& rFmt );
    sal_Bool IsActive() const { return bActive; }
    void SetActive( sal_Bool bNew = sal_True ) { bActive = bNew; }
};

inline const SwFmtHeader &SwAttrSet::GetHeader(sal_Bool bInP) const
    { return (const SwFmtHeader&)Get( RES_HEADER,bInP); }
inline const SwFmtFooter &SwAttrSet::GetFooter(sal_Bool bInP) const
    { return (const SwFmtFooter&)Get( RES_FOOTER,bInP); }

inline const SwFmtHeader &SwFmt::GetHeader(sal_Bool bInP) const
    { return aSet.GetHeader(bInP); }
inline const SwFmtFooter &SwFmt::GetFooter(sal_Bool bInP) const
    { return aSet.GetFooter(bInP); }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
