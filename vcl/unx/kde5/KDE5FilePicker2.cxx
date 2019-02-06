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
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
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
    uno::Sequence<OUString> aRet(4);
    aRet[0] = "com.sun.star.ui.dialogs.FilePicker";
    aRet[1] = "com.sun.star.ui.dialogs.SystemFilePicker";
    aRet[2] = "com.sun.star.ui.dialogs.KDE5FilePicker";
    aRet[3] = "com.sun.star.ui.dialogs.KDE5FolderPicker";
    return aRet;
}
}

// KDE5FilePicker

KDE5FilePicker::KDE5FilePicker(QFileDialog::FileMode eMode)
    : Qt5FilePicker(eMode)
    , _extraControls(new QWidget)
    , _layout(new QGridLayout(_extraControls))
    , allowRemoteUrls(false)
{
    // use native dialog
    m_pFileDialog->setOption(QFileDialog::DontUseNativeDialog, false);

    m_pFileDialog->setSupportedSchemes({
        QStringLiteral("file"),
        QStringLiteral("ftp"),
        QStringLiteral("http"),
        QStringLiteral("https"),
        QStringLiteral("webdav"),
        QStringLiteral("webdavs"),
        QStringLiteral("smb"),
    });

    connect(this, &KDE5FilePicker::executeSignal, this, &KDE5FilePicker::execute,
            Qt::BlockingQueuedConnection);

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

    // used to set the custom controls
    qApp->installEventFilter(this);
}

KDE5FilePicker::~KDE5FilePicker() { delete _extraControls; }

sal_Int16 SAL_CALL KDE5FilePicker::execute()
{
    if (qApp->thread() != QThread::currentThread())
    {
        //SolarMutexReleaser aReleaser;
        return Q_EMIT executeSignal();
    }

    if (!_filters.isEmpty())
        m_pFileDialog->setNameFilters(_filters);
    if (!_currentFilter.isEmpty())
        m_pFileDialog->selectNameFilter(_currentFilter);

    m_pFileDialog->show();
    //block and wait for user input
    return m_pFileDialog->exec() == QFileDialog::Accepted ? 1 : 0;
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

    OUString filter = toOUString(_titleToFilters.key(m_pFileDialog->selectedNameFilter()));

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
    else if (_customListboxes.contains(controlId))
    {
        QComboBox* cb = dynamic_cast<QComboBox*>(_customListboxes.value(controlId));
        if (cb)
            handleSetListValue(cb, nControlAction, value);
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
    else if (_customListboxes.contains(controlId))
    {
        QComboBox* cb = dynamic_cast<QComboBox*>(_customListboxes.value(controlId));
        if (cb)
            return handleGetListValue(cb, nControlAction);
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
        SAL_WARN("vcl.kde5", "enable on unknown control " << controlId);
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
        SAL_WARN("vcl.kde5", "set label on unknown control " << controlId);
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
        SAL_WARN("vcl.kde5", "get label on unknown control " << controlId);

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

    auto resString = toQString(VclResId(resId));
    resString.replace('~', '&');

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
        case LISTBOX_VERSION_LABEL:
        case LISTBOX_TEMPLATE_LABEL:
        case LISTBOX_IMAGE_TEMPLATE_LABEL:
        case LISTBOX_IMAGE_ANCHOR_LABEL:
        case LISTBOX_FILTER_SELECTOR:
            break;

        case LISTBOX_IMAGE_ANCHOR:
        case LISTBOX_IMAGE_TEMPLATE:
        case LISTBOX_TEMPLATE:
        case LISTBOX_VERSION:
        {
            auto widget = new QComboBox(_extraControls);
            QLabel* label = new QLabel(resString);
            label->setBuddy(widget);

            QHBoxLayout* hBox = new QHBoxLayout;
            hBox->addWidget(label);
            hBox->addWidget(widget);
            _layout->addLayout(hBox, _layout->rowCount(), 0, Qt::AlignLeft);
            _customListboxes.insert(controlId, widget);
            break;
        }
    }
}

void KDE5FilePicker::handleSetListValue(QComboBox* pQComboBox, sal_Int16 nAction,
                                        const css::uno::Any& rValue)
{
    switch (nAction)
    {
        case ControlActions::ADD_ITEM:
        {
            OUString sItem;
            rValue >>= sItem;
            pQComboBox->addItem(toQString(sItem));
        }
        break;
        case ControlActions::ADD_ITEMS:
        {
            Sequence<OUString> aStringList;
            rValue >>= aStringList;
            sal_Int32 nItemCount = aStringList.getLength();
            for (sal_Int32 i = 0; i < nItemCount; ++i)
            {
                pQComboBox->addItem(toQString(aStringList[i]));
            }
        }
        break;
        case ControlActions::SET_SELECT_ITEM:
        {
            sal_Int32 nPos = 0;
            rValue >>= nPos;
            pQComboBox->setCurrentIndex(nPos);
        }
        break;
        default:
            SAL_WARN("vcl.kde5", "unknown action on list control " << nAction);
            break;
    }
}

uno::Any KDE5FilePicker::handleGetListValue(QComboBox* pQComboBox, sal_Int16 nAction)
{
    uno::Any aAny;
    switch (nAction)
    {
        case ControlActions::GET_ITEMS:
        {
            uno::Sequence<OUString> aItemList;

            for (int i = 0; i < pQComboBox->count(); ++i)
            {
                aItemList[i] = toOUString(pQComboBox->itemText(i));
            }
            aAny <<= aItemList;
        }
        break;
        case ControlActions::GET_SELECTED_ITEM:
        {
            OUString sItem = toOUString(pQComboBox->currentText());
            aAny <<= sItem;
        }
        break;
        case ControlActions::GET_SELECTED_ITEM_INDEX:
        {
            int nCurrent = pQComboBox->currentIndex();
            aAny <<= static_cast<sal_Int32>(nCurrent);
        }
        break;
        default:
            SAL_WARN("vcl.kde5", "unknown action on list control " << nAction);
            break;
    }
    return aAny;
}

OUString KDE5FilePicker::implGetDirectory()
{
    OUString dir = toOUString(m_pFileDialog->directoryUrl().url());
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

    m_pFileDialog->setAcceptMode(operationMode);

    if (saveDialog)
    {
        m_pFileDialog->setConfirmOverwrite(true);
        m_pFileDialog->setFileMode(QFileDialog::AnyFile);
    }

    setTitle(VclResId(saveDialog ? STR_FPICKER_SAVE : STR_FPICKER_OPEN));
}

// XCancellable
void SAL_CALL KDE5FilePicker::cancel()
{
    // TODO
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

bool KDE5FilePicker::eventFilter(QObject* o, QEvent* e)
{
    if (e->type() == QEvent::Show && o->isWidgetType())
    {
        auto* w = static_cast<QWidget*>(o);
        if (!w->parentWidget() && w->isModal())
        {
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

#include <KDE5FilePicker.moc>

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
