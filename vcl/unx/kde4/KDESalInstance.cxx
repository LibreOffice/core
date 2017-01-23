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

#include "KDE4FilePicker.hxx"
#include "KDESalInstance.hxx"

#include "KDESalFrame.hxx"

#include "KDEXLib.hxx"
#include "KDESalDisplay.hxx"

#include <QtGui/QApplication>
#include <QtCore/QThread>
#include <QX11Info>

using namespace com::sun::star;

KDESalInstance::KDESalInstance(SalYieldMutex* pMutex)
    : X11SalInstance(pMutex)
{
    ImplSVData* pSVData = ImplGetSVData();
    delete pSVData->maAppData.mpToolkitName;
    pSVData->maAppData.mpToolkitName = new OUString("kde4");
}

SalFrame* KDESalInstance::CreateFrame( SalFrame *pParent, SalFrameStyleFlags nState )
{
    return new KDESalFrame( pParent, nState );
}

uno::Reference< ui::dialogs::XFilePicker2 > KDESalInstance::createFilePicker(
    const uno::Reference< uno::XComponentContext >& xMSF )
{
    KDEXLib* kdeXLib = static_cast<KDEXLib*>( mpXLib );
    if (kdeXLib->allowKdeDialogs())
        return uno::Reference< ui::dialogs::XFilePicker2 >(
            kdeXLib->createFilePicker(xMSF) );
    else
        return X11SalInstance::createFilePicker( xMSF );
}

SalX11Display* KDESalInstance::CreateDisplay() const
{
    return new SalKDEDisplay( QX11Info::display() );
}

bool KDESalInstance::IsMainThread() const
{
    return qApp->thread() == QThread::currentThread();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
