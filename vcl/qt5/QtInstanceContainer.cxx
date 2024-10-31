/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QtInstanceContainer.hxx>

QtInstanceContainer::QtInstanceContainer(QWidget* pWidget)
    : QtInstanceWidget(pWidget)
{
    assert(pWidget->layout() && "no layout to use for container");
}

void QtInstanceContainer::move(weld::Widget*, weld::Container*)
{
    assert(false && "Not implemented yet");
}

css::uno::Reference<css::awt::XWindow> QtInstanceContainer::CreateChildFrame()
{
    assert(false && "Not implemented yet");
    return css::uno::Reference<css::awt::XWindow>();
}

void QtInstanceContainer::child_grab_focus() { assert(false && "Not implemented yet"); }

void QtInstanceContainer::connect_container_focus_changed(const Link<Container&, void>&)
{
    assert(false && "Not implemented yet");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
