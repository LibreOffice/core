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

#include <QtWidgets/QScrollBar>

class QtInstanceScrollbar : public QtInstanceWidget, public virtual weld::Scrollbar
{
    Q_OBJECT

    QScrollBar* m_pScrollBar;

public:
    QtInstanceScrollbar(QScrollBar* pScrollBar);

    virtual void adjustment_configure(int nValue, int nLower, int nUpper, int nStepIncrement,
                                      int nPageIncrement, int nPageSize) override;
    virtual int adjustment_get_value() const override;
    virtual void adjustment_set_value(int nValue) override;
    virtual int adjustment_get_upper() const override;
    virtual void adjustment_set_upper(int nUpper) override;
    virtual int adjustment_get_page_size() const override;
    virtual void adjustment_set_page_size(int nSize) override;
    virtual int adjustment_get_page_increment() const override;
    virtual void adjustment_set_page_increment(int nSize) override;
    virtual int adjustment_get_step_increment() const override;
    virtual void adjustment_set_step_increment(int nSize) override;
    virtual int adjustment_get_lower() const override;
    virtual void adjustment_set_lower(int nLower) override;

    virtual int get_scroll_thickness() const override;
    virtual void set_scroll_thickness(int nThickness) override;
    virtual void set_scroll_swap_arrows(bool bSwap) override;

    virtual ScrollType get_scroll_type() const override;

private Q_SLOTS:
    void signalValueChanged();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
