/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU General Public License Version 2.
 *
 *
 *    GNU General Public License, version 2
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This program is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU General Public License as
 *    published by the Free Software Foundation; either version 2 of
 *    the License, or (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public
 *    License along with this program; if not, write to the Free
 *    Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *    Boston, MA 02110-1301, USA.
 *
 ************************************************************************/

#include "pdfioutdev_gpl.hxx"
//#include "SecurityHandler.h"
#ifdef WNT
# include <io.h>
# include <fcntl.h>  /*_O_BINARY*/
#endif

FILE* g_binary_out=stderr;

#ifndef SYSTEM_POPPLER
static char ownerPassword[33] = "\001";
static char userPassword[33]  = "\001";
static char outputFile[256]   = "\001";

static ArgDesc argDesc[] = {
  {(char*)"-f",          argString,      outputFile,     sizeof(outputFile),
   (char*)"output file for binary streams"},
  {(char*)"-opw",        argString,      ownerPassword,  sizeof(ownerPassword),
   (char*)"owner password (for encrypted files)"},
  {(char*)"-upw",        argString,      userPassword,   sizeof(userPassword),
   (char*)"user password (for encrypted files)"},
  {NULL, argString, NULL, 0, NULL }
};
#else
static const char *ownerPassword = "\001";
static const char *userPassword  = "\001";
static const char *outputFile   = "\001";
#endif

int main(int argc, char **argv)
{
#ifndef SYSTEM_POPPLER
    // parse args; initialize to defaults
    if( !parseArgs(argDesc, &argc, argv) )
        return 1;
#else
    int k = 0;
    while (k < argc)
    {
        if (!strcmp(argv[k], "-f"))
        {
            outputFile = argv[k+1];
            --argc;
            for (int j = k; j < argc; ++j)
                argv[j] = argv[j+1];
        }
        else if (!strcmp(argv[k], "-opw"))
        {
            ownerPassword = argv[k+1];
            --argc;
            for (int j = k; j < argc; ++j)
                argv[j] = argv[j+1];
        }
        else if (!strcmp(argv[k], "-upw"))
        {
            userPassword = argv[k+1];
            --argc;
            for (int j = k; j < argc; ++j)
                argv[j] = argv[j+1];
        }
    ++k;
    }
#endif

    if( argc < 2 )
        return 1;

    // read config file
    globalParams = new GlobalParams(
#ifndef SYSTEM_POPPLER
        (char*)""
#endif
    );
    globalParams->setErrQuiet(gTrue);
#if !defined(SYSTEM_POPPLER) || defined(_MSC_VER)
    globalParams->setupBaseFonts(NULL);
#endif

    // try to read a possible open password form stdin
    char aPwBuf[129];
    aPwBuf[128] = 0;
    if( ! fgets( aPwBuf, sizeof(aPwBuf)-1, stdin ) )
        aPwBuf[0] = 0; // mark as empty
    else
    {
        for( unsigned int i = 0; i < sizeof(aPwBuf); i++ )
        {
            if( aPwBuf[i] == '\n' )
            {
                aPwBuf[i] = 0;
                break;
            }
        }
    }

    // PDFDoc takes over ownership for all strings below
    GooString* pFileName    = new GooString(argv[1]);
    GooString* pTempErrFileName     = new GooString("_err.pdf");
    GooString* pTempErrFileNamePath = new GooString(argv[0]);

    GooString* pErrFileName = new GooString(pTempErrFileNamePath,pTempErrFileName);


    // check for password string(s)
    GooString* pOwnerPasswordStr( aPwBuf[0] != 0
                                 ? new GooString( aPwBuf )
                                 : (ownerPassword[0] != '\001'
                                    ? new GooString(ownerPassword)
                                    : (GooString *)NULL ) );
    GooString* pUserPasswordStr(  userPassword[0] != '\001'
                                  ? new GooString(userPassword)
                                  : (GooString *)NULL );
    if( outputFile[0] != '\001' )
        g_binary_out = fopen(outputFile,"wb");

#ifdef WNT
    // Win actually modifies output for O_TEXT file mode, so need to
    // revert to binary here
    _setmode( _fileno( g_binary_out ), _O_BINARY );
#endif

    PDFDoc aDoc( pFileName,
                 pOwnerPasswordStr,
                 pUserPasswordStr );

    PDFDoc aErrDoc( pErrFileName,
                 pOwnerPasswordStr,
                 pUserPasswordStr );


   // Check various permissions.
   if ( !aDoc.isOk()||
        !aDoc.okToPrint() ||
        !aDoc.okToChange()||
        !aDoc.okToCopy()||
        !aDoc.okToAddNotes() )
   {
        pdfi::PDFOutDev* pOutDev( new pdfi::PDFOutDev(&aErrDoc) );

        const int nPages = aErrDoc.isOk() ? aErrDoc.getNumPages() : 0;

        // tell receiver early - needed for proper progress calculation
        pOutDev->setPageNum( nPages );

        // virtual resolution of the PDF OutputDev in dpi
        static const int PDFI_OUTDEV_RESOLUTION=7200;

       // do the conversion
       for( int i=1; i<=nPages; ++i )
       {
          aErrDoc.displayPage( pOutDev,
                            i,
                            PDFI_OUTDEV_RESOLUTION,
                            PDFI_OUTDEV_RESOLUTION,
                            0, gTrue, gTrue, gTrue );
          aErrDoc.processLinks( pOutDev, i );
       }
   }
   else
   {
      pdfi::PDFOutDev* pOutDev( new pdfi::PDFOutDev(&aDoc) );

      // tell receiver early - needed for proper progress calculation
      pOutDev->setPageNum( aDoc.getNumPages() );

      // virtual resolution of the PDF OutputDev in dpi
      static const int PDFI_OUTDEV_RESOLUTION=7200;

      // do the conversion
      const int nPages = aDoc.getNumPages();
      for( int i=1; i<=nPages; ++i )
      {
        aDoc.displayPage( pOutDev,
                          i,
                          PDFI_OUTDEV_RESOLUTION,
                          PDFI_OUTDEV_RESOLUTION,
                          0, gTrue, gTrue, gTrue );
        aDoc.processLinks( pOutDev, i );
      }
   }
    return 0;
}

