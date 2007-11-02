/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: comdep.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 13:01:41 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _COMDEP_HXX
#define _COMDEP_HXX

#include <tools/fsys.hxx>

#define ACCESSDELIM(e)  ( (e == FSYS_STYLE_MAC) ? ":" : \
                        ( ( e == FSYS_STYLE_VFAT || e == FSYS_STYLE_HPFS || \
                            e == FSYS_STYLE_FAT ) || e == FSYS_STYLE_NTFS ) \
                          ? "\\" : "/" )
#define ACCESSDELIM_C(e)(char)\
                        ( (e == FSYS_STYLE_MAC) ? ':' : \
                        ( ( e == FSYS_STYLE_VFAT || e == FSYS_STYLE_HPFS || \
                            e == FSYS_STYLE_FAT ) || e == FSYS_STYLE_NTFS ) \
                          ? '\\' : '/' )
#define SEARCHDELIM(e)  ( (e == FSYS_STYLE_SYSV || e == FSYS_STYLE_BSD) ? ":" \
                        : ";" )
#define SEARCHDELIM_C(e)(char)\
                        ( (e == FSYS_STYLE_SYSV || e == FSYS_STYLE_BSD) ? ':' \
                        : ';' )
#define ACTPARENT(e)    ( (e == FSYS_STYLE_MAC) ? ":" : ".." )
#define ACTCURRENT(e)   ( (e == FSYS_STYLE_MAC) ? "" : "." )

#if defined UNX
#include "unx.hxx"
#elif defined WNT
#include "wntmsc.hxx"
#elif defined OS2
#include "os2.hxx"
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
    ByteString  aBypass;
    BOOL        bReady;
    BOOL        bInUse;

                DirReader_Impl( Dir &rDir )
                :   pDir( &rDir ),
                    pDosEntry( 0 ),
                    pParent( 0 ),
                    aPath( GUI2FSYS(rDir.GetFull()) ),
                    bReady ( FALSE ),
                    bInUse( FALSE )
                {
#ifndef BOOTSTRAP
                    // Redirection
                    FSysRedirector::DoRedirect( aPath );
#endif

                    // nur den String der Memer-Var nehmen!

#if defined(UNX) || defined(OS2) //for further exlpanation see DirReader_Impl::Read() in unx.cxx
                    pDosDir = NULL;
#else
                    aBypass = ByteString(aPath, osl_getThreadTextEncoding());
                    pDosDir = opendir( (char*) aBypass.GetBuffer() );
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
    USHORT      Init(); // initialisiert, liest ggf. devices
    USHORT      Read(); // liest 1 Eintrag, F2ugt ein falls ok
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

#if defined WNT || defined OS2
BOOL IsRedirectable_Impl( const ByteString &rPath );
#else
#define IsRedirectable_Impl( rPath )    TRUE
#endif

//--------------------------------------------------------------------


#endif
