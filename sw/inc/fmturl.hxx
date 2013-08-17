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
#ifndef _FMTURL_HXX
#define _FMTURL_HXX

#include <svl/poolitem.hxx>
#include "swdllapi.h"
#include <hintids.hxx>
#include <format.hxx>

class ImageMap;
class IntlWrapper;

// URL, ServerMap und ClientMap

class SW_DLLPUBLIC SwFmtURL: public SfxPoolItem
{
    OUString  sTargetFrameName; ///< Target frame for URL.
    OUString  sURL;             ///< Simple URL.
    OUString  sName;            ///< Name of the anchor.
    ImageMap *pMap;             ///< ClientSide images.

    sal_Bool      bIsServerMap;     ///< A ServerSideImageMap with the URL.

    SwFmtURL& operator=( const SwFmtURL& );

public:
    SwFmtURL();

    /// @@@ copy construction allowed, but assigment is not? @@@
    SwFmtURL( const SwFmtURL& );

    virtual ~SwFmtURL();

    /// "Pure virtual methods" of SfxPoolItem.
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText,
                                    const IntlWrapper*    pIntl = 0 ) const;
    virtual bool QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual bool PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

    void SetTargetFrameName( const OUString& rStr ) { sTargetFrameName = rStr; }
    void SetURL(const OUString &rURL, bool bServerMap);
    void SetMap( const ImageMap *pM );  ///< Pointer will be copied.

    OUString GetTargetFrameName()const { return sTargetFrameName; }
    OUString GetURL()            const { return sURL; }
          sal_Bool      IsServerMap()       const { return bIsServerMap; }
    const ImageMap *GetMap()            const { return pMap; }
          ImageMap *GetMap()                  { return pMap; }

    OUString GetName() const                { return sName; }
    void SetName( const OUString& rNm )     { sName = rNm; }
};


inline const SwFmtURL &SwAttrSet::GetURL(sal_Bool bInP) const
    { return (const SwFmtURL&)Get( RES_URL,bInP); }

inline const SwFmtURL &SwFmt::GetURL(sal_Bool bInP) const
    { return aSet.GetURL(bInP); }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
