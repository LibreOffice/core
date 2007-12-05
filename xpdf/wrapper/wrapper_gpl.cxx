/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: wrapper_gpl.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: thb $ $Date: 2007-12-05 14:16:44 $
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
#ifdef WNT
# include <io.h>
# include <fcntl.h>  /*_O_BINARY*/
#endif

static char ownerPassword[33] = "\001";
static char userPassword[33]  = "\001";
static char outputFile[256]   = "\001";
static char cfgFileName[256]  = "";

FILE* g_binary_out=stderr;

static ArgDesc argDesc[] = {
  {"-f",          argString,      outputFile,     sizeof(outputFile),
   "output file for binary streams"},
  {"-opw",        argString,      ownerPassword,  sizeof(ownerPassword),
   "owner password (for encrypted files)"},
  {"-upw",        argString,      userPassword,   sizeof(userPassword),
   "user password (for encrypted files)"},
  {NULL, argString, NULL, 0, NULL }
};

int main(int argc, char **argv)
{
    // parse args; initialize to defaults
    if( !parseArgs(argDesc, &argc, argv) )
        return 1;

    if( argc < 2 )
        return 1;

    // read config file
    globalParams = new GlobalParams(cfgFileName);
    globalParams->setErrQuiet(gTrue);
    globalParams->setupBaseFonts(NULL);

    // PDFDoc takes over ownership for all strings below
    GString* pFileName = new GString(argv[1]);

    // check for password string(s)
    GString* pOwnerPasswordStr(
        ownerPassword[0] != '\001' ? new GString(ownerPassword)
        : (GString *)NULL );
    GString* pUserPasswordStr(
        userPassword[0] != '\001' ? new GString(userPassword)
        : (GString *)NULL );
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

    if( !aDoc.isOk() )
        return 1;

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

    return 0;
}

