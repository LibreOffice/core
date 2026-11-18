/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#include <salframe.hxx>
#include <salvtables.hxx>

#include <o3tl/string_view.hxx>
#include <rtl/ustrbuf.hxx>
#include <vcl/weld/Window.hxx>

SalFrame::SalFrame()
    : m_pWindow(nullptr)
    , m_pProc(nullptr)
{
}

// this file contains the virtual destructors of the sal interface
// compilers usually put their vtables where the destructor is

SalFrame::~SalFrame() {}

void SalFrame::SetCallback(vcl::Window* pWindow, SALFRAMEPROC pProc)
{
    m_pWindow = pWindow;
    m_pProc = pProc;
}

// default to full-frame flushes
// on ports where partial-flushes are much cheaper this method should be overridden
void SalFrame::Flush(const tools::Rectangle&) { Flush(); }

void SalFrame::SetRepresentedURL(const OUString&)
{
    // currently this is Mac only functionality
}

OUString SalFrame::DumpSetPosSize(tools::Long nX, tools::Long nY, tools::Long nWidth,
                                  tools::Long nHeight, sal_uInt16 nFlags)
{
    // assuming the 4 integers normally don't have more than 4 digits, but might be negative
    OUStringBuffer aBuffer(4 * 5 + 5);
    if (nFlags & SAL_FRAME_POSSIZE_WIDTH)
        aBuffer << nWidth << "x";
    else
        aBuffer << "?x";
    if (nFlags & SAL_FRAME_POSSIZE_HEIGHT)
        aBuffer << nHeight << "@(";
    else
        aBuffer << "?@(";
    if (nFlags & SAL_FRAME_POSSIZE_X)
        aBuffer << nX << ",";
    else
        aBuffer << "?,";
    if (nFlags & SAL_FRAME_POSSIZE_Y)
        aBuffer << nY << ")";
    else
        aBuffer << "?)";
    return aBuffer.makeStringAndClear();
}

void SalFrame::SetExtendedFrameStyle(SalExtStyle) {}

SalFrameGeometry SalFrame::GetGeometry() const
{
    SalFrameGeometry aGeometry = GetUnmirroredGeometry();

    // mirror frame coordinates at parent
    SalFrame* pParent = GetParent();
    if (pParent && AllSettings::GetLayoutRTL())
    {
        SalFrameGeometry aParentGeometry = pParent->GetUnmirroredGeometry();
        const int nParentX = aGeometry.x() - aParentGeometry.x();
        aGeometry.setX(aParentGeometry.x() + aParentGeometry.width() - aGeometry.width()
                       - nParentX);
    }

    return aGeometry;
}

weld::Window* SalFrame::GetFrameWeld() const
{
    if (!m_xFrameWeld)
    {
        vcl::Window* pWindow = GetWindow();
        if (pWindow)
        {
            assert(pWindow == pWindow->GetFrameWindow());
            // resolve from a possible BorderWindow to the ClientWindow (returns itself if not)
            pWindow = pWindow->ImplGetWindow();
            m_xFrameWeld.reset(new SalInstanceWindow(pWindow, nullptr, false));
        }
    }
    return m_xFrameWeld.get();
}

void SalFrame::SimulateKeyPress(sal_uInt16) {}

Selection SalFrame::CalcDeleteSurroundingSelection(std::u16string_view rSurroundingText,
                                                   sal_Int32 nCursorIndex, int nOffset, int nChars)
{
    Selection aInvalid(SAL_MAX_UINT32, SAL_MAX_UINT32);

    if (nCursorIndex == -1)
        return aInvalid;

    if (nOffset > 0)
    {
        while (nOffset && nCursorIndex < static_cast<sal_Int32>(rSurroundingText.size()))
        {
            o3tl::iterateCodePoints(rSurroundingText, &nCursorIndex, 1);
            --nOffset;
        }
    }
    else if (nOffset < 0)
    {
        while (nOffset && nCursorIndex > 0)
        {
            o3tl::iterateCodePoints(rSurroundingText, &nCursorIndex, -1);
            ++nOffset;
        }
    }

    if (nOffset)
    {
        SAL_WARN("vcl",
                 "SalFrame::CalcDeleteSurroundingSelection, unable to move to offset: " << nOffset);
        return aInvalid;
    }

    sal_Int32 nCursorEndIndex(nCursorIndex);
    sal_Int32 nCount(0);
    while (nCount < nChars && nCursorEndIndex < static_cast<sal_Int32>(rSurroundingText.size()))
    {
        o3tl::iterateCodePoints(rSurroundingText, &nCursorEndIndex, 1);
        ++nCount;
    }

    if (nCount != nChars)
    {
        SAL_WARN("vcl", "SalFrame::CalcDeleteSurroundingSelection, unable to select: "
                            << nChars << " characters");
        return aInvalid;
    }

    return Selection(nCursorIndex, nCursorEndIndex);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
