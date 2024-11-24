/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QtExpander.hxx>
#include <QtExpander.moc>

QtExpander::QtExpander(QWidget* pParent)
    : QWidget(pParent)
    , m_pContentWidget(nullptr)
    , m_bExpanded(false)
{
    m_pLayout = new QGridLayout;
    setLayout(m_pLayout);

    m_pButton = new QPushButton;
    m_pButton->setFlat(true);
    m_pButton->setSizePolicy(QSizePolicy::Policy::Maximum, QSizePolicy::Policy::Maximum);
    m_pLayout->addWidget(m_pButton, 0, 0);
    m_pLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Policy::MinimumExpanding,
                                       QSizePolicy::Policy::MinimumExpanding),
                       0, 1);

    update();

    connect(m_pButton, &QAbstractButton::clicked, this, &QtExpander::handleButtonClick);
}

void QtExpander::setContentWidget(QWidget* pWidget)
{
    assert(pWidget);
    m_pContentWidget = pWidget;
    m_pLayout->addWidget(m_pContentWidget, 1, 0, 1, 2);
    update();
}

void QtExpander::setText(const QString& rText) { m_pButton->setText(rText); }

QString QtExpander::text() const { return m_pButton->text(); }

void QtExpander::setExpanded(bool bExpand)
{
    if (m_bExpanded == bExpand)
        return;

    m_bExpanded = bExpand;
    update();

    emit expandedChanged(isExpanded());
}

bool QtExpander::isExpanded() const { return m_bExpanded; }

void QtExpander::update()
{
    const QString sIcon = m_bExpanded ? "go-down" : "go-next";
    m_pButton->setIcon(QIcon::fromTheme(sIcon));

    if (m_pContentWidget)
        m_pContentWidget->setVisible(m_bExpanded);
}

void QtExpander::handleButtonClick()
{
    // toggle
    setExpanded(!isExpanded());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
