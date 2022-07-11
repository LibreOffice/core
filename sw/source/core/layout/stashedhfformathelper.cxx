/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <stashedhfformathelper.hxx>
#include <doc.hxx>
#include <DocumentContentOperationsManager.hxx>
#include <fmtcntnt.hxx>
#include <ndindex.hxx>
#include <pam.hxx>

SwStashedHFFormatHelper::SwStashedHFFormatHelper()
{
    m_pStashedFirstHeader.reset();
    m_pStashedLeftHeader.reset();
    m_pStashedFirstLeftHeader.reset();

    m_pStashedFirstFooter.reset();
    m_pStashedLeftFooter.reset();
    m_pStashedFirstLeftFooter.reset();

    m_pStashedLeftHeaderFormat.reset();
    m_pStashedFirstHeaderFormat.reset();
    m_pStashedFirstLeftHeaderFormat.reset();

    m_pStashedLeftFooterFormat.reset();
    m_pStashedFirstFooterFormat.reset();
    m_pStashedFirstLeftFooterFormat.reset();
}

SwStashedHFFormatHelper::SwStashedHFFormatHelper(const SwStashedHFFormatHelper& r)
{
    if (r.m_pStashedLeftHeader && r.m_pStashedLeftHeaderFormat)
        Copy(r.m_pStashedLeftHeader.get(), m_pStashedLeftHeader, m_pStashedLeftHeaderFormat);

    if (r.m_pStashedFirstHeader && r.m_pStashedFirstHeaderFormat)
        Copy(r.m_pStashedFirstHeader.get(), m_pStashedFirstHeader, m_pStashedFirstHeaderFormat);

    if (r.m_pStashedFirstLeftHeader && r.m_pStashedFirstLeftHeaderFormat)
        Copy(r.m_pStashedFirstLeftHeader.get(), m_pStashedFirstLeftHeader,
             m_pStashedFirstLeftHeaderFormat);

    if (r.m_pStashedLeftFooter && r.m_pStashedLeftFooterFormat)
        Copy(r.m_pStashedLeftFooter.get(), m_pStashedLeftFooter, m_pStashedLeftFooterFormat);

    if (r.m_pStashedFirstFooter && r.m_pStashedFirstFooterFormat)
        Copy(r.m_pStashedFirstFooter.get(), m_pStashedFirstFooter, m_pStashedFirstFooterFormat);

    if (r.m_pStashedFirstLeftFooter && r.m_pStashedFirstLeftFooterFormat)
        Copy(r.m_pStashedFirstLeftFooter.get(), m_pStashedFirstLeftFooter,
             m_pStashedFirstLeftFooterFormat);
}

SwStashedHFFormatHelper::~SwStashedHFFormatHelper()
{
    if (m_pStashedLeftHeaderFormat && m_pStashedLeftHeader)
        Remove_Impl(m_pStashedLeftHeader, m_pStashedLeftHeaderFormat);

    if (m_pStashedFirstHeaderFormat && m_pStashedFirstHeader)
        Remove_Impl(m_pStashedFirstHeader, m_pStashedFirstHeaderFormat);

    if (m_pStashedFirstLeftHeaderFormat && m_pStashedFirstLeftHeader)
        Remove_Impl(m_pStashedFirstLeftHeader, m_pStashedFirstLeftHeaderFormat);

    if (m_pStashedLeftFooterFormat && m_pStashedLeftFooter)
        Remove_Impl(m_pStashedLeftFooter, m_pStashedLeftFooterFormat);

    if (m_pStashedFirstFooterFormat && m_pStashedFirstFooter)
        Remove_Impl(m_pStashedFirstFooter, m_pStashedFirstFooterFormat);

    if (m_pStashedFirstLeftFooterFormat && m_pStashedFirstLeftFooter)
        Remove_Impl(m_pStashedFirstLeftFooter, m_pStashedFirstLeftFooterFormat);
}

void SwStashedHFFormatHelper::Stash(const SwFormatHeader* pHeader, bool bLeft, bool bFirst)
{
    assert(pHeader && pHeader->GetRegisteredIn());

    if (bLeft && !bFirst)
        Copy(pHeader, m_pStashedLeftHeader, m_pStashedLeftHeaderFormat);

    else if (!bLeft && bFirst)
        Copy(pHeader, m_pStashedFirstHeader, m_pStashedFirstHeaderFormat);

    else if (bLeft && bFirst)
        Copy(pHeader, m_pStashedFirstLeftHeader, m_pStashedFirstLeftHeaderFormat);

    else
        SAL_WARN("sw", "SwStashedHFFormatHelper::Stash: Stashing the right page header is "
                       "pointless.");
}

void SwStashedHFFormatHelper::Stash(const SwFormatFooter* pFooter, bool bLeft, bool bFirst)
{
    assert(pFooter && pFooter->GetRegisteredIn());

    if (bLeft && !bFirst)
        Copy(pFooter, m_pStashedLeftFooter, m_pStashedLeftFooterFormat);

    else if (!bLeft && bFirst)
        Copy(pFooter, m_pStashedFirstFooter, m_pStashedFirstFooterFormat);

    else if (bLeft && bFirst)
        Copy(pFooter, m_pStashedFirstLeftFooter, m_pStashedFirstLeftFooterFormat);

    else
        SAL_WARN("sw", "SwStashedHFFormatHelper::Stash: Stashing the right page footer is "
                       "pointless.");
}

const SwFormatHeader* SwStashedHFFormatHelper::GetStashedHeader(bool bLeft, bool bFirst) const
{
    if (bLeft && !bFirst)
        return m_pStashedLeftHeader.get();
    else if (!bLeft && bFirst)
        return m_pStashedFirstHeader.get();
    else if (bLeft && bFirst)
        return m_pStashedFirstLeftHeader.get();

    SAL_WARN("sw", "SwStashedHFFormatHelper::GetStashedHeader: Right header is never stashed.");
    return nullptr;
}

const SwFrameFormat* SwStashedHFFormatHelper::GetStashedHeaderFormat(bool bLeft, bool bFirst) const
{
    if (bLeft && !bFirst)
        return m_pStashedLeftHeaderFormat.get();
    else if (!bLeft && bFirst)
        return m_pStashedFirstHeaderFormat.get();
    else if (bLeft && bFirst)
        return m_pStashedFirstLeftHeaderFormat.get();

    SAL_WARN(
        "sw",
        "SwStashedHFFormatHelper::GetStashedHeaderFormat: Right header format is never stashed.");
    return nullptr;
}

const SwFormatFooter* SwStashedHFFormatHelper::GetStashedFooter(bool bLeft, bool bFirst) const
{
    if (bLeft && !bFirst)
        return m_pStashedLeftFooter.get();
    else if (!bLeft && bFirst)
        return m_pStashedFirstFooter.get();
    else if (bLeft && bFirst)
        return m_pStashedFirstLeftFooter.get();

    SAL_WARN("sw", "SwStashedHFFormatHelper::GetStashedFooter: Right footer is never stashed.");
    return nullptr;
}

const SwFrameFormat* SwStashedHFFormatHelper::GetStashedFooterFormat(bool bLeft, bool bFirst) const
{
    if (bLeft && !bFirst)
        return m_pStashedLeftFooterFormat.get();
    else if (!bLeft && bFirst)
        return m_pStashedFirstFooterFormat.get();
    else if (bLeft && bFirst)
        return m_pStashedFirstLeftFooterFormat.get();

    SAL_WARN(
        "sw",
        "SwStashedHFFormatHelper::GetStashedFooterFormat: Right footer format is never stashed.");
    return nullptr;
}

void SwStashedHFFormatHelper::RemoveStashedFormat(bool bHeader, bool bLeft, bool bFirst)
{
    if (bHeader)
    {
        if (bLeft && !bFirst && m_pStashedLeftHeaderFormat && m_pStashedLeftHeader)
            Remove_Impl(m_pStashedLeftHeader, m_pStashedLeftHeaderFormat);
        else if (!bLeft && bFirst && m_pStashedFirstHeaderFormat && m_pStashedFirstHeader)
            Remove_Impl(m_pStashedFirstHeader, m_pStashedFirstHeaderFormat);
        else if (bLeft && bFirst && m_pStashedFirstLeftHeaderFormat && m_pStashedFirstLeftHeader)
            Remove_Impl(m_pStashedFirstLeftHeader, m_pStashedFirstLeftHeaderFormat);
        else
            SAL_WARN("sw", "SwStashedHFFormatHelper::RemoveStashedFormat: Right header is "
                           "never stashed.");
    }
    else
    {
        if (bLeft && !bFirst && m_pStashedLeftFooterFormat && m_pStashedLeftFooter)
            Remove_Impl(m_pStashedLeftFooter, m_pStashedLeftFooterFormat);
        else if (!bLeft && bFirst && m_pStashedFirstFooterFormat && m_pStashedFirstFooter)
            Remove_Impl(m_pStashedFirstFooter, m_pStashedFirstFooterFormat);
        else if (bLeft && bFirst && m_pStashedFirstLeftFooterFormat && m_pStashedFirstLeftFooter)
            Remove_Impl(m_pStashedFirstLeftFooter, m_pStashedFirstLeftFooterFormat);
        else
            SAL_WARN("sw", "SwStashedHFFormatHelper::RemoveStashedFormat: Right footer is "
                           "never stashed.");
    }
}

bool SwStashedHFFormatHelper::HasStashedFormat(bool bHeader, bool bLeft, bool bFirst) const
{
    if (bLeft && !bFirst)
    {
        if (bHeader)
            return m_pStashedLeftHeader.get() && m_pStashedLeftHeaderFormat.get();
        else
            return m_pStashedLeftFooter.get() && m_pStashedLeftFooterFormat.get();
    }
    else if (!bLeft && bFirst)
    {
        if (bHeader)
            return m_pStashedFirstHeader.get() && m_pStashedFirstHeaderFormat.get();
        else
            return m_pStashedFirstFooter.get() && m_pStashedFirstFooterFormat.get();
    }
    else if (bLeft && bFirst)
    {
        if (bHeader)
            return m_pStashedFirstLeftHeader.get() && m_pStashedFirstLeftHeaderFormat.get();
        else
            return m_pStashedFirstLeftFooter.get() && m_pStashedFirstLeftFooterFormat.get();
    }

    SAL_WARN("sw", "SwStashedFormatHelper::HasStashedFormat: Right page format is never stashed.");
    return false;
}

void SwStashedHFFormatHelper::Copy(const SwFormatHeader* pSource,
                                   std::unique_ptr<SwFormatHeader>& rpTarget,
                                   std::unique_ptr<SwFrameFormat>& rpTargetFormat)
{
    assert(pSource);
    if (!pSource->GetHeaderFormat())
        return;

    rpTarget = std::make_unique<SwFormatHeader>(SwFormatHeader(*pSource));
    rpTargetFormat = std::make_unique<SwFrameFormat>(SwFrameFormat(*pSource->GetHeaderFormat()));

    rpTarget->RegisterToFormat(*rpTargetFormat);

    assert(rpTargetFormat->GetDoc());

    CopyHFContent_Impl(*rpTargetFormat->GetDoc(), pSource->GetHeaderFormat()->GetContent(),
                       rpTargetFormat.get(), true);
}

void SwStashedHFFormatHelper::Copy(const SwFormatFooter* pSource,
                                   std::unique_ptr<SwFormatFooter>& rpTarget,
                                   std::unique_ptr<SwFrameFormat>& rpTargetFormat)
{
    assert(pSource);
    if (!pSource->GetFooterFormat())
        return;

    rpTarget = std::make_unique<SwFormatFooter>(SwFormatFooter(*pSource));
    rpTargetFormat = std::make_unique<SwFrameFormat>(SwFrameFormat(*pSource->GetFooterFormat()));

    rpTarget->RegisterToFormat(*rpTargetFormat);

    assert(rpTargetFormat->GetDoc());

    CopyHFContent_Impl(*rpTargetFormat->GetDoc(), pSource->GetFooterFormat()->GetContent(),
                       rpTargetFormat.get(), false);
}

void SwStashedHFFormatHelper::Remove_Impl(std::unique_ptr<SwFormatHeader>& rpToDel,
                                          std::unique_ptr<SwFrameFormat>& rpDelFormat)
{
    if (rpDelFormat->GetDoc()->IsInDtor())
        SwPageDesc::DelHFFormat(rpToDel.get(), rpDelFormat.get());

    rpToDel.release();
    rpDelFormat.release();
}

void SwStashedHFFormatHelper::Remove_Impl(std::unique_ptr<SwFormatFooter>& rpToDel,
                                          std::unique_ptr<SwFrameFormat>& rpDelFormat)
{
    if (rpDelFormat->GetDoc()->IsInDtor())
        SwPageDesc::DelHFFormat(rpToDel.get(), rpDelFormat.get());

    rpToDel.release();
    rpDelFormat.release();
}

void SwStashedHFFormatHelper::CopyHFContent_Impl(SwDoc& rDoc, const SwFormatContent& rSrc,
                                                 SwFrameFormat* pTarget, bool bHeader)
{
    // Code taken from SwDoc::CopyMasterHeader()
    SwNodeIndex aTmp(rDoc.GetNodes().GetEndOfAutotext());
    SwStartNode* pSttNd
        = SwNodes::MakeEmptySection(aTmp, bHeader ? SwHeaderStartNode : SwFooterStartNode);
    SwNodeRange aRange(rSrc.GetContentIdx()->GetNode(), SwNodeOffset(0),
                       *rSrc.GetContentIdx()->GetNode().EndOfSectionNode());
    aTmp = *pSttNd->EndOfSectionNode();
    rDoc.GetNodes().Copy_(aRange, aTmp, false);
    aTmp = *pSttNd;
    rDoc.GetDocumentContentOperationsManager().CopyFlyInFlyImpl(aRange, nullptr, aTmp);
    SwPaM const source(aRange.aStart, aRange.aEnd);
    SwPosition dest(aTmp);
    sw::CopyBookmarks(source, dest);
    pTarget->SetFormatAttr(SwFormatContent(pSttNd));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
