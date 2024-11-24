/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QtInstanceExpander.hxx>
#include <QtInstanceExpander.moc>

#include <vcl/qt/QtUtils.hxx>

QtInstanceExpander::QtInstanceExpander(QtExpander* pExpander)
    : QtInstanceWidget(pExpander)
    , m_pExpander(pExpander)
{
    assert(m_pExpander);

    connect(m_pExpander, &QtExpander::expandedChanged, this, [&] { signal_expanded(); });
}

void QtInstanceExpander::set_label(const OUString& rText)
{
    SolarMutexGuard g;
    GetQtInstance().RunInMainThread([&] { m_pExpander->setText(toQString(rText)); });
}

OUString QtInstanceExpander::get_label() const
{
    SolarMutexGuard g;
    OUString sLabel;
    GetQtInstance().RunInMainThread([&] { sLabel = toOUString(m_pExpander->text()); });
    return sLabel;
}

bool QtInstanceExpander::get_expanded() const
{
    SolarMutexGuard g;
    bool bExpanded = false;
    GetQtInstance().RunInMainThread([&] { bExpanded = m_pExpander->isExpanded(); });
    return bExpanded;
}

void QtInstanceExpander::set_expanded(bool bExpand)
{
    SolarMutexGuard g;
    GetQtInstance().RunInMainThread([&] { m_pExpander->setExpanded(bExpand); });
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
