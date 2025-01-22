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

class QtInstanceBox : public QtInstanceContainer, public virtual weld::Box
{
    Q_OBJECT

public:
    QtInstanceBox(QWidget* pWidget);

    virtual void reorder_child(weld::Widget* pWidget, int nPosition) override;
    virtual void sort_native_button_order() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
