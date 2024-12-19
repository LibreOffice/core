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

#include <QtWidgets/QSlider>

class QtInstanceScale : public QtInstanceWidget, public virtual weld::Scale
{
    Q_OBJECT

    QSlider* m_pSlider;

public:
    QtInstanceScale(QSlider* pSlider);

    virtual void set_value(int nValue) override;
    virtual int get_value() const override;
    virtual void set_range(int nMin, int nMax) override;

    virtual void set_increments(int nStep, int nPage) override;
    virtual void get_increments(int& rStep, int& rPage) const override;

private Q_SLOTS:
    void handleValueChanged();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
