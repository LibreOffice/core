/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <memory>
#include <frmfmt.hxx>
#include <fmthdft.hxx>

using namespace ::com::sun::star;

class SW_DLLPUBLIC SwStashedHFFormatHelper
{
    std::unique_ptr<SwFormatHeader> m_pStashedFirstHeader;
    std::unique_ptr<SwFormatHeader> m_pStashedLeftHeader;
    std::unique_ptr<SwFormatHeader> m_pStashedFirstLeftHeader;

    std::unique_ptr<SwFormatFooter> m_pStashedFirstFooter;
    std::unique_ptr<SwFormatFooter> m_pStashedLeftFooter;
    std::unique_ptr<SwFormatFooter> m_pStashedFirstLeftFooter;

    std::unique_ptr<SwFrameFormat> m_pStashedFirstHeaderFormat;
    std::unique_ptr<SwFrameFormat> m_pStashedLeftHeaderFormat;
    std::unique_ptr<SwFrameFormat> m_pStashedFirstLeftHeaderFormat;

    std::unique_ptr<SwFrameFormat> m_pStashedFirstFooterFormat;
    std::unique_ptr<SwFrameFormat> m_pStashedLeftFooterFormat;
    std::unique_ptr<SwFrameFormat> m_pStashedFirstLeftFooterFormat;

public:
    SwStashedHFFormatHelper();
    SwStashedHFFormatHelper(const SwStashedHFFormatHelper&);
    ~SwStashedHFFormatHelper();

    void StashHeader(const SwFormatHeader* pHeader, bool bLeft, bool bFirst);
    void StashFooter(const SwFormatFooter* pFooter, bool bLeft, bool bFirst);

    const SwFormatHeader* GetStashedHeader(bool bLeft, bool bFirst) const;
    const SwFrameFormat* GetStashedHeaderFormat(bool bLeft, bool bFirst) const;
    const SwFormatFooter* GetStashedFooter(bool bLeft, bool bFirst) const;
    const SwFrameFormat* GetStashedFooterFormat(bool bLeft, bool bFirst) const;

    void RemoveStashedFormat(bool bHeader, bool bLeft, bool bFirst);
    bool HasStashedFormat(bool bHeader, bool bLeft, bool bFirst) const;
};

typedef std::unique_ptr<SwStashedHFFormatHelper> SwStashedFormatHelperPtr;

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
