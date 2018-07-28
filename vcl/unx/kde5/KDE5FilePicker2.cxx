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

#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <cppuhelper/interfacecontainer.h>
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/ui/dialogs/CommonFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/ControlActions.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>

#include <osl/mutex.hxx>
#include <sal/log.hxx>

#undef Region

#include <unx/geninst.h>
#include <qt5/Qt5Tools.hxx>

#include <QtCore/QDebug>
#include <QtCore/QThread>
#include <QtCore/QUrl>
#include <QtGui/QClipboard>
#include <QtGui/QWindow>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QWidget>
#include <QtWidgets/QApplication>
#include <KFileWidget>

#include <fpicker/strings.hrc>
#include <strings.hrc>

using namespace ::com::sun::star;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::ui::dialogs::TemplateDescription;
using namespace ::com::sun::star::ui::dialogs::ExtendedFilePickerElementIds;
using namespace ::com::sun::star::ui::dialogs::CommonFilePickerElementIds;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;

// helper functions

namespace
{
uno::Sequence<OUString> FilePicker_getSupportedServiceNames()
{
    uno::Sequence<OUString> aRet(3);
    aRet[0] = "com.sun.star.ui.dialogs.FilePicker";
    aRet[1] = "com.sun.star.ui.dialogs.SystemFilePicker";
    aRet[2] = "com.sun.star.ui.dialogs.KDE5FilePicker";
    return aRet;
}
}

// KDE5FilePicker

KDE5FilePicker::KDE5FilePicker(QFileDialog::FileMode eMode)
    : KDE5FilePicker_Base(_helperMutex)
    , _dialog(new QFileDialog(nullptr, {}, QDir::homePath()))
    , _extraControls(new QWidget)
    , _layout(new QGridLayout(_extraControls))
    , allowRemoteUrls(false)
    , mbIsFolderPicker(eMode == QFileDialog::Directory)
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

    _dialog->setFileMode(eMode);

    if (mbIsFolderPicker)
    {
        _dialog->setOption(QFileDialog::ShowDirsOnly, true);
        _dialog->setWindowTitle(toQString(VclResId(STR_FPICKER_FOLDER_DEFAULT_TITLE)));
    }

    connect(_dialog, &QFileDialog::filterSelected, this, &KDE5FilePicker::filterChanged);
    connect(_dialog, &QFileDialog::fileSelected, this, &KDE5FilePicker::selectionChanged);

    // XExecutableDialog
    connect(this, &KDE5FilePicker::setTitleSignal, this, &KDE5FilePicker::setTitleSlot,
            Qt::BlockingQueuedConnection);
    // XFilePicker
    connect(this, &KDE5FilePicker::setMultiSelectionSignal, this,
            &KDE5FilePicker::setMultiSelectionSlot, Qt::BlockingQueuedConnection);
    connect(this, &KDE5FilePicker::setDefaultNameSignal, this, &KDE5FilePicker::setDefaultNameSlot,
            Qt::BlockingQueuedConnection);
    connect(this, &KDE5FilePicker::setDisplayDirectorySignal, this,
            &KDE5FilePicker::setDisplayDirectorySlot, Qt::BlockingQueuedConnection);
    connect(this, &KDE5FilePicker::getDisplayDirectorySignal, this,
            &KDE5FilePicker::getDisplayDirectorySlot, Qt::BlockingQueuedConnection);
    // XFolderPicker
    connect(this, &KDE5FilePicker::getDirectorySignal, this, &KDE5FilePicker::getDirectorySlot,
            Qt::BlockingQueuedConnection);
    // XFilterManager
    connect(this, &KDE5FilePicker::appendFilterSignal, this, &KDE5FilePicker::appendFilterSlot,
            Qt::BlockingQueuedConnection);
    connect(this, &KDE5FilePicker::setCurrentFilterSignal, this,
            &KDE5FilePicker::setCurrentFilterSlot, Qt::BlockingQueuedConnection);
    connect(this, &KDE5FilePicker::getCurrentFilterSignal, this,
            &KDE5FilePicker::getCurrentFilterSlot, Qt::BlockingQueuedConnection);
    // XFilterGroupManager
    connect(this, &KDE5FilePicker::appendFilterGroupSignal, this,
            &KDE5FilePicker::appendFilterGroupSlot, Qt::BlockingQueuedConnection);
    // XFilePickerControlAccess
    connect(this, &KDE5FilePicker::setValueSignal, this, &KDE5FilePicker::setValueSlot,
            Qt::BlockingQueuedConnection);
    connect(this, &KDE5FilePicker::getValueSignal, this, &KDE5FilePicker::getValueSlot,
            Qt::BlockingQueuedConnection);
    connect(this, &KDE5FilePicker::setLabelSignal, this, &KDE5FilePicker::setLabelSlot,
            Qt::BlockingQueuedConnection);
    connect(this, &KDE5FilePicker::getLabelSignal, this, &KDE5FilePicker::getLabelSlot,
            Qt::BlockingQueuedConnection);
    connect(this, &KDE5FilePicker::enableControlSignal, this, &KDE5FilePicker::enableControlSlot,
            Qt::BlockingQueuedConnection);
    // XFilePicker2
    connect(this, &KDE5FilePicker::getSelectedFilesSignal, this,
            &KDE5FilePicker::getSelectedFilesSlot, Qt::BlockingQueuedConnection);

    qApp->installEventFilter(this);
}

KDE5FilePicker::~KDE5FilePicker()
{
    delete _extraControls;
    delete _dialog;
}

void SAL_CALL
KDE5FilePicker::addFilePickerListener(const uno::Reference<XFilePickerListener>& xListener)
{
    SolarMutexGuard aGuard;
    m_xListener = xListener;
}

void SAL_CALL KDE5FilePicker::removeFilePickerListener(const uno::Reference<XFilePickerListener>&)
{
    SolarMutexGuard aGuard;
    m_xListener.clear();
}

// XExecutableDialog
void SAL_CALL KDE5FilePicker::setTitle(const OUString& title)
{
    if (qApp->thread() != QThread::currentThread())
    {
        SolarMutexReleaser aReleaser;
        return Q_EMIT setTitleSignal(title);
    }

    _dialog->setWindowTitle(toQString(title));
}

sal_Int16 SAL_CALL KDE5FilePicker::execute()
{
    if (!_filters.isEmpty())
        _dialog->setNameFilters(_filters);
    if (!_currentFilter.isEmpty())
        _dialog->selectNameFilter(_currentFilter);

    _dialog->show();
    //block and wait for user input
    return _dialog->exec() == QFileDialog::Accepted ? 1 : 0;
}

// XFilePicker
void SAL_CALL KDE5FilePicker::setMultiSelectionMode(sal_Bool multiSelect)
{
    if (qApp->thread() != QThread::currentThread())
    {
        SolarMutexReleaser aReleaser;
        return Q_EMIT setMultiSelectionSignal(multiSelect);
    }

    if (mbIsFolderPicker)
        return;

    _dialog->setFileMode(multiSelect ? QFileDialog::ExistingFiles : QFileDialog::ExistingFile);
}

void SAL_CALL KDE5FilePicker::setDefaultName(const OUString& name)
{
    if (qApp->thread() != QThread::currentThread())
    {
        SolarMutexReleaser aReleaser;
        return Q_EMIT setDefaultNameSignal(name);
    }

    _dialog->selectFile(toQString(name));
}

void SAL_CALL KDE5FilePicker::setDisplayDirectory(const OUString& dir)
{
    if (qApp->thread() != QThread::currentThread())
    {
        SolarMutexReleaser aReleaser;
        return Q_EMIT setDisplayDirectorySignal(dir);
    }

    QString qDir(toQString(dir));
    _dialog->setDirectoryUrl(QUrl(qDir));
}

OUString SAL_CALL KDE5FilePicker::getDisplayDirectory()
{
    if (qApp->thread() != QThread::currentThread())
    {
        SolarMutexReleaser aReleaser;
        return Q_EMIT getDisplayDirectorySignal();
    }

    return implGetDirectory();
}

uno::Sequence<OUString> SAL_CALL KDE5FilePicker::getFiles()
{
    uno::Sequence<OUString> seq = getSelectedFiles();
    if (seq.getLength() > 1)
        seq.realloc(1);
    return seq;
}

// XFilePicker2
uno::Sequence<OUString> SAL_CALL KDE5FilePicker::getSelectedFiles()
{
    if (qApp->thread() != QThread::currentThread())
    {
        SolarMutexReleaser aReleaser;
        return Q_EMIT getSelectedFilesSignal();
    }

    QList<QUrl> aURLs = _dialog->selectedUrls();
    uno::Sequence<OUString> seq(aURLs.size());

    size_t i = 0;
    for (auto& aURL : aURLs)
    {
        seq[i++] = toOUString(aURL.toString());
    }

    return seq;
}

// XFilterManager
void SAL_CALL KDE5FilePicker::appendFilter(const OUString& title, const OUString& filter)
{
    if (qApp->thread() != QThread::currentThread())
    {
        SolarMutexReleaser aReleaser;
        return Q_EMIT appendFilterSignal(title, filter);
    }

    QString t(toQString(title));
    QString f(toQString(filter));
    // '/' need to be escaped else they are assumed to be mime types by kfiledialog
    //see the docs
    t.replace("/", "\\/");

    // libreoffice separates by filters by ';' qt dialogs by space
    f.replace(";", " ");

    // make sure "*.*" is not used as "all files"
    f.replace("*.*", "*");

    _filters << QStringLiteral("%1 (%2)").arg(t, f);
    _titleToFilters[t] = _filters.constLast();
}

void SAL_CALL KDE5FilePicker::setCurrentFilter(const OUString& title)
{
    if (qApp->thread() != QThread::currentThread())
    {
        SolarMutexReleaser aReleaser;
        return Q_EMIT setCurrentFilterSignal(title);
    }

    _currentFilter = _titleToFilters.value(toQString(title));
}

OUString SAL_CALL KDE5FilePicker::getCurrentFilter()
{
    if (qApp->thread() != QThread::currentThread())
    {
        SolarMutexReleaser aReleaser;
        return Q_EMIT getCurrentFilterSignal();
    }

    OUString filter = toOUString(_titleToFilters.key(_dialog->selectedNameFilter()));

    //default if not found
    if (filter.isEmpty())
        filter = "ODF Text Document (.odt)";

    return filter;
}

// XFilterGroupManager
void SAL_CALL KDE5FilePicker::appendFilterGroup(const OUString& rGroupTitle,
                                                const uno::Sequence<beans::StringPair>& filters)
{
    if (qApp->thread() != QThread::currentThread())
    {
        SolarMutexReleaser aReleaser;
        return Q_EMIT appendFilterGroupSignal(rGroupTitle, filters);
    }

    const sal_uInt16 length = filters.getLength();
    for (sal_uInt16 i = 0; i < length; ++i)
    {
        beans::StringPair aPair = filters[i];
        appendFilter(aPair.First, aPair.Second);
    }
}

// XFilePickerControlAccess
void SAL_CALL KDE5FilePicker::setValue(sal_Int16 controlId, sal_Int16 nControlAction,
                                       const uno::Any& value)
{
    if (qApp->thread() != QThread::currentThread())
    {
        SolarMutexReleaser aReleaser;
        return Q_EMIT setValueSignal(controlId, nControlAction, value);
    }

    if (_customWidgets.contains(controlId))
    {
        bool bChecked = false;
        value >>= bChecked;

        QCheckBox* cb = dynamic_cast<QCheckBox*>(_customWidgets.value(controlId));
        if (cb)
            cb->setChecked(bChecked);
    }
    else
        SAL_WARN("vcl.kde5", "set value on unknown control " << controlId);
}

uno::Any SAL_CALL KDE5FilePicker::getValue(sal_Int16 controlId, sal_Int16 nControlAction)
{
    if (qApp->thread() != QThread::currentThread())
    {
        SolarMutexReleaser aReleaser;
        return Q_EMIT getValueSignal(controlId, nControlAction);
    }

    if (CHECKBOX_AUTOEXTENSION == controlId)
        // We ignore this one and rely on QFileDialog to provide the function.
        // Always return false, to pretend we do not support this, otherwise
        // LO core would try to be smart and cut the extension in some places,
        // interfering with QFileDialog's handling of it. QFileDialog also
        // saves the value of the setting, so LO core is not needed for that either.
        return uno::Any(false);

    bool value = false;
    if (_customWidgets.contains(controlId))
    {
        QCheckBox* cb = dynamic_cast<QCheckBox*>(_customWidgets.value(controlId));
        if (cb)
            value = cb->isChecked();
    }
    else
        SAL_WARN("vcl.kde5", "get value on unknown control" << controlId);

    return uno::Any(value);
}

void SAL_CALL KDE5FilePicker::enableControl(sal_Int16 controlId, sal_Bool enable)
{
    if (qApp->thread() != QThread::currentThread())
    {
        SolarMutexReleaser aReleaser;
        return Q_EMIT enableControlSignal(controlId, enable);
    }

    if (_customWidgets.contains(controlId))
        _customWidgets.value(controlId)->setEnabled(enable);
    else
        SAL_WARN("vcl.kde5", "enable on unknown control" << controlId);
}

void SAL_CALL KDE5FilePicker::setLabel(sal_Int16 controlId, const OUString& label)
{
    if (qApp->thread() != QThread::currentThread())
    {
        SolarMutexReleaser aReleaser;
        return Q_EMIT setLabelSignal(controlId, label);
    }

    if (_customWidgets.contains(controlId))
    {
        QCheckBox* cb = dynamic_cast<QCheckBox*>(_customWidgets.value(controlId));
        if (cb)
            cb->setText(toQString(label));
    }
    else
        SAL_WARN("vcl.kde5", "set label on unknown control" << controlId);
}

OUString SAL_CALL KDE5FilePicker::getLabel(sal_Int16 controlId)
{
    if (qApp->thread() != QThread::currentThread())
    {
        SolarMutexReleaser aReleaser;
        return Q_EMIT getLabelSignal(controlId);
    }

    OUString label;
    if (_customWidgets.contains(controlId))
    {
        QCheckBox* cb = dynamic_cast<QCheckBox*>(_customWidgets.value(controlId));
        if (cb)
            label = toOUString(cb->text());
    }
    else
        SAL_WARN("vcl.kde5", "get label on unknown control" << controlId);

    return label;
}

// XFolderPicker
OUString SAL_CALL KDE5FilePicker::getDirectory()
{
    if (qApp->thread() != QThread::currentThread())
    {
        SolarMutexReleaser aReleaser;
        return Q_EMIT getDirectorySignal();
    }

    return implGetDirectory();
}

void SAL_CALL KDE5FilePicker::setDescription(const OUString&) {}

void KDE5FilePicker::addCustomControl(sal_Int16 controlId)
{
    const char* resId = nullptr;

    switch (controlId)
    {
        case CHECKBOX_AUTOEXTENSION:
            resId = STR_FPICKER_AUTO_EXTENSION;
            break;
        case CHECKBOX_PASSWORD:
            resId = STR_FPICKER_PASSWORD;
            break;
        case CHECKBOX_FILTEROPTIONS:
            resId = STR_FPICKER_FILTER_OPTIONS;
            break;
        case CHECKBOX_READONLY:
            resId = STR_FPICKER_READONLY;
            break;
        case CHECKBOX_LINK:
            resId = STR_FPICKER_INSERT_AS_LINK;
            break;
        case CHECKBOX_PREVIEW:
            resId = STR_FPICKER_SHOW_PREVIEW;
            break;
        case CHECKBOX_SELECTION:
            resId = STR_FPICKER_SELECTION;
            break;
        case CHECKBOX_GPGENCRYPTION:
            resId = STR_FPICKER_GPGENCRYPT;
            break;
        case PUSHBUTTON_PLAY:
            resId = STR_FPICKER_PLAY;
            break;
        case LISTBOX_VERSION:
            resId = STR_FPICKER_VERSION;
            break;
        case LISTBOX_TEMPLATE:
            resId = STR_FPICKER_TEMPLATES;
            break;
        case LISTBOX_IMAGE_TEMPLATE:
            resId = STR_FPICKER_IMAGE_TEMPLATE;
            break;
        case LISTBOX_IMAGE_ANCHOR:
            resId = STR_FPICKER_IMAGE_ANCHOR;
            break;
        case LISTBOX_VERSION_LABEL:
        case LISTBOX_TEMPLATE_LABEL:
        case LISTBOX_IMAGE_TEMPLATE_LABEL:
        case LISTBOX_IMAGE_ANCHOR_LABEL:
        case LISTBOX_FILTER_SELECTOR:
            break;
    }

    switch (controlId)
    {
        case CHECKBOX_AUTOEXTENSION:
        case CHECKBOX_PASSWORD:
        case CHECKBOX_FILTEROPTIONS:
        case CHECKBOX_READONLY:
        case CHECKBOX_LINK:
        case CHECKBOX_PREVIEW:
        case CHECKBOX_SELECTION:
        case CHECKBOX_GPGENCRYPTION:
        {
            // the checkbox is created even for CHECKBOX_AUTOEXTENSION to simplify
            // code, but the checkbox is hidden and ignored
            bool hidden = controlId == CHECKBOX_AUTOEXTENSION;
            auto resString = toQString(VclResId(resId));
            resString.replace('~', '&');

            auto widget = new QCheckBox(resString, _extraControls);
            widget->setHidden(hidden);
            if (!hidden)
            {
                _layout->addWidget(widget);
            }
            _customWidgets.insert(controlId, widget);

            break;
        }
        case PUSHBUTTON_PLAY:
        case LISTBOX_VERSION:
        case LISTBOX_TEMPLATE:
        case LISTBOX_IMAGE_TEMPLATE:
        case LISTBOX_IMAGE_ANCHOR:
        case LISTBOX_VERSION_LABEL:
        case LISTBOX_TEMPLATE_LABEL:
        case LISTBOX_IMAGE_TEMPLATE_LABEL:
        case LISTBOX_IMAGE_ANCHOR_LABEL:
        case LISTBOX_FILTER_SELECTOR:
            break;
    }
}

OUString KDE5FilePicker::implGetDirectory()
{
    OUString dir = toOUString(_dialog->directoryUrl().url());
    return dir;
}

// XInitialization
void SAL_CALL KDE5FilePicker::initialize(const uno::Sequence<uno::Any>& args)
{
    // parameter checking
    uno::Any arg;
    if (args.getLength() == 0)
    {
        throw lang::IllegalArgumentException("no arguments", static_cast<XFilePicker2*>(this), 1);
    }

    arg = args[0];

    if ((arg.getValueType() != cppu::UnoType<sal_Int16>::get())
        && (arg.getValueType() != cppu::UnoType<sal_Int8>::get()))
    {
        throw lang::IllegalArgumentException("invalid argument type",
                                             static_cast<XFilePicker2*>(this), 1);
    }

    sal_Int16 templateId = -1;
    arg >>= templateId;

    bool saveDialog = false;
    switch (templateId)
    {
        case FILEOPEN_SIMPLE:
            break;

        case FILESAVE_SIMPLE:
            saveDialog = true;
            break;

        case FILESAVE_AUTOEXTENSION:
            saveDialog = true;
            addCustomControl(CHECKBOX_AUTOEXTENSION);
            break;

        case FILESAVE_AUTOEXTENSION_PASSWORD:
        {
            saveDialog = true;
            addCustomControl(CHECKBOX_PASSWORD);
            addCustomControl(CHECKBOX_GPGENCRYPTION);
            break;
        }
        case FILESAVE_AUTOEXTENSION_PASSWORD_FILTEROPTIONS:
        {
            saveDialog = true;
            addCustomControl(CHECKBOX_AUTOEXTENSION);
            addCustomControl(CHECKBOX_PASSWORD);
            addCustomControl(CHECKBOX_GPGENCRYPTION);
            addCustomControl(CHECKBOX_FILTEROPTIONS);
            break;
        }
        case FILESAVE_AUTOEXTENSION_SELECTION:
            saveDialog = true;
            addCustomControl(CHECKBOX_AUTOEXTENSION);
            addCustomControl(CHECKBOX_SELECTION);
            break;

        case FILESAVE_AUTOEXTENSION_TEMPLATE:
            saveDialog = true;
            addCustomControl(CHECKBOX_AUTOEXTENSION);
            addCustomControl(LISTBOX_TEMPLATE);
            break;

        case FILEOPEN_LINK_PREVIEW_IMAGE_TEMPLATE:
            addCustomControl(CHECKBOX_LINK);
            addCustomControl(CHECKBOX_PREVIEW);
            addCustomControl(LISTBOX_IMAGE_TEMPLATE);
            break;

        case FILEOPEN_LINK_PREVIEW_IMAGE_ANCHOR:
            addCustomControl(CHECKBOX_LINK);
            addCustomControl(CHECKBOX_PREVIEW);
            addCustomControl(LISTBOX_IMAGE_ANCHOR);
            break;

        case FILEOPEN_PLAY:
            addCustomControl(PUSHBUTTON_PLAY);
            break;

        case FILEOPEN_LINK_PLAY:
            addCustomControl(CHECKBOX_LINK);
            addCustomControl(PUSHBUTTON_PLAY);
            break;

        case FILEOPEN_READONLY_VERSION:
            addCustomControl(CHECKBOX_READONLY);
            addCustomControl(LISTBOX_VERSION);
            break;

        case FILEOPEN_LINK_PREVIEW:
            addCustomControl(CHECKBOX_LINK);
            addCustomControl(CHECKBOX_PREVIEW);
            break;

        case FILEOPEN_PREVIEW:
            addCustomControl(CHECKBOX_PREVIEW);
            break;

        default:
            OSL_TRACE("Unknown templates %d", templateId);
            return;
    }

    //default is opening
    QFileDialog::AcceptMode operationMode
        = saveDialog ? QFileDialog::AcceptSave : QFileDialog::AcceptOpen;

    _dialog->setAcceptMode(operationMode);

    if (saveDialog)
    {
        _dialog->setConfirmOverwrite(true);
        _dialog->setFileMode(QFileDialog::AnyFile);
    }

    setTitle(VclResId(saveDialog ? STR_FPICKER_SAVE : STR_FPICKER_OPEN));
}

// XCancellable
void SAL_CALL KDE5FilePicker::cancel()
{
    // TODO
}

// XEventListener
void KDE5FilePicker::disposing(const lang::EventObject& rEvent)
{
    uno::Reference<XFilePickerListener> xFilePickerListener(rEvent.Source, uno::UNO_QUERY);

    if (xFilePickerListener.is())
    {
        removeFilePickerListener(xFilePickerListener);
    }
}

// XServiceInfo
OUString SAL_CALL KDE5FilePicker::getImplementationName()
{
    return OUString("com.sun.star.ui.dialogs.KDE5FilePicker");
}

sal_Bool SAL_CALL KDE5FilePicker::supportsService(const OUString& ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

uno::Sequence<OUString> SAL_CALL KDE5FilePicker::getSupportedServiceNames()
{
    return FilePicker_getSupportedServiceNames();
}

void KDE5FilePicker::filterChanged()
{
    FilePickerEvent aEvent;
    aEvent.ElementId = LISTBOX_FILTER;
    OSL_TRACE("filter changed");
    if (m_xListener.is())
        m_xListener->controlStateChanged(aEvent);
}

void KDE5FilePicker::selectionChanged()
{
    FilePickerEvent aEvent;
    OSL_TRACE("file selection changed");
    if (m_xListener.is())
        m_xListener->fileSelectionChanged(aEvent);
}

bool KDE5FilePicker::eventFilter(QObject* o, QEvent* e)
{
    if (e->type() == QEvent::Show && o->isWidgetType())
    {
        auto* w = static_cast<QWidget*>(o);
        if (!w->parentWidget() && w->isModal())
        {
            if (auto* fileWidget = w->findChild<KFileWidget*>({}, Qt::FindDirectChildrenOnly))
                fileWidget->setCustomWidget(_extraControls);
        }
    }
    return QObject::eventFilter(o, e);
}

#include <KDE5FilePicker.moc>

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
