/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QtInstanceDrawingArea.hxx>
#include <QtInstanceDrawingArea.moc>

QtInstanceDrawingArea::QtInstanceDrawingArea(QLabel* pLabel)
    : QtInstanceWidget(pLabel)
    , m_pLabel(pLabel)
    , m_xDevice(DeviceFormat::WITHOUT_ALPHA)
{
    assert(m_pLabel);
}

void QtInstanceDrawingArea::queue_draw()
{
    SolarMutexGuard g;
    GetQtInstance().RunInMainThread([&] { getQWidget()->update(); });
}

void QtInstanceDrawingArea::queue_draw_area(int, int, int, int)
{
    assert(false && "Not implemented yet");
}

void QtInstanceDrawingArea::enable_drag_source(rtl::Reference<TransferDataContainer>&, sal_uInt8)
{
    assert(false && "Not implemented yet");
}

void QtInstanceDrawingArea::set_cursor(PointerStyle) { assert(false && "Not implemented yet"); }

Point QtInstanceDrawingArea::get_pointer_position() const
{
    assert(false && "Not implemented yet");
    return Point();
}

void QtInstanceDrawingArea::set_input_context(const InputContext&)
{
    assert(false && "Not implemented yet");
}

void QtInstanceDrawingArea::im_context_set_cursor_location(const tools::Rectangle&, int)
{
    assert(false && "Not implemented yet");
}

OutputDevice& QtInstanceDrawingArea::get_ref_device() { return *m_xDevice; }

a11yref QtInstanceDrawingArea::get_accessible_parent()
{
    assert(false && "Not implemented yet");
    return nullptr;
}

a11yrelationset QtInstanceDrawingArea::get_accessible_relation_set()
{
    assert(false && "Not implemented yet");
    return nullptr;
}

AbsoluteScreenPixelPoint QtInstanceDrawingArea::get_accessible_location_on_screen()
{
    assert(false && "Not implemented yet");
    return AbsoluteScreenPixelPoint(0, 0);
}

void QtInstanceDrawingArea::click(const Point&) { assert(false && "Not implemented yet"); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
