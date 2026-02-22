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

#include <vcl/weld/Paned.hxx>

#include <QtWidgets/QSplitter>

class QtInstancePaned : public QtInstanceWidget, public virtual weld::Paned
{
    Q_OBJECT

public:
    QtInstancePaned(QSplitter* pSplitter);

    virtual void set_position(int nPos) override;
    virtual int get_position() const override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
