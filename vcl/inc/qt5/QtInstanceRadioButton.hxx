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

#include <QtCore/QObject>
#include <QtWidgets/QRadioButton>

class QtInstanceRadioButton : public QtInstanceWidget, public virtual weld::RadioButton
{
    Q_OBJECT

    QRadioButton* m_pRadioButton;

public:
    QtInstanceRadioButton(QRadioButton* pRadioButton);

    // weld::Toggleable methods
    virtual void set_active(bool bActive) override;
    virtual bool get_active() const override;

    // weld::RadioButton methods
    virtual void set_label(const OUString& rText) override;
    virtual OUString get_label() const override;
    virtual void set_label_wrap(bool bWrap) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
