/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/config.h>

#include "QtInstanceButton.hxx"

#include <vcl/weld/ToggleButton.hxx>

#include <QtWidgets/QToolButton>

class QtInstanceToggleButton : public QtInstanceButton, public virtual weld::ToggleButton
{
    Q_OBJECT

    QToolButton* m_pToolButton;

public:
    QtInstanceToggleButton(QToolButton* pButton);

    virtual void set_label(const OUString& rText) override;

    virtual void do_set_active(bool bActive) override;
    virtual bool get_active() const override;

    static void updateToolButtonStyle(QToolButton& rToolButton);

protected:
    QToolButton& getToolButton() const;

private Q_SLOTS:
    void signalToggled();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
