/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "QtInstanceContainer.hxx"

#include <QtWidgets/QGridLayout>

class QtInstanceGrid : public QtInstanceContainer, public virtual weld::Grid
{
    Q_OBJECT

public:
    QtInstanceGrid(QWidget* pWidget);

    virtual void set_child_left_attach(weld::Widget& rWidget, int nAttach) override;
    virtual int get_child_left_attach(weld::Widget& rWidget) const override;
    virtual void set_child_column_span(weld::Widget& rWidget, int nCols) override;
    virtual void set_child_top_attach(weld::Widget& rWidget, int nAttach) override;
    virtual int get_child_top_attach(weld::Widget& rWidget) const override;

protected:
    virtual QGridLayout& getLayout() const override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
