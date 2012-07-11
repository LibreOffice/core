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

#ifndef _COMDEP_HXX
#define _COMDEP_HXX

#include <tools/fsys.hxx>

#define ACCESSDELIM(e)  ( ( e == FSYS_STYLE_NTFS ) ? "\\" : "/" )
#define ACCESSDELIM_C(e)(char)\
                        ( ( e == FSYS_STYLE_NTFS ) ? '\\' : '/' )
#define SEARCHDELIM(e)  ( (e == FSYS_STYLE_BSD) ? ":" : ";" )
#define SEARCHDELIM_C(e)(char)\
                        ( (e == FSYS_STYLE_BSD) ? ':' : ';' )
#define ACTPARENT(e)    ".."
#define ACTCURRENT(e)   "."

#if defined UNX
#include "unx.hxx"
#elif defined WNT
#include "wntmsc.hxx"
#endif

//--------------------------------------------------------------------

#ifndef LINUX
DIR *opendir( const char* pPfad );
dirent *readdir( DIR *pDir );
int closedir( DIR *pDir );
char *volumeid( const char* pPfad );
#endif

//--------------------------------------------------------------------

struct DirReader_Impl
{
    Dir*        pDir;
    DIR*        pDosDir;
    dirent*     pDosEntry;
    DirEntry*   pParent;
    String      aPath;
    rtl::OString aBypass;
    sal_Bool        bReady;
    sal_Bool        bInUse;

                DirReader_Impl( Dir &rDir )
                :   pDir( &rDir ),
                    pDosEntry( 0 ),
                    pParent( 0 ),
                    aPath(rDir.GetFull()),
                    bReady ( sal_False ),
                    bInUse( sal_False )
                {

                    // nur den String der Memer-Var nehmen!

#if defined(UNX)    //for further eplanation see DirReader_Impl::Read() in unx.cxx
                    pDosDir = NULL;
#else
                    aBypass = rtl::OUStringToOString(aPath, osl_getThreadTextEncoding());
                    pDosDir = opendir( aBypass.getStr() );
#endif

                    // Parent f"ur die neuen DirEntries ermitteln
                    pParent = pDir->GetFlag() == FSYS_FLAG_NORMAL ||
                              pDir->GetFlag() == FSYS_FLAG_ABSROOT
                                    ? pDir
                                    : pDir->GetParent();

                }

                ~DirReader_Impl()
                { if( pDosDir ) closedir( pDosDir ); }

                // die folgenden sind systemabh"angig implementiert
    sal_uInt16      Init(); // initialisiert, liest ggf. devices
    sal_uInt16      Read(); // liest 1 Eintrag, F2ugt ein falls ok
};

//--------------------------------------------------------------------

struct FileCopier_Impl
{
    FSysAction      nActions;       // was zu tun ist (Copy/Move/recur)
    Link            aErrorLink;     // bei Fehlern zu rufen
    ErrCode         eErr;           // aktueller Fehlercode im Error-Handler
    const DirEntry* pErrSource;     // fuer Error-Handler falls Source-Fehler
    const DirEntry* pErrTarget;     // fuer Error-Handler falls Target-Fehler

                    FileCopier_Impl()
                    :   nActions( 0 ), eErr( 0 ),
                        pErrSource( 0 ), pErrTarget( 0 )
                    {}
                    FileCopier_Impl( const FileCopier_Impl &rOrig )
                    :   nActions( rOrig.nActions ), eErr( 0 ),
                        pErrSource( 0 ), pErrTarget( 0 )
                    {}

    FileCopier_Impl& operator=( const FileCopier_Impl &rOrig )
                    {
                        nActions = rOrig.nActions;
                        eErr = 0; pErrSource = 0; pErrTarget = 0;
                        return *this;
                    }
};

//--------------------------------------------------------------------

#if defined WNT
sal_Bool IsRedirectable_Impl( const rtl::OString &rPath );
#else
#define IsRedirectable_Impl( rPath )    sal_True
#endif

//--------------------------------------------------------------------


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
