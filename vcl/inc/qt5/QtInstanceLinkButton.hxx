/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "QtHyperlinkLabel.hxx"
#include "QtInstanceWidget.hxx"

#include <QtWidgets/QLabel>

class QtInstanceLinkButton : public QtInstanceWidget, public virtual weld::LinkButton
{
    Q_OBJECT

    QtHyperlinkLabel* m_pLabel;

public:
    QtInstanceLinkButton(QtHyperlinkLabel* pLabel);

    virtual void set_label(const OUString& rText) override;
    virtual OUString get_label() const override;
    virtual void set_label_wrap(bool bWrap) override;
    virtual void set_uri(const OUString& rUri) override;
    virtual OUString get_uri() const override;

private Q_SLOTS:
    void linkActivated(const QString& rUrl);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
