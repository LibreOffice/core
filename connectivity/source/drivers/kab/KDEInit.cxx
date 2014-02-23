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

#include "KDEInit.h"
#include <osl/diagnose.h>
#include <osl/process.h>

#include <shell/kde_headers.h>

namespace connectivity
{
    namespace kab
    {

        // = KDEInit

        class KDEInit
        {
        private:
            /// KDE application if we own it
            static KApplication*    s_pKApplication;
            static bool             s_bDidInsertCatalogue;

        public:
            static void Init();
            static void Shutdown();
        };


        KApplication* KDEInit::s_pKApplication = NULL;
        bool          KDEInit::s_bDidInsertCatalogue = false;


        void KDEInit::Init()
        {
            // TODO: All this is not thread-safe

            // we create a KDE application only if it is not already done
            if (KApplication::kApplication() == NULL)
            {
                OSL_ENSURE(s_pKApplication == NULL, "KDEInit::Init: inconsistency in the application pointers!");

                char *kabargs[1] = {(char*)"libkab1"};
                KCmdLineArgs::init(1, kabargs, "KAddressBook", *kabargs, "Address Book driver", KAB_DRIVER_VERSION);

                s_pKApplication = new KApplication(false, false);
            }

            /* FIXME-BCP47: what slumbering dogs may we wake up here? */
            // set language
            rtl_Locale *pProcessLocale;
            osl_getProcessLocale(&pProcessLocale);
            // sal_Unicode and QChar are (currently) both 16 bits characters
            QString aLanguage(
                (const QChar *) pProcessLocale->Language->buffer,
                (int) pProcessLocale->Language->length);
            KGlobal::locale()->setLanguage(aLanguage);

            // load KDE address book's localized messages
            KGlobal::locale()->insertCatalogue("kaddressbook");
            s_bDidInsertCatalogue = true;
        }


        void KDEInit::Shutdown()
        {
            if ( s_bDidInsertCatalogue )
                // this guard is necessary, since KDE 3.3 seems to crash if we remove a catalogue
                // which we did not previously insert
                KGlobal::locale()->removeCatalogue("kaddressbook");

            if ( s_pKApplication != NULL )
            {
                delete s_pKApplication;
                s_pKApplication = NULL;
            }
        }
    }
}


namespace
{
    double normalizeVersion( unsigned int major, unsigned int minor )
    {
        return major + 1.0 * minor / 1000;
    }
}


extern "C" SAL_DLLPUBLIC_EXPORT void SAL_CALL initKApplication()
{
    ::connectivity::kab::KDEInit::Init();
}


extern "C" SAL_DLLPUBLIC_EXPORT void SAL_CALL shutdownKApplication()
{
    ::connectivity::kab::KDEInit::Shutdown();
}

/** checks whether the KDE version on the system we're running at is supported
    by the driver

    Has to be called before any other code from this library, in particular,
    it has to be called before initKApplication()

    If this function returns <code>0</code>, then no other code from this library
    has to be called, else the results are unpredictable.

    @return
        <ul><li><code>0</code> if the KDE version is supportednon</li>
            <li>a negative value if the version is too old</li>
            <li>a positive value if the version is too new to know whether it works with this driver</li>
        </ul>

    #i60062# / 2006-01-06 / frank.schoenheit@sun.com
*/
extern "C" SAL_DLLPUBLIC_EXPORT int SAL_CALL matchKDEVersion()
{
    double nMinVersion = normalizeVersion( MIN_KDE_VERSION_MAJOR, MIN_KDE_VERSION_MINOR );
    double nCurVersion = normalizeVersion( ::KDE::versionMajor(), ::KDE::versionMinor() );
    double nMaxVersion = normalizeVersion( MAX_KDE_VERSION_MAJOR, MAX_KDE_VERSION_MINOR );

    if ( nCurVersion < nMinVersion )
        return -1;
    if ( nCurVersion > nMaxVersion )
        return 1;

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
