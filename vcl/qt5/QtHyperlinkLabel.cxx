/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QtHyperlinkLabel.hxx>

QtHyperlinkLabel::QtHyperlinkLabel(QWidget* pParent)
    : QLabel(pParent)
{
}

void QtHyperlinkLabel::setDisplayText(const QString& rDisplayText)
{
    m_sDisplayText = rDisplayText;
    update();
};

void QtHyperlinkLabel::setUri(const QString& rUri)
{
    m_sUri = rUri;
    update();
};

void QtHyperlinkLabel::update()
{
    setText(QLatin1String("<a href=\"%1\">%2</a>").arg(m_sUri).arg(m_sDisplayText));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
