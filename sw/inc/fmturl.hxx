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
#ifndef INCLUDED_SW_INC_FMTURL_HXX
#define INCLUDED_SW_INC_FMTURL_HXX

#include <svl/poolitem.hxx>
#include "swdllapi.h"
#include <hintids.hxx>
#include <format.hxx>

class ImageMap;
class IntlWrapper;

// URL, ServerMap und ClientMap

class SW_DLLPUBLIC SwFormatURL: public SfxPoolItem
{
    OUString  sTargetFrameName; ///< Target frame for URL.
    OUString  sURL;             ///< Simple URL.
    OUString  sName;            ///< Name of the anchor.
    ImageMap *pMap;             ///< ClientSide images.

    bool      bIsServerMap;     ///< A ServerSideImageMap with the URL.

    SwFormatURL& operator=( const SwFormatURL& ) = delete;

public:
    SwFormatURL();

    /// @@@ copy construction allowed, but assigment is not? @@@
    SwFormatURL( const SwFormatURL& );

    virtual ~SwFormatURL();

    /// "Pure virtual methods" of SfxPoolItem.
    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const override;
    virtual bool GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText,
                                    const IntlWrapper*    pIntl = 0 ) const override;
    virtual bool QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    void SetTargetFrameName( const OUString& rStr ) { sTargetFrameName = rStr; }
    void SetURL(const OUString &rURL, bool bServerMap);
    void SetMap( const ImageMap *pM );  ///< Pointer will be copied.

    OUString GetTargetFrameName()const { return sTargetFrameName; }
    OUString GetURL()            const { return sURL; }
          bool      IsServerMap()       const { return bIsServerMap; }
    const ImageMap *GetMap()            const { return pMap; }
          ImageMap *GetMap()                  { return pMap; }

    OUString GetName() const                { return sName; }
    void SetName( const OUString& rNm )     { sName = rNm; }
};

inline const SwFormatURL &SwAttrSet::GetURL(bool bInP) const
    { return static_cast<const SwFormatURL&>(Get( RES_URL,bInP)); }

inline const SwFormatURL &SwFormat::GetURL(bool bInP) const
    { return m_aSet.GetURL(bInP); }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
