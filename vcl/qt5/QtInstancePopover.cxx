/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QtInstancePopover.hxx>
#include <QtInstancePopover.moc>

QtInstancePopover::QtInstancePopover(QWidget* pWidget)
    : QtInstanceContainer(pWidget)
{
}

void QtInstancePopover::popup_at_rect(weld::Widget* pParent, const tools::Rectangle& rRect,
                                      weld::Placement ePlace)
{
    SolarMutexGuard g;

    assert(ePlace == weld::Placement::Under && "placement type not supported yet");
    (void)ePlace;

    GetQtInstance().RunInMainThread([&] {
        QWidget* pPopoverWidget = getQWidget();
        pPopoverWidget->adjustSize();
        QWidget* pParentWidget = QtInstance::GetNativeParentFromWeldParent(pParent);
        QPoint aPos = pParentWidget->mapToGlobal(toQPoint(rRect.BottomLeft()));
        aPos.setX(aPos.x() + rRect.GetWidth() - pPopoverWidget->width() / 2);

        pPopoverWidget->move(aPos);
        pPopoverWidget->show();
    });
}

void QtInstancePopover::popdown()
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] { getQWidget()->hide(); });
}

void QtInstancePopover::resize_to_request() { assert(false && "Not implemented yet"); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
