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

#include "KFFilePicker.hxx"
#include <KFFilePicker.moc>

#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <osl/mutex.hxx>

#include <QtInstance.hxx>

#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QWidget>
#include <KFileWidget>

using namespace ::com::sun::star;
using ::com::sun::star::ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_AUTOEXTENSION;

namespace
{
uno::Sequence<OUString> FilePicker_getSupportedServiceNames()
{
    return { u"com.sun.star.ui.dialogs.FilePicker"_ustr,
             u"com.sun.star.ui.dialogs.SystemFilePicker"_ustr,
             u"com.sun.star.ui.dialogs.KFFilePicker"_ustr,
             u"com.sun.star.ui.dialogs.KFFolderPicker"_ustr };
}
}

// KFFilePicker

KFFilePicker::KFFilePicker(css::uno::Reference<css::uno::XComponentContext> const& context,
                           QFileDialog::FileMode eMode)
    // Native KF5/KF6 filepicker does not add file extension automatically
    : QtFilePicker(context, eMode, true)
    , _layout(new QGridLayout(m_pExtraControls))
{
    // only columns 0 and 1 are used by controls (s. QtFilePicker::addCustomControl);
    // set stretch for (unused) column 2 in order for the controls to only take the space
    // they actually need and avoid empty space in between
    _layout->setColumnStretch(2, 1);

    // set layout so custom widgets show up in our native file dialog
    setCustomControlWidgetLayout(_layout.get());

    m_pFileDialog->setSupportedSchemes({
        QStringLiteral("file"), QStringLiteral("http"), QStringLiteral("https"),
        QStringLiteral("webdav"), QStringLiteral("webdavs"), QStringLiteral("smb"),
        QStringLiteral(""), // this makes removable devices shown
    });

    // used to set the custom controls
    qApp->installEventFilter(this);
}

// XFilePickerControlAccess
void SAL_CALL KFFilePicker::setValue(sal_Int16 controlId, sal_Int16 nControlAction,
                                     const uno::Any& value)
{
    if (CHECKBOX_AUTOEXTENSION == controlId)
        // We ignore this one and rely on QFileDialog to provide the functionality
        return;

    QtFilePicker::setValue(controlId, nControlAction, value);
}

uno::Any SAL_CALL KFFilePicker::getValue(sal_Int16 controlId, sal_Int16 nControlAction)
{
    SolarMutexGuard g;
    auto* pSalInst(GetQtInstance());
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

    return QtFilePicker::getValue(controlId, nControlAction);
}

void SAL_CALL KFFilePicker::enableControl(sal_Int16 controlId, sal_Bool enable)
{
    if (CHECKBOX_AUTOEXTENSION == controlId)
        // We ignore this one and rely on QFileDialog to provide the functionality
        return;

    QtFilePicker::enableControl(controlId, enable);
}

void SAL_CALL KFFilePicker::setLabel(sal_Int16 controlId, const OUString& label)
{
    if (CHECKBOX_AUTOEXTENSION == controlId)
        // We ignore this one and rely on QFileDialog to provide the functionality
        return;

    QtFilePicker::setLabel(controlId, label);
}

OUString SAL_CALL KFFilePicker::getLabel(sal_Int16 controlId)
{
    // We ignore this one and rely on QFileDialog to provide the functionality
    if (CHECKBOX_AUTOEXTENSION == controlId)
        return u""_ustr;

    return QtFilePicker::getLabel(controlId);
}

void KFFilePicker::addCustomControl(sal_Int16 controlId)
{
    // native KF5/KF6 filepicker has its own autoextension checkbox,
    // therefore avoid adding yet another one
    if (controlId == CHECKBOX_AUTOEXTENSION)
        return;

    QtFilePicker::addCustomControl(controlId);
}

// XServiceInfo
OUString SAL_CALL KFFilePicker::getImplementationName()
{
    return u"com.sun.star.ui.dialogs.KFFilePicker"_ustr;
}

sal_Bool SAL_CALL KFFilePicker::supportsService(const OUString& ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

uno::Sequence<OUString> SAL_CALL KFFilePicker::getSupportedServiceNames()
{
    return FilePicker_getSupportedServiceNames();
}

bool KFFilePicker::eventFilter(QObject* o, QEvent* e)
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
