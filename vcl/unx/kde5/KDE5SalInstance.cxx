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

//#include "KDE4FilePicker.hxx"
#include <QtCore/QAbstractEventDispatcher>
#include <QtCore/QThread>
#include <QtGui/QClipboard>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFrame>
#include <QtX11Extras/QX11Info>


#include "KDE5SalInstance.hxx"
#include "KDE5SalFrame.hxx"
#include "KDE5XLib.hxx"
#include "KDE5SalDisplay.hxx"

using namespace com::sun::star;

KDE5SalInstance::KDE5SalInstance(SalYieldMutex* pMutex)
    :SvpSalInstance( pMutex )
{
    ImplSVData* pSVData = ImplGetSVData();
    delete pSVData->maAppData.mpToolkitName;
    pSVData->maAppData.mpToolkitName = new OUString("kde5");
}

SalFrame* KDE5SalInstance::CreateFrame( SalFrame *pParent, SalFrameStyleFlags nState )
{
    return new KDE5SalFrame( pParent, nState );
}

uno::Reference< ui::dialogs::XFilePicker2 > KDE5SalInstance::createFilePicker(
    const uno::Reference< uno::XComponentContext >& xMSF )
{
    /*KDEXLib* kdeXLib = static_cast<KDEXLib*>( mpXLib );
    if (kdeXLib->allowKdeDialogs())
        return uno::Reference< ui::dialogs::XFilePicker2 >(
            kdeXLib->createFilePicker(xMSF) );
    else*/
        return SalInstance::createFilePicker( xMSF );
}

/*SalX11Display* KDE5SalInstance::CreateDisplay() const
{
    return new SalKDE5Display( QX11Info::display() );
}*/

bool KDE5SalInstance::IsMainThread() const
{
    return qApp->thread() == QThread::currentThread();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
