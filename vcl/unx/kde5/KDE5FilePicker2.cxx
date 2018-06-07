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
#include "FPServiceInfo.hxx"

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

KDE5FilePicker::KDE5FilePicker(const uno::Reference<uno::XComponentContext>&)
    : KDE5FilePicker_Base(_helperMutex)
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
    connect(this, &KDE5FilePicker::setTitleSignal /*(const OUString&)*/, this,
            &KDE5FilePicker::setTitleSlot /*(const OUString&)*/, Qt::BlockingQueuedConnection);
    connect(this, &KDE5FilePicker::setDisplayDirectorySignal /*(const OUString&)*/, this,
            &KDE5FilePicker::setDisplayDirectorySlot /*(const OUString&)*/,
            Qt::BlockingQueuedConnection);

    qApp->installEventFilter(this);
    setMultiSelectionMode(false);
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
    return _dialog->exec() == QFileDialog::Accepted;
}

void SAL_CALL KDE5FilePicker::setMultiSelectionMode(sal_Bool multiSelect) {}

void SAL_CALL KDE5FilePicker::setDefaultName(const OUString& name) {}

void SAL_CALL KDE5FilePicker::setDisplayDirectory(const OUString& dir)
{
    if (qApp->thread() != QThread::currentThread())
    {
        SolarMutexReleaser aReleaser;
        return Q_EMIT setDisplayDirectorySignal(dir);
    }

    _dialog->selectUrl(QUrl(toQString(dir)));
}

OUString SAL_CALL KDE5FilePicker::getDisplayDirectory()
{
    OUString dir;
    return dir;
}

uno::Sequence<OUString> SAL_CALL KDE5FilePicker::getFiles()
{
    uno::Sequence<OUString> seq = getSelectedFiles();
    if (seq.getLength() > 1)
        seq.realloc(1);
    return seq;
}

uno::Sequence<OUString> SAL_CALL KDE5FilePicker::getSelectedFiles()
{
    uno::Sequence<OUString> seq;
    return seq;
}

void SAL_CALL KDE5FilePicker::appendFilter(const OUString& title, const OUString& filter) {}

void SAL_CALL KDE5FilePicker::setCurrentFilter(const OUString& title) {}

OUString SAL_CALL KDE5FilePicker::getCurrentFilter()
{
    OUString filter;
    return filter;
}

void SAL_CALL KDE5FilePicker::appendFilterGroup(const OUString& /*rGroupTitle*/,
                                                const uno::Sequence<beans::StringPair>& filters)
{
    const sal_uInt16 length = filters.getLength();
    for (sal_uInt16 i = 0; i < length; ++i)
    {
        beans::StringPair aPair = filters[i];
        appendFilter(aPair.First, aPair.Second);
    }
}

void SAL_CALL KDE5FilePicker::setValue(sal_Int16 controlId, sal_Int16 nControlAction,
                                       const uno::Any& value)
{
}

uno::Any SAL_CALL KDE5FilePicker::getValue(sal_Int16 controlId, sal_Int16 nControlAction)
{
    if (CHECKBOX_AUTOEXTENSION == controlId)
        // We ignore this one and rely on QFileDialog to provide the function.
        // Always return false, to pretend we do not support this, otherwise
        // LO core would try to be smart and cut the extension in some places,
        // interfering with QFileDialog's handling of it. QFileDialog also
        // saves the value of the setting, so LO core is not needed for that either.
        return uno::Any(false);

    bool value = false;

    return uno::Any(value);
}

void SAL_CALL KDE5FilePicker::enableControl(sal_Int16 controlId, sal_Bool enable) {}

void SAL_CALL KDE5FilePicker::setLabel(sal_Int16 controlId, const OUString& label) {}

OUString SAL_CALL KDE5FilePicker::getLabel(sal_Int16 controlId)
{
    OUString label;
    return label;
}

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

    setTitle(VclResId(saveDialog ? STR_FPICKER_SAVE : STR_FPICKER_OPEN));
}

void SAL_CALL KDE5FilePicker::cancel()
{
    // TODO
}

void KDE5FilePicker::disposing(const lang::EventObject& rEvent)
{
    uno::Reference<XFilePickerListener> xFilePickerListener(rEvent.Source, uno::UNO_QUERY);

    if (xFilePickerListener.is())
    {
        removeFilePickerListener(xFilePickerListener);
    }
}

OUString SAL_CALL KDE5FilePicker::getImplementationName()
{
    return OUString(FILE_PICKER_IMPL_NAME);
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
