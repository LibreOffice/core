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

#include <o3tl/make_unique.hxx>
#include <osl/mutex.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <ucbhelper/content.hxx>
#include <cppuhelper/implbase.hxx>
#include <tools/fract.hxx>
#include <tools/resmgr.hxx>
#include <tools/stream.hxx>
#include <tools/urlobj.hxx>
#include <tools/zcodec.hxx>
#include <vcl/dibtools.hxx>
#include <vcl/salctype.hxx>
#include <vcl/pngread.hxx>
#include <vcl/pngwrite.hxx>
#include <vcl/svgdata.hxx>
#include <vcl/virdev.hxx>
#include <vcl/svapp.hxx>
#include <osl/file.hxx>
#include <vcl/graphicfilter.hxx>
#include <vcl/FilterConfigItem.hxx>
#include <vcl/wmf.hxx>
#include <vcl/settings.hxx>
#include "igif/gifread.hxx"
#include "ipdf/pdfread.hxx"
#include "jpeg/jpeg.hxx"
#include "ixbm/xbmread.hxx"
#include "ixpm/xpmread.hxx"
#include "sgffilt.hxx"
#include "osl/module.hxx"
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/uno/XWeak.hpp>
#include <com/sun/star/uno/XAggregation.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/svg/XSVGWriter.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/xml/sax/Writer.hpp>
#include <com/sun/star/ucb/CommandAbortedException.hpp>
#include <unotools/ucbstreamhelper.hxx>
#include <rtl/bootstrap.hxx>
#include <rtl/instance.hxx>
#include <vcl/metaact.hxx>
#include <vector>
#include <memory>

#include "FilterConfigCache.hxx"
#include "graphicfilter_internal.hxx"

#define PMGCHUNG_msOG       0x6d734f47      // Microsoft Office Animated GIF

using comphelper::string::getTokenCount;

typedef ::std::vector< GraphicFilter* > FilterList_impl;
static FilterList_impl* pFilterHdlList = nullptr;

static ::osl::Mutex& getListMutex()
{
    static ::osl::Mutex s_aListProtection;
    return s_aListProtection;
}

class ImpFilterOutputStream : public ::cppu::WeakImplHelper< css::io::XOutputStream >
{
protected:

    SvStream&               mrStm;

    virtual void SAL_CALL   writeBytes( const css::uno::Sequence< sal_Int8 >& rData ) override
        { mrStm.WriteBytes(rData.getConstArray(), rData.getLength()); }
    virtual void SAL_CALL   flush() override
        { mrStm.Flush(); }
    virtual void SAL_CALL   closeOutput() override {}

public:

    explicit ImpFilterOutputStream( SvStream& rStm ) : mrStm( rStm ) {}
};

static bool DirEntryExists( const INetURLObject& rObj )
{
    bool bExists = false;

    try
    {
        ::ucbhelper::Content aCnt( rObj.GetMainURL( INetURLObject::DecodeMechanism::NONE ),
                             css::uno::Reference< css::ucb::XCommandEnvironment >(),
                             comphelper::getProcessComponentContext() );

        bExists = aCnt.isDocument();
    }
    catch(const css::ucb::CommandAbortedException&)
    {
        SAL_WARN( "vcl.filter", "CommandAbortedException" );
    }
    catch(const css::ucb::ContentCreationException&)
    {
        SAL_WARN( "vcl.filter", "ContentCreationException" );
    }
    catch( ... )
    {
        SAL_WARN( "vcl.filter", "Any other exception" );
    }
    return bExists;
}

static void KillDirEntry( const OUString& rMainUrl )
{
    try
    {
        ::ucbhelper::Content aCnt( rMainUrl,
                             css::uno::Reference< css::ucb::XCommandEnvironment >(),
                             comphelper::getProcessComponentContext() );

        aCnt.executeCommand( "delete",
                             css::uno::makeAny( true ) );
    }
    catch(const css::ucb::CommandAbortedException&)
    {
        SAL_WARN( "vcl.filter", "CommandAbortedException" );
    }
    catch( ... )
    {
        SAL_WARN( "vcl.filter", "Any other exception" );
    }
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

inline OUString ImpGetExtension( const OUString &rPath )
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

/*************************************************************************
 *
 *    ImpPeekGraphicFormat()
 *
 *    Description:
 *        This function is two-fold:
 *        1.) Start reading file, determine the file format:
 *            Input parameters:
 *              rPath            - file path
 *              rFormatExtension - content matter
 *              bTest            - set false
 *            Output parameters:
 *              Return value     - true if success
 *              rFormatExtension - on success: normal file extension in capitals
 *        2.) Start reading file, verify file format
 *            Input parameters:
 *              rPath            - file path
 *              rFormatExtension - normal file extension in capitals
 *              bTest            - set true
 *            Output parameters:
 *              Return value    - false, if cannot verify the file type
 *                                  passed to the function
 *                                 true, when the format is PROBABLY verified or
 *                                 WHEN THE FORMAT IS NOT KNOWN!
 *
 *************************************************************************/

static bool ImpPeekGraphicFormat( SvStream& rStream, OUString& rFormatExtension, bool bTest )
{
    sal_uInt8   sFirstBytes[ 256 ];
    sal_uLong   nFirstLong(0), nSecondLong(0);
    sal_uLong   nStreamPos = rStream.Tell();

    rStream.Seek( STREAM_SEEK_TO_END );
    sal_uLong nStreamLen = rStream.Tell() - nStreamPos;
    rStream.Seek( nStreamPos );

    if ( !nStreamLen )
    {
        SvLockBytes* pLockBytes = rStream.GetLockBytes();
        if ( pLockBytes  )
            pLockBytes->SetSynchronMode();

        rStream.Seek( STREAM_SEEK_TO_END );
        nStreamLen = rStream.Tell() - nStreamPos;
        rStream.Seek( nStreamPos );
    }

    if (!nStreamLen)
    {
        return false; // this prevents at least a STL assertion
    }
    else if (nStreamLen >= 256)
    {
        // load first 256 bytes into a buffer
        sal_uLong nRead = rStream.ReadBytes(sFirstBytes, 256);
        if (nRead < 256)
            nStreamLen = nRead;
    }
    else
    {
        nStreamLen = rStream.ReadBytes(sFirstBytes, nStreamLen);
    }


    if (rStream.GetError())
        return false;

    for (sal_uLong i = nStreamLen; i < 256; ++i)
        sFirstBytes[i] = 0;

    // Accommodate the first 8 bytes in nFirstLong, nSecondLong
    // Big-Endian:
    for (int i = 0; i < 4; ++i)
    {
        nFirstLong=(nFirstLong<<8)|(sal_uLong)sFirstBytes[i];
        nSecondLong=(nSecondLong<<8)|(sal_uLong)sFirstBytes[i+4];
    }

    // The following variable is used when bTest == true. It remains false
    // if the format (rFormatExtension) has not yet been set.
    bool bSomethingTested = false;

    // Now the different formats are checked. The order *does* matter. e.g. a MET file
    // could also go through the BMP test, however, a BMP file can hardly go through the MET test.
    // So MET should be tested prior to BMP. However, theoretically a BMP file could conceivably
    // go through the MET test. These problems are of course not only in MET and BMP.
    // Therefore, in the case of a format check (bTest == true)  we only test *exactly* this
    // format. Everything else could have fatal consequences, for example if the user says it is
    // a BMP file (and it is a BMP) file, and the file would go through the MET test ...
    //--------------------------- MET ------------------------------------
    if( !bTest || rFormatExtension.startsWith( "MET" ) )
    {
        bSomethingTested=true;
        if( sFirstBytes[2] == 0xd3 )
        {
            rStream.SetEndian( SvStreamEndian::BIG );
            rStream.Seek( nStreamPos );
            sal_uInt16 nFieldSize;
            sal_uInt8 nMagic;
            bool bOK=true;
            rStream.ReadUInt16( nFieldSize ).ReadUChar( nMagic );
            for (int i=0; i<3; i++) {
                if (nFieldSize<6) { bOK=false; break; }
                if (nStreamLen < rStream.Tell() + nFieldSize ) { bOK=false; break; }
                rStream.SeekRel(nFieldSize-3);
                rStream.ReadUInt16( nFieldSize ).ReadUChar( nMagic );
                if (nMagic!=0xd3) { bOK=false; break; }
            }
            rStream.SetEndian( SvStreamEndian::LITTLE );
            if (bOK && !rStream.GetError()) {
                rFormatExtension = "MET";
                return true;
            }
        }
    }

    //--------------------------- BMP ------------------------------------
    if( !bTest || rFormatExtension.startsWith( "BMP" )  )
    {
        sal_uInt8 nOffs;

        bSomethingTested=true;

        // We're possibly also able to read an OS/2 bitmap array
        // ('BA'), therefore we must adjust the offset to discover the
        // first bitmap in the array
        if ( sFirstBytes[0] == 0x42 && sFirstBytes[1] == 0x41 )
            nOffs = 14;
        else
            nOffs = 0;

        // Now we initially test on 'BM'
        if ( sFirstBytes[0+nOffs]==0x42 && sFirstBytes[1+nOffs]==0x4d )
        {

            // OS/2 can set the Reserved flags to a value other than 0
            // (which they really should not do...);
            // In this case we test the size of the BmpInfoHeaders
            if ( ( sFirstBytes[6+nOffs]==0x00 &&
                   sFirstBytes[7+nOffs]==0x00 &&
                   sFirstBytes[8+nOffs]==0x00 &&
                   sFirstBytes[9+nOffs]==0x00 ) ||
                   sFirstBytes[14+nOffs] == 0x28 ||
                   sFirstBytes[14+nOffs] == 0x0c )
            {
                rFormatExtension = "BMP";
                return true;
            }
        }
    }

    //--------------------------- WMF/EMF ------------------------------------

    if( !bTest ||
        rFormatExtension.startsWith( "WMF" ) ||
        rFormatExtension.startsWith( "EMF" ) )
    {
        bSomethingTested = true;

        if ( nFirstLong==0xd7cdc69a || nFirstLong==0x01000900 )
        {
            rFormatExtension = "WMF";
            return true;
        }
        else if( nFirstLong == 0x01000000 && sFirstBytes[ 40 ] == 0x20 && sFirstBytes[ 41 ] == 0x45 &&
            sFirstBytes[ 42 ] == 0x4d && sFirstBytes[ 43 ] == 0x46 )
        {
            rFormatExtension = "EMF";
            return true;
        }
    }

    //--------------------------- PCX ------------------------------------
    if( !bTest || rFormatExtension.startsWith( "PCX" ) )
    {
        bSomethingTested=true;
        if (sFirstBytes[0]==0x0a)
        {
            sal_uInt8 nVersion=sFirstBytes[1];
            sal_uInt8 nEncoding=sFirstBytes[2];
            if( ( nVersion==0 || nVersion==2 || nVersion==3 || nVersion==5 ) && nEncoding<=1 )
            {
                rFormatExtension = "PCX";
                return true;
            }
        }
    }

    //--------------------------- TIF ------------------------------------
    if( !bTest || rFormatExtension.startsWith( "TIF" ) )
    {
        bSomethingTested=true;
        if ( nFirstLong==0x49492a00 || nFirstLong==0x4d4d002a )
        {
            rFormatExtension = "TIF";
            return true;
        }
    }

    //--------------------------- GIF ------------------------------------
    if( !bTest || rFormatExtension.startsWith( "GIF" ) )
    {
        bSomethingTested=true;
        if ( nFirstLong==0x47494638 && (sFirstBytes[4]==0x37 || sFirstBytes[4]==0x39) && sFirstBytes[5]==0x61 )
        {
            rFormatExtension = "GIF";
            return true;
        }
    }

    //--------------------------- PNG ------------------------------------
    if( !bTest || rFormatExtension.startsWith( "PNG" ) )
    {
        bSomethingTested=true;
        if (nFirstLong==0x89504e47 && nSecondLong==0x0d0a1a0a)
        {
            rFormatExtension = "PNG";
            return true;
        }
    }

    //--------------------------- JPG ------------------------------------
    if( !bTest || rFormatExtension.startsWith( "JPG" ) )
    {
        bSomethingTested=true;
        if ( ( nFirstLong==0xffd8ffe0 && sFirstBytes[6]==0x4a && sFirstBytes[7]==0x46 && sFirstBytes[8]==0x49 && sFirstBytes[9]==0x46 ) ||
             ( nFirstLong==0xffd8fffe ) || ( 0xffd8ff00 == ( nFirstLong & 0xffffff00 ) ) )
        {
            rFormatExtension = "JPG";
            return true;
        }
    }

    //--------------------------- SVM ------------------------------------
    if( !bTest || rFormatExtension.startsWith( "SVM" ) )
    {
        bSomethingTested=true;
        if( nFirstLong==0x53564744 && sFirstBytes[4]==0x49 )
        {
            rFormatExtension = "SVM";
            return true;
        }
        else if( sFirstBytes[0]==0x56 && sFirstBytes[1]==0x43 && sFirstBytes[2]==0x4C &&
                 sFirstBytes[3]==0x4D && sFirstBytes[4]==0x54 && sFirstBytes[5]==0x46 )
        {
            rFormatExtension = "SVM";
            return true;
        }
    }

    //--------------------------- PCD ------------------------------------
    if( !bTest || rFormatExtension.startsWith( "PCD" ) )
    {
        bSomethingTested = true;
        if( nStreamLen >= 2055 )
        {
            char sBuf[8];
            rStream.Seek( nStreamPos + 2048 );
            rStream.ReadBytes( sBuf, 7 );

            if( strncmp( sBuf, "PCD_IPI", 7 ) == 0 )
            {
                rFormatExtension = "PCD";
                return true;
            }
        }
    }

    //--------------------------- PSD ------------------------------------
    if( !bTest || rFormatExtension.startsWith( "PSD" ) )
    {
        bSomethingTested = true;
        if ( ( nFirstLong == 0x38425053 ) && ( (nSecondLong >> 16 ) == 1 ) )
        {
            rFormatExtension = "PSD";
            return true;
        }
    }

    //--------------------------- EPS ------------------------------------
    if( !bTest || rFormatExtension.startsWith( "EPS" ) )
    {
        bSomethingTested = true;
        if ( ( nFirstLong == 0xC5D0D3C6 ) || ( ImplSearchEntry( sFirstBytes, reinterpret_cast<sal_uInt8 const *>("%!PS-Adobe"), 10, 10 ) &&
             ImplSearchEntry( &sFirstBytes[15], reinterpret_cast<sal_uInt8 const *>("EPS"), 3, 3 ) ) )
        {
            rFormatExtension = "EPS";
            return true;
        }
    }

    //--------------------------- DXF ------------------------------------
    if( !bTest || rFormatExtension.startsWith( "DXF" ) )
    {
        // Binary DXF File Format
        if( strncmp( reinterpret_cast<char*>(sFirstBytes), "AutoCAD Binary DXF", 18 ) == 0 )
        {
            rFormatExtension = "DXF";
            return true;
        }

        // ASCII DXF File Format
        int i=0;
        while (i<256 && sFirstBytes[i]<=32)
            ++i;

        if (i<256 && sFirstBytes[i]=='0')
        {
            ++i;

            // only now do we have sufficient data to make a judgement
            // based on a '0' + 'SECTION' == DXF argument
            bSomethingTested=true;

            while( i<256 && sFirstBytes[i]<=32 )
                ++i;

            if (i+7<256 && (strncmp(reinterpret_cast<char*>(sFirstBytes+i),"SECTION",7)==0))
            {
                rFormatExtension = "DXF";
                return true;
            }
        }

    }

    //--------------------------- PCT ------------------------------------
    if( !bTest || rFormatExtension.startsWith( "PCT" ) )
    {
        bSomethingTested = true;
        if (isPCT(rStream, nStreamPos, nStreamLen))
        {
            rFormatExtension = "PCT";
            return true;
        }
    }

    //------------------------- PBM + PGM + PPM ---------------------------
    if( !bTest ||
        rFormatExtension.startsWith( "PBM" ) ||
        rFormatExtension.startsWith( "PGM" ) ||
        rFormatExtension.startsWith( "PPM" ) )
    {
        bSomethingTested=true;
        if ( sFirstBytes[ 0 ] == 'P' )
        {
            switch( sFirstBytes[ 1 ] )
            {
                case '1' :
                case '4' :
                    rFormatExtension = "PBM";
                return true;

                case '2' :
                case '5' :
                    rFormatExtension = "PGM";
                return true;

                case '3' :
                case '6' :
                    rFormatExtension = "PPM";
                return true;
            }
        }
    }

    //--------------------------- RAS( SUN RasterFile )------------------
    if( !bTest || rFormatExtension.startsWith( "RAS" ) )
    {
        bSomethingTested=true;
        if( nFirstLong == 0x59a66a95 )
        {
            rFormatExtension = "RAS";
            return true;
        }
    }

    //--------------------------- XPM ------------------------------------
    if( !bTest )
    {
        bSomethingTested = true;
        if( ImplSearchEntry( sFirstBytes, reinterpret_cast<sal_uInt8 const *>("/* XPM */"), 256, 9 ) )
        {
            rFormatExtension = "XPM";
            return true;
        }
    }
    else if( rFormatExtension.startsWith( "XPM" ) )
    {
        return true;
    }

    //--------------------------- XBM ------------------------------------
    if( !bTest )
    {
        sal_uLong nSize = ( nStreamLen > 2048 ) ? 2048 : nStreamLen;
        std::unique_ptr<sal_uInt8[]> pBuf(new sal_uInt8 [ nSize ]);

        rStream.Seek( nStreamPos );
        rStream.ReadBytes( pBuf.get(), nSize );
        sal_uInt8* pPtr = ImplSearchEntry( pBuf.get(), reinterpret_cast<sal_uInt8 const *>("#define"), nSize, 7 );

        if( pPtr )
        {
            if( ImplSearchEntry( pPtr, reinterpret_cast<sal_uInt8 const *>("_width"), pBuf.get() + nSize - pPtr, 6 ) )
            {
                rFormatExtension = "XBM";
                return true;
            }
        }
    }
    else if( rFormatExtension.startsWith( "XBM" ) )
    {
        return true;
    }

    //--------------------------- SVG ------------------------------------
    if( !bTest )
    {
        sal_uInt8* pCheckArray = sFirstBytes;
        sal_uLong nCheckSize = nStreamLen < 256 ? nStreamLen : 256;

        sal_uInt8 sExtendedOrDecompressedFirstBytes[2048];
        sal_uLong nDecompressedSize = nCheckSize;

        bool bIsGZip(false);

        // check if it is gzipped -> svgz
        if(sFirstBytes[0] == 0x1F && sFirstBytes[1] == 0x8B)
        {
            ZCodec aCodec;
            rStream.Seek(nStreamPos);
            aCodec.BeginCompression(ZCODEC_DEFAULT_COMPRESSION, false, true);
            nDecompressedSize = aCodec.Read(rStream, sExtendedOrDecompressedFirstBytes, 2048);
            nCheckSize = nDecompressedSize < 256 ? nDecompressedSize : 256;
            aCodec.EndCompression();
            pCheckArray = sExtendedOrDecompressedFirstBytes;

            bIsGZip = true;
        }

        bool bIsSvg(false);

        // check for Xml
        // #119176# SVG files which have no xml header at all have shown up this is optional
        if( ImplSearchEntry(pCheckArray, reinterpret_cast<sal_uInt8 const *>("<?xml"), nCheckSize, 5 ) // is it xml
            && ImplSearchEntry(pCheckArray, reinterpret_cast<sal_uInt8 const *>("version"), nCheckSize, 7 )) // does it have a version (required for xml)
        {

            // check for DOCTYPE svg combination
            if( ImplSearchEntry(pCheckArray, reinterpret_cast<sal_uInt8 const *>("DOCTYPE"), nCheckSize, 7 ) // 'DOCTYPE' is there
                && ImplSearchEntry(pCheckArray, reinterpret_cast<sal_uInt8 const *>("svg"), nCheckSize, 3 )) // 'svg' is there
            {
                bIsSvg = true;
            }
        }

        // check for svg element in 1st 256 bytes
        if(!bIsSvg && ImplSearchEntry(pCheckArray, reinterpret_cast<sal_uInt8 const *>("<svg"), nCheckSize, 4 )) // '<svg'
        {
            bIsSvg = true;
        }

        // extended search for svg element
        if(!bIsSvg)
        {
            // it's a xml, look for '<svg' in full file. Should not happen too
            // often since the tests above will handle most cases, but can happen
            // with Svg files containing big comment headers or Svg as the host
            // language

            pCheckArray = sExtendedOrDecompressedFirstBytes;

            if (bIsGZip)
            {
                nCheckSize = nDecompressedSize < 2048 ? nDecompressedSize : 2048;
            }
            else
            {
                nCheckSize = nStreamLen < 2048 ? nStreamLen : 2048;
                rStream.Seek(nStreamPos);
                nCheckSize = rStream.ReadBytes(sExtendedOrDecompressedFirstBytes, nCheckSize);
            }

            if(ImplSearchEntry(pCheckArray, reinterpret_cast<sal_uInt8 const *>("<svg"), nCheckSize, 4)) // '<svg'
            {
                bIsSvg = true;
            }
        }

        if(bIsSvg)
        {
            rFormatExtension = "SVG";
            return true;
        }
    }
    else if( rFormatExtension.startsWith( "SVG" ) )
    {
        return true;
    }

    //--------------------------- TGA ------------------------------------
    if( !bTest || rFormatExtension.startsWith( "TGA" ) )
    {
        bSomethingTested = true;

        // just a simple test for the extension
        if( rFormatExtension.startsWith( "TGA" ) )
            return true;
    }

    //--------------------------- SGV ------------------------------------
    if( !bTest || rFormatExtension.startsWith( "SGV" ) )
    {
        bSomethingTested = true;

        // just a simple test for the extension
        if( rFormatExtension.startsWith( "SGV" ) )
            return true;
    }

    //--------------------------- SGF ------------------------------------
    if( !bTest || rFormatExtension.startsWith( "SGF" ) )
    {
        bSomethingTested=true;
        if( sFirstBytes[ 0 ] == 'J' && sFirstBytes[ 1 ] == 'J' )
        {
            rFormatExtension = "SGF";
            return true;
        }
    }

    if(!bTest || rFormatExtension.startsWith( "MOV" ))
    {
        if ((sFirstBytes[ 4 ] == 'f' && sFirstBytes[ 5 ] == 't' && sFirstBytes[ 6 ] == 'y' &&
             sFirstBytes[ 7 ] == 'p' && sFirstBytes[ 8 ] == 'q' && sFirstBytes[ 9 ] == 't') ||
            (sFirstBytes[ 4 ] == 'm' && sFirstBytes[ 5 ] == 'o' && sFirstBytes[ 6 ] == 'o' &&
             sFirstBytes[ 7 ] == 'v' && sFirstBytes[ 11 ] == 'l' && sFirstBytes[ 12 ] == 'm'))
        {
            rFormatExtension = "MOV";
            return true;
        }
    }

    if (!bTest || rFormatExtension.startsWith("PDF"))
    {
        if ((sFirstBytes[0] == '%' && sFirstBytes[1] == 'P' && sFirstBytes[2] == 'D' &&
             sFirstBytes[3] == 'F' && sFirstBytes[4] == '-'))
        {
            rFormatExtension = "PDF";
            return true;
        }
    }

    return bTest && !bSomethingTested;
}

sal_uInt16 GraphicFilter::ImpTestOrFindFormat( const OUString& rPath, SvStream& rStream, sal_uInt16& rFormat )
{
    // determine or check the filter/format by reading into it
    if( rFormat == GRFILTER_FORMAT_DONTKNOW )
    {
        OUString aFormatExt;
        if( ImpPeekGraphicFormat( rStream, aFormatExt, false ) )
        {
            rFormat = pConfig->GetImportFormatNumberForExtension( aFormatExt );
            if( rFormat != GRFILTER_FORMAT_DONTKNOW )
                return GRFILTER_OK;
        }
        // determine filter by file extension
        if( !rPath.isEmpty() )
        {
            OUString aExt( ImpGetExtension( rPath ) );
            rFormat = pConfig->GetImportFormatNumberForExtension( aExt );
            if( rFormat != GRFILTER_FORMAT_DONTKNOW )
                return GRFILTER_OK;
        }
        return GRFILTER_FORMATERROR;
    }
    else
    {
        OUString aTmpStr( pConfig->GetImportFormatExtension( rFormat ) );
        aTmpStr = aTmpStr.toAsciiUpperCase();
        if( !ImpPeekGraphicFormat( rStream, aTmpStr, true ) )
            return GRFILTER_FORMATERROR;
        if ( pConfig->GetImportFormatExtension( rFormat ).equalsIgnoreAsciiCase( "pcd" ) )
        {
            sal_Int32 nBase = 2;    // default Base0
            if ( pConfig->GetImportFilterType( rFormat ).equalsIgnoreAsciiCase( "pcd_Photo_CD_Base4" ) )
                nBase = 1;
            else if ( pConfig->GetImportFilterType( rFormat ).equalsIgnoreAsciiCase( "pcd_Photo_CD_Base16" ) )
                nBase = 0;
            OUString aFilterConfigPath( "Office.Common/Filter/Graphic/Import/PCD" );
            FilterConfigItem aFilterConfigItem( aFilterConfigPath );
            aFilterConfigItem.WriteInt32( "Resolution", nBase );
        }
    }

    return GRFILTER_OK;
}

static Graphic ImpGetScaledGraphic( const Graphic& rGraphic, FilterConfigItem& rConfigItem )
{
    Graphic     aGraphic;

    std::unique_ptr<ResMgr> xResMgr(ResMgr::CreateResMgr( "svt", Application::GetSettings().GetUILanguageTag() ));

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
        if ( aPrefMapMode == MapUnit::MapPixel )
            aOriginalSize = Application::GetDefaultDevice()->PixelToLogic( aPrefSize, MapUnit::Map100thMM );
        else
            aOriginalSize = OutputDevice::LogicToLogic( aPrefSize, aPrefMapMode, MapUnit::Map100thMM );
        if ( !nLogicalWidth )
            nLogicalWidth = aOriginalSize.Width();
        if ( !nLogicalHeight )
            nLogicalHeight = aOriginalSize.Height();
        if( rGraphic.GetType() == GraphicType::Bitmap )
        {

            // Resolution is set
            if( nMode == 1 )
            {
                Bitmap      aBitmap( rGraphic.GetBitmap() );
                MapMode     aMap( MapUnit::Map100thInch );

                sal_Int32   nDPI = rConfigItem.ReadInt32( "Resolution", 75 );
                Fraction    aFrac( 1, std::min( std::max( nDPI, sal_Int32( 75 ) ), sal_Int32( 600 ) ) );

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
                aBmpEx.Convert( (BmpConversion)nColors );   // the entries in the xml section have the same meaning as
                aGraphic = aBmpEx;                          // they have in the BmpConversion enum, so it should be
            }                                               // allowed to cast them
        }
        else
        {
            if( ( nMode == 1 ) || ( nMode == 2 ) )
            {
                GDIMetaFile aMtf( rGraphic.GetGDIMetaFile() );
                Size aNewSize( OutputDevice::LogicToLogic( Size( nLogicalWidth, nLogicalHeight ), MapUnit::Map100thMM, aMtf.GetPrefMapMode() ) );

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

static OUString ImpCreateFullFilterPath( const OUString& rPath, const OUString& rFilterName )
{
    OUString aPathURL;

    ::osl::FileBase::getFileURLFromSystemPath( rPath, aPathURL );
    aPathURL += "/";

    OUString aSystemPath;
    ::osl::FileBase::getSystemPathFromFileURL( aPathURL, aSystemPath );
    aSystemPath += rFilterName;

    return OUString( aSystemPath );
}

class ImpFilterLibCache;

struct ImpFilterLibCacheEntry
{
    ImpFilterLibCacheEntry* mpNext;
#ifndef DISABLE_DYNLOADING
    osl::Module             maLibrary;
#endif
    OUString                maFiltername;
    OUString                maFormatName;
    PFilterCall             mpfnImport;

    ImpFilterLibCacheEntry(const OUString& rPathname, const OUString& rFiltername, const OUString& rFormatName);
    bool                    operator==( const OUString& rFiltername ) const { return maFiltername == rFiltername; }

    PFilterCall             GetImportFunction();
};

ImpFilterLibCacheEntry::ImpFilterLibCacheEntry( const OUString& rPathname, const OUString& rFiltername, const OUString& rFormatName ) :
        mpNext          ( nullptr ),
#ifndef DISABLE_DYNLOADING
        maLibrary       ( rPathname ),
#endif
        maFiltername    ( rFiltername ),
        maFormatName    ( rFormatName ),
        mpfnImport      ( nullptr )
{
#ifdef DISABLE_DYNLOADING
    (void) rPathname;
#endif
}

#ifdef DISABLE_DYNLOADING

extern "C" bool icdGraphicImport( SvStream& rStream, Graphic& rGraphic, FilterConfigItem* pConfigItem );
extern "C" bool idxGraphicImport( SvStream& rStream, Graphic& rGraphic, FilterConfigItem* pConfigItem );
extern "C" bool imeGraphicImport( SvStream& rStream, Graphic& rGraphic, FilterConfigItem* pConfigItem );
extern "C" bool ipbGraphicImport( SvStream& rStream, Graphic& rGraphic, FilterConfigItem* pConfigItem );
extern "C" bool ipdGraphicImport( SvStream& rStream, Graphic& rGraphic, FilterConfigItem* pConfigItem );
extern "C" bool ipsGraphicImport( SvStream& rStream, Graphic& rGraphic, FilterConfigItem* pConfigItem );
extern "C" bool iptGraphicImport( SvStream& rStream, Graphic& rGraphic, FilterConfigItem* pConfigItem );
extern "C" bool ipxGraphicImport( SvStream& rStream, Graphic& rGraphic, FilterConfigItem* pConfigItem );
extern "C" bool iraGraphicImport( SvStream& rStream, Graphic& rGraphic, FilterConfigItem* pConfigItem );
extern "C" bool itgGraphicImport( SvStream& rStream, Graphic& rGraphic, FilterConfigItem* pConfigItem );
extern "C" bool itiGraphicImport( SvStream& rStream, Graphic& rGraphic, FilterConfigItem* pConfigItem );

#endif

PFilterCall ImpFilterLibCacheEntry::GetImportFunction()
{
    if( !mpfnImport )
    {
#ifndef DISABLE_DYNLOADING
        if (maFormatName == "icd")
            mpfnImport = reinterpret_cast<PFilterCall>(maLibrary.getFunctionSymbol("icdGraphicImport"));
        else if (maFormatName == "idx")
            mpfnImport = reinterpret_cast<PFilterCall>(maLibrary.getFunctionSymbol("idxGraphicImport"));
        else if (maFormatName == "ime")
            mpfnImport = reinterpret_cast<PFilterCall>(maLibrary.getFunctionSymbol("imeGraphicImport"));
        else if (maFormatName == "ipb")
            mpfnImport = reinterpret_cast<PFilterCall>(maLibrary.getFunctionSymbol("ipbGraphicImport"));
        else if (maFormatName == "ipd")
            mpfnImport = reinterpret_cast<PFilterCall>(maLibrary.getFunctionSymbol("ipdGraphicImport"));
        else if (maFormatName == "ips")
            mpfnImport = reinterpret_cast<PFilterCall>(maLibrary.getFunctionSymbol("ipsGraphicImport"));
        else if (maFormatName == "ipt")
            mpfnImport = reinterpret_cast<PFilterCall>(maLibrary.getFunctionSymbol("iptGraphicImport"));
        else if (maFormatName == "ipx")
            mpfnImport = reinterpret_cast<PFilterCall>(maLibrary.getFunctionSymbol("ipxGraphicImport"));
        else if (maFormatName == "ira")
            mpfnImport = reinterpret_cast<PFilterCall>(maLibrary.getFunctionSymbol("iraGraphicImport"));
        else if (maFormatName == "itg")
            mpfnImport = reinterpret_cast<PFilterCall>(maLibrary.getFunctionSymbol("itgGraphicImport"));
        else if (maFormatName == "iti")
            mpfnImport = reinterpret_cast<PFilterCall>(maLibrary.getFunctionSymbol("itiGraphicImport"));
 #else
        if (maFiltername ==  "icd")
            mpfnImport = icdGraphicImport;
        else if (maFiltername ==  "idx")
            mpfnImport = idxGraphicImport;
        else if (maFiltername ==  "ime")
            mpfnImport = imeGraphicImport;
        else if (maFiltername ==  "ipb")
            mpfnImport = ipbGraphicImport;
        else if (maFiltername ==  "ipd")
            mpfnImport = ipdGraphicImport;
        else if (maFiltername ==  "ips")
            mpfnImport = ipsGraphicImport;
        else if (maFiltername ==  "ipt")
            mpfnImport = iptGraphicImport;
        else if (maFiltername ==  "ipx")
            mpfnImport = ipxGraphicImport;
        else if (maFiltername ==  "ira")
            mpfnImport = iraGraphicImport;
        else if (maFiltername ==  "itg")
            mpfnImport = itgGraphicImport;
        else if (maFiltername ==  "iti")
            mpfnImport = itiGraphicImport;
 #endif
    }

    return mpfnImport;
}

class ImpFilterLibCache
{
    ImpFilterLibCacheEntry* mpFirst;
    ImpFilterLibCacheEntry* mpLast;

public:
                            ImpFilterLibCache();
                            ~ImpFilterLibCache();

    ImpFilterLibCacheEntry* GetFilter( const OUString& rFilterPath, const OUString& rFiltername, const OUString& rFormatName );
};

ImpFilterLibCache::ImpFilterLibCache() :
    mpFirst     ( nullptr ),
    mpLast      ( nullptr )
{
}

ImpFilterLibCache::~ImpFilterLibCache()
{
    ImpFilterLibCacheEntry* pEntry = mpFirst;
    while( pEntry )
    {
        ImpFilterLibCacheEntry* pNext = pEntry->mpNext;
        delete pEntry;
        pEntry = pNext;
    }
}

ImpFilterLibCacheEntry* ImpFilterLibCache::GetFilter(const OUString& rFilterPath, const OUString& rFilterName, const OUString& rFormatName)
{
    ImpFilterLibCacheEntry* pEntry = mpFirst;

    while( pEntry )
    {
        if( *pEntry == rFilterName && pEntry->maFormatName == rFormatName )
            break;
        else
            pEntry = pEntry->mpNext;
    }
    if( !pEntry )
    {
        OUString aPhysicalName( ImpCreateFullFilterPath( rFilterPath, rFilterName ) );
        pEntry = new ImpFilterLibCacheEntry(aPhysicalName, rFilterName, rFormatName );
#ifndef DISABLE_DYNLOADING
        if ( pEntry->maLibrary.is() )
#endif
        {
            if( !mpFirst )
                mpFirst = mpLast = pEntry;
            else
                mpLast = mpLast->mpNext = pEntry;
        }
#ifndef DISABLE_DYNLOADING
        else
        {
            delete pEntry;
            pEntry = nullptr;
        }
#endif
    }
    return pEntry;
};

namespace { struct Cache : public rtl::Static<ImpFilterLibCache, Cache> {}; }

GraphicFilter::GraphicFilter( bool bConfig )
    : pErrorEx(nullptr)
    , bUseConfig(bConfig)
    , nExpGraphHint(0)
{
    ImplInit();
}

GraphicFilter::~GraphicFilter()
{
    {
        ::osl::MutexGuard aGuard( getListMutex() );
        for(
            FilterList_impl::iterator it = pFilterHdlList->begin();
            it != pFilterHdlList->end();
            ++it
        ) {
            if( *it == this )
            {
                pFilterHdlList->erase( it );
                break;
            }
        }
        if( pFilterHdlList->empty() )
        {
            delete pFilterHdlList;
            pFilterHdlList = nullptr;
            delete pConfig;
        }
    }

    delete pErrorEx;
}

void GraphicFilter::ImplInit()
{
    {
        ::osl::MutexGuard aGuard( getListMutex() );

        if ( !pFilterHdlList )
        {
            pFilterHdlList = new FilterList_impl;
            pConfig = new FilterConfigCache( bUseConfig );
        }
        else
            pConfig = pFilterHdlList->front()->pConfig;

        pFilterHdlList->push_back( this );
    }

    if( bUseConfig )
    {
        OUString url("$BRAND_BASE_DIR/" LIBO_LIB_FOLDER);
        rtl::Bootstrap::expandMacros(url); //TODO: detect failure
        osl::FileBase::getSystemPathFromFileURL(url, aFilterPath);
    }

    pErrorEx = new FilterErrorEx;
}

sal_uLong GraphicFilter::ImplSetError( sal_uLong nError, const SvStream* pStm )
{
    pErrorEx->nFilterError = nError;
    pErrorEx->nStreamError = pStm ? pStm->GetError() : ERRCODE_NONE;
    return nError;
}

sal_uInt16 GraphicFilter::GetImportFormatCount()
{
    return pConfig->GetImportFormatCount();
}

sal_uInt16 GraphicFilter::GetImportFormatNumber( const OUString& rFormatName )
{
    return pConfig->GetImportFormatNumber( rFormatName );
}

sal_uInt16 GraphicFilter::GetImportFormatNumberForShortName( const OUString& rShortName )
{
    return pConfig->GetImportFormatNumberForShortName( rShortName );
}

sal_uInt16 GraphicFilter::GetImportFormatNumberForTypeName( const OUString& rType )
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

sal_uInt16 GraphicFilter::GetExportFormatCount()
{
    return pConfig->GetExportFormatCount();
}

sal_uInt16 GraphicFilter::GetExportFormatNumber( const OUString& rFormatName )
{
    return pConfig->GetExportFormatNumber( rFormatName );
}

sal_uInt16 GraphicFilter::GetExportFormatNumberForMediaType( const OUString& rMediaType )
{
    return pConfig->GetExportFormatNumberForMediaType( rMediaType );
}

sal_uInt16 GraphicFilter::GetExportFormatNumberForShortName( const OUString& rShortName )
{
    return pConfig->GetExportFormatNumberForShortName( rShortName );
}

OUString GraphicFilter::GetExportInternalFilterName( sal_uInt16 nFormat )
{
    return pConfig->GetExportInternalFilterName( nFormat );
}

sal_uInt16 GraphicFilter::GetExportFormatNumberForTypeName( const OUString& rType )
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

sal_uInt16 GraphicFilter::CanImportGraphic( const INetURLObject& rPath,
                                        sal_uInt16 nFormat, sal_uInt16* pDeterminedFormat )
{
    sal_uInt16  nRetValue = GRFILTER_FORMATERROR;
    SAL_WARN_IF( rPath.GetProtocol() == INetProtocol::NotValid, "vcl.filter", "GraphicFilter::CanImportGraphic() : ProtType == INetProtocol::NotValid" );

    OUString    aMainUrl( rPath.GetMainURL( INetURLObject::DecodeMechanism::NONE ) );
    std::unique_ptr<SvStream> xStream(::utl::UcbStreamHelper::CreateStream( aMainUrl, StreamMode::READ | StreamMode::SHARE_DENYNONE ));
    if (xStream)
    {
        nRetValue = CanImportGraphic( aMainUrl, *xStream, nFormat, pDeterminedFormat );
    }
    return nRetValue;
}

sal_uInt16 GraphicFilter::CanImportGraphic( const OUString& rMainUrl, SvStream& rIStream,
                                        sal_uInt16 nFormat, sal_uInt16* pDeterminedFormat )
{
    sal_uLong nStreamPos = rIStream.Tell();
    sal_uInt16 nRes = ImpTestOrFindFormat( rMainUrl, rIStream, nFormat );

    rIStream.Seek(nStreamPos);

    if( nRes==GRFILTER_OK && pDeterminedFormat!=nullptr )
        *pDeterminedFormat = nFormat;

    return (sal_uInt16) ImplSetError( nRes, &rIStream );
}

//SJ: TODO, we need to create a GraphicImporter component
sal_uInt16 GraphicFilter::ImportGraphic( Graphic& rGraphic, const INetURLObject& rPath,
                                     sal_uInt16 nFormat, sal_uInt16 * pDeterminedFormat, GraphicFilterImportFlags nImportFlags )
{
    SAL_WARN_IF( rPath.GetProtocol() == INetProtocol::NotValid, "vcl.filter", "GraphicFilter::ImportGraphic() : ProtType == INetProtocol::NotValid" );

    OUString    aMainUrl( rPath.GetMainURL( INetURLObject::DecodeMechanism::NONE ) );
    if (rPath.IsExoticProtocol())
    {
        SAL_WARN("vcl.filter", "GraphicFilter::ImportGraphic(), ignore exotic protocol: " << aMainUrl);
        return GRFILTER_FORMATERROR;
    }

    ErrCode nRetValue = GRFILTER_FORMATERROR;
    std::unique_ptr<SvStream> xStream(::utl::UcbStreamHelper::CreateStream( aMainUrl, StreamMode::READ | StreamMode::SHARE_DENYNONE ));
    if (xStream)
    {
        nRetValue = ImportGraphic( rGraphic, aMainUrl, *xStream, nFormat, pDeterminedFormat, nImportFlags );
    }
    return nRetValue;
}

sal_uInt16 GraphicFilter::ImportGraphic( Graphic& rGraphic, const OUString& rPath, SvStream& rIStream,
                                     sal_uInt16 nFormat, sal_uInt16* pDeterminedFormat, GraphicFilterImportFlags nImportFlags, WMF_EXTERNALHEADER *pExtHeader )
{
    return ImportGraphic( rGraphic, rPath, rIStream, nFormat, pDeterminedFormat, nImportFlags, nullptr, pExtHeader );
}

sal_uInt16 GraphicFilter::ImportGraphic( Graphic& rGraphic, const OUString& rPath, SvStream& rIStream,
                                     sal_uInt16 nFormat, sal_uInt16* pDeterminedFormat, GraphicFilterImportFlags nImportFlags,
                                     css::uno::Sequence< css::beans::PropertyValue >* pFilterData,
                                     WMF_EXTERNALHEADER *pExtHeader )
{
    OUString                       aFilterName;
    OUString                       aExternalFilterName;
    sal_uLong                      nStreamBegin;
    sal_uInt16                     nStatus;
    std::shared_ptr<GraphicReader> pContext = rGraphic.GetContext();
    GfxLinkType                    eLinkType = GfxLinkType::NONE;
    bool                           bDummyContext = rGraphic.IsDummyContext();
    const bool                     bLinkSet = rGraphic.IsLink();
    std::unique_ptr<FilterConfigItem> pFilterConfigItem;

    Size                aPreviewSizeHint( 0, 0 );
    bool                bAllowPartialStreamRead = false;
    bool                bCreateNativeLink = true;

    std::unique_ptr<sal_uInt8[]> pGraphicContent;
    sal_Int32  nGraphicContentSize = 0;

    ResetLastError();

    if ( pFilterData )
    {
        sal_Int32 i;
        for ( i = 0; i < pFilterData->getLength(); i++ )
        {
            if ( (*pFilterData)[ i ].Name == "PreviewSizeHint" )
            {
                css::awt::Size aSize;
                if ( (*pFilterData)[ i ].Value >>= aSize )
                {
                    aPreviewSizeHint = Size( aSize.Width, aSize.Height );
                    if ( aSize.Width || aSize.Height )
                        nImportFlags |= GraphicFilterImportFlags::ForPreview;
                    else
                        nImportFlags &=~GraphicFilterImportFlags::ForPreview;
                }
            }
            else if ( (*pFilterData)[ i ].Name == "AllowPartialStreamRead" )
            {
                (*pFilterData)[ i ].Value >>= bAllowPartialStreamRead;
                if ( bAllowPartialStreamRead )
                    nImportFlags |= GraphicFilterImportFlags::AllowPartialStreamRead;
                else
                    nImportFlags &=~GraphicFilterImportFlags::AllowPartialStreamRead;
            }
            else if ( (*pFilterData)[ i ].Name == "CreateNativeLink" )
            {
                (*pFilterData)[ i ].Value >>= bCreateNativeLink;
            }
        }
    }

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
        // if pending, return GRFILTER_OK in order to request more bytes
        if( rIStream.GetError() == ERRCODE_IO_PENDING )
        {
            rGraphic.SetDummyContext(true);
            rIStream.ResetError();
            rIStream.Seek( nStreamBegin );
            return (sal_uInt16) ImplSetError( GRFILTER_OK );
        }

        rIStream.Seek( nStreamBegin );

        if( ( nStatus != GRFILTER_OK ) || rIStream.GetError() )
            return (sal_uInt16) ImplSetError( ( nStatus != GRFILTER_OK ) ? nStatus : GRFILTER_OPENERROR, &rIStream );

        if( pDeterminedFormat )
            *pDeterminedFormat = nFormat;

        aFilterName = pConfig->GetImportFilterName( nFormat );
        aExternalFilterName = pConfig->GetExternalFilterName(nFormat, false);
    }
    else
    {
        if( pContext && !bDummyContext )
            aFilterName = pContext->GetUpperFilterName();

        nStreamBegin = 0;
        nStatus = GRFILTER_OK;
    }

    // read graphic
    if ( pConfig->IsImportInternalFilter( nFormat ) )
    {
        if( aFilterName.equalsIgnoreAsciiCase( IMP_GIF )  )
        {
            if( rGraphic.IsDummyContext())
                rGraphic.SetDummyContext( false );

            if( !ImportGIF( rIStream, rGraphic ) )
                nStatus = GRFILTER_FILTERERROR;
            else
                eLinkType = GfxLinkType::NativeGif;
        }
        else if( aFilterName.equalsIgnoreAsciiCase( IMP_PNG ) )
        {
            if( rGraphic.IsDummyContext())
                rGraphic.SetDummyContext( false );

            vcl::PNGReader aPNGReader( rIStream );

            // ignore animation for previews and set preview size
            if( aPreviewSizeHint.Width() || aPreviewSizeHint.Height() )
            {
                // position the stream at the end of the image if requested
                if( !bAllowPartialStreamRead )
                    aPNGReader.GetChunks();
            }
            else
            {
                // check if this PNG contains a GIF chunk!
                const std::vector<vcl::PNGReader::ChunkData>& rChunkData = aPNGReader.GetChunks();
                std::vector<vcl::PNGReader::ChunkData>::const_iterator aIter(rChunkData.begin());
                std::vector<vcl::PNGReader::ChunkData>::const_iterator aEnd(rChunkData.end());

                while (aIter != aEnd)
                {
                    // Microsoft Office is storing Animated GIFs in following chunk
                    if (aIter->nType == PMGCHUNG_msOG)
                    {
                        sal_uInt32 nChunkSize = aIter->aData.size();

                        if (nChunkSize > 11)
                        {
                            const std::vector<sal_uInt8>& rData = aIter->aData;
                            nGraphicContentSize = nChunkSize - 11;
                            SvMemoryStream aIStrm(const_cast<sal_uInt8*>(&rData[11]), nGraphicContentSize, StreamMode::READ);
                            pGraphicContent = std::unique_ptr<sal_uInt8[]>(new sal_uInt8[nGraphicContentSize]);
                            sal_uInt64 aCurrentPosition = aIStrm.Tell();
                            aIStrm.ReadBytes(pGraphicContent.get(), nGraphicContentSize);
                            aIStrm.Seek(aCurrentPosition);
                            ImportGIF(aIStrm, rGraphic);
                            eLinkType = GfxLinkType::NativeGif;
                            break;
                        }
                    }
                    ++aIter;
                }
            }

            if ( eLinkType == GfxLinkType::NONE )
            {
                BitmapEx aBmpEx( aPNGReader.Read( aPreviewSizeHint ) );
                if ( aBmpEx.IsEmpty() )
                    nStatus = GRFILTER_FILTERERROR;
                else
                {
                    rGraphic = aBmpEx;
                    eLinkType = GfxLinkType::NativePng;
                }
            }
        }
        else if( aFilterName.equalsIgnoreAsciiCase( IMP_JPEG ) )
        {
            if( rGraphic.IsDummyContext())
                rGraphic.SetDummyContext( false );

            // set LOGSIZE flag always, if not explicitly disabled
            // (see #90508 and #106763)
            if( !( nImportFlags & GraphicFilterImportFlags::DontSetLogsizeForJpeg ) )
                nImportFlags |= GraphicFilterImportFlags::SetLogsizeForJpeg;

            sal_uInt64 nPosition = rIStream.Tell();
            if( !ImportJPEG( rIStream, rGraphic, nImportFlags | GraphicFilterImportFlags::OnlyCreateBitmap, nullptr ) )
                nStatus = GRFILTER_FILTERERROR;
            else
            {
                Bitmap& rBitmap = const_cast<Bitmap&>(rGraphic.GetBitmapExRef().GetBitmapRef());
                Bitmap::ScopedWriteAccess pWriteAccess(rBitmap);
                rIStream.Seek(nPosition);
                if( !ImportJPEG( rIStream, rGraphic, nImportFlags | GraphicFilterImportFlags::UseExistingBitmap, &pWriteAccess ) )
                    nStatus = GRFILTER_FILTERERROR;
                else
                    eLinkType = GfxLinkType::NativeJpg;
            }
        }
        else if( aFilterName.equalsIgnoreAsciiCase( IMP_SVG ) )
        {
            if( rGraphic.IsDummyContext())
                rGraphic.SetDummyContext( false );

            const sal_uInt32 nStreamPosition(rIStream.Tell());
            const sal_uInt32 nStreamLength(rIStream.Seek(STREAM_SEEK_TO_END) - nStreamPosition);

            bool bOkay(false);

            if(nStreamLength > 0)
            {
                std::vector<sal_uInt8> aTwoBytes(2);
                rIStream.Seek(nStreamPosition);
                rIStream.ReadBytes(&aTwoBytes[0], 2);
                rIStream.Seek(nStreamPosition);

                if(aTwoBytes[0] == 0x1F && aTwoBytes[1] == 0x8B)
                {
                    SvMemoryStream aMemStream;
                    ZCodec aCodec;
                    long nMemoryLength;

                    aCodec.BeginCompression(ZCODEC_DEFAULT_COMPRESSION, false, true);
                    nMemoryLength = aCodec.Decompress(rIStream, aMemStream);
                    aCodec.EndCompression();

                    if (!rIStream.GetError() && nMemoryLength >= 0)
                    {
                        SvgDataArray aNewData(nMemoryLength);
                        aMemStream.Seek(STREAM_SEEK_TO_BEGIN);
                        aMemStream.ReadBytes(aNewData.begin(), nMemoryLength);

                        // Make a uncompressed copy for GfxLink
                        nGraphicContentSize = nMemoryLength;
                        pGraphicContent = std::unique_ptr<sal_uInt8[]>(new sal_uInt8[nGraphicContentSize]);
                        std::copy(aNewData.begin(), aNewData.end(), pGraphicContent.get());

                        if(!aMemStream.GetError() )
                        {
                            SvgDataPtr aSvgDataPtr(new SvgData(aNewData, rPath));
                            rGraphic = Graphic(aSvgDataPtr);
                            bOkay = true;
                        }
                    }
                }
                else
                {
                    SvgDataArray aNewData(nStreamLength);
                    rIStream.Seek(nStreamPosition);
                    rIStream.ReadBytes(aNewData.begin(), nStreamLength);

                    if(!rIStream.GetError())
                    {
                        SvgDataPtr aSvgDataPtr(new SvgData(aNewData, rPath));
                        rGraphic = Graphic(aSvgDataPtr);
                        bOkay = true;
                    }
                }
            }

            if (bOkay)
            {
                eLinkType = GfxLinkType::NativeSvg;
            }
            else
            {
                nStatus = GRFILTER_FILTERERROR;
            }
        }
        else if( aFilterName.equalsIgnoreAsciiCase( IMP_XBM ) )
        {
            if( rGraphic.IsDummyContext())
                rGraphic.SetDummyContext( false );

            if( !ImportXBM( rIStream, rGraphic ) )
                nStatus = GRFILTER_FILTERERROR;
        }
        else if( aFilterName.equalsIgnoreAsciiCase( IMP_XPM ) )
        {
            if( rGraphic.IsDummyContext())
                rGraphic.SetDummyContext( false );

            if( !ImportXPM( rIStream, rGraphic ) )
                nStatus = GRFILTER_FILTERERROR;
        }
        else if( aFilterName.equalsIgnoreAsciiCase( IMP_BMP ) ||
                    aFilterName.equalsIgnoreAsciiCase( IMP_SVMETAFILE ) )
        {
            // SV internal filters for import bitmaps and MetaFiles
            ReadGraphic( rIStream, rGraphic );
            if( rIStream.GetError() )
            {
                nStatus = GRFILTER_FORMATERROR;
            }
            else if (aFilterName.equalsIgnoreAsciiCase(IMP_BMP))
            {
                // #i15508# added BMP type (checked, works)
                eLinkType = GfxLinkType::NativeBmp;
            }
        }
        else if( aFilterName.equalsIgnoreAsciiCase( IMP_MOV ) )
        {
            ReadGraphic( rIStream, rGraphic );
            if( rIStream.GetError() )
                nStatus = GRFILTER_FORMATERROR;
            else
            {
                rGraphic.SetDefaultType();
                rIStream.Seek( STREAM_SEEK_TO_END );
                eLinkType = GfxLinkType::NativeMov;
            }
        }
        else if( aFilterName.equalsIgnoreAsciiCase( IMP_WMF ) ||
                aFilterName.equalsIgnoreAsciiCase( IMP_EMF ) )
        {
            GDIMetaFile aMtf;
            if( !ConvertWMFToGDIMetaFile( rIStream, aMtf, nullptr, pExtHeader ) )
                nStatus = GRFILTER_FORMATERROR;
            else
            {
                rGraphic = aMtf;
                eLinkType = GfxLinkType::NativeWmf;
            }
        }
        else if( aFilterName.equalsIgnoreAsciiCase( IMP_SVSGF )
                || aFilterName.equalsIgnoreAsciiCase( IMP_SVSGV ) )
        {
            sal_uInt16          nVersion;
            unsigned char   nTyp = CheckSgfTyp( rIStream, nVersion );

            switch( nTyp )
            {
                case SGF_BITIMAGE:
                {
                    SvMemoryStream aTempStream;
                    if( aTempStream.GetError() )
                        return GRFILTER_OPENERROR;

                    if( !SgfBMapFilter( rIStream, aTempStream ) )
                        nStatus = GRFILTER_FILTERERROR;
                    else
                    {
                        aTempStream.Seek( 0L );
                        ReadGraphic( aTempStream, rGraphic );

                        if( aTempStream.GetError() )
                            nStatus = GRFILTER_FILTERERROR;
                    }
                }
                break;

                case SGF_SIMPVECT:
                {
                    GDIMetaFile aMtf;
                    if( !SgfVectFilter( rIStream, aMtf ) )
                        nStatus = GRFILTER_FILTERERROR;
                    else
                        rGraphic = Graphic( aMtf );
                }
                break;

                case SGF_STARDRAW:
                {
                    if( nVersion != SGV_VERSION )
                        nStatus = GRFILTER_VERSIONERROR;
                    else
                    {
                        GDIMetaFile aMtf;
                        if( !SgfSDrwFilter( rIStream, aMtf,
                                INetURLObject(aFilterPath) ) )
                        {
                            nStatus = GRFILTER_FILTERERROR;
                        }
                        else
                            rGraphic = Graphic( aMtf );
                    }
                }
                break;

                default:
                {
                    nStatus = GRFILTER_FORMATERROR;
                }
                break;
            }
        }
        else if (aFilterName == IMP_PDF)
        {
            if (!vcl::ImportPDF(rIStream, rGraphic))
                nStatus = GRFILTER_FILTERERROR;
            else
                eLinkType = GfxLinkType::NativePdf;
        }
        else
            nStatus = GRFILTER_FILTERERROR;
    }
    else
    {
        ImpFilterLibCacheEntry* pFilter = nullptr;

        // find first filter in filter paths
        sal_Int32 i, nTokenCount = getTokenCount(aFilterPath, ';');
        ImpFilterLibCache &rCache = Cache::get();
        for( i = 0; ( i < nTokenCount ) && ( pFilter == nullptr ); i++ )
            pFilter = rCache.GetFilter(aFilterPath.getToken(i, ';'), aFilterName, aExternalFilterName);
        if( !pFilter )
            nStatus = GRFILTER_FILTERERROR;
        else
        {
            PFilterCall pFunc = pFilter->GetImportFunction();

            if( !pFunc )
                nStatus = GRFILTER_FILTERERROR;
            else
            {
                OUString aShortName;
                if( nFormat != GRFILTER_FORMAT_DONTKNOW )
                {
                    aShortName = GetImportFormatShortName( nFormat ).toAsciiUpperCase();
                    if ( ( !pFilterConfigItem ) && aShortName == "PCD" )
                    {
                        OUString aFilterConfigPath( "Office.Common/Filter/Graphic/Import/PCD" );
                        pFilterConfigItem = o3tl::make_unique<FilterConfigItem>( aFilterConfigPath );
                    }
                }
                if( !(*pFunc)( rIStream, rGraphic, pFilterConfigItem.get() ) )
                    nStatus = GRFILTER_FORMATERROR;
                else
                {
                    // try to set link type if format matches
                    if( nFormat != GRFILTER_FORMAT_DONTKNOW )
                    {
                        if( aShortName.startsWith( TIF_SHORTNAME ) )
                            eLinkType = GfxLinkType::NativeTif;
                        else if( aShortName.startsWith( MET_SHORTNAME ) )
                            eLinkType = GfxLinkType::NativeMet;
                        else if( aShortName.startsWith( PCT_SHORTNAME ) )
                            eLinkType = GfxLinkType::NativePct;
                    }
                }
            }
        }
    }

    if( nStatus == GRFILTER_OK && bCreateNativeLink && ( eLinkType != GfxLinkType::NONE ) && !rGraphic.GetContext() && !bLinkSet )
    {
        if (!pGraphicContent)
        {
            const sal_uLong nStreamEnd = rIStream.Tell();
            nGraphicContentSize = nStreamEnd - nStreamBegin;

            if (nGraphicContentSize > 0)
            {
                try
                {
                    pGraphicContent =  std::unique_ptr<sal_uInt8[]>(new sal_uInt8[nGraphicContentSize]);
                }
                catch (const std::bad_alloc&)
                {
                    nStatus = GRFILTER_TOOBIG;
                }

                if( nStatus == GRFILTER_OK )
                {
                    rIStream.Seek(nStreamBegin);
                    rIStream.ReadBytes(pGraphicContent.get(), nGraphicContentSize);
                }
            }
        }
        if( nStatus == GRFILTER_OK )
        {
            rGraphic.SetLink( GfxLink( std::move(pGraphicContent), nGraphicContentSize, eLinkType ) );
        }
    }

    // Set error code or try to set native buffer
    if( nStatus != GRFILTER_OK )
    {
        ImplSetError( nStatus, &rIStream );
        rIStream.Seek( nStreamBegin );
        rGraphic.Clear();
    }

    return nStatus;
}

sal_uInt16 GraphicFilter::ExportGraphic( const Graphic& rGraphic, const INetURLObject& rPath,
    sal_uInt16 nFormat, const css::uno::Sequence< css::beans::PropertyValue >* pFilterData )
{
    SAL_INFO( "vcl.filter", "GraphicFilter::ExportGraphic() (thb)" );
    sal_uInt16  nRetValue = GRFILTER_FORMATERROR;
    SAL_WARN_IF( rPath.GetProtocol() == INetProtocol::NotValid, "vcl.filter", "GraphicFilter::ExportGraphic() : ProtType == INetProtocol::NotValid" );
    bool bAlreadyExists = DirEntryExists( rPath );

    OUString    aMainUrl( rPath.GetMainURL( INetURLObject::DecodeMechanism::NONE ) );
    std::unique_ptr<SvStream> xStream(::utl::UcbStreamHelper::CreateStream( aMainUrl, StreamMode::WRITE | StreamMode::TRUNC ));
    if (xStream)
    {
        nRetValue = ExportGraphic( rGraphic, aMainUrl, *xStream, nFormat, pFilterData );
        xStream.reset();

        if( ( GRFILTER_OK != nRetValue ) && !bAlreadyExists )
            KillDirEntry( aMainUrl );
    }
    return nRetValue;
}

#ifdef DISABLE_DYNLOADING

extern "C" bool egiGraphicExport( SvStream& rStream, Graphic& rGraphic, FilterConfigItem* pConfigItem );
extern "C" bool epsGraphicExport( SvStream& rStream, Graphic& rGraphic, FilterConfigItem* pConfigItem );
extern "C" bool etiGraphicExport( SvStream& rStream, Graphic& rGraphic, FilterConfigItem* pConfigItem );

#endif

sal_uInt16 GraphicFilter::ExportGraphic( const Graphic& rGraphic, const OUString& rPath,
    SvStream& rOStm, sal_uInt16 nFormat, const css::uno::Sequence< css::beans::PropertyValue >* pFilterData )
{
    SAL_INFO( "vcl.filter", "GraphicFilter::ExportGraphic() (thb)" );
    sal_uInt16 nFormatCount = GetExportFormatCount();

    ResetLastError();
    nExpGraphHint = 0;

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
        return (sal_uInt16) ImplSetError( GRFILTER_FORMATERROR );

    FilterConfigItem aConfigItem( const_cast<css::uno::Sequence< css::beans::PropertyValue >*>(pFilterData) );
    OUString aFilterName( pConfig->GetExportFilterName( nFormat ) );
#ifndef DISABLE_DYNLOADING
    OUString aExternalFilterName(pConfig->GetExternalFilterName(nFormat, true));
#endif
    sal_uInt16      nStatus = GRFILTER_OK;
    GraphicType eType;
    Graphic     aGraphic( rGraphic );

    aGraphic = ImpGetScaledGraphic( rGraphic, aConfigItem );
    eType = aGraphic.GetType();

    if( pConfig->IsExportPixelFormat( nFormat ) )
    {
        if( eType != GraphicType::Bitmap )
        {
            Size aSizePixel;
            sal_uLong nColorCount,nBitsPerPixel,nNeededMem,nMaxMem;
            ScopedVclPtrInstance< VirtualDevice > aVirDev;

            nMaxMem = 1024;
            nMaxMem *= 1024; // In Bytes

            // Calculate how big the image would normally be:
            aSizePixel=aVirDev->LogicToPixel(aGraphic.GetPrefSize(),aGraphic.GetPrefMapMode());

            // Calculate how much memory the image will take up
            nColorCount=aVirDev->GetColorCount();
            if      (nColorCount<=2)     nBitsPerPixel=1;
            else if (nColorCount<=4)     nBitsPerPixel=2;
            else if (nColorCount<=16)    nBitsPerPixel=4;
            else if (nColorCount<=256)   nBitsPerPixel=8;
            else if (nColorCount<=65536) nBitsPerPixel=16;
            else                         nBitsPerPixel=24;
            nNeededMem=((sal_uLong)aSizePixel.Width()*(sal_uLong)aSizePixel.Height()*nBitsPerPixel+7)/8;

            // is the image larger than available memory?
            if (nMaxMem<nNeededMem)
            {
                double fFak=sqrt(((double)nMaxMem)/((double)nNeededMem));
                aSizePixel.Width()=(sal_uLong)(((double)aSizePixel.Width())*fFak);
                aSizePixel.Height()=(sal_uLong)(((double)aSizePixel.Height())*fFak);
            }

            aVirDev->SetMapMode(MapMode(MapUnit::MapPixel));
            aVirDev->SetOutputSizePixel(aSizePixel);
            Graphic aGraphic2=aGraphic;
            aGraphic2.Draw(aVirDev.get(),Point(0,0),aSizePixel); // this changes the MapMode
            aVirDev->SetMapMode(MapMode(MapUnit::MapPixel));
            aGraphic=Graphic(aVirDev->GetBitmap(Point(0,0),aSizePixel));
        }
    }
    if( rOStm.GetError() )
        nStatus = GRFILTER_IOERROR;
    if( GRFILTER_OK == nStatus )
    {
        if ( pConfig->IsExportInternalFilter( nFormat ) )
        {
            if( aFilterName.equalsIgnoreAsciiCase( EXP_BMP ) )
            {
                Bitmap aBmp( aGraphic.GetBitmap() );
                BmpConversion nColorRes = (BmpConversion) aConfigItem.ReadInt32( "Colors", 0 );
                if ( nColorRes != BmpConversion::NNONE && ( nColorRes <= BmpConversion::N24Bit) )
                {
                    if( !aBmp.Convert( nColorRes ) )
                        aBmp = aGraphic.GetBitmap();
                }
                bool    bRleCoding = aConfigItem.ReadBool( "RLE_Coding", true );
                // save RLE encoded?
                WriteDIB(aBmp, rOStm, bRleCoding, true);

                if( rOStm.GetError() )
                    nStatus = GRFILTER_IOERROR;
            }
            else if( aFilterName.equalsIgnoreAsciiCase( EXP_SVMETAFILE ) )
            {
                sal_Int32 nVersion = aConfigItem.ReadInt32( "Version", 0 ) ;
                if ( nVersion )
                    rOStm.SetVersion( nVersion );

                // #i119735# just use GetGDIMetaFile, it will create a bufferd version of contained bitmap now automatically
                GDIMetaFile aMTF(aGraphic.GetGDIMetaFile());

                aMTF.Write( rOStm );

                if( rOStm.GetError() )
                    nStatus = GRFILTER_IOERROR;
            }
            else if ( aFilterName.equalsIgnoreAsciiCase( EXP_WMF ) )
            {
                // #i119735# just use GetGDIMetaFile, it will create a bufferd version of contained bitmap now automatically
                if ( !ConvertGDIMetaFileToWMF( aGraphic.GetGDIMetaFile(), rOStm, &aConfigItem ) )
                    nStatus = GRFILTER_FORMATERROR;

                if( rOStm.GetError() )
                    nStatus = GRFILTER_IOERROR;
            }
            else if ( aFilterName.equalsIgnoreAsciiCase( EXP_EMF ) )
            {
                // #i119735# just use GetGDIMetaFile, it will create a bufferd version of contained bitmap now automatically
                if ( !ConvertGDIMetaFileToEMF(aGraphic.GetGDIMetaFile(), rOStm))
                    nStatus = GRFILTER_FORMATERROR;

                if( rOStm.GetError() )
                    nStatus = GRFILTER_IOERROR;
            }
            else if( aFilterName.equalsIgnoreAsciiCase( EXP_JPEG ) )
            {
                bool bExportedGrayJPEG = false;
                if( !ExportJPEG( rOStm, aGraphic, pFilterData, &bExportedGrayJPEG ) )
                    nStatus = GRFILTER_FORMATERROR;
                nExpGraphHint = bExportedGrayJPEG ? GRFILTER_OUTHINT_GREY : 0;

                if( rOStm.GetError() )
                    nStatus = GRFILTER_IOERROR;
            }
            else if ( aFilterName.equalsIgnoreAsciiCase( EXP_PNG ) )
            {
                vcl::PNGWriter aPNGWriter( aGraphic.GetBitmapEx(), pFilterData );
                if ( pFilterData )
                {
                    sal_Int32 k, j, i = 0;
                    for ( i = 0; i < pFilterData->getLength(); i++ )
                    {
                        if ( (*pFilterData)[ i ].Name == "AdditionalChunks" )
                        {
                            css::uno::Sequence< css::beans::PropertyValue > aAdditionalChunkSequence;
                            if ( (*pFilterData)[ i ].Value >>= aAdditionalChunkSequence )
                            {
                                for ( j = 0; j < aAdditionalChunkSequence.getLength(); j++ )
                                {
                                    if ( aAdditionalChunkSequence[ j ].Name.getLength() == 4 )
                                    {
                                        sal_uInt32 nChunkType = 0;
                                        for ( k = 0; k < 4; k++ )
                                        {
                                            nChunkType <<= 8;
                                            nChunkType |= (sal_uInt8)aAdditionalChunkSequence[ j ].Name[ k ];
                                        }
                                        css::uno::Sequence< sal_Int8 > aByteSeq;
                                        if ( aAdditionalChunkSequence[ j ].Value >>= aByteSeq )
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
                                                    memcpy( &aChunkData.aData[ 0 ], aByteSeq.getConstArray(), nChunkLen );
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
                    nStatus = GRFILTER_IOERROR;
            }
            else if( aFilterName.equalsIgnoreAsciiCase( EXP_SVG ) )
            {
                bool bDone(false);

                // do we have a native SVG RenderGraphic, whose data can be written directly?
                const SvgDataPtr& aSvgDataPtr(rGraphic.getSvgData());

                if (aSvgDataPtr.get() && aSvgDataPtr->getSvgDataArrayLength())
                {
                    rOStm.WriteBytes(aSvgDataPtr->getSvgDataArray().getConstArray(), aSvgDataPtr->getSvgDataArrayLength());

                    if( rOStm.GetError() )
                    {
                        nStatus = GRFILTER_IOERROR;
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
                        css::uno::Sequence< css::uno::Any > aArguments( 1 );
                        aArguments[ 0 ] <<= aConfigItem.GetFilterData();
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
                                const_cast<GDIMetaFile&>( aGraphic.GetGDIMetaFile() ).Write( aMemStm );

                                xActiveDataSource->setOutputStream( css::uno::Reference< css::io::XOutputStream >(
                                    xStmIf, css::uno::UNO_QUERY ) );
                                css::uno::Sequence< sal_Int8 > aMtfSeq( static_cast<sal_Int8 const *>(aMemStm.GetData()), aMemStm.Tell() );
                                xSVGWriter->write( xSaxWriter, aMtfSeq );
                            }
                        }
                    }
                    catch(const css::uno::Exception&)
                    {
                        nStatus = GRFILTER_IOERROR;
                    }
                }
            }
            else
                nStatus = GRFILTER_FILTERERROR;
        }
        else
        {
            sal_Int32 i, nTokenCount = getTokenCount(aFilterPath, ';');
            for ( i = 0; i < nTokenCount; i++ )
            {
#ifndef DISABLE_DYNLOADING
                OUString aPhysicalName( ImpCreateFullFilterPath( aFilterPath.getToken(i, ';'), aFilterName ) );
                osl::Module aLibrary( aPhysicalName );

                PFilterCall pFunc = nullptr;
                if (aExternalFilterName == "egi")
                    pFunc = reinterpret_cast<PFilterCall>(aLibrary.getFunctionSymbol("egiGraphicExport"));
                else if (aExternalFilterName == "eps")
                    pFunc = reinterpret_cast<PFilterCall>(aLibrary.getFunctionSymbol("epsGraphicExport"));
                else if (aExternalFilterName == "eti")
                    pFunc = reinterpret_cast<PFilterCall>(aLibrary.getFunctionSymbol("etiGraphicExport"));
                 // Execute dialog in DLL
 #else
                PFilterCall pFunc = NULL;
                if (aFilterName == "egi")
                    pFunc = egiGraphicExport;
                else if (aFilterName == "eps")
                    pFunc = epsGraphicExport;
                else if (aFilterName == "eti")
                    pFunc = etiGraphicExport;
 #endif
                if( pFunc )
                {
                    if ( !(*pFunc)( rOStm, aGraphic, &aConfigItem ) )
                        nStatus = GRFILTER_FORMATERROR;
                    break;
                }
                else
                    nStatus = GRFILTER_FILTERERROR;
            }
        }
    }
    if( nStatus != GRFILTER_OK )
    {
        ImplSetError( nStatus, &rOStm );
    }
    return nStatus;
}


void GraphicFilter::ResetLastError()
{
    pErrorEx->nFilterError = pErrorEx->nStreamError = 0UL;
}

const Link<ConvertData&,bool> GraphicFilter::GetFilterCallback() const
{
    const Link<ConvertData&,bool> aLink( LINK( const_cast<GraphicFilter*>(this), GraphicFilter, FilterCallback ) );
    return aLink;
}

IMPL_LINK( GraphicFilter, FilterCallback, ConvertData&, rData, bool )
{
    bool bRet = false;

    sal_uInt16      nFormat = GRFILTER_FORMAT_DONTKNOW;
    OString aShortName;
    switch( rData.mnFormat )
    {
        case( ConvertDataFormat::BMP ): aShortName = BMP_SHORTNAME; break;
        case( ConvertDataFormat::GIF ): aShortName = GIF_SHORTNAME; break;
        case( ConvertDataFormat::JPG ): aShortName = JPG_SHORTNAME; break;
        case( ConvertDataFormat::MET ): aShortName = MET_SHORTNAME; break;
        case( ConvertDataFormat::PCT ): aShortName = PCT_SHORTNAME; break;
        case( ConvertDataFormat::PNG ): aShortName = PNG_SHORTNAME; break;
        case( ConvertDataFormat::SVM ): aShortName = SVM_SHORTNAME; break;
        case( ConvertDataFormat::TIF ): aShortName = TIF_SHORTNAME; break;
        case( ConvertDataFormat::WMF ): aShortName = WMF_SHORTNAME; break;
        case( ConvertDataFormat::EMF ): aShortName = EMF_SHORTNAME; break;
        case( ConvertDataFormat::SVG ): aShortName = SVG_SHORTNAME; break;

        default:
        break;
    }
    if( GraphicType::NONE == rData.maGraphic.GetType() || rData.maGraphic.GetContext() ) // Import
    {
        // Import
        nFormat = GetImportFormatNumberForShortName( OStringToOUString( aShortName, RTL_TEXTENCODING_UTF8) );
        bRet = ImportGraphic( rData.maGraphic, OUString(), rData.mrStm, nFormat ) == 0;
    }
    else if( !aShortName.isEmpty() )
    {
        // Export
        nFormat = GetExportFormatNumberForShortName( OStringToOUString(aShortName, RTL_TEXTENCODING_UTF8) );
        bRet = ExportGraphic( rData.maGraphic, OUString(), rData.mrStm, nFormat ) == 0;
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

    class theGraphicFilter : public rtl::Static<StandardGraphicFilter, theGraphicFilter> {};
}

GraphicFilter& GraphicFilter::GetGraphicFilter()
{
    return theGraphicFilter::get().m_aFilter;
}

int GraphicFilter::LoadGraphic( const OUString &rPath, const OUString &rFilterName,
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

    SvStream* pStream = nullptr;
    if ( INetProtocol::File != aURL.GetProtocol() )
    {
        pStream = ::utl::UcbStreamHelper::CreateStream( rPath, StreamMode::READ );
    }

    int nRes = GRFILTER_OK;
    if ( !pStream )
        nRes = pFilter->ImportGraphic( rGraphic, aURL, nFilter, pDeterminedFormat );
    else
        nRes = pFilter->ImportGraphic( rGraphic, rPath, *pStream, nFilter, pDeterminedFormat );

#ifdef DBG_UTIL
    OUString aReturnString;

    switch (nRes)
    {
        case GRFILTER_OPENERROR:
            aReturnString="open error";
            break;
        case GRFILTER_IOERROR:
            aReturnString="IO error";
            break;
        case GRFILTER_FORMATERROR:
            aReturnString="format error";
            break;
        case GRFILTER_VERSIONERROR:
            aReturnString="version error";
            break;
        case GRFILTER_FILTERERROR:
            aReturnString="filter error";
            break;
        case GRFILTER_ABORT:
            aReturnString="import aborted";
            break;
        case GRFILTER_TOOBIG:
            aReturnString="graphic is too big";
            break;
        default:
            // nothing more to do
            break;
    }

    SAL_INFO_IF( nRes, "vcl.filter", "Problem importing graphic " << rPath << ". Reason: " << aReturnString );
#endif

    return nRes;
}

sal_uInt16 GraphicFilter::compressAsPNG(const Graphic& rGraphic, SvStream& rOutputStream)
{
    css::uno::Sequence< css::beans::PropertyValue > aFilterData(1);
    aFilterData[0].Name = "Compression";
    aFilterData[0].Value <<= (sal_uInt32) 9;

    sal_uInt16 nFilterFormat = GetExportFormatNumberForShortName("PNG");
    return ExportGraphic(rGraphic, OUString(), rOutputStream, nFilterFormat, &aFilterData);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
