/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QtInstanceDrawingArea.hxx>
#include <QtInstanceDrawingArea.moc>

#include <QtData.hxx>

#include <vcl/qt/QtUtils.hxx>

QtInstanceDrawingArea::QtInstanceDrawingArea(QLabel* pLabel)
    : QtInstanceWidget(pLabel)
    , m_pLabel(pLabel)
    , m_xDevice(DeviceFormat::WITHOUT_ALPHA)
{
    assert(m_pLabel);

    // install event filter, so eventFilter() can handle widget events
    m_pLabel->installEventFilter(this);
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

void QtInstanceDrawingArea::set_cursor(PointerStyle ePointerStyle)
{
    SolarMutexGuard g;
    GetQtInstance().RunInMainThread(
        [&] { getQWidget()->setCursor(GetQtData()->getCursor(ePointerStyle)); });
}

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

bool QtInstanceDrawingArea::eventFilter(QObject* pObject, QEvent* pEvent)
{
    if (pObject != m_pLabel)
        return false;

    SolarMutexGuard g;
    assert(GetQtInstance().IsMainThread());

    switch (pEvent->type())
    {
        case QEvent::Paint:
            handlePaintEvent();
            return false;
        case QEvent::Resize:
            handleResizeEvent();
            return false;
        default:
            return QtInstanceWidget::eventFilter(pObject, pEvent);
    }
}

void QtInstanceDrawingArea::handlePaintEvent()
{
    tools::Rectangle aRect(0, 0, m_pLabel->width(), m_pLabel->height());
    aRect = m_xDevice->PixelToLogic(aRect);
    m_xDevice->Erase(aRect);
    m_aDrawHdl.Call(std::pair<vcl::RenderContext&, const tools::Rectangle&>(*m_xDevice, aRect));
    QPixmap aPixmap = toQPixmap(*m_xDevice);

    // set new pixmap if it changed
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    if (aPixmap.toImage() != m_pLabel->pixmap().toImage())
#else
    if (aPixmap.toImage() != m_pLabel->pixmap(Qt::ReturnByValue).toImage())
#endif
        m_pLabel->setPixmap(aPixmap);
}

void QtInstanceDrawingArea::handleResizeEvent()
{
    const Size aSize = toSize(m_pLabel->size());
    m_xDevice->SetOutputSizePixel(aSize);
    m_aSizeAllocateHdl.Call(aSize);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
