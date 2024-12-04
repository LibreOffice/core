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

#include <QtWidgets/QProgressBar>

class QtInstanceProgressBar : public QtInstanceWidget, public virtual weld::ProgressBar
{
    Q_OBJECT

    QProgressBar* m_pProgressBar;

public:
    QtInstanceProgressBar(QProgressBar* pProgressBar);

    virtual void set_percentage(int nValue) override;
    virtual OUString get_text() const override;
    virtual void set_text(const OUString& rText) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
