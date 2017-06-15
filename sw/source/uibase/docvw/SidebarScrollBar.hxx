/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SW_SOURCE_UIBASE_DOCVW_SIDEBARSCROLLBAR_HXX
#define INCLUDED_SW_SOURCE_UIBASE_DOCVW_SIDEBARSCROLLBAR_HXX

#include <vcl/scrbar.hxx>

class SwView;

namespace sw
{
namespace annotation
{
class SwAnnotationWin;
}
}

namespace sw
{
namespace sidebarwindows
{

/// Similar to the VCL scrollbar, but instrumented with Writer-specific details for LOK.
class SidebarScrollBar : public ScrollBar
{
    sw::annotation::SwAnnotationWin& m_rSidebarWin;
    SwView& m_rView;

protected:
    /// @see OutputDevice::LogicInvalidate().
    void LogicInvalidate(const tools::Rectangle* pRectangle) override;
    void MouseMove(const MouseEvent& rMouseEvent) override;
    void MouseButtonUp(const MouseEvent& rMouseEvent) override;
public:
    SidebarScrollBar(sw::annotation::SwAnnotationWin& rSidebarWin, WinBits nStyle, SwView& rView);
    ~SidebarScrollBar() override;
};

}
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
