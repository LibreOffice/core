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

// MARKER(update_precomp.py): autogen include statement, do not remove


#if defined UNX && defined ALPHA
#include <fstream.hxx>
#endif
#include <osl/mutex.hxx>
#include <comphelper/processfactory.hxx>
#include <ucbhelper/content.hxx>
#include <cppuhelper/implbase1.hxx>
#include <tools/urlobj.hxx>
#include <vcl/salctype.hxx>
#include <vcl/pngread.hxx>
#include <vcl/pngwrite.hxx>
#include <vcl/virdev.hxx>
#include <vcl/svapp.hxx>
#include <osl/file.hxx>
#include <bf_svtools/filter.hxx>
#include "FilterConfigCache.hxx"
#include <bf_svtools/FilterConfigItem.hxx>
#include <bf_svtools/fltcall.hxx>
#include <bf_svtools/wmf.hxx>
#include "gifread.hxx"
#include "jpeg.hxx"
#include "xbmread.hxx"
#include "xpmread.hxx"
#include <bf_svtools/solar.hrc>
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
#include <com/sun/star/ucb/CommandAbortedException.hpp>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/localfilehelper.hxx>
#include <comphelper/processfactory.hxx>
#include <rtl/bootstrap.hxx>
#include <rtl/instance.hxx>

#define PMGCHUNG_msOG		0x6d734f47		// Microsoft Office Animated GIF

#if defined WIN || (defined OS2 && !defined ICC)

#define IMPORT_FUNCTION_NAME	"_GraphicImport"
#define EXPORT_FUNCTION_NAME	"_GraphicExport"
#define IMPDLG_FUNCTION_NAME	"_DoImportDialog"
#define EXPDLG_FUNCTION_NAME	"_DoExportDialog"

#else

#define IMPORT_FUNCTION_NAME	"GraphicImport"
#define EXPORT_FUNCTION_NAME	"GraphicExport"
#define IMPDLG_FUNCTION_NAME	"DoImportDialog"
#define EXPDLG_FUNCTION_NAME	"DoExportDialog"

#endif

// Compilerfehler, wenn Optimierung bei WNT & MSC
#ifdef _MSC_VER
#pragma optimize( "", off )
#endif

// -----------
// - statics -
// -----------

using namespace ::rtl;
using namespace ::com::sun::star;

namespace binfilter
{

static List*		pFilterHdlList = NULL;

static ::osl::Mutex& getListMutex()
{
    static ::osl::Mutex	s_aListProtection;
    return s_aListProtection;
}

static GraphicFilter* pGraphicFilter=0;

// -------------------------
// - ImpFilterOutputStream -
// -------------------------

class ImpFilterOutputStream : public ::cppu::WeakImplHelper1< ::com::sun::star::io::XOutputStream >
{
protected:

    SvStream& 							mrStm;

    virtual void SAL_CALL				writeBytes( const ::com::sun::star::uno::Sequence< sal_Int8 >& rData ) throw (::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException) { mrStm.Write( rData.getConstArray(), rData.getLength() ); }
    virtual void SAL_CALL				flush() throw (::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException) { mrStm.Flush(); }
    virtual void SAL_CALL				closeOutput() throw() {}

public:

                                        ImpFilterOutputStream( SvStream& rStm ) : mrStm( rStm ) {}
                                        ~ImpFilterOutputStream() {}
};

BOOL ImplDirEntryHelper::Exists( const INetURLObject& rObj )
{
    BOOL bExists = FALSE;

    try
    {
        ::rtl::OUString	aTitle;
        ::ucbhelper::Content	aCnt( rObj.GetMainURL( INetURLObject::NO_DECODE ),
                              ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XCommandEnvironment >() );

        bExists = aCnt.isDocument();
    }
    catch( ::com::sun::star::ucb::CommandAbortedException& )
    {
        DBG_ERRORFILE( "CommandAbortedException" );
    }
    catch( ::com::sun::star::ucb::ContentCreationException& )
    {
        DBG_ERRORFILE( "ContentCreationException" );
    }
    catch( ... )
    {
//		DBG_ERRORFILE( "Any other exception" );
    }
    return bExists;
}

// -----------------------------------------------------------------------------

void ImplDirEntryHelper::Kill( const String& rMainUrl )
{
    try
    {
        ::ucbhelper::Content aCnt( rMainUrl,
                             ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XCommandEnvironment >() );

        aCnt.executeCommand( ::rtl::OUString::createFromAscii( "delete" ),
                             ::com::sun::star::uno::makeAny( sal_Bool( sal_True ) ) );
    }
    catch( ::com::sun::star::ucb::CommandAbortedException& )
    {
        DBG_ERRORFILE( "CommandAbortedException" );
    }
    catch( ... )
    {
        DBG_ERRORFILE( "Any other exception" );
    }
}

// --------------------
// - Helper functions -
// --------------------

//--------------------------------------------------------------------------

BYTE* ImplSearchEntry( BYTE* pSource, BYTE* pDest, ULONG nComp, ULONG nSize )
{
    while ( nComp-- >= nSize )
    {
        ULONG i;
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

//--------------------------------------------------------------------------

inline String ImpGetExtension( const String &rPath )
{
    String			aExt;
    INetURLObject	aURL( rPath );
    aExt = aURL.GetFileExtension().toAsciiUpperCase();
    return aExt;
}

/*************************************************************************
|*
|*    ImpPeekGraphicFormat()
|*
|*    Beschreibung:
|*        Diese Funktion kann zweierlei:
|*        1.) Datei anlesen, Dateiformat ermitteln
|*            Eingabe-prarameter:
|*              rPath            - Dateipfad
|*              rFormatExtension - Inhalt egal
|*              bTest            - setze FALSE
|*            Ausgabe-parameter:
|*              Funkionswert     - TRUE wenn Erfolg
|*              rFormatExtension - Bei Erfolg: uebliche Dateiendung
|*                                 des Formats (Grossbuchstaben)
|*        2.) Datei anlesen, Dateiformat ueberpruefen
|*            Eingabe-prarameter:
|*              rPath            - Dateipfad
|*              rFormatExtension - uebliche Dateiendung des Formats
|*                                 (Grossbuchstaben)
|*              bTest            - setze TRUE
|*            Ausgabe-parameter:
|*              Funkionswert     - FALSE, wenn die Datei bestimmt nicht
|*                                 vom uebgebenen Format ist.
|*                                 TRUE, wenn die Datei WAHRSCHEINLICH von
|*                                 dem Format ist, ODER WENN DAS FORMAT
|*                                 DIESER FUNKTION NICHT BEKANNT IST!
|*
|*    Ersterstellung    OH 26.05.95
|*    Letzte Aenderung  OH 07.08.95
|*
*************************************************************************/

static BOOL ImpPeekGraphicFormat( SvStream& rStream, String& rFormatExtension, BOOL bTest )
{
    USHORT  i;
    BYTE    sFirstBytes[ 256 ];
    ULONG   nFirstLong,nSecondLong;
    ULONG	nStreamPos = rStream.Tell();

    rStream.Seek( STREAM_SEEK_TO_END );
    ULONG nStreamLen = rStream.Tell() - nStreamPos;
    rStream.Seek( nStreamPos );

    if ( !nStreamLen )
    {
        SvLockBytes* pLockBytes = rStream.GetLockBytes();
        if ( pLockBytes  )
            pLockBytes->SetSynchronMode( TRUE );

        rStream.Seek( STREAM_SEEK_TO_END );
        nStreamLen = rStream.Tell() - nStreamPos;
        rStream.Seek( nStreamPos );
    }
    // Die ersten 256 Bytes in einen Buffer laden:
    if( nStreamLen >= 256 )
        rStream.Read( sFirstBytes, 256 );
    else
    {
        rStream.Read( sFirstBytes, nStreamLen );

        for( i = (USHORT) nStreamLen; i < 256; i++ )
            sFirstBytes[ i ]=0;
    }

    if( rStream.GetError() )
        return FALSE;

    // Die ersten 8 Bytes in nFirstLong, nSecondLong unterbringen,
    // Big-Endian:
    for( i = 0, nFirstLong = 0L, nSecondLong = 0L; i < 4; i++ )
    {
        nFirstLong=(nFirstLong<<8)|(ULONG)sFirstBytes[i];
        nSecondLong=(nSecondLong<<8)|(ULONG)sFirstBytes[i+4];
    }

    // Folgende Variable ist nur bei bTest==TRUE interessant. Sie
    // bleibt FALSE, wenn das Format (rFormatExtension) hier noch nicht
    // einprogrammiert wurde.
    BOOL bSomethingTested = FALSE;

    // Nun werden die verschieden Formate ueberprueft. Dabei ist die
    // Reihenfolge nicht egal. Z.b. koennte eine MET-Datei auch durch
    // den BMP-Test gehen, umgekehrt kann eine BMP-Datei kaum durch den
    // MET-Test gehen. Also sollte MET vor BMP getestet werden.
    // Theoretisch waere aber vielleicht auch eine BMP-Datei denkbar,
    // die durch den MET-Test geht.
    // Diese Probleme gibt es natuerlich nicht nur bei MET und BMP.
    // Deshalb wird im Falle der Uberpruefung eines Formats (bTest==TRUE)
    // nur genau dieses eine Format getestet. Alles andere koennte fatale
    // Folgen haben, z.B. wenn der Benutzer sagt, es sei BMP-Datei (und es
    // ist BMP-Datei), und hier wuerde die Datei durch den MET-Test gehen...

    //--------------------------- MET ------------------------------------
    if( !bTest || ( rFormatExtension.CompareToAscii( "MET", 3 ) == COMPARE_EQUAL ) )
    {
        bSomethingTested=TRUE;
        if( sFirstBytes[2] == 0xd3 )
        {
            rStream.SetNumberFormatInt( NUMBERFORMAT_INT_BIGENDIAN );
            rStream.Seek( nStreamPos );
            USHORT nFieldSize;
            BYTE nMagic;
            BOOL bOK=TRUE;
            rStream >> nFieldSize >> nMagic;
            for (i=0; i<3; i++) {
                if (nFieldSize<6) { bOK=FALSE; break; }
                if (nStreamLen < rStream.Tell() + nFieldSize ) { bOK=FALSE; break; }
                rStream.SeekRel(nFieldSize-3);
                rStream >> nFieldSize >> nMagic;
                if (nMagic!=0xd3) { bOK=FALSE; break; }
            }
            rStream.SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
            if (bOK && !rStream.GetError()) {
                rFormatExtension= UniString::CreateFromAscii( "MET", 3 );
                return TRUE;
            }
        }
    }

    //--------------------------- BMP ------------------------------------
    if( !bTest || ( rFormatExtension.CompareToAscii( "BMP", 3 ) == COMPARE_EQUAL ) )
    {
        BYTE nOffs;

        bSomethingTested=TRUE;

        // OS/2-Bitmaparray ('BA') koennen wir evtl. auch lesen,
        // dementspr. muessen wir den Offset anpassen,
        // um auf die erste Bitmap im Array zu stossen
        if ( sFirstBytes[0] == 0x42 && sFirstBytes[1] == 0x41 )
            nOffs = 14;
        else
            nOffs = 0;

        // Jetzt testen wir zunaechst auf 'BM'
        if ( sFirstBytes[0+nOffs]==0x42 && sFirstBytes[1+nOffs]==0x4d )
        {
            // unter OS/2 koennen die Reserved-Flags != 0 sein
            // (was sie eigentlich nicht duerften);
            // in diesem Fall testen wir die Groesse des BmpInfoHeaders
            if ( ( sFirstBytes[6+nOffs]==0x00 &&
                   sFirstBytes[7+nOffs]==0x00 &&
                   sFirstBytes[8+nOffs]==0x00 &&
                   sFirstBytes[9+nOffs]==0x00 ) ||
                   sFirstBytes[14+nOffs] == 0x28 ||
                   sFirstBytes[14+nOffs] == 0x0c )
            {
                rFormatExtension = UniString::CreateFromAscii( "BMP", 3 );
                return TRUE;
            }
        }
    }

    //--------------------------- WMF/EMF ------------------------------------

    if( !bTest ||
        ( rFormatExtension.CompareToAscii( "WMF", 3 ) == COMPARE_EQUAL ) ||
            ( rFormatExtension.CompareToAscii( "EMF", 3 ) == COMPARE_EQUAL ) )
    {
        bSomethingTested = TRUE;

        if ( nFirstLong==0xd7cdc69a || nFirstLong==0x01000900 )
        {
            rFormatExtension = UniString::CreateFromAscii( "WMF", 3 );
            return TRUE;
        }
        else if( nFirstLong == 0x01000000 && sFirstBytes[ 40 ] == 0x20 && sFirstBytes[ 41 ] == 0x45 &&
            sFirstBytes[ 42 ] == 0x4d && sFirstBytes[ 43 ] == 0x46 )
        {
            rFormatExtension = UniString::CreateFromAscii( "EMF", 3 );
            return TRUE;
        }
    }

    //--------------------------- PCX ------------------------------------
    if( !bTest || ( rFormatExtension.CompareToAscii( "PCX", 3 ) == COMPARE_EQUAL ) )
    {
        bSomethingTested=TRUE;
        if (sFirstBytes[0]==0x0a)
        {
            BYTE nVersion=sFirstBytes[1];
            BYTE nEncoding=sFirstBytes[2];
            if( ( nVersion==0 || nVersion==2 || nVersion==3 || nVersion==5 ) && nEncoding<=1 )
            {
                rFormatExtension = UniString::CreateFromAscii( "PCX", 3 );
                return TRUE;
            }
        }
    }

    //--------------------------- TIF ------------------------------------
    if( !bTest || ( rFormatExtension.CompareToAscii( "TIF", 3 ) == COMPARE_EQUAL ) )
    {
        bSomethingTested=TRUE;
        if ( nFirstLong==0x49492a00 || nFirstLong==0x4d4d002a )
        {
            rFormatExtension=UniString::CreateFromAscii( "TIF", 3 );
            return TRUE;
        }
    }

    //--------------------------- GIF ------------------------------------
    if( !bTest || ( rFormatExtension.CompareToAscii( "GIF", 3 ) == COMPARE_EQUAL ) )
    {
        bSomethingTested=TRUE;
        if ( nFirstLong==0x47494638 && (sFirstBytes[4]==0x37 || sFirstBytes[4]==0x39) && sFirstBytes[5]==0x61 )
        {
            rFormatExtension = UniString::CreateFromAscii( "GIF", 3 );
            return TRUE;
        }
    }

    //--------------------------- PNG ------------------------------------
    if( !bTest || ( rFormatExtension.CompareToAscii( "PNG", 3 ) == COMPARE_EQUAL ) )
    {
        bSomethingTested=TRUE;
        if (nFirstLong==0x89504e47 && nSecondLong==0x0d0a1a0a)
        {
            rFormatExtension = UniString::CreateFromAscii( "PNG", 3 );
            return TRUE;
        }
    }

    //--------------------------- JPG ------------------------------------
    if( !bTest || ( rFormatExtension.CompareToAscii( "JPG", 3 ) == COMPARE_EQUAL ) )
    {
        bSomethingTested=TRUE;
        if ( ( nFirstLong==0xffd8ffe0 && sFirstBytes[6]==0x4a && sFirstBytes[7]==0x46 && sFirstBytes[8]==0x49 && sFirstBytes[9]==0x46 ) ||
             ( nFirstLong==0xffd8fffe ) || ( 0xffd8ff00 == ( nFirstLong & 0xffffff00 ) ) )
        {
            rFormatExtension = UniString::CreateFromAscii( "JPG", 3 );
            return TRUE;
        }
    }

    //--------------------------- SVM ------------------------------------
    if( !bTest || ( rFormatExtension.CompareToAscii( "SVM", 3 ) == COMPARE_EQUAL ) )
    {
        bSomethingTested=TRUE;
        if( nFirstLong==0x53564744 && sFirstBytes[4]==0x49 )
        {
            rFormatExtension = UniString::CreateFromAscii( "SVM", 3 );
            return TRUE;
        }
        else if( sFirstBytes[0]==0x56 && sFirstBytes[1]==0x43 && sFirstBytes[2]==0x4C &&
                 sFirstBytes[3]==0x4D && sFirstBytes[4]==0x54 && sFirstBytes[5]==0x46 )
        {
            rFormatExtension = UniString::CreateFromAscii( "SVM", 3 );
            return TRUE;
        }
    }

    //--------------------------- PCD ------------------------------------
    if( !bTest || ( rFormatExtension.CompareToAscii( "PCD", 3 ) == COMPARE_EQUAL ) )
    {
        bSomethingTested = TRUE;
        if( nStreamLen >= 2055 )
        {
            char sBuf[8];
            rStream.Seek( nStreamPos + 2048 );
            rStream.Read( sBuf, 7 );

            if( strncmp( sBuf, "PCD_IPI", 7 ) ==  0 )
            {
                rFormatExtension = UniString::CreateFromAscii( "PCD", 3 );
                return TRUE;
            }
        }
    }

    //--------------------------- PSD ------------------------------------
    if( !bTest || ( rFormatExtension.CompareToAscii( "PSD", 3 ) == COMPARE_EQUAL ) )
    {
        bSomethingTested = TRUE;
        if ( ( nFirstLong == 0x38425053 ) && ( (nSecondLong >> 16 ) == 1 ) )
        {
            rFormatExtension = UniString::CreateFromAscii( "PSD", 3 );
            return TRUE;
        }
    }

    //--------------------------- EPS ------------------------------------
    if( !bTest || ( rFormatExtension.CompareToAscii( "EPS", 3 ) == COMPARE_EQUAL ) )
    {
        bSomethingTested = TRUE;
        if ( ( nFirstLong == 0xC5D0D3C6 ) || ( ImplSearchEntry( sFirstBytes, (BYTE*)"%!PS-Adobe", 10, 10 ) &&
             ImplSearchEntry( &sFirstBytes[15], (BYTE*)"EPS", 3, 3 ) ) )
        {
            rFormatExtension = UniString::CreateFromAscii( "EPS", 3 );
            return TRUE;
        }
    }

    //--------------------------- DXF ------------------------------------
    if( !bTest || ( rFormatExtension.CompareToAscii( "DXF", 3 ) == COMPARE_EQUAL ) )
    {
        bSomethingTested=TRUE;

        i=0;
        while (i<256 && sFirstBytes[i]<=32)
            i++;

        if (i<256)
        {
            if( sFirstBytes[i]=='0' )
                i++;
            else
                i=256;
        }
        while( i<256 && sFirstBytes[i]<=32 )
            i++;

        if (i+7<256)
        {
            if (strncmp((char*)(sFirstBytes+i),"SECTION",7)==0)
            {
                rFormatExtension = UniString::CreateFromAscii( "DXF", 3 );
                return TRUE;
            }
        }

        if( strncmp( (char*) sFirstBytes, "AutoCAD Binary DXF", 18 ) == 0 )
        {
            rFormatExtension = UniString::CreateFromAscii( "DXF", 3 );
            return TRUE;
        }
    }

    //--------------------------- PCT ------------------------------------
    if( !bTest || ( rFormatExtension.CompareToAscii( "PCT", 3 ) == COMPARE_EQUAL ) )
    {
        bSomethingTested = TRUE;
        BYTE sBuf[4];
        sal_uInt32 nOffset;	// in ms documents the pict format is used without the first 512 bytes
        for ( nOffset = 10; ( nOffset <= 522 ) && ( ( nStreamPos + nOffset + 3 ) <= nStreamLen ); nOffset += 512 )
        {
            rStream.Seek( nStreamPos + nOffset );
            rStream.Read( sBuf,3 );
            if ( sBuf[ 0 ] == 0x00 && sBuf[ 1 ] == 0x11 && ( sBuf[ 2 ] == 0x01 || sBuf[ 2 ] == 0x02 ) )
            {
                rFormatExtension = UniString::CreateFromAscii( "PCT", 3 );
                return TRUE;
            }
        }
    }

    //------------------------- PBM + PGM + PPM ---------------------------
    if( !bTest ||
        ( rFormatExtension.CompareToAscii( "PBM", 3 ) == COMPARE_EQUAL ) ||
            ( rFormatExtension.CompareToAscii( "PGM", 3 ) == COMPARE_EQUAL ) ||
                ( rFormatExtension.CompareToAscii( "PPM", 3 ) == COMPARE_EQUAL ) )
    {
        bSomethingTested=TRUE;
        if ( sFirstBytes[ 0 ] == 'P' )
        {
            switch( sFirstBytes[ 1 ] )
            {
                case '1' :
                case '4' :
                    rFormatExtension = UniString::CreateFromAscii( "PBM", 3 );
                return TRUE;

                case '2' :
                case '5' :
                    rFormatExtension = UniString::CreateFromAscii( "PGM", 3 );
                return TRUE;

                case '3' :
                case '6' :
                    rFormatExtension = UniString::CreateFromAscii( "PPM", 3 );
                return TRUE;
            }
        }
    }

    //--------------------------- RAS( SUN RasterFile )------------------
    if( !bTest || ( rFormatExtension.CompareToAscii( "RAS", 3 ) == COMPARE_EQUAL ) )
    {
        bSomethingTested=TRUE;
        if( nFirstLong == 0x59a66a95 )
        {
            rFormatExtension = UniString::CreateFromAscii( "RAS", 3 );
            return TRUE;
        }
    }

    //--------------------------- XPM ------------------------------------
    if( !bTest )
    {
        bSomethingTested = TRUE;
        if( ImplSearchEntry( sFirstBytes, (BYTE*)"/* XPM */", 256, 9 ) )
        {
            rFormatExtension = UniString::CreateFromAscii( "XPM", 3 );
            return TRUE;
        }
    }
    else if( rFormatExtension.CompareToAscii( "XPM", 3 ) == COMPARE_EQUAL )
    {
        bSomethingTested = TRUE;
        return TRUE;
    }

    //--------------------------- XBM ------------------------------------
    if( !bTest )
    {
        ULONG nSize = ( nStreamLen > 2048 ) ? 2048 : nStreamLen;
        BYTE* pBuf = new BYTE [ nSize ];

        rStream.Seek( nStreamPos );
        rStream.Read( pBuf, nSize );
        BYTE* pPtr = ImplSearchEntry( pBuf, (BYTE*)"#define", nSize, 7 );

        if( pPtr )
        {
            if( ImplSearchEntry( pPtr, (BYTE*)"_width", pBuf + nSize - pPtr, 6 ) )
            {
                rFormatExtension = UniString::CreateFromAscii( "XBM", 3 );
                delete[] pBuf;
                return TRUE;
            }
        }
        delete[] pBuf;
    }
    else if( rFormatExtension.CompareToAscii( "XBM", 3 ) == COMPARE_EQUAL )
    {
        bSomethingTested = TRUE;
        return TRUE;
    }

    //--------------------------- TGA ------------------------------------
    if( !bTest || ( rFormatExtension.CompareToAscii( "TGA", 3 ) == COMPARE_EQUAL ) )
    {
        bSomethingTested = TRUE;
        if( rFormatExtension.CompareToAscii( "TGA", 3 ) == COMPARE_EQUAL )
            return TRUE;
    }

    //--------------------------- SGV ------------------------------------
    if( !bTest || ( rFormatExtension.CompareToAscii( "SGV", 3 ) == COMPARE_EQUAL ) )
    {
        bSomethingTested = TRUE;
        if( rFormatExtension.CompareToAscii( "SGV", 3 ) == COMPARE_EQUAL )
            return TRUE;
    }

    //--------------------------- SGF ------------------------------------
    if( !bTest || ( rFormatExtension.CompareToAscii( "SGF", 3 ) == COMPARE_EQUAL ) )
    {
        bSomethingTested=TRUE;
        if( sFirstBytes[ 0 ] == 'J' && sFirstBytes[ 1 ] == 'J' )
        {
            rFormatExtension = UniString::CreateFromAscii( "SGF", 3 );
            return TRUE;
        }
    }

    return bTest && !bSomethingTested;
}

//--------------------------------------------------------------------------

sal_uInt16 GraphicFilter::ImpTestOrFindFormat( const String& rPath, SvStream& rStream, sal_uInt16& rFormat )
{
    sal_uInt16 n = pConfig->GetImportFormatCount();

    // ggf. Filter bzw. Format durch anlesen ermitteln,
    // oder durch anlesen zusichern, dass das Format stimmt:
    if( rFormat == GRFILTER_FORMAT_DONTKNOW )
    {
        String aFormatExt;
        if( ImpPeekGraphicFormat( rStream, aFormatExt, FALSE ) )
        {
            for( sal_uInt16 i = 0; i < n; i++ )
            {
                if( pConfig->GetImportFormatExtension( i ).EqualsIgnoreCaseAscii( aFormatExt ) )
                {
                    rFormat = i;
                    return GRFILTER_OK;
                }
            }
        }
        // ggf. Filter anhand der Datei-Endung raussuchen:
        if( rPath.Len() )
        {
            String aExt( ImpGetExtension( rPath ) );
            for( sal_uInt16 i = 0; i < n; i++ )
            {
                if( pConfig->GetImportFormatExtension( i ).EqualsIgnoreCaseAscii( aExt ) )
                {
                    rFormat = i;
                    return GRFILTER_OK;
                }
            }
        }
        return GRFILTER_FORMATERROR;
    }
    else
    {
        String aTmpStr( pConfig->GetImportFormatExtension( rFormat ) );
        if( !ImpPeekGraphicFormat( rStream, aTmpStr, TRUE ) )
            return GRFILTER_FORMATERROR;
        if ( pConfig->GetImportFormatExtension( rFormat ).EqualsIgnoreCaseAscii( "pcd" ) )
        {
            sal_Int32 nBase = 2;    // default Base0
            if ( pConfig->GetImportFilterType( rFormat ).EqualsIgnoreCaseAscii( "pcd_Photo_CD_Base4" ) )
                nBase = 1;
            else if ( pConfig->GetImportFilterType( rFormat ).EqualsIgnoreCaseAscii( "pcd_Photo_CD_Base16" ) )
                nBase = 0;
            String aFilterConfigPath( RTL_CONSTASCII_USTRINGPARAM( "Office.Common/Filter/Graphic/Import/PCD" ) );
            FilterConfigItem aFilterConfigItem( aFilterConfigPath );
            aFilterConfigItem.WriteInt32( String( RTL_CONSTASCII_USTRINGPARAM( "Resolution" ) ), nBase );
        }
    }

    return GRFILTER_OK;
}

//--------------------------------------------------------------------------

static Graphic ImpGetScaledGraphic( const Graphic& rGraphic, FilterConfigItem& rConfigItem )
{
    Graphic		aGraphic;

    sal_Int32 nLogicalWidth = rConfigItem.ReadInt32( String( RTL_CONSTASCII_USTRINGPARAM( "LogicalWidth" ) ), 0 );
    sal_Int32 nLogicalHeight = rConfigItem.ReadInt32( String( RTL_CONSTASCII_USTRINGPARAM( "LogicalHeight" ) ), 0 );

    if ( rGraphic.GetType() != GRAPHIC_NONE )
    {
        sal_Int32 nMode = rConfigItem.ReadInt32( String( RTL_CONSTASCII_USTRINGPARAM( "ExportMode" ) ), -1 );

        if ( nMode == -1 )	// the property is not there, this is possible, if the graphic filter
        {					// is called via UnoGraphicExporter and not from a graphic export Dialog
            nMode = 0;		// then we are defaulting this mode to 0
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

            // Aufloesung wird eingestellt
            if( nMode == 1 )
            {
                Bitmap 		aBitmap( rGraphic.GetBitmap() );
                MapMode		aMap( MAP_100TH_INCH );

                sal_Int32   nDPI = rConfigItem.ReadInt32( String( RTL_CONSTASCII_USTRINGPARAM( "Resolution" ) ), 75 );
                Fraction	aFrac( 1, Min( Max( nDPI, sal_Int32( 75 ) ), sal_Int32( 600 ) ) );

                aMap.SetScaleX( aFrac );
                aMap.SetScaleY( aFrac );

                Size aOldSize = aBitmap.GetSizePixel();
                aBitmap.SetPrefMapMode( aMap );
                aBitmap.SetPrefSize( Size( aOldSize.Width() * 100,
                                           aOldSize.Height() * 100 ) );

                aGraphic = Graphic( aBitmap );
            }
            // Groesse wird eingestellt
            else if( nMode == 2 )
            {
                BitmapEx aBitmapEx( rGraphic.GetBitmapEx() );
                aBitmapEx.SetPrefMapMode( MapMode( MAP_100TH_MM ) );
                aBitmapEx.SetPrefSize( Size( nLogicalWidth, nLogicalHeight ) );
                aGraphic = Graphic( aBitmapEx );
            }
            else
                aGraphic = rGraphic;

            sal_Int32 nColors = rConfigItem.ReadInt32( String( RTL_CONSTASCII_USTRINGPARAM( "Color" ) ), 0 ); // #92767#
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
                GDIMetaFile	aMtf( rGraphic.GetGDIMetaFile() );
                ::com::sun::star::awt::Size aDefaultSize( 10000, 10000 );
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

    return aGraphic;
}

static String ImpCreateFullFilterPath( const String& rPath, const String& rFilterName )
{
    ::rtl::OUString	aPathURL;

    ::osl::FileBase::getFileURLFromSystemPath( rPath, aPathURL );
    aPathURL += String( '/' );

    ::rtl::OUString	aSystemPath;
    ::osl::FileBase::getSystemPathFromFileURL( aPathURL, aSystemPath );
    aSystemPath += ::rtl::OUString( rFilterName );

    return String( aSystemPath );
}


// --------------------------
// - ImpFilterLibCacheEntry -
// --------------------------

class ImpFilterLibCache;

struct ImpFilterLibCacheEntry
{
    ImpFilterLibCacheEntry*	mpNext;
    osl::Module			    maLibrary;
    String					maFiltername;
    PFilterCall				mpfnImport;
    PFilterDlgCall			mpfnImportDlg;

                            ImpFilterLibCacheEntry( const String& rPathname, const String& rFiltername );
    int						operator==( const String& rFiltername ) const { return maFiltername == rFiltername; }

    PFilterCall				GetImportFunction();
    PFilterDlgCall			GetImportDlgFunction();
    PFilterCall				GetExportFunction() { return (PFilterCall) maLibrary.getFunctionSymbol( UniString::CreateFromAscii( EXPORT_FUNCTION_NAME ) ); }
    PFilterDlgCall			GetExportDlgFunction() { return (PFilterDlgCall) maLibrary.getFunctionSymbol( UniString::CreateFromAscii( EXPDLG_FUNCTION_NAME ) ); }
};

// ------------------------------------------------------------------------

ImpFilterLibCacheEntry::ImpFilterLibCacheEntry( const String& rPathname, const String& rFiltername ) :
        mpNext			( NULL ),
        maLibrary		( rPathname ),
        maFiltername	( rFiltername ),
        mpfnImport		( NULL ),
        mpfnImportDlg	( NULL )
{
}

// ------------------------------------------------------------------------

PFilterCall ImpFilterLibCacheEntry::GetImportFunction()
{
    if( !mpfnImport )
        mpfnImport = (PFilterCall) maLibrary.getFunctionSymbol( UniString::CreateFromAscii( IMPORT_FUNCTION_NAME ) );

    return mpfnImport;
}

// ------------------------------------------------------------------------

PFilterDlgCall ImpFilterLibCacheEntry::GetImportDlgFunction()
{
    if( !mpfnImportDlg )
        mpfnImportDlg = (PFilterDlgCall)maLibrary.getFunctionSymbol( UniString::CreateFromAscii( IMPDLG_FUNCTION_NAME ) );

    return mpfnImportDlg;
}

// ---------------------
// - ImpFilterLibCache -
// ---------------------

class ImpFilterLibCache
{
    ImpFilterLibCacheEntry*	mpFirst;
    ImpFilterLibCacheEntry*	mpLast;

public:
                            ImpFilterLibCache();
                            ~ImpFilterLibCache();

    ImpFilterLibCacheEntry*	GetFilter( const String& rFilterPath, const String& rFiltername );
};

// ------------------------------------------------------------------------

ImpFilterLibCache::ImpFilterLibCache() :
    mpFirst		( NULL ),
    mpLast		( NULL )
{
}

// ------------------------------------------------------------------------

ImpFilterLibCache::~ImpFilterLibCache()
{
    ImpFilterLibCacheEntry*	pEntry = mpFirst;
    while( pEntry )
    {
        ImpFilterLibCacheEntry* pNext = pEntry->mpNext;
        delete pEntry;
        pEntry = pNext;
    }
}

// ------------------------------------------------------------------------

ImpFilterLibCacheEntry* ImpFilterLibCache::GetFilter( const String& rFilterPath, const String& rFilterName )
{
    ImpFilterLibCacheEntry*	pEntry = mpFirst;

    while( pEntry )
    {
        if( *pEntry == rFilterName )
            break;
        else
            pEntry = pEntry->mpNext;
    }
    if( !pEntry )
    {
        String aPhysicalName( ImpCreateFullFilterPath( rFilterPath, rFilterName ) );
        pEntry = new ImpFilterLibCacheEntry( aPhysicalName, rFilterName );

        if ( pEntry->maLibrary.is() )
        {
            if( !mpFirst )
                mpFirst = mpLast = pEntry;
            else
                mpLast = mpLast->mpNext = pEntry;
        }
        else
        {
            delete pEntry;
            pEntry = NULL;
        }
    }
    return pEntry;
};

// ------------------------------------------------------------------------

namespace { struct Cache : public rtl::Static<ImpFilterLibCache, Cache> {}; }

// -----------------
// - GraphicFilter -
// -----------------

GraphicFilter::GraphicFilter( sal_Bool bConfig ) :
    bUseConfig	( bConfig )
{
    ImplInit();
}

// ------------------------------------------------------------------------

GraphicFilter::~GraphicFilter()
{
    {
        ::osl::MutexGuard aGuard( getListMutex() );
        pFilterHdlList->Remove( (void*)this );
        if ( !pFilterHdlList->Count() )
        {
            delete pFilterHdlList, pFilterHdlList = NULL;
            delete pConfig;
        }
    }


    delete pErrorEx;
}

// ------------------------------------------------------------------------

void GraphicFilter::ImplInit()
{
    {
        ::osl::MutexGuard aGuard( getListMutex() );

        if ( !pFilterHdlList )
        {
            pFilterHdlList = new List;
            pConfig = new FilterConfigCache( bUseConfig );
        }
        else
            pConfig = ((GraphicFilter*)pFilterHdlList->First())->pConfig;

        pFilterHdlList->Insert( (void*)this );
    }

    if( bUseConfig )
    {
#if defined WNT
        rtl::OUString url(RTL_CONSTASCII_USTRINGPARAM("BRAND_BASE_DIR"));
#else
        rtl::OUString url(RTL_CONSTASCII_USTRINGPARAM("OOO_BASE_DIR"));
#endif
        rtl::Bootstrap::expandMacros(url); //TODO: detect failure
        utl::LocalFileHelper::ConvertURLToPhysicalName(url, aFilterPath);
    }

    pErrorEx = new FilterErrorEx;
    bAbort = sal_False;
}

// ------------------------------------------------------------------------

ULONG GraphicFilter::ImplSetError( ULONG nError, const SvStream* pStm )
{
    pErrorEx->nFilterError = nError;
    pErrorEx->nStreamError = pStm ? pStm->GetError() : ERRCODE_NONE;
    return nError;
}
// ------------------------------------------------------------------------

USHORT GraphicFilter::GetImportFormatCount()
{
    return pConfig->GetImportFormatCount();
}

// ------------------------------------------------------------------------

USHORT GraphicFilter::GetImportFormatNumber( const String& rFormatName )
{
    return pConfig->GetImportFormatNumber( rFormatName );
}

// ------------------------------------------------------------------------

USHORT GraphicFilter::GetImportFormatNumberForMediaType( const String& rMediaType )
{
    return pConfig->GetImportFormatNumberForMediaType( rMediaType );
}

// ------------------------------------------------------------------------

USHORT GraphicFilter::GetImportFormatNumberForShortName( const String& rShortName )
{
    return pConfig->GetImportFormatNumberForShortName( rShortName );
}

// ------------------------------------------------------------------------

sal_uInt16 GraphicFilter::GetImportFormatNumberForTypeName( const String& rType )
{
    return pConfig->GetImportFormatNumberForTypeName( rType );
}

// ------------------------------------------------------------------------

String GraphicFilter::GetImportFormatName( USHORT nFormat )
{
    return pConfig->GetImportFormatName( nFormat );
}

// ------------------------------------------------------------------------

String GraphicFilter::GetImportFormatTypeName( USHORT nFormat )
{
    return pConfig->GetImportFilterTypeName( nFormat );
}

// ------------------------------------------------------------------------

String GraphicFilter::GetImportFormatMediaType( USHORT nFormat )
{
    return pConfig->GetImportFormatMediaType( nFormat );
}

// ------------------------------------------------------------------------

String GraphicFilter::GetImportFormatShortName( USHORT nFormat )
{
    return pConfig->GetImportFormatShortName( nFormat );
}

// ------------------------------------------------------------------------

String GraphicFilter::GetImportWildcard( USHORT nFormat, sal_Int32 nEntry )
{
    return pConfig->GetImportWildcard( nFormat, nEntry );
}

// ------------------------------------------------------------------------

BOOL GraphicFilter::IsImportPixelFormat( USHORT nFormat )
{
    return pConfig->IsImportPixelFormat( nFormat );
}

// ------------------------------------------------------------------------

USHORT GraphicFilter::GetExportFormatCount()
{
    return pConfig->GetExportFormatCount();
}

// ------------------------------------------------------------------------

USHORT GraphicFilter::GetExportFormatNumber( const String& rFormatName )
{
    return pConfig->GetExportFormatNumber( rFormatName );
}

// ------------------------------------------------------------------------

USHORT GraphicFilter::GetExportFormatNumberForMediaType( const String& rMediaType )
{
    return pConfig->GetExportFormatNumberForMediaType( rMediaType );
}

// ------------------------------------------------------------------------

USHORT GraphicFilter::GetExportFormatNumberForShortName( const String& rShortName )
{
    return pConfig->GetExportFormatNumberForShortName( rShortName );
}

// ------------------------------------------------------------------------

sal_uInt16 GraphicFilter::GetExportFormatNumberForTypeName( const String& rType )
{
    return pConfig->GetExportFormatNumberForTypeName( rType );
}

// ------------------------------------------------------------------------

String GraphicFilter::GetExportFormatName( USHORT nFormat )
{
    return pConfig->GetExportFormatName( nFormat );
}

// ------------------------------------------------------------------------

String GraphicFilter::GetExportFormatTypeName( USHORT nFormat )
{
    return pConfig->GetExportFilterTypeName( nFormat );
}

// ------------------------------------------------------------------------

String GraphicFilter::GetExportFormatMediaType( USHORT nFormat )
{
    return pConfig->GetExportFormatMediaType( nFormat );
}

// ------------------------------------------------------------------------

String GraphicFilter::GetExportFormatShortName( USHORT nFormat )
{
    return pConfig->GetExportFormatShortName( nFormat );
}

// ------------------------------------------------------------------------

String GraphicFilter::GetExportWildcard( USHORT nFormat, sal_Int32 nEntry )
{
    return pConfig->GetExportWildcard( nFormat, nEntry );
}

// ------------------------------------------------------------------------

USHORT GraphicFilter::CanImportGraphic( const INetURLObject& rPath,
                                        USHORT nFormat, USHORT* pDeterminedFormat )
{
    sal_uInt16	nRetValue = GRFILTER_FORMATERROR;
    DBG_ASSERT( rPath.GetProtocol() != INET_PROT_NOT_VALID, "GraphicFilter::CanImportGraphic() : ProtType == INET_PROT_NOT_VALID" );

    String		aMainUrl( rPath.GetMainURL( INetURLObject::NO_DECODE ) );
    SvStream*	pStream = ::utl::UcbStreamHelper::CreateStream( aMainUrl, STREAM_READ | STREAM_SHARE_DENYNONE );
    if ( pStream )
    {
        nRetValue = CanImportGraphic( aMainUrl, *pStream, nFormat, pDeterminedFormat );
        delete pStream;
    }
    return nRetValue;
}

// ------------------------------------------------------------------------

USHORT GraphicFilter::CanImportGraphic( const String& rMainUrl, SvStream& rIStream,
                                        USHORT nFormat, USHORT* pDeterminedFormat )
{
    ULONG nStreamPos = rIStream.Tell();
    sal_uInt16 nRes = ImpTestOrFindFormat( rMainUrl, rIStream, nFormat );

    rIStream.Seek(nStreamPos);

    if( nRes==GRFILTER_OK && pDeterminedFormat!=NULL )
        *pDeterminedFormat = nFormat;

    return (USHORT) ImplSetError( nRes, &rIStream );
}

// ------------------------------------------------------------------------
//SJ: TODO, we need to create a GraphicImporter component
USHORT GraphicFilter::ImportGraphic( Graphic& rGraphic, const INetURLObject& rPath,
                                     USHORT nFormat, USHORT * pDeterminedFormat, sal_uInt32 nImportFlags )
{
    sal_uInt16 nRetValue = GRFILTER_FORMATERROR;
    DBG_ASSERT( rPath.GetProtocol() != INET_PROT_NOT_VALID, "GraphicFilter::ImportGraphic() : ProtType == INET_PROT_NOT_VALID" );

    String		aMainUrl( rPath.GetMainURL( INetURLObject::NO_DECODE ) );
    SvStream*	pStream = ::utl::UcbStreamHelper::CreateStream( aMainUrl, STREAM_READ | STREAM_SHARE_DENYNONE );
    if ( pStream )
    {
        nRetValue = ImportGraphic( rGraphic, aMainUrl, *pStream, nFormat, pDeterminedFormat, nImportFlags );
        delete pStream;
    }
    return nRetValue;
}

USHORT GraphicFilter::ImportGraphic( Graphic& rGraphic, const String& rPath, SvStream& rIStream,
                                     USHORT nFormat, USHORT* pDeterminedFormat, sal_uInt32 nImportFlags )
{
    return ImportGraphic( rGraphic, rPath, rIStream, nFormat, pDeterminedFormat, nImportFlags, NULL );
}

//-------------------------------------------------------------------------

USHORT GraphicFilter::ImportGraphic( Graphic& rGraphic, const String& rPath, SvStream& rIStream,
                                     USHORT nFormat, USHORT* pDeterminedFormat, sal_uInt32 nImportFlags,
                                     com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >* pFilterData )
{
    String					aFilterName;
    ULONG					nStmBegin;
    USHORT					nStatus;
    GraphicReader*			pContext = rGraphic.GetContext();
    GfxLinkType				eLinkType = GFX_LINK_TYPE_NONE;
    BOOL					bDummyContext = ( pContext == (GraphicReader*) 1 );
    const BOOL				bLinkSet = rGraphic.IsLink();
    FilterConfigItem*       pFilterConfigItem = NULL;

    Size					aPreviewSizeHint( 0, 0 );
    sal_Bool				bAllowPartialStreamRead = sal_False;
    sal_Bool				bCreateNativeLink = sal_True;

    ResetLastError();

    if ( pFilterData )
    {
        sal_Int32 i;
        for ( i = 0; i < pFilterData->getLength(); i++ )
        {
            if ( (*pFilterData)[ i ].Name.equalsAscii( "PreviewSizeHint" ) )
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
            else if ( (*pFilterData)[ i ].Name.equalsAscii( "AllowPartialStreamRead" ) )
            {
                (*pFilterData)[ i ].Value >>= bAllowPartialStreamRead;
                if ( bAllowPartialStreamRead )
                    nImportFlags |= GRFILTER_I_FLAGS_ALLOW_PARTIAL_STREAMREAD;
                else
                    nImportFlags &=~GRFILTER_I_FLAGS_ALLOW_PARTIAL_STREAMREAD;
            }
            else if ( (*pFilterData)[ i ].Name.equalsAscii( "CreateNativeLink" ) )
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

        bAbort = FALSE;
        nStatus = ImpTestOrFindFormat( rPath, rIStream, nFormat );
        // Falls Pending, geben wir GRFILTER_OK zurueck,
        // um mehr Bytes anzufordern
        if( rIStream.GetError() == ERRCODE_IO_PENDING )
        {
            rGraphic.SetContext( (GraphicReader*) 1 );
            rIStream.ResetError();
            rIStream.Seek( nStmBegin );
            return (USHORT) ImplSetError( GRFILTER_OK );
        }

        rIStream.Seek( nStmBegin );

        if( ( nStatus != GRFILTER_OK ) || rIStream.GetError() )
            return (USHORT) ImplSetError( ( nStatus != GRFILTER_OK ) ? nStatus : GRFILTER_OPENERROR, &rIStream );

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
        if( aFilterName.EqualsIgnoreCaseAscii( IMP_GIF )  )
        {
            if( rGraphic.GetContext() == (GraphicReader*) 1 )
                rGraphic.SetContext( NULL );

            if( !ImportGIF( rIStream, rGraphic ) )
                nStatus = GRFILTER_FILTERERROR;
            else
                eLinkType = GFX_LINK_TYPE_NATIVE_GIF;
        }
        else if( aFilterName.EqualsIgnoreCaseAscii( IMP_PNG ) )
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
                    aIter++;
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
        else if( aFilterName.EqualsIgnoreCaseAscii( IMP_JPEG ) )
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
        else if( aFilterName.EqualsIgnoreCaseAscii( IMP_XBM ) )
        {
            if( rGraphic.GetContext() == (GraphicReader*) 1 )
                rGraphic.SetContext( NULL );

            if( !ImportXBM( rIStream, rGraphic ) )
                nStatus = GRFILTER_FILTERERROR;
        }
        else if( aFilterName.EqualsIgnoreCaseAscii( IMP_XPM ) )
        {
            if( rGraphic.GetContext() == (GraphicReader*) 1 )
                rGraphic.SetContext( NULL );

            if( !ImportXPM( rIStream, rGraphic ) )
                nStatus = GRFILTER_FILTERERROR;
        }
        else if( aFilterName.EqualsIgnoreCaseAscii( IMP_BMP ) ||
                    aFilterName.EqualsIgnoreCaseAscii( IMP_SVMETAFILE ) )
        {
            // SV interne Importfilter fuer Bitmaps und MetaFiles
            rIStream >> rGraphic;
            if( rIStream.GetError() )
                nStatus = GRFILTER_FORMATERROR;
        }
        else if( aFilterName.EqualsIgnoreCaseAscii( IMP_WMF ) ||
                aFilterName.EqualsIgnoreCaseAscii( IMP_EMF ) )
        {
            GDIMetaFile aMtf;
            if( !ConvertWMFToGDIMetaFile( rIStream, aMtf, NULL ) )
                nStatus = GRFILTER_FORMATERROR;
            else
            {
                rGraphic = aMtf;
                eLinkType = GFX_LINK_TYPE_NATIVE_WMF;
            }
        }
        else if( aFilterName.EqualsIgnoreCaseAscii( IMP_SVSGF )
                || aFilterName.EqualsIgnoreCaseAscii( IMP_SVSGV ) )
        {
            USHORT			nVersion;
            unsigned char	nTyp = CheckSgfTyp( rIStream, nVersion );

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
        ImpFilterLibCacheEntry*	pFilter = NULL;

        // find first filter in filter pathes
        xub_StrLen i, nTokenCount = aFilterPath.GetTokenCount( ';' );
        ImpFilterLibCache &rCache = Cache::get();
        for( i = 0; ( i < nTokenCount ) && ( pFilter == NULL ); i++ )
            pFilter = rCache.GetFilter( aFilterPath.GetToken(i), aFilterName );
        if( !pFilter )
            nStatus = GRFILTER_FILTERERROR;
        else
        {
            PFilterCall pFunc = pFilter->GetImportFunction();

            if( !pFunc )
                nStatus = GRFILTER_FILTERERROR;
            else
            {
                String aShortName;
                if( nFormat != GRFILTER_FORMAT_DONTKNOW )
                {
                    aShortName = GetImportFormatShortName( nFormat ).ToUpperAscii();
                    if ( ( pFilterConfigItem == NULL ) && aShortName.EqualsAscii( "PCD" ) )
                    {
                        String aFilterConfigPath( RTL_CONSTASCII_USTRINGPARAM( "Office.Common/Filter/Graphic/Import/PCD" ) );
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
                        if( aShortName.CompareToAscii( TIF_SHORTNAME ) == COMPARE_EQUAL )
                            eLinkType = GFX_LINK_TYPE_NATIVE_TIF;
                        else if( aShortName.CompareToAscii( MET_SHORTNAME ) == COMPARE_EQUAL )
                            eLinkType = GFX_LINK_TYPE_NATIVE_MET;
                        else if( aShortName.CompareToAscii( PCT_SHORTNAME ) == COMPARE_EQUAL )
                            eLinkType = GFX_LINK_TYPE_NATIVE_PCT;
                    }
                }
            }
        }
    }

    if( nStatus == GRFILTER_OK && bCreateNativeLink && ( eLinkType != GFX_LINK_TYPE_NONE ) && !rGraphic.GetContext() && !bLinkSet )
    {
        const ULONG nStmEnd = rIStream.Tell();
        const ULONG	nBufSize = nStmEnd - nStmBegin;

        if( nBufSize )
        {
            BYTE*	pBuf=0;
            try
            {
                pBuf = new BYTE[ nBufSize ];
            }
                catch (std::bad_alloc)
            {
                nStatus = GRFILTER_TOOBIG;
            }

            if( nStatus == GRFILTER_OK )
            {
                rIStream.Seek( nStmBegin );
                rIStream.Read( pBuf, nBufSize );
                rGraphic.SetLink( GfxLink( pBuf, nBufSize, eLinkType, TRUE ) );
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


// ------------------------------------------------------------------------

USHORT GraphicFilter::ExportGraphic( const Graphic& rGraphic, const INetURLObject& rPath,
    sal_uInt16 nFormat, const uno::Sequence< beans::PropertyValue >* pFilterData )
{
    sal_uInt16	nRetValue = GRFILTER_FORMATERROR;
    DBG_ASSERT( rPath.GetProtocol() != INET_PROT_NOT_VALID, "GraphicFilter::ExportGraphic() : ProtType == INET_PROT_NOT_VALID" );
    BOOL		bAlreadyExists = ImplDirEntryHelper::Exists( rPath );

    String		aMainUrl( rPath.GetMainURL( INetURLObject::NO_DECODE ) );
    SvStream*	pStream = ::utl::UcbStreamHelper::CreateStream( aMainUrl, STREAM_WRITE | STREAM_TRUNC );
    if ( pStream )
    {
        nRetValue = ExportGraphic( rGraphic, aMainUrl, *pStream, nFormat, pFilterData );
        delete pStream;

        if( ( GRFILTER_OK != nRetValue ) && !bAlreadyExists )
            ImplDirEntryHelper::Kill( aMainUrl );
    }
    return nRetValue;
}

// ------------------------------------------------------------------------

USHORT GraphicFilter::ExportGraphic( const Graphic& rGraphic, const String& rPath,
    SvStream& rOStm, sal_uInt16 nFormat, const uno::Sequence< beans::PropertyValue >* pFilterData )
{
    USHORT nFormatCount = GetExportFormatCount();

    ResetLastError();

    if( nFormat == GRFILTER_FORMAT_DONTKNOW )
    {
        INetURLObject aURL( rPath );
        String aExt( aURL.GetFileExtension().toAsciiUpperCase() );


        for( USHORT i = 0; i < nFormatCount; i++ )
        {
            if ( pConfig->GetExportFormatExtension( nFormat ).EqualsIgnoreCaseAscii( aExt ) )
            {
                nFormat=i;
                break;
            }
        }
    }
    if( nFormat >= nFormatCount )
        return (USHORT) ImplSetError( GRFILTER_FORMATERROR );

    FilterConfigItem aConfigItem( (uno::Sequence< beans::PropertyValue >*)pFilterData );
    String aFilterName( pConfig->GetExportFilterName( nFormat ) );

    bAbort				= FALSE;
    USHORT		nStatus = GRFILTER_OK;
    GraphicType	eType;
    Graphic		aGraphic( rGraphic );

    aGraphic = ImpGetScaledGraphic( rGraphic, aConfigItem );
    eType = aGraphic.GetType();

    if( pConfig->IsExportPixelFormat( nFormat ) )
    {
        if( eType != GRAPHIC_BITMAP )
        {
            Size aSizePixel;
            ULONG nColorCount,nBitsPerPixel,nNeededMem,nMaxMem;
            VirtualDevice aVirDev;

            // Maximalen Speicherbedarf fuer das Bildes holen:
//			if( GetOptionsConfig() )
//				nMaxMem = (UINT32)GetOptionsConfig()->ReadKey( "VEC-TO-PIX-MAX-KB", "1024" ).ToInt32();
//			else
                nMaxMem = 1024;

            nMaxMem *= 1024; // In Bytes

            // Berechnen, wie gross das Bild normalerweise werden wuerde:
            aSizePixel=aVirDev.LogicToPixel(aGraphic.GetPrefSize(),aGraphic.GetPrefMapMode());

            // Berechnen, wieviel Speicher das Bild benoetigen wuerde:
            nColorCount=aVirDev.GetColorCount();
            if      (nColorCount<=2)     nBitsPerPixel=1;
            else if (nColorCount<=4)     nBitsPerPixel=2;
            else if (nColorCount<=16)    nBitsPerPixel=4;
            else if (nColorCount<=256)   nBitsPerPixel=8;
            else if (nColorCount<=65536) nBitsPerPixel=16;
            else                         nBitsPerPixel=24;
            nNeededMem=((ULONG)aSizePixel.Width()*(ULONG)aSizePixel.Height()*nBitsPerPixel+7)/8;

            // ggf. Groesse des Bildes einschraenken:
            if (nMaxMem<nNeededMem)
            {
                double fFak=sqrt(((double)nMaxMem)/((double)nNeededMem));
                aSizePixel.Width()=(ULONG)(((double)aSizePixel.Width())*fFak);
                aSizePixel.Height()=(ULONG)(((double)aSizePixel.Height())*fFak);
            }

            aVirDev.SetMapMode(MapMode(MAP_PIXEL));
            aVirDev.SetOutputSizePixel(aSizePixel);
            Graphic aGraphic2=aGraphic;
            aGraphic2.Draw(&aVirDev,Point(0,0),aSizePixel); // Gemein: dies aendert den MapMode
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
            if( aFilterName.EqualsIgnoreCaseAscii( EXP_BMP ) )
            {
                Bitmap aBmp( aGraphic.GetBitmap() );
                sal_Int32 nColorRes = aConfigItem.ReadInt32( String( RTL_CONSTASCII_USTRINGPARAM( "Colors" ) ), 0 );
                if ( nColorRes && ( nColorRes <= (USHORT)BMP_CONVERSION_24BIT) )
                {
                    if( !aBmp.Convert( (BmpConversion) nColorRes ) )
                        aBmp = aGraphic.GetBitmap();
                }
                sal_Bool    bRleCoding = aConfigItem.ReadBool( String( RTL_CONSTASCII_USTRINGPARAM( "RLE_Coding" ) ), sal_True );
                // Wollen wir RLE-Kodiert speichern?
                aBmp.Write( rOStm, bRleCoding );

                if( rOStm.GetError() )
                    nStatus = GRFILTER_IOERROR;
            }
            else if( aFilterName.EqualsIgnoreCaseAscii( EXP_SVMETAFILE ) )
            {
                sal_Int32 nVersion = aConfigItem.ReadInt32( String( RTL_CONSTASCII_USTRINGPARAM( "Version" ) ), 0 ) ;
                if ( nVersion )
                    rOStm.SetVersion( nVersion );
                GDIMetaFile	aMTF;

                if ( eType != GRAPHIC_BITMAP )
                    aMTF = aGraphic.GetGDIMetaFile();
                else
                {
                    VirtualDevice aVirDev;

                    aMTF.Record( &aVirDev );
                    aGraphic.Draw( &aVirDev, Point(), aGraphic.GetPrefSize() );
                    aMTF.Stop();
                    aMTF.SetPrefSize( aGraphic.GetPrefSize() );
                    aMTF.SetPrefMapMode( aGraphic.GetPrefMapMode() );
                }
                rOStm << aMTF;
                if( rOStm.GetError() )
                    nStatus = GRFILTER_IOERROR;
            }
            else if ( aFilterName.EqualsIgnoreCaseAscii( EXP_WMF ) )
            {
                if( eType == GRAPHIC_GDIMETAFILE )
                {
                    if ( !ConvertGDIMetaFileToWMF( aGraphic.GetGDIMetaFile(), rOStm, &aConfigItem ) )
                        nStatus = GRFILTER_FORMATERROR;
                }
                else
                {
                    Bitmap			aBmp( aGraphic.GetBitmap() );
                    GDIMetaFile		aMTF;
                    VirtualDevice	aVirDev;

                    aMTF.Record( &aVirDev );
                    aVirDev.DrawBitmap( Point(), aBmp );
                    aMTF.Stop();
                    aMTF.SetPrefSize( aBmp.GetSizePixel() );

                    if( !ConvertGDIMetaFileToWMF( aMTF, rOStm, &aConfigItem ) )
                        nStatus = GRFILTER_FORMATERROR;
                }
                if( rOStm.GetError() )
                    nStatus = GRFILTER_IOERROR;
            }
            else if ( aFilterName.EqualsIgnoreCaseAscii( EXP_EMF ) )
            {
                if( eType == GRAPHIC_GDIMETAFILE )
                {
                    if ( !ConvertGDIMetaFileToEMF( aGraphic.GetGDIMetaFile(), rOStm, &aConfigItem ) )
                        nStatus = GRFILTER_FORMATERROR;
                }
                else
                {
                    Bitmap			aBmp( aGraphic.GetBitmap() );
                    GDIMetaFile		aMTF;
                    VirtualDevice	aVirDev;

                    aMTF.Record( &aVirDev );
                    aVirDev.DrawBitmap( Point(), aBmp );
                    aMTF.Stop();
                    aMTF.SetPrefSize( aBmp.GetSizePixel() );

                    if( !ConvertGDIMetaFileToEMF( aMTF, rOStm, &aConfigItem ) )
                        nStatus = GRFILTER_FORMATERROR;
                }
                if( rOStm.GetError() )
                    nStatus = GRFILTER_IOERROR;
            }
            else if( aFilterName.EqualsIgnoreCaseAscii( EXP_JPEG ) )
            {
                if( !ExportJPEG( rOStm, aGraphic, pFilterData ) )
                    nStatus = GRFILTER_FORMATERROR;

                if( rOStm.GetError() )
                    nStatus = GRFILTER_IOERROR;
            }
            else if ( aFilterName.EqualsIgnoreCaseAscii( EXP_PNG ) )
            {
                vcl::PNGWriter aPNGWriter( aGraphic.GetBitmapEx(), pFilterData );
                if ( pFilterData )
                {
                    sal_Int32 k, j, i = 0;
                    for ( i = 0; i < pFilterData->getLength(); i++ )
                    {
                        if ( (*pFilterData)[ i ].Name.equalsAscii( "AdditionalChunks" ) )
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
                                            if ( rChunkData.size() )
                                            {
                                                sal_uInt32 nChunkLen = aByteSeq.getLength();

                                                vcl::PNGWriter::ChunkData aChunkData;
                                                aChunkData.nType = nChunkType;
                                                if ( nChunkLen )
                                                {
                                                    aChunkData.aData.resize( nChunkLen );
                                                    rtl_copyMemory( &aChunkData.aData[ 0 ], aByteSeq.getConstArray(), nChunkLen );
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
            else if( aFilterName.EqualsIgnoreCaseAscii( EXP_SVG ) )
            {
                try
                {
                    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xMgr( ::comphelper::getProcessServiceFactory() );

                    if( xMgr.is() )
                    {
                        ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler > xSaxWriter( xMgr->createInstance(
                            ::rtl::OUString::createFromAscii( "com.sun.star.xml.sax.Writer" ) ), ::com::sun::star::uno::UNO_QUERY );

                        ::com::sun::star::uno::Reference< ::com::sun::star::svg::XSVGWriter > xSVGWriter( xMgr->createInstance(
                            ::rtl::OUString::createFromAscii( "com.sun.star.svg.SVGWriter" ) ), ::com::sun::star::uno::UNO_QUERY );

                        if( xSaxWriter.is() && xSVGWriter.is() )
                        {
                            ::com::sun::star::uno::Reference< ::com::sun::star::io::XActiveDataSource > xActiveDataSource(
                                xSaxWriter, ::com::sun::star::uno::UNO_QUERY );

                            if( xActiveDataSource.is() )
                            {
                                const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >	xStmIf(
                                    static_cast< ::cppu::OWeakObject* >( new ImpFilterOutputStream( rOStm ) ) );

                                SvMemoryStream aMemStm( 65535, 65535 );

                                aMemStm.SetCompressMode( COMPRESSMODE_FULL );
                                ( (GDIMetaFile&) aGraphic.GetGDIMetaFile() ).Write( aMemStm );

                                xActiveDataSource->setOutputStream( ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >(
                                    xStmIf, ::com::sun::star::uno::UNO_QUERY ) );
                                ::com::sun::star::uno::Sequence< sal_Int8 > aMtfSeq( (sal_Int8*) aMemStm.GetData(), aMemStm.Tell() );
                                xSVGWriter->write( xSaxWriter, aMtfSeq );
                            }
                        }
                    }
                }
                catch( ::com::sun::star::uno::Exception& )
                {
                    nStatus = GRFILTER_IOERROR;
                }
            }
            else
                nStatus = GRFILTER_FILTERERROR;
        }
        else
        {
            xub_StrLen i, nTokenCount = aFilterPath.GetTokenCount( ';' );
            for ( i = 0; i < nTokenCount; i++ )
            {
                String aPhysicalName( ImpCreateFullFilterPath( aFilterPath.GetToken( i ), aFilterName ) );
                osl::Module aLibrary( aPhysicalName );

                PFilterCall pFunc = (PFilterCall) aLibrary.getFunctionSymbol( UniString::CreateFromAscii( EXPORT_FUNCTION_NAME ) );
                // Dialog in DLL ausfuehren
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
}

// ------------------------------------------------------------------------

void GraphicFilter::ResetLastError()
{
    pErrorEx->nFilterError = pErrorEx->nStreamError = 0UL;
}

// ------------------------------------------------------------------------

IMPL_LINK( GraphicFilter, FilterCallback, ConvertData*, pData )
{
    long nRet = 0L;

    if( pData )
    {
        USHORT		nFormat = GRFILTER_FORMAT_DONTKNOW;
        ByteString	aShortName;
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

            default:
            break;
        }
        if( GRAPHIC_NONE == pData->maGraphic.GetType() || pData->maGraphic.GetContext() ) // Import
        {
            // Import
            nFormat = GetImportFormatNumberForShortName( String( aShortName.GetBuffer(), RTL_TEXTENCODING_UTF8 ) );
            nRet = ImportGraphic( pData->maGraphic, String(), pData->mrStm ) == 0;
        }
        else if( aShortName.Len() )
        {
            // Export
            nFormat = GetExportFormatNumberForShortName( String( aShortName.GetBuffer(), RTL_TEXTENCODING_UTF8 ) );
            nRet = ExportGraphic( pData->maGraphic, String(), pData->mrStm, nFormat ) == 0;
        }
    }
    return nRet;
}

// ------------------------------------------------------------------------

GraphicFilter* GraphicFilter::GetGraphicFilter()
{
    if( !pGraphicFilter )
    {
        pGraphicFilter = new GraphicFilter;
        pGraphicFilter->GetImportFormatCount();
    }
    return pGraphicFilter;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
