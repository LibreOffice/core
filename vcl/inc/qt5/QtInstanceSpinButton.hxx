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

#include <optional>

class QtInstanceSpinButton : public QtInstanceEntry, public virtual weld::SpinButton
{
    Q_OBJECT

    QtDoubleSpinBox* m_pSpinBox;

public:
    QtInstanceSpinButton(QtDoubleSpinBox* pSpinBox);

    virtual QWidget* getQWidget() const override;

    virtual void set_floating_point_value(double fValue) override;
    virtual double get_floating_point_value() const override;
    virtual void set_floating_point_range(double fMin, double fMax) override;
    virtual void get_floating_point_range(double& rMin, double& rMax) const override;

    virtual void set_floating_point_increments(double fStep, double fPage) override;
    virtual void get_floating_point_increments(double& rStep, double& rPage) const override;
    virtual void set_digits(unsigned int nDigits) override;
    virtual unsigned int get_digits() const override;

private:
    std::optional<double> convertTextToDouble(const QString& rText);
    std::optional<QString> formatValue(double fValue);

private Q_SLOTS:
    void handleValueChanged();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
