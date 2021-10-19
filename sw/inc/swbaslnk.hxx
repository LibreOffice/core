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
#ifndef INCLUDED_SW_INC_SWBASLNK_HXX
#define INCLUDED_SW_INC_SWBASLNK_HXX

#include <sfx2/lnkbase.hxx>
#include <tools/solar.h>

class SwNode;
class SwContentNode;

class SwBaseLink : public ::sfx2::SvBaseLink
{
    SwContentNode* m_pContentNode;
    bool m_bNoDataFlag : 1;

public:

    SwBaseLink( SfxLinkUpdateMode nMode, SotClipboardFormatId nFormat, SwContentNode* pNode = nullptr )
        : ::sfx2::SvBaseLink( nMode, nFormat ), m_pContentNode( pNode ),
        m_bNoDataFlag( false )
    {}
    virtual ~SwBaseLink() override;

    virtual ::sfx2::SvBaseLink::UpdateResult DataChanged(
        const OUString& rMimeType, const css::uno::Any & rValue ) override;

    virtual void Closed() override;

    virtual const SwNode* GetAnchor() const;

    // For graphics only.
    bool SwapIn( bool bWaitForData = false, bool bNativFormat = false );

    void Connect() { SvBaseLink::GetRealObject(); }

    // Only for graphics-links (for switching between DDE / Grf-link).
    using SvBaseLink::SetObjType;

    bool IsRecursion( const SwBaseLink* pChkLnk ) const;
    virtual bool IsInRange( SwNodeOffset nSttNd, SwNodeOffset nEndNd ) const;

    void SetNoDataFlag() { m_bNoDataFlag = true; }
    bool ChkNoDataFlag() { const bool bRet = m_bNoDataFlag; m_bNoDataFlag = false; return bRet; }
    bool IsNoDataFlag() const { return m_bNoDataFlag; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
