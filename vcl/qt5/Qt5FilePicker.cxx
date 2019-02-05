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

#include <Qt5FilePicker.hxx>
#include <Qt5FilePicker.moc>

#include <Qt5Frame.hxx>
#include <Qt5Tools.hxx>
#include <Qt5Widget.hxx>

#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/ui/dialogs/CommonFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/ControlActions.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <cppuhelper/interfacecontainer.h>
#include <cppuhelper/supportsservice.hxx>
#include <sal/log.hxx>

#include <QtCore/QDebug>
#include <QtCore/QRegularExpression>
#include <QtCore/QThread>
#include <QtCore/QUrl>
#include <QtGui/QClipboard>
#include <QtGui/QWindow>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

#include <unx/geninst.h>
#include <strings.hrc>

using namespace ::com::sun::star;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::ui::dialogs::TemplateDescription;
using namespace ::com::sun::star::ui::dialogs::ExtendedFilePickerElementIds;
using namespace ::com::sun::star::ui::dialogs::CommonFilePickerElementIds;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;

namespace
{
uno::Sequence<OUString> FilePicker_getSupportedServiceNames()
{
    uno::Sequence<OUString> aRet(3);
    aRet[0] = "com.sun.star.ui.dialogs.FilePicker";
    aRet[1] = "com.sun.star.ui.dialogs.SystemFilePicker";
    aRet[2] = "com.sun.star.ui.dialogs.Qt5FilePicker";
    return aRet;
}
}

Qt5FilePicker::Qt5FilePicker(QFileDialog::FileMode eMode)
    : Qt5FilePicker_Base(m_aHelperMutex)
    , m_pFileDialog(new QFileDialog())
{
    m_pFileDialog->setOption(QFileDialog::DontUseNativeDialog);
    m_pFileDialog->setFileMode(eMode);
    m_pFileDialog->setWindowModality(Qt::ApplicationModal);

    m_pExtraControls = new QWidget();
    m_pLayout = dynamic_cast<QGridLayout*>(m_pFileDialog->layout());

    setMultiSelectionMode(false);

    // XExecutableDialog functions
    connect(this, SIGNAL(setTitleSignal(const OUString&)), this,
            SLOT(setTitleSlot(const OUString&)), Qt::BlockingQueuedConnection);
    connect(this, SIGNAL(executeSignal()), this, SLOT(executeSlot()), Qt::BlockingQueuedConnection);

    // XFilePicker functions
    connect(this, SIGNAL(setMultiSelectionModeSignal(bool)), this,
            SLOT(setMultiSelectionModeSlot(bool)), Qt::BlockingQueuedConnection);
    connect(this, SIGNAL(setDefaultNameSignal(const OUString&)), this,
            SLOT(setDefaultNameSlot(const OUString&)), Qt::BlockingQueuedConnection);
    connect(this, SIGNAL(setDisplayDirectorySignal(const OUString&)), this,
            SLOT(setDisplayDirectorySlot(const OUString&)), Qt::BlockingQueuedConnection);
    connect(this, SIGNAL(getDisplayDirectorySignal()), this, SLOT(getDisplayDirectorySlot()),
            Qt::BlockingQueuedConnection);
    connect(this, SIGNAL(getFilesSignal()), this, SLOT(getFilesSlot()),
            Qt::BlockingQueuedConnection);

    // XFilterManager functions
    connect(this, SIGNAL(appendFilterSignal(const OUString&, const OUString&)), this,
            SLOT(appendFilterSlot(const OUString&, const OUString&)), Qt::BlockingQueuedConnection);
    connect(this, SIGNAL(setCurrentFilterSignal(const OUString&)), this,
            SLOT(setCurrentFilterSlot(const OUString&)), Qt::BlockingQueuedConnection);
    connect(this, SIGNAL(getCurrentFilterSignal()), this, SLOT(getCurrentFilterSlot()),
            Qt::BlockingQueuedConnection);

    // XFilterGroupManager functions
    connect(this,
            SIGNAL(appendFilterGroupSignal(const OUString&,
                                           const css::uno::Sequence<css::beans::StringPair>&)),
            this,
            SLOT(appendFilterGroupSlot(const OUString&,
                                       const css::uno::Sequence<css::beans::StringPair>&)),
            Qt::BlockingQueuedConnection);

    // XFilePickerControlAccess functions
    connect(this, SIGNAL(setValueSignal(sal_Int16, sal_Int16, const css::uno::Any&)), this,
            SLOT(setValueSlot(sal_Int16, sal_Int16, const css::uno::Any&)),
            Qt::BlockingQueuedConnection);
    connect(this, SIGNAL(getValueSignal(sal_Int16, sal_Int16)), this,
            SLOT(getValueSlot(sal_Int16, sal_Int16)), Qt::BlockingQueuedConnection);
    connect(this, SIGNAL(enableControlSignal(sal_Int16, bool)), this,
            SLOT(enableControlSlot(sal_Int16, bool)), Qt::BlockingQueuedConnection);
    connect(this, SIGNAL(setLabelSignal(sal_Int16, const OUString&)), this,
            SLOT(setLabelSlot(sal_Int16, const OUString&)), Qt::BlockingQueuedConnection);
    connect(this, SIGNAL(getLabelSignal(sal_Int16)), this, SLOT(getLabelSlot(sal_Int16)),
            Qt::BlockingQueuedConnection);

    // XFilePicker2 functions
    connect(this, SIGNAL(getSelectedFilesSignal()), this, SLOT(getSelectedFilesSlot()),
            Qt::BlockingQueuedConnection);

    // XInitialization
    connect(this, SIGNAL(initializeSignal(const css::uno::Sequence<css::uno::Any>&)), this,
            SLOT(initializeSlot(const css::uno::Sequence<css::uno::Any>&)),
            Qt::BlockingQueuedConnection);

    // XFilePickerListener notifications
    connect(m_pFileDialog.get(), SIGNAL(filterSelected(const QString&)), this,
            SLOT(filterSelected(const QString&)));
    connect(m_pFileDialog.get(), SIGNAL(currentChanged(const QString&)), this,
            SLOT(currentChanged(const QString&)));

    // update automatic file extension when filter is changed
    connect(m_pFileDialog.get(), SIGNAL(filterSelected(const QString&)), this,
            SLOT(updateAutomaticFileExtension()));
}

Qt5FilePicker::~Qt5FilePicker() {}

void SAL_CALL
Qt5FilePicker::addFilePickerListener(const uno::Reference<XFilePickerListener>& xListener)
{
    SolarMutexGuard aGuard;
    m_xListener = xListener;
}

void SAL_CALL Qt5FilePicker::removeFilePickerListener(const uno::Reference<XFilePickerListener>&)
{
    SolarMutexGuard aGuard;
    m_xListener.clear();
}

void SAL_CALL Qt5FilePicker::setTitle(const OUString& title)
{
    if (qApp->thread() != QThread::currentThread())
    {
        SolarMutexReleaser aReleaser;
        return Q_EMIT setTitleSignal(title);
    }

    m_pFileDialog->setWindowTitle(toQString(title));
}

sal_Int16 SAL_CALL Qt5FilePicker::execute()
{
    if (qApp->thread() != QThread::currentThread())
    {
        SolarMutexReleaser aReleaser;
        return Q_EMIT executeSignal();
    }

    vcl::Window* pWindow = ::Application::GetActiveTopWindow();
    QWidget* pTransientParent = nullptr;
    QWindow* pTransientWindow = nullptr;
    if (pWindow)
    {
        Qt5Frame* pFrame = dynamic_cast<Qt5Frame*>(pWindow->ImplGetFrame());
        assert(pFrame);
        if (pFrame)
        {
            pTransientParent = pFrame->GetQWidget();
            pTransientWindow = pTransientParent->window()->windowHandle();
        }
    }

    if (!m_aNamedFilterList.isEmpty())
        m_pFileDialog->setNameFilters(m_aNamedFilterList);
    if (!m_aCurrentFilter.isEmpty())
        m_pFileDialog->selectNameFilter(m_aCurrentFilter);

    if (pTransientParent)
    {
        m_pFileDialog->show();
        m_pFileDialog->window()->windowHandle()->setTransientParent(pTransientWindow);
        m_pFileDialog->setFocusProxy(pTransientParent);
    }

    updateAutomaticFileExtension();

    int result = m_pFileDialog->exec();
    if (QFileDialog::Rejected == result)
        return ExecutableDialogResults::CANCEL;
    return ExecutableDialogResults::OK;
}

void SAL_CALL Qt5FilePicker::setMultiSelectionMode(sal_Bool multiSelect)
{
    if (qApp->thread() != QThread::currentThread())
    {
        SolarMutexReleaser aReleaser;
        return Q_EMIT setMultiSelectionModeSignal(multiSelect);
    }

    if (multiSelect)
        m_pFileDialog->setFileMode(QFileDialog::ExistingFiles);
    else
        m_pFileDialog->setFileMode(QFileDialog::ExistingFile);
}

void SAL_CALL Qt5FilePicker::setDefaultName(const OUString& name)
{
    if (qApp->thread() != QThread::currentThread())
    {
        SolarMutexReleaser aReleaser;
        return Q_EMIT setDefaultNameSignal(name);
    }
    m_pFileDialog->selectFile(toQString(name));
}

void SAL_CALL Qt5FilePicker::setDisplayDirectory(const OUString& dir)
{
    if (qApp->thread() != QThread::currentThread())
    {
        SolarMutexReleaser aReleaser;
        return Q_EMIT setDisplayDirectorySignal(dir);
    }

    QString qDir(toQString(dir));
    m_pFileDialog->setDirectoryUrl(QUrl(qDir));
}

OUString SAL_CALL Qt5FilePicker::getDisplayDirectory()
{
    if (qApp->thread() != QThread::currentThread())
    {
        SolarMutexReleaser aReleaser;
        return Q_EMIT getDisplayDirectorySignal();
    }
    return toOUString(m_pFileDialog->directoryUrl().toString());
}

uno::Sequence<OUString> SAL_CALL Qt5FilePicker::getFiles()
{
    if (qApp->thread() != QThread::currentThread())
    {
        SolarMutexReleaser aReleaser;
        return Q_EMIT getFilesSignal();
    }
    uno::Sequence<OUString> seq = getSelectedFiles();
    if (seq.getLength() > 1)
        seq.realloc(1);
    return seq;
}

uno::Sequence<OUString> SAL_CALL Qt5FilePicker::getSelectedFiles()
{
    if (qApp->thread() != QThread::currentThread())
    {
        SolarMutexReleaser aReleaser;
        return Q_EMIT getSelectedFilesSignal();
    }
    QList<QUrl> urls = m_pFileDialog->selectedUrls();
    uno::Sequence<OUString> seq(urls.size());
    int i = 0;
    foreach (const QUrl& url, urls)
        seq[i++] = toOUString(url.toString());
    return seq;
}

void SAL_CALL Qt5FilePicker::appendFilter(const OUString& title, const OUString& filter)
{
    if (qApp->thread() != QThread::currentThread())
    {
        SolarMutexReleaser aReleaser;
        return Q_EMIT appendFilterSignal(title, filter);
    }

    // '/' need to be escaped else they are assumed to be mime types
    QString t = toQString(title).replace("/", "\\/");

    QString n = t;
    // strip duplicated type
    int pos = n.indexOf(" (");
    if (pos >= 0)
        n.truncate(pos);

    QString f = toQString(filter);

    // openoffice gives us filters separated by ';' qt dialogs just want space separated
    f.replace(";", " ");

    // make sure "*.*" is not used as "all files"
    f.replace("*.*", "*");

    m_aNamedFilterList << QStringLiteral("%1 (%2)").arg(n, f);
    m_aTitleToFilterMap[t] = m_aNamedFilterList.constLast();
    m_aNamedFilterToExtensionMap[m_aNamedFilterList.constLast()] = f;
}

void SAL_CALL Qt5FilePicker::setCurrentFilter(const OUString& title)
{
    if (qApp->thread() != QThread::currentThread())
    {
        SolarMutexReleaser aReleaser;
        return Q_EMIT setCurrentFilterSignal(title);
    }

    m_aCurrentFilter = m_aTitleToFilterMap.value(toQString(title).replace("/", "\\/"));
}

OUString SAL_CALL Qt5FilePicker::getCurrentFilter()
{
    if (qApp->thread() != QThread::currentThread())
    {
        SolarMutexReleaser aReleaser;
        return Q_EMIT getCurrentFilterSignal();
    }

    QString filter = m_aTitleToFilterMap.key(m_pFileDialog->selectedNameFilter());
    if (filter.isEmpty())
        filter = "ODF Text Document (.odt)";
    return toOUString(filter);
}

void SAL_CALL Qt5FilePicker::appendFilterGroup(const OUString& rGroupTitle,
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

static uno::Any HandleGetListValue(QComboBox* pWidget, sal_Int16 nControlAction)
{
    uno::Any aAny;
    switch (nControlAction)
    {
        case ControlActions::GET_ITEMS:
        {
            Sequence<OUString> aItemList(pWidget->count());
            for (sal_Int32 i = 0; i < pWidget->count(); ++i)
                aItemList[i] = toOUString(pWidget->itemText(i));
            aAny <<= aItemList;
            break;
        }
        case ControlActions::GET_SELECTED_ITEM:
        {
            if (!pWidget->currentText().isEmpty())
                aAny <<= toOUString(pWidget->currentText());
            break;
        }
        case ControlActions::GET_SELECTED_ITEM_INDEX:
        {
            if (pWidget->currentIndex() >= 0)
                aAny <<= static_cast<sal_Int32>(pWidget->currentIndex());
            break;
        }
        default:
            SAL_WARN("vcl.qt5",
                     "undocumented/unimplemented ControlAction for a list " << nControlAction);
            break;
    }
    return aAny;
}

static void HandleSetListValue(QComboBox* pWidget, sal_Int16 nControlAction, const uno::Any& rValue)
{
    switch (nControlAction)
    {
        case ControlActions::ADD_ITEM:
        {
            OUString sItem;
            rValue >>= sItem;
            pWidget->addItem(toQString(sItem));
            break;
        }
        case ControlActions::ADD_ITEMS:
        {
            Sequence<OUString> aStringList;
            rValue >>= aStringList;
            for (auto const& sItem : aStringList)
                pWidget->addItem(toQString(sItem));
            break;
        }
        case ControlActions::DELETE_ITEM:
        {
            sal_Int32 nPos = 0;
            rValue >>= nPos;
            pWidget->removeItem(nPos);
            break;
        }
        case ControlActions::SET_SELECT_ITEM:
        {
            sal_Int32 nPos = 0;
            rValue >>= nPos;
            pWidget->setCurrentIndex(nPos);
            break;
        }
        default:
            SAL_WARN("vcl.qt5",
                     "undocumented/unimplemented ControlAction for a list " << nControlAction);
            break;
    }

    pWidget->setEnabled(pWidget->count() > 0);
}

void SAL_CALL Qt5FilePicker::setValue(sal_Int16 controlId, sal_Int16 nControlAction,
                                      const uno::Any& value)
{
    if (qApp->thread() != QThread::currentThread())
    {
        SolarMutexReleaser aReleaser;
        return Q_EMIT setValueSignal(controlId, nControlAction, value);
    }

    if (m_aCustomWidgetsMap.contains(controlId))
    {
        QWidget* widget = m_aCustomWidgetsMap.value(controlId);
        QCheckBox* cb = dynamic_cast<QCheckBox*>(widget);
        if (cb)
            cb->setChecked(value.get<bool>());
        else
        {
            QComboBox* combo = dynamic_cast<QComboBox*>(widget);
            if (combo)
                HandleSetListValue(combo, nControlAction, value);
        }
    }
    else
        SAL_WARN("vcl.qt5", "set value on unknown control " << controlId);
}

uno::Any SAL_CALL Qt5FilePicker::getValue(sal_Int16 controlId, sal_Int16 nControlAction)
{
    if (qApp->thread() != QThread::currentThread())
    {
        SolarMutexReleaser aReleaser;
        return Q_EMIT getValueSignal(controlId, nControlAction);
    }

    uno::Any res(false);
    if (m_aCustomWidgetsMap.contains(controlId))
    {
        QWidget* widget = m_aCustomWidgetsMap.value(controlId);
        QCheckBox* cb = dynamic_cast<QCheckBox*>(widget);
        if (cb)
            res <<= cb->isChecked();
        else
        {
            QComboBox* combo = dynamic_cast<QComboBox*>(widget);
            if (combo)
                res = HandleGetListValue(combo, nControlAction);
        }
    }
    else
        SAL_WARN("vcl.qt5", "get value on unknown control " << controlId);

    return res;
}

void SAL_CALL Qt5FilePicker::enableControl(sal_Int16 controlId, sal_Bool enable)
{
    if (qApp->thread() != QThread::currentThread())
    {
        SolarMutexReleaser aReleaser;
        return Q_EMIT enableControlSignal(controlId, enable);
    }

    if (m_aCustomWidgetsMap.contains(controlId))
        m_aCustomWidgetsMap.value(controlId)->setEnabled(enable);
    else
        SAL_WARN("vcl.qt5", "enable unknown control " << controlId);
}

void SAL_CALL Qt5FilePicker::setLabel(sal_Int16 controlId, const OUString& label)
{
    if (qApp->thread() != QThread::currentThread())
    {
        SolarMutexReleaser aReleaser;
        return Q_EMIT setLabelSignal(controlId, label);
    }

    if (m_aCustomWidgetsMap.contains(controlId))
    {
        QCheckBox* cb = dynamic_cast<QCheckBox*>(m_aCustomWidgetsMap.value(controlId));
        if (cb)
            cb->setText(toQString(label));
    }
    else
        SAL_WARN("vcl.qt5", "set label on unknown control " << controlId);
}

OUString SAL_CALL Qt5FilePicker::getLabel(sal_Int16 controlId)
{
    if (qApp->thread() != QThread::currentThread())
    {
        SolarMutexReleaser aReleaser;
        return Q_EMIT getLabelSignal(controlId);
    }

    QString label;
    if (m_aCustomWidgetsMap.contains(controlId))
    {
        QCheckBox* cb = dynamic_cast<QCheckBox*>(m_aCustomWidgetsMap.value(controlId));
        if (cb)
            label = cb->text();
    }
    else
        SAL_WARN("vcl.qt5", "get label on unknown control " << controlId);

    return toOUString(label);
}

QString Qt5FilePicker::getResString(const char* pResId)
{
    QString aResString;

    if (pResId == nullptr)
        return aResString;

    aResString = toQString(VclResId(pResId));

    return aResString.replace('~', '&');
}

void Qt5FilePicker::addCustomControl(sal_Int16 controlId)
{
    QWidget* widget = nullptr;
    QLabel* label = nullptr;
    const char* resId = nullptr;
    QCheckBox* pCheckbox = nullptr;

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
        case LISTBOX_VERSION_LABEL:
        case LISTBOX_TEMPLATE_LABEL:
        case LISTBOX_IMAGE_TEMPLATE_LABEL:
        case LISTBOX_FILTER_SELECTOR:
            break;
    }

    switch (controlId)
    {
        case CHECKBOX_AUTOEXTENSION:
            pCheckbox = new QCheckBox(getResString(resId), m_pExtraControls);
            // to add/remove automatic file extension based on checkbox
            connect(pCheckbox, SIGNAL(stateChanged(int)), this,
                    SLOT(updateAutomaticFileExtension()));
            widget = pCheckbox;
            break;
        case CHECKBOX_PASSWORD:
        case CHECKBOX_FILTEROPTIONS:
        case CHECKBOX_READONLY:
        case CHECKBOX_LINK:
        case CHECKBOX_PREVIEW:
        case CHECKBOX_SELECTION:
        case CHECKBOX_GPGENCRYPTION:
            widget = new QCheckBox(getResString(resId), m_pExtraControls);
            break;
        case PUSHBUTTON_PLAY:
            break;
        case LISTBOX_VERSION:
        case LISTBOX_TEMPLATE:
        case LISTBOX_IMAGE_TEMPLATE:
        case LISTBOX_FILTER_SELECTOR:
            label = new QLabel(getResString(resId), m_pExtraControls);
            widget = new QComboBox(m_pExtraControls);
            label->setBuddy(widget);
            break;
        case LISTBOX_VERSION_LABEL:
        case LISTBOX_TEMPLATE_LABEL:
        case LISTBOX_IMAGE_TEMPLATE_LABEL:
            break;
    }

    if (widget)
    {
        const int row = m_pLayout->rowCount();
        if (label)
            m_pLayout->addWidget(label, row, 0);
        m_pLayout->addWidget(widget, row, 1);
        m_aCustomWidgetsMap.insert(controlId, widget);
    }
}

void SAL_CALL Qt5FilePicker::initialize(const uno::Sequence<uno::Any>& args)
{
    if (qApp->thread() != QThread::currentThread())
    {
        SolarMutexReleaser aReleaser;
        return Q_EMIT initializeSignal(args);
    }

    m_aNamedFilterList.clear();
    m_aTitleToFilterMap.clear();
    m_aCurrentFilter.clear();

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

    QFileDialog::AcceptMode acceptMode = QFileDialog::AcceptOpen;
    switch (templateId)
    {
        case FILEOPEN_SIMPLE:
            break;

        case FILESAVE_SIMPLE:
            acceptMode = QFileDialog::AcceptSave;
            break;

        case FILESAVE_AUTOEXTENSION:
            acceptMode = QFileDialog::AcceptSave;
            addCustomControl(CHECKBOX_AUTOEXTENSION);
            break;

        case FILESAVE_AUTOEXTENSION_PASSWORD:
            acceptMode = QFileDialog::AcceptSave;
            addCustomControl(CHECKBOX_AUTOEXTENSION);
            addCustomControl(CHECKBOX_PASSWORD);
            addCustomControl(CHECKBOX_GPGENCRYPTION);
            break;

        case FILESAVE_AUTOEXTENSION_PASSWORD_FILTEROPTIONS:
            acceptMode = QFileDialog::AcceptSave;
            addCustomControl(CHECKBOX_AUTOEXTENSION);
            addCustomControl(CHECKBOX_PASSWORD);
            addCustomControl(CHECKBOX_GPGENCRYPTION);
            addCustomControl(CHECKBOX_FILTEROPTIONS);
            break;

        case FILESAVE_AUTOEXTENSION_SELECTION:
            acceptMode = QFileDialog::AcceptSave;
            addCustomControl(CHECKBOX_AUTOEXTENSION);
            addCustomControl(CHECKBOX_SELECTION);
            break;

        case FILESAVE_AUTOEXTENSION_TEMPLATE:
            acceptMode = QFileDialog::AcceptSave;
            addCustomControl(CHECKBOX_AUTOEXTENSION);
            addCustomControl(LISTBOX_TEMPLATE);
            break;

        case FILEOPEN_LINK_PREVIEW_IMAGE_TEMPLATE:
            addCustomControl(CHECKBOX_LINK);
            addCustomControl(CHECKBOX_PREVIEW);
            addCustomControl(LISTBOX_IMAGE_TEMPLATE);
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
            throw lang::IllegalArgumentException("Unknown template",
                                                 static_cast<XFilePicker2*>(this), 1);
    }

    const char* resId = nullptr;
    switch (acceptMode)
    {
        case QFileDialog::AcceptOpen:
            resId = STR_FPICKER_OPEN;
            break;
        case QFileDialog::AcceptSave:
            resId = STR_FPICKER_SAVE;
            m_pFileDialog->setFileMode(QFileDialog::AnyFile);
            break;
    }

    m_pFileDialog->setAcceptMode(acceptMode);
    m_pFileDialog->setWindowTitle(getResString(resId));
}

void SAL_CALL Qt5FilePicker::cancel() {}

void Qt5FilePicker::disposing(const lang::EventObject& rEvent)
{
    uno::Reference<XFilePickerListener> xFilePickerListener(rEvent.Source, uno::UNO_QUERY);

    if (xFilePickerListener.is())
    {
        removeFilePickerListener(xFilePickerListener);
    }
}

OUString SAL_CALL Qt5FilePicker::getImplementationName()
{
    return OUString("com.sun.star.ui.dialogs.Qt5FilePicker");
}

sal_Bool SAL_CALL Qt5FilePicker::supportsService(const OUString& ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

uno::Sequence<OUString> SAL_CALL Qt5FilePicker::getSupportedServiceNames()
{
    return FilePicker_getSupportedServiceNames();
}

void Qt5FilePicker::updateAutomaticFileExtension()
{
    bool bSetAutoExtension
        = getValue(CHECKBOX_AUTOEXTENSION, ControlActions::GET_SELECTED_ITEM).get<bool>();
    if (bSetAutoExtension)
    {
        QString sSuffix = m_aNamedFilterToExtensionMap.value(m_pFileDialog->selectedNameFilter());
        // string is "*.<SUFFIX>" if a specific filter was selected that has exactly one possible file extension
        if (sSuffix.lastIndexOf("*.") == 0)
        {
            sSuffix = sSuffix.remove("*.");
            m_pFileDialog->setDefaultSuffix(sSuffix);
        }
        else
        {
            // fall back to setting none otherwise
            SAL_INFO(
                "vcl.qt5",
                "Unable to retrieve unambiguous file extension. Will not add any automatically.");
            bSetAutoExtension = false;
        }
    }

    if (!bSetAutoExtension)
        m_pFileDialog->setDefaultSuffix("");
}

void Qt5FilePicker::filterSelected(const QString&)
{
    FilePickerEvent aEvent;
    aEvent.ElementId = LISTBOX_FILTER;
    SAL_INFO("vcl.qt5", "filter changed");
    if (m_xListener.is())
        m_xListener->controlStateChanged(aEvent);
}

void Qt5FilePicker::currentChanged(const QString&)
{
    FilePickerEvent aEvent;
    SAL_INFO("vcl.qt5", "file selection changed");
    if (m_xListener.is())
        m_xListener->fileSelectionChanged(aEvent);
}

OUString Qt5FilePicker::getDirectory()
{
    uno::Sequence<OUString> seq = getSelectedFiles();
    if (seq.getLength() > 1)
        seq.realloc(1);
    return seq[0];
}

void Qt5FilePicker::setDescription(const OUString&) {}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
