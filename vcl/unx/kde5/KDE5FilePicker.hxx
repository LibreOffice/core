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

#pragma once

#include <Qt5FilePicker.hxx>

#include <cppuhelper/compbase.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/ui/dialogs/XFilePicker3.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#include <com/sun/star/ui/dialogs/XFolderPicker2.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <osl/conditn.hxx>
#include <rtl/ustrbuf.hxx>

#include <functional>

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QHash>
#include <QtWidgets/QFileDialog>

class QFileDialog;
class QGridLayout;
class QWidget;
class QComboBox;

class KDE5FilePicker : public Qt5FilePicker
{
    Q_OBJECT
protected:
    //layout for extra custom controls
    QGridLayout* _layout;

    bool allowRemoteUrls;

public:
    explicit KDE5FilePicker(QFileDialog::FileMode);

    // XExecutableDialog functions
    virtual sal_Int16 SAL_CALL execute() override;

    // XFilePickerControlAccess functions
    virtual void SAL_CALL setValue(sal_Int16 nControlId, sal_Int16 nControlAction,
                                   const css::uno::Any& rValue) override;
    virtual css::uno::Any SAL_CALL getValue(sal_Int16 nControlId,
                                            sal_Int16 nControlAction) override;
    virtual void SAL_CALL enableControl(sal_Int16 nControlId, sal_Bool bEnable) override;
    virtual void SAL_CALL setLabel(sal_Int16 nControlId, const OUString& rLabel) override;
    virtual OUString SAL_CALL getLabel(sal_Int16 nControlId) override;

    /* TODO XFilePreview

    virtual css::uno::Sequence< sal_Int16 > SAL_CALL getSupportedImageFormats(  );
    virtual sal_Int32 SAL_CALL  getTargetColorDepth(  );
    virtual sal_Int32 SAL_CALL  getAvailableWidth(  );
    virtual sal_Int32 SAL_CALL  getAvailableHeight(  );
    virtual void SAL_CALL       setImage( sal_Int16 aImageFormat, const css::uno::Any &rImage );
    virtual sal_Bool SAL_CALL   setShowState( sal_Bool bShowState );
    virtual sal_Bool SAL_CALL   getShowState(  );
    */

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& rServiceName) override;
    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

private:
    //add a custom control widget to the file dialog
    void addCustomControl(sal_Int16 controlId) override;

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
