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

#include <vcl/weldutils.hxx>

class QtInstanceFormattedSpinButton : public QtInstanceEntry,
                                      public virtual weld::FormattedSpinButton
{
    Q_OBJECT

    QtDoubleSpinBox* m_pSpinBox;
    std::unique_ptr<weld::EntryFormatter> m_xOwnFormatter;
    weld::EntryFormatter* m_pFormatter;

public:
    QtInstanceFormattedSpinButton(QtDoubleSpinBox* pSpinBox);
    virtual ~QtInstanceFormattedSpinButton();

    virtual QWidget* getQWidget() const override;

    virtual Formatter& GetFormatter() override;
    virtual void SetFormatter(weld::EntryFormatter* pFormatter) override;

    virtual void sync_range_from_formatter() override;
    virtual void sync_value_from_formatter() override;
    virtual void sync_increments_from_formatter() override;

private Q_SLOTS:
    void handleValueChanged();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
