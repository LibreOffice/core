/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <QtWidgets/QGridLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

class QtExpander : public QWidget
{
    Q_OBJECT

    QPushButton* m_pButton;
    QGridLayout* m_pLayout;
    QWidget* m_pContentWidget;
    bool m_bExpanded;

public:
    QtExpander(QWidget* pParent);
    void setContentWidget(QWidget* pWidget);

    void setText(const QString& rText);
    QString text() const;

    void setExpanded(bool bExpand);
    bool isExpanded() const;

Q_SIGNALS:
    void expandedChanged(bool bExpanded);

private:
    void update();

private Q_SLOTS:
    void handleButtonClick();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
