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
#include <comphelper/errcode.hxx>
#include <o3tl/typed_flags_set.hxx>
#include <vcl/BinaryDataContainer.hxx>
#include <vcl/graphic/GraphicMetadata.hxx>

#include <memory>

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
#define IMP_WMZ                 "SVWMZ"
#define IMP_EMF                 "SVEMF"
#define IMP_EMZ                 "SVEMZ"
#define IMP_GIF                 "SVIGIF"
#define IMP_PNG                 "SVIPNG"
#define IMP_JPEG                "SVIJPEG"
#define IMP_XBM                 "SVIXBM"
#define IMP_XPM                 "SVIXPM"
#define IMP_SVG                 "SVISVG"
#define IMP_SVGZ                "SVISVGZ"
#define IMP_PDF                 "SVIPDF"
#define IMP_TIFF                "SVTIFF"
#define IMP_TGA                 "SVTGA"
#define IMP_PICT                "SVPICT"
#define IMP_MET                 "SVMET"
#define IMP_RAS                 "SVRAS"
#define IMP_PCX                 "SVPCX"
#define IMP_EPS                 "SVIEPS"
#define IMP_PSD                 "SVPSD"
#define IMP_PCD                 "SVPCD"
#define IMP_PBM                 "SVPBM"
#define IMP_DXF                 "SVDXF"
#define IMP_WEBP                "SVIWEBP"

#define EXP_BMP                 "SVBMP"
#define EXP_SVMETAFILE          "SVMETAFILE"
#define EXP_WMF                 "SVWMF"
#define EXP_WMZ                 "SVWMZ"
#define EXP_EMF                 "SVEMF"
#define EXP_EMZ                 "SVEMZ"
#define EXP_JPEG                "SVEJPEG"
#define EXP_SVG                 "SVESVG"
#define EXP_SVGZ                "SVESVGZ"
#define EXP_PDF                 "SVEPDF"
#define EXP_PNG                 "SVEPNG"
#define EXP_APNG                "SVEAPNG"
#define EXP_TIFF                "SVTIFF"
#define EXP_EPS                 "SVEEPS"
#define EXP_GIF                 "SVEGIF"
#define EXP_WEBP                "SVEWEBP"


inline constexpr OUString BMP_SHORTNAME = u"BMP"_ustr;
inline constexpr OUString GIF_SHORTNAME = u"GIF"_ustr;
inline constexpr OUString JPG_SHORTNAME = u"JPG"_ustr;
inline constexpr OUString MET_SHORTNAME = u"MET"_ustr;
inline constexpr OUString PCT_SHORTNAME = u"PCT"_ustr;
inline constexpr OUString PNG_SHORTNAME = u"PNG"_ustr;
inline constexpr OUString SVM_SHORTNAME = u"SVM"_ustr;
inline constexpr OUString TIF_SHORTNAME = u"TIF"_ustr;
inline constexpr OUString WMF_SHORTNAME = u"WMF"_ustr;
inline constexpr OUString EMF_SHORTNAME = u"EMF"_ustr;
inline constexpr OUString SVG_SHORTNAME = u"SVG"_ustr;
inline constexpr OUString PDF_SHORTNAME = u"PDF"_ustr;
inline constexpr OUString WEBP_SHORTNAME = u"WEBP"_ustr;

class VCL_DLLPUBLIC GraphicDescriptor final
{
    SvStream*            pFileStm;
    OUString             aPathExt;
    GraphicMetadata      aMetadata;
    bool                 bOwnStream;

    SAL_DLLPRIVATE void                ImpConstruct();

    SAL_DLLPRIVATE bool            ImpDetectBMP( SvStream& rStm, bool bExtendedInfo );
    SAL_DLLPRIVATE bool            ImpDetectGIF( SvStream& rStm, bool bExtendedInfo );
    SAL_DLLPRIVATE bool            ImpDetectJPG( SvStream& rStm, bool bExtendedInfo );
    SAL_DLLPRIVATE bool            ImpDetectPCD( SvStream& rStm, bool bExtendedInfo );
    SAL_DLLPRIVATE bool            ImpDetectPCX( SvStream& rStm );
    SAL_DLLPRIVATE bool            ImpDetectPNG( SvStream& rStm, bool bExtendedInfo );
    SAL_DLLPRIVATE bool            ImpDetectTIF( SvStream& rStm, bool bExtendedInfo );
    SAL_DLLPRIVATE bool            ImpDetectXBM( SvStream& rStm, bool bExtendedInfo );
    SAL_DLLPRIVATE bool            ImpDetectXPM( SvStream& rStm, bool bExtendedInfo );
    SAL_DLLPRIVATE bool            ImpDetectPBM( SvStream& rStm, bool bExtendedInfo );
    SAL_DLLPRIVATE bool            ImpDetectPGM( SvStream& rStm, bool bExtendedInfo );
    SAL_DLLPRIVATE bool            ImpDetectPPM( SvStream& rStm, bool bExtendedInfo );
    SAL_DLLPRIVATE bool            ImpDetectRAS( SvStream& rStm, bool bExtendedInfo );
    SAL_DLLPRIVATE bool            ImpDetectTGA( SvStream& rStm, bool bExtendedInfo );
    SAL_DLLPRIVATE bool            ImpDetectPSD( SvStream& rStm, bool bExtendedInfo );
    SAL_DLLPRIVATE bool            ImpDetectEPS( SvStream& rStm, bool bExtendedInfo );
    SAL_DLLPRIVATE bool            ImpDetectWEBP( SvStream& rStm, bool bExtendedInfo );
    SAL_DLLPRIVATE bool            ImpDetectDXF( SvStream& rStm, bool bExtendedInfo );
    SAL_DLLPRIVATE bool            ImpDetectMET( SvStream& rStm, bool bExtendedInfo );
    SAL_DLLPRIVATE bool            ImpDetectPCT( SvStream& rStm, bool bExtendedInfo );
    SAL_DLLPRIVATE bool            ImpDetectSVM( SvStream& rStm, bool bExtendedInfo );
    SAL_DLLPRIVATE bool            ImpDetectWMF( SvStream& rStm, bool bExtendedInfo );
    SAL_DLLPRIVATE bool            ImpDetectEMF( SvStream& rStm, bool bExtendedInfo );
    SAL_DLLPRIVATE bool            ImpDetectSVG( SvStream& rStm, bool bExtendedInfo );
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
    GraphicFileFormat  GetFileFormat() const { return aMetadata.mnFormat; }

    /** @return graphic size in pixels or 0 size */
    const Size&     GetSizePixel() const { return aMetadata.maPixSize; }

    /** @return the logical graphic size in 1/100mm or 0 size */
    const Size&     GetSize_100TH_MM() const { return aMetadata.maLogSize; }

    /**
     * Returns the logic size, according to the map mode available via GetPreferredMapMode(). Prefer
     * this size over GetSize_100TH_MM().
     */
    const std::optional<Size>& GetPreferredLogSize() const { return aMetadata.maPreferredLogSize; }

    /**
     * If available, this returns the map mode the graphic prefers, which may be other than pixel or
     * 100th mm. Prefer this map mode over just assuming MapUnit::Map100thMM.
     */
    const std::optional<MapMode>& GetPreferredMapMode() const { return aMetadata.maPreferredMapMode; }

    /** @return bits/pixel or 0 **/
    sal_uInt16          GetBitsPerPixel() const { return aMetadata.mnBitsPerPixel; }

    /** @return number of color channels */
    sal_uInt8 GetNumberOfImageComponents() const { return aMetadata.mnNumberOfImageComponents; }

    /** @return whether image supports transparency */
    bool IsTransparent() const { return aMetadata.mbIsTransparent; }

    /** @return whether image supports alpha values for translucent colours */
    bool IsAlpha() const { return aMetadata.mbIsAlpha; }

    /** @return filter number that is needed by the GraphFilter to read this format */
    static OUString GetImportFormatShortName( GraphicFileFormat nFormat );
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
    ErrCode             ExportGraphic( const Graphic& rGraphic, std::u16string_view rPath,
                                       SvStream& rOStm, sal_uInt16 nFormat,
                                       const css::uno::Sequence< css::beans::PropertyValue >* pFilterData = nullptr );

    ErrCode             CanImportGraphic( const INetURLObject& rPath,
                                      sal_uInt16 nFormat,
                                      sal_uInt16 * pDeterminedFormat);

    ErrCode             ImportGraphic( Graphic& rGraphic, const INetURLObject& rPath,
                                   sal_uInt16 nFormat = GRFILTER_FORMAT_DONTKNOW,
                                   sal_uInt16 * pDeterminedFormat = nullptr, GraphicFilterImportFlags nImportFlags = GraphicFilterImportFlags::NONE );

    ErrCode             CanImportGraphic( std::u16string_view rPath, SvStream& rStream,
                                      sal_uInt16 nFormat,
                                      sal_uInt16 * pDeterminedFormat);

    ErrCode             ImportGraphic( Graphic& rGraphic, std::u16string_view rPath,
                                   SvStream& rStream,
                                   sal_uInt16 nFormat = GRFILTER_FORMAT_DONTKNOW,
                                   sal_uInt16 * pDeterminedFormat = nullptr, GraphicFilterImportFlags nImportFlags = GraphicFilterImportFlags::NONE );

    /// Imports multiple graphics.
    ///
    /// The resulting graphic is added to rGraphics on success, nullptr is added on failure.
    SAL_DLLPRIVATE void ImportGraphics(std::vector< std::shared_ptr<Graphic> >& rGraphics, std::vector< std::unique_ptr<SvStream> > vStreams);

    /**
     Tries to ensure all Graphic objects are available (Graphic::isAvailable()). Only an optimization, may
     not process all items.
    */
    void MakeGraphicsAvailableThreaded(std::vector< Graphic* >& rGraphics);

    // Setting sizeLimit limits how much will be read from the stream.
    Graphic ImportUnloadedGraphic(SvStream& rIStream, sal_uInt64 sizeLimit = 0, const Size* pSizeHint = nullptr);

    const ErrCode&          GetLastError() const { return *mxErrorEx;}
    SAL_DLLPRIVATE void     ResetLastError();

    Link<ConvertData&,bool> GetFilterCallback() const;
    static GraphicFilter& GetGraphicFilter();
    static ErrCode  LoadGraphic( const OUString& rPath, const OUString& rFilter,
                     Graphic& rGraphic,
                     GraphicFilter* pFilter = nullptr,
                     sal_uInt16* pDeterminedFormat = nullptr );

    ErrCode         compressAsPNG(const Graphic& rGraphic, SvStream& rOutputStream);

    SAL_DLLPRIVATE static ErrCode readGIF(SvStream& rStream, Graphic& rGraphic, GfxLinkType& rLinkType);
    SAL_DLLPRIVATE static ErrCode readPNG(SvStream & rStream, Graphic & rGraphic, GfxLinkType & rLinkType,
                    BinaryDataContainer & rpGraphicContent);
    SAL_DLLPRIVATE static ErrCode readJPEG(SvStream & rStream, Graphic & rGraphic, GfxLinkType & rLinkType,
                    GraphicFilterImportFlags nImportFlags);
    SAL_DLLPRIVATE static ErrCode readSVG(SvStream & rStream, Graphic & rGraphic, GfxLinkType & rLinkType,
                    BinaryDataContainer & rpGraphicContent);
    SAL_DLLPRIVATE static ErrCode readXBM(SvStream & rStream, Graphic & rGraphic);
    SAL_DLLPRIVATE static ErrCode readXPM(SvStream & rStream, Graphic & rGraphic);

    SAL_DLLPRIVATE static ErrCode readWMF_EMF(SvStream & rStream, Graphic & rGraphic, GfxLinkType & rLinkType, VectorGraphicDataType eType);
    SAL_DLLPRIVATE static ErrCode readWMF(SvStream & rStream, Graphic & rGraphic, GfxLinkType & rLinkType);
    SAL_DLLPRIVATE static ErrCode readEMF(SvStream & rStream, Graphic & rGraphic, GfxLinkType & rLinkType);

    SAL_DLLPRIVATE static ErrCode readPDF(SvStream & rStream, Graphic & rGraphic, GfxLinkType & rLinkType);
    SAL_DLLPRIVATE static ErrCode readTIFF(SvStream & rStream, Graphic & rGraphic, GfxLinkType & rLinkType);
    SAL_DLLPRIVATE static ErrCode readWithTypeSerializer(SvStream & rStream, Graphic & rGraphic, GfxLinkType & rLinkType, std::u16string_view aFilterName);
    SAL_DLLPRIVATE static ErrCode readBMP(SvStream & rStream, Graphic & rGraphic, GfxLinkType & rLinkType);
    SAL_DLLPRIVATE static ErrCode readTGA(SvStream & rStream, Graphic & rGraphic);
    SAL_DLLPRIVATE static ErrCode readPICT(SvStream & rStream, Graphic & rGraphic, GfxLinkType & rLinkType);
    SAL_DLLPRIVATE static ErrCode readMET(SvStream & rStream, Graphic & rGraphic, GfxLinkType & rLinkType);
    SAL_DLLPRIVATE static ErrCode readRAS(SvStream & rStream, Graphic & rGraphic);
    SAL_DLLPRIVATE static ErrCode readPCX(SvStream & rStream, Graphic & rGraphic);
    SAL_DLLPRIVATE static ErrCode readEPS(SvStream & rStream, Graphic & rGraphic);
    SAL_DLLPRIVATE static ErrCode readPSD(SvStream & rStream, Graphic & rGraphic);
    SAL_DLLPRIVATE static ErrCode readPCD(SvStream & rStream, Graphic & rGraphic);
    SAL_DLLPRIVATE static ErrCode readPBM(SvStream & rStream, Graphic & rGraphic);
    SAL_DLLPRIVATE static ErrCode readDXF(SvStream & rStream, Graphic & rGraphic);
    SAL_DLLPRIVATE static ErrCode readWEBP(SvStream & rStream, Graphic & rGraphic, GfxLinkType & rLinkType);

private:
    OUString        aFilterPath;
    FilterConfigCache*  pConfig;

    SAL_DLLPRIVATE void ImplInit();
    SAL_DLLPRIVATE ErrCode ImplSetError( ErrCode nError, const SvStream* pStm = nullptr );
    SAL_DLLPRIVATE ErrCode ImpTestOrFindFormat( std::u16string_view rPath, SvStream& rStream, sal_uInt16& rFormat );

                    DECL_DLLPRIVATE_LINK( FilterCallback, ConvertData&, bool );

    /** Information about errors during the GraphicFilter operation. */
    std::optional<ErrCode> mxErrorEx;
    bool                bUseConfig;
};

#endif // INCLUDED_VCL_GRAPHICFILTER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
