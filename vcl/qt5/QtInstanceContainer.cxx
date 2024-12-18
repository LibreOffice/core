/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QtInstanceContainer.hxx>
#include <QtInstanceContainer.moc>

#include <QtInstanceWindow.hxx>

QtInstanceContainer::QtInstanceContainer(QWidget* pWidget)
    : QtInstanceWidget(pWidget)
{
    assert(pWidget->layout() && "no layout to use for container");
}

void QtInstanceContainer::move(weld::Widget* pWidget, weld::Container* pNewParent)
{
    QtInstanceWidget* pQtInstanceWidget = dynamic_cast<QtInstanceWidget*>(pWidget);
    assert(pQtInstanceWidget);
    QWidget* pQWidget = pQtInstanceWidget->getQWidget();
    assert(pQWidget);
    getLayout().removeWidget(pQWidget);

    if (!pNewParent)
    {
        pQWidget->deleteLater();
        return;
    }

    QtInstanceContainer* pNewContainer = dynamic_cast<QtInstanceContainer*>(pNewParent);
    assert(pNewContainer);
    QLayout& rNewLayout = pNewContainer->getLayout();
    rNewLayout.addWidget(pQWidget);
}

css::uno::Reference<css::awt::XWindow> QtInstanceContainer::CreateChildFrame()
{
    assert(false && "Not implemented yet");
    return css::uno::Reference<css::awt::XWindow>();
}

void QtInstanceContainer::child_grab_focus() { assert(false && "Not implemented yet"); }

void QtInstanceContainer::connect_container_focus_changed(const Link<Container&, void>& rLink)
{
    // for QtInstanceWindow, no special handling is needed, activate/deactivate events trigger the signal
    if (!qobject_cast<QtInstanceWindow*>(this))
        assert(false && "Not implemented yet");

    weld::Container::connect_container_focus_changed(rLink);
}

QLayout& QtInstanceContainer::getLayout() const
{
    QLayout* pLayout = getQWidget()->layout();
    assert(pLayout);
    return *pLayout;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
