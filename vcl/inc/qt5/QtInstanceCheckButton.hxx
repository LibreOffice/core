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
#include <QtWidgets/QCheckBox>

class QtInstanceCheckButton : public QtInstanceWidget, public virtual weld::CheckButton
{
    Q_OBJECT

    QCheckBox* m_pCheckBox;

public:
    QtInstanceCheckButton(QCheckBox* pCheckBox);

    virtual void set_state(TriState eState) override;
    virtual TriState get_state() const override;

    virtual void set_label(const OUString& rText) override;
    virtual OUString get_label() const override;
    virtual void set_label_wrap(bool bWrap) override;

private Q_SLOTS:
    void handleToggled();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
