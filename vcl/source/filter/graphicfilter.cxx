/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <config_folders.h>

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
#include <vcl/settings.hxx>
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

#define PMGCHUNG_msOG       0x6d734f47      

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

#endif 



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
    
    sal_uInt16 oldNumberFormat = rStream.GetNumberFormatInt();
    sal_uInt32 nOffset; 
    for ( nOffset = 0; ( nOffset <= 512 ) && ( ( nStreamPos + nOffset + 14 ) <= nStreamLen ); nOffset += 512 )
    {
        short y1,x1,y2,x2;
        bool bdBoxOk = true;

        rStream.Seek( nStreamPos + nOffset);
        
        rStream.SeekRel(2);
        
        rStream.SetNumberFormatInt(NUMBERFORMAT_INT_BIGENDIAN);
        rStream.ReadInt16( y1 ).ReadInt16( x1 ).ReadInt16( y2 ).ReadInt16( x2 );
        rStream.SetNumberFormatInt(oldNumberFormat); 

        if (x1 > x2 || y1 > y2 || 
            (x1 == x2 && y1 == y2) || 
            x2-x1 > 2048 || y2-y1 > 2048 ) 
          bdBoxOk = false;

        
        rStream.Read( sBuf,3 );
        
        
        if ( sBuf[ 0 ] == 0x00 && sBuf[ 1 ] == 0x11 && sBuf[ 2 ] == 0x02)
            return true;
        
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
            pLockBytes->SetSynchronMode( true );

        rStream.Seek( STREAM_SEEK_TO_END );
        nStreamLen = rStream.Tell() - nStreamPos;
        rStream.Seek( nStreamPos );
    }
    if (!nStreamLen)
    {
        return false; 
    }
    else if (nStreamLen >= 256)
    {   
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

    
    
    for( i = 0, nFirstLong = 0L, nSecondLong = 0L; i < 4; i++ )
    {
        nFirstLong=(nFirstLong<<8)|(sal_uLong)sFirstBytes[i];
        nSecondLong=(nSecondLong<<8)|(sal_uLong)sFirstBytes[i+4];
    }

    
    
    bool bSomethingTested = false;

    
    
    
    
    
    
    
    
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
            rStream.ReadUInt16( nFieldSize ).ReadUChar( nMagic );
            for (i=0; i<3; i++) {
                if (nFieldSize<6) { bOK=false; break; }
                if (nStreamLen < rStream.Tell() + nFieldSize ) { bOK=false; break; }
                rStream.SeekRel(nFieldSize-3);
                rStream.ReadUInt16( nFieldSize ).ReadUChar( nMagic );
                if (nMagic!=0xd3) { bOK=false; break; }
            }
            rStream.SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
            if (bOK && !rStream.GetError()) {
                rFormatExtension = "MET";
                return true;
            }
        }
    }

    
    if( !bTest || rFormatExtension.startsWith( "BMP" )  )
    {
        sal_uInt8 nOffs;

        bSomethingTested=true;

        
        
        
        if ( sFirstBytes[0] == 0x42 && sFirstBytes[1] == 0x41 )
            nOffs = 14;
        else
            nOffs = 0;

        
        if ( sFirstBytes[0+nOffs]==0x42 && sFirstBytes[1+nOffs]==0x4d )
        {

            
            
            
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

    
    if( !bTest || rFormatExtension.startsWith( "TIF" ) )
    {
        bSomethingTested=true;
        if ( nFirstLong==0x49492a00 || nFirstLong==0x4d4d002a )
        {
            rFormatExtension = "TIF";
            return true;
        }
    }

    
    if( !bTest || rFormatExtension.startsWith( "GIF" ) )
    {
        bSomethingTested=true;
        if ( nFirstLong==0x47494638 && (sFirstBytes[4]==0x37 || sFirstBytes[4]==0x39) && sFirstBytes[5]==0x61 )
        {
            rFormatExtension = "GIF";
            return true;
        }
    }

    
    if( !bTest || rFormatExtension.startsWith( "PNG" ) )
    {
        bSomethingTested=true;
        if (nFirstLong==0x89504e47 && nSecondLong==0x0d0a1a0a)
        {
            rFormatExtension = "PNG";
            return true;
        }
    }

    
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

    
    if( !bTest || rFormatExtension.startsWith( "PCD" ) )
    {
        bSomethingTested = true;
        if( nStreamLen >= 2055 )
        {
            char sBuf[8];
            rStream.Seek( nStreamPos + 2048 );
            rStream.Read( sBuf, 7 );

            if( strncmp( sBuf, "PCD_IPI", 7 ) == 0 )
            {
                rFormatExtension = "PCD";
                return true;
            }
        }
    }

    
    if( !bTest || rFormatExtension.startsWith( "PSD" ) )
    {
        bSomethingTested = true;
        if ( ( nFirstLong == 0x38425053 ) && ( (nSecondLong >> 16 ) == 1 ) )
        {
            rFormatExtension = "PSD";
            return true;
        }
    }

    
    if( !bTest || rFormatExtension.startsWith( "EPS" ) )
    {
        bSomethingTested = true;
        if ( ( nFirstLong == 0xC5D0D3C6 ) || ( ImplSearchEntry( sFirstBytes, (sal_uInt8*)"%!PS-Adobe", 10, 10 ) &&
             ImplSearchEntry( &sFirstBytes[15], (sal_uInt8*)"EPS", 3, 3 ) ) )
        {
            rFormatExtension = "EPS";
            return true;
        }
    }

    
    if( !bTest || rFormatExtension.startsWith( "DXF" ) )
    {
        
        if( strncmp( (const char*) sFirstBytes, "AutoCAD Binary DXF", 18 ) == 0 )
        {
            rFormatExtension = "DXF";
            return true;
        }

        
        i=0;
        while (i<256 && sFirstBytes[i]<=32)
            ++i;

        if (i<256 && sFirstBytes[i]=='0')
        {
            ++i;

            
            
            bSomethingTested=true;

            while( i<256 && sFirstBytes[i]<=32 )
                ++i;

            if (i+7<256 && (strncmp((const char*)(sFirstBytes+i),"SECTION",7)==0))
            {
                rFormatExtension = "DXF";
                return true;
            }
        }

    }

    
    if( !bTest || rFormatExtension.startsWith( "PCT" ) )
    {
        bSomethingTested = true;
        if (isPCT(rStream, nStreamPos, nStreamLen))
        {
            rFormatExtension = "PCT";
            return true;
        }
    }

    
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

    
    if( !bTest || rFormatExtension.startsWith( "RAS" ) )
    {
        bSomethingTested=true;
        if( nFirstLong == 0x59a66a95 )
        {
            rFormatExtension = "RAS";
            return true;
        }
    }

    
    if( !bTest )
    {
        bSomethingTested = true;
        if( ImplSearchEntry( sFirstBytes, (sal_uInt8*)"/* XPM */", 256, 9 ) )
        {
            rFormatExtension = "XPM";
            return true;
        }
    }
    else if( rFormatExtension.startsWith( "XPM" ) )
    {
        bSomethingTested = true;
        return true;
    }

    
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
                rFormatExtension = "XBM";
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

    
    if( !bTest )
    {
        
        if( ImplSearchEntry( sFirstBytes, (sal_uInt8*)"<?xml", 256, 5 ) 
            && ImplSearchEntry( sFirstBytes, (sal_uInt8*)"version", 256, 7 )) 
        {
            bool bIsSvg(false);

            
            if( ImplSearchEntry( sFirstBytes, (sal_uInt8*)"DOCTYPE", 256, 7 ) 
                && ImplSearchEntry( sFirstBytes, (sal_uInt8*)"svg", 256, 3 )) 
            {
                bIsSvg = true;
            }

            
            if(!bIsSvg && ImplSearchEntry( sFirstBytes, (sal_uInt8*)"<svg", 256, 4 )) 
            {
                bIsSvg = true;
            }

            if(!bIsSvg)
            {
                
                
                
                
                const sal_uLong nSize((nStreamLen > 2048) ? 2048 : nStreamLen);
                sal_uInt8* pBuf = new sal_uInt8[nSize];

                rStream.Seek(nStreamPos);
                rStream.Read(pBuf, nSize);

                if(ImplSearchEntry(pBuf, (sal_uInt8*)"<svg", nSize, 4)) 
                {
                    bIsSvg = true;
                }

                delete[] pBuf;
            }

            if(bIsSvg)
            {
                rFormatExtension = "SVG";
                return true;
            }
        }
        else
        {
            
            
            bool bIsSvg(false);

            
            if(ImplSearchEntry( sFirstBytes, (sal_uInt8*)"<svg", 256, 4 )) 
            {
                bIsSvg = true;
            }

            if(!bIsSvg)
            {
                
                
                
                
                const sal_uLong nSize((nStreamLen > 2048) ? 2048 : nStreamLen);
                sal_uInt8* pBuf = new sal_uInt8[nSize];

                rStream.Seek(nStreamPos);
                rStream.Read(pBuf, nSize);

                if(ImplSearchEntry(pBuf, (sal_uInt8*)"<svg", nSize, 4)) 
                {
                    bIsSvg = true;
                }

                delete[] pBuf;
            }

            if(bIsSvg)
            {
                rFormatExtension = "SVG";
                return true;
            }
        }
    }
    else if( rFormatExtension.startsWith( "SVG" ) )
    {
        bSomethingTested = true;
        return true;
    }

    
    if( !bTest || rFormatExtension.startsWith( "TGA" ) )
    {
        bSomethingTested = true;

        
        if( rFormatExtension.startsWith( "TGA" ) )
            return true;
    }

    
    if( !bTest || rFormatExtension.startsWith( "SGV" ) )
    {
        bSomethingTested = true;

        
        if( rFormatExtension.startsWith( "SGV" ) )
            return true;
    }

    
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
            bSomethingTested=true;
            rFormatExtension = "MOV";
            return true;
        }
    }

    return bTest && !bSomethingTested;
}



sal_uInt16 GraphicFilter::ImpTestOrFindFormat( const OUString& rPath, SvStream& rStream, sal_uInt16& rFormat )
{
    
    if( rFormat == GRFILTER_FORMAT_DONTKNOW )
    {
        OUString aFormatExt;
        if( ImpPeekGraphicFormat( rStream, aFormatExt, false ) )
        {
            rFormat = pConfig->GetImportFormatNumberForExtension( aFormatExt );
            if( rFormat != GRFILTER_FORMAT_DONTKNOW )
                return GRFILTER_OK;
        }
        
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
            sal_Int32 nBase = 2;    
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

        if ( nMode == -1 )  
        {                   
            nMode = 0;      
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
            
            else if( nMode == 2 )
            {
               aGraphic = rGraphic;
               aGraphic.SetPrefMapMode( MapMode( MAP_100TH_MM ) );
               aGraphic.SetPrefSize( Size( nLogicalWidth, nLogicalHeight ) );
            }
            else
                aGraphic = rGraphic;

            sal_Int32 nColors = rConfigItem.ReadInt32( "Color", 0 ); 
            if ( nColors )  
            {
                BitmapEx aBmpEx( aGraphic.GetBitmapEx() );
                aBmpEx.Convert( (BmpConversion)nColors );   
                aGraphic = aBmpEx;                          
            }                                               
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
    PFilterCall             mpfnImport;
    PFilterDlgCall          mpfnImportDlg;

                            ImpFilterLibCacheEntry( const OUString& rPathname, const OUString& rFiltername );
    bool                    operator==( const OUString& rFiltername ) const { return maFiltername == rFiltername; }

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
        if( maFiltername.equalsAscii( "icd" ) )
            mpfnImport = icdGraphicImport;
        else if( maFiltername.equalsAscii( "idx" ) )
            mpfnImport = idxGraphicImport;
        else if( maFiltername.equalsAscii( "ime" ) )
            mpfnImport = imeGraphicImport;
        else if( maFiltername.equalsAscii( "ipb" ) )
            mpfnImport = ipbGraphicImport;
        else if( maFiltername.equalsAscii( "ipd" ) )
            mpfnImport = ipdGraphicImport;
        else if( maFiltername.equalsAscii( "ips" ) )
            mpfnImport = ipsGraphicImport;
        else if( maFiltername.equalsAscii( "ipt" ) )
            mpfnImport = iptGraphicImport;
        else if( maFiltername.equalsAscii( "ipx" ) )
            mpfnImport = ipxGraphicImport;
        else if( maFiltername.equalsAscii( "ira" ) )
            mpfnImport = iraGraphicImport;
        else if( maFiltername.equalsAscii( "itg" ) )
            mpfnImport = itgGraphicImport;
        else if( maFiltername.equalsAscii( "iti" ) )
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

GraphicFilter::GraphicFilter( bool bConfig ) :
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
        OUString url("$BRAND_BASE_DIR/" LIBO_LIB_FOLDER);
        rtl::Bootstrap::expandMacros(url); 
        utl::LocalFileHelper::ConvertURLToPhysicalName(url, aFilterPath);
    }

    pErrorEx = new FilterErrorEx;
    bAbort = false;
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

bool GraphicFilter::IsImportPixelFormat( sal_uInt16 nFormat )
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

bool GraphicFilter::IsExportPixelFormat( sal_uInt16 nFormat )
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
    const bool              bLinkSet = rGraphic.IsLink();
    FilterConfigItem*       pFilterConfigItem = NULL;

    Size                    aPreviewSizeHint( 0, 0 );
    bool                bAllowPartialStreamRead = false;
    bool                bCreateNativeLink = true;

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

        bAbort = false;
        nStatus = ImpTestOrFindFormat( rPath, rIStream, nFormat );
        
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

            
            if( aPreviewSizeHint.Width() || aPreviewSizeHint.Height() )
            {
                
                if( !bAllowPartialStreamRead )
                    aPNGReader.GetChunks();
            }
            else
            {
                
                const std::vector< vcl::PNGReader::ChunkData >&    rChunkData = aPNGReader.GetChunks();
                std::vector< vcl::PNGReader::ChunkData >::const_iterator aIter( rChunkData.begin() );
                std::vector< vcl::PNGReader::ChunkData >::const_iterator aEnd ( rChunkData.end() );
                while( aIter != aEnd )
                {
                    
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
            
            ReadGraphic( rIStream, rGraphic );
            if( rIStream.GetError() )
                nStatus = GRFILTER_FORMATERROR;
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
                eLinkType = GFX_LINK_TYPE_NATIVE_MOV;
            }
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
        else
            nStatus = GRFILTER_FILTERERROR;
    }
    else
    {
        ImpFilterLibCacheEntry* pFilter = NULL;

        
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
                rGraphic.SetLink( GfxLink( pBuf, nBufSize, eLinkType, true ) );
            }
        }
    }

    
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
            if ( pConfig->GetExportFormatExtension( i ).equalsIgnoreAsciiCase( aExt ) )
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

    bAbort              = false;
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
            nMaxMem *= 1024; 

            
            aSizePixel=aVirDev.LogicToPixel(aGraphic.GetPrefSize(),aGraphic.GetPrefMapMode());

            
            nColorCount=aVirDev.GetColorCount();
            if      (nColorCount<=2)     nBitsPerPixel=1;
            else if (nColorCount<=4)     nBitsPerPixel=2;
            else if (nColorCount<=16)    nBitsPerPixel=4;
            else if (nColorCount<=256)   nBitsPerPixel=8;
            else if (nColorCount<=65536) nBitsPerPixel=16;
            else                         nBitsPerPixel=24;
            nNeededMem=((sal_uLong)aSizePixel.Width()*(sal_uLong)aSizePixel.Height()*nBitsPerPixel+7)/8;

            
            if (nMaxMem<nNeededMem)
            {
                double fFak=sqrt(((double)nMaxMem)/((double)nNeededMem));
                aSizePixel.Width()=(sal_uLong)(((double)aSizePixel.Width())*fFak);
                aSizePixel.Height()=(sal_uLong)(((double)aSizePixel.Height())*fFak);
            }

            aVirDev.SetMapMode(MapMode(MAP_PIXEL));
            aVirDev.SetOutputSizePixel(aSizePixel);
            Graphic aGraphic2=aGraphic;
            aGraphic2.Draw(&aVirDev,Point(0,0),aSizePixel); 
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
                bool    bRleCoding = aConfigItem.ReadBool( "RLE_Coding", true );
                
                WriteDIB(aBmp, rOStm, bRleCoding, true);


                if( rOStm.GetError() )
                    nStatus = GRFILTER_IOERROR;
            }
            else if( aFilterName.equalsIgnoreAsciiCase( EXP_SVMETAFILE ) )
            {
                sal_Int32 nVersion = aConfigItem.ReadInt32( "Version", 0 ) ;
                if ( nVersion )
                    rOStm.SetVersion( nVersion );

                
                GDIMetaFile aMTF(aGraphic.GetGDIMetaFile());

                aMTF.Write( rOStm );

                if( rOStm.GetError() )
                    nStatus = GRFILTER_IOERROR;
            }
            else if ( aFilterName.equalsIgnoreAsciiCase( EXP_WMF ) )
            {
                
                if ( !ConvertGDIMetaFileToWMF( aGraphic.GetGDIMetaFile(), rOStm, &aConfigItem ) )
                    nStatus = GRFILTER_FORMATERROR;

                if( rOStm.GetError() )
                    nStatus = GRFILTER_IOERROR;
            }
            else if ( aFilterName.equalsIgnoreAsciiCase( EXP_EMF ) )
            {
                
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
                
#else
                PFilterCall pFunc = NULL;
                if( aFilterName.equalsAscii( "egi" ) )
                    pFunc = egiGraphicExport;
                else if( aFilterName.equalsAscii( "eme" ) )
                    pFunc = emeGraphicExport;
                else if( aFilterName.equalsAscii( "epb" ) )
                    pFunc = epbGraphicExport;
                else if( aFilterName.equalsAscii( "epg" ) )
                    pFunc = epgGraphicExport;
                else if( aFilterName.equalsAscii( "epp" ) )
                    pFunc = eppGraphicExport;
                else if( aFilterName.equalsAscii( "eps" ) )
                    pFunc = epsGraphicExport;
                else if( aFilterName.equalsAscii( "ept" ) )
                    pFunc = eptGraphicExport;
                else if( aFilterName.equalsAscii( "era" ) )
                    pFunc = eraGraphicExport;
                else if( aFilterName.equalsAscii( "eti" ) )
                    pFunc = etiGraphicExport;
                else if( aFilterName.equalsAscii( "exp" ) )
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
    bool nRet = false;

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
        if( GRAPHIC_NONE == pData->maGraphic.GetType() || pData->maGraphic.GetContext() ) 
        {
            
            nFormat = GetImportFormatNumberForShortName( OStringToOUString( aShortName, RTL_TEXTENCODING_UTF8) );
            nRet = ImportGraphic( pData->maGraphic, OUString(), pData->mrStm, nFormat ) == 0;
        }
#ifndef DISABLE_EXPORT
        else if( !aShortName.isEmpty() )
        {
            
            nFormat = GetExportFormatNumberForShortName( OStringToOUString(aShortName, RTL_TEXTENCODING_UTF8) );
            nRet = ExportGraphic( pData->maGraphic, OUString(), pData->mrStm, nFormat ) == 0;
        }
#endif
    }
    return long(nRet);
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
        aURL.SetSmartProtocol( INET_PROT_FILE );
        aURL.SetSmartURL( rPath );
    }

    SvStream* pStream = NULL;
    if ( INET_PROT_FILE != aURL.GetProtocol() )
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
