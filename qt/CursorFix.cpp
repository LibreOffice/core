/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "CursorFix.hpp"

#include <QCursor>
#include <QEvent>
#include <QGuiApplication>
#include <QLatin1String>
#include <QPixmap>
#include <QPoint>
#include <QPointer>
#include <QTimer>
#include <QWidget>

namespace
{
// QtWebEngine builds bitmap mouse cursors (the spreadsheet cell crosshair, custom CSS image
// cursors) in one of two layouts, depending on the Qt version: hotspot in physical pixels with
// the pixmap's device pixel ratio left at 1 (QTBUG-68571), or hotspot in logical pixels with the
// pixmap tagged with the display's ratio. The X11 plugin expects the first layout and the
// Wayland plugin the second. On a scaled display, a cursor in the wrong layout draws at double
// size or with the hotspot at half its position, away from where clicks land.
//
// This filter rebuilds each mismatched cursor into the layout the running platform expects. The
// rebuild runs from a zero-delay timer because QtWebEngine sets the cursor several times in one
// burst, and only the cursor applied last stays visible. The rebuilt cursor passes through this
// filter once more, but its pixmap ratio already matches the platform's layout, so the second
// pass returns at its first guard.
class BitmapCursorFixer : public QObject
{
public:
    enum class Mode
    {
        Wayland,
        X11
    };

    BitmapCursorFixer(Mode eMode, QObject* pParent)
        : QObject(pParent)
        , m_eMode(eMode)
    {
    }

protected:
    bool eventFilter(QObject* pWatched, QEvent* pEvent) override
    {
        if (pEvent->type() != QEvent::CursorChange)
            return false;
        QWidget* pWidget = qobject_cast<QWidget*>(pWatched);
        if (!pWidget || !pWidget->testAttribute(Qt::WA_SetCursor))
            return false;
        if (pWidget->cursor().shape() != Qt::BitmapCursor)
            return false;
        const QPointer<QWidget> pGuard(pWidget);
        const Mode eMode = m_eMode;
        QTimer::singleShot(0, this, [pGuard, eMode]() {
            QWidget* pWidget = pGuard.data();
            if (!pWidget || !pWidget->testAttribute(Qt::WA_SetCursor))
                return;
            const QCursor aCursor = pWidget->cursor();
            if (aCursor.shape() != Qt::BitmapCursor)
                return;
            QPixmap aPixmap = aCursor.pixmap();
            if (aPixmap.isNull())
                return;
            const QPoint aHotspot = aCursor.hotSpot();
            if (eMode == Mode::Wayland)
            {
                if (aPixmap.devicePixelRatio() != 1.0)
                    return;
                const qreal fRatio = pWidget->devicePixelRatio();
                if (fRatio <= 1.0)
                    return;
                aPixmap.setDevicePixelRatio(fRatio);
                pWidget->setCursor(QCursor(aPixmap, qRound(aHotspot.x() / fRatio),
                                           qRound(aHotspot.y() / fRatio)));
            }
            else
            {
                const qreal fRatio = aPixmap.devicePixelRatio();
                if (fRatio <= 1.0)
                    return;
                aPixmap.setDevicePixelRatio(1.0);
                pWidget->setCursor(QCursor(aPixmap, qRound(aHotspot.x() * fRatio),
                                           qRound(aHotspot.y() * fRatio)));
            }
        });
        return false;
    }

private:
    Mode m_eMode;
};
} // namespace

void installBitmapCursorFix()
{
    const QString aPlatform = QGuiApplication::platformName();
    BitmapCursorFixer::Mode eMode;
    if (aPlatform.startsWith(QLatin1String("wayland")))
        eMode = BitmapCursorFixer::Mode::Wayland;
    else if (aPlatform == QLatin1String("xcb"))
        eMode = BitmapCursorFixer::Mode::X11;
    else
        return;
    qApp->installEventFilter(new BitmapCursorFixer(eMode, qApp));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
