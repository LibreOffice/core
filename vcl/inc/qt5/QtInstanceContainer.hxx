/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "QtInstanceWidget.hxx"

#include <QtWidgets/QLayout>

class QtInstanceContainer : public QtInstanceWidget, public virtual weld::Container
{
    Q_OBJECT

public:
    QtInstanceContainer(QWidget* pWidget);

    virtual void move(weld::Widget* pWidget, weld::Container* pNewParent) override;

    virtual css::uno::Reference<css::awt::XWindow> CreateChildFrame() override;

    virtual void child_grab_focus() override;

    virtual void connect_container_focus_changed(const Link<Container&, void>& rLink) override;

protected:
    virtual QLayout& getLayout() const;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
