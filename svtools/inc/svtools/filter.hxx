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

#ifndef _FILTER_HXX
#define _FILTER_HXX

#include <svtools/fltcall.hxx>
#include "svtools/svtdllapi.h"
#include <tools/stream.hxx>
#include <vcl/graph.hxx>
#include <tools/gen.hxx>
#include <tools/urlobj.hxx>
#include <vcl/field.hxx>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/beans/PropertyValue.hpp>

struct WMF_EXTERNALHEADER;
// -----------------------
// - GraphicFilter-Types -
// -----------------------

struct ImplDirEntryHelper
{
    static sal_Bool Exists( const INetURLObject& rObj );
    static void Kill( const String& rStr );
};

class Window;
class Graphic;

#define OPT_FILTERSECTION           "Graphic"

#define GRFILTER_OK                 0
#define GRFILTER_OPENERROR          1
#define GRFILTER_IOERROR            2
#define GRFILTER_FORMATERROR        3
#define GRFILTER_VERSIONERROR       4
#define GRFILTER_FILTERERROR        5
#define GRFILTER_ABORT              6
#define GRFILTER_TOOBIG             7

#define GRFILTER_OUTHINT_GREY       1

#define GRFILTER_FORMAT_NOTFOUND    ((sal_uInt16)0xFFFF)
#define GRFILTER_FORMAT_DONTKNOW    ((sal_uInt16)0xFFFF)

#define GRFILTER_I_FLAGS_SET_LOGSIZE_FOR_JPEG       00000001
#define GRFILTER_I_FLAGS_DONT_SET_LOGSIZE_FOR_JPEG  00000002
#define GRFILTER_I_FLAGS_FOR_PREVIEW                00000004
#define GRFILTER_I_FLAGS_ALLOW_PARTIAL_STREAMREAD   00000010

#define IMP_BMP                 "SVBMP"
#define IMP_SVMETAFILE          "SVMETAFILE"
#define IMP_WMF                 "SVWMF"
#define IMP_EMF                 "SVEMF"
#define IMP_SVSGF               "SVSGF"
#define IMP_SVSGV               "SVSGV"
#define IMP_GIF                 "SVIGIF"
#define IMP_PNG                 "SVIPNG"
#define IMP_JPEG                "SVIJPEG"
#define IMP_XBM                 "SVIXBM"
#define IMP_XPM                 "SVIXPM"
#define IMP_SVG                 "SVISVG"
#define EXP_BMP                 "SVBMP"
#define EXP_SVMETAFILE          "SVMETAFILE"
#define EXP_WMF                 "SVWMF"
#define EXP_EMF                 "SVEMF"
#define EXP_JPEG                "SVEJPEG"
#define EXP_SVG                 "SVESVG"
#define EXP_PNG                 "SVEPNG"

#define BMP_SHORTNAME           "BMP"
#define GIF_SHORTNAME           "GIF"
#define JPG_SHORTNAME           "JPG"
#define MET_SHORTNAME           "MET"
#define PCT_SHORTNAME           "PCT"
#define PNG_SHORTNAME           "PNG"
#define SVM_SHORTNAME           "SVM"
#define TIF_SHORTNAME           "TIF"
#define WMF_SHORTNAME           "WMF"
#define EMF_SHORTNAME           "EMF"
#define SVG_SHORTNAME           "SVG"

// ------------------------------------
// - Info-Klasse fuer alle von uns
//  unterstuetzten Grafik-Fileformate
// ------------------------------------

#define GFF_NOT ( (sal_uInt16)0x0000 )
#define GFF_BMP ( (sal_uInt16)0x0001 )
#define GFF_GIF ( (sal_uInt16)0x0002 )
#define GFF_JPG ( (sal_uInt16)0x0003 )
#define GFF_PCD ( (sal_uInt16)0x0004 )
#define GFF_PCX ( (sal_uInt16)0x0005 )
#define GFF_PNG ( (sal_uInt16)0x0006 )
#define GFF_TIF ( (sal_uInt16)0x0007 )
#define GFF_XBM ( (sal_uInt16)0x0008 )
#define GFF_XPM ( (sal_uInt16)0x0009 )
#define GFF_PBM ( (sal_uInt16)0x000a )
#define GFF_PGM ( (sal_uInt16)0x000b )
#define GFF_PPM ( (sal_uInt16)0x000c )
#define GFF_RAS ( (sal_uInt16)0x000d )
#define GFF_TGA ( (sal_uInt16)0x000e )
#define GFF_PSD ( (sal_uInt16)0x000f )
#define GFF_EPS ( (sal_uInt16)0x0010 )
#define GFF_DXF ( (sal_uInt16)0x00f1 )
#define GFF_MET ( (sal_uInt16)0x00f2 )
#define GFF_PCT ( (sal_uInt16)0x00f3 )
#define GFF_SGF ( (sal_uInt16)0x00f4 )
#define GFF_SVM ( (sal_uInt16)0x00f5 )
#define GFF_WMF ( (sal_uInt16)0x00f6 )
#define GFF_SGV ( (sal_uInt16)0x00f7 )
#define GFF_EMF ( (sal_uInt16)0x00f8 )
#define GFF_SVG ( (sal_uInt16)0x00f9 )
#define GFF_XXX ( (sal_uInt16)0xffff )

// ---------------------
// - GraphicDescriptor -
// ---------------------

class SVT_DLLPUBLIC GraphicDescriptor
{
    SvStream*           pFileStm;

    String              aPathExt;
    Size                aPixSize;
    Size                aLogSize;
    sal_uInt16              nBitsPerPixel;
    sal_uInt16              nPlanes;
    sal_uInt16              nFormat;
    sal_Bool                bCompressed;
    sal_Bool                bOwnStream;

    void                ImpConstruct();

    sal_Bool            ImpDetectBMP( SvStream& rStm, sal_Bool bExtendedInfo );
    sal_Bool            ImpDetectGIF( SvStream& rStm, sal_Bool bExtendedInfo );
    sal_Bool            ImpDetectJPG( SvStream& rStm, sal_Bool bExtendedInfo );
    sal_Bool            ImpDetectPCD( SvStream& rStm, sal_Bool bExtendedInfo );
    sal_Bool            ImpDetectPCX( SvStream& rStm, sal_Bool bExtendedInfo );
    sal_Bool            ImpDetectPNG( SvStream& rStm, sal_Bool bExtendedInfo );
    sal_Bool            ImpDetectTIF( SvStream& rStm, sal_Bool bExtendedInfo );
    sal_Bool            ImpDetectXBM( SvStream& rStm, sal_Bool bExtendedInfo );
    sal_Bool            ImpDetectXPM( SvStream& rStm, sal_Bool bExtendedInfo );
    sal_Bool            ImpDetectPBM( SvStream& rStm, sal_Bool bExtendedInfo );
    sal_Bool            ImpDetectPGM( SvStream& rStm, sal_Bool bExtendedInfo );
    sal_Bool            ImpDetectPPM( SvStream& rStm, sal_Bool bExtendedInfo );
    sal_Bool            ImpDetectRAS( SvStream& rStm, sal_Bool bExtendedInfo );
    sal_Bool            ImpDetectTGA( SvStream& rStm, sal_Bool bExtendedInfo );
    sal_Bool            ImpDetectPSD( SvStream& rStm, sal_Bool bExtendedInfo );
    sal_Bool            ImpDetectEPS( SvStream& rStm, sal_Bool bExtendedInfo );
    sal_Bool            ImpDetectDXF( SvStream& rStm, sal_Bool bExtendedInfo );
    sal_Bool            ImpDetectMET( SvStream& rStm, sal_Bool bExtendedInfo );
    sal_Bool            ImpDetectPCT( SvStream& rStm, sal_Bool bExtendedInfo );
    sal_Bool            ImpDetectSGF( SvStream& rStm, sal_Bool bExtendedInfo );
    sal_Bool            ImpDetectSVM( SvStream& rStm, sal_Bool bExtendedInfo );
    sal_Bool            ImpDetectWMF( SvStream& rStm, sal_Bool bExtendedInfo );
    sal_Bool            ImpDetectSGV( SvStream& rStm, sal_Bool bExtendedInfo );
    sal_Bool            ImpDetectEMF( SvStream& rStm, sal_Bool bExtendedInfo );
    sal_Bool            ImpDetectSVG( SvStream& rStm, sal_Bool bExtendedInfo );
    GraphicDescriptor( const GraphicDescriptor& );
    GraphicDescriptor& operator=( const GraphicDescriptor& );

public:

    // Ctor, um einen Filenamen zu setzen. Es muss ::Detect() gerufen werden,
    // um das File zu identifizieren;
    // wenn das File keinen eindeutigen Header besitzt ( Mtf's ) wird das
    // Format anhand der Extension bestimmt
    GraphicDescriptor( const INetURLObject& rPath );

    // Ctor, um einen Stream zu setzen. Es muss ::Detect() gerufen werden,
    // um das File zu identifizieren;
    // da einige Formate ( Mtf's ) keinen eindeutigen Header besitzen,
    // ist es sinnvoll den Filenamen (inkl. Ext. ) mitanzugeben,
    // da so das Format ueber die Extension ermittelt werden kann
    GraphicDescriptor( SvStream& rInStream, const String* pPath = NULL );

    // Dtor
    virtual ~GraphicDescriptor();

    // Startet die Detektion;
    // bei bExtendedInfo == sal_True werden soweit wie moeglich
    // Daten aus dem jeweiligen FileHeader ermittelt
    // ( Groesse, Farbtiefe usw. )
    virtual sal_Bool    Detect( sal_Bool bExtendedInfo = sal_False );

    // liefert das Fileformat nach erfolgreicher  Detektion zurueck;
    // wenn kein Format erkannt wurde, ist das Formart GFF_NOT
    sal_uInt16          GetFileFormat() const { return nFormat; }

    // liefert die Pixel-Bildgroesse oder 0-Size zurueck
    const Size&     GetSizePixel() const { return (Size&) aPixSize; }

    // liefert die logische Bildgroesse in 1/100mm oder 0-Size zurueck
    const Size&     GetSize_100TH_MM() const { return (Size&) aLogSize; }

    // liefert die Bits/Pixel oder 0 zurueck
    sal_uInt16          GetBitsPerPixel() const { return nBitsPerPixel; }

    // liefert die Anzahl der Planes oder 0 zurueck
    sal_uInt16          GetPlanes() const { return nPlanes; }

    // zeigt an, ob das Bild evtl. komprimiert (wie auch immer) ist
    sal_Bool            IsCompressed() const { return bCompressed; }

    // gibt die Filternummer des Filters zurueck,
    // der im GraphicFilter zum Lesen dieses Formats
    // benoetigt wird
    static String GetImportFormatShortName( sal_uInt16 nFormat );
};

// -----------------
// - GraphicFilter -
// -----------------

struct FilterErrorEx
{
    sal_uLong   nFilterError;
    sal_uLong   nStreamError;

            FilterErrorEx() : nFilterError( 0UL ), nStreamError( 0UL ) {}
};

// -----------------------------------------------------------------------------

struct ConvertData;
class FilterConfigCache;
class SVT_DLLPUBLIC GraphicFilter
{
    friend class SvFilterOptionsDialog;

private:

    void            ImplInit();
    sal_uLong           ImplSetError( sal_uLong nError, const SvStream* pStm = NULL );
    sal_uInt16      ImpTestOrFindFormat( const String& rPath, SvStream& rStream, sal_uInt16& rFormat );

                    DECL_LINK( FilterCallback, ConvertData* pData );

protected:

    rtl::OUString       aFilterPath;
    FilterConfigCache*  pConfig;
    FilterErrorEx*      pErrorEx;
    sal_Bool            bAbort;
    sal_Bool            bUseConfig;
    long                nExpGraphHint;

public:

                    GraphicFilter( sal_Bool bUseConfig = sal_True );
                    ~GraphicFilter();

    void            SetFilterPath( const rtl::OUString& rFilterPath ) { aFilterPath = rFilterPath; };

    sal_uInt16          GetImportFormatCount();
    sal_uInt16          GetImportFormatNumber( const String& rFormatName );
    sal_uInt16          GetImportFormatNumberForMediaType( const String& rMediaType );
    sal_uInt16          GetImportFormatNumberForShortName( const String& rShortName );
    sal_uInt16      GetImportFormatNumberForTypeName( const String& rType );
    String          GetImportFormatName( sal_uInt16 nFormat );
    String          GetImportFormatTypeName( sal_uInt16 nFormat );
    String          GetImportFormatMediaType( sal_uInt16 nFormat );
    String          GetImportFormatShortName( sal_uInt16 nFormat );
    String          GetImportOSFileType( sal_uInt16 nFormat );
    String          GetImportWildcard( sal_uInt16 nFormat, sal_Int32 nEntry = 0 );
    sal_Bool            IsImportPixelFormat( sal_uInt16 nFormat );

    sal_uInt16          GetExportFormatCount();
    sal_uInt16          GetExportFormatNumber( const String& rFormatName );
    sal_uInt16          GetExportFormatNumberForMediaType( const String& rShortName );
    sal_uInt16          GetExportFormatNumberForShortName( const String& rShortName );
    sal_uInt16      GetExportFormatNumberForTypeName( const String& rType );
    String          GetExportFormatName( sal_uInt16 nFormat );
    String          GetExportFormatTypeName( sal_uInt16 nFormat );
    String          GetExportFormatMediaType( sal_uInt16 nFormat );
    String          GetExportFormatShortName( sal_uInt16 nFormat );
    String          GetExportOSFileType( sal_uInt16 nFormat );
    String          GetExportWildcard( sal_uInt16 nFormat, sal_Int32 nEntry = 0 );
    sal_Bool            IsExportPixelFormat( sal_uInt16 nFormat );

    sal_Bool            HasExportDialog( sal_uInt16 nFormat );
    sal_Bool            DoExportDialog( Window* pWindow, sal_uInt16 nFormat );
    sal_Bool            DoExportDialog( Window* pWindow, sal_uInt16 nFormat, FieldUnit eFieldUnit );

    sal_uInt16          ExportGraphic( const Graphic& rGraphic, const INetURLObject& rPath,
                                    sal_uInt16 nFormat = GRFILTER_FORMAT_DONTKNOW,
                                        const com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >* pFilterData = NULL );
    sal_uInt16          ExportGraphic( const Graphic& rGraphic, const String& rPath,
                                    SvStream& rOStm, sal_uInt16 nFormat = GRFILTER_FORMAT_DONTKNOW,
                                        const com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >* pFilterData = NULL );
    long            GetExportGraphicHint() const { return nExpGraphHint; }

    sal_uInt16          CanImportGraphic( const INetURLObject& rPath,
                                      sal_uInt16 nFormat = GRFILTER_FORMAT_DONTKNOW,
                                      sal_uInt16 * pDeterminedFormat = NULL);

    sal_uInt16          ImportGraphic( Graphic& rGraphic, const INetURLObject& rPath,
                                   sal_uInt16 nFormat = GRFILTER_FORMAT_DONTKNOW,
                                   sal_uInt16 * pDeterminedFormat = NULL, sal_uInt32 nImportFlags = 0 );

    sal_uInt16          CanImportGraphic( const String& rPath, SvStream& rStream,
                                      sal_uInt16 nFormat = GRFILTER_FORMAT_DONTKNOW,
                                      sal_uInt16 * pDeterminedFormat = NULL);

    sal_uInt16          ImportGraphic( Graphic& rGraphic, const String& rPath,
                                   SvStream& rStream,
                                   sal_uInt16 nFormat = GRFILTER_FORMAT_DONTKNOW,
                                   sal_uInt16 * pDeterminedFormat = NULL, sal_uInt32 nImportFlags = 0,
                                   WMF_EXTERNALHEADER *pExtHeader = NULL );

    sal_uInt16          ImportGraphic( Graphic& rGraphic, const String& rPath,
                                   SvStream& rStream,
                                   sal_uInt16 nFormat,
                                   sal_uInt16 * pDeterminedFormat, sal_uInt32 nImportFlags,
                                   com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >* pFilterData,
                                   WMF_EXTERNALHEADER *pExtHeader = NULL );

    sal_Bool            Setup( sal_uInt16 nFormat );

    void            Abort() { bAbort = sal_True; }

    const FilterErrorEx&    GetLastError() const;
    void                    ResetLastError();

    const Link      GetFilterCallback() const;
    static GraphicFilter& GetGraphicFilter();
    static int      LoadGraphic( const String& rPath, const String& rFilter,
                     Graphic& rGraphic,
                     GraphicFilter* pFilter = NULL,
                     sal_uInt16* pDeterminedFormat = NULL );
};

// ------------------------------------
// - Windows Metafile Lesen/Schreiben -
// ------------------------------------

SVT_DLLPUBLIC sal_Bool ReadWindowMetafile( SvStream& rStream, GDIMetaFile& rMTF, FilterConfigItem* pConfigItem );
SVT_DLLPUBLIC sal_Bool WriteWindowMetafileBits( SvStream& rStream, const GDIMetaFile& rMTF );

#endif  //_FILTER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
