/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <QtWidgets/QLabel>

/**
 * QLabel subclass for a label that holds a hyperlink,
 * with convenient getters/setters for the displayed text
 * and the hyperlink target.
 */
class QtHyperlinkLabel : public QLabel
{
    Q_OBJECT

    QString m_sDisplayText;
    QString m_sUri;

public:
    QtHyperlinkLabel(QWidget* pParent);

    const QString& displayText() const { return m_sDisplayText; }
    void setDisplayText(const QString& rDisplayText);

    const QString& uri() const { return m_sUri; }
    void setUri(const QString& rUri);

private:
    void update();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
