/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef SC_FILTER_HXX
#define SC_FILTER_HXX

#include <tools/string.hxx>
#include <rtl/textenc.h>

#define ENABLE_LOTUS123_EXPORT 0

class SfxMedium;
class SvStream;

class ScAddress;
class ScDocument;
class ScRange;
class SvNumberFormatter;

// Return-Werte Im-/Exportfilter    (sal_uLong)

typedef sal_uLong FltError;
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
enum ExportFormatExcel { ExpBiff2, ExpBiff3, ExpBiff4, ExpBiff4W, ExpBiff5, ExpBiff8, Exp2007Xml };


// Optionen fuer DIF-Im-/Export (Kombination ueber '|')
#define SC_DIFOPT_PLAIN     0x00000000
#define SC_DIFOPT_DATE      0x00000001
#define SC_DIFOPT_TIME      0x00000002
#define SC_DIFOPT_CURRENCY  0x00000004

#define SC_DIFOPT_EXCEL     (SC_DIFOPT_DATE|SC_DIFOPT_TIME|SC_DIFOPT_CURRENCY)

// These are implemented inside the scfilt library and lazy loaded

class ScRTFImport;
class ScHTMLImport;

class ScEEAbsImport {
  public:
    virtual ~ScEEAbsImport() {}
    virtual sal_uLong   Read( SvStream& rStream, const String& rBaseURL ) = 0;
    virtual ScRange GetRange() = 0;
    virtual void    WriteToDocument(
        sal_Bool bSizeColsRows = sal_False, double nOutputFactor = 1.0,
        SvNumberFormatter* pFormatter = NULL, bool bConvertDate = true ) = 0;
};

class ScFormatFilterPlugin {
  public:
    // various import filters
    virtual FltError ScImportLotus123( SfxMedium&, ScDocument*, CharSet eSrc = RTL_TEXTENCODING_DONTKNOW ) = 0;
    virtual FltError ScImportQuattroPro( SfxMedium &rMedium, ScDocument *pDoc ) = 0;
    virtual FltError ScImportExcel( SfxMedium&, ScDocument*, const EXCIMPFORMAT ) = 0;
        // eFormat == EIF_AUTO  -> passender Filter wird automatisch verwendet
        // eFormat == EIF_BIFF5 -> nur Biff5-Stream fuehrt zum Erfolg (auch wenn in einem Excel97-Doc)
        // eFormat == EIF_BIFF8 -> nur Biff8-Stream fuehrt zum Erfolg (nur in Excel97-Docs)
        // eFormat == EIF_BIFF_LE4 -> nur Nicht-Storage-Dateien _koennen_ zum Erfolg fuehren
    virtual FltError ScImportStarCalc10( SvStream&, ScDocument* ) = 0;
    virtual FltError ScImportDif( SvStream&, ScDocument*, const ScAddress& rInsPos,
                 const CharSet eSrc = RTL_TEXTENCODING_DONTKNOW, sal_uInt32 nDifOption = SC_DIFOPT_EXCEL ) = 0;
    virtual FltError ScImportRTF( SvStream&, const String& rBaseURL, ScDocument*, ScRange& rRange ) = 0;
    virtual FltError ScImportHTML( SvStream&, const String& rBaseURL, ScDocument*, ScRange& rRange, double nOutputFactor = 1.0,
                                   sal_Bool bCalcWidthHeight = sal_True, SvNumberFormatter* pFormatter = NULL, bool bConvertDate = true ) = 0;

    // various import helpers
    virtual ScEEAbsImport *CreateRTFImport( ScDocument* pDoc, const ScRange& rRange ) = 0;
    virtual ScEEAbsImport *CreateHTMLImport( ScDocument* pDocP, const String& rBaseURL, const ScRange& rRange, sal_Bool bCalcWidthHeight ) = 0;
    virtual String         GetHTMLRangeNameList( ScDocument* pDoc, const String& rOrigName ) = 0;

    // various export filters
#if ENABLE_LOTUS123_EXPORT
    virtual FltError ScExportLotus123( SvStream&, ScDocument*, ExportFormatLotus, CharSet eDest ) = 0;
#endif
    virtual FltError ScExportExcel5( SfxMedium&, ScDocument*, ExportFormatExcel eFormat, CharSet eDest ) = 0;
    virtual FltError ScExportDif( SvStream&, ScDocument*, const ScAddress& rOutPos, const CharSet eDest,
                                 sal_uInt32 nDifOption = SC_DIFOPT_EXCEL ) = 0;
    virtual FltError ScExportDif( SvStream&, ScDocument*, const ScRange& rRange, const CharSet eDest,
                 sal_uInt32 nDifOption = SC_DIFOPT_EXCEL ) = 0;
    virtual FltError ScExportHTML( SvStream&, const String& rBaseURL, ScDocument*, const ScRange& rRange, const CharSet eDest, sal_Bool bAll,
                  const String& rStreamPath, String& rNonConvertibleChars ) = 0;
    virtual FltError ScExportRTF( SvStream&, ScDocument*, const ScRange& rRange, const CharSet eDest ) = 0;
};

// scfilt plugin symbol
extern "C" {
  ScFormatFilterPlugin * SAL_CALL ScFilterCreate(void);
}

class ScFormatFilter {
    public:
    static ScFormatFilterPlugin &Get();
};

#endif

