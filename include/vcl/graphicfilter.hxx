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

#ifndef INCLUDED_VCL_GRAPHICFILTER_HXX
#define INCLUDED_VCL_GRAPHICFILTER_HXX

#include <tools/gen.hxx>
#include <tools/urlobj.hxx>
#include <vcl/dllapi.h>
#include <vcl/field.hxx>
#include <vcl/graph.hxx>
#include <o3tl/typed_flags_set.hxx>

#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/beans/PropertyValue.hpp>

class FilterConfigCache;
class SvStream;
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

enum class GraphicFilterImportFlags
{
    NONE                   = 0x000,
    SetLogsizeForJpeg      = 0x001,
    DontSetLogsizeForJpeg  = 0x002,
    ForPreview             = 0x004,
    AllowPartialStreamRead = 0x010,
};
namespace o3tl
{
    template<> struct typed_flags<GraphicFilterImportFlags> : is_typed_flags<GraphicFilterImportFlags, 0x0017> {};
}

#define IMP_BMP                 "SVBMP"
#define IMP_MOV                 "SVMOV"
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
#define IMP_PDF                 "SVIPDF"
#define EXP_BMP                 "SVBMP"
#define EXP_SVMETAFILE          "SVMETAFILE"
#define EXP_WMF                 "SVWMF"
#define EXP_EMF                 "SVEMF"
#define EXP_JPEG                "SVEJPEG"
#define EXP_SVG                 "SVESVG"
#define EXP_PDF                 "SVEPDF"
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

enum class GraphicFileFormat
{
    NOT = 0x0000,
    BMP = 0x0001,
    GIF = 0x0002,
    JPG = 0x0003,
    PCD = 0x0004,
    PCX = 0x0005,
    PNG = 0x0006,
    TIF = 0x0007,
    XBM = 0x0008,
    XPM = 0x0009,
    PBM = 0x000a,
    PGM = 0x000b,
    PPM = 0x000c,
    RAS = 0x000d,
    TGA = 0x000e,
    PSD = 0x000f,
    EPS = 0x0010,
    DXF = 0x00f1,
    MET = 0x00f2,
    PCT = 0x00f3,
    SGF = 0x00f4,
    SVM = 0x00f5,
    WMF = 0x00f6,
    SGV = 0x00f7,
    EMF = 0x00f8,
    SVG = 0x00f9
};


class VCL_DLLPUBLIC GraphicDescriptor final
{
    SvStream*           pFileStm;

    OUString            aPathExt;
    Size                aPixSize;
    Size                aLogSize;
    sal_uInt16          nBitsPerPixel;
    sal_uInt16          nPlanes;
    GraphicFileFormat   nFormat;
    bool                bCompressed;
    bool                bOwnStream;

    void                ImpConstruct();

    bool            ImpDetectBMP( SvStream& rStm, bool bExtendedInfo );
    bool            ImpDetectGIF( SvStream& rStm, bool bExtendedInfo );
    bool            ImpDetectJPG( SvStream& rStm, bool bExtendedInfo );
    bool            ImpDetectPCD( SvStream& rStm, bool bExtendedInfo );
    bool            ImpDetectPCX( SvStream& rStm, bool bExtendedInfo );
    bool            ImpDetectPNG( SvStream& rStm, bool bExtendedInfo );
    bool            ImpDetectTIF( SvStream& rStm, bool bExtendedInfo );
    bool            ImpDetectXBM( SvStream& rStm, bool bExtendedInfo );
    bool            ImpDetectXPM( SvStream& rStm, bool bExtendedInfo );
    bool            ImpDetectPBM( SvStream& rStm, bool bExtendedInfo );
    bool            ImpDetectPGM( SvStream& rStm, bool bExtendedInfo );
    bool            ImpDetectPPM( SvStream& rStm, bool bExtendedInfo );
    bool            ImpDetectRAS( SvStream& rStm, bool bExtendedInfo );
    bool            ImpDetectTGA( SvStream& rStm, bool bExtendedInfo );
    bool            ImpDetectPSD( SvStream& rStm, bool bExtendedInfo );
    bool            ImpDetectEPS( SvStream& rStm, bool bExtendedInfo );
    bool            ImpDetectDXF( SvStream& rStm, bool bExtendedInfo );
    bool            ImpDetectMET( SvStream& rStm, bool bExtendedInfo );
    bool            ImpDetectPCT( SvStream& rStm, bool bExtendedInfo );
    bool            ImpDetectSGF( SvStream& rStm, bool bExtendedInfo );
    bool            ImpDetectSVM( SvStream& rStm, bool bExtendedInfo );
    bool            ImpDetectWMF( SvStream& rStm, bool bExtendedInfo );
    bool            ImpDetectSGV( SvStream& rStm, bool bExtendedInfo );
    bool            ImpDetectEMF( SvStream& rStm, bool bExtendedInfo );
    bool            ImpDetectSVG( SvStream& rStm, bool bExtendedInfo );
    GraphicDescriptor( const GraphicDescriptor& ) = delete;
    GraphicDescriptor& operator=( const GraphicDescriptor& ) = delete;

public:

    /** Ctor to set a filename

        Detect() must be called to identify the file
        If the file has no unique header (Mtf's), the format
        is determined from the extension */
    GraphicDescriptor( const INetURLObject& rPath );

    /** Ctor using a stream

        Detect() must be called to identify the file
        As some formats (Mtf's) do not have a unique header, it makes sense
        to supply the file name (incl. ext.), so that the format can be
        derived from the extension */
    GraphicDescriptor( SvStream& rInStream, const OUString* pPath );

    ~GraphicDescriptor();

    /** starts the detection

        if bExtendedInfo == true the file header is used to derive
        as many properties as possible (size, color, etc.) */
    bool    Detect( bool bExtendedInfo = false );

    /** @return the file format, GraphicFileFormat::NOT if no format was recognized */
    GraphicFileFormat  GetFileFormat() const { return nFormat; }

    /** @return graphic size in pixels or 0 size */
    const Size&     GetSizePixel() const { return (Size&) aPixSize; }

    /** @return the logical graphic size in 1/100mm or 0 size */
    const Size&     GetSize_100TH_MM() const { return (Size&) aLogSize; }

    /** @return bits/pixel or 0 **/
    sal_uInt16          GetBitsPerPixel() const { return nBitsPerPixel; }

    /** @return filter number that is needed by the GraphFilter to read this format */
    static OUString GetImportFormatShortName( GraphicFileFormat nFormat );
};

/** Information about errors during the GraphicFilter operation. */
struct FilterErrorEx
{
    ErrCode     nFilterError;
    ErrCode     nStreamError;

            FilterErrorEx() : nFilterError( ERRCODE_NONE ), nStreamError( ERRCODE_NONE ) {}
};

/** Class to import and export graphic formats. */
class VCL_DLLPUBLIC GraphicFilter
{
public:
                    GraphicFilter( bool bUseConfig = true );
                    ~GraphicFilter();

    sal_uInt16      GetImportFormatCount();
    sal_uInt16      GetImportFormatNumber( const OUString& rFormatName );
    sal_uInt16      GetImportFormatNumberForShortName( const OUString& rShortName );
    sal_uInt16      GetImportFormatNumberForTypeName( const OUString& rType );
    OUString        GetImportFormatName( sal_uInt16 nFormat );
    OUString        GetImportFormatTypeName( sal_uInt16 nFormat );
#ifdef _WIN32
    OUString        GetImportFormatMediaType( sal_uInt16 nFormat );
#endif
    OUString        GetImportFormatShortName( sal_uInt16 nFormat );
    OUString        GetImportWildcard( sal_uInt16 nFormat, sal_Int32 nEntry );

    sal_uInt16      GetExportFormatCount();
    sal_uInt16      GetExportFormatNumber( const OUString& rFormatName );
    sal_uInt16      GetExportFormatNumberForMediaType( const OUString& rShortName );
    sal_uInt16      GetExportFormatNumberForShortName( const OUString& rShortName );
    OUString        GetExportInternalFilterName( sal_uInt16 nFormat );
    sal_uInt16      GetExportFormatNumberForTypeName( const OUString& rType );
    OUString        GetExportFormatName( sal_uInt16 nFormat );
    OUString        GetExportFormatMediaType( sal_uInt16 nFormat );
    OUString        GetExportFormatShortName( sal_uInt16 nFormat );
    OUString        GetExportWildcard( sal_uInt16 nFormat );
    bool            IsExportPixelFormat( sal_uInt16 nFormat );

    sal_uInt16          ExportGraphic( const Graphic& rGraphic, const INetURLObject& rPath,
                                       sal_uInt16 nFormat,
                                       const css::uno::Sequence< css::beans::PropertyValue >* pFilterData = nullptr );
    sal_uInt16          ExportGraphic( const Graphic& rGraphic, const OUString& rPath,
                                       SvStream& rOStm, sal_uInt16 nFormat,
                                       const css::uno::Sequence< css::beans::PropertyValue >* pFilterData = nullptr );

    sal_uInt16          CanImportGraphic( const INetURLObject& rPath,
                                      sal_uInt16 nFormat,
                                      sal_uInt16 * pDeterminedFormat);

    sal_uInt16          ImportGraphic( Graphic& rGraphic, const INetURLObject& rPath,
                                   sal_uInt16 nFormat = GRFILTER_FORMAT_DONTKNOW,
                                   sal_uInt16 * pDeterminedFormat = nullptr, GraphicFilterImportFlags nImportFlags = GraphicFilterImportFlags::NONE );

    sal_uInt16          CanImportGraphic( const OUString& rPath, SvStream& rStream,
                                      sal_uInt16 nFormat,
                                      sal_uInt16 * pDeterminedFormat);

    sal_uInt16          ImportGraphic( Graphic& rGraphic, const OUString& rPath,
                                   SvStream& rStream,
                                   sal_uInt16 nFormat = GRFILTER_FORMAT_DONTKNOW,
                                   sal_uInt16 * pDeterminedFormat = nullptr, GraphicFilterImportFlags nImportFlags = GraphicFilterImportFlags::NONE,
                                   WMF_EXTERNALHEADER *pExtHeader = nullptr );

    sal_uInt16          ImportGraphic( Graphic& rGraphic, const OUString& rPath,
                                   SvStream& rStream,
                                   sal_uInt16 nFormat,
                                   sal_uInt16 * pDeterminedFormat, GraphicFilterImportFlags nImportFlags,
                                   css::uno::Sequence< css::beans::PropertyValue >* pFilterData,
                                   WMF_EXTERNALHEADER *pExtHeader = nullptr );

    const FilterErrorEx&    GetLastError() const { return *pErrorEx;}
    void                    ResetLastError();

    const Link<ConvertData&,bool> GetFilterCallback() const;
    static GraphicFilter& GetGraphicFilter();
    static int      LoadGraphic( const OUString& rPath, const OUString& rFilter,
                     Graphic& rGraphic,
                     GraphicFilter* pFilter = nullptr,
                     sal_uInt16* pDeterminedFormat = nullptr );

    sal_uInt16 compressAsPNG(const Graphic& rGraphic, SvStream& rOutputStream);

protected:
    OUString        aFilterPath;
    FilterConfigCache*  pConfig;

private:
    void            ImplInit();
    sal_uLong       ImplSetError( sal_uLong nError, const SvStream* pStm = nullptr );
    sal_uInt16      ImpTestOrFindFormat( const OUString& rPath, SvStream& rStream, sal_uInt16& rFormat );

                    DECL_LINK( FilterCallback, ConvertData&, bool );

    FilterErrorEx*      pErrorEx;
    bool                bUseConfig;
    long                nExpGraphHint;
};

#endif // INCLUDED_VCL_GRAPHICFILTER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
