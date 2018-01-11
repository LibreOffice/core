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

#include "kde5_filepicker.hxx"

#include <KWindowSystem>
#include <KFileWidget>

#include <QtCore/QDebug>
#include <QtCore/QUrl>
#include <QtGui/QClipboard>
#include <QtGui/QWindow>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QWidget>
#include <QtWidgets/QApplication>

// The dialog should check whether LO also supports the protocol
// provided by KIO, and KFileWidget::dirOperator() is only 4.3+ .
// Moreover it's only in this somewhat internal KFileWidget class,
// which may not necessarily be what QFileDialog::fileWidget() returns,
// but that's hopefully not a problem in practice.
//#if Qt_VERSION_MAJOR == 4 && Qt_VERSION_MINOR >= 2
//#define ALLOW_REMOTE_URLS 1
//#else
#define ALLOW_REMOTE_URLS 0
//#endif

// KDE5FilePicker

KDE5FilePicker::KDE5FilePicker(QObject* parent)
    : QObject(parent)
    , _dialog(new QFileDialog(nullptr, QString(""), QString("~")))
    , _extraControls(new QWidget)
    , _layout(new QFormLayout(_extraControls))
    , _winId(0)
    , allowRemoteUrls(false)
{
#if ALLOW_REMOTE_URLS
    if (KFileWidget* fileWidget = dynamic_cast<KFileWidget*>(_dialog->fileWidget()))
    {
        allowRemoteUrls = true;
        // Use finishedLoading signal rather than e.g. urlEntered, because if there's a problem
        // such as the URL being mistyped, there's no way to prevent two message boxes about it,
        // one from us and one from Qt code.
        connect(fileWidget->dirOperator(), SIGNAL(finishedLoading()), SLOT(checkProtocol()));
    }
#endif

    setMultiSelectionMode(false);

    connect(_dialog, &QFileDialog::filterSelected, this, &KDE5FilePicker::filterChanged);
    connect(_dialog, &QFileDialog::fileSelected, this, &KDE5FilePicker::selectionChanged);

    qApp->installEventFilter(this);
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

void SAL_CALL KDE5FilePicker::setTitle(const QString& title) { _dialog->setWindowTitle(title); }

bool SAL_CALL KDE5FilePicker::execute()
{
    if (!_filters.isEmpty())
        _dialog->setNameFilters(_filters);
    if (!_currentFilter.isEmpty())
        _dialog->selectNameFilter(_currentFilter);

    _dialog->show();
    //block and wait for user input
    return _dialog->exec() == QFileDialog::Accepted;
}

void SAL_CALL KDE5FilePicker::setMultiSelectionMode(bool multiSelect)
{
    _dialog->setFileMode(multiSelect ? QFileDialog::ExistingFiles : QFileDialog::ExistingFile);
}

void SAL_CALL KDE5FilePicker::setDefaultName(const QString& name)
{
    _dialog->selectUrl(QUrl(name));
}

void SAL_CALL KDE5FilePicker::setDisplayDirectory(const QString& dir)
{
    _dialog->selectUrl(QUrl(dir));
}

QString SAL_CALL KDE5FilePicker::getDisplayDirectory() const
{
    return _dialog->directoryUrl().url();
}

QList<QUrl> SAL_CALL KDE5FilePicker::getSelectedFiles() const { return _dialog->selectedUrls(); }

void SAL_CALL KDE5FilePicker::appendFilter(const QString& title, const QString& filter)
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

void SAL_CALL KDE5FilePicker::setCurrentFilter(const QString& title)
{
    _currentFilter = _titleToFilters.value(title);
}

QString SAL_CALL KDE5FilePicker::getCurrentFilter() const
{
    QString filter = _titleToFilters.key(_dialog->selectedNameFilter());

    //default if not found
    if (filter.isEmpty())
        filter = "ODF Text Document (.odt)";

    return filter;
}

void SAL_CALL KDE5FilePicker::setValue(sal_Int16 controlId, sal_Int16 /*nControlAction*/,
                                       bool value)
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

bool SAL_CALL KDE5FilePicker::getValue(sal_Int16 controlId, sal_Int16 /*nControlAction*/) const
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

void SAL_CALL KDE5FilePicker::enableControl(sal_Int16 controlId, sal_Bool enable)
{
    if (_customWidgets.contains(controlId))
        _customWidgets.value(controlId)->setEnabled(enable);
    else
        qWarning() << "enable on unknown control" << controlId;
}

void SAL_CALL KDE5FilePicker::setLabel(sal_Int16 controlId, const QString& label)
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

QString SAL_CALL KDE5FilePicker::getLabel(sal_Int16 controlId) const
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
    auto widget = new QCheckBox(_extraControls);
    widget->setHidden(hidden);
    if (!hidden)
    {
        auto resString = label;
        resString.replace('~', '&');
        _layout->addRow(resString, widget);
    }
    _customWidgets.insert(controlId, widget);
}

void SAL_CALL KDE5FilePicker::initialize(bool saveDialog)
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

void KDE5FilePicker::checkProtocol()
{
    // There's no libreoffice.desktop :(, so find a matching one.
    /*
    KService::List services = KServiceTypeTrader::self()->query( "Application", "Exec =~ 'libreoffice %U'" );
    QStringList protocols;
    if( !services.isEmpty())
        protocols = services[ 0 ]->property( "X-Qt-Protocols" ).toStringList();
    if( protocols.isEmpty()) // incorrect (developer?) installation ?
        protocols << "file" << "http";
    if( !protocols.contains( _dialog->baseUrl().protocol()) && !protocols.contains( "KIO" ))
        KMessageBox::error( _dialog, KIO::buildErrorString( KIO::ERR_UNSUPPORTED_PROTOCOL, _dialog->baseUrl().protocol()));
*/
}

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
            return false;
        }
    }
    return QObject::eventFilter(o, e);
}

#include "kde5_filepicker.moc"

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
