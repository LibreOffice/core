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
#include <qt5/Qt5Instance.hxx>

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
    // Native kde5 filepicker does not add file extension automatically
    : Qt5FilePicker(eMode, true, true)
    , _layout(new QGridLayout(m_pExtraControls))
    , allowRemoteUrls(false)
{
    // use native dialog
    m_pFileDialog->setOption(QFileDialog::DontUseNativeDialog, false);

    // only columns 0 and 1 are used by controls (s. Qt5FilePicker::addCustomControl);
    // set stretch for (unused) column 2 in order for the controls to only take the space
    // they actually need and avoid empty space in between
    _layout->setColumnStretch(2, 1);

    // set layout so custom widgets show up in our native file dialog
    setCustomControlWidgetLayout(_layout);

    m_pFileDialog->setSupportedSchemes({
        QStringLiteral("file"),
        QStringLiteral("ftp"),
        QStringLiteral("http"),
        QStringLiteral("https"),
        QStringLiteral("webdav"),
        QStringLiteral("webdavs"),
        QStringLiteral("smb"),
    });

    // used to set the custom controls
    qApp->installEventFilter(this);
}

sal_Int16 SAL_CALL KDE5FilePicker::execute()
{
    SolarMutexGuard g;
    auto* pSalInst(static_cast<Qt5Instance*>(GetSalData()->m_pInstance));
    assert(pSalInst);
    if (!pSalInst->IsMainThread())
    {
        sal_Int16 ret;
        pSalInst->RunInMainThread([&ret, this] { ret = execute(); });
        return ret;
    }

    if (!m_aNamedFilterList.isEmpty())
        m_pFileDialog->setNameFilters(m_aNamedFilterList);
    if (!m_aCurrentFilter.isEmpty())
        m_pFileDialog->selectNameFilter(m_aCurrentFilter);

    m_pFileDialog->show();
    //block and wait for user input
    return m_pFileDialog->exec() == QFileDialog::Accepted ? 1 : 0;
}

// XFilePickerControlAccess
void SAL_CALL KDE5FilePicker::setValue(sal_Int16 controlId, sal_Int16 nControlAction,
                                       const uno::Any& value)
{
    if (CHECKBOX_AUTOEXTENSION == controlId)
        // We ignore this one and rely on QFileDialog to provide the functionality
        return;

    Qt5FilePicker::setValue(controlId, nControlAction, value);
}

uno::Any SAL_CALL KDE5FilePicker::getValue(sal_Int16 controlId, sal_Int16 nControlAction)
{
    SolarMutexGuard g;
    auto* pSalInst(static_cast<Qt5Instance*>(GetSalData()->m_pInstance));
    assert(pSalInst);
    if (!pSalInst->IsMainThread())
    {
        uno::Any ret;
        pSalInst->RunInMainThread([&ret, this, controlId, nControlAction]() {
            ret = getValue(controlId, nControlAction);
        });
        return ret;
    }

    if (CHECKBOX_AUTOEXTENSION == controlId)
        // We ignore this one and rely on QFileDialog to provide the function.
        // Always return false, to pretend we do not support this, otherwise
        // LO core would try to be smart and cut the extension in some places,
        // interfering with QFileDialog's handling of it. QFileDialog also
        // saves the value of the setting, so LO core is not needed for that either.
        return uno::Any(false);

    return Qt5FilePicker::getValue(controlId, nControlAction);
}

void SAL_CALL KDE5FilePicker::enableControl(sal_Int16 controlId, sal_Bool enable)
{
    if (CHECKBOX_AUTOEXTENSION == controlId)
        // We ignore this one and rely on QFileDialog to provide the functionality
        return;

    Qt5FilePicker::enableControl(controlId, enable);
}

void SAL_CALL KDE5FilePicker::setLabel(sal_Int16 controlId, const OUString& label)
{
    if (CHECKBOX_AUTOEXTENSION == controlId)
        // We ignore this one and rely on QFileDialog to provide the functionality
        return;

    Qt5FilePicker::setLabel(controlId, label);
}

OUString SAL_CALL KDE5FilePicker::getLabel(sal_Int16 controlId)
{
    // We ignore this one and rely on QFileDialog to provide the functionality
    if (CHECKBOX_AUTOEXTENSION == controlId)
        return "";

    return Qt5FilePicker::getLabel(controlId);
}

void KDE5FilePicker::addCustomControl(sal_Int16 controlId)
{
    // native kde5 filepicker has its own autoextension checkbox,
    // therefore avoid adding yet another one
    if (controlId == CHECKBOX_AUTOEXTENSION)
        return;

    Qt5FilePicker::addCustomControl(controlId);
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
                fileWidget->setCustomWidget(m_pExtraControls);
                // remove event filter again; the only purpose was to set the custom widget here
                qApp->removeEventFilter(this);
            }
        }
    }
    return QObject::eventFilter(o, e);
}

#include <KDE5FilePicker.moc>

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
