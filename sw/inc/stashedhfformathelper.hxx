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

/// This helper class provides the possibility to save header/footer
/// content when changing the shared setting. This fature keeps that
/// content what in case of sharing not visible, and restores that when
/// the setting is changed back. This mechanism is a deep copy of the
/// header/footer what after the copy stored in the stash member of the
/// right page descriptor. On restoring the same happens accordingly.
/// The lifetime of stashed content handled automatically via unique
/// ptrs, so when the content not needed, the dtor calls the remove methods.
class SW_DLLPUBLIC SwStashedHFFormatHelper
{
    // These members stores the headers/footers according to their names.

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
    // Lifetime handling.
    SwStashedHFFormatHelper();
    SwStashedHFFormatHelper(const SwStashedHFFormatHelper&);
    ~SwStashedHFFormatHelper();

    // Calling of this, stores the content given by the first parameter.
    void Stash(const SwFormatHeader* pHeader, bool bLeft, bool bFirst);
    void Stash(const SwFormatFooter* pFooter, bool bLeft, bool bFirst);

    // This restores the right content
    const SwFormatHeader* GetStashedHeader(bool bLeft, bool bFirst) const;
    const SwFrameFormat* GetStashedHeaderFormat(bool bLeft, bool bFirst) const;
    const SwFormatFooter* GetStashedFooter(bool bLeft, bool bFirst) const;
    const SwFrameFormat* GetStashedFooterFormat(bool bLeft, bool bFirst) const;

    // This explicitly removes the right contnt when needed.
    void RemoveStashedFormat(bool bHeader, bool bLeft, bool bFirst);
    // Returns true when content hold according to the parameters.
    bool HasStashedFormat(bool bHeader, bool bLeft, bool bFirst) const;

private:
    // Does the copy between the given pointers
    static void Copy(const SwFormatHeader* pSource, std::unique_ptr<SwFormatHeader>& rpTarget,
                     std::unique_ptr<SwFrameFormat>& rpTargetFormat);
    static void Copy(const SwFormatFooter* pSource, std::unique_ptr<SwFormatFooter>& rpTarget,
                     std::unique_ptr<SwFrameFormat>& rpTargetFormat);
    // Does the clering
    static void Remove_Impl(std::unique_ptr<SwFormatHeader>& rpToDel,
                            std::unique_ptr<SwFrameFormat>& rpDelFormat);
    static void Remove_Impl(std::unique_ptr<SwFormatFooter>& rpToDel,
                            std::unique_ptr<SwFrameFormat>& rpDelFormat);
    // Copies the nodes. This is taken from swdoc.
    static void CopyHFContent_Impl(SwDoc& rDoc, const SwFormatContent& rSrc, SwFrameFormat* pTarget,
                                   bool bHeader);
};

typedef std::unique_ptr<SwStashedHFFormatHelper> SwStashedFormatHelperPtr;

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
