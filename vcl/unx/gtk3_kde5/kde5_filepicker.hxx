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
class QGridLayout;

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
    QGridLayout* _layout;

    sal_uIntPtr _winId;

    bool allowRemoteUrls;

public:
    explicit KDE5FilePicker(QObject* parent = nullptr);
    ~KDE5FilePicker() override;

    void enableFolderMode();

    // XExecutableDialog functions
    void setTitle(const QString& rTitle);
    bool execute();

    // XFilePicker functions
    void setMultiSelectionMode(bool bMode);
    void setDefaultName(const QString& rName);
    void setDisplayDirectory(const QString& rDirectory);
    QString getDisplayDirectory() const;

    // XFilterManager functions
    void appendFilter(const QString& rTitle, const QString& rFilter);
    void setCurrentFilter(const QString& rTitle);
    QString getCurrentFilter() const;

    // XFilePickerControlAccess functions
    void setValue(sal_Int16 nControlId, sal_Int16 nControlAction, bool rValue);
    bool getValue(sal_Int16 nControlId, sal_Int16 nControlAction) const;
    void enableControl(sal_Int16 nControlId, bool bEnable);
    void setLabel(sal_Int16 nControlId, const QString& rLabel);
    QString getLabel(sal_Int16 nControlId) const;

    // XFilePicker2 functions
    QList<QUrl> getSelectedFiles() const;

    // XInitialization
    void initialize(bool saveDialog);

    //add a custom control widget to the file dialog
    void addCheckBox(sal_Int16 nControlId, const QString& label, bool hidden);

    void setWinId(sal_uIntPtr winId);

private:
    Q_DISABLE_COPY(KDE5FilePicker)
    // adds the custom controls to the dialog
    void setupCustomWidgets();

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

Q_SIGNALS:
    void filterChanged();
    void selectionChanged();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
