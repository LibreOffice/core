/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#include <config_gpgme.h>

#include <fpicker/fpsofficeResMgr.hxx>
#include <QtFilePicker.hxx>
#include <QtFilePicker.moc>

#include <QtFrame.hxx>
#include <QtInstance.hxx>
#include <QtXWindow.hxx>

#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/TerminationVetoException.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/ui/dialogs/CommonFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/ControlActions.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/uri/ExternalUriReferenceTranslator.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <sal/log.hxx>
#include <vcl/qt/QtUtils.hxx>
#include <vcl/toolkit/unowrap.hxx>

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

#include <fpicker/strings.hrc>
#include <utility>

using namespace ::com::sun::star;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::ui::dialogs::TemplateDescription;
using namespace ::com::sun::star::ui::dialogs::ExtendedFilePickerElementIds;
using namespace ::com::sun::star::ui::dialogs::CommonFilePickerElementIds;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;

QtFilePicker::QtFilePicker(css::uno::Reference<css::uno::XComponentContext> context,
                           QFileDialog::FileMode eMode, bool bUseNative)
    : QtFilePicker_Base(m_aHelperMutex)
    , m_context(std::move(context))
    , m_bIsFolderPicker(eMode == QFileDialog::Directory)
    , m_pParentWidget(nullptr)
    , m_pFileDialog(new QFileDialog(nullptr, {}, QDir::homePath()))
    , m_pExtraControls(new QWidget())
{
    m_pFileDialog->setOption(QFileDialog::DontUseNativeDialog, !bUseNative);

    m_pFileDialog->setFileMode(eMode);
    m_pFileDialog->setWindowModality(Qt::ApplicationModal);

    if (m_bIsFolderPicker)
    {
        m_pFileDialog->setOption(QFileDialog::ShowDirsOnly, true);
        m_pFileDialog->setWindowTitle(toQString(FpsResId(STR_SVT_FOLDERPICKER_DEFAULT_TITLE)));
    }

    m_pLayout = qobject_cast<QGridLayout*>(m_pFileDialog->layout());

    setMultiSelectionMode(false);

    // XFilePickerListener notifications
    connect(m_pFileDialog.get(), &QFileDialog::filterSelected, this, &QtFilePicker::filterSelected);
    connect(m_pFileDialog.get(), &QFileDialog::currentChanged, this, &QtFilePicker::currentChanged);

    // update automatic file extension when filter is changed
    connect(m_pFileDialog.get(), &QFileDialog::filterSelected, this,
            &QtFilePicker::updateAutomaticFileExtension);

    connect(m_pFileDialog.get(), &QFileDialog::finished, this, &QtFilePicker::finished);
}

QtFilePicker::~QtFilePicker()
{
    SolarMutexGuard g;
    GetQtInstance().RunInMainThread([this]() {
        // must delete it in main thread, otherwise
        // QSocketNotifier::setEnabled() will crash us
        m_pFileDialog.reset();
    });
}

void SAL_CALL
QtFilePicker::addFilePickerListener(const uno::Reference<XFilePickerListener>& xListener)
{
    SolarMutexGuard aGuard;
    m_xListener = xListener;
}

void SAL_CALL QtFilePicker::removeFilePickerListener(const uno::Reference<XFilePickerListener>&)
{
    SolarMutexGuard aGuard;
    m_xListener.clear();
}

void SAL_CALL QtFilePicker::setTitle(const OUString& title)
{
    SolarMutexGuard g;
    GetQtInstance().RunInMainThread(
        [this, &title]() { m_pFileDialog->setWindowTitle(toQString(title)); });
}

void QtFilePicker::prepareExecute()
{
    QWidget* pTransientParent = m_pParentWidget;
    if (!pTransientParent)
    {
        vcl::Window* pWindow = ::Application::GetActiveTopWindow();
        if (pWindow)
        {
            QtFrame* pFrame = dynamic_cast<QtFrame*>(pWindow->ImplGetFrame());
            assert(pFrame);
            if (pFrame)
                pTransientParent = pFrame->asChild();
        }
    }

    if (!m_aNamedFilterList.isEmpty())
        m_pFileDialog->setNameFilters(m_aNamedFilterList);
    if (!m_aCurrentFilter.isEmpty())
        m_pFileDialog->selectNameFilter(m_aCurrentFilter);

    updateAutomaticFileExtension();

    uno::Reference<css::frame::XDesktop> xDesktop(css::frame::Desktop::create(m_context),
                                                  UNO_QUERY_THROW);
    m_pFileDialog->setParent(pTransientParent, m_pFileDialog->windowFlags());
    xDesktop->addTerminateListener(this);
}

void QtFilePicker::finished(int nResult)
{
    SolarMutexGuard g;
    uno::Reference<css::frame::XDesktop> xDesktop(css::frame::Desktop::create(m_context),
                                                  UNO_QUERY_THROW);
    xDesktop->removeTerminateListener(this);
    m_pFileDialog->setParent(nullptr, m_pFileDialog->windowFlags());

    if (m_xClosedListener.is())
    {
        const sal_Int16 nRet = (QFileDialog::Rejected == nResult) ? ExecutableDialogResults::CANCEL
                                                                  : ExecutableDialogResults::OK;
        css::ui::dialogs::DialogClosedEvent aEvent(*this, nRet);
        m_xClosedListener->dialogClosed(aEvent);
        m_xClosedListener.clear();
    }
}

sal_Int16 SAL_CALL QtFilePicker::execute()
{
    SolarMutexGuard g;
    QtInstance& rQtInstance = GetQtInstance();
    if (!rQtInstance.IsMainThread())
    {
        sal_uInt16 ret;
        rQtInstance.RunInMainThread([&ret, this]() { ret = execute(); });
        return ret;
    }

    prepareExecute();
    int result = m_pFileDialog->exec();

    if (QFileDialog::Rejected == result)
        return ExecutableDialogResults::CANCEL;
    return ExecutableDialogResults::OK;
}

// XAsynchronousExecutableDialog functions
void SAL_CALL QtFilePicker::setDialogTitle(const OUString& _rTitle) { setTitle(_rTitle); }

void SAL_CALL
QtFilePicker::startExecuteModal(const Reference<css::ui::dialogs::XDialogClosedListener>& xListener)
{
    m_xClosedListener = xListener;
    prepareExecute();
    m_pFileDialog->show();
}

void SAL_CALL QtFilePicker::setMultiSelectionMode(sal_Bool multiSelect)
{
    SolarMutexGuard g;
    GetQtInstance().RunInMainThread([this, multiSelect]() {
        if (m_bIsFolderPicker || m_pFileDialog->acceptMode() == QFileDialog::AcceptSave)
            return;

        m_pFileDialog->setFileMode(multiSelect ? QFileDialog::ExistingFiles
                                               : QFileDialog::ExistingFile);
    });
}

void SAL_CALL QtFilePicker::setDefaultName(const OUString& name)
{
    SolarMutexGuard g;
    GetQtInstance().RunInMainThread(
        [this, &name]() { m_pFileDialog->selectFile(toQString(name)); });
}

void SAL_CALL QtFilePicker::setDisplayDirectory(const OUString& dir)
{
    SolarMutexGuard g;
    GetQtInstance().RunInMainThread([this, &dir]() {
        QString qDir(toQString(dir));
        m_pFileDialog->setDirectoryUrl(QUrl(qDir));
    });
}

OUString SAL_CALL QtFilePicker::getDisplayDirectory()
{
    SolarMutexGuard g;
    OUString ret;
    GetQtInstance().RunInMainThread(
        [&ret, this]() { ret = toOUString(m_pFileDialog->directoryUrl().toString()); });
    return ret;
}

uno::Sequence<OUString> SAL_CALL QtFilePicker::getFiles()
{
    uno::Sequence<OUString> seq = getSelectedFiles();
    if (seq.getLength() > 1)
        seq.realloc(1);
    return seq;
}

uno::Sequence<OUString> SAL_CALL QtFilePicker::getSelectedFiles()
{
    SolarMutexGuard g;
    QList<QUrl> urls;
    GetQtInstance().RunInMainThread([&urls, this]() { urls = m_pFileDialog->selectedUrls(); });

    uno::Sequence<OUString> seq(urls.size());
    auto seqRange = asNonConstRange(seq);

    auto const trans = css::uri::ExternalUriReferenceTranslator::create(m_context);
    size_t i = 0;
    for (const QUrl& aURL : urls)
    {
        // Unlike LO, QFileDialog (<https://doc.qt.io/qt-5/qfiledialog.html>) apparently always
        // treats file-system pathnames as UTF-8--encoded, regardless of LANG/LC_CTYPE locale
        // setting.  And pathnames containing byte sequences that are not valid UTF-8 are apparently
        // filtered out and not even displayed by QFileDialog, so aURL will always have a "payload"
        // that matches the pathname's byte sequence.  So the pathname's byte sequence (which
        // happens to also be aURL's payload) in the LANG/LC_CTYPE encoding needs to be converted
        // into LO's internal UTF-8 file URL encoding via
        // XExternalUriReferenceTranslator::translateToInternal (which looks somewhat paradoxical as
        // aURL.toEncoded() nominally already has a UTF-8 payload):
        auto const extUrl = toOUString(aURL.toEncoded());
        auto intUrl = trans->translateToInternal(extUrl);
        if (intUrl.isEmpty())
        {
            // If translation failed, fall back to original URL:
            SAL_WARN("vcl.qt", "cannot convert <" << extUrl << "> from locale encoding to UTF-8");
            intUrl = extUrl;
        }
        seqRange[i++] = intUrl;
    }

    return seq;
}

void SAL_CALL QtFilePicker::appendFilter(const OUString& title, const OUString& filter)
{
    SolarMutexGuard g;
    QtInstance& rQtInstance = GetQtInstance();
    if (!rQtInstance.IsMainThread())
    {
        rQtInstance.RunInMainThread([this, &title, &filter]() { appendFilter(title, filter); });
        return;
    }

    // '/' need to be escaped else they are assumed to be mime types
    QString sTitle = toQString(title).replace("/", "\\/");

    QString sFilterName = sTitle;
    // the Qt non-native file picker adds the extensions to the filter title, so strip them
    if (m_pFileDialog->testOption(QFileDialog::DontUseNativeDialog))
    {
        int pos = sFilterName.indexOf(" (");
        if (pos >= 0)
            sFilterName.truncate(pos);
    }

    QString sGlobFilter = toQString(filter);

    // LibreOffice gives us filters separated by ';' qt dialogs just want space separated
    sGlobFilter.replace(";", " ");

    // make sure "*.*" is not used as "all files"
    sGlobFilter.replace("*.*", "*");

    m_aNamedFilterList << QStringLiteral("%1 (%2)").arg(sFilterName, sGlobFilter);
    m_aTitleToFilterMap[sTitle] = m_aNamedFilterList.constLast();
    m_aNamedFilterToExtensionMap[m_aNamedFilterList.constLast()] = sGlobFilter;
}

void SAL_CALL QtFilePicker::setCurrentFilter(const OUString& title)
{
    SolarMutexGuard g;
    GetQtInstance().RunInMainThread([this, &title]() {
        m_aCurrentFilter = m_aTitleToFilterMap.value(toQString(title).replace("/", "\\/"));
    });
}

OUString SAL_CALL QtFilePicker::getCurrentFilter()
{
    SolarMutexGuard g;
    QString filter;
    QtInstance& rQtInstance = GetQtInstance();
    rQtInstance.RunInMainThread([&filter, this]() {
        filter = m_aTitleToFilterMap.key(m_pFileDialog->selectedNameFilter());
    });

    if (filter.isEmpty())
        filter = "ODF Text Document (.odt)";
    return toOUString(filter);
}

void SAL_CALL QtFilePicker::appendFilterGroup(const OUString& rGroupTitle,
                                              const uno::Sequence<beans::StringPair>& filters)
{
    SolarMutexGuard g;
    QtInstance& rQtInstance = GetQtInstance();
    if (!rQtInstance.IsMainThread())
    {
        rQtInstance.RunInMainThread(
            [this, &rGroupTitle, &filters]() { appendFilterGroup(rGroupTitle, filters); });
        return;
    }

    const sal_uInt16 length = filters.getLength();
    for (sal_uInt16 i = 0; i < length; ++i)
    {
        const beans::StringPair& aPair = filters[i];
        appendFilter(aPair.First, aPair.Second);
    }
}

uno::Any QtFilePicker::handleGetListValue(const QComboBox* pWidget, sal_Int16 nControlAction)
{
    uno::Any aAny;
    switch (nControlAction)
    {
        case ControlActions::GET_ITEMS:
        {
            Sequence<OUString> aItemList(pWidget->count());
            auto aItemListRange = asNonConstRange(aItemList);
            for (sal_Int32 i = 0; i < pWidget->count(); ++i)
                aItemListRange[i] = toOUString(pWidget->itemText(i));
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
            SAL_WARN("vcl.qt",
                     "undocumented/unimplemented ControlAction for a list " << nControlAction);
            break;
    }
    return aAny;
}

void QtFilePicker::handleSetListValue(QComboBox* pWidget, sal_Int16 nControlAction,
                                      const uno::Any& rValue)
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
        case ControlActions::DELETE_ITEMS:
        {
            pWidget->clear();
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
            SAL_WARN("vcl.qt",
                     "undocumented/unimplemented ControlAction for a list " << nControlAction);
            break;
    }

    pWidget->setEnabled(pWidget->count() > 0);
}

void SAL_CALL QtFilePicker::setValue(sal_Int16 controlId, sal_Int16 nControlAction,
                                     const uno::Any& value)
{
    SolarMutexGuard g;
    QtInstance& rQtInstance = GetQtInstance();
    if (!rQtInstance.IsMainThread())
    {
        rQtInstance.RunInMainThread([this, controlId, nControlAction, &value]() {
            setValue(controlId, nControlAction, value);
        });
        return;
    }

    if (m_aCustomWidgetsMap.contains(controlId))
    {
        QWidget* widget = m_aCustomWidgetsMap.value(controlId);
        QCheckBox* cb = qobject_cast<QCheckBox*>(widget);
        if (cb)
            cb->setChecked(value.get<bool>());
        else
        {
            QComboBox* combo = qobject_cast<QComboBox*>(widget);
            if (combo)
                handleSetListValue(combo, nControlAction, value);
        }
    }
    else
        SAL_WARN("vcl.qt", "set value on unknown control " << controlId);
}

uno::Any SAL_CALL QtFilePicker::getValue(sal_Int16 controlId, sal_Int16 nControlAction)
{
    SolarMutexGuard g;
    QtInstance& rQtInstance = GetQtInstance();
    if (!rQtInstance.IsMainThread())
    {
        uno::Any ret;
        rQtInstance.RunInMainThread([&ret, this, controlId, nControlAction]() {
            ret = getValue(controlId, nControlAction);
        });
        return ret;
    }

    uno::Any res(false);
    if (m_aCustomWidgetsMap.contains(controlId))
    {
        QWidget* widget = m_aCustomWidgetsMap.value(controlId);
        QCheckBox* cb = qobject_cast<QCheckBox*>(widget);
        if (cb)
            res <<= cb->isChecked();
        else
        {
            QComboBox* combo = qobject_cast<QComboBox*>(widget);
            if (combo)
                res = handleGetListValue(combo, nControlAction);
        }
    }
    else
        SAL_WARN("vcl.qt", "get value on unknown control " << controlId);

    return res;
}

void SAL_CALL QtFilePicker::enableControl(sal_Int16 controlId, sal_Bool enable)
{
    SolarMutexGuard g;
    GetQtInstance().RunInMainThread([this, controlId, enable]() {
        if (m_aCustomWidgetsMap.contains(controlId))
            m_aCustomWidgetsMap.value(controlId)->setEnabled(enable);
        else
            SAL_WARN("vcl.qt", "enable unknown control " << controlId);
    });
}

void SAL_CALL QtFilePicker::setLabel(sal_Int16 controlId, const OUString& label)
{
    SolarMutexGuard g;
    QtInstance& rQtInstance = GetQtInstance();
    if (!rQtInstance.IsMainThread())
    {
        rQtInstance.RunInMainThread([this, controlId, label]() { setLabel(controlId, label); });
        return;
    }

    if (m_aCustomWidgetsMap.contains(controlId))
    {
        QCheckBox* cb = qobject_cast<QCheckBox*>(m_aCustomWidgetsMap.value(controlId));
        if (cb)
            cb->setText(toQString(label));
    }
    else
        SAL_WARN("vcl.qt", "set label on unknown control " << controlId);
}

OUString SAL_CALL QtFilePicker::getLabel(sal_Int16 controlId)
{
    SolarMutexGuard g;
    QtInstance& rQtInstance = GetQtInstance();
    if (!rQtInstance.IsMainThread())
    {
        OUString ret;
        rQtInstance.RunInMainThread([&ret, this, controlId]() { ret = getLabel(controlId); });
        return ret;
    }

    QString label;
    if (m_aCustomWidgetsMap.contains(controlId))
    {
        QCheckBox* cb = qobject_cast<QCheckBox*>(m_aCustomWidgetsMap.value(controlId));
        if (cb)
            label = cb->text();
    }
    else
        SAL_WARN("vcl.qt", "get label on unknown control " << controlId);

    return toOUString(label);
}

QString QtFilePicker::getResString(TranslateId pResId)
{
    QString aResString;

    if (!pResId)
        return aResString;

    aResString = toQString(FpsResId(pResId));

    return aResString.replace('~', '&');
}

void QtFilePicker::applyTemplate(sal_Int16 nTemplateId)
{
    QFileDialog::AcceptMode acceptMode = QFileDialog::AcceptOpen;
    switch (nTemplateId)
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
#if HAVE_FEATURE_GPGME
            addCustomControl(CHECKBOX_GPGSIGN);
#endif
            break;

        case FILESAVE_AUTOEXTENSION_PASSWORD_FILTEROPTIONS:
            acceptMode = QFileDialog::AcceptSave;
            addCustomControl(CHECKBOX_AUTOEXTENSION);
            addCustomControl(CHECKBOX_PASSWORD);
            addCustomControl(CHECKBOX_GPGENCRYPTION);
#if HAVE_FEATURE_GPGME
            addCustomControl(CHECKBOX_GPGSIGN);
#endif
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

        case FILEOPEN_READONLY_VERSION_FILTEROPTIONS:
            addCustomControl(CHECKBOX_READONLY);
            addCustomControl(LISTBOX_VERSION);
            addCustomControl(CHECKBOX_FILTEROPTIONS);
            break;

        case FILEOPEN_LINK_PREVIEW:
            addCustomControl(CHECKBOX_LINK);
            addCustomControl(CHECKBOX_PREVIEW);
            break;

        case FILEOPEN_PREVIEW:
            addCustomControl(CHECKBOX_PREVIEW);
            break;

        default:
            throw lang::IllegalArgumentException(u"Unknown template"_ustr,
                                                 static_cast<XFilePicker2*>(this), 1);
    }

    TranslateId resId;
    switch (acceptMode)
    {
        case QFileDialog::AcceptOpen:
            resId = STR_FILEDLG_OPEN;
            break;
        case QFileDialog::AcceptSave:
            resId = STR_FILEDLG_SAVE;
            m_pFileDialog->setFileMode(QFileDialog::AnyFile);
            break;
    }

    m_pFileDialog->setAcceptMode(acceptMode);
    m_pFileDialog->setWindowTitle(getResString(resId));
}

void QtFilePicker::addCustomControl(sal_Int16 controlId)
{
    QWidget* widget = nullptr;
    QLabel* label = nullptr;
    TranslateId resId;
    QCheckBox* pCheckbox = nullptr;

    switch (controlId)
    {
        case CHECKBOX_AUTOEXTENSION:
            resId = STR_SVT_FILEPICKER_AUTO_EXTENSION;
            break;
        case CHECKBOX_PASSWORD:
            resId = STR_SVT_FILEPICKER_PASSWORD;
            break;
        case CHECKBOX_FILTEROPTIONS:
            resId = STR_SVT_FILEPICKER_FILTER_OPTIONS;
            break;
        case CHECKBOX_READONLY:
            resId = STR_SVT_FILEPICKER_READONLY;
            break;
        case CHECKBOX_LINK:
            resId = STR_SVT_FILEPICKER_INSERT_AS_LINK;
            break;
        case CHECKBOX_PREVIEW:
            resId = STR_SVT_FILEPICKER_SHOW_PREVIEW;
            break;
        case CHECKBOX_SELECTION:
            resId = STR_SVT_FILEPICKER_SELECTION;
            break;
        case CHECKBOX_GPGENCRYPTION:
            resId = STR_SVT_FILEPICKER_GPGENCRYPT;
            break;
        case CHECKBOX_GPGSIGN:
            resId = STR_SVT_FILEPICKER_GPGSIGN;
            break;
        case PUSHBUTTON_PLAY:
            resId = STR_SVT_FILEPICKER_PLAY;
            break;
        case LISTBOX_VERSION:
            resId = STR_SVT_FILEPICKER_VERSION;
            break;
        case LISTBOX_TEMPLATE:
            resId = STR_SVT_FILEPICKER_TEMPLATES;
            break;
        case LISTBOX_IMAGE_TEMPLATE:
            resId = STR_SVT_FILEPICKER_IMAGE_TEMPLATE;
            break;
        case LISTBOX_IMAGE_ANCHOR:
            resId = STR_SVT_FILEPICKER_IMAGE_ANCHOR;
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
            pCheckbox = new QCheckBox(getResString(resId), m_pExtraControls);
            // to add/remove automatic file extension based on checkbox
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
            connect(pCheckbox, &QCheckBox::checkStateChanged, this,
                    &QtFilePicker::updateAutomaticFileExtension);
#else
            connect(pCheckbox, &QCheckBox::stateChanged, this,
                    &QtFilePicker::updateAutomaticFileExtension);
#endif
            widget = pCheckbox;
            break;
        case CHECKBOX_PASSWORD:
        case CHECKBOX_FILTEROPTIONS:
        case CHECKBOX_READONLY:
        case CHECKBOX_LINK:
        case CHECKBOX_PREVIEW:
        case CHECKBOX_SELECTION:
        case CHECKBOX_GPGENCRYPTION:
        case CHECKBOX_GPGSIGN:
            widget = new QCheckBox(getResString(resId), m_pExtraControls);
            break;
        case PUSHBUTTON_PLAY:
            break;
        case LISTBOX_VERSION:
        case LISTBOX_TEMPLATE:
        case LISTBOX_IMAGE_ANCHOR:
        case LISTBOX_IMAGE_TEMPLATE:
        case LISTBOX_FILTER_SELECTOR:
            label = new QLabel(getResString(resId), m_pExtraControls);
            widget = new QComboBox(m_pExtraControls);
            label->setBuddy(widget);
            break;
        case LISTBOX_VERSION_LABEL:
        case LISTBOX_TEMPLATE_LABEL:
        case LISTBOX_IMAGE_TEMPLATE_LABEL:
        case LISTBOX_IMAGE_ANCHOR_LABEL:
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

void SAL_CALL QtFilePicker::initialize(const uno::Sequence<uno::Any>& args)
{
    // parameter checking
    if (args.getLength() == 0)
        throw lang::IllegalArgumentException(u"no arguments"_ustr, static_cast<XFilePicker2*>(this),
                                             1);

    SolarMutexGuard g;
    QtInstance& rQtInstance = GetQtInstance();
    if (!rQtInstance.IsMainThread())
    {
        rQtInstance.RunInMainThread([this, args]() { initialize(args); });
        return;
    }

    m_aNamedFilterToExtensionMap.clear();
    m_aNamedFilterList.clear();
    m_aTitleToFilterMap.clear();
    m_aCurrentFilter.clear();

    // handle template which is set in XFilePicker3, but not XFolderPicker2 initialization
    uno::Any arg;
    arg = args[0];

    if ((arg.getValueType() == cppu::UnoType<sal_Int16>::get())
        || (arg.getValueType() == cppu::UnoType<sal_Int8>::get()))
    {
        sal_Int16 templateId = -1;
        arg >>= templateId;
        applyTemplate(templateId);
    }

    css::uno::Reference<css::awt::XWindow> xParentWindow;
    if (args.getLength() > 1)
        args[1] >>= xParentWindow;
    if (!xParentWindow.is())
        return;

    if (QtXWindow* pQtXWindow = dynamic_cast<QtXWindow*>(xParentWindow.get()))
    {
        m_pParentWidget = pQtXWindow->getQWidget();
        return;
    }

    UnoWrapperBase* pWrapper = UnoWrapperBase::GetUnoWrapper();
    if (!pWrapper)
        return;

    VclPtr<vcl::Window> xWindow = pWrapper->GetWindow(xParentWindow);
    if (!xWindow)
        return;

    if (QtFrame* pFrame = static_cast<QtFrame*>(xWindow->ImplGetFrame()))
        m_pParentWidget = pFrame->asChild();
}

void SAL_CALL QtFilePicker::cancel() { m_pFileDialog->reject(); }

void SAL_CALL QtFilePicker::disposing(const lang::EventObject& rEvent)
{
    uno::Reference<XFilePickerListener> xFilePickerListener(rEvent.Source, uno::UNO_QUERY);

    if (xFilePickerListener.is())
    {
        removeFilePickerListener(xFilePickerListener);
    }
}

void SAL_CALL QtFilePicker::queryTermination(const css::lang::EventObject&)
{
    throw css::frame::TerminationVetoException();
}

void SAL_CALL QtFilePicker::notifyTermination(const css::lang::EventObject&)
{
    SolarMutexGuard aGuard;
    m_pFileDialog->reject();
}

OUString SAL_CALL QtFilePicker::getImplementationName()
{
    return u"com.sun.star.ui.dialogs.QtFilePicker"_ustr;
}

sal_Bool SAL_CALL QtFilePicker::supportsService(const OUString& ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

uno::Sequence<OUString> SAL_CALL QtFilePicker::getSupportedServiceNames()
{
    return { u"com.sun.star.ui.dialogs.FilePicker"_ustr,
             u"com.sun.star.ui.dialogs.SystemFilePicker"_ustr,
             u"com.sun.star.ui.dialogs.QtFilePicker"_ustr };
}

void QtFilePicker::updateAutomaticFileExtension()
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
                "vcl.qt",
                "Unable to retrieve unambiguous file extension. Will not add any automatically.");
            bSetAutoExtension = false;
        }
    }

    if (!bSetAutoExtension)
        m_pFileDialog->setDefaultSuffix("");
}

void QtFilePicker::filterSelected(const QString&)
{
    FilePickerEvent aEvent;
    aEvent.ElementId = LISTBOX_FILTER;
    SAL_INFO("vcl.qt", "filter changed");
    if (m_xListener.is())
        m_xListener->controlStateChanged(aEvent);
}

void QtFilePicker::currentChanged(const QString&)
{
    FilePickerEvent aEvent;
    SAL_INFO("vcl.qt", "file selection changed");
    if (m_xListener.is())
        m_xListener->fileSelectionChanged(aEvent);
}

OUString QtFilePicker::getDirectory()
{
    uno::Sequence<OUString> seq = getSelectedFiles();
    if (seq.getLength() > 1)
        seq.realloc(1);
    return seq.hasElements() ? seq[0] : OUString();
}

void QtFilePicker::setDescription(const OUString&) {}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
