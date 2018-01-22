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

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QHash>

#include <sal/types.h>

class QFileDialog;
class QWidget;
class QFormLayout;

class KDE5FilePicker : public QObject
{
    Q_OBJECT
protected:
    //the dialog to display
    QFileDialog* _dialog;

    //running filter string to add to dialog
    QStringList _filters;
    // map of filter titles to full filter for selection
    QHash<QString, QString> _titleToFilters;
    // string to set the current filter
    QString _currentFilter;

    //mapping of SAL control ID's to created custom controls
    QHash<sal_Int16, QWidget*> _customWidgets;

    //widget to contain extra custom controls
    QWidget* _extraControls;

    //layout for extra custom controls
    QFormLayout* _layout;

    sal_uIntPtr _winId;

    bool allowRemoteUrls;

public:
    explicit KDE5FilePicker(QObject* parent = nullptr);
    ~KDE5FilePicker();

    void enableFolderMode();

    // XExecutableDialog functions
    void SAL_CALL setTitle(const QString& rTitle);
    bool SAL_CALL execute();

    // XFilePicker functions
    void SAL_CALL setMultiSelectionMode(bool bMode);
    void SAL_CALL setDefaultName(const QString& rName);
    void SAL_CALL setDisplayDirectory(const QString& rDirectory);
    QString SAL_CALL getDisplayDirectory() const;

    // XFilterManager functions
    void SAL_CALL appendFilter(const QString& rTitle, const QString& rFilter);
    void SAL_CALL setCurrentFilter(const QString& rTitle);
    QString SAL_CALL getCurrentFilter() const;

    // XFilePickerControlAccess functions
    void SAL_CALL setValue(sal_Int16 nControlId, sal_Int16 nControlAction, bool rValue);
    bool SAL_CALL getValue(sal_Int16 nControlId, sal_Int16 nControlAction) const;
    void SAL_CALL enableControl(sal_Int16 nControlId, sal_Bool bEnable);
    void SAL_CALL setLabel(sal_Int16 nControlId, const QString& rLabel);
    QString SAL_CALL getLabel(sal_Int16 nControlId) const;

    // XFilePicker2 functions
    QList<QUrl> getSelectedFiles() const;

    // XInitialization
    void initialize(bool saveDialog);

    //add a custom control widget to the file dialog
    void addCheckBox(sal_Int16 nControlId, const QString& label, bool hidden);

    void setWinId(sal_uIntPtr winId);

private:
    Q_DISABLE_COPY(KDE5FilePicker)

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

Q_SIGNALS:
    void filterChanged();
    void selectionChanged();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
