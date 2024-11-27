/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "QtDoubleSpinBox.hxx"
#include "QtInstanceEntry.hxx"

#include <QtWidgets/QSpinBox>

class QtInstanceSpinButton : public QtInstanceEntry, public virtual weld::SpinButton
{
    Q_OBJECT

    QtDoubleSpinBox* m_pSpinBox;

public:
    QtInstanceSpinButton(QtDoubleSpinBox* pSpinBox);

    virtual QWidget* getQWidget() const override;

    virtual void set_value(sal_Int64 nValue) override;
    virtual sal_Int64 get_value() const override;
    virtual void set_range(sal_Int64 nMin, sal_Int64 nMax) override;
    virtual void get_range(sal_Int64& rMin, sal_Int64& rMax) const override;

    virtual void set_increments(sal_Int64 nStep, sal_Int64 nPage) override;
    virtual void get_increments(sal_Int64& rStep, sal_Int64& rPage) const override;
    virtual void set_digits(unsigned int nDigits) override;
    virtual unsigned int get_digits() const override;

private Q_SLOTS:
    void handleValueChanged();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
