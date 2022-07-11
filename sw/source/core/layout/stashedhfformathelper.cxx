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
    {
        m_pStashedLeftHeader
            = std::make_unique<SwFormatHeader>(SwFormatHeader(*r.m_pStashedLeftHeader.get()));
        m_pStashedLeftHeaderFormat
            = std::make_unique<SwFrameFormat>(SwFrameFormat(*r.m_pStashedLeftHeaderFormat.get()));

        m_pStashedLeftHeader->RegisterToFormat(*m_pStashedLeftHeaderFormat.get());
    }

    if (r.m_pStashedFirstHeader && r.m_pStashedFirstHeaderFormat)
    {
        m_pStashedFirstHeader
            = std::make_unique<SwFormatHeader>(SwFormatHeader(*r.m_pStashedFirstHeader.get()));
        m_pStashedFirstHeaderFormat
            = std::make_unique<SwFrameFormat>(SwFrameFormat(*r.m_pStashedFirstHeaderFormat.get()));

        m_pStashedFirstHeader->RegisterToFormat(*m_pStashedFirstHeaderFormat.get());
    }

    if (r.m_pStashedFirstLeftHeader && r.m_pStashedFirstLeftHeaderFormat)
    {
        m_pStashedFirstLeftHeader
            = std::make_unique<SwFormatHeader>(SwFormatHeader(*r.m_pStashedFirstLeftHeader.get()));
        m_pStashedFirstLeftHeaderFormat = std::make_unique<SwFrameFormat>(
            SwFrameFormat(*r.m_pStashedFirstLeftHeaderFormat.get()));

        m_pStashedFirstLeftHeader->RegisterToFormat(*m_pStashedFirstLeftHeaderFormat.get());
    }

    if (r.m_pStashedLeftFooter && r.m_pStashedLeftFooterFormat)
    {
        m_pStashedLeftFooter
            = std::make_unique<SwFormatFooter>(SwFormatFooter(*r.m_pStashedLeftFooter.get()));
        m_pStashedLeftFooterFormat
            = std::make_unique<SwFrameFormat>(SwFrameFormat(*r.m_pStashedLeftFooterFormat.get()));

        m_pStashedLeftFooter->RegisterToFormat(*m_pStashedLeftFooterFormat.get());
    }

    if (r.m_pStashedFirstFooter && r.m_pStashedFirstFooterFormat)
    {
        m_pStashedFirstFooter
            = std::make_unique<SwFormatFooter>(SwFormatFooter(*r.m_pStashedFirstFooter.get()));
        m_pStashedLeftFooterFormat
            = std::make_unique<SwFrameFormat>(SwFrameFormat(*r.m_pStashedLeftFooterFormat.get()));

        m_pStashedFirstFooter->RegisterToFormat(*m_pStashedFirstFooterFormat.get());
    }

    if (r.m_pStashedFirstLeftFooter && r.m_pStashedFirstLeftFooterFormat)
    {
        m_pStashedFirstLeftFooter
            = std::make_unique<SwFormatFooter>(SwFormatFooter(*r.m_pStashedFirstLeftFooter.get()));
        m_pStashedFirstLeftFooterFormat = std::make_unique<SwFrameFormat>(
            SwFrameFormat(*r.m_pStashedFirstLeftFooterFormat.get()));

        m_pStashedFirstLeftFooter->RegisterToFormat(*m_pStashedFirstLeftFooterFormat);
    }
}

SwStashedHFFormatHelper::~SwStashedHFFormatHelper()
{
    if (m_pStashedLeftHeaderFormat && m_pStashedLeftHeader)
    {
        if (m_pStashedLeftHeaderFormat->GetDoc()->IsInDtor())
            SwPageDesc::DelHFFormat(m_pStashedLeftHeader.get(), m_pStashedLeftHeaderFormat.get());
        m_pStashedLeftHeader.release();
        m_pStashedLeftHeaderFormat.release();
    }

    if (m_pStashedFirstHeaderFormat && m_pStashedFirstHeader)
    {
        if (m_pStashedFirstHeaderFormat->GetDoc()->IsInDtor())
            SwPageDesc::DelHFFormat(m_pStashedFirstHeader.get(), m_pStashedFirstHeaderFormat.get());
        m_pStashedFirstHeader.release();
        m_pStashedFirstHeaderFormat.release();
    }

    if (m_pStashedFirstLeftHeaderFormat && m_pStashedFirstLeftHeader)
    {
        if (m_pStashedFirstLeftHeaderFormat->GetDoc()->IsInDtor())
            SwPageDesc::DelHFFormat(m_pStashedFirstLeftHeader.get(),
                                    m_pStashedFirstLeftHeaderFormat.get());
        m_pStashedFirstLeftHeader.release();
        m_pStashedFirstLeftHeaderFormat.release();
    }

    if (m_pStashedLeftFooterFormat && m_pStashedLeftFooter)
    {
        if (m_pStashedLeftFooterFormat->GetDoc()->IsInDtor())
            SwPageDesc::DelHFFormat(m_pStashedLeftFooter.get(), m_pStashedLeftFooterFormat.get());
        m_pStashedLeftFooter.release();
        m_pStashedLeftFooterFormat.release();
    }

    if (m_pStashedFirstFooterFormat && m_pStashedFirstFooter)
    {
        if (m_pStashedFirstFooterFormat->GetDoc()->IsInDtor())
            SwPageDesc::DelHFFormat(m_pStashedFirstFooter.get(), m_pStashedFirstFooterFormat.get());
        m_pStashedFirstFooter.release();
        m_pStashedFirstFooterFormat.release();
    }

    if (m_pStashedFirstLeftFooterFormat && m_pStashedFirstLeftFooter)
    {
        if (m_pStashedFirstLeftFooterFormat->GetDoc()->IsInDtor())
            SwPageDesc::DelHFFormat(m_pStashedFirstLeftFooter.get(),
                                    m_pStashedFirstLeftFooterFormat.get());
        m_pStashedFirstLeftFooter.release();
        m_pStashedFirstLeftFooterFormat.release();
    }
}

void SwStashedHFFormatHelper::StashHeader(const SwFormatHeader* pHeader, bool bLeft, bool bFirst)
{
    assert(pHeader && pHeader->GetRegisteredIn());

    if (bLeft && !bFirst)
    {
        m_pStashedLeftHeader = std::make_unique<SwFormatHeader>(SwFormatHeader(*pHeader));
        m_pStashedLeftHeaderFormat
            = std::make_unique<SwFrameFormat>(SwFrameFormat(*pHeader->GetHeaderFormat()));

        m_pStashedLeftHeader->RegisterToFormat(*m_pStashedLeftHeaderFormat.get());
    }
    else if (!bLeft && bFirst)
    {
        m_pStashedFirstHeader = std::make_unique<SwFormatHeader>(SwFormatHeader(*pHeader));
        m_pStashedFirstHeaderFormat
            = std::make_unique<SwFrameFormat>(SwFrameFormat(*pHeader->GetHeaderFormat()));

        m_pStashedFirstHeader->RegisterToFormat(*m_pStashedFirstHeaderFormat.get());
    }
    else if (bLeft && bFirst)
    {
        m_pStashedFirstLeftHeader = std::make_unique<SwFormatHeader>(SwFormatHeader(*pHeader));
        m_pStashedFirstLeftHeaderFormat
            = std::make_unique<SwFrameFormat>(SwFrameFormat(*pHeader->GetHeaderFormat()));

        m_pStashedFirstLeftHeader->RegisterToFormat(*m_pStashedFirstLeftHeaderFormat.get());
    }

    else
        SAL_WARN("sw",
                 "SwStashedHFFormatHelper::StashHeaderFormat: Stashing the right page header is "
                 "pointless.");
}

void SwStashedHFFormatHelper::StashFooter(const SwFormatFooter* pFooter, bool bLeft, bool bFirst)
{
    assert(pFooter && pFooter->GetRegisteredIn());

    if (bLeft && !bFirst)
    {
        m_pStashedLeftFooter = std::make_unique<SwFormatFooter>(SwFormatFooter(*pFooter));
        m_pStashedLeftFooterFormat
            = std::make_unique<SwFrameFormat>(SwFrameFormat(*pFooter->GetFooterFormat()));

        m_pStashedLeftFooter->RegisterToFormat(*m_pStashedLeftFooterFormat.get());
    }
    else if (!bLeft && bFirst)
    {
        m_pStashedFirstFooter = std::make_unique<SwFormatFooter>(SwFormatFooter(*pFooter));
        m_pStashedFirstFooterFormat
            = std::make_unique<SwFrameFormat>(SwFrameFormat(*pFooter->GetFooterFormat()));

        m_pStashedFirstFooter->RegisterToFormat(*m_pStashedFirstFooterFormat.get());
    }
    else if (bLeft && bFirst)
    {
        m_pStashedFirstLeftFooter = std::make_unique<SwFormatFooter>(SwFormatFooter(*pFooter));
        m_pStashedFirstLeftFooterFormat
            = std::make_unique<SwFrameFormat>(SwFrameFormat(*pFooter->GetFooterFormat()));

        m_pStashedFirstLeftFooter->RegisterToFormat(*m_pStashedFirstLeftFooterFormat.get());
    }

    else
        SAL_WARN("sw",
                 "SwStashedHFFormatHelper::StashFooterFormat: Stashing the right page footer is "
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
        if (bLeft && !bFirst && m_pStashedLeftHeaderFormat && m_pStashedFirstLeftHeader)
        {
            if (m_pStashedLeftHeaderFormat->GetDoc()->IsInDtor())
                SwPageDesc::DelHFFormat(m_pStashedLeftHeader.get(),
                                        m_pStashedLeftHeaderFormat.get());
            m_pStashedLeftHeader.release();
            m_pStashedLeftHeaderFormat.release();
        }
        else if (!bLeft && bFirst && m_pStashedFirstHeaderFormat && m_pStashedFirstHeader)
        {
            if (m_pStashedFirstHeaderFormat->GetDoc()->IsInDtor())
                SwPageDesc::DelHFFormat(m_pStashedFirstHeader.get(),
                                        m_pStashedFirstHeaderFormat.get());
            m_pStashedFirstHeader.release();
            m_pStashedFirstHeaderFormat.release();
        }
        else if (bLeft && bFirst && m_pStashedFirstLeftHeaderFormat && m_pStashedFirstLeftHeader)
        {
            if (m_pStashedFirstLeftHeaderFormat->GetDoc()->IsInDtor())
                SwPageDesc::DelHFFormat(m_pStashedFirstLeftHeader.get(),
                                        m_pStashedFirstLeftHeaderFormat.get());
            m_pStashedFirstLeftHeader.release();
            m_pStashedFirstLeftHeaderFormat.release();
        }
        else
        {
            SAL_WARN("sw", "SwStashedHFFormatHelper::RemoveStashedFormat: Right header is "
                           "never stashed.");
        }
    }
    else
    {
        if (bLeft && !bFirst && m_pStashedLeftFooterFormat && m_pStashedLeftFooter)
        {
            if (m_pStashedLeftFooterFormat->GetDoc()->IsInDtor())
                SwPageDesc::DelHFFormat(m_pStashedLeftFooter.get(),
                                        m_pStashedLeftFooterFormat.get());
            m_pStashedLeftFooter.release();
            m_pStashedLeftFooterFormat.release();
        }
        else if (!bLeft && bFirst && m_pStashedFirstFooterFormat && m_pStashedFirstFooter)
        {
            if (m_pStashedFirstFooterFormat->GetDoc()->IsInDtor())
                SwPageDesc::DelHFFormat(m_pStashedFirstFooter.get(),
                                        m_pStashedFirstFooterFormat.get());
            m_pStashedFirstFooter.release();
            m_pStashedFirstFooterFormat.release();
        }
        else if (bLeft && bFirst && m_pStashedFirstLeftFooterFormat && m_pStashedFirstLeftFooter)
        {
            if (m_pStashedFirstLeftFooterFormat->GetDoc()->IsInDtor())
                SwPageDesc::DelHFFormat(m_pStashedFirstLeftFooter.get(),
                                        m_pStashedFirstLeftFooterFormat.get());
            m_pStashedFirstLeftFooter.release();
            m_pStashedFirstLeftFooterFormat.release();
        }
        else
        {
            SAL_WARN("sw", "SwStashedHFFormatHelper::RemoveStashedFormat: Right footer is "
                           "never stashed.");
        }
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
