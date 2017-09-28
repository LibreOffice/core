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

#include <QtWidgets/QStyle>
#include <QtWidgets/QApplication>

#undef Region

#include "KDE5Data.hxx"

#include "KDE5XLib.hxx"
#include "KDE5SalDisplay.hxx"

KDE5Data::~KDE5Data()
{
}

void KDE5Data::Init()
{
    pXLib_ = new KDE5XLib();
    pXLib_->Init();
    //SetDisplay( SalKDE5Display::self() );

    int argc = 1;
    qMyApp = new QApplication(argc, nullptr);
}

void KDE5Data::initNWF()
{
    ImplSVData *pSVData = ImplGetSVData();

    // draw toolbars on separate lines
    pSVData->maNWFData.mbDockingAreaSeparateTB = true;
    // no borders for menu, theming does that
    pSVData->maNWFData.mbFlatMenu = true;
    // Qt theme engines may support a rollover menubar
    pSVData->maNWFData.mbRolloverMenubar = true;

    pSVData->maNWFData.mbNoFocusRects = true;

    // Styled menus need additional space
    //QStyle *style = QApplication::style();
    QStyle *style = qMyApp->style();
    pSVData->maNWFData.mnMenuFormatBorderX =
       style->pixelMetric( QStyle::PM_MenuPanelWidth ) +
       style->pixelMetric( QStyle::PM_MenuHMargin );
    pSVData->maNWFData.mnMenuFormatBorderY =
       style->pixelMetric( QStyle::PM_MenuPanelWidth ) +
       style->pixelMetric( QStyle::PM_MenuVMargin );
}

void KDE5Data::deInitNWF()
{
    delete qMyApp;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
