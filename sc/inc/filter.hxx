/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

class SfxMedium;
class SvStream;

class ScAddress;
class ScDocument;
class ScRange;
class SvNumberFormatter;
class ScOrcusFilters;

// return values im-/export filter  (sal_uLong)

typedef sal_uLong FltError;

#define eERR_OK         ERRCODE_NONE                // no error
#define eERR_OPEN       SCERR_IMPORT_OPEN           // ...
#define eERR_UNBEK      SCERR_IMPORT_UNKNOWN        // unknown error, historical meaning
#define eERR_NOMEM      SCERR_IMPORT_OUTOFMEM       // out of memory
#define eERR_UNKN_WK    SCERR_IMPORT_UNKNOWN_WK     // unknown WK? format (Lotus 1-2-3)
#define eERR_FORMAT     SCERR_IMPORT_FORMAT         // format error during reading (no formula error!)
#define eERR_NI         SCERR_IMPORT_NI             // filter not implemented
#define eERR_UNKN_BIFF  SCERR_IMPORT_UNKNOWN_BIFF   // unknown BIFF format (Excel)
#define eERR_NI_BIFF    SCERR_IMPORT_NI_BIFF        // not implemented BIFF format
#define eERR_FILEPASSWD SCERR_IMPORT_FILEPASSWD     // file password protected
#define eERR_INTERN     SCERR_IMPORT_INTERNAL       // internal error
#define eERR_RNGOVRFLW  SCWARN_IMPORT_RANGE_OVERFLOW// overflow of cell coordinates
                                                    // table restricted to valid area (?)
// more error codes: s. scerrors.hxx

// for import
enum EXCIMPFORMAT { EIF_AUTO, EIF_BIFF5, EIF_BIFF8, EIF_BIFF_LE4 };

// for export
enum ExportFormatLotus { ExpWK1, ExpWK3, ExpWK4 };
enum ExportFormatExcel { ExpBiff2, ExpBiff3, ExpBiff4, ExpBiff4W, ExpBiff5, ExpBiff8, Exp2007Xml };


// options for DIF im-/export (combine with '|')
#define SC_DIFOPT_PLAIN     0x00000000
#define SC_DIFOPT_DATE      0x00000001
#define SC_DIFOPT_TIME      0x00000002
#define SC_DIFOPT_CURRENCY  0x00000004

#define SC_DIFOPT_EXCEL     (SC_DIFOPT_DATE|SC_DIFOPT_TIME|SC_DIFOPT_CURRENCY)

// These are implemented inside the scfilt library and lazy loaded

class ScEEAbsImport {
  public:
    virtual ~ScEEAbsImport() {}
    virtual sal_uLong   Read( SvStream& rStream, const String& rBaseURL ) = 0;
    virtual ScRange GetRange() = 0;
    virtual void    WriteToDocument(
        bool bSizeColsRows = false, double nOutputFactor = 1.0,
        SvNumberFormatter* pFormatter = NULL, bool bConvertDate = true ) = 0;
};

class ScFormatFilterPlugin {
  public:
    // various import filters
    virtual FltError ScImportLotus123( SfxMedium&, ScDocument*, CharSet eSrc = RTL_TEXTENCODING_DONTKNOW ) = 0;
    virtual FltError ScImportQuattroPro( SfxMedium &rMedium, ScDocument *pDoc ) = 0;
    virtual FltError ScImportExcel( SfxMedium&, ScDocument*, const EXCIMPFORMAT ) = 0;
        // eFormat == EIF_AUTO  -> matching filter is used automatically
        // eFormat == EIF_BIFF5 -> only Biff5 stream is read sucessfully (in an Excel97 doc, too)
        // eFormat == EIF_BIFF8 -> only Biff8 stream is read sucessfully (only in Excel97 docs)
        // eFormat == EIF_BIFF_LE4 -> only non storage files _might_ be read sucessfully
    virtual FltError ScImportStarCalc10( SvStream&, ScDocument* ) = 0;
    virtual FltError ScImportDif( SvStream&, ScDocument*, const ScAddress& rInsPos,
                 const CharSet eSrc = RTL_TEXTENCODING_DONTKNOW, sal_uInt32 nDifOption = SC_DIFOPT_EXCEL ) = 0;
    virtual FltError ScImportRTF( SvStream&, const String& rBaseURL, ScDocument*, ScRange& rRange ) = 0;
    virtual FltError ScImportHTML( SvStream&, const String& rBaseURL, ScDocument*, ScRange& rRange, double nOutputFactor = 1.0,
                                   bool bCalcWidthHeight = true, SvNumberFormatter* pFormatter = NULL, bool bConvertDate = true ) = 0;

    // various import helpers
    virtual ScEEAbsImport *CreateRTFImport( ScDocument* pDoc, const ScRange& rRange ) = 0;
    virtual ScEEAbsImport *CreateHTMLImport( ScDocument* pDocP, const String& rBaseURL, const ScRange& rRange, bool bCalcWidthHeight ) = 0;
    virtual String         GetHTMLRangeNameList( ScDocument* pDoc, const String& rOrigName ) = 0;

    // various export filters
    virtual FltError ScExportExcel5( SfxMedium&, ScDocument*, ExportFormatExcel eFormat, CharSet eDest ) = 0;
    virtual FltError ScExportDif( SvStream&, ScDocument*, const ScAddress& rOutPos, const CharSet eDest,
                                 sal_uInt32 nDifOption = SC_DIFOPT_EXCEL ) = 0;
    virtual FltError ScExportDif( SvStream&, ScDocument*, const ScRange& rRange, const CharSet eDest,
                 sal_uInt32 nDifOption = SC_DIFOPT_EXCEL ) = 0;
    virtual FltError ScExportHTML( SvStream&, const String& rBaseURL, ScDocument*, const ScRange& rRange, const CharSet eDest, bool bAll,
                  const String& rStreamPath, String& rNonConvertibleChars ) = 0;
    virtual FltError ScExportRTF( SvStream&, ScDocument*, const ScRange& rRange, const CharSet eDest ) = 0;

    virtual ScOrcusFilters* GetOrcusFilters() = 0;

protected:
    ~ScFormatFilterPlugin() {}
};

// scfilt plugin symbol
extern "C" {
  SAL_DLLPUBLIC_EXPORT ScFormatFilterPlugin * SAL_CALL ScFilterCreate(void);
}

class ScFormatFilter {
    public:
    static ScFormatFilterPlugin &Get();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
