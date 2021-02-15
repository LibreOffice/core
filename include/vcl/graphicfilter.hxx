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
#include <vcl/dllapi.h>
#include <vcl/graph.hxx>
#include <vcl/errcode.hxx>
#include <o3tl/typed_flags_set.hxx>

#include <memory>
#include <optional>

namespace com::sun::star::beans { struct PropertyValue; }
namespace com::sun::star::uno { template <class E> class Sequence; }

class INetURLObject;

class FilterConfigCache;
class SvStream;
struct WmfExternal;
struct ConvertData;

#define ERRCODE_GRFILTER_OPENERROR    ErrCode(ErrCodeArea::Vcl, ErrCodeClass::General, 1)
#define ERRCODE_GRFILTER_IOERROR      ErrCode(ErrCodeArea::Vcl, ErrCodeClass::General, 2)
#define ERRCODE_GRFILTER_FORMATERROR  ErrCode(ErrCodeArea::Vcl, ErrCodeClass::General, 3)
#define ERRCODE_GRFILTER_VERSIONERROR ErrCode(ErrCodeArea::Vcl, ErrCodeClass::General, 4)
#define ERRCODE_GRFILTER_FILTERERROR  ErrCode(ErrCodeArea::Vcl, ErrCodeClass::General, 5)
#define ERRCODE_GRFILTER_TOOBIG       ErrCode(ErrCodeArea::Vcl, ErrCodeClass::General, 7)

#define GRFILTER_OUTHINT_GREY       1

#define GRFILTER_FORMAT_NOTFOUND    (sal_uInt16(0xFFFF))
#define GRFILTER_FORMAT_DONTKNOW    (sal_uInt16(0xFFFF))

enum class GraphicFilterImportFlags
{
    NONE                   = 0x000,
    SetLogsizeForJpeg      = 0x001,
    DontSetLogsizeForJpeg  = 0x002,
    /// Only create a bitmap, do not read pixel data.
    OnlyCreateBitmap       = 0x020,
    /// Read pixel data into an existing bitmap.
    UseExistingBitmap      = 0x040,
};
namespace o3tl
{
    template<> struct typed_flags<GraphicFilterImportFlags> : is_typed_flags<GraphicFilterImportFlags, 0x0063> {};
}

#define IMP_BMP                 "SVBMP"
#define IMP_MOV                 "SVMOV"
#define IMP_SVMETAFILE          "SVMETAFILE"
#define IMP_WMF                 "SVWMF"
#define IMP_EMF                 "SVEMF"
#define IMP_GIF                 "SVIGIF"
#define IMP_PNG                 "SVIPNG"
#define IMP_JPEG                "SVIJPEG"
#define IMP_XBM                 "SVIXBM"
#define IMP_XPM                 "SVIXPM"
#define IMP_SVG                 "SVISVG"
#define IMP_PDF                 "SVIPDF"
#define IMP_TIFF                "SVTIFF"
#define IMP_TGA                 "SVTGA"
#define IMP_PICT                "SVPICT"
#define IMP_MET                 "SVMET"
#define IMP_RAS                 "SVRAS"
#define IMP_PCX                 "SVPCX"
#define IMP_EPS                 "SVIEPS"
#define IMP_PSD                 "SVPSD"
#define EXP_BMP                 "SVBMP"
#define EXP_SVMETAFILE          "SVMETAFILE"
#define EXP_WMF                 "SVWMF"
#define EXP_EMF                 "SVEMF"
#define EXP_JPEG                "SVEJPEG"
#define EXP_SVG                 "SVESVG"
#define EXP_PDF                 "SVEPDF"
#define EXP_PNG                 "SVEPNG"
#define EXP_TIFF                "SVTIFF"
#define EXP_EPS                 "SVEEPS"


#define BMP_SHORTNAME           u"BMP"
#define GIF_SHORTNAME           u"GIF"
#define JPG_SHORTNAME           u"JPG"
#define MET_SHORTNAME           u"MET"
#define PCT_SHORTNAME           u"PCT"
#define PNG_SHORTNAME           u"PNG"
#define SVM_SHORTNAME           u"SVM"
#define TIF_SHORTNAME           u"TIF"
#define WMF_SHORTNAME           u"WMF"
#define EMF_SHORTNAME           u"EMF"
#define SVG_SHORTNAME           u"SVG"
#define PDF_SHORTNAME           u"PDF"

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
    // retired SGF = 0x00f4,
    SVM = 0x00f5,
    WMF = 0x00f6,
    // retired SGV = 0x00f7,
    EMF = 0x00f8,
    SVG = 0x00f9
};


class VCL_DLLPUBLIC GraphicDescriptor final
{
    SvStream*           pFileStm;

    OUString            aPathExt;
    Size                aPixSize;
    Size                aLogSize;
    std::optional<Size> maPreferredLogSize;
    std::optional<MapMode> maPreferredMapMode;
    sal_uInt16          nBitsPerPixel;
    sal_uInt16          nPlanes;
    GraphicFileFormat   nFormat;
    bool                bOwnStream;
    sal_uInt8 mnNumberOfImageComponents;
    bool                bIsTransparent;
    bool                bIsAlpha;

    void                ImpConstruct();

    bool            ImpDetectBMP( SvStream& rStm, bool bExtendedInfo );
    bool            ImpDetectGIF( SvStream& rStm, bool bExtendedInfo );
    bool            ImpDetectJPG( SvStream& rStm, bool bExtendedInfo );
    bool            ImpDetectPCD( SvStream& rStm, bool bExtendedInfo );
    bool            ImpDetectPCX( SvStream& rStm );
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
    bool            ImpDetectSVM( SvStream& rStm, bool bExtendedInfo );
    bool            ImpDetectWMF( SvStream& rStm, bool bExtendedInfo );
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
    const Size&     GetSizePixel() const { return aPixSize; }

    /** @return the logical graphic size in 1/100mm or 0 size */
    const Size&     GetSize_100TH_MM() const { return aLogSize; }

    /**
     * Returns the logic size, according to the map mode available via GetPreferredMapMode(). Prefer
     * this size over GetSize_100TH_MM().
     */
    std::optional<Size> GetPreferredLogSize() const { return maPreferredLogSize; }

    /**
     * If available, this returns the map mode the graphic prefers, which may be other than pixel or
     * 100th mm. Prefer this map mode over just assuming MapUnit::Map100thMM.
     */
    std::optional<MapMode> GetPreferredMapMode() const { return maPreferredMapMode; }

    /** @return bits/pixel or 0 **/
    sal_uInt16          GetBitsPerPixel() const { return nBitsPerPixel; }

    /** @return number of color channels */
    sal_uInt8 GetNumberOfImageComponents() const { return mnNumberOfImageComponents; }

    /** @return whether image supports transparency */
    bool IsTransparent() const { return bIsTransparent; }

    /** @return whether image supports alpha values for translucent colours */
    bool IsAlpha() const { return bIsAlpha; }

    /** @return filter number that is needed by the GraphFilter to read this format */
    static OUString GetImportFormatShortName( GraphicFileFormat nFormat );
};

/** Information about errors during the GraphicFilter operation. */
struct FilterErrorEx
{
    ErrCode   nStreamError;

    FilterErrorEx() : nStreamError( ERRCODE_NONE ) {}
};

/** Class to import and export graphic formats. */
class VCL_DLLPUBLIC GraphicFilter
{
public:
                    GraphicFilter( bool bUseConfig = true );
                    ~GraphicFilter();

    sal_uInt16      GetImportFormatCount() const;
    sal_uInt16      GetImportFormatNumber( std::u16string_view rFormatName );
    sal_uInt16      GetImportFormatNumberForShortName( std::u16string_view rShortName );
    sal_uInt16      GetImportFormatNumberForTypeName( std::u16string_view rType );
    OUString        GetImportFormatName( sal_uInt16 nFormat );
    OUString        GetImportFormatTypeName( sal_uInt16 nFormat );
#ifdef _WIN32
    OUString        GetImportFormatMediaType( sal_uInt16 nFormat );
#endif
    OUString        GetImportFormatShortName( sal_uInt16 nFormat );
    OUString        GetImportWildcard( sal_uInt16 nFormat, sal_Int32 nEntry );

    sal_uInt16      GetExportFormatCount() const;
    sal_uInt16      GetExportFormatNumber( std::u16string_view rFormatName );
    sal_uInt16      GetExportFormatNumberForMediaType( std::u16string_view rShortName );
    sal_uInt16      GetExportFormatNumberForShortName( std::u16string_view rShortName );
    OUString        GetExportInternalFilterName( sal_uInt16 nFormat );
    sal_uInt16      GetExportFormatNumberForTypeName( std::u16string_view rType );
    OUString        GetExportFormatName( sal_uInt16 nFormat );
    OUString        GetExportFormatMediaType( sal_uInt16 nFormat );
    OUString        GetExportFormatShortName( sal_uInt16 nFormat );
    OUString        GetExportWildcard( sal_uInt16 nFormat );
    bool            IsExportPixelFormat( sal_uInt16 nFormat );

    ErrCode             ExportGraphic( const Graphic& rGraphic, const INetURLObject& rPath,
                                       sal_uInt16 nFormat,
                                       const css::uno::Sequence< css::beans::PropertyValue >* pFilterData = nullptr );
    ErrCode             ExportGraphic( const Graphic& rGraphic, const OUString& rPath,
                                       SvStream& rOStm, sal_uInt16 nFormat,
                                       const css::uno::Sequence< css::beans::PropertyValue >* pFilterData = nullptr );

    ErrCode             CanImportGraphic( const INetURLObject& rPath,
                                      sal_uInt16 nFormat,
                                      sal_uInt16 * pDeterminedFormat);

    ErrCode             ImportGraphic( Graphic& rGraphic, const INetURLObject& rPath,
                                   sal_uInt16 nFormat = GRFILTER_FORMAT_DONTKNOW,
                                   sal_uInt16 * pDeterminedFormat = nullptr, GraphicFilterImportFlags nImportFlags = GraphicFilterImportFlags::NONE );

    ErrCode             CanImportGraphic( const OUString& rPath, SvStream& rStream,
                                      sal_uInt16 nFormat,
                                      sal_uInt16 * pDeterminedFormat);

    ErrCode             ImportGraphic( Graphic& rGraphic, const OUString& rPath,
                                   SvStream& rStream,
                                   sal_uInt16 nFormat = GRFILTER_FORMAT_DONTKNOW,
                                   sal_uInt16 * pDeterminedFormat = nullptr, GraphicFilterImportFlags nImportFlags = GraphicFilterImportFlags::NONE,
                                   WmfExternal const *pExtHeader = nullptr );

    /// Imports multiple graphics.
    ///
    /// The resulting graphic is added to rGraphics on success, nullptr is added on failure.
    void ImportGraphics(std::vector< std::shared_ptr<Graphic> >& rGraphics, std::vector< std::unique_ptr<SvStream> > vStreams);

    /**
     Tries to ensure all Graphic objects are available (Graphic::isAvailable()). Only an optimization, may
     not process all items.
    */
    void MakeGraphicsAvailableThreaded(std::vector< Graphic* >& rGraphics);

    ErrCode             ImportGraphic( Graphic& rGraphic, const OUString& rPath,
                                   SvStream& rStream,
                                   sal_uInt16 nFormat,
                                   sal_uInt16 * pDeterminedFormat, GraphicFilterImportFlags nImportFlags,
                                   const css::uno::Sequence< css::beans::PropertyValue >* pFilterData,
                                   WmfExternal const *pExtHeader = nullptr );

    // Setting sizeLimit limits how much will be read from the stream.
    Graphic ImportUnloadedGraphic(SvStream& rIStream, sal_uInt64 sizeLimit = 0, const Size* pSizeHint = nullptr);

    const FilterErrorEx&    GetLastError() const { return *pErrorEx;}
    void                    ResetLastError();

    Link<ConvertData&,bool> GetFilterCallback() const;
    static GraphicFilter& GetGraphicFilter();
    static ErrCode  LoadGraphic( const OUString& rPath, const OUString& rFilter,
                     Graphic& rGraphic,
                     GraphicFilter* pFilter = nullptr,
                     sal_uInt16* pDeterminedFormat = nullptr );

    ErrCode         compressAsPNG(const Graphic& rGraphic, SvStream& rOutputStream);

    void preload();

    static ErrCode readGIF(SvStream& rStream, Graphic& rGraphic, GfxLinkType& rLinkType);
    static ErrCode readPNG(SvStream & rStream, Graphic & rGraphic, GfxLinkType & rLinkType,
                    std::unique_ptr<sal_uInt8[]> & rpGraphicContent, sal_Int32& rGraphicContentSize);
    static ErrCode readJPEG(SvStream & rStream, Graphic & rGraphic, GfxLinkType & rLinkType,
                    GraphicFilterImportFlags nImportFlags);
    static ErrCode readSVG(SvStream & rStream, Graphic & rGraphic, GfxLinkType & rLinkType,
                    std::unique_ptr<sal_uInt8[]> & rpGraphicContent, sal_Int32& rGraphicContentSize);
    static ErrCode readXBM(SvStream & rStream, Graphic & rGraphic);
    static ErrCode readXPM(SvStream & rStream, Graphic & rGraphic);

    static ErrCode readWMF_EMF(SvStream & rStream, Graphic & rGraphic, GfxLinkType & rLinkType,
                        WmfExternal const* pExtHeader, VectorGraphicDataType eType);
    static ErrCode readWMF(SvStream & rStream, Graphic & rGraphic, GfxLinkType & rLinkType, WmfExternal const* pExtHeader);
    static ErrCode readEMF(SvStream & rStream, Graphic & rGraphic, GfxLinkType & rLinkType, WmfExternal const* pExtHeader);

    static ErrCode readPDF(SvStream & rStream, Graphic & rGraphic, GfxLinkType & rLinkType);
    static ErrCode readTIFF(SvStream & rStream, Graphic & rGraphic, GfxLinkType & rLinkType);
    static ErrCode readWithTypeSerializer(SvStream & rStream, Graphic & rGraphic, GfxLinkType & rLinkType, OUString aFilterName);
    static ErrCode readTGA(SvStream & rStream, Graphic & rGraphic);
    static ErrCode readPICT(SvStream & rStream, Graphic & rGraphic, GfxLinkType & rLinkType);
    static ErrCode readMET(SvStream & rStream, Graphic & rGraphic, GfxLinkType & rLinkType);
    static ErrCode readRAS(SvStream & rStream, Graphic & rGraphic);
    static ErrCode readPCX(SvStream & rStream, Graphic & rGraphic);
    static ErrCode readEPS(SvStream & rStream, Graphic & rGraphic);
    static ErrCode readPSD(SvStream & rStream, Graphic & rGraphic);

private:
    OUString        aFilterPath;
    FilterConfigCache*  pConfig;

    void            ImplInit();
    ErrCode         ImplSetError( ErrCode nError, const SvStream* pStm = nullptr );
    ErrCode         ImpTestOrFindFormat( const OUString& rPath, SvStream& rStream, sal_uInt16& rFormat );

                    DECL_LINK( FilterCallback, ConvertData&, bool );

    std::unique_ptr<FilterErrorEx> pErrorEx;
    bool                bUseConfig;
};

#endif // INCLUDED_VCL_GRAPHICFILTER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
