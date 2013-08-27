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

#include <cstddef>

#include <kdemodalityfilter.hxx>
#include <kdefilepicker.hxx>
#include <kdecommandthread.hxx>

#if ENABLE_TDE
#include <tqeventloop.h>
#include <tdeaboutdata.h>
#include <tdeapplication.h>
#include <tdecmdlineargs.h>
#else // ENABLE_TDE
#include <kaboutdata.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#endif // ENABLE_TDE

#include <iostream>
#include <stdlib.h>

#include <config_vclplug.h>

#if ENABLE_TDE
#define THIS_DESKENV_NAME_CAP "TDE"
#define THIS_DESKENV_NAME_LOW "tde"
#else // ENABLE_TDE
#define THIS_DESKENV_NAME_CAP "KDE"
#define THIS_DESKENV_NAME_LOW "kde"
#endif // ENABLE_TDE

#if ENABLE_TDE
#define KAboutData TDEAboutData
#define KCmdLineArgs TDECmdLineArgs
#define KCmdLineOptions TDECmdLineOptions
#define KCmdLineLastOption TDECmdLineLastOption
#define KApplication TDEApplication
#define KLocale TDELocale
#endif // ENABLE_TDE

//////////////////////////////////////////////////////////////////////////
// Main
//////////////////////////////////////////////////////////////////////////

static const KCmdLineOptions sOptions[] =
{
    { "winid <argument>", I18N_NOOP("Window ID to which is the fpicker modal"), "0" },
    KCmdLineLastOption
};

int main( int argc, char* argv[] )
{
    // we fake the name of the application to have "LibreOffice" in the
    // title
    KAboutData qAboutData( "kdefilepicker", I18N_NOOP( "LibreOffice" ),
            "0.1", I18N_NOOP( "kdefilepicker is an implementation of the " THIS_DESKENV_NAME_CAP " file dialog for LibreOffice." ),
            KAboutData::License_LGPL,
            "(c) 2004, Jan Holesovsky" );
    qAboutData.addAuthor( "Jan Holesovsky", I18N_NOOP("Original author and current maintainer"), "kendy@openoffice.org" );

    // Let the user see that this does something...
    ::std::cerr << "kdefilepicker, an implementation of a " THIS_DESKENV_NAME_CAP " file dialog for OOo." << ::std::endl
        << "Type 'exit' and press Enter to finish." << ::std::endl;

    KCmdLineArgs::addCmdLineOptions( sOptions );
    KCmdLineArgs::init( argc, argv, &qAboutData );

    KLocale::setMainCatalogue( "kdialog" );

    KApplication kApplication;

    // Setup the modality
    KCmdLineArgs *pArgs = KCmdLineArgs::parsedArgs();
    long nWinId = atol( pArgs->getOption( "winid" ) );
    pArgs->clear();

    KDEModalityFilter qModalityFilter( nWinId );

    KDEFileDialog aFileDialog( NULL, QString(), NULL, THIS_DESKENV_NAME_LOW "filedialog" );

    KDECommandThread qCommandThread( &aFileDialog );
    qCommandThread.start();

    kApplication.exec();

    qCommandThread.wait();

    ::std::cout << "exited" << ::std::endl;

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
