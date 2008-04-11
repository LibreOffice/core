/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: xsalprn.h,v $
 * $Revision: 1.4 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef __salprint_h
#define __salprint_h


class String;

#ifdef __cplusplus
extern "C" {
#endif

/* printer interface */
extern int XSalIsDisplay( const Display * );
extern int XSalIsPrinter( const Display * );

/* error handling */
typedef int (*XSalPrinterErrorHandler)( XErrorEvent * );

extern XSalPrinterErrorHandler XSalSetPrinterErrorHandler( XSalPrinterErrorHandler );

/* common callbacks */
typedef void* XSalPointer;
typedef int (*XSalPrinterCallback)( XSalPointer cb_data, XSalPointer client_data );

#ifndef _SV_SV_H
#define _SV_SV_H
#define USHORT unsigned short
#include <vcl/prntypes.hxx>
#undef USHORT
#undef _SV_SV_H
#else
#include <vcl/prntypes.hxx>
#endif

typedef enum Orientation Orientation;

/* initialize before first use */
extern void XSalPrinterInit( const String& installPath );

typedef unsigned short XSalEnum;
typedef unsigned char  XSalBool;

#define COLOR_SPACE_COLOR   1
#define COLOR_SPACE_GRAY    0

typedef struct
{
    unsigned int    nMagic;             /* internal data                */
    unsigned short  nVersion;           /* internal data                */
    unsigned short  nFlags;             /* internal data                */
    XSalEnum        eDriver;            /* PostScript, PCL, ...         */
    unsigned short  nCopies;            /* number of copies             */
    unsigned short  nPaperBin;          /* paper bin to use             */
    XSalEnum        ePaper;             /* A4, A5, ...                  */
    unsigned int    nPaperWidth;        /* paper width if PAPER_USER    */
    unsigned int    nPaperHeight;       /* paper height if PAPER_USER   */
    XSalEnum        eOrientation;       /* portrait / landscape         */
    unsigned int    nScale;             /* [%] ( 100 => 1:1 )           */
    unsigned short  nResolutionX;       /* [dots per inch]              */
    unsigned short  nResolutionY;       /* [dots per inch]              */
    char            sCompatCommand[256];/* current shell command        */
    char            sPort[256];         /* default shell command        */
    char            cDriverName[32];    /* Druckertreibername           */
    unsigned int    nTrailingBytes;     /* trailing bytes appended to this structure */
} XSalPrinterSetup;

#define XSAL_PRINTER_SETUP_MAGIC    0x0000ede1
#define WRONG_ENDIANESS_MAGIC       0xe1ed0000

/* definition for XSalPrinterSetup.nFlags */
#define XSALPRINTERSETUP_FLAG_LEVEL_DEFAULT     0x0000
#define XSALPRINTERSETUP_FLAG_LEVEL             0x000f
#define XSALPRINTERSETUP_FLAG_LEVEL_SHIFT       0
#define XSALPRINTERSETUP_FLAG_COLOR             0x0010  /* colored bitmaps  */
#define XSALPRINTERSETUP_FLAG_COLOR_DEFAULT     0x0020
#define XSALPRINTERSETUP_FLAG_COMPRESS          0x0040  /* compress bitmaps */
#define XSALPRINTERSETUP_FLAG_COMPRESS_DEFAULT  0x0080  /* compress bitmaps */
#define XSALPRINTERSETUP_FLAG_DEPTH_DEFAULT     0x0700
#define XSALPRINTERSETUP_FLAG_DEPTH             0x0700  /* depth n = depth 2^n, 6 = 24Bit, 7 = default */
#define XSALPRINTERSETUP_FLAG_DEPTH_SHIFT       8

#define XSALPRINTERSETUP_FLAG_DEFAULT\
                                (XSALPRINTERSETUP_FLAG_LEVEL_DEFAULT | \
                                 XSALPRINTERSETUP_FLAG_COMPRESS_DEFAULT | \
                                 XSALPRINTERSETUP_FLAG_COLOR_DEFAULT | \
                                 XSALPRINTERSETUP_FLAG_DEPTH_DEFAULT )


/* XSalPrinterSetup access */
extern unsigned short XSalGetPrinterDriverId( const char* driverName );
extern const char* XSalGetPrinterDriverName( unsigned short driverId );

extern unsigned short XSalGetLanguageLevel( const XSalPrinterSetup* pSetup );
extern void XSalGetLanguageLevels(
    const XSalPrinterSetup* pSetup,
    unsigned short* minLevel,
    unsigned short* maxLevel );
extern void XSalSetLanguageLevel( XSalPrinterSetup* pSetup, unsigned short);

extern unsigned short XSalGetDepth(  const XSalPrinterSetup* pSetup );
extern void XSalSetDepth( XSalPrinterSetup* pSetup, unsigned short depth );

extern XSalEnum XSalGetColorSpace( const XSalPrinterSetup* pSetup );
extern void XSalSetColorSpace( XSalPrinterSetup* pSetup, XSalEnum space );

extern XSalBool XSalGetBmpCompression( const XSalPrinterSetup* pSetup );
extern void XSalSetBmpCompression( XSalPrinterSetup* pSetup, XSalBool compress );

extern XSalEnum XSalGetOrientation( const char* string );
extern const char* XSalGetOrientationName( XSalEnum eOrientation );

extern XSalEnum XSalGetPaper( const char* sPaperName );
extern const char* XSalGetPaperName( XSalEnum ePaper );

/* use XSalInitPrinterSetup to initialize internal data */
extern void XSalInitPrinterSetup( XSalPrinterSetup* );
extern void XSalCorrectEndianess( XSalPrinterSetup* );
extern void XSalSetupPrinterSetup( XSalPrinterSetup*, Display* display, XLIB_Window parent);


/* the following two functions set defaults of the profile */
extern void XSalReadPrinterSetup( XSalPrinterSetup*, const String& rPrinter );
extern void XSalReadPrinterSetupDefaults( XSalPrinterSetup* );


typedef Display XSalPrinter;    /* an XSalPrinter is a Display. Draw into RootWindow */


/* open, change setup and close printer */
extern XSalPrinter* XSalOpenPrinter( const XSalPrinterSetup * pSetup, const String& rPrinterName, const String& rPrintFile );
/* XSalSetupPrinter() can setup: Orientation, Copies, Page, PaperBin */
extern void XSalSetupPrinter( XSalPrinter *, const XSalPrinterSetup * pSetup );
extern void XSalClosePrinter( XSalPrinter * );

typedef struct
{
  int nWidth;           /* [dots] drawable area */
  int nHeight;          /* [dots] drawable area */
  int nMarginLeft;      /* [dots] left margin   */
  int nMarginTop;       /* [dots] top margin    */
  int nMarginRight;     /* [dots] right margin  */
  int nMarginBottom;    /* [dots] bottom margin */
  int nResolutionX;     /* [dpi] resolution x   */
  int nResolutionY;     /* [dpi] resolution y   */
} XSalPageInfo;

extern void XSalGetPageInfo(
    const XSalPrinter* printer,
    const XSalPrinterSetup* pSetup,
    XSalPageInfo* pPageInfo );


/* printer job control */
extern int XSalStartDoc( XSalPrinter * printer, const String& jobName );
extern int XSalStartPage( XSalPrinter * printer );
extern int XSalEndPage( XSalPrinter * printer );
extern int XSalEndDoc( XSalPrinter * printer );
extern int XSalAbortDoc( XSalPrinter * printer );

#ifdef __cplusplus
} // extern "C"
#endif

#endif
