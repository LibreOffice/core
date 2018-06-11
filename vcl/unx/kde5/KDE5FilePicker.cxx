/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "KDE5FilePicker.hxx"

#include <KWindowSystem>
#include <KFileWidget>

#include <QtCore/QDebug>
#include <QtCore/QUrl>
#include <QtGui/QClipboard>
#include <QtGui/QWindow>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QWidget>
#include <QtWidgets/QApplication>

// KDE5FilePicker

void KDE5FilePicker::enableFolderMode()
{
    _dialog->setOption(QFileDialog::ShowDirsOnly, true);
    _dialog->setFileMode(QFileDialog::Directory);
}

KDE5FilePicker::~KDE5FilePicker()
{
    delete _extraControls;
    delete _dialog;
}

/*void KDE5FilePicker::setMultiSelectionMode(bool multiSelect)
{
    _dialog->setFileMode(multiSelect ? QFileDialog::ExistingFiles : QFileDialog::ExistingFile);
}

void KDE5FilePicker::setDefaultName(const QString& name) { _dialog->selectUrl(QUrl(name)); }

QString KDE5FilePicker::getDisplayDirectory() const { return _dialog->directoryUrl().url(); }

QList<QUrl> KDE5FilePicker::getSelectedFiles() const { return _dialog->selectedUrls(); }

void KDE5FilePicker::setCurrentFilter(const QString& title)
{
    _currentFilter = _titleToFilters.value(title);
}

QString KDE5FilePicker::getCurrentFilter() const
{
    QString filter = _titleToFilters.key(_dialog->selectedNameFilter());

    //default if not found
    if (filter.isEmpty())
        filter = "ODF Text Document (.odt)";

    return filter;
}

bool KDE5FilePicker::getValue(sal_Int16 controlId, sal_Int16 nControlAction) const
{
    bool ret = false;
    if (_customWidgets.contains(controlId))
    {
        QCheckBox* cb = dynamic_cast<QCheckBox*>(_customWidgets.value(controlId));
        if (cb)
            ret = cb->isChecked();
    }
    else
        qWarning() << "get value on unknown control" << controlId;

    return ret;
}

void KDE5FilePicker::enableControl(sal_Int16 controlId, bool enable)
{
    if (_customWidgets.contains(controlId))
        _customWidgets.value(controlId)->setEnabled(enable);
    else
        qWarning() << "enable on unknown control" << controlId;
}

void KDE5FilePicker::setLabel(sal_Int16 controlId, const QString& label)
{
    if (_customWidgets.contains(controlId))
    {
        QCheckBox* cb = dynamic_cast<QCheckBox*>(_customWidgets.value(controlId));
        if (cb)
            cb->setText(label);
    }
    else
        qWarning() << "set label on unknown control" << controlId;
}

QString KDE5FilePicker::getLabel(sal_Int16 controlId) const
{
    QString label;
    if (_customWidgets.contains(controlId))
    {
        QCheckBox* cb = dynamic_cast<QCheckBox*>(_customWidgets.value(controlId));
        if (cb)
            label = cb->text();
    }
    else
        qWarning() << "get label on unknown control" << controlId;

    return label;
}*/

void KDE5FilePicker::setWinId(sal_uIntPtr winId) { _winId = winId; }

bool KDE5FilePicker::eventFilter(QObject* o, QEvent* e)
{
    if (e->type() == QEvent::Show && o->isWidgetType())
    {
        auto* w = static_cast<QWidget*>(o);
        if (!w->parentWidget() && w->isModal())
        {
            KWindowSystem::setMainWindow(w, _winId);
            if (auto* fileWidget = w->findChild<KFileWidget*>({}, Qt::FindDirectChildrenOnly))
                fileWidget->setCustomWidget(_extraControls);
        }
    }
    return QObject::eventFilter(o, e);
}

#include <KDE5FilePicker.moc>

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
