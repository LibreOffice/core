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

#include <config_folders.h>

#include <sal/log.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/threadpool.hxx>
#include <cppuhelper/implbase.hxx>
#include <tools/fract.hxx>
#include <unotools/configmgr.hxx>
#include <tools/stream.hxx>
#include <tools/urlobj.hxx>
#include <tools/zcodec.hxx>
#include <fltcall.hxx>
#include <vcl/salctype.hxx>
#include <vcl/filter/PngImageReader.hxx>
#include <vcl/filter/SvmWriter.hxx>
#include <vcl/pngwrite.hxx>
#include <vcl/vectorgraphicdata.hxx>
#include <vcl/virdev.hxx>
#include <impgraph.hxx>
#include <vcl/svapp.hxx>
#include <osl/file.hxx>
#include <vcl/graphicfilter.hxx>
#include <vcl/FilterConfigItem.hxx>
#include <vcl/wmf.hxx>
#include "igif/gifread.hxx"
#include <vcl/pdfread.hxx>
#include "jpeg/jpeg.hxx"
#include "png/png.hxx"
#include "ixbm/xbmread.hxx"
#include <filter/XpmReader.hxx>
#include <filter/TiffReader.hxx>
#include <filter/TiffWriter.hxx>
#include <filter/TgaReader.hxx>
#include <filter/PictReader.hxx>
#include <filter/MetReader.hxx>
#include <filter/RasReader.hxx>
#include <filter/PcxReader.hxx>
#include <filter/EpsReader.hxx>
#include <filter/EpsWriter.hxx>
#include <filter/PsdReader.hxx>
#include <filter/PcdReader.hxx>
#include <filter/PbmReader.hxx>
#include <filter/DxfReader.hxx>
#include <filter/GifWriter.hxx>
#include <filter/BmpReader.hxx>
#include <filter/BmpWriter.hxx>
#include <osl/module.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/svg/XSVGWriter.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/xml/sax/Writer.hpp>
#include <unotools/ucbstreamhelper.hxx>
#include <rtl/bootstrap.hxx>
#include <tools/svlibrary.h>
#include <comphelper/string.hxx>
#include <unotools/ucbhelper.hxx>
#include <vector>
#include <memory>
#include <mutex>
#include <string_view>
#include <vcl/TypeSerializer.hxx>

#include "FilterConfigCache.hxx"
#include "graphicfilter_internal.hxx"

#include <graphic/GraphicFormatDetector.hxx>
#include <graphic/GraphicReader.hxx>

static std::vector< GraphicFilter* > gaFilterHdlList;

static std::mutex& getListMutex()
{
    static std::mutex s_aListProtection;
    return s_aListProtection;
}

namespace {

class ImpFilterOutputStream : public ::cppu::WeakImplHelper< css::io::XOutputStream >
{
    SvStream&               mrStm;

    virtual void SAL_CALL   writeBytes( const css::uno::Sequence< sal_Int8 >& rData ) override
        { mrStm.WriteBytes(rData.getConstArray(), rData.getLength()); }
    virtual void SAL_CALL   flush() override
        { mrStm.Flush(); }
    virtual void SAL_CALL   closeOutput() override {}

public:

    explicit ImpFilterOutputStream( SvStream& rStm ) : mrStm( rStm ) {}
};

}

// Helper functions

sal_uInt8* ImplSearchEntry( sal_uInt8* pSource, sal_uInt8 const * pDest, sal_uLong nComp, sal_uLong nSize )
{
    while ( nComp-- >= nSize )
    {
        sal_uLong i;
        for ( i = 0; i < nSize; i++ )
        {
            if ( ( pSource[i]&~0x20 ) != ( pDest[i]&~0x20 ) )
                break;
        }
        if ( i == nSize )
            return pSource;
        pSource++;
    }
    return nullptr;
}

static OUString ImpGetExtension( const OUString &rPath )
{
    OUString        aExt;
    INetURLObject   aURL( rPath );
    aExt = aURL.GetFileExtension().toAsciiUpperCase();
    return aExt;
}

bool isPCT(SvStream& rStream, sal_uLong nStreamPos, sal_uLong nStreamLen)
{
    sal_uInt8 sBuf[3];
    // store number format
    SvStreamEndian oldNumberFormat = rStream.GetEndian();
    sal_uInt32 nOffset; // in MS documents the pict format is used without the first 512 bytes
    for ( nOffset = 0; ( nOffset <= 512 ) && ( ( nStreamPos + nOffset + 14 ) <= nStreamLen ); nOffset += 512 )
    {
        short y1,x1,y2,x2;
        bool bdBoxOk = true;

        rStream.Seek( nStreamPos + nOffset);
        // size of the pict in version 1 pict ( 2bytes) : ignored
        rStream.SeekRel(2);
        // bounding box (bytes 2 -> 9)
        rStream.SetEndian(SvStreamEndian::BIG);
        rStream.ReadInt16( y1 ).ReadInt16( x1 ).ReadInt16( y2 ).ReadInt16( x2 );
        rStream.SetEndian(oldNumberFormat); // reset format

        if (x1 > x2 || y1 > y2 || // bad bdbox
            (x1 == x2 && y1 == y2) || // 1 pixel picture
            x2-x1 > 2048 || y2-y1 > 2048 ) // picture abnormally big
          bdBoxOk = false;

        // read version op
        rStream.ReadBytes(sBuf, 3);
        // see http://developer.apple.com/legacy/mac/library/documentation/mac/pdf/Imaging_With_QuickDraw/Appendix_A.pdf
        // normal version 2 - page A23 and A24
        if ( sBuf[ 0 ] == 0x00 && sBuf[ 1 ] == 0x11 && sBuf[ 2 ] == 0x02)
            return true;
        // normal version 1 - page A25
        else if (sBuf[ 0 ] == 0x11 && sBuf[ 1 ] == 0x01 && bdBoxOk)
            return true;
    }
    return false;
}

ErrCode GraphicFilter::ImpTestOrFindFormat( const OUString& rPath, SvStream& rStream, sal_uInt16& rFormat )
{
    // determine or check the filter/format by reading into it
    if( rFormat == GRFILTER_FORMAT_DONTKNOW )
    {
        OUString aFormatExt;
        if (vcl::peekGraphicFormat(rStream, aFormatExt, false))
        {
            rFormat = pConfig->GetImportFormatNumberForExtension( aFormatExt );
            if( rFormat != GRFILTER_FORMAT_DONTKNOW )
                return ERRCODE_NONE;
        }
        // determine filter by file extension
        if( !rPath.isEmpty() )
        {
            OUString aExt( ImpGetExtension( rPath ) );
            rFormat = pConfig->GetImportFormatNumberForExtension( aExt );
            if( rFormat != GRFILTER_FORMAT_DONTKNOW )
                return ERRCODE_NONE;
        }
        return ERRCODE_GRFILTER_FORMATERROR;
    }
    else
    {
        OUString aTmpStr( pConfig->GetImportFormatExtension( rFormat ) );
        aTmpStr = aTmpStr.toAsciiUpperCase();
        if (!vcl::peekGraphicFormat(rStream, aTmpStr, true))
            return ERRCODE_GRFILTER_FORMATERROR;
        if ( pConfig->GetImportFormatExtension( rFormat ).equalsIgnoreAsciiCase( "pcd" ) )
        {
            sal_Int32 nBase = 2;    // default Base0
            if ( pConfig->GetImportFilterType( rFormat ).equalsIgnoreAsciiCase( "pcd_Photo_CD_Base4" ) )
                nBase = 1;
            else if ( pConfig->GetImportFilterType( rFormat ).equalsIgnoreAsciiCase( "pcd_Photo_CD_Base16" ) )
                nBase = 0;
            FilterConfigItem aFilterConfigItem( u"Office.Common/Filter/Graphic/Import/PCD" );
            aFilterConfigItem.WriteInt32( "Resolution", nBase );
        }
    }

    return ERRCODE_NONE;
}

static Graphic ImpGetScaledGraphic( const Graphic& rGraphic, FilterConfigItem& rConfigItem )
{
    Graphic     aGraphic;

    sal_Int32 nLogicalWidth = rConfigItem.ReadInt32( "LogicalWidth", 0 );
    sal_Int32 nLogicalHeight = rConfigItem.ReadInt32( "LogicalHeight", 0 );

    if ( rGraphic.GetType() != GraphicType::NONE )
    {
        sal_Int32 nMode = rConfigItem.ReadInt32( "ExportMode", -1 );

        if ( nMode == -1 )  // the property is not there, this is possible, if the graphic filter
        {                   // is called via UnoGraphicExporter and not from a graphic export Dialog
            nMode = 0;      // then we are defaulting this mode to 0
            if ( nLogicalWidth || nLogicalHeight )
                nMode = 2;
        }

        Size aOriginalSize;
        Size aPrefSize( rGraphic.GetPrefSize() );
        MapMode aPrefMapMode( rGraphic.GetPrefMapMode() );
        if (aPrefMapMode.GetMapUnit() == MapUnit::MapPixel)
            aOriginalSize = Application::GetDefaultDevice()->PixelToLogic(aPrefSize, MapMode(MapUnit::Map100thMM));
        else
            aOriginalSize = OutputDevice::LogicToLogic(aPrefSize, aPrefMapMode, MapMode(MapUnit::Map100thMM));
        if ( !nLogicalWidth )
            nLogicalWidth = aOriginalSize.Width();
        if ( !nLogicalHeight )
            nLogicalHeight = aOriginalSize.Height();
        if( rGraphic.GetType() == GraphicType::Bitmap )
        {

            // Resolution is set
            if( nMode == 1 )
            {
                BitmapEx    aBitmap( rGraphic.GetBitmapEx() );
                MapMode     aMap( MapUnit::Map100thInch );

                sal_Int32   nDPI = rConfigItem.ReadInt32( "Resolution", 75 );
                Fraction    aFrac( 1, std::clamp( nDPI, sal_Int32(75), sal_Int32(600) ) );

                aMap.SetScaleX( aFrac );
                aMap.SetScaleY( aFrac );

                Size aOldSize = aBitmap.GetSizePixel();
                aGraphic = rGraphic;
                aGraphic.SetPrefMapMode( aMap );
                aGraphic.SetPrefSize( Size( aOldSize.Width() * 100,
                                            aOldSize.Height() * 100 ) );
            }
            // Size is set
            else if( nMode == 2 )
            {
               aGraphic = rGraphic;
               aGraphic.SetPrefMapMode( MapMode( MapUnit::Map100thMM ) );
               aGraphic.SetPrefSize( Size( nLogicalWidth, nLogicalHeight ) );
            }
            else
                aGraphic = rGraphic;

            sal_Int32 nColors = rConfigItem.ReadInt32( "Color", 0 );
            if ( nColors )  // graphic conversion necessary ?
            {
                BitmapEx aBmpEx( aGraphic.GetBitmapEx() );
                aBmpEx.Convert( static_cast<BmpConversion>(nColors) );   // the entries in the xml section have the same meaning as
                aGraphic = aBmpEx;                          // they have in the BmpConversion enum, so it should be
            }                                               // allowed to cast them
        }
        else
        {
            if( ( nMode == 1 ) || ( nMode == 2 ) )
            {
                GDIMetaFile aMtf( rGraphic.GetGDIMetaFile() );
                Size aNewSize( OutputDevice::LogicToLogic(Size(nLogicalWidth, nLogicalHeight), MapMode(MapUnit::Map100thMM), aMtf.GetPrefMapMode()) );

                if( aNewSize.Width() && aNewSize.Height() )
                {
                    const Size aPreferredSize( aMtf.GetPrefSize() );
                    aMtf.Scale( Fraction( aNewSize.Width(), aPreferredSize.Width() ),
                                Fraction( aNewSize.Height(), aPreferredSize.Height() ) );
                }
                aGraphic = Graphic( aMtf );
            }
            else
                aGraphic = rGraphic;
        }

    }
    else
        aGraphic = rGraphic;

    return aGraphic;
}

GraphicFilter::GraphicFilter( bool bConfig )
    : bUseConfig(bConfig)
{
    ImplInit();
}

GraphicFilter::~GraphicFilter()
{
    {
        std::scoped_lock aGuard( getListMutex() );
        auto it = std::find(gaFilterHdlList.begin(), gaFilterHdlList.end(), this);
        if( it != gaFilterHdlList.end() )
            gaFilterHdlList.erase( it );

        if( gaFilterHdlList.empty() )
            delete pConfig;
    }

    mxErrorEx.reset();
}

void GraphicFilter::ImplInit()
{
    {
        std::scoped_lock aGuard( getListMutex() );

        if ( gaFilterHdlList.empty() )
            pConfig = new FilterConfigCache( bUseConfig );
        else
            pConfig = gaFilterHdlList.front()->pConfig;

        gaFilterHdlList.push_back( this );
    }

    if( bUseConfig )
    {
        OUString url("$BRAND_BASE_DIR/" LIBO_LIB_FOLDER);
        rtl::Bootstrap::expandMacros(url); //TODO: detect failure
        osl::FileBase::getSystemPathFromFileURL(url, aFilterPath);
    }

    mxErrorEx = ERRCODE_NONE;
}

ErrCode GraphicFilter::ImplSetError( ErrCode nError, const SvStream* pStm )
{
    mxErrorEx = pStm ? pStm->GetError() : ERRCODE_NONE;
    return nError;
}

sal_uInt16 GraphicFilter::GetImportFormatCount() const
{
    return pConfig->GetImportFormatCount();
}

sal_uInt16 GraphicFilter::GetImportFormatNumber( std::u16string_view rFormatName )
{
    return pConfig->GetImportFormatNumber( rFormatName );
}

sal_uInt16 GraphicFilter::GetImportFormatNumberForShortName( std::u16string_view rShortName )
{
    return pConfig->GetImportFormatNumberForShortName( rShortName );
}

sal_uInt16 GraphicFilter::GetImportFormatNumberForTypeName( std::u16string_view rType )
{
    return pConfig->GetImportFormatNumberForTypeName( rType );
}

OUString GraphicFilter::GetImportFormatName( sal_uInt16 nFormat )
{
    return pConfig->GetImportFormatName( nFormat );
}

OUString GraphicFilter::GetImportFormatTypeName( sal_uInt16 nFormat )
{
    return pConfig->GetImportFilterTypeName( nFormat );
}

#ifdef _WIN32
OUString GraphicFilter::GetImportFormatMediaType( sal_uInt16 nFormat )
{
    return pConfig->GetImportFormatMediaType( nFormat );
}
#endif

OUString GraphicFilter::GetImportFormatShortName( sal_uInt16 nFormat )
{
    return pConfig->GetImportFormatShortName( nFormat );
}

OUString GraphicFilter::GetImportWildcard( sal_uInt16 nFormat, sal_Int32 nEntry )
{
    return pConfig->GetImportWildcard( nFormat, nEntry );
}

sal_uInt16 GraphicFilter::GetExportFormatCount() const
{
    return pConfig->GetExportFormatCount();
}

sal_uInt16 GraphicFilter::GetExportFormatNumber( std::u16string_view rFormatName )
{
    return pConfig->GetExportFormatNumber( rFormatName );
}

sal_uInt16 GraphicFilter::GetExportFormatNumberForMediaType( std::u16string_view rMediaType )
{
    return pConfig->GetExportFormatNumberForMediaType( rMediaType );
}

sal_uInt16 GraphicFilter::GetExportFormatNumberForShortName( std::u16string_view rShortName )
{
    return pConfig->GetExportFormatNumberForShortName( rShortName );
}

OUString GraphicFilter::GetExportInternalFilterName( sal_uInt16 nFormat )
{
    return pConfig->GetExportInternalFilterName( nFormat );
}

sal_uInt16 GraphicFilter::GetExportFormatNumberForTypeName( std::u16string_view rType )
{
    return pConfig->GetExportFormatNumberForTypeName( rType );
}

OUString GraphicFilter::GetExportFormatName( sal_uInt16 nFormat )
{
    return pConfig->GetExportFormatName( nFormat );
}

OUString GraphicFilter::GetExportFormatMediaType( sal_uInt16 nFormat )
{
    return pConfig->GetExportFormatMediaType( nFormat );
}

OUString GraphicFilter::GetExportFormatShortName( sal_uInt16 nFormat )
{
    return pConfig->GetExportFormatShortName( nFormat );
}

OUString GraphicFilter::GetExportWildcard( sal_uInt16 nFormat )
{
    return pConfig->GetExportWildcard( nFormat, 0 );
}

bool GraphicFilter::IsExportPixelFormat( sal_uInt16 nFormat )
{
    return pConfig->IsExportPixelFormat( nFormat );
}

ErrCode GraphicFilter::CanImportGraphic( const INetURLObject& rPath,
                                        sal_uInt16 nFormat, sal_uInt16* pDeterminedFormat )
{
    ErrCode  nRetValue = ERRCODE_GRFILTER_FORMATERROR;
    SAL_WARN_IF( rPath.GetProtocol() == INetProtocol::NotValid, "vcl.filter", "GraphicFilter::CanImportGraphic() : ProtType == INetProtocol::NotValid" );

    OUString    aMainUrl( rPath.GetMainURL( INetURLObject::DecodeMechanism::NONE ) );
    std::unique_ptr<SvStream> xStream(::utl::UcbStreamHelper::CreateStream( aMainUrl, StreamMode::READ | StreamMode::SHARE_DENYNONE ));
    if (xStream)
    {
        nRetValue = CanImportGraphic( aMainUrl, *xStream, nFormat, pDeterminedFormat );
    }
    return nRetValue;
}

ErrCode GraphicFilter::CanImportGraphic( const OUString& rMainUrl, SvStream& rIStream,
                                        sal_uInt16 nFormat, sal_uInt16* pDeterminedFormat )
{
    sal_uLong nStreamPos = rIStream.Tell();
    ErrCode nRes = ImpTestOrFindFormat( rMainUrl, rIStream, nFormat );

    rIStream.Seek(nStreamPos);

    if( nRes==ERRCODE_NONE && pDeterminedFormat!=nullptr )
        *pDeterminedFormat = nFormat;

    return ImplSetError( nRes, &rIStream );
}

//SJ: TODO, we need to create a GraphicImporter component
ErrCode GraphicFilter::ImportGraphic( Graphic& rGraphic, const INetURLObject& rPath,
                                     sal_uInt16 nFormat, sal_uInt16 * pDeterminedFormat, GraphicFilterImportFlags nImportFlags )
{
    ErrCode nRetValue = ERRCODE_GRFILTER_FORMATERROR;
    SAL_WARN_IF( rPath.GetProtocol() == INetProtocol::NotValid, "vcl.filter", "GraphicFilter::ImportGraphic() : ProtType == INetProtocol::NotValid" );

    OUString    aMainUrl( rPath.GetMainURL( INetURLObject::DecodeMechanism::NONE ) );
    std::unique_ptr<SvStream> xStream(::utl::UcbStreamHelper::CreateStream( aMainUrl, StreamMode::READ | StreamMode::SHARE_DENYNONE ));
    if (xStream)
    {
        nRetValue = ImportGraphic( rGraphic, aMainUrl, *xStream, nFormat, pDeterminedFormat, nImportFlags );
    }
    return nRetValue;
}

ErrCode GraphicFilter::ImportGraphic(
    Graphic& rGraphic,
    const OUString& rPath,
    SvStream& rIStream,
    sal_uInt16 nFormat,
    sal_uInt16* pDeterminedFormat,
    GraphicFilterImportFlags nImportFlags,
    WmfExternal const *pExtHeader)
{
    return ImportGraphic( rGraphic, rPath, rIStream, nFormat, pDeterminedFormat, nImportFlags, nullptr, pExtHeader );
}

namespace {

/// Contains a stream and other associated data to import pixels into a
/// Graphic.
struct GraphicImportContext
{
    /// Pixel data is read from this stream.
    std::unique_ptr<SvStream> m_pStream;
    /// The Graphic the import filter gets.
    std::shared_ptr<Graphic> m_pGraphic;
    /// Write pixel data using this access.
    std::unique_ptr<BitmapScopedWriteAccess> m_pAccess;
    std::unique_ptr<AlphaScopedWriteAccess> m_pAlphaAccess;
    // Need to have an AlphaMask instance to keep its lifetime.
    AlphaMask mAlphaMask;
    /// Signals if import finished correctly.
    ErrCode m_nStatus = ERRCODE_GRFILTER_FILTERERROR;
    /// Original graphic format.
    GfxLinkType m_eLinkType = GfxLinkType::NONE;
    /// Position of the stream before reading the data.
    sal_uInt64 m_nStreamBegin = 0;
    /// Flags for the import filter.
    GraphicFilterImportFlags m_nImportFlags = GraphicFilterImportFlags::NONE;
};

/// Graphic import worker that gets executed on a thread.
class GraphicImportTask : public comphelper::ThreadTask
{
    GraphicImportContext& m_rContext;
public:
    GraphicImportTask(const std::shared_ptr<comphelper::ThreadTaskTag>& pTag, GraphicImportContext& rContext);
    void doWork() override;
    /// Shared code between threaded and non-threaded version.
    static void doImport(GraphicImportContext& rContext);
};

}

GraphicImportTask::GraphicImportTask(const std::shared_ptr<comphelper::ThreadTaskTag>& pTag, GraphicImportContext& rContext)
    : comphelper::ThreadTask(pTag),
      m_rContext(rContext)
{
}

void GraphicImportTask::doWork()
{
    GraphicImportTask::doImport(m_rContext);
}

void GraphicImportTask::doImport(GraphicImportContext& rContext)
{
    if(rContext.m_eLinkType == GfxLinkType::NativeJpg)
    {
        if (!ImportJPEG(*rContext.m_pStream, *rContext.m_pGraphic, rContext.m_nImportFlags | GraphicFilterImportFlags::UseExistingBitmap, rContext.m_pAccess.get()))
            rContext.m_nStatus = ERRCODE_GRFILTER_FILTERERROR;
    }
    else if(rContext.m_eLinkType == GfxLinkType::NativePng)
    {
        if (!vcl::ImportPNG(*rContext.m_pStream, *rContext.m_pGraphic,
            rContext.m_nImportFlags | GraphicFilterImportFlags::UseExistingBitmap,
            rContext.m_pAccess.get(), rContext.m_pAlphaAccess.get()))
        {
            rContext.m_nStatus = ERRCODE_GRFILTER_FILTERERROR;
        }
    }
}

void GraphicFilter::ImportGraphics(std::vector< std::shared_ptr<Graphic> >& rGraphics, std::vector< std::unique_ptr<SvStream> > vStreams)
{
    static bool bThreads = !getenv("VCL_NO_THREAD_IMPORT");
    std::vector<GraphicImportContext> aContexts;
    aContexts.reserve(vStreams.size());
    comphelper::ThreadPool& rSharedPool = comphelper::ThreadPool::getSharedOptimalPool();
    std::shared_ptr<comphelper::ThreadTaskTag> pTag = comphelper::ThreadPool::createThreadTaskTag();

    for (auto& pStream : vStreams)
    {
        aContexts.emplace_back();
        GraphicImportContext& rContext = aContexts.back();

        if (pStream)
        {
            rContext.m_pStream = std::move(pStream);
            rContext.m_pGraphic = std::make_shared<Graphic>();
            rContext.m_nStatus = ERRCODE_NONE;

            // Detect the format.
            ResetLastError();
            rContext.m_nStreamBegin = rContext.m_pStream->Tell();
            sal_uInt16 nFormat = GRFILTER_FORMAT_DONTKNOW;
            rContext.m_nStatus = ImpTestOrFindFormat(OUString(), *rContext.m_pStream, nFormat);
            rContext.m_pStream->Seek(rContext.m_nStreamBegin);

            // Import the graphic.
            if (rContext.m_nStatus == ERRCODE_NONE && !rContext.m_pStream->GetError())
            {
                OUString aFilterName = pConfig->GetImportFilterName(nFormat);

                if (aFilterName.equalsIgnoreAsciiCase(IMP_JPEG))
                {
                    rContext.m_eLinkType = GfxLinkType::NativeJpg;
                    rContext.m_nImportFlags = GraphicFilterImportFlags::SetLogsizeForJpeg;

                    if (ImportJPEG( *rContext.m_pStream, *rContext.m_pGraphic, rContext.m_nImportFlags | GraphicFilterImportFlags::OnlyCreateBitmap, nullptr))
                    {
                        Bitmap& rBitmap = const_cast<Bitmap&>(rContext.m_pGraphic->GetBitmapExRef().GetBitmap());
                        rContext.m_pAccess = std::make_unique<BitmapScopedWriteAccess>(rBitmap);
                        rContext.m_pStream->Seek(rContext.m_nStreamBegin);
                        if (bThreads)
                            rSharedPool.pushTask(std::make_unique<GraphicImportTask>(pTag, rContext));
                        else
                            GraphicImportTask::doImport(rContext);
                    }
                    else
                        rContext.m_nStatus = ERRCODE_GRFILTER_FILTERERROR;
                }
                else if (aFilterName.equalsIgnoreAsciiCase(IMP_PNG))
                {
                    rContext.m_eLinkType = GfxLinkType::NativePng;

                    if (vcl::ImportPNG( *rContext.m_pStream, *rContext.m_pGraphic, rContext.m_nImportFlags | GraphicFilterImportFlags::OnlyCreateBitmap, nullptr, nullptr))
                    {
                        const BitmapEx& rBitmapEx = rContext.m_pGraphic->GetBitmapExRef();
                        Bitmap& rBitmap = const_cast<Bitmap&>(rBitmapEx.GetBitmap());
                        rContext.m_pAccess = std::make_unique<BitmapScopedWriteAccess>(rBitmap);
                        if(rBitmapEx.IsAlpha())
                        {
                            // The separate alpha bitmap causes a number of complications. Not only
                            // we need to have an extra bitmap access for it, but we also need
                            // to keep an AlphaMask instance in the context. This is because
                            // BitmapEx internally keeps Bitmap and not AlphaMask (because the Bitmap
                            // may be also a mask, not alpha). So BitmapEx::GetAlpha() returns
                            // a temporary, and direct access to the Bitmap wouldn't work
                            // with AlphaScopedBitmapAccess. *sigh*
                            rContext.mAlphaMask = rBitmapEx.GetAlpha();
                            rContext.m_pAlphaAccess = std::make_unique<AlphaScopedWriteAccess>(rContext.mAlphaMask);
                        }
                        rContext.m_pStream->Seek(rContext.m_nStreamBegin);
                        if (bThreads)
                            rSharedPool.pushTask(std::make_unique<GraphicImportTask>(pTag, rContext));
                        else
                            GraphicImportTask::doImport(rContext);
                    }
                    else
                        rContext.m_nStatus = ERRCODE_GRFILTER_FILTERERROR;
                }
                else
                    rContext.m_nStatus = ERRCODE_GRFILTER_FILTERERROR;
            }
        }
    }

    rSharedPool.waitUntilDone(pTag);

    // Process data after import.
    for (auto& rContext : aContexts)
    {
        if(rContext.m_pAlphaAccess) // Need to move the AlphaMask back to the BitmapEx.
            *rContext.m_pGraphic = BitmapEx( rContext.m_pGraphic->GetBitmapExRef().GetBitmap(), rContext.mAlphaMask );
        rContext.m_pAccess.reset();
        rContext.m_pAlphaAccess.reset();

        if (rContext.m_nStatus == ERRCODE_NONE && (rContext.m_eLinkType != GfxLinkType::NONE) && !rContext.m_pGraphic->GetReaderContext())
        {
            std::unique_ptr<sal_uInt8[]> pGraphicContent;

            const sal_uInt64 nStreamEnd = rContext.m_pStream->Tell();
            sal_Int32 nGraphicContentSize = nStreamEnd - rContext.m_nStreamBegin;

            if (nGraphicContentSize > 0)
            {
                try
                {
                    pGraphicContent.reset(new sal_uInt8[nGraphicContentSize]);
                }
                catch (const std::bad_alloc&)
                {
                    rContext.m_nStatus = ERRCODE_GRFILTER_TOOBIG;
                }

                if (rContext.m_nStatus == ERRCODE_NONE)
                {
                    rContext.m_pStream->Seek(rContext.m_nStreamBegin);
                    rContext.m_pStream->ReadBytes(pGraphicContent.get(), nGraphicContentSize);
                }
            }

            if (rContext.m_nStatus == ERRCODE_NONE)
                rContext.m_pGraphic->SetGfxLink(std::make_shared<GfxLink>(std::move(pGraphicContent), nGraphicContentSize, rContext.m_eLinkType));
        }

        if (rContext.m_nStatus != ERRCODE_NONE)
            rContext.m_pGraphic = nullptr;

        rGraphics.push_back(rContext.m_pGraphic);
    }
}

void GraphicFilter::MakeGraphicsAvailableThreaded(std::vector<Graphic*>& graphics)
{
    // Graphic::makeAvailable() is not thread-safe. Only the jpeg and png loaders are, so here
    // we process only jpeg and png images that also have their stream data, load new Graphic's
    // from them and then update the passed objects using them.
    std::vector< Graphic* > toLoad;
    for(auto graphic : graphics)
    {
        // Need to use GetSharedGfxLink, to access the pointer without copying.
        if(!graphic->isAvailable() && graphic->IsGfxLink()
            && graphic->GetSharedGfxLink()->GetDataSize() != 0
            && (graphic->GetSharedGfxLink()->GetType() == GfxLinkType::NativeJpg
                || graphic->GetSharedGfxLink()->GetType() == GfxLinkType::NativePng))
        {
            // Graphic objects share internal ImpGraphic, do not process any of those twice.
            const auto predicate = [graphic](Graphic* item) { return item->ImplGetImpGraphic() == graphic->ImplGetImpGraphic(); };
            if( std::find_if(toLoad.begin(), toLoad.end(), predicate ) == toLoad.end())
                toLoad.push_back( graphic );
        }
    }
    if( toLoad.empty())
        return;
    std::vector< std::unique_ptr<SvStream>> streams;
    for( auto graphic : toLoad )
    {
        streams.push_back( std::make_unique<SvMemoryStream>( const_cast<sal_uInt8*>(graphic->GetSharedGfxLink()->GetData()),
            graphic->GetSharedGfxLink()->GetDataSize(), StreamMode::READ | StreamMode::WRITE));
    }
    std::vector< std::shared_ptr<Graphic>> loadedGraphics;
    ImportGraphics(loadedGraphics, std::move(streams));
    assert(loadedGraphics.size() == toLoad.size());
    for( size_t i = 0; i < toLoad.size(); ++i )
    {
        if(loadedGraphics[ i ] != nullptr)
            toLoad[ i ]->ImplGetImpGraphic()->updateFromLoadedGraphic(loadedGraphics[ i ]->ImplGetImpGraphic());
    }
}

Graphic GraphicFilter::ImportUnloadedGraphic(SvStream& rIStream, sal_uInt64 sizeLimit,
                                             const Size* pSizeHint)
{
    Graphic aGraphic;
    sal_uInt16 nFormat = GRFILTER_FORMAT_DONTKNOW;
    GfxLinkType eLinkType = GfxLinkType::NONE;

    ResetLastError();

    const sal_uLong nStreamBegin = rIStream.Tell();

    rIStream.Seek(nStreamBegin);

    ErrCode nStatus = ImpTestOrFindFormat("", rIStream, nFormat);

    rIStream.Seek(nStreamBegin);
    sal_uInt32 nStreamLength(rIStream.remainingSize());
    if (sizeLimit && sizeLimit < nStreamLength)
        nStreamLength = sizeLimit;

    OUString aFilterName = pConfig->GetImportFilterName(nFormat);

    std::unique_ptr<sal_uInt8[]> pGraphicContent;
    sal_Int32 nGraphicContentSize = 0;

    // read graphic
    {
        if (aFilterName.equalsIgnoreAsciiCase(IMP_GIF))
        {
            eLinkType = GfxLinkType::NativeGif;
        }
        else if (aFilterName.equalsIgnoreAsciiCase(IMP_PNG))
        {
            // check if this PNG contains a GIF chunk!
            pGraphicContent = vcl::PngImageReader::getMicrosoftGifChunk(rIStream, &nGraphicContentSize);
            if( pGraphicContent )
                eLinkType = GfxLinkType::NativeGif;
            else
                eLinkType = GfxLinkType::NativePng;
        }
        else if (aFilterName.equalsIgnoreAsciiCase(IMP_JPEG))
        {
            eLinkType = GfxLinkType::NativeJpg;
        }
        else if (aFilterName.equalsIgnoreAsciiCase(IMP_SVG))
        {
            bool bOkay(false);

            if (nStreamLength > 0)
            {
                std::vector<sal_uInt8> aTwoBytes(2);
                rIStream.ReadBytes(aTwoBytes.data(), 2);
                rIStream.Seek(nStreamBegin);

                if (aTwoBytes[0] == 0x1F && aTwoBytes[1] == 0x8B)
                {
                    SvMemoryStream aMemStream;
                    ZCodec aCodec;
                    tools::Long nMemoryLength;

                    aCodec.BeginCompression(ZCODEC_DEFAULT_COMPRESSION, /*gzLib*/true);
                    nMemoryLength = aCodec.Decompress(rIStream, aMemStream);
                    aCodec.EndCompression();

                    if (!rIStream.GetError() && nMemoryLength >= 0)
                    {
                        nGraphicContentSize = nMemoryLength;
                        pGraphicContent.reset(new sal_uInt8[nGraphicContentSize]);

                        aMemStream.Seek(STREAM_SEEK_TO_BEGIN);
                        aMemStream.ReadBytes(pGraphicContent.get(), nGraphicContentSize);

                        bOkay = true;
                    }
                }
                else
                {
                    nGraphicContentSize = nStreamLength;
                    pGraphicContent.reset(new sal_uInt8[nGraphicContentSize]);
                    rIStream.ReadBytes(pGraphicContent.get(), nStreamLength);

                    bOkay = true;
                }
            }

            if (bOkay)
            {
                eLinkType = GfxLinkType::NativeSvg;
            }
            else
            {
                nStatus = ERRCODE_GRFILTER_FILTERERROR;
            }
        }
        else if (aFilterName.equalsIgnoreAsciiCase(IMP_BMP))
        {
            eLinkType = GfxLinkType::NativeBmp;
        }
        else if (aFilterName.equalsIgnoreAsciiCase(IMP_MOV))
        {
            eLinkType = GfxLinkType::NativeMov;
        }
        else if (aFilterName.equalsIgnoreAsciiCase(IMP_WMF) ||
                 aFilterName.equalsIgnoreAsciiCase(IMP_EMF))
        {
            nGraphicContentSize = nStreamLength;
            pGraphicContent.reset(new sal_uInt8[nGraphicContentSize]);

            rIStream.Seek(nStreamBegin);
            rIStream.ReadBytes(pGraphicContent.get(), nStreamLength);

            if (!rIStream.GetError())
            {
                eLinkType = GfxLinkType::NativeWmf;
            }
            else
            {
                nStatus = ERRCODE_GRFILTER_FILTERERROR;
            }
        }
        else if (aFilterName == IMP_PDF)
        {
            eLinkType = GfxLinkType::NativePdf;
        }
        else if (aFilterName == IMP_TIFF)
        {
            eLinkType = GfxLinkType::NativeTif;
        }
        else if (aFilterName == IMP_PICT)
        {
            eLinkType = GfxLinkType::NativePct;
        }
        else if (aFilterName == IMP_MET)
        {
            eLinkType = GfxLinkType::NativeMet;
        }
        else
        {
            nStatus = ERRCODE_GRFILTER_FILTERERROR;
        }
    }

    if (nStatus == ERRCODE_NONE && eLinkType != GfxLinkType::NONE)
    {
        if (!pGraphicContent)
        {
            nGraphicContentSize = nStreamLength;

            if (nGraphicContentSize > 0)
            {
                try
                {
                    pGraphicContent.reset(new sal_uInt8[nGraphicContentSize]);
                }
                catch (const std::bad_alloc&)
                {
                    nStatus = ERRCODE_GRFILTER_TOOBIG;
                }

                if (nStatus == ERRCODE_NONE)
                {
                    rIStream.Seek(nStreamBegin);
                    nGraphicContentSize = rIStream.ReadBytes(pGraphicContent.get(), nGraphicContentSize);
                }
            }
        }

        if( nStatus == ERRCODE_NONE )
        {
            bool bAnimated = false;
            Size aLogicSize;
            if (eLinkType == GfxLinkType::NativeGif)
            {
                SvMemoryStream aMemoryStream(pGraphicContent.get(), nGraphicContentSize, StreamMode::READ);
                bAnimated = IsGIFAnimated(aMemoryStream, aLogicSize);
                if (!pSizeHint && aLogicSize.getWidth() && aLogicSize.getHeight())
                {
                    pSizeHint = &aLogicSize;
                }
            }
            aGraphic.SetGfxLink(std::make_shared<GfxLink>(std::move(pGraphicContent), nGraphicContentSize, eLinkType));
            aGraphic.ImplGetImpGraphic()->setPrepared(bAnimated, pSizeHint);
        }
    }

    // Set error code or try to set native buffer
    if (nStatus != ERRCODE_NONE)
        ImplSetError(nStatus, &rIStream);
    if (nStatus != ERRCODE_NONE || eLinkType == GfxLinkType::NONE)
        rIStream.Seek(nStreamBegin);

    return aGraphic;
}

ErrCode GraphicFilter::readGIF(SvStream & rStream, Graphic & rGraphic, GfxLinkType & rLinkType)
{
    if (ImportGIF(rStream, rGraphic))
    {
        rLinkType = GfxLinkType::NativeGif;
        return ERRCODE_NONE;
    }
    else
        return ERRCODE_GRFILTER_FILTERERROR;
}

ErrCode GraphicFilter::readPNG(SvStream & rStream, Graphic & rGraphic, GfxLinkType & rLinkType, std::unique_ptr<sal_uInt8[]> & rpGraphicContent,
    sal_Int32& rGraphicContentSize)
{
    ErrCode aReturnCode = ERRCODE_NONE;

    // check if this PNG contains a GIF chunk!
    rpGraphicContent = vcl::PngImageReader::getMicrosoftGifChunk(rStream, &rGraphicContentSize);
    if( rpGraphicContent )
    {
        SvMemoryStream aIStrm(rpGraphicContent.get(), rGraphicContentSize, StreamMode::READ);
        ImportGIF(aIStrm, rGraphic);
        rLinkType = GfxLinkType::NativeGif;
        return aReturnCode;
    }

    // PNG has no GIF chunk
    vcl::PngImageReader aPNGReader(rStream);
    BitmapEx aBitmapEx(aPNGReader.read());
    if (!aBitmapEx.IsEmpty())
    {
        rGraphic = aBitmapEx;
        rLinkType = GfxLinkType::NativePng;
    }
    else
        aReturnCode = ERRCODE_GRFILTER_FILTERERROR;

    return aReturnCode;
}

ErrCode GraphicFilter::readJPEG(SvStream & rStream, Graphic & rGraphic, GfxLinkType & rLinkType, GraphicFilterImportFlags nImportFlags)
{
    ErrCode aReturnCode = ERRCODE_NONE;

    // set LOGSIZE flag always, if not explicitly disabled
    // (see #90508 and #106763)
    if (!(nImportFlags & GraphicFilterImportFlags::DontSetLogsizeForJpeg))
    {
        nImportFlags |= GraphicFilterImportFlags::SetLogsizeForJpeg;
    }

    sal_uInt64 nPosition = rStream.Tell();
    if (!ImportJPEG(rStream, rGraphic, nImportFlags | GraphicFilterImportFlags::OnlyCreateBitmap, nullptr))
        aReturnCode = ERRCODE_GRFILTER_FILTERERROR;
    else
    {
        Bitmap& rBitmap = const_cast<Bitmap&>(rGraphic.GetBitmapExRef().GetBitmap());
        BitmapScopedWriteAccess pWriteAccess(rBitmap);
        rStream.Seek(nPosition);
        if (!ImportJPEG(rStream, rGraphic, nImportFlags | GraphicFilterImportFlags::UseExistingBitmap, &pWriteAccess))
            aReturnCode = ERRCODE_GRFILTER_FILTERERROR;
        else
            rLinkType = GfxLinkType::NativeJpg;
    }

    return aReturnCode;
}

ErrCode GraphicFilter::readSVG(SvStream & rStream, Graphic & rGraphic, GfxLinkType & rLinkType, std::unique_ptr<sal_uInt8[]> & rpGraphicContent,
    sal_Int32& rGraphicContentSize)
{
    ErrCode aReturnCode = ERRCODE_NONE;

    const sal_uInt32 nStreamPosition(rStream.Tell());
    const sal_uInt32 nStreamLength(rStream.remainingSize());

    bool bOkay(false);

    if (nStreamLength > 0)
    {
        std::vector<sal_uInt8> aTwoBytes(2);
        rStream.ReadBytes(aTwoBytes.data(), 2);
        rStream.Seek(nStreamPosition);

        if (aTwoBytes[0] == 0x1F && aTwoBytes[1] == 0x8B)
        {
            SvMemoryStream aMemStream;
            ZCodec aCodec;
            tools::Long nMemoryLength;

            aCodec.BeginCompression(ZCODEC_DEFAULT_COMPRESSION, /*gzLib*/true);
            nMemoryLength = aCodec.Decompress(rStream, aMemStream);
            aCodec.EndCompression();

            if (!rStream.GetError() && nMemoryLength >= 0)
            {
                VectorGraphicDataArray aNewData(nMemoryLength);
                aMemStream.Seek(STREAM_SEEK_TO_BEGIN);
                aMemStream.ReadBytes(aNewData.getArray(), nMemoryLength);

                // Make a uncompressed copy for GfxLink
                rGraphicContentSize = nMemoryLength;
                rpGraphicContent.reset(new sal_uInt8[rGraphicContentSize]);
                std::copy(std::cbegin(aNewData), std::cend(aNewData), rpGraphicContent.get());

                if (!aMemStream.GetError())
                {
                    BinaryDataContainer aDataContainer(reinterpret_cast<const sal_uInt8*>(aNewData.getConstArray()), aNewData.getLength());
                    auto aVectorGraphicDataPtr = std::make_shared<VectorGraphicData>(aDataContainer, VectorGraphicDataType::Svg);
                    rGraphic = Graphic(aVectorGraphicDataPtr);
                    bOkay = true;
                }
            }
        }
        else
        {
            VectorGraphicDataArray aNewData(nStreamLength);
            rStream.ReadBytes(aNewData.getArray(), nStreamLength);

            if (!rStream.GetError())
            {
                BinaryDataContainer aDataContainer(reinterpret_cast<const sal_uInt8*>(aNewData.getConstArray()), aNewData.getLength());
                auto aVectorGraphicDataPtr = std::make_shared<VectorGraphicData>(aDataContainer, VectorGraphicDataType::Svg);
                rGraphic = Graphic(aVectorGraphicDataPtr);
                bOkay = true;
            }
        }
    }

    if (bOkay)
    {
        rLinkType = GfxLinkType::NativeSvg;
    }
    else
    {
        aReturnCode = ERRCODE_GRFILTER_FILTERERROR;
    }

    return aReturnCode;
}

ErrCode GraphicFilter::readXBM(SvStream & rStream, Graphic & rGraphic)
{
    if (ImportXBM(rStream, rGraphic))
        return ERRCODE_NONE;
    else
        return ERRCODE_GRFILTER_FILTERERROR;
}

ErrCode GraphicFilter::readXPM(SvStream & rStream, Graphic & rGraphic)
{
    if (ImportXPM(rStream, rGraphic))
        return ERRCODE_NONE;
    else
        return ERRCODE_GRFILTER_FILTERERROR;
}

ErrCode GraphicFilter::readWMF_EMF(SvStream & rStream, Graphic & rGraphic, GfxLinkType & rLinkType,
                                   WmfExternal const *pExtHeader, VectorGraphicDataType eType)
{
    // use new UNO API service, do not directly import but create a
    // Graphic that contains the original data and decomposes to
    // primitives on demand

    ErrCode aReturnCode = ERRCODE_GRFILTER_FILTERERROR;

    const sal_uInt32 nStreamLength(rStream.remainingSize());
    VectorGraphicDataArray aNewData(nStreamLength);

    rStream.ReadBytes(aNewData.getArray(), nStreamLength);

    if (!rStream.GetError())
    {
        const VectorGraphicDataType aDataType(eType);
        BinaryDataContainer aDataContainer(reinterpret_cast<const sal_uInt8*>(aNewData.getConstArray()), aNewData.getLength());

        auto aVectorGraphicDataPtr = std::make_shared<VectorGraphicData>(aDataContainer, aDataType);

        if (pExtHeader)
        {
            aVectorGraphicDataPtr->setWmfExternalHeader(*pExtHeader);
        }

        rGraphic = Graphic(aVectorGraphicDataPtr);
        rLinkType = GfxLinkType::NativeWmf;
        aReturnCode = ERRCODE_NONE;
    }

    return aReturnCode;
}

ErrCode GraphicFilter::readWMF(SvStream & rStream, Graphic & rGraphic, GfxLinkType & rLinkType, WmfExternal const* pExtHeader)
{
    return readWMF_EMF(rStream, rGraphic, rLinkType, pExtHeader, VectorGraphicDataType::Wmf);
}

ErrCode GraphicFilter::readEMF(SvStream & rStream, Graphic & rGraphic, GfxLinkType & rLinkType, WmfExternal const* pExtHeader)
{
    return readWMF_EMF(rStream, rGraphic, rLinkType, pExtHeader, VectorGraphicDataType::Emf);
}

ErrCode GraphicFilter::readPDF(SvStream & rStream, Graphic & rGraphic, GfxLinkType & rLinkType)
{
    if (vcl::ImportPDF(rStream, rGraphic))
    {
        rLinkType = GfxLinkType::NativePdf;
        return ERRCODE_NONE;
    }
    else
        return ERRCODE_GRFILTER_FILTERERROR;
}

ErrCode GraphicFilter::readTIFF(SvStream & rStream, Graphic & rGraphic, GfxLinkType & rLinkType)
{
    if (ImportTiffGraphicImport(rStream, rGraphic))
    {
        rLinkType = GfxLinkType::NativeTif;
        return ERRCODE_NONE;
    }
    else
        return ERRCODE_GRFILTER_FILTERERROR;
}

ErrCode GraphicFilter::readWithTypeSerializer(SvStream & rStream, Graphic & rGraphic, GfxLinkType & rLinkType, OUString aFilterName)
{
    ErrCode aReturnCode = ERRCODE_GRFILTER_FILTERERROR;

    // SV internal filters for import bitmaps and MetaFiles
    TypeSerializer aSerializer(rStream);
    aSerializer.readGraphic(rGraphic);

    if (!rStream.GetError())
    {
        if (aFilterName.equalsIgnoreAsciiCase(IMP_MOV))
        {
            rGraphic.SetDefaultType();
            rStream.Seek(STREAM_SEEK_TO_END);
            rLinkType = GfxLinkType::NativeMov;
        }
        aReturnCode = ERRCODE_NONE;
    }
    return aReturnCode;
}

ErrCode GraphicFilter::readBMP(SvStream & rStream, Graphic & rGraphic, GfxLinkType & rLinkType)
{
    if (BmpReader(rStream, rGraphic))
    {
        rLinkType = GfxLinkType::NativeBmp;
        return ERRCODE_NONE;
    }
    else
        return ERRCODE_GRFILTER_FILTERERROR;
}

ErrCode GraphicFilter::readTGA(SvStream & rStream, Graphic & rGraphic)
{
    if (ImportTgaGraphic(rStream, rGraphic))
        return ERRCODE_NONE;
    else
        return ERRCODE_GRFILTER_FILTERERROR;
}

ErrCode GraphicFilter::readPICT(SvStream & rStream, Graphic & rGraphic, GfxLinkType & rLinkType)
{
    if (ImportPictGraphic(rStream, rGraphic))
    {
        rLinkType = GfxLinkType::NativePct;
        return ERRCODE_NONE;
    }
    else
        return ERRCODE_GRFILTER_FILTERERROR;
}

ErrCode GraphicFilter::readMET(SvStream & rStream, Graphic & rGraphic, GfxLinkType & rLinkType)
{
    if (ImportMetGraphic(rStream, rGraphic))
    {
        rLinkType = GfxLinkType::NativeMet;
        return ERRCODE_NONE;
    }
    else
        return ERRCODE_GRFILTER_FILTERERROR;
}

ErrCode GraphicFilter::readRAS(SvStream & rStream, Graphic & rGraphic)
{
    if (ImportRasGraphic(rStream, rGraphic))
        return ERRCODE_NONE;
    else
        return ERRCODE_GRFILTER_FILTERERROR;
}

ErrCode GraphicFilter::readPCX(SvStream & rStream, Graphic & rGraphic)
{
    if (ImportPcxGraphic(rStream, rGraphic))
        return ERRCODE_NONE;
    else
        return ERRCODE_GRFILTER_FILTERERROR;
}

ErrCode GraphicFilter::readEPS(SvStream & rStream, Graphic & rGraphic)
{
    if (ImportEpsGraphic(rStream, rGraphic))
        return ERRCODE_NONE;
    else
        return ERRCODE_GRFILTER_FILTERERROR;
}

ErrCode GraphicFilter::readPSD(SvStream & rStream, Graphic & rGraphic)
{
    if (ImportPsdGraphic(rStream, rGraphic))
        return ERRCODE_NONE;
    else
        return ERRCODE_GRFILTER_FILTERERROR;
}

ErrCode GraphicFilter::readPCD(SvStream & rStream, Graphic & rGraphic)
{
    std::unique_ptr<FilterConfigItem> pFilterConfigItem;
    if (!utl::ConfigManager::IsFuzzing())
    {
        OUString aFilterConfigPath( "Office.Common/Filter/Graphic/Import/PCD" );
        pFilterConfigItem = std::make_unique<FilterConfigItem>(aFilterConfigPath);
    }

    if (ImportPcdGraphic(rStream, rGraphic, pFilterConfigItem.get()))
        return ERRCODE_NONE;
    else
        return ERRCODE_GRFILTER_FILTERERROR;
}

ErrCode GraphicFilter::readPBM(SvStream & rStream, Graphic & rGraphic)
{
    if (ImportPbmGraphic(rStream, rGraphic))
        return ERRCODE_NONE;
    else
        return ERRCODE_GRFILTER_FILTERERROR;
}

ErrCode GraphicFilter::readDXF(SvStream & rStream, Graphic & rGraphic)
{
    if (ImportDxfGraphic(rStream, rGraphic))
        return ERRCODE_NONE;
    else
        return ERRCODE_GRFILTER_FILTERERROR;
}

ErrCode GraphicFilter::ImportGraphic( Graphic& rGraphic, const OUString& rPath, SvStream& rIStream,
                                     sal_uInt16 nFormat, sal_uInt16* pDeterminedFormat, GraphicFilterImportFlags nImportFlags,
                                     const css::uno::Sequence< css::beans::PropertyValue >* /*pFilterData*/,
                                     WmfExternal const *pExtHeader )
{
    OUString aFilterName;
    sal_uInt64 nStreamBegin;
    ErrCode nStatus;
    GfxLinkType eLinkType = GfxLinkType::NONE;
    const bool bLinkSet = rGraphic.IsGfxLink();

    std::unique_ptr<sal_uInt8[]> pGraphicContent;
    sal_Int32  nGraphicContentSize = 0;

    ResetLastError();

    std::shared_ptr<GraphicReader> pContext = rGraphic.GetReaderContext();
    bool  bDummyContext = rGraphic.IsDummyContext();
    if( !pContext || bDummyContext )
    {
        if( bDummyContext )
        {
            rGraphic.SetDummyContext( false );
            nStreamBegin = 0;
        }
        else
            nStreamBegin = rIStream.Tell();

        nStatus = ImpTestOrFindFormat( rPath, rIStream, nFormat );
        // if pending, return ERRCODE_NONE in order to request more bytes
        if( rIStream.GetError() == ERRCODE_IO_PENDING )
        {
            rGraphic.SetDummyContext(true);
            rIStream.ResetError();
            rIStream.Seek( nStreamBegin );
            return ImplSetError( ERRCODE_NONE );
        }

        rIStream.Seek( nStreamBegin );

        if( ( nStatus != ERRCODE_NONE ) || rIStream.GetError() )
            return ImplSetError( ( nStatus != ERRCODE_NONE ) ? nStatus : ERRCODE_GRFILTER_OPENERROR, &rIStream );

        if( pDeterminedFormat )
            *pDeterminedFormat = nFormat;

        aFilterName = pConfig->GetImportFilterName( nFormat );
    }
    else
    {
        aFilterName = pContext->GetUpperFilterName();

        nStreamBegin = 0;
        nStatus = ERRCODE_NONE;
    }

    // read graphic
    {
        if (aFilterName.equalsIgnoreAsciiCase(IMP_GIF))
        {
            nStatus = readGIF(rIStream, rGraphic, eLinkType);
        }
        else if (aFilterName.equalsIgnoreAsciiCase(IMP_PNG))
        {
            nStatus = readPNG(rIStream, rGraphic, eLinkType, pGraphicContent, nGraphicContentSize);
        }
        else if (aFilterName.equalsIgnoreAsciiCase(IMP_JPEG))
        {
            nStatus = readJPEG(rIStream, rGraphic, eLinkType, nImportFlags);
        }
        else if (aFilterName.equalsIgnoreAsciiCase(IMP_SVG))
        {
            nStatus = readSVG(rIStream, rGraphic, eLinkType, pGraphicContent, nGraphicContentSize);
        }
        else if( aFilterName.equalsIgnoreAsciiCase( IMP_XBM ) )
        {
            nStatus = readXBM(rIStream, rGraphic);
        }
        else if( aFilterName.equalsIgnoreAsciiCase( IMP_XPM ) )
        {
            nStatus = readXPM(rIStream, rGraphic);
        }
        else if (aFilterName.equalsIgnoreAsciiCase(IMP_BMP))
        {
            nStatus = readBMP(rIStream, rGraphic, eLinkType);
        }
        else if (aFilterName.equalsIgnoreAsciiCase(IMP_SVMETAFILE))
        {
            nStatus = readWithTypeSerializer(rIStream, rGraphic, eLinkType, aFilterName);
        }
        else if( aFilterName.equalsIgnoreAsciiCase(IMP_MOV))
        {
            nStatus = readWithTypeSerializer(rIStream, rGraphic, eLinkType, aFilterName);
        }
        else if (aFilterName.equalsIgnoreAsciiCase(IMP_WMF))
        {
            nStatus = readWMF(rIStream, rGraphic, eLinkType, pExtHeader);
        }
        else if (aFilterName.equalsIgnoreAsciiCase(IMP_EMF))
        {
            nStatus = readEMF(rIStream, rGraphic, eLinkType, pExtHeader);
        }
        else if (aFilterName.equalsIgnoreAsciiCase(IMP_PDF))
        {
            nStatus = readPDF(rIStream, rGraphic, eLinkType);
        }
        else if (aFilterName.equalsIgnoreAsciiCase(IMP_TIFF) )
        {
            nStatus = readTIFF(rIStream, rGraphic, eLinkType);
        }
        else if (aFilterName.equalsIgnoreAsciiCase(IMP_TGA) )
        {
            nStatus = readTGA(rIStream, rGraphic);
        }
        else if (aFilterName.equalsIgnoreAsciiCase(IMP_PICT))
        {
            nStatus = readPICT(rIStream, rGraphic, eLinkType);
        }
        else if (aFilterName.equalsIgnoreAsciiCase(IMP_MET))
        {
            nStatus = readMET(rIStream, rGraphic, eLinkType);
        }
        else if (aFilterName.equalsIgnoreAsciiCase(IMP_RAS))
        {
            nStatus = readRAS(rIStream, rGraphic);
        }
        else if (aFilterName.equalsIgnoreAsciiCase(IMP_PCX))
        {
            nStatus = readPCX(rIStream, rGraphic);
        }
        else if (aFilterName.equalsIgnoreAsciiCase(IMP_EPS))
        {
            nStatus = readEPS(rIStream, rGraphic);
        }
        else if (aFilterName.equalsIgnoreAsciiCase(IMP_PSD))
        {
            nStatus = readPSD(rIStream, rGraphic);
        }
        else if (aFilterName.equalsIgnoreAsciiCase(IMP_PCD))
        {
            nStatus = readPCD(rIStream, rGraphic);
        }
        else if (aFilterName.equalsIgnoreAsciiCase(IMP_PBM))
        {
            nStatus = readPBM(rIStream, rGraphic);
        }
        else if (aFilterName.equalsIgnoreAsciiCase(IMP_DXF))
        {
            nStatus = readDXF(rIStream, rGraphic);
        }
        else
            nStatus = ERRCODE_GRFILTER_FILTERERROR;
    }

    if( nStatus == ERRCODE_NONE && ( eLinkType != GfxLinkType::NONE ) && !rGraphic.GetReaderContext() && !bLinkSet )
    {
        if (!pGraphicContent)
        {
            const sal_uInt64 nStreamEnd = rIStream.Tell();
            nGraphicContentSize = nStreamEnd - nStreamBegin;

            if (nGraphicContentSize > 0)
            {
                try
                {
                    pGraphicContent.reset(new sal_uInt8[nGraphicContentSize]);
                }
                catch (const std::bad_alloc&)
                {
                    nStatus = ERRCODE_GRFILTER_TOOBIG;
                }

                if( nStatus == ERRCODE_NONE )
                {
                    rIStream.Seek(nStreamBegin);
                    rIStream.ReadBytes(pGraphicContent.get(), nGraphicContentSize);
                }
            }
        }
        if( nStatus == ERRCODE_NONE )
        {
            rGraphic.SetGfxLink(std::make_shared<GfxLink>(std::move(pGraphicContent), nGraphicContentSize, eLinkType));
        }
    }

    // Set error code or try to set native buffer
    if( nStatus != ERRCODE_NONE )
    {
        ImplSetError( nStatus, &rIStream );
        rIStream.Seek( nStreamBegin );
        rGraphic.Clear();
    }

    return nStatus;
}

ErrCode GraphicFilter::ExportGraphic( const Graphic& rGraphic, const INetURLObject& rPath,
    sal_uInt16 nFormat, const css::uno::Sequence< css::beans::PropertyValue >* pFilterData )
{
    SAL_INFO( "vcl.filter", "GraphicFilter::ExportGraphic() (thb)" );
    ErrCode  nRetValue = ERRCODE_GRFILTER_FORMATERROR;
    SAL_WARN_IF( rPath.GetProtocol() == INetProtocol::NotValid, "vcl.filter", "GraphicFilter::ExportGraphic() : ProtType == INetProtocol::NotValid" );

    OUString aMainUrl(rPath.GetMainURL(INetURLObject::DecodeMechanism::NONE));
    bool bAlreadyExists = utl::UCBContentHelper::IsDocument(aMainUrl);

    std::unique_ptr<SvStream> xStream(::utl::UcbStreamHelper::CreateStream( aMainUrl, StreamMode::WRITE | StreamMode::TRUNC ));
    if (xStream)
    {
        nRetValue = ExportGraphic( rGraphic, aMainUrl, *xStream, nFormat, pFilterData );
        xStream.reset();

        if( ( ERRCODE_NONE != nRetValue ) && !bAlreadyExists )
            utl::UCBContentHelper::Kill(aMainUrl);
    }
    return nRetValue;
}

ErrCode GraphicFilter::ExportGraphic( const Graphic& rGraphic, const OUString& rPath,
    SvStream& rOStm, sal_uInt16 nFormat, const css::uno::Sequence< css::beans::PropertyValue >* pFilterData )
{
    SAL_INFO( "vcl.filter", "GraphicFilter::ExportGraphic() (thb)" );
    sal_uInt16 nFormatCount = GetExportFormatCount();

    ResetLastError();

    if( nFormat == GRFILTER_FORMAT_DONTKNOW )
    {
        INetURLObject aURL( rPath );
        OUString aExt( aURL.GetFileExtension().toAsciiUpperCase() );

        for( sal_uInt16 i = 0; i < nFormatCount; i++ )
        {
            if ( pConfig->GetExportFormatExtension( i ).equalsIgnoreAsciiCase( aExt ) )
            {
                nFormat=i;
                break;
            }
        }
    }
    if( nFormat >= nFormatCount )
        return ImplSetError( ERRCODE_GRFILTER_FORMATERROR );

    FilterConfigItem aConfigItem( pFilterData );
    OUString aFilterName( pConfig->GetExportFilterName( nFormat ) );
    ErrCode     nStatus = ERRCODE_NONE;
    GraphicType eType;
    Graphic     aGraphic = ImpGetScaledGraphic( rGraphic, aConfigItem );
    eType = aGraphic.GetType();

    if( pConfig->IsExportPixelFormat( nFormat ) )
    {
        if( eType != GraphicType::Bitmap )
        {
            Size aSizePixel;
            sal_uLong nBitsPerPixel,nNeededMem,nMaxMem;
            ScopedVclPtrInstance< VirtualDevice > aVirDev;

            nMaxMem = 1024;
            nMaxMem *= 1024; // In Bytes

            // Calculate how big the image would normally be:
            aSizePixel=aVirDev->LogicToPixel(aGraphic.GetPrefSize(),aGraphic.GetPrefMapMode());

            // Calculate how much memory the image will take up
            nBitsPerPixel=aVirDev->GetBitCount();
            nNeededMem=(static_cast<sal_uLong>(aSizePixel.Width())*static_cast<sal_uLong>(aSizePixel.Height())*nBitsPerPixel+7)/8;

            // is the image larger than available memory?
            if (nMaxMem<nNeededMem)
            {
                double fFak=sqrt(static_cast<double>(nMaxMem)/static_cast<double>(nNeededMem));
                aSizePixel.setWidth(static_cast<sal_uLong>(static_cast<double>(aSizePixel.Width())*fFak) );
                aSizePixel.setHeight(static_cast<sal_uLong>(static_cast<double>(aSizePixel.Height())*fFak) );
            }

            aVirDev->SetMapMode(MapMode(MapUnit::MapPixel));
            aVirDev->SetOutputSizePixel(aSizePixel);
            Graphic aGraphic2=aGraphic;
            aGraphic2.Draw(*aVirDev, Point(0, 0), aSizePixel); // this changes the MapMode
            aVirDev->SetMapMode(MapMode(MapUnit::MapPixel));
            aGraphic=Graphic(aVirDev->GetBitmapEx(Point(0,0),aSizePixel));
        }
    }
    if( rOStm.GetError() )
        nStatus = ERRCODE_GRFILTER_IOERROR;
    if( ERRCODE_NONE == nStatus )
    {
            if( aFilterName.equalsIgnoreAsciiCase( EXP_BMP ) )
            {
                if (!BmpWriter(rOStm, aGraphic, &aConfigItem))
                    nStatus = ERRCODE_GRFILTER_FORMATERROR;
                if (rOStm.GetError())
                    nStatus = ERRCODE_GRFILTER_IOERROR;
            }
            else if (aFilterName.equalsIgnoreAsciiCase(EXP_TIFF))
            {
                if (!ExportTiffGraphicImport(rOStm, aGraphic, &aConfigItem))
                    nStatus = ERRCODE_GRFILTER_FORMATERROR;

                if( rOStm.GetError() )
                    nStatus = ERRCODE_GRFILTER_IOERROR;
            }
            else if (aFilterName.equalsIgnoreAsciiCase(EXP_GIF))
            {
                if (!ExportGifGraphic(rOStm, aGraphic, &aConfigItem))
                    nStatus = ERRCODE_GRFILTER_FORMATERROR;

                if( rOStm.GetError() )
                    nStatus = ERRCODE_GRFILTER_IOERROR;
            }
            else if( aFilterName.equalsIgnoreAsciiCase( EXP_SVMETAFILE ) )
            {
                sal_Int32 nVersion = aConfigItem.ReadInt32( "Version", 0 ) ;
                if ( nVersion )
                    rOStm.SetVersion( nVersion );

                // #i119735# just use GetGDIMetaFile, it will create a buffered version of contained bitmap now automatically
                GDIMetaFile aMTF(aGraphic.GetGDIMetaFile());

                SvmWriter aWriter( rOStm );
                aWriter.Write( aMTF );

                if( rOStm.GetError() )
                    nStatus = ERRCODE_GRFILTER_IOERROR;
            }
            else if ( aFilterName.equalsIgnoreAsciiCase( EXP_WMF ) )
            {
                bool bDone(false);

                // do we have a native Vector Graphic Data RenderGraphic, whose data can be written directly?
                auto const & rVectorGraphicDataPtr(rGraphic.getVectorGraphicData());

                bool bIsEMF = rGraphic.GetGfxLink().IsEMF();

                // VectorGraphicDataType::Wmf means WMF or EMF, allow direct write in the WMF case
                // only.
                if (rVectorGraphicDataPtr
                    && rVectorGraphicDataPtr->getType() == VectorGraphicDataType::Wmf
                    && !rVectorGraphicDataPtr->getBinaryDataContainer().isEmpty()
                    && !bIsEMF)
                {
                    auto & aDataContainer = rVectorGraphicDataPtr->getBinaryDataContainer();
                    rOStm.WriteBytes(aDataContainer.getData(), aDataContainer.getSize());

                    if (rOStm.GetError())
                    {
                        nStatus = ERRCODE_GRFILTER_IOERROR;
                    }
                    else
                    {
                        bDone = true;
                    }
                }

                if (!bDone)
                {
                    // #i119735# just use GetGDIMetaFile, it will create a buffered version of contained bitmap now automatically
                    if (!ConvertGraphicToWMF(aGraphic, rOStm, &aConfigItem))
                        nStatus = ERRCODE_GRFILTER_FORMATERROR;

                    if (rOStm.GetError())
                        nStatus = ERRCODE_GRFILTER_IOERROR;
                }
            }
            else if ( aFilterName.equalsIgnoreAsciiCase( EXP_EMF ) )
            {
                bool bDone(false);

                // do we have a native Vector Graphic Data RenderGraphic, whose data can be written directly?
                auto const & rVectorGraphicDataPtr(rGraphic.getVectorGraphicData());

                if (rVectorGraphicDataPtr
                    && rVectorGraphicDataPtr->getType() == VectorGraphicDataType::Emf
                    && !rVectorGraphicDataPtr->getBinaryDataContainer().isEmpty())
                {
                    auto & aDataContainer = rVectorGraphicDataPtr->getBinaryDataContainer();
                    rOStm.WriteBytes(aDataContainer.getData(), aDataContainer.getSize());

                    if (rOStm.GetError())
                    {
                        nStatus = ERRCODE_GRFILTER_IOERROR;
                    }
                    else
                    {
                        bDone = true;
                    }
                }

                if (!bDone)
                {
                    // #i119735# just use GetGDIMetaFile, it will create a buffered version of contained bitmap now automatically
                    if (!ConvertGDIMetaFileToEMF(aGraphic.GetGDIMetaFile(), rOStm))
                        nStatus = ERRCODE_GRFILTER_FORMATERROR;

                    if (rOStm.GetError())
                        nStatus = ERRCODE_GRFILTER_IOERROR;
                }
            }
            else if( aFilterName.equalsIgnoreAsciiCase( EXP_JPEG ) )
            {
                bool bExportedGrayJPEG = false;
                if( !ExportJPEG( rOStm, aGraphic, pFilterData, &bExportedGrayJPEG ) )
                    nStatus = ERRCODE_GRFILTER_FORMATERROR;

                if( rOStm.GetError() )
                    nStatus = ERRCODE_GRFILTER_IOERROR;
            }
            else if (aFilterName.equalsIgnoreAsciiCase(EXP_EPS))
            {
                if (!ExportEpsGraphic(rOStm, aGraphic, &aConfigItem))
                    nStatus = ERRCODE_GRFILTER_FORMATERROR;

                if (rOStm.GetError())
                    nStatus = ERRCODE_GRFILTER_IOERROR;
            }
            else if ( aFilterName.equalsIgnoreAsciiCase( EXP_PNG ) )
            {
                vcl::PNGWriter aPNGWriter( aGraphic.GetBitmapEx(), pFilterData );
                if ( pFilterData )
                {
                    for ( const auto& rPropVal : *pFilterData )
                    {
                        if ( rPropVal.Name == "AdditionalChunks" )
                        {
                            css::uno::Sequence< css::beans::PropertyValue > aAdditionalChunkSequence;
                            if ( rPropVal.Value >>= aAdditionalChunkSequence )
                            {
                                for ( const auto& rAdditionalChunk : std::as_const(aAdditionalChunkSequence) )
                                {
                                    if ( rAdditionalChunk.Name.getLength() == 4 )
                                    {
                                        sal_uInt32 nChunkType = 0;
                                        for ( sal_Int32 k = 0; k < 4; k++ )
                                        {
                                            nChunkType <<= 8;
                                            nChunkType |= static_cast<sal_uInt8>(rAdditionalChunk.Name[ k ]);
                                        }
                                        css::uno::Sequence< sal_Int8 > aByteSeq;
                                        if ( rAdditionalChunk.Value >>= aByteSeq )
                                        {
                                            std::vector< vcl::PNGWriter::ChunkData >& rChunkData = aPNGWriter.GetChunks();
                                            if ( !rChunkData.empty() )
                                            {
                                                sal_uInt32 nChunkLen = aByteSeq.getLength();

                                                vcl::PNGWriter::ChunkData aChunkData;
                                                aChunkData.nType = nChunkType;
                                                if ( nChunkLen )
                                                {
                                                    aChunkData.aData.resize( nChunkLen );
                                                    memcpy( aChunkData.aData.data(), aByteSeq.getConstArray(), nChunkLen );
                                                }
                                                std::vector< vcl::PNGWriter::ChunkData >::iterator aIter = rChunkData.end() - 1;
                                                rChunkData.insert( aIter, aChunkData );
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                aPNGWriter.Write( rOStm );

                if( rOStm.GetError() )
                    nStatus = ERRCODE_GRFILTER_IOERROR;
            }
            else if( aFilterName.equalsIgnoreAsciiCase( EXP_SVG ) )
            {
                bool bDone(false);

                // do we have a native Vector Graphic Data RenderGraphic, whose data can be written directly?
                auto const & rVectorGraphicDataPtr(rGraphic.getVectorGraphicData());

                if (rVectorGraphicDataPtr
                    && rVectorGraphicDataPtr->getType()  ==  VectorGraphicDataType::Svg
                    && !rVectorGraphicDataPtr->getBinaryDataContainer().isEmpty())
                {
                    auto & aDataContainer = rVectorGraphicDataPtr->getBinaryDataContainer();
                    rOStm.WriteBytes(aDataContainer.getData(), aDataContainer.getSize());

                    if( rOStm.GetError() )
                    {
                        nStatus = ERRCODE_GRFILTER_IOERROR;
                    }
                    else
                    {
                        bDone = true;
                    }
                }

                if( !bDone )
                {
                    // do the normal GDIMetaFile export instead
                    try
                    {
                        css::uno::Reference< css::uno::XComponentContext > xContext( ::comphelper::getProcessComponentContext() );

                        css::uno::Reference< css::xml::sax::XDocumentHandler > xSaxWriter(
                            css::xml::sax::Writer::create( xContext ), css::uno::UNO_QUERY_THROW);
                        css::uno::Sequence< css::uno::Any > aArguments{ css::uno::Any(
                            aConfigItem.GetFilterData()) };
                        css::uno::Reference< css::svg::XSVGWriter > xSVGWriter(
                            xContext->getServiceManager()->createInstanceWithArgumentsAndContext( "com.sun.star.svg.SVGWriter", aArguments, xContext),
                                css::uno::UNO_QUERY );
                        if( xSaxWriter.is() && xSVGWriter.is() )
                        {
                            css::uno::Reference< css::io::XActiveDataSource > xActiveDataSource(
                                xSaxWriter, css::uno::UNO_QUERY );

                            if( xActiveDataSource.is() )
                            {
                                const css::uno::Reference< css::uno::XInterface > xStmIf(
                                    static_cast< ::cppu::OWeakObject* >( new ImpFilterOutputStream( rOStm ) ) );

                                SvMemoryStream aMemStm( 65535, 65535 );

                                // #i119735# just use GetGDIMetaFile, it will create a buffered version of contained bitmap now automatically
                                SvmWriter aWriter( aMemStm );
                                aWriter.Write( aGraphic.GetGDIMetaFile() );

                                xActiveDataSource->setOutputStream( css::uno::Reference< css::io::XOutputStream >(
                                    xStmIf, css::uno::UNO_QUERY ) );
                                css::uno::Sequence< sal_Int8 > aMtfSeq( static_cast<sal_Int8 const *>(aMemStm.GetData()), aMemStm.Tell() );
                                xSVGWriter->write( xSaxWriter, aMtfSeq );
                            }
                        }
                    }
                    catch(const css::uno::Exception&)
                    {
                        nStatus = ERRCODE_GRFILTER_IOERROR;
                    }
                }
            }
            else
                nStatus = ERRCODE_GRFILTER_FILTERERROR;
    }
    if( nStatus != ERRCODE_NONE )
    {
        ImplSetError( nStatus, &rOStm );
    }
    return nStatus;
}


void GraphicFilter::ResetLastError()
{
    mxErrorEx = ERRCODE_NONE;
}

Link<ConvertData&,bool> GraphicFilter::GetFilterCallback() const
{
    Link<ConvertData&,bool> aLink( LINK( const_cast<GraphicFilter*>(this), GraphicFilter, FilterCallback ) );
    return aLink;
}

IMPL_LINK( GraphicFilter, FilterCallback, ConvertData&, rData, bool )
{
    bool bRet = false;

    sal_uInt16      nFormat = GRFILTER_FORMAT_DONTKNOW;
    OUString aShortName;
    css::uno::Sequence< css::beans::PropertyValue > aFilterData;
    switch( rData.mnFormat )
    {
        case ConvertDataFormat::BMP: aShortName = BMP_SHORTNAME; break;
        case ConvertDataFormat::GIF: aShortName = GIF_SHORTNAME; break;
        case ConvertDataFormat::JPG: aShortName = JPG_SHORTNAME; break;
        case ConvertDataFormat::MET: aShortName = MET_SHORTNAME; break;
        case ConvertDataFormat::PCT: aShortName = PCT_SHORTNAME; break;
        case ConvertDataFormat::PNG: aShortName = PNG_SHORTNAME; break;
        case ConvertDataFormat::SVM: aShortName = SVM_SHORTNAME; break;
        case ConvertDataFormat::TIF: aShortName = TIF_SHORTNAME; break;
        case ConvertDataFormat::WMF: aShortName = WMF_SHORTNAME; break;
        case ConvertDataFormat::EMF: aShortName = EMF_SHORTNAME; break;
        case ConvertDataFormat::SVG: aShortName = SVG_SHORTNAME; break;

        default:
        break;
    }
    if( GraphicType::NONE == rData.maGraphic.GetType() || rData.maGraphic.GetReaderContext() ) // Import
    {
        // Import
        nFormat = GetImportFormatNumberForShortName( aShortName );
        bRet = ImportGraphic( rData.maGraphic, OUString(), rData.mrStm, nFormat ) == ERRCODE_NONE;
    }
    else if( !aShortName.isEmpty() )
    {
        // Export
#if defined(IOS) || defined(ANDROID)
        if (aShortName == PNG_SHORTNAME)
        {
            aFilterData.realloc(aFilterData.getLength() + 1);
            auto pFilterData = aFilterData.getArray();
            pFilterData[aFilterData.getLength() - 1].Name = "Compression";
            // We "know" that this gets passed to zlib's deflateInit2_(). 1 means best speed.
            pFilterData[aFilterData.getLength() - 1].Value <<= static_cast<sal_Int32>(1);
        }
#endif
        nFormat = GetExportFormatNumberForShortName( aShortName );
        bRet = ExportGraphic( rData.maGraphic, OUString(), rData.mrStm, nFormat, &aFilterData ) == ERRCODE_NONE;
    }

    return bRet;
}

namespace
{
    class StandardGraphicFilter
    {
    public:
        StandardGraphicFilter()
        {
            m_aFilter.GetImportFormatCount();
        }
        GraphicFilter m_aFilter;
    };
}

GraphicFilter& GraphicFilter::GetGraphicFilter()
{
    static StandardGraphicFilter gStandardFilter;
    return gStandardFilter.m_aFilter;
}

ErrCode GraphicFilter::LoadGraphic( const OUString &rPath, const OUString &rFilterName,
                 Graphic& rGraphic, GraphicFilter* pFilter,
                 sal_uInt16* pDeterminedFormat )
{
    if ( !pFilter )
        pFilter = &GetGraphicFilter();

    const sal_uInt16 nFilter = !rFilterName.isEmpty() && pFilter->GetImportFormatCount()
                    ? pFilter->GetImportFormatNumber( rFilterName )
                    : GRFILTER_FORMAT_DONTKNOW;

    INetURLObject aURL( rPath );
    if ( aURL.HasError() )
    {
        aURL.SetSmartProtocol( INetProtocol::File );
        aURL.SetSmartURL( rPath );
    }

    std::unique_ptr<SvStream> pStream;
    if ( INetProtocol::File != aURL.GetProtocol() )
        pStream = ::utl::UcbStreamHelper::CreateStream( rPath, StreamMode::READ );

    ErrCode nRes = ERRCODE_NONE;
    if ( !pStream )
        nRes = pFilter->ImportGraphic( rGraphic, aURL, nFilter, pDeterminedFormat );
    else
        nRes = pFilter->ImportGraphic( rGraphic, rPath, *pStream, nFilter, pDeterminedFormat );

#ifdef DBG_UTIL
    OUString aReturnString;

    if (nRes == ERRCODE_GRFILTER_OPENERROR)
            aReturnString="open error";
    else if (nRes == ERRCODE_GRFILTER_IOERROR)
            aReturnString="IO error";
    else if (nRes == ERRCODE_GRFILTER_FORMATERROR)
            aReturnString="format error";
    else if (nRes == ERRCODE_GRFILTER_VERSIONERROR)
            aReturnString="version error";
    else if (nRes == ERRCODE_GRFILTER_FILTERERROR)
            aReturnString="filter error";
    else if (nRes == ERRCODE_GRFILTER_TOOBIG)
            aReturnString="graphic is too big";

    SAL_INFO_IF( nRes, "vcl.filter", "Problem importing graphic " << rPath << ". Reason: " << aReturnString );
#endif

    return nRes;
}

ErrCode GraphicFilter::compressAsPNG(const Graphic& rGraphic, SvStream& rOutputStream)
{
    css::uno::Sequence< css::beans::PropertyValue > aFilterData{ comphelper::makePropertyValue(
        "Compression", sal_uInt32(9)) };

    sal_uInt16 nFilterFormat = GetExportFormatNumberForShortName(u"PNG");
    return ExportGraphic(rGraphic, OUString(), rOutputStream, nFilterFormat, &aFilterData);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
