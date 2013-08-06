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

#include <osl/mutex.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <ucbhelper/content.hxx>
#include <cppuhelper/implbase1.hxx>
#include <tools/urlobj.hxx>
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
#include "igif/gifread.hxx"
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
#include <unotools/localfilehelper.hxx>
#include <rtl/bootstrap.hxx>
#include <rtl/instance.hxx>
#include <vcl/metaact.hxx>
#include <vector>

#include "FilterConfigCache.hxx"

#define PMGCHUNG_msOG       0x6d734f47      // Microsoft Office Animated GIF

#ifndef DISABLE_DYNLOADING
#define IMPORT_FUNCTION_NAME    "GraphicImport"
#define EXPORT_FUNCTION_NAME    "GraphicExport"
#endif

using namespace ::rtl;
using namespace ::com::sun::star;

using comphelper::string::getTokenCount;
using comphelper::string::getToken;

typedef ::std::vector< GraphicFilter* > FilterList_impl;
static FilterList_impl* pFilterHdlList = NULL;

static ::osl::Mutex& getListMutex()
{
    static ::osl::Mutex s_aListProtection;
    return s_aListProtection;
}

class ImpFilterOutputStream : public ::cppu::WeakImplHelper1< css::io::XOutputStream >
{
protected:

    SvStream&               mrStm;

    virtual void SAL_CALL   writeBytes( const css::uno::Sequence< sal_Int8 >& rData )
        throw (css::io::NotConnectedException, css::io::BufferSizeExceededException, css::io::IOException, css::uno::RuntimeException)
        { mrStm.Write( rData.getConstArray(), rData.getLength() ); }
    virtual void SAL_CALL   flush()
        throw (css::io::NotConnectedException, css::io::BufferSizeExceededException, css::io::IOException, css::uno::RuntimeException)
        { mrStm.Flush(); }
    virtual void SAL_CALL   closeOutput() throw() {}

public:

    ImpFilterOutputStream( SvStream& rStm ) : mrStm( rStm ) {}
    ~ImpFilterOutputStream() {}
};

#ifndef DISABLE_EXPORT

static bool DirEntryExists( const INetURLObject& rObj )
{
    bool bExists = false;

    try
    {
        ::ucbhelper::Content aCnt( rObj.GetMainURL( INetURLObject::NO_DECODE ),
                             css::uno::Reference< css::ucb::XCommandEnvironment >(),
                             comphelper::getProcessComponentContext() );

        bExists = aCnt.isDocument();
    }
    catch(const css::ucb::CommandAbortedException&)
    {
        SAL_WARN( "svtools.filter", "CommandAbortedException" );
    }
    catch(const css::ucb::ContentCreationException&)
    {
        SAL_WARN( "svtools.filter", "ContentCreationException" );
    }
    catch( ... )
    {
        SAL_WARN( "svtools.filter", "Any other exception" );
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
                             css::uno::makeAny( sal_Bool( sal_True ) ) );
    }
    catch(const css::ucb::CommandAbortedException&)
    {
        SAL_WARN( "svtools.filter", "CommandAbortedException" );
    }
    catch( ... )
    {
        SAL_WARN( "svtools.filter", "Any other exception" );
    }
}

#endif // !DISABLE_EXPORT

// Helper functions

sal_uInt8* ImplSearchEntry( sal_uInt8* pSource, sal_uInt8* pDest, sal_uLong nComp, sal_uLong nSize )
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
    return NULL;
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
    sal_uInt16 oldNumberFormat = rStream.GetNumberFormatInt();
    sal_uInt32 nOffset; // in MS documents the pict format is used without the first 512 bytes
    for ( nOffset = 0; ( nOffset <= 512 ) && ( ( nStreamPos + nOffset + 14 ) <= nStreamLen ); nOffset += 512 )
    {
        short y1,x1,y2,x2;
        bool bdBoxOk = true;

        rStream.Seek( nStreamPos + nOffset);
        // size of the pict in version 1 pict ( 2bytes) : ignored
        rStream.SeekRel(2);
        // bounding box (bytes 2 -> 9)
        rStream.SetNumberFormatInt(NUMBERFORMAT_INT_BIGENDIAN);
        rStream >> y1 >> x1 >> y2 >> x2;
        rStream.SetNumberFormatInt(oldNumberFormat); // reset format

        if (x1 > x2 || y1 > y2 || // bad bdbox
            (x1 == x2 && y1 == y2) || // 1 pixel picture
            x2-x1 > 2048 || y2-y1 > 2048 ) // picture anormaly big
          bdBoxOk = false;

        // read version op
        rStream.Read( sBuf,3 );
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
    sal_uInt16  i;
    sal_uInt8    sFirstBytes[ 256 ];
    sal_uLong   nFirstLong,nSecondLong;
    sal_uLong   nStreamPos = rStream.Tell();

    rStream.Seek( STREAM_SEEK_TO_END );
    sal_uLong nStreamLen = rStream.Tell() - nStreamPos;
    rStream.Seek( nStreamPos );

    if ( !nStreamLen )
    {
        SvLockBytes* pLockBytes = rStream.GetLockBytes();
        if ( pLockBytes  )
            pLockBytes->SetSynchronMode( sal_True );

        rStream.Seek( STREAM_SEEK_TO_END );
        nStreamLen = rStream.Tell() - nStreamPos;
        rStream.Seek( nStreamPos );
    }
    if (!nStreamLen)
    {
        return false; // this prevents at least a STL assertion
    }
    else if (nStreamLen >= 256)
    {   // load first 256 bytes into a buffer
        rStream.Read( sFirstBytes, 256 );
    }
    else
    {
        rStream.Read( sFirstBytes, nStreamLen );

        for( i = (sal_uInt16) nStreamLen; i < 256; i++ )
            sFirstBytes[ i ]=0;
    }

    if( rStream.GetError() )
        return false;

    // Accommodate the first 8 bytes in nFirstLong, nSecondLong
    // Big-Endian:
    for( i = 0, nFirstLong = 0L, nSecondLong = 0L; i < 4; i++ )
    {
        nFirstLong=(nFirstLong<<8)|(sal_uLong)sFirstBytes[i];
        nSecondLong=(nSecondLong<<8)|(sal_uLong)sFirstBytes[i+4];
    }

    // The following variable is used when bTest == true. It remains sal_False
    // if the format (rFormatExtension) has not yet been set.
    bool bSomethingTested = false;

    // Now the different formats are checked. The order *does* matter. e.g. a MET file
    // could also go through the BMP test, howeve a BMP file can hardly go through the MET test.
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
            rStream.SetNumberFormatInt( NUMBERFORMAT_INT_BIGENDIAN );
            rStream.Seek( nStreamPos );
            sal_uInt16 nFieldSize;
            sal_uInt8 nMagic;
            bool bOK=true;
            rStream >> nFieldSize >> nMagic;
            for (i=0; i<3; i++) {
                if (nFieldSize<6) { bOK=false; break; }
                if (nStreamLen < rStream.Tell() + nFieldSize ) { bOK=false; break; }
                rStream.SeekRel(nFieldSize-3);
                rStream >> nFieldSize >> nMagic;
                if (nMagic!=0xd3) { bOK=false; break; }
            }
            rStream.SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
            if (bOK && !rStream.GetError()) {
                rFormatExtension = OUString("MET");
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
                rFormatExtension = OUString("BMP");
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
            rFormatExtension = OUString("WMF");
            return true;
        }
        else if( nFirstLong == 0x01000000 && sFirstBytes[ 40 ] == 0x20 && sFirstBytes[ 41 ] == 0x45 &&
            sFirstBytes[ 42 ] == 0x4d && sFirstBytes[ 43 ] == 0x46 )
        {
            rFormatExtension = OUString("EMF");
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
                rFormatExtension = OUString("PCX");
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
            rFormatExtension = OUString("TIF");
            return true;
        }
    }

    //--------------------------- GIF ------------------------------------
    if( !bTest || rFormatExtension.startsWith( "GIF" ) )
    {
        bSomethingTested=true;
        if ( nFirstLong==0x47494638 && (sFirstBytes[4]==0x37 || sFirstBytes[4]==0x39) && sFirstBytes[5]==0x61 )
        {
            rFormatExtension = OUString("GIF");
            return true;
        }
    }

    //--------------------------- PNG ------------------------------------
    if( !bTest || rFormatExtension.startsWith( "PNG" ) )
    {
        bSomethingTested=true;
        if (nFirstLong==0x89504e47 && nSecondLong==0x0d0a1a0a)
        {
            rFormatExtension = OUString("PNG");
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
            rFormatExtension = OUString("JPG");
            return true;
        }
    }

    //--------------------------- SVM ------------------------------------
    if( !bTest || rFormatExtension.startsWith( "SVM" ) )
    {
        bSomethingTested=true;
        if( nFirstLong==0x53564744 && sFirstBytes[4]==0x49 )
        {
            rFormatExtension = OUString("SVM");
            return true;
        }
        else if( sFirstBytes[0]==0x56 && sFirstBytes[1]==0x43 && sFirstBytes[2]==0x4C &&
                 sFirstBytes[3]==0x4D && sFirstBytes[4]==0x54 && sFirstBytes[5]==0x46 )
        {
            rFormatExtension = OUString("SVM");
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
            rStream.Read( sBuf, 7 );

            if( strncmp( sBuf, "PCD_IPI", 7 ) ==  0 )
            {
                rFormatExtension = OUString("PCD");
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
            rFormatExtension = OUString("PSD");
            return true;
        }
    }

    //--------------------------- EPS ------------------------------------
    if( !bTest || rFormatExtension.startsWith( "EPS" ) )
    {
        bSomethingTested = true;
        if ( ( nFirstLong == 0xC5D0D3C6 ) || ( ImplSearchEntry( sFirstBytes, (sal_uInt8*)"%!PS-Adobe", 10, 10 ) &&
             ImplSearchEntry( &sFirstBytes[15], (sal_uInt8*)"EPS", 3, 3 ) ) )
        {
            rFormatExtension = OUString("EPS");
            return true;
        }
    }

    //--------------------------- DXF ------------------------------------
    if( !bTest || rFormatExtension.startsWith( "DXF" ) )
    {
        // Binary DXF File Format
        if( strncmp( (const char*) sFirstBytes, "AutoCAD Binary DXF", 18 ) == 0 )
        {
            rFormatExtension = OUString("DXF");
            return true;
        }

        // ASCII DXF File Format
        i=0;
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

            if (i+7<256 && (strncmp((const char*)(sFirstBytes+i),"SECTION",7)==0))
            {
                rFormatExtension = OUString("DXF");
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
            rFormatExtension = OUString("PCT");
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
                    rFormatExtension = OUString("PBM");
                return true;

                case '2' :
                case '5' :
                    rFormatExtension = OUString("PGM");
                return true;

                case '3' :
                case '6' :
                    rFormatExtension = OUString("PPM");
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
            rFormatExtension = OUString("RAS");
            return true;
        }
    }

    //--------------------------- XPM ------------------------------------
    if( !bTest )
    {
        bSomethingTested = true;
        if( ImplSearchEntry( sFirstBytes, (sal_uInt8*)"/* XPM */", 256, 9 ) )
        {
            rFormatExtension = OUString("XPM");
            return true;
        }
    }
    else if( rFormatExtension.startsWith( "XPM" ) )
    {
        bSomethingTested = true;
        return true;
    }

    //--------------------------- XBM ------------------------------------
    if( !bTest )
    {
        sal_uLong nSize = ( nStreamLen > 2048 ) ? 2048 : nStreamLen;
        sal_uInt8* pBuf = new sal_uInt8 [ nSize ];

        rStream.Seek( nStreamPos );
        rStream.Read( pBuf, nSize );
        sal_uInt8* pPtr = ImplSearchEntry( pBuf, (sal_uInt8*)"#define", nSize, 7 );

        if( pPtr )
        {
            if( ImplSearchEntry( pPtr, (sal_uInt8*)"_width", pBuf + nSize - pPtr, 6 ) )
            {
                rFormatExtension = OUString("XBM");
                delete[] pBuf;
                return true;
            }
        }
        delete[] pBuf;
    }
    else if( rFormatExtension.startsWith( "XBM" ) )
    {
        bSomethingTested = true;
        return true;
    }

    //--------------------------- SVG ------------------------------------
    if( !bTest )
    {
        // check for Xml
        if( ImplSearchEntry( sFirstBytes, (sal_uInt8*)"<?xml", 256, 5 ) // is it xml
            && ImplSearchEntry( sFirstBytes, (sal_uInt8*)"version", 256, 7 )) // does it have a version (required for xml)
        {
            bool bIsSvg(false);

            // check for DOCTYPE svg combination
            if( ImplSearchEntry( sFirstBytes, (sal_uInt8*)"DOCTYPE", 256, 7 ) // 'DOCTYPE' is there
                && ImplSearchEntry( sFirstBytes, (sal_uInt8*)"svg", 256, 3 )) // 'svg' is there
            {
                bIsSvg = true;
            }

            // check for svg element in 1st 256 bytes
            if(!bIsSvg && ImplSearchEntry( sFirstBytes, (sal_uInt8*)"<svg", 256, 4 )) // '<svg'
            {
                bIsSvg = true;
            }

            if(!bIsSvg)
            {
                // it's a xml, look for '<svg' in full file. Should not happen too
                // often since the tests above will handle most cases, but can happen
                // with Svg files containing big comment headers or Svg as the host
                // language
                const sal_uLong nSize((nStreamLen > 2048) ? 2048 : nStreamLen);
                sal_uInt8* pBuf = new sal_uInt8[nSize];

                rStream.Seek(nStreamPos);
                rStream.Read(pBuf, nSize);

                if(ImplSearchEntry(pBuf, (sal_uInt8*)"<svg", nSize, 4)) // '<svg'
                {
                    bIsSvg = true;
                }

                delete[] pBuf;
            }

            if(bIsSvg)
            {
                rFormatExtension = OUString( "SVG" );
                return true;
            }
        }
        else
        {
            // #119176# SVG files which have no xml header at all have shown up,
            // detect those, too
            bool bIsSvg(false);

            // check for svg element in 1st 256 bytes
            if(ImplSearchEntry( sFirstBytes, (sal_uInt8*)"<svg", 256, 4 )) // '<svg'
            {
                bIsSvg = true;
            }

            if(!bIsSvg)
            {
                // look for '<svg' in full file. Should not happen too
                // often since the tests above will handle most cases, but can happen
                // with SVG files containing big comment headers or SVG as the host
                // language
                const sal_uLong nSize((nStreamLen > 2048) ? 2048 : nStreamLen);
                sal_uInt8* pBuf = new sal_uInt8[nSize];

                rStream.Seek(nStreamPos);
                rStream.Read(pBuf, nSize);

                if(ImplSearchEntry(pBuf, (sal_uInt8*)"<svg", nSize, 4)) // '<svg'
                {
                    bIsSvg = true;
                }

                delete[] pBuf;
            }

            if(bIsSvg)
            {
                rFormatExtension = OUString( "SVG" );
                return true;
            }
        }
    }
    else if( rFormatExtension.startsWith( "SVG" ) )
    {
        bSomethingTested = true;
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
            rFormatExtension = OUString("SGF");
            return true;
        }
    }

    return bTest && !bSomethingTested;
}

//--------------------------------------------------------------------------

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
        if ( pConfig->GetImportFormatExtension( rFormat ).EqualsIgnoreCaseAscii( "pcd" ) )
        {
            sal_Int32 nBase = 2;    // default Base0
            if ( pConfig->GetImportFilterType( rFormat ).EqualsIgnoreCaseAscii( "pcd_Photo_CD_Base4" ) )
                nBase = 1;
            else if ( pConfig->GetImportFilterType( rFormat ).EqualsIgnoreCaseAscii( "pcd_Photo_CD_Base16" ) )
                nBase = 0;
            OUString aFilterConfigPath( "Office.Common/Filter/Graphic/Import/PCD" );
            FilterConfigItem aFilterConfigItem( aFilterConfigPath );
            aFilterConfigItem.WriteInt32( "Resolution", nBase );
        }
    }

    return GRFILTER_OK;
}

//--------------------------------------------------------------------------

#ifndef DISABLE_EXPORT

static Graphic ImpGetScaledGraphic( const Graphic& rGraphic, FilterConfigItem& rConfigItem )
{
    Graphic     aGraphic;

    ResMgr*     pResMgr = ResMgr::CreateResMgr( "svt", Application::GetSettings().GetUILanguageTag() );

    sal_Int32 nLogicalWidth = rConfigItem.ReadInt32( "LogicalWidth", 0 );
    sal_Int32 nLogicalHeight = rConfigItem.ReadInt32( "LogicalHeight", 0 );

    if ( rGraphic.GetType() != GRAPHIC_NONE )
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
        if ( aPrefMapMode == MAP_PIXEL )
            aOriginalSize = Application::GetDefaultDevice()->PixelToLogic( aPrefSize, MAP_100TH_MM );
        else
            aOriginalSize = Application::GetDefaultDevice()->LogicToLogic( aPrefSize, aPrefMapMode, MAP_100TH_MM );
        if ( !nLogicalWidth )
            nLogicalWidth = aOriginalSize.Width();
        if ( !nLogicalHeight )
            nLogicalHeight = aOriginalSize.Height();
        if( rGraphic.GetType() == GRAPHIC_BITMAP )
        {

            // Resolution is set
            if( nMode == 1 )
            {
                Bitmap      aBitmap( rGraphic.GetBitmap() );
                MapMode     aMap( MAP_100TH_INCH );

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
               aGraphic.SetPrefMapMode( MapMode( MAP_100TH_MM ) );
               aGraphic.SetPrefSize( Size( nLogicalWidth, nLogicalHeight ) );
            }
            else
                aGraphic = rGraphic;

            sal_Int32 nColors = rConfigItem.ReadInt32( "Color", 0 ); // #92767#
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
                css::awt::Size aDefaultSize( 10000, 10000 );
                Size aNewSize( OutputDevice::LogicToLogic( Size( nLogicalWidth, nLogicalHeight ), MAP_100TH_MM, aMtf.GetPrefMapMode() ) );

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

    delete pResMgr;

    return aGraphic;
}

#endif

static OUString ImpCreateFullFilterPath( const OUString& rPath, const OUString& rFilterName )
{
    OUString aPathURL;

    ::osl::FileBase::getFileURLFromSystemPath( rPath, aPathURL );
    aPathURL += OUString( '/' );

    OUString aSystemPath;
    ::osl::FileBase::getSystemPathFromFileURL( aPathURL, aSystemPath );
    aSystemPath += OUString( rFilterName );

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
    PFilterCall             mpfnImport;
    PFilterDlgCall          mpfnImportDlg;

                            ImpFilterLibCacheEntry( const OUString& rPathname, const OUString& rFiltername );
    int                     operator==( const OUString& rFiltername ) const { return maFiltername == rFiltername; }

    PFilterCall             GetImportFunction();
};

ImpFilterLibCacheEntry::ImpFilterLibCacheEntry( const OUString& rPathname, const OUString& rFiltername ) :
        mpNext          ( NULL ),
#ifndef DISABLE_DYNLOADING
        maLibrary       ( rPathname ),
#endif
        maFiltername    ( rFiltername ),
        mpfnImport      ( NULL ),
        mpfnImportDlg   ( NULL )
{
#ifdef DISABLE_DYNLOADING
    (void) rPathname;
#endif
}

#ifdef DISABLE_DYNLOADING

extern "C" sal_Bool icdGraphicImport( SvStream& rStream, Graphic& rGraphic, FilterConfigItem* pConfigItem, sal_Bool );
extern "C" sal_Bool idxGraphicImport( SvStream& rStream, Graphic& rGraphic, FilterConfigItem* pConfigItem, sal_Bool );
extern "C" sal_Bool imeGraphicImport( SvStream& rStream, Graphic& rGraphic, FilterConfigItem* pConfigItem, sal_Bool );
extern "C" sal_Bool ipbGraphicImport( SvStream& rStream, Graphic& rGraphic, FilterConfigItem* pConfigItem, sal_Bool );
extern "C" sal_Bool ipdGraphicImport( SvStream& rStream, Graphic& rGraphic, FilterConfigItem* pConfigItem, sal_Bool );
extern "C" sal_Bool ipsGraphicImport( SvStream& rStream, Graphic& rGraphic, FilterConfigItem* pConfigItem, sal_Bool );
extern "C" sal_Bool iptGraphicImport( SvStream& rStream, Graphic& rGraphic, FilterConfigItem* pConfigItem, sal_Bool );
extern "C" sal_Bool ipxGraphicImport( SvStream& rStream, Graphic& rGraphic, FilterConfigItem* pConfigItem, sal_Bool );
extern "C" sal_Bool iraGraphicImport( SvStream& rStream, Graphic& rGraphic, FilterConfigItem* pConfigItem, sal_Bool );
extern "C" sal_Bool itgGraphicImport( SvStream& rStream, Graphic& rGraphic, FilterConfigItem* pConfigItem, sal_Bool );
extern "C" sal_Bool itiGraphicImport( SvStream& rStream, Graphic& rGraphic, FilterConfigItem* pConfigItem, sal_Bool );

#endif

PFilterCall ImpFilterLibCacheEntry::GetImportFunction()
{
    if( !mpfnImport )
    {
#ifndef DISABLE_DYNLOADING
        mpfnImport = (PFilterCall) maLibrary.getFunctionSymbol(OUString(IMPORT_FUNCTION_NAME));
#else
        if( maFiltername.EqualsAscii( "icd" ) )
            mpfnImport = icdGraphicImport;
        else if( maFiltername.EqualsAscii( "idx" ) )
            mpfnImport = idxGraphicImport;
        else if( maFiltername.EqualsAscii( "ime" ) )
            mpfnImport = imeGraphicImport;
        else if( maFiltername.EqualsAscii( "ipb" ) )
            mpfnImport = ipbGraphicImport;
        else if( maFiltername.EqualsAscii( "ipd" ) )
            mpfnImport = ipdGraphicImport;
        else if( maFiltername.EqualsAscii( "ips" ) )
            mpfnImport = ipsGraphicImport;
        else if( maFiltername.EqualsAscii( "ipt" ) )
            mpfnImport = iptGraphicImport;
        else if( maFiltername.EqualsAscii( "ipx" ) )
            mpfnImport = ipxGraphicImport;
        else if( maFiltername.EqualsAscii( "ira" ) )
            mpfnImport = iraGraphicImport;
        else if( maFiltername.EqualsAscii( "itg" ) )
            mpfnImport = itgGraphicImport;
        else if( maFiltername.EqualsAscii( "iti" ) )
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

    ImpFilterLibCacheEntry* GetFilter( const OUString& rFilterPath, const OUString& rFiltername );
};

ImpFilterLibCache::ImpFilterLibCache() :
    mpFirst     ( NULL ),
    mpLast      ( NULL )
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

ImpFilterLibCacheEntry* ImpFilterLibCache::GetFilter( const OUString& rFilterPath, const OUString& rFilterName )
{
    ImpFilterLibCacheEntry* pEntry = mpFirst;

    while( pEntry )
    {
        if( *pEntry == rFilterName )
            break;
        else
            pEntry = pEntry->mpNext;
    }
    if( !pEntry )
    {
        OUString aPhysicalName( ImpCreateFullFilterPath( rFilterPath, rFilterName ) );
        pEntry = new ImpFilterLibCacheEntry( aPhysicalName, rFilterName );
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
            pEntry = NULL;
        }
#endif
    }
    return pEntry;
};

namespace { struct Cache : public rtl::Static<ImpFilterLibCache, Cache> {}; }

GraphicFilter::GraphicFilter( sal_Bool bConfig ) :
    bUseConfig        ( bConfig ),
    nExpGraphHint     ( 0 )
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
            delete pFilterHdlList, pFilterHdlList = NULL;
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
        OUString url("$BRAND_BASE_DIR/program");
        rtl::Bootstrap::expandMacros(url); //TODO: detect failure
        utl::LocalFileHelper::ConvertURLToPhysicalName(url, aFilterPath);
    }

    pErrorEx = new FilterErrorEx;
    bAbort = sal_False;
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

sal_uInt16 GraphicFilter::GetImportFormatNumberForMediaType( const OUString& rMediaType )
{
    return pConfig->GetImportFormatNumberForMediaType( rMediaType );
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

OUString GraphicFilter::GetImportFormatMediaType( sal_uInt16 nFormat )
{
    return pConfig->GetImportFormatMediaType( nFormat );
}

OUString GraphicFilter::GetImportFormatShortName( sal_uInt16 nFormat )
{
    return pConfig->GetImportFormatShortName( nFormat );
}

OUString GraphicFilter::GetImportOSFileType( sal_uInt16 )
{
    OUString aOSFileType;
    return aOSFileType;
}

OUString GraphicFilter::GetImportWildcard( sal_uInt16 nFormat, sal_Int32 nEntry )
{
    return pConfig->GetImportWildcard( nFormat, nEntry );
}

sal_Bool GraphicFilter::IsImportPixelFormat( sal_uInt16 nFormat )
{
    return pConfig->IsImportPixelFormat( nFormat );
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

OUString GraphicFilter::GetExportFormatTypeName( sal_uInt16 nFormat )
{
    return pConfig->GetExportFilterTypeName( nFormat );
}

OUString GraphicFilter::GetExportFormatMediaType( sal_uInt16 nFormat )
{
    return pConfig->GetExportFormatMediaType( nFormat );
}

OUString GraphicFilter::GetExportFormatShortName( sal_uInt16 nFormat )
{
    return pConfig->GetExportFormatShortName( nFormat );
}

OUString GraphicFilter::GetExportOSFileType( sal_uInt16 )
{
    OUString aOSFileType;
    return aOSFileType;
}

OUString GraphicFilter::GetExportWildcard( sal_uInt16 nFormat, sal_Int32 nEntry )
{
    return pConfig->GetExportWildcard( nFormat, nEntry );
}

sal_Bool GraphicFilter::IsExportPixelFormat( sal_uInt16 nFormat )
{
    return pConfig->IsExportPixelFormat( nFormat );
}

sal_uInt16 GraphicFilter::CanImportGraphic( const INetURLObject& rPath,
                                        sal_uInt16 nFormat, sal_uInt16* pDeterminedFormat )
{
    sal_uInt16  nRetValue = GRFILTER_FORMATERROR;
    DBG_ASSERT( rPath.GetProtocol() != INET_PROT_NOT_VALID, "GraphicFilter::CanImportGraphic() : ProtType == INET_PROT_NOT_VALID" );

    OUString    aMainUrl( rPath.GetMainURL( INetURLObject::NO_DECODE ) );
    SvStream*   pStream = ::utl::UcbStreamHelper::CreateStream( aMainUrl, STREAM_READ | STREAM_SHARE_DENYNONE );
    if ( pStream )
    {
        nRetValue = CanImportGraphic( aMainUrl, *pStream, nFormat, pDeterminedFormat );
        delete pStream;
    }
    return nRetValue;
}

sal_uInt16 GraphicFilter::CanImportGraphic( const OUString& rMainUrl, SvStream& rIStream,
                                        sal_uInt16 nFormat, sal_uInt16* pDeterminedFormat )
{
    sal_uLong nStreamPos = rIStream.Tell();
    sal_uInt16 nRes = ImpTestOrFindFormat( rMainUrl, rIStream, nFormat );

    rIStream.Seek(nStreamPos);

    if( nRes==GRFILTER_OK && pDeterminedFormat!=NULL )
        *pDeterminedFormat = nFormat;

    return (sal_uInt16) ImplSetError( nRes, &rIStream );
}

//SJ: TODO, we need to create a GraphicImporter component
sal_uInt16 GraphicFilter::ImportGraphic( Graphic& rGraphic, const INetURLObject& rPath,
                                     sal_uInt16 nFormat, sal_uInt16 * pDeterminedFormat, sal_uInt32 nImportFlags )
{
    sal_uInt16 nRetValue = GRFILTER_FORMATERROR;
    DBG_ASSERT( rPath.GetProtocol() != INET_PROT_NOT_VALID, "GraphicFilter::ImportGraphic() : ProtType == INET_PROT_NOT_VALID" );

    OUString    aMainUrl( rPath.GetMainURL( INetURLObject::NO_DECODE ) );
    SvStream*   pStream = ::utl::UcbStreamHelper::CreateStream( aMainUrl, STREAM_READ | STREAM_SHARE_DENYNONE );
    if ( pStream )
    {
        nRetValue = ImportGraphic( rGraphic, aMainUrl, *pStream, nFormat, pDeterminedFormat, nImportFlags );
        delete pStream;
    }
    return nRetValue;
}

sal_uInt16 GraphicFilter::ImportGraphic( Graphic& rGraphic, const OUString& rPath, SvStream& rIStream,
                                     sal_uInt16 nFormat, sal_uInt16* pDeterminedFormat, sal_uInt32 nImportFlags, WMF_EXTERNALHEADER *pExtHeader )
{
    return ImportGraphic( rGraphic, rPath, rIStream, nFormat, pDeterminedFormat, nImportFlags, NULL, pExtHeader );
}

//-------------------------------------------------------------------------

sal_uInt16 GraphicFilter::ImportGraphic( Graphic& rGraphic, const OUString& rPath, SvStream& rIStream,
                                     sal_uInt16 nFormat, sal_uInt16* pDeterminedFormat, sal_uInt32 nImportFlags,
                                     com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >* pFilterData,
                                     WMF_EXTERNALHEADER *pExtHeader )
{
    OUString                aFilterName;
    sal_uLong               nStmBegin;
    sal_uInt16              nStatus;
    GraphicReader*          pContext = rGraphic.GetContext();
    GfxLinkType             eLinkType = GFX_LINK_TYPE_NONE;
    bool                    bDummyContext = ( pContext == (GraphicReader*) 1 );
    const sal_Bool              bLinkSet = rGraphic.IsLink();
    FilterConfigItem*       pFilterConfigItem = NULL;

    Size                    aPreviewSizeHint( 0, 0 );
    sal_Bool                bAllowPartialStreamRead = sal_False;
    sal_Bool                bCreateNativeLink = sal_True;

    ResetLastError();

    if ( pFilterData )
    {
        sal_Int32 i;
        for ( i = 0; i < pFilterData->getLength(); i++ )
        {
            if ( (*pFilterData)[ i ].Name == "PreviewSizeHint" )
            {
                awt::Size aSize;
                if ( (*pFilterData)[ i ].Value >>= aSize )
                {
                    aPreviewSizeHint = Size( aSize.Width, aSize.Height );
                    if ( aSize.Width || aSize.Height )
                        nImportFlags |= GRFILTER_I_FLAGS_FOR_PREVIEW;
                    else
                        nImportFlags &=~GRFILTER_I_FLAGS_FOR_PREVIEW;
                }
            }
            else if ( (*pFilterData)[ i ].Name == "AllowPartialStreamRead" )
            {
                (*pFilterData)[ i ].Value >>= bAllowPartialStreamRead;
                if ( bAllowPartialStreamRead )
                    nImportFlags |= GRFILTER_I_FLAGS_ALLOW_PARTIAL_STREAMREAD;
                else
                    nImportFlags &=~GRFILTER_I_FLAGS_ALLOW_PARTIAL_STREAMREAD;
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
            rGraphic.SetContext( NULL );
            nStmBegin = 0;
        }
        else
            nStmBegin = rIStream.Tell();

        bAbort = sal_False;
        nStatus = ImpTestOrFindFormat( rPath, rIStream, nFormat );
        // if pending, return GRFILTER_OK in order to request more bytes
        if( rIStream.GetError() == ERRCODE_IO_PENDING )
        {
            rGraphic.SetContext( (GraphicReader*) 1 );
            rIStream.ResetError();
            rIStream.Seek( nStmBegin );
            return (sal_uInt16) ImplSetError( GRFILTER_OK );
        }

        rIStream.Seek( nStmBegin );

        if( ( nStatus != GRFILTER_OK ) || rIStream.GetError() )
            return (sal_uInt16) ImplSetError( ( nStatus != GRFILTER_OK ) ? nStatus : GRFILTER_OPENERROR, &rIStream );

        if( pDeterminedFormat )
            *pDeterminedFormat = nFormat;

        aFilterName = pConfig->GetImportFilterName( nFormat );
    }
    else
    {
        if( pContext && !bDummyContext )
            aFilterName = pContext->GetUpperFilterName();

        nStmBegin = 0;
        nStatus = GRFILTER_OK;
    }

    // read graphic
    if ( pConfig->IsImportInternalFilter( nFormat ) )
    {
        if( aFilterName.equalsIgnoreAsciiCase( IMP_GIF )  )
        {
            if( rGraphic.GetContext() == (GraphicReader*) 1 )
                rGraphic.SetContext( NULL );

            if( !ImportGIF( rIStream, rGraphic ) )
                nStatus = GRFILTER_FILTERERROR;
            else
                eLinkType = GFX_LINK_TYPE_NATIVE_GIF;
        }
        else if( aFilterName.equalsIgnoreAsciiCase( IMP_PNG ) )
        {
            if ( rGraphic.GetContext() == (GraphicReader*) 1 )
                rGraphic.SetContext( NULL );

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
                const std::vector< vcl::PNGReader::ChunkData >&    rChunkData = aPNGReader.GetChunks();
                std::vector< vcl::PNGReader::ChunkData >::const_iterator aIter( rChunkData.begin() );
                std::vector< vcl::PNGReader::ChunkData >::const_iterator aEnd ( rChunkData.end() );
                while( aIter != aEnd )
                {
                    // Microsoft Office is storing Animated GIFs in following chunk
                    if ( aIter->nType == PMGCHUNG_msOG )
                    {
                        sal_uInt32 nChunkSize = aIter->aData.size();
                        if ( nChunkSize > 11 )
                        {
                            const std::vector< sal_uInt8 >& rData = aIter->aData;
                            SvMemoryStream aIStrm( (void*)&rData[ 11 ], nChunkSize - 11, STREAM_READ );
                            ImportGIF( aIStrm, rGraphic );
                            eLinkType = GFX_LINK_TYPE_NATIVE_PNG;
                            break;
                        }
                    }
                    ++aIter;
                }
            }

            if ( eLinkType == GFX_LINK_TYPE_NONE )
            {
                BitmapEx aBmpEx( aPNGReader.Read( aPreviewSizeHint ) );
                if ( aBmpEx.IsEmpty() )
                    nStatus = GRFILTER_FILTERERROR;
                else
                {
                    rGraphic = aBmpEx;
                    eLinkType = GFX_LINK_TYPE_NATIVE_PNG;
                }
            }
        }
        else if( aFilterName.equalsIgnoreAsciiCase( IMP_JPEG ) )
        {
            if( rGraphic.GetContext() == (GraphicReader*) 1 )
                rGraphic.SetContext( NULL );

            // set LOGSIZE flag always, if not explicitly disabled
            // (see #90508 and #106763)
            if( 0 == ( nImportFlags & GRFILTER_I_FLAGS_DONT_SET_LOGSIZE_FOR_JPEG ) )
                nImportFlags |= GRFILTER_I_FLAGS_SET_LOGSIZE_FOR_JPEG;

            if( !ImportJPEG( rIStream, rGraphic, NULL, nImportFlags ) )
                nStatus = GRFILTER_FILTERERROR;
            else
                eLinkType = GFX_LINK_TYPE_NATIVE_JPG;
        }
        else if( aFilterName.equalsIgnoreAsciiCase( IMP_SVG ) )
        {
            if( rGraphic.GetContext() == (GraphicReader*) 1 )
                rGraphic.SetContext( NULL );

            const sal_uInt32 nStmPos(rIStream.Tell());
            const sal_uInt32 nStmLen(rIStream.Seek(STREAM_SEEK_TO_END) - nStmPos);
            bool bOkay(false);

            if(nStmLen)
            {
                SvgDataArray aNewData(new sal_uInt8[nStmLen]);

                rIStream.Seek(nStmPos);
                rIStream.Read(aNewData.get(), nStmLen);

                if(!rIStream.GetError())
                {
                    SvgDataPtr aSvgDataPtr(
                        new SvgData(
                            aNewData,
                            nStmLen,
                            rPath));

                    rGraphic = Graphic(aSvgDataPtr);
                    bOkay = true;
                }
            }

            if(bOkay)
            {
                eLinkType = GFX_LINK_TYPE_NATIVE_SVG;
            }
            else
            {
                nStatus = GRFILTER_FILTERERROR;
            }
        }
        else if( aFilterName.equalsIgnoreAsciiCase( IMP_XBM ) )
        {
            if( rGraphic.GetContext() == (GraphicReader*) 1 )
                rGraphic.SetContext( NULL );

            if( !ImportXBM( rIStream, rGraphic ) )
                nStatus = GRFILTER_FILTERERROR;
        }
        else if( aFilterName.equalsIgnoreAsciiCase( IMP_XPM ) )
        {
            if( rGraphic.GetContext() == (GraphicReader*) 1 )
                rGraphic.SetContext( NULL );

            if( !ImportXPM( rIStream, rGraphic ) )
                nStatus = GRFILTER_FILTERERROR;
        }
        else if( aFilterName.equalsIgnoreAsciiCase( IMP_BMP ) ||
                    aFilterName.equalsIgnoreAsciiCase( IMP_SVMETAFILE ) )
        {
            // SV internal filters for import bitmaps and MetaFiles
            rIStream >> rGraphic;
            if( rIStream.GetError() )
                nStatus = GRFILTER_FORMATERROR;
        }
        else if( aFilterName.equalsIgnoreAsciiCase( IMP_WMF ) ||
                aFilterName.equalsIgnoreAsciiCase( IMP_EMF ) )
        {
            GDIMetaFile aMtf;
            if( !ConvertWMFToGDIMetaFile( rIStream, aMtf, NULL, pExtHeader ) )
                nStatus = GRFILTER_FORMATERROR;
            else
            {
                rGraphic = aMtf;
                eLinkType = GFX_LINK_TYPE_NATIVE_WMF;
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
                        aTempStream >> rGraphic;

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
        else
            nStatus = GRFILTER_FILTERERROR;
    }
    else
    {
        ImpFilterLibCacheEntry* pFilter = NULL;

        // find first filter in filter paths
        sal_Int32 i, nTokenCount = getTokenCount(aFilterPath, ';');
        ImpFilterLibCache &rCache = Cache::get();
        for( i = 0; ( i < nTokenCount ) && ( pFilter == NULL ); i++ )
            pFilter = rCache.GetFilter( getToken(aFilterPath, i, ';'), aFilterName );
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
                    if ( ( pFilterConfigItem == NULL ) && aShortName == "PCD" )
                    {
                        OUString aFilterConfigPath( "Office.Common/Filter/Graphic/Import/PCD" );
                        pFilterConfigItem = new FilterConfigItem( aFilterConfigPath );
                    }
                }
                if( !(*pFunc)( rIStream, rGraphic, pFilterConfigItem, sal_False ) )
                    nStatus = GRFILTER_FORMATERROR;
                else
                {
                    // try to set link type if format matches
                    if( nFormat != GRFILTER_FORMAT_DONTKNOW )
                    {
                        if( aShortName.startsWith( TIF_SHORTNAME ) )
                            eLinkType = GFX_LINK_TYPE_NATIVE_TIF;
                        else if( aShortName.startsWith( MET_SHORTNAME ) )
                            eLinkType = GFX_LINK_TYPE_NATIVE_MET;
                        else if( aShortName.startsWith( PCT_SHORTNAME ) )
                            eLinkType = GFX_LINK_TYPE_NATIVE_PCT;
                    }
                }
            }
        }
    }

    if( nStatus == GRFILTER_OK && bCreateNativeLink && ( eLinkType != GFX_LINK_TYPE_NONE ) && !rGraphic.GetContext() && !bLinkSet )
    {
        const sal_uLong nStmEnd = rIStream.Tell();
        const sal_uLong nBufSize = nStmEnd - nStmBegin;

        if( nBufSize )
        {
            sal_uInt8*  pBuf=0;
            try
            {
                pBuf = new sal_uInt8[ nBufSize ];
            }
            catch (const std::bad_alloc&)
            {
                nStatus = GRFILTER_TOOBIG;
            }

            if( nStatus == GRFILTER_OK )
            {
                rIStream.Seek( nStmBegin );
                rIStream.Read( pBuf, nBufSize );
                rGraphic.SetLink( GfxLink( pBuf, nBufSize, eLinkType, sal_True ) );
            }
        }
    }

    // Set error code or try to set native buffer
    if( nStatus != GRFILTER_OK )
    {
        if( bAbort )
            nStatus = GRFILTER_ABORT;

        ImplSetError( nStatus, &rIStream );
        rIStream.Seek( nStmBegin );
        rGraphic.Clear();
    }

    delete pFilterConfigItem;
    return nStatus;
}

sal_uInt16 GraphicFilter::ExportGraphic( const Graphic& rGraphic, const INetURLObject& rPath,
    sal_uInt16 nFormat, const uno::Sequence< beans::PropertyValue >* pFilterData )
{
#ifdef DISABLE_EXPORT
    (void) rGraphic;
    (void) rPath;
    (void) nFormat;
    (void) pFilterData;

    return GRFILTER_FORMATERROR;
#else
    SAL_INFO( "vcl.filter", "GraphicFilter::ExportGraphic() (thb)" );
    sal_uInt16  nRetValue = GRFILTER_FORMATERROR;
    DBG_ASSERT( rPath.GetProtocol() != INET_PROT_NOT_VALID, "GraphicFilter::ExportGraphic() : ProtType == INET_PROT_NOT_VALID" );
    bool bAlreadyExists = DirEntryExists( rPath );

    OUString    aMainUrl( rPath.GetMainURL( INetURLObject::NO_DECODE ) );
    SvStream*   pStream = ::utl::UcbStreamHelper::CreateStream( aMainUrl, STREAM_WRITE | STREAM_TRUNC );
    if ( pStream )
    {
        nRetValue = ExportGraphic( rGraphic, aMainUrl, *pStream, nFormat, pFilterData );
        delete pStream;

        if( ( GRFILTER_OK != nRetValue ) && !bAlreadyExists )
            KillDirEntry( aMainUrl );
    }
    return nRetValue;
#endif
}

#ifdef DISABLE_DYNLOADING

#ifndef DISABLE_EXPORT

extern "C" sal_Bool egiGraphicExport( SvStream& rStream, Graphic& rGraphic, FilterConfigItem* pConfigItem, sal_Bool );
extern "C" sal_Bool emeGraphicExport( SvStream& rStream, Graphic& rGraphic, FilterConfigItem* pConfigItem, sal_Bool );
extern "C" sal_Bool epbGraphicExport( SvStream& rStream, Graphic& rGraphic, FilterConfigItem* pConfigItem, sal_Bool );
extern "C" sal_Bool epgGraphicExport( SvStream& rStream, Graphic& rGraphic, FilterConfigItem* pConfigItem, sal_Bool );
extern "C" sal_Bool eppGraphicExport( SvStream& rStream, Graphic& rGraphic, FilterConfigItem* pConfigItem, sal_Bool );
extern "C" sal_Bool epsGraphicExport( SvStream& rStream, Graphic& rGraphic, FilterConfigItem* pConfigItem, sal_Bool );
extern "C" sal_Bool eptGraphicExport( SvStream& rStream, Graphic& rGraphic, FilterConfigItem* pConfigItem, sal_Bool );
extern "C" sal_Bool eraGraphicExport( SvStream& rStream, Graphic& rGraphic, FilterConfigItem* pConfigItem, sal_Bool );
extern "C" sal_Bool etiGraphicExport( SvStream& rStream, Graphic& rGraphic, FilterConfigItem* pConfigItem, sal_Bool );
extern "C" sal_Bool expGraphicExport( SvStream& rStream, Graphic& rGraphic, FilterConfigItem* pConfigItem, sal_Bool );

#endif

#endif

sal_uInt16 GraphicFilter::ExportGraphic( const Graphic& rGraphic, const OUString& rPath,
    SvStream& rOStm, sal_uInt16 nFormat, const uno::Sequence< beans::PropertyValue >* pFilterData )
{
#ifdef DISABLE_EXPORT
    (void) rGraphic;
    (void) rPath;
    (void) rOStm;
    (void) nFormat;
    (void) pFilterData;

    return GRFILTER_FORMATERROR;
#else
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
            if ( pConfig->GetExportFormatExtension( i ).EqualsIgnoreCaseAscii( aExt ) )
            {
                nFormat=i;
                break;
            }
        }
    }
    if( nFormat >= nFormatCount )
        return (sal_uInt16) ImplSetError( GRFILTER_FORMATERROR );

    FilterConfigItem aConfigItem( (uno::Sequence< beans::PropertyValue >*)pFilterData );
    OUString aFilterName( pConfig->GetExportFilterName( nFormat ) );

    bAbort              = sal_False;
    sal_uInt16      nStatus = GRFILTER_OK;
    GraphicType eType;
    Graphic     aGraphic( rGraphic );

    aGraphic = ImpGetScaledGraphic( rGraphic, aConfigItem );
    eType = aGraphic.GetType();

    if( pConfig->IsExportPixelFormat( nFormat ) )
    {
        if( eType != GRAPHIC_BITMAP )
        {
            Size aSizePixel;
            sal_uLong nColorCount,nBitsPerPixel,nNeededMem,nMaxMem;
            VirtualDevice aVirDev;

            nMaxMem = 1024;
            nMaxMem *= 1024; // In Bytes

            // Calculate how big the image would normally be:
            aSizePixel=aVirDev.LogicToPixel(aGraphic.GetPrefSize(),aGraphic.GetPrefMapMode());

            // Calculate how much memory the image will take up
            nColorCount=aVirDev.GetColorCount();
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

            aVirDev.SetMapMode(MapMode(MAP_PIXEL));
            aVirDev.SetOutputSizePixel(aSizePixel);
            Graphic aGraphic2=aGraphic;
            aGraphic2.Draw(&aVirDev,Point(0,0),aSizePixel); // this changes the MapMode
            aVirDev.SetMapMode(MapMode(MAP_PIXEL));
            aGraphic=Graphic(aVirDev.GetBitmap(Point(0,0),aSizePixel));
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
                sal_Int32 nColorRes = aConfigItem.ReadInt32( "Colors", 0 );
                if ( nColorRes && ( nColorRes <= (sal_uInt16)BMP_CONVERSION_24BIT) )
                {
                    if( !aBmp.Convert( (BmpConversion) nColorRes ) )
                        aBmp = aGraphic.GetBitmap();
                }
                sal_Bool    bRleCoding = aConfigItem.ReadBool( "RLE_Coding", sal_True );
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
                if ( !ConvertGDIMetaFileToEMF( aGraphic.GetGDIMetaFile(), rOStm, &aConfigItem ) )
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
                            com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue > aAdditionalChunkSequence;
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
                                        com::sun::star::uno::Sequence< sal_Int8 > aByteSeq;
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
                const SvgDataPtr aSvgDataPtr(rGraphic.getSvgData());

                if(aSvgDataPtr.get() && aSvgDataPtr->getSvgDataArrayLength())
                {
                    rOStm.Write(aSvgDataPtr->getSvgDataArray().get(), aSvgDataPtr->getSvgDataArrayLength());

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
                            xml::sax::Writer::create( xContext ), uno::UNO_QUERY_THROW);
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
                                ( (GDIMetaFile&) aGraphic.GetGDIMetaFile() ).Write( aMemStm );

                                xActiveDataSource->setOutputStream( css::uno::Reference< css::io::XOutputStream >(
                                    xStmIf, css::uno::UNO_QUERY ) );
                                css::uno::Sequence< sal_Int8 > aMtfSeq( (sal_Int8*) aMemStm.GetData(), aMemStm.Tell() );
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
                OUString aPhysicalName( ImpCreateFullFilterPath( getToken(aFilterPath, i, ';'), aFilterName ) );
                osl::Module aLibrary( aPhysicalName );

                PFilterCall pFunc = (PFilterCall) aLibrary.getFunctionSymbol(OUString(EXPORT_FUNCTION_NAME));
                // Execute dialog in DLL
#else
                PFilterCall pFunc = NULL;
                if( aFilterName.EqualsAscii( "egi" ) )
                    pFunc = egiGraphicExport;
                else if( aFilterName.EqualsAscii( "eme" ) )
                    pFunc = emeGraphicExport;
                else if( aFilterName.EqualsAscii( "epb" ) )
                    pFunc = epbGraphicExport;
                else if( aFilterName.EqualsAscii( "epg" ) )
                    pFunc = epgGraphicExport;
                else if( aFilterName.EqualsAscii( "epp" ) )
                    pFunc = eppGraphicExport;
                else if( aFilterName.EqualsAscii( "eps" ) )
                    pFunc = epsGraphicExport;
                else if( aFilterName.EqualsAscii( "ept" ) )
                    pFunc = eptGraphicExport;
                else if( aFilterName.EqualsAscii( "era" ) )
                    pFunc = eraGraphicExport;
                else if( aFilterName.EqualsAscii( "eti" ) )
                    pFunc = etiGraphicExport;
                else if( aFilterName.EqualsAscii( "exp" ) )
                    pFunc = expGraphicExport;
#endif
                if( pFunc )
                {
                    if ( !(*pFunc)( rOStm, aGraphic, &aConfigItem, sal_False ) )
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
        if( bAbort )
            nStatus = GRFILTER_ABORT;

        ImplSetError( nStatus, &rOStm );
    }
    return nStatus;
#endif
}

const FilterErrorEx& GraphicFilter::GetLastError() const
{
    return *pErrorEx;
}

void GraphicFilter::ResetLastError()
{
    pErrorEx->nFilterError = pErrorEx->nStreamError = 0UL;
}

const Link GraphicFilter::GetFilterCallback() const
{
    const Link aLink( LINK( this, GraphicFilter, FilterCallback ) );
    return aLink;
}

IMPL_LINK( GraphicFilter, FilterCallback, ConvertData*, pData )
{
    long nRet = 0L;

    if( pData )
    {
        sal_uInt16      nFormat = GRFILTER_FORMAT_DONTKNOW;
        OString aShortName;
        switch( pData->mnFormat )
        {
            case( CVT_BMP ): aShortName = BMP_SHORTNAME; break;
            case( CVT_GIF ): aShortName = GIF_SHORTNAME; break;
            case( CVT_JPG ): aShortName = JPG_SHORTNAME; break;
            case( CVT_MET ): aShortName = MET_SHORTNAME; break;
            case( CVT_PCT ): aShortName = PCT_SHORTNAME; break;
            case( CVT_PNG ): aShortName = PNG_SHORTNAME; break;
            case( CVT_SVM ): aShortName = SVM_SHORTNAME; break;
            case( CVT_TIF ): aShortName = TIF_SHORTNAME; break;
            case( CVT_WMF ): aShortName = WMF_SHORTNAME; break;
            case( CVT_EMF ): aShortName = EMF_SHORTNAME; break;
            case( CVT_SVG ): aShortName = SVG_SHORTNAME; break;

            default:
            break;
        }
        if( GRAPHIC_NONE == pData->maGraphic.GetType() || pData->maGraphic.GetContext() ) // Import
        {
            // Import
            nFormat = GetImportFormatNumberForShortName( OStringToOUString( aShortName, RTL_TEXTENCODING_UTF8) );
            nRet = ImportGraphic( pData->maGraphic, OUString(), pData->mrStm, nFormat ) == 0;
        }
#ifndef DISABLE_EXPORT
        else if( !aShortName.isEmpty() )
        {
            // Export
            nFormat = GetExportFormatNumberForShortName( OStringToOUString(aShortName, RTL_TEXTENCODING_UTF8) );
            nRet = ExportGraphic( pData->maGraphic, OUString(), pData->mrStm, nFormat ) == 0;
        }
#endif
    }
    return nRet;
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

    SvStream* pStream = NULL;
    INetURLObject aURL( rPath );

    if ( aURL.HasError() || INET_PROT_NOT_VALID == aURL.GetProtocol() )
    {
        aURL.SetSmartProtocol( INET_PROT_FILE );
        aURL.SetSmartURL( rPath );
    }
    else if ( INET_PROT_FILE != aURL.GetProtocol() )
    {
        pStream = ::utl::UcbStreamHelper::CreateStream( rPath, STREAM_READ );
    }

    int nRes = GRFILTER_OK;
    if ( !pStream )
        nRes = pFilter->ImportGraphic( rGraphic, aURL, nFilter, pDeterminedFormat );
    else
        nRes = pFilter->ImportGraphic( rGraphic, rPath, *pStream, nFilter, pDeterminedFormat );

#ifdef DBG_UTIL
    if( nRes )
        DBG_WARNING2( "GrafikFehler [%d] - [%s]", nRes, rPath.getStr() );
#endif

    return nRes;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
