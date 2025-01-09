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

#include <memory>
#include <svl/poolitem.hxx>
#include "swdllapi.h"
#include "hintids.hxx"
#include "format.hxx"

class ImageMap;
class IntlWrapper;

/// SfxPoolItem subclass that wraps a URL. This can appear in the item set of e.g. a
/// sw::SpzFrameFormat (Writer image).
class SW_DLLPUBLIC SwFormatURL final : public SfxPoolItem
{
    OUString  m_sTargetFrameName; ///< Target frame for URL.
    OUString  m_sURL;             ///< Simple URL.
    OUString  m_sName;            ///< Name of the anchor.
    std::unique_ptr<ImageMap>
              m_pMap;             ///< ClientSide images.

    bool      m_bIsServerMap;     ///< A ServerSideImageMap with the URL.

public:
    DECLARE_ITEM_TYPE_FUNCTION(SwFormatURL)
    SwFormatURL();

    SwFormatURL( const SwFormatURL& );

    virtual ~SwFormatURL() override;

    /// "Pure virtual methods" of SfxPoolItem.
    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual SwFormatURL*    Clone( SfxItemPool* pPool = nullptr ) const override;
    virtual bool GetPresentation( SfxItemPresentation ePres,
                                  MapUnit eCoreMetric,
                                  MapUnit ePresMetric,
                                  OUString &rText,
                                  const IntlWrapper& rIntl ) const override;
    virtual bool QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    void SetTargetFrameName( const OUString& rStr ) { m_sTargetFrameName = rStr; }
    void SetURL(const OUString &rURL, bool bServerMap);
    void SetMap( const ImageMap *pM );  ///< Pointer will be copied.

    const OUString& GetTargetFrameName()const { return m_sTargetFrameName; }
    const OUString& GetURL()            const { return m_sURL; }
          bool      IsServerMap()       const { return m_bIsServerMap; }
    const ImageMap *GetMap()            const { return m_pMap.get(); }
          ImageMap *GetMap()                  { return m_pMap.get(); }

    const OUString& GetName() const                { return m_sName; }
    void SetName( const OUString& rNm )     { m_sName = rNm; }
};

inline const SwFormatURL &SwAttrSet::GetURL(bool bInP) const
    { return Get( RES_URL,bInP); }

inline const SwFormatURL &SwFormat::GetURL(bool bInP) const
    { return m_aSet.GetURL(bInP); }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
