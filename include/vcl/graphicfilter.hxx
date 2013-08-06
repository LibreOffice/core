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

#ifndef _FILTER_HXX
#define _FILTER_HXX

#include <tools/gen.hxx>
#include <tools/stream.hxx>
#include <tools/urlobj.hxx>
#include <vcl/dllapi.h>
#include <vcl/field.hxx>
#include <vcl/graph.hxx>

#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/beans/PropertyValue.hpp>

class FilterConfigCache;
struct WMF_EXTERNALHEADER;
struct ConvertData;

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

//  Info class for all supported file formats

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

class VCL_DLLPUBLIC GraphicDescriptor
{
    SvStream*           pFileStm;

    OUString            aPathExt;
    Size                aPixSize;
    Size                aLogSize;
    sal_uInt16          nBitsPerPixel;
    sal_uInt16          nPlanes;
    sal_uInt16          nFormat;
    sal_Bool            bCompressed;
    sal_Bool            bOwnStream;

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

    /** Ctor to set a filename

        ::Detect() must be called to identify the file
        If the file has no unique header (Mtf's), the format
        is determined from the extension */
    GraphicDescriptor( const INetURLObject& rPath );

    /** Ctor using a stream

        ::Detect() must be called to identify the file
        As some formats (Mtf's) do not have a unique header, it makes sense
        to supply the file name (incl. ext.), so that the format can be
        derived from the extension */
    GraphicDescriptor( SvStream& rInStream, const OUString* pPath = NULL );

    virtual ~GraphicDescriptor();

    /** starts the detection

        if bExtendedInfo == sal_True the file header is used to derive
        as many properties as possible (size, color, etc.) */
    virtual sal_Bool    Detect( sal_Bool bExtendedInfo = sal_False );

    /** @return the file format, GFF_NOT if no format was recognized */
    sal_uInt16          GetFileFormat() const { return nFormat; }

    /** @return graphic size in pixels or 0 size */
    const Size&     GetSizePixel() const { return (Size&) aPixSize; }

    /** @return the logical graphic size in 1/100mm or 0 size */
    const Size&     GetSize_100TH_MM() const { return (Size&) aLogSize; }

    /** @return bits/pixel or 0 **/
    sal_uInt16          GetBitsPerPixel() const { return nBitsPerPixel; }

    /** return number of planes or 0 */
    sal_uInt16          GetPlanes() const { return nPlanes; }

    /** @return true if the graphic is compressed */
    sal_Bool            IsCompressed() const { return bCompressed; }

    /** @return filter number that is needed by the GraphFilter to read this format */
    static OUString GetImportFormatShortName( sal_uInt16 nFormat );
};

/** Information about errors during the GraphicFilter operation. */
struct FilterErrorEx
{
    sal_uLong   nFilterError;
    sal_uLong   nStreamError;

            FilterErrorEx() : nFilterError( 0UL ), nStreamError( 0UL ) {}
};

/** Class to import and export graphic formats. */
class VCL_DLLPUBLIC GraphicFilter
{
    friend class SvFilterOptionsDialog;

private:

    void            ImplInit();
    sal_uLong           ImplSetError( sal_uLong nError, const SvStream* pStm = NULL );
    sal_uInt16      ImpTestOrFindFormat( const OUString& rPath, SvStream& rStream, sal_uInt16& rFormat );

                    DECL_LINK( FilterCallback, ConvertData* pData );

protected:

    OUString       aFilterPath;
    FilterConfigCache*  pConfig;
    FilterErrorEx*      pErrorEx;
    sal_Bool            bAbort;
    sal_Bool            bUseConfig;
    long                nExpGraphHint;

public:

                    GraphicFilter( sal_Bool bUseConfig = sal_True );
                    ~GraphicFilter();

    void            SetFilterPath( const OUString& rFilterPath ) { aFilterPath = rFilterPath; };

    sal_uInt16          GetImportFormatCount();
    sal_uInt16          GetImportFormatNumber( const OUString& rFormatName );
    sal_uInt16          GetImportFormatNumberForMediaType( const OUString& rMediaType );
    sal_uInt16          GetImportFormatNumberForShortName( const OUString& rShortName );
    sal_uInt16      GetImportFormatNumberForTypeName( const OUString& rType );
    OUString        GetImportFormatName( sal_uInt16 nFormat );
    OUString        GetImportFormatTypeName( sal_uInt16 nFormat );
    OUString        GetImportFormatMediaType( sal_uInt16 nFormat );
    OUString        GetImportFormatShortName( sal_uInt16 nFormat );
    OUString        GetImportOSFileType( sal_uInt16 nFormat );
    OUString        GetImportWildcard( sal_uInt16 nFormat, sal_Int32 nEntry = 0 );
    sal_Bool            IsImportPixelFormat( sal_uInt16 nFormat );

    sal_uInt16          GetExportFormatCount();
    sal_uInt16          GetExportFormatNumber( const OUString& rFormatName );
    sal_uInt16          GetExportFormatNumberForMediaType( const OUString& rShortName );
    sal_uInt16          GetExportFormatNumberForShortName( const OUString& rShortName );
    OUString        GetExportInternalFilterName( sal_uInt16 nFormat );
    sal_uInt16      GetExportFormatNumberForTypeName( const OUString& rType );
    OUString        GetExportFormatName( sal_uInt16 nFormat );
    OUString        GetExportFormatTypeName( sal_uInt16 nFormat );
    OUString        GetExportFormatMediaType( sal_uInt16 nFormat );
    OUString        GetExportFormatShortName( sal_uInt16 nFormat );
    OUString        GetExportOSFileType( sal_uInt16 nFormat );
    OUString        GetExportWildcard( sal_uInt16 nFormat, sal_Int32 nEntry = 0 );
    sal_Bool            IsExportPixelFormat( sal_uInt16 nFormat );

    sal_uInt16          ExportGraphic( const Graphic& rGraphic, const INetURLObject& rPath,
                                    sal_uInt16 nFormat = GRFILTER_FORMAT_DONTKNOW,
                                        const com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >* pFilterData = NULL );
    sal_uInt16          ExportGraphic( const Graphic& rGraphic, const OUString& rPath,
                                    SvStream& rOStm, sal_uInt16 nFormat = GRFILTER_FORMAT_DONTKNOW,
                                        const com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >* pFilterData = NULL );
    long            GetExportGraphicHint() const { return nExpGraphHint; }

    sal_uInt16          CanImportGraphic( const INetURLObject& rPath,
                                      sal_uInt16 nFormat = GRFILTER_FORMAT_DONTKNOW,
                                      sal_uInt16 * pDeterminedFormat = NULL);

    sal_uInt16          ImportGraphic( Graphic& rGraphic, const INetURLObject& rPath,
                                   sal_uInt16 nFormat = GRFILTER_FORMAT_DONTKNOW,
                                   sal_uInt16 * pDeterminedFormat = NULL, sal_uInt32 nImportFlags = 0 );

    sal_uInt16          CanImportGraphic( const OUString& rPath, SvStream& rStream,
                                      sal_uInt16 nFormat = GRFILTER_FORMAT_DONTKNOW,
                                      sal_uInt16 * pDeterminedFormat = NULL);

    sal_uInt16          ImportGraphic( Graphic& rGraphic, const OUString& rPath,
                                   SvStream& rStream,
                                   sal_uInt16 nFormat = GRFILTER_FORMAT_DONTKNOW,
                                   sal_uInt16 * pDeterminedFormat = NULL, sal_uInt32 nImportFlags = 0,
                                   WMF_EXTERNALHEADER *pExtHeader = NULL );

    sal_uInt16          ImportGraphic( Graphic& rGraphic, const OUString& rPath,
                                   SvStream& rStream,
                                   sal_uInt16 nFormat,
                                   sal_uInt16 * pDeterminedFormat, sal_uInt32 nImportFlags,
                                   com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >* pFilterData,
                                   WMF_EXTERNALHEADER *pExtHeader = NULL );

    void            Abort() { bAbort = sal_True; }

    const FilterErrorEx&    GetLastError() const;
    void                    ResetLastError();

    const Link      GetFilterCallback() const;
    static GraphicFilter& GetGraphicFilter();
    static int      LoadGraphic( const OUString& rPath, const OUString& rFilter,
                     Graphic& rGraphic,
                     GraphicFilter* pFilter = NULL,
                     sal_uInt16* pDeterminedFormat = NULL );
};

#endif  //_FILTER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
