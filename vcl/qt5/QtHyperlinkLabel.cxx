/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QtHyperlinkLabel.hxx>
#include <QtHyperlinkLabel.moc>

QtHyperlinkLabel::QtHyperlinkLabel(QWidget* pParent)
    : QLabel(pParent)
{
    setTextInteractionFlags(Qt::LinksAccessibleByMouse | Qt::LinksAccessibleByKeyboard);
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
    // Always set a non-empty URI, otherwise there's no clickable hyperlink
    // (custom slot for QLabel::linkActivated doesn't necessarily need a URI)
    const QString sUri = !m_sUri.isEmpty() ? m_sUri : QStringLiteral(" ");
    setText(QLatin1String("<a href=\"%1\">%2</a>").arg(sUri).arg(m_sDisplayText));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
