/*************************************************************************
 *
 *  $RCSfile: filter.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: dr $ $Date: 2001-04-05 10:54:23 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef SC_FILTER_HXX
#define SC_FILTER_HXX

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif
#ifndef _RTL_TEXTENC_H
#include <rtl/textenc.h>
#endif

class SfxMedium;
class SvStream;

class ScAddress;
class ScDocument;
class ScRange;

// Return-Werte Im-/Exportfilter    (ULONG)

typedef ULONG FltError;
//enum FltError {

#define eERR_OK         ERRCODE_NONE                // kein Fehler, alles OK
#define eERR_OPEN       SCERR_IMPORT_OPEN           // ...
#define eERR_UNBEK      SCERR_IMPORT_UNKNOWN        // unbekannter Fehler, auch historische Bedeutung
#define eERR_NOMEM      SCERR_IMPORT_OUTOFMEM       // nicht mehr genuegend Speicher zur Verfuegung
#define eERR_UNKN_WK    SCERR_IMPORT_UNKNOWN_WK     // unbekanntes WK?-Format (Lotus 1-2-3)
#define eERR_FORMAT     SCERR_IMPORT_FORMAT         // Formatfehler beim Lesen (kein Formel-Fehler!)
#define eERR_NI         SCERR_IMPORT_NI             // Nicht implementierter Filter
#define eERR_UNKN_BIFF  SCERR_IMPORT_UNKNOWN_BIFF   // unbekanntes BIFF-Format (Excel)
#define eERR_NI_BIFF    SCERR_IMPORT_NI_BIFF        // nicht implementiertes BIFF-Format
#define eERR_FILEPASSWD SCERR_IMPORT_FILEPASSWD     // File Passwordgeschuetzt
#define eERR_INTERN     SCERR_IMPORT_INTERNAL       // interner Fehler
#define eERR_RNGOVRFLW  SCWARN_IMPORT_RANGE_OVERFLOW// ueberlauf der Zellkoordinaten:
                                                    //  Tabelle abgschnitten auf erlaubtem Bereich
// mehr Fehlercodes siehe scerrors.hxx

//  };


// fuer Import
enum EXCIMPFORMAT { EIF_AUTO, EIF_BIFF5, EIF_BIFF8, EIF_BIFF_LE4 };

// fuer Export
enum ExportFormatLotus { ExpWK1, ExpWK3, ExpWK4 };
enum ExportFormatExcel { ExpBiff2, ExpBiff3, ExpBiff4, ExpBiff4W, ExpBiff5 };


// Optionen fuer DIF-Im-/Export (Kombination ueber '|')
#define SC_DIFOPT_PLAIN     0x00000000
#define SC_DIFOPT_DATE      0x00000001
#define SC_DIFOPT_TIME      0x00000002
#define SC_DIFOPT_CURRENCY  0x00000004

#define SC_DIFOPT_EXCEL     (SC_DIFOPT_DATE|SC_DIFOPT_TIME|SC_DIFOPT_CURRENCY)


// ***********************************************************************
// Diverse Importfilter
// ***********************************************************************

FltError ScImportLotus123( SfxMedium&, ScDocument*, CharSet eSrc = RTL_TEXTENCODING_DONTKNOW );

FltError ScImportExcel( SvStream&, ScDocument* );

FltError ScImportExcel( SfxMedium&, ScDocument* );

FltError ScImportExcel( SfxMedium&, ScDocument*, const EXCIMPFORMAT );
        // eFormat == EIF_AUTO  -> passender Filter wird automatisch verwendet
        // eFormat == EIF_BIFF5 -> nur Biff5-Stream fuehrt zum Erfolg (auch wenn in einem Excel97-Doc)
        // eFormat == EIF_BIFF8 -> nur Biff8-Stream fuehrt zum Erfolg (nur in Excel97-Docs)
        // eFormat == EIF_BIFF_LE4 -> nur Nicht-Storage-Dateien _koennen_ zum Erfolg fuehren

FltError ScImportStarCalc10( SvStream&, ScDocument* );

FltError ScImportDif( SvStream&, ScDocument*, const ScAddress& rInsPos,
                        const CharSet eSrc = RTL_TEXTENCODING_DONTKNOW, UINT32 nDifOption = SC_DIFOPT_EXCEL );

FltError ScImportRTF( SvStream&, ScDocument*, ScRange& rRange );

FltError ScImportHTML( SvStream&, ScDocument*, ScRange& rRange, double nOutputFactor = 1.0, BOOL bCalcWidthHeight = TRUE );

// ***********************************************************************
// Diverse Exportfilter
// ***********************************************************************

FltError ScExportLotus123( SvStream&, ScDocument*, ExportFormatLotus, CharSet eDest );

FltError ScExportExcel234( SvStream&, ScDocument*, ExportFormatExcel, CharSet eDest );

FltError ScExportExcel5( SfxMedium&, ScDocument*, const BOOL bTuerk, CharSet eDest );

FltError ScExportDif( SvStream&, ScDocument*, const ScAddress& rOutPos, const CharSet eDest,
                        UINT32 nDifOption = SC_DIFOPT_EXCEL );

FltError ScExportDif( SvStream&, ScDocument*, const ScRange& rRange, const CharSet eDest,
                        UINT32 nDifOption = SC_DIFOPT_EXCEL );

FltError ScExportHTML( SvStream&, ScDocument*, const ScRange& rRange, const CharSet eDest, BOOL bAll,
                        const String& rStreamPath );

FltError ScExportRTF( SvStream&, ScDocument*, const ScRange& rRange, const CharSet eDest );

#endif

