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

#include <vcl/svapp.hxx>

#include "kde5_filepicker.hxx"

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

KDE5FilePicker::KDE5FilePicker(QObject* parent)
    : QObject(parent)
    , _dialog(new QFileDialog(nullptr, {}, QDir::homePath()))
    , _extraControls(new QWidget)
    , _layout(new QGridLayout(_extraControls))
    , _winId(0)
    , allowRemoteUrls(false)
{
    _dialog->setSupportedSchemes({
        QStringLiteral("file"),
        QStringLiteral("ftp"),
        QStringLiteral("http"),
        QStringLiteral("https"),
        QStringLiteral("webdav"),
        QStringLiteral("webdavs"),
        QStringLiteral("smb"),
    });

    setMultiSelectionMode(false);

    connect(_dialog, &QFileDialog::filterSelected, this, &KDE5FilePicker::filterChanged);
    connect(_dialog, &QFileDialog::fileSelected, this, &KDE5FilePicker::selectionChanged);

    setupCustomWidgets();
}

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

void KDE5FilePicker::setTitle(const QString& title) { _dialog->setWindowTitle(title); }

bool KDE5FilePicker::execute()
{
    if (!_filters.isEmpty())
        _dialog->setNameFilters(_filters);
    if (!_currentFilter.isEmpty())
        _dialog->selectNameFilter(_currentFilter);

    _dialog->show();
    //block and wait for user input
    return _dialog->exec() == QFileDialog::Accepted;
}

void KDE5FilePicker::setMultiSelectionMode(bool multiSelect)
{
    if (_dialog->acceptMode() == QFileDialog::AcceptSave)
        return;

    _dialog->setFileMode(multiSelect ? QFileDialog::ExistingFiles : QFileDialog::ExistingFile);
}

void KDE5FilePicker::setDefaultName(const QString& name) { _dialog->selectFile(name); }

void KDE5FilePicker::setDisplayDirectory(const QString& dir)
{
    _dialog->setDirectoryUrl(QUrl(dir));
}

QString KDE5FilePicker::getDisplayDirectory() const { return _dialog->directoryUrl().url(); }

QList<QUrl> KDE5FilePicker::getSelectedFiles() const { return _dialog->selectedUrls(); }

void KDE5FilePicker::appendFilter(const QString& title, const QString& filter)
{
    QString t = title;
    QString f = filter;
    // '/' need to be escaped else they are assumed to be mime types by kfiledialog
    //see the docs
    t.replace("/", "\\/");

    // openoffice gives us filters separated by ';' qt dialogs just want space separated
    f.replace(";", " ");

    // make sure "*.*" is not used as "all files"
    f.replace("*.*", "*");

    _filters << QStringLiteral("%1 (%2)").arg(t, f);
    _titleToFilters[t] = _filters.constLast();
}

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

void KDE5FilePicker::setValue(sal_Int16 controlId, sal_Int16 /*nControlAction*/, bool value)
{
    if (_customWidgets.contains(controlId))
    {
        QCheckBox* cb = dynamic_cast<QCheckBox*>(_customWidgets.value(controlId));
        if (cb)
            cb->setChecked(value);
    }
    else
        qWarning() << "set value on unknown control" << controlId;
}

bool KDE5FilePicker::getValue(sal_Int16 controlId, sal_Int16 /*nControlAction*/) const
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
}

void KDE5FilePicker::addCheckBox(sal_Int16 controlId, const QString& label, bool hidden)
{
    auto resString = label;
    resString.replace('~', '&');

    auto widget = new QCheckBox(resString, _extraControls);
    widget->setHidden(hidden);
    if (!hidden)
    {
        _layout->addWidget(widget);
    }
    _customWidgets.insert(controlId, widget);
}

void KDE5FilePicker::initialize(bool saveDialog)
{
    //default is opening
    QFileDialog::AcceptMode operationMode
        = saveDialog ? QFileDialog::AcceptSave : QFileDialog::AcceptOpen;

    _dialog->setAcceptMode(operationMode);

    if (saveDialog)
    {
        _dialog->setConfirmOverwrite(true);
        _dialog->setFileMode(QFileDialog::AnyFile);
    }
}

void KDE5FilePicker::setWinId(sal_uIntPtr winId) { _winId = winId; }

void KDE5FilePicker::setupCustomWidgets()
{
    // When using the platform-native Plasma/KDE5 file picker, we currently rely on KFileWidget
    // being present to add the custom controls visible (s. 'eventFilter' method).
    // Since this doesn't work for other desktop environments, use a non-native
    // dialog there in order not to lose the custom controls and insert the custom
    // widget in the layout returned by QFileDialog::layout()
    // (which returns nullptr for native file dialogs)
    if (Application::GetDesktopEnvironment() == "KDE5")
    {
        qApp->installEventFilter(this);
    }
    else
    {
        _dialog->setOption(QFileDialog::DontUseNativeDialog);
        QGridLayout* pLayout = static_cast<QGridLayout*>(_dialog->layout());
        assert(pLayout);
        const int row = pLayout->rowCount();
        pLayout->addWidget(_extraControls, row, 1);
    }
}

bool KDE5FilePicker::eventFilter(QObject* o, QEvent* e)
{
    if (e->type() == QEvent::Show && o->isWidgetType())
    {
        auto* w = static_cast<QWidget*>(o);
        if (!w->parentWidget() && w->isModal())
        {
            KWindowSystem::setMainWindow(w, _winId);
            if (auto* fileWidget = w->findChild<KFileWidget*>({}, Qt::FindDirectChildrenOnly))
            {
                fileWidget->setCustomWidget(_extraControls);
                // remove event filter again; the only purpose was to set the custom widget here
                qApp->removeEventFilter(this);
            }
        }
    }
    return QObject::eventFilter(o, e);
}

#include <kde5_filepicker.moc>

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
