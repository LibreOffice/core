/*************************************************************************
 *
 *  $RCSfile: filter.cxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: sj $ $Date: 2001-03-28 15:16:11 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#if defined UNX && defined ALPHA
#include <fstream.hxx>
#endif
#include <comphelper/processfactory.hxx>
#include <ucbhelper/content.hxx>
#include <cppuhelper/implbase1.hxx>
#include <tools/urlobj.hxx>
#include <vcl/salctype.hxx>
#include <vcl/virdev.hxx>
#include <vcl/svapp.hxx>
#include <osl/file.hxx>
#include "filter.hxx"
#include "FilterConfigCache.hxx"
#include <FilterConfigItem.hxx>
#include "fltcall.hxx"
#include "wmf.hxx"
#include "gifread.hxx"
#include "pngread.hxx"
#include "jpeg.hxx"
#include "xbmread.hxx"
#include "xpmread.hxx"
#include "solar.hrc"
#include "strings.hrc"
#include "dlgexpor.hxx"
#include "dlgejpg.hxx"
#include "sgffilt.hxx"
#ifndef _VOS_MODULE_HXX_
#include "vos/module.hxx"
#endif

#ifdef MAC
#include "extattr.hxx"
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif
#ifndef _COM_SUN_STAR_AWT_SIZE_HPP_
#include <com/sun/star/awt/Size.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XINTERFACE_HPP_
#include <com/sun/star/uno/XInterface.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XWEAK_HPP_
#include <com/sun/star/uno/XWeak.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XAGGREGATION_HPP_
#include <com/sun/star/uno/XAggregation.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XTYPEPROVIDER_HPP_
#include <com/sun/star/lang/XTypeProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XACTIVEDATASOURCE_HPP_
#include <com/sun/star/io/XActiveDataSource.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XOutputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_SVG_XSVGWRITER_HPP_
#include <com/sun/star/svg/XSVGWriter.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_XDOCUMENTHANDLER_HPP_
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_COMMANDABORTEDEXCEPTION_HPP_
#include <com/sun/star/ucb/CommandAbortedException.hpp>
#endif
#ifndef _UNTOOLS_UCBSTREAMHELPER_HXX
#include <unotools/ucbstreamhelper.hxx>
#endif
#ifndef _UNOTOOLS_LOCALFILEHELPER_HXX
#include <unotools/localfilehelper.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <pathoptions.hxx>
#endif

#if defined WIN || (defined OS2 && !defined ICC)

#define IMPORT_FUNCTION_NAME    "_GraphicImport"
#define EXPORT_FUNCTION_NAME    "_GraphicExport"
#define IMPDLG_FUNCTION_NAME    "_DoImportDialog"
#define EXPDLG_FUNCTION_NAME    "_DoExportDialog"

#else

#define IMPORT_FUNCTION_NAME    "GraphicImport"
#define EXPORT_FUNCTION_NAME    "GraphicExport"
#define IMPDLG_FUNCTION_NAME    "DoImportDialog"
#define EXPDLG_FUNCTION_NAME    "DoExportDialog"

#endif

// Compilerfehler, wenn Optimierung bei WNT & MSC
#if defined WNT && defined MSC
#pragma optimize( "", off )
#endif

// -----------
// - statics -
// -----------

ULONG   GraphicFilter::nFilterCount = 0UL;
Link    GraphicFilter::aLastFilterHdl = Link();

// -------------------------
// - ImpFilterOutputStream -
// -------------------------

class ImpFilterOutputStream : public ::cppu::WeakImplHelper1< ::com::sun::star::io::XOutputStream >
{
protected:

    SvStream&                           mrStm;

    virtual void SAL_CALL               writeBytes( const ::com::sun::star::uno::Sequence< sal_Int8 >& rData ) { mrStm.Write( rData.getConstArray(), rData.getLength() ); }
    virtual void SAL_CALL               flush() { mrStm.Flush(); }
    virtual void SAL_CALL               closeOutput() {}

public:

                                        ImpFilterOutputStream( SvStream& rStm ) : mrStm( rStm ) {}
                                        ~ImpFilterOutputStream() {}
};

// -------------------------
// - ImpFilterCallbackData -
// -------------------------

struct ImpFilterCallbackData
{
    GraphicFilter * pFilt;
    USHORT *        pPercent;
    Link *          pUpdatePercentHdl;
    BOOL *          pAbort;
    USHORT          nFilePercentOfTotal;
};

// ---------------------
// - ImpFilterCallback -
// ---------------------

BOOL ImplDirEntryHelper::Exists( const INetURLObject& rObj )
{
    BOOL bExists = FALSE;

    try
    {
        ::rtl::OUString aTitle;
        ::ucb::Content  aCnt( rObj.GetMainURL(),
                              ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XCommandEnvironment >() );

        aCnt.getPropertyValue( ::rtl::OUString::createFromAscii( "Title" ) ) >>= aTitle;

        if( aTitle.getLength() )
            bExists = TRUE;
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
        DBG_ERRORFILE( "Any other exception" );
    }

    return bExists;
}

// -----------------------------------------------------------------------------

void ImplDirEntryHelper::Kill( const String& rMainUrl )
{
    try
    {
        ::ucb::Content aCnt( rMainUrl,
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

// ---------------------
// - ImpFilterCallback -
// ---------------------

BOOL ImpFilterCallback( void* pCallerData, USHORT nPercent )
{
    ImpFilterCallbackData* pData= (ImpFilterCallbackData*) pCallerData;

    nPercent = nPercent * pData->nFilePercentOfTotal / 100;

    if( nPercent >= ( 3 + *pData->pPercent ) )
    {
        *pData->pPercent = nPercent;
        pData->pUpdatePercentHdl->Call( pData->pFilt );
    }

    return *pData->pAbort;
}

// --------------------
// - Helper functions -
// --------------------

//--------------------------------------------------------------------------

static BYTE* ImplSearchEntry( BYTE* pSource, BYTE* pDest, ULONG nComp, ULONG nSize )
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
    String          aExt;
    INetURLObject   aURL( rPath );
    aExt = aURL.GetFileExtension().ToUpperAscii();
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
    ULONG   nStreamPos = rStream.Tell();

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
        if( nStreamLen >= 525 )
        {
            BYTE sBuf[4];
            sal_uInt32 nOffset; // in ms documents the pict format is used without the first 512 bytes
            for ( nOffset = 10; nOffset <= 522; nOffset += 512 )
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
                delete pBuf;
                return TRUE;
            }
        }
        delete pBuf;
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

    return bTest && !bSomethingTested;
}

//--------------------------------------------------------------------------

sal_uInt16 GraphicFilter::ImpTestOrFindFormat( const String& rPath, SvStream& rStream, sal_uInt16& rFormat )
{
    sal_uInt16 n = pConfig->GetImportFormatCount();

/*
#ifdef MAC
    // ggf. Filter anhand der Mac-Typen raussuchen:
    if( *pFormat == GRFILTER_FORMAT_DONTKNOW )
    {
        String aFormType;
        String aType;
        SvEaMgr aFile( rPath );

        aFile.GetFileType( aType );
        aType.Cut( 4 );

        for( USHORT i = 0; i < n; i++ )
        {
            aFormType=ImpGetMacType(rConfig.ReadKey(i));
            aFormType.Cut( 4 );
            if( aFormType == aType && aFormType != "????" )
            {
                *pFormat = i;
                break;
            }
        }
    }
#endif
*/
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
        if( !ImpPeekGraphicFormat( rStream, pConfig->GetImportFormatExtension( rFormat ), TRUE ) )
            return GRFILTER_FORMATERROR;
    }

    return GRFILTER_OK;
}

//--------------------------------------------------------------------------

static Graphic ImpGetScaledGraphic( const Graphic& rGraphic, FilterConfigItem& rConfigItem )
{
    Graphic     aGraphic;
    ByteString  aResMgrName( "svt", 3 );
    ResMgr*     pResMgr;

    aResMgrName.Append( ByteString::CreateFromInt32( SOLARUPD ) );
    pResMgr = ResMgr::CreateResMgr( aResMgrName.GetBuffer(), Application::GetAppInternational().GetLanguage() );

    if ( rGraphic.GetType() != GRAPHIC_NONE )
    {
        sal_Int32   nMode = rConfigItem.ReadInt32( String( ResId( KEY_MODE, pResMgr ) ), 0 );

        if( rGraphic.GetType() == GRAPHIC_BITMAP )
        {
            // Auflösung wird eingestellt
            if( nMode == 1 )
            {
                Bitmap      aBitmap( rGraphic.GetBitmap() );
                MapMode     aMap( MAP_100TH_INCH );

                sal_Int32   nDPI = rConfigItem.ReadInt32( String( ResId( KEY_RES, pResMgr ) ), 75 );
                Fraction    aFrac( 1, Min( Max( nDPI, 75L ), 600L ) );

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
                Bitmap  aBitmap( rGraphic.GetBitmap() );

                ::com::sun::star::awt::Size aDefaultSize( 10000, 10000 );
                ::com::sun::star::awt::Size aSize = rConfigItem.ReadSize( String( ResId( KEY_SIZE, pResMgr ) ), aDefaultSize );
                aBitmap.SetPrefMapMode( MapMode( MAP_100TH_MM ) );
                aBitmap.SetPrefSize( Size( aSize.Width, aSize.Height ) );
                aGraphic = Graphic( aBitmap );
            }
            else
                aGraphic = rGraphic;
        }
        else
        {
            if( ( nMode == 1 ) || ( nMode == 2 ) )
            {
                GDIMetaFile aMtf( rGraphic.GetGDIMetaFile() );
                ::com::sun::star::awt::Size aDefaultSize( 10000, 10000 );
                ::com::sun::star::awt::Size aSize = rConfigItem.ReadSize( String( ResId( KEY_SIZE, pResMgr ) ), aDefaultSize );
                Size aNewSize( OutputDevice::LogicToLogic( Size( aSize.Width, aSize.Height ), MAP_100TH_MM, aMtf.GetPrefMapMode() ) );

                if( aNewSize.Width() && aNewSize.Height() )
                {
                    const Size aPrefSize( aMtf.GetPrefSize() );
                    aMtf.Scale( Fraction( aNewSize.Width(), aPrefSize.Width() ),
                                Fraction( aNewSize.Height(), aPrefSize.Height() ) );
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

static String ImpCreateFullFilterPath( const String& rPath, const String& rFilterName )
{
    ::rtl::OUString aDest, aNormalizedPath;
    ::osl::FileBase::normalizePath( rPath, aDest );
    aDest += ::rtl::OUString( sal_Unicode( '/' ) );
    aDest += ::rtl::OUString( rFilterName );
    ::osl::FileBase::getSystemPathFromNormalizedPath( aDest, aNormalizedPath );
    return String( aNormalizedPath );
}


// --------------------------
// - ImpFilterLibCacheEntry -
// --------------------------

class ImpFilterLibCache;

struct ImpFilterLibCacheEntry
{
    ImpFilterLibCacheEntry* mpNext;
    ::vos::OModule          maLibrary;
    String                  maFiltername;
    PFilterCall             mpfnImport;
    PFilterDlgCall          mpfnImportDlg;

                            ImpFilterLibCacheEntry( const String& rPathname, const String& rFiltername );
    int                     operator==( const String& rFiltername ) const { return maFiltername == rFiltername; }

    PFilterCall             GetImportFunction();
    PFilterDlgCall          GetImportDlgFunction();
    PFilterCall             GetExportFunction() { return (PFilterCall) maLibrary.getSymbol( UniString::CreateFromAscii( EXPORT_FUNCTION_NAME ) ); }
    PFilterDlgCall          GetExportDlgFunction() { return (PFilterDlgCall) maLibrary.getSymbol( UniString::CreateFromAscii( EXPDLG_FUNCTION_NAME ) ); }
};

// ------------------------------------------------------------------------

ImpFilterLibCacheEntry::ImpFilterLibCacheEntry( const String& rPathname, const String& rFiltername ) :
        mpNext          ( NULL ),
        maLibrary       ( rPathname ),
        maFiltername    ( rFiltername ),
        mpfnImport      ( NULL ),
        mpfnImportDlg   ( NULL )
{
}

// ------------------------------------------------------------------------

PFilterCall ImpFilterLibCacheEntry::GetImportFunction()
{
    if( !mpfnImport )
        mpfnImport = (PFilterCall) maLibrary.getSymbol( UniString::CreateFromAscii( IMPORT_FUNCTION_NAME ) );

    return mpfnImport;
}

// ------------------------------------------------------------------------

PFilterDlgCall ImpFilterLibCacheEntry::GetImportDlgFunction()
{
    if( !mpfnImportDlg )
        mpfnImportDlg = (PFilterDlgCall)maLibrary.getSymbol( UniString::CreateFromAscii( IMPDLG_FUNCTION_NAME ) );

    return mpfnImportDlg;
}

// ---------------------
// - ImpFilterLibCache -
// ---------------------

class ImpFilterLibCache
{
    ImpFilterLibCacheEntry* mpFirst;
    ImpFilterLibCacheEntry* mpLast;

public:
                            ImpFilterLibCache();
                            ~ImpFilterLibCache();

    ImpFilterLibCacheEntry* GetFilter( const String& rFilterPath, const String& rFiltername );
};

// ------------------------------------------------------------------------

ImpFilterLibCache::ImpFilterLibCache() :
    mpFirst     ( NULL ),
    mpLast      ( NULL )
{
}

// ------------------------------------------------------------------------

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

// ------------------------------------------------------------------------

ImpFilterLibCacheEntry* ImpFilterLibCache::GetFilter( const String& rFilterPath, const String& rFilterName )
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
        String aPhysicalName( ImpCreateFullFilterPath( rFilterPath, rFilterName ) );
        pEntry = new ImpFilterLibCacheEntry( aPhysicalName, rFilterName );

        if ( pEntry->maLibrary.isLoaded() )
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

static ImpFilterLibCache aCache;

// -----------------
// - GraphicFilter -
// -----------------

GraphicFilter::GraphicFilter()
{
    ImplInit();
}

// ------------------------------------------------------------------------

GraphicFilter::~GraphicFilter()
{
    // no graphic filter, no handler => restore old handler
    if( !--nFilterCount )
        Application::SetFilterHdl( aLastFilterHdl );

    delete pErrorEx;
    delete pConfig;
}

// ------------------------------------------------------------------------

void GraphicFilter::ImplInit()
{
    SvtPathOptions aPathOpt;
    aFilterPath = aPathOpt.GetFilterPath();
    pErrorEx = new FilterErrorEx;
    pConfig = new FilterConfigCache();
    nPercent = 0;
    bAbort = sal_False;

    // first instance of GraphicFilter sets link
    if( !nFilterCount++ )
    {
        // save old link
        aLastFilterHdl = Application::GetFilterHdl();
        Application::SetFilterHdl( LINK( this, GraphicFilter, FilterCallback ) );
    }
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

String GraphicFilter::GetImportFormatName( USHORT nFormat )
{
    return pConfig->GetImportFormatName( nFormat );
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

String GraphicFilter::GetImportOSFileType( USHORT nFormat )
{
    String aOSFileType;
/*
#ifdef MAC
    pConfig->SetGroup( IMP_FILTERSECTION );
    aOSFileType = ImpGetMacType( pConfig->ReadKey( nFormat ) );
#endif
*/
    return aOSFileType;
}

// ------------------------------------------------------------------------

String GraphicFilter::GetImportWildcard( USHORT nFormat )
{
    return pConfig->GetImportWildcard( nFormat );
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

String GraphicFilter::GetExportFormatName( USHORT nFormat )
{
    return pConfig->GetExportFormatName( nFormat );
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

String GraphicFilter::GetExportOSFileType( USHORT nFormat )
{
    String aOSFileType;
/*
#ifdef MAC
    pConfig->SetGroup( EXP_FILTERSECTION );
    aOSFileType = ImpGetMacType( pConfig->ReadKey( nFormat ) );
#endif
*/
    return aOSFileType;
}

// ------------------------------------------------------------------------

String GraphicFilter::GetExportWildcard( USHORT nFormat )
{
    return pConfig->GetExportWildcard( nFormat );
}

// ------------------------------------------------------------------------

BOOL GraphicFilter::IsExportPixelFormat( USHORT nFormat )
{
    return pConfig->IsExportPixelFormat( nFormat );
}

// ------------------------------------------------------------------------

USHORT GraphicFilter::CanImportGraphic( const INetURLObject& rPath,
                                        USHORT nFormat, USHORT* pDeterminedFormat )
{
    sal_uInt16  nRetValue = GRFILTER_FORMATERROR;
    DBG_ASSERT( rPath.GetProtocol() != INET_PROT_NOT_VALID, "GraphicFilter::CanImportGraphic() : ProtType == INET_PROT_NOT_VALID" );

    String      aMainUrl( rPath.GetMainURL() );
    SvStream*   pStream = ::utl::UcbStreamHelper::CreateStream( aMainUrl, STREAM_READ | STREAM_SHARE_DENYNONE );
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

USHORT GraphicFilter::ImportGraphic( Graphic& rGraphic, const INetURLObject& rPath,
                                     USHORT nFormat, USHORT * pDeterminedFormat )
{
    sal_uInt16 nRetValue = GRFILTER_FORMATERROR;
    DBG_ASSERT( rPath.GetProtocol() != INET_PROT_NOT_VALID, "GraphicFilter::ImportGraphic() : ProtType == INET_PROT_NOT_VALID" );

    String      aMainUrl( rPath.GetMainURL() );
    SvStream*   pStream = ::utl::UcbStreamHelper::CreateStream( aMainUrl, STREAM_READ | STREAM_SHARE_DENYNONE );
    if ( pStream )
    {
        nRetValue = ImportGraphic( rGraphic, aMainUrl, *pStream, nFormat, pDeterminedFormat );
        delete pStream;
    }
    return nRetValue;
}

//-------------------------------------------------------------------------

USHORT GraphicFilter::ImportGraphic( Graphic& rGraphic, const String& rPath, SvStream& rIStream,
                                     USHORT nFormat, USHORT* pDeterminedFormat )
{
    ImpFilterCallbackData   aCallbackData;
    String                  aFilterName;
    ULONG                   nStmBegin;
    USHORT                  nStatus;
    GraphicReader*          pContext = rGraphic.GetContext();
    GfxLinkType             eLinkType = GFX_LINK_TYPE_NONE;
    BOOL                    bDummyContext = ( pContext == (GraphicReader*) 1 );
    const BOOL              bLinkSet = rGraphic.IsLink();

    ResetLastError();

    if( !pContext || bDummyContext )
    {
        if( bDummyContext )
        {
            rGraphic.SetContext( NULL );
            nStmBegin = 0;
        }
        else
            nStmBegin = rIStream.Tell();

        aCallbackData.pFilt = this;
        aCallbackData.pPercent = &nPercent;
        aCallbackData.pUpdatePercentHdl = &aUpdatePercentHdlLink;
        aCallbackData.pAbort = &bAbort;
        aCallbackData.nFilePercentOfTotal = 100;
        bAbort = FALSE;
        nPercent = 0;
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

            if( !ImportGIF( rIStream, rGraphic, NULL ) )
                nStatus = GRFILTER_FILTERERROR;
            else
                eLinkType = GFX_LINK_TYPE_NATIVE_GIF;
        }
        else if( aFilterName.EqualsIgnoreCaseAscii( IMP_PNG ) )
        {
            if ( rGraphic.GetContext() == (GraphicReader*) 1 )
                rGraphic.SetContext( NULL );
            if ( !ImportPNG( rIStream, rGraphic, NULL ) )
                nStatus = GRFILTER_FILTERERROR;
            else
                eLinkType = GFX_LINK_TYPE_NATIVE_PNG;
        }
        else if( aFilterName.EqualsIgnoreCaseAscii( IMP_JPEG ) )
        {
            if( rGraphic.GetContext() == (GraphicReader*) 1 )
                rGraphic.SetContext( NULL );

            if( !ImportJPEG( rIStream, rGraphic, NULL ) )
                nStatus = GRFILTER_FILTERERROR;
            else
                eLinkType = GFX_LINK_TYPE_NATIVE_JPG;
        }
        else if( aFilterName.EqualsIgnoreCaseAscii( IMP_XBM ) )
        {
            if( rGraphic.GetContext() == (GraphicReader*) 1 )
                rGraphic.SetContext( NULL );

            if( !ImportXBM( rIStream, rGraphic, NULL ) )
                nStatus = GRFILTER_FILTERERROR;
        }
        else if( aFilterName.EqualsIgnoreCaseAscii( IMP_XPM ) )
        {
            if( rGraphic.GetContext() == (GraphicReader*) 1 )
                rGraphic.SetContext( NULL );

            if( !ImportXPM( rIStream, rGraphic, NULL ) )
                nStatus = GRFILTER_FILTERERROR;
        }
        else if( aFilterName.EqualsIgnoreCaseAscii( IMP_BMP ) ||
                    aFilterName.EqualsIgnoreCaseAscii( IMP_SVMETAFILE ) )
        {
            // SV interne Importfilter fuer Bitmaps und MetaFiles
            aStartFilterHdlLink.Call( this );
            nPercent = 60;
            aUpdatePercentHdlLink.Call( this );
            rIStream >> rGraphic;
            nPercent = 100;
            aUpdatePercentHdlLink.Call( this );

            if( rIStream.GetError() )
                nStatus = GRFILTER_FORMATERROR;
        }
        else if( aFilterName.EqualsIgnoreCaseAscii( IMP_WMF ) ||
                aFilterName.EqualsIgnoreCaseAscii( IMP_EMF ) )
        {
            GDIMetaFile aMtf;
            aStartFilterHdlLink.Call( this );
            nPercent = 1;

            if( !ConvertWMFToGDIMetaFile( rIStream, aMtf, &ImpFilterCallback, &aCallbackData ) )
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
            USHORT          nVersion;
            unsigned char   nTyp = CheckSgfTyp( rIStream, nVersion );

            switch( nTyp )
            {
                case SGF_BITIMAGE:
                {
                    SvMemoryStream aTempStream;
                    if( aTempStream.GetError() )
                        return GRFILTER_OPENERROR;

                    // SGF in temporaere Datei filtern
                    aStartFilterHdlLink.Call( this );
                    nPercent = 30;
                    aUpdatePercentHdlLink.Call( this );

                    if( !SgfBMapFilter( rIStream, aTempStream ) )
                        nStatus = GRFILTER_FILTERERROR;
                    else
                    {
                        nPercent = 60;
                        aUpdatePercentHdlLink.Call( this );
                        aTempStream.Seek( 0L );
                        aTempStream >> rGraphic;

                        nPercent = 100;
                        aUpdatePercentHdlLink.Call( this );
                        if( aTempStream.GetError() )
                            nStatus = GRFILTER_FILTERERROR;
                    }
                }
                break;

                case SGF_SIMPVECT:
                {
                    GDIMetaFile aMtf;

                    aStartFilterHdlLink.Call( this );
                    nPercent = 50;
                    aUpdatePercentHdlLink.Call( this );
                    if( !SgfVectFilter( rIStream, aMtf ) )
                        nStatus = GRFILTER_FILTERERROR;
                    else
                    {
                        nPercent = 100;
                        aUpdatePercentHdlLink.Call( this );
                        rGraphic = Graphic( aMtf );
                    }
                }
                break;

                case SGF_STARDRAW:
                {
                    if( nVersion != SGV_VERSION )
                        nStatus = GRFILTER_VERSIONERROR;
                    else
                    {
                        GDIMetaFile aMtf;

                        aStartFilterHdlLink.Call( this );
                        nPercent = 50;
                        aUpdatePercentHdlLink.Call( this );
                        if( !SgfSDrwFilter( rIStream, aMtf, aFilterPath ) )
                            nStatus = GRFILTER_FILTERERROR;
                        else
                        {
                            nPercent = 100;
                            aUpdatePercentHdlLink.Call( this );
                            rGraphic = Graphic( aMtf );
                        }
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

        // find first filter in filter pathes
        xub_StrLen i, nTokenCount = aFilterPath.GetTokenCount( ';' );
        for( i = 0; ( i < nTokenCount ) && ( pFilter == NULL ); i++ )
            pFilter = aCache.GetFilter( aFilterPath, aFilterName );
        if( !pFilter )
            nStatus = GRFILTER_FILTERERROR;
        else
        {
            PFilterCall pFunc = pFilter->GetImportFunction();

            if( !pFunc )
                nStatus = GRFILTER_FILTERERROR;
            else
            {
                aStartFilterHdlLink.Call( this );
                aUpdatePercentHdlLink.Call( this );

                if( !(*pFunc)( rIStream, rGraphic, &ImpFilterCallback, &aCallbackData, NULL, sal_False ) )
                    nStatus = GRFILTER_FORMATERROR;
                else
                {
                    nPercent = 100;
                    aUpdatePercentHdlLink.Call( this );

                    // try to set link type if format matches
                    if( nFormat != GRFILTER_FORMAT_DONTKNOW )
                    {
                        const String aShortName( GetImportFormatShortName( nFormat ).ToUpperAscii() );

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
    // Set error code or try to set native buffer
    if( nStatus != GRFILTER_OK )
    {
        if( bAbort )
            nStatus = GRFILTER_ABORT;

        ImplSetError( nStatus, &rIStream );
        rIStream.Seek( nStmBegin );
        rGraphic.Clear();

        if( nPercent )
            aErrorHdlLink.Call( this );
    }
    else if( ( eLinkType != GFX_LINK_TYPE_NONE ) && !rGraphic.GetContext() && !bLinkSet )
    {
        const ULONG nStmEnd = rIStream.Tell();
        const ULONG nBufSize = nStmEnd - nStmBegin;

        if( nBufSize )
        {
            BYTE*           pBuf = new BYTE[ nBufSize ];
            GraphicReader*  pOldContext = rGraphic.GetContext();

            rIStream.Seek( nStmBegin );
            rIStream.Read( pBuf, nBufSize );
            rGraphic.SetLink( GfxLink( pBuf, nBufSize, eLinkType, TRUE ) );
        }
    }

    if( nPercent && !pContext )
    {
        nPercent = 0;
        aEndFilterHdlLink.Call( this );
    }

    return nStatus;
}

// ------------------------------------------------------------------------

USHORT GraphicFilter::ExportGraphic( const Graphic& rGraphic, const INetURLObject& rPath,
                                            sal_uInt16 nFormat, sal_Bool bIgnoreOptions )
{
    sal_uInt16  nRetValue = GRFILTER_FORMATERROR;
    DBG_ASSERT( rPath.GetProtocol() != INET_PROT_NOT_VALID, "GraphicFilter::ExportGraphic() : ProtType == INET_PROT_NOT_VALID" );
    BOOL        bAlreadyExists = ImplDirEntryHelper::Exists( rPath );

    String      aMainUrl( rPath.GetMainURL() );
    SvStream*   pStream = ::utl::UcbStreamHelper::CreateStream( aMainUrl, STREAM_WRITE | STREAM_TRUNC );
    if ( pStream )
    {
        nRetValue = ExportGraphic( rGraphic, aMainUrl, *pStream, nFormat, bIgnoreOptions );
        delete pStream;

        if( ( GRFILTER_OK != nRetValue ) && !bAlreadyExists )
            ImplDirEntryHelper::Kill( aMainUrl );
    }
    return nRetValue;
}

// ------------------------------------------------------------------------

USHORT GraphicFilter::ExportGraphic( const Graphic& rGraphic, const String& rPath,
                                     SvStream& rOStm, sal_uInt16 nFormat, sal_Bool bIgnoreOptions )
{
    USHORT nFormatCount = GetExportFormatCount();

    ResetLastError();

    if( nFormat == GRFILTER_FORMAT_DONTKNOW )
    {
        INetURLObject aURL( rPath );
        String aExt( aURL.GetFileExtension().ToUpperAscii() );


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

    FilterConfigItem* pConfigItem = NULL;
    String aFilterName( pConfig->GetExportFilterName( nFormat ) );

    ImpFilterCallbackData aCallbackData;
    aCallbackData.pFilt=this;
    aCallbackData.pPercent=&nPercent;
    aCallbackData.pUpdatePercentHdl=&aUpdatePercentHdlLink;
    aCallbackData.pAbort=&bAbort;
    aCallbackData.nFilePercentOfTotal=100;
    bAbort=FALSE;

    nPercent=0;
    aStartFilterHdlLink.Call(this);
    aUpdatePercentHdlLink.Call(this);

    USHORT      nStatus = GRFILTER_OK;
    GraphicType eType;
    Graphic     aGraphic( rGraphic );

    if ( !bIgnoreOptions )
    {
        String aFilterConfigPath( RTL_CONSTASCII_USTRINGPARAM( "Office.Common/Filter/Graphic/Export/" ) );
        String aFilterType( pConfig->GetExportFormatShortName( nFormat ) );
        aFilterConfigPath.Append( aFilterType );
        pConfigItem = new FilterConfigItem( aFilterConfigPath );

        ImpGetScaledGraphic( rGraphic, *pConfigItem );
    }

    eType = aGraphic.GetType();

    if( pConfig->IsExportPixelFormat( nFormat ) )
    {
        if( eType != GRAPHIC_BITMAP )
        {
            Size aSizePixel;
            ULONG nColorCount,nBitsPerPixel,nNeededMem,nMaxMem;
            VirtualDevice aVirDev;

            // Maximalen Speicherbedarf fuer das Bildes holen:
//          if( GetOptionsConfig() )
//              nMaxMem = (UINT32)GetOptionsConfig()->ReadKey( "VEC-TO-PIX-MAX-KB", "1024" ).ToInt32();
//          else
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
    if( GRFILTER_OK == nStatus )
    {
        if ( pConfig->IsExportInternalFilter( nFormat ) )
        {
            if( aFilterName.EqualsIgnoreCaseAscii( EXP_BMP ) )
            {
                nPercent = 60;
                aUpdatePercentHdlLink.Call( this );

                if( !rOStm.GetError() )
                {
                    Bitmap      aBmp( aGraphic.GetBitmap() );
                    if ( pConfigItem )
                    {
                        ResMgr*     pResMgr = CREATERESMGR( svt );
                        sal_Bool    bRleCoding = pConfigItem->ReadBool( String( ResId( KEY_RLE_CODING, pResMgr ) ), sal_True );
                        // Wollen wir RLE-Kodiert speichern?
                        aGraphic.GetBitmap().Write( rOStm, bRleCoding );
                        delete pResMgr;
                    }
                    else
                        rOStm << aBmp;
                }
                nPercent = 90;
                aUpdatePercentHdlLink.Call( this );

                if( rOStm.GetError() )
                    nStatus = GRFILTER_IOERROR;
            }
            else if( aFilterName.EqualsIgnoreCaseAscii( EXP_SVMETAFILE ) )
            {
                if( !rOStm.GetError() )
                {
                    GDIMetaFile aMTF;

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

                    nPercent = 60;
                    aUpdatePercentHdlLink.Call( this );
                    rOStm << aMTF;
                    nPercent = 90;
                    aUpdatePercentHdlLink.Call( this );
                }

                if( rOStm.GetError() )
                    nStatus = GRFILTER_IOERROR;
            }
            else if ( aFilterName.EqualsIgnoreCaseAscii( EXP_WMF ) )
            {
                if( !rOStm.GetError() )
                {
                    if( eType == GRAPHIC_GDIMETAFILE )
                    {
                        if ( !ConvertGDIMetaFileToWMF( aGraphic.GetGDIMetaFile(), rOStm, &ImpFilterCallback, &aCallbackData ) )
                            nStatus = GRFILTER_FORMATERROR;
                    }
                    else
                    {
                        Bitmap          aBmp( aGraphic.GetBitmap() );
                        GDIMetaFile     aMTF;
                        VirtualDevice   aVirDev;

                        aMTF.Record( &aVirDev );
                        aVirDev.DrawBitmap( Point(), aBmp );
                        aMTF.Stop();
                        aMTF.SetPrefSize( aBmp.GetSizePixel() );

                        if( !ConvertGDIMetaFileToWMF( aMTF, rOStm, &ImpFilterCallback, &aCallbackData) )
                            nStatus = GRFILTER_FORMATERROR;
                    }
                }

                if( rOStm.GetError() )
                    nStatus = GRFILTER_IOERROR;
            }
            else if ( aFilterName.EqualsIgnoreCaseAscii( EXP_EMF ) )
            {
                if( !rOStm.GetError() )
                {
                    if( eType == GRAPHIC_GDIMETAFILE )
                    {
                        if ( !ConvertGDIMetaFileToEMF( aGraphic.GetGDIMetaFile(), rOStm, &ImpFilterCallback, &aCallbackData ) )
                            nStatus = GRFILTER_FORMATERROR;
                    }
                    else
                    {
                        Bitmap          aBmp( aGraphic.GetBitmap() );
                        GDIMetaFile     aMTF;
                        VirtualDevice   aVirDev;

                        aMTF.Record( &aVirDev );
                        aVirDev.DrawBitmap( Point(), aBmp );
                        aMTF.Stop();
                        aMTF.SetPrefSize( aBmp.GetSizePixel() );

                        if( !ConvertGDIMetaFileToEMF( aMTF, rOStm, &ImpFilterCallback, &aCallbackData) )
                            nStatus = GRFILTER_FORMATERROR;
                    }
                }

                if( rOStm.GetError() )
                    nStatus = GRFILTER_IOERROR;
            }
            else if( aFilterName.EqualsIgnoreCaseAscii( EXP_JPEG ) )
            {
                if( !rOStm.GetError() )
                {
                    if( !ExportJPEG( rOStm, aGraphic, &ImpFilterCallback, &aCallbackData, bIgnoreOptions ) )
                        nStatus = GRFILTER_FORMATERROR;
                }

                if( rOStm.GetError() )
                    nStatus = GRFILTER_IOERROR;
            }
            else if( aFilterName.EqualsIgnoreCaseAscii( EXP_SVG ) )
            {
                if( !rOStm.GetError() )
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
                                const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > xStmIf(
                                    static_cast< ::cppu::OWeakObject* >( new ImpFilterOutputStream( rOStm ) ) );

                                SvMemoryStream aMemStm( 65535, 65535 );

                                aMemStm.SetCompressMode( COMPRESSMODE_FULL );
                                ( (GDIMetaFile&) aGraphic.GetGDIMetaFile() ).Write( aMemStm );

                                nPercent = 60;
                                aUpdatePercentHdlLink.Call( this );

                                xActiveDataSource->setOutputStream( ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >(
                                    xStmIf, ::com::sun::star::uno::UNO_QUERY ) );
                                ::com::sun::star::uno::Sequence< sal_Int8 > aMtfSeq( (sal_Int8*) aMemStm.GetData(), aMemStm.Tell() );
                                xSVGWriter->write( xSaxWriter, aMtfSeq );
                                nPercent = 90;
                                aUpdatePercentHdlLink.Call( this );
                            }
                        }
                    }
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
                ::vos::OModule aLibrary( aPhysicalName );

                PFilterCall pFunc = (PFilterCall) aLibrary.getSymbol( UniString::CreateFromAscii( EXPORT_FUNCTION_NAME ) );
                // Dialog in DLL ausfuehren
                if( pFunc )
                {
                    if( !rOStm.GetError() )
                    {
                        if ( !(*pFunc)( rOStm, aGraphic, &ImpFilterCallback, &aCallbackData, pConfigItem, sal_False ) )
                            nStatus = GRFILTER_FORMATERROR;
                    }
                    else
                        nStatus = GRFILTER_IOERROR;
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
        aErrorHdlLink.Call( this );
    }
    else
    {
        nPercent = 100;
        aUpdatePercentHdlLink.Call( this );
    }

    aEndFilterHdlLink.Call( this );
    delete pConfigItem;

    return nStatus;
}

// ------------------------------------------------------------------------

BOOL GraphicFilter::Setup( USHORT nFormat )
{
    return FALSE;
}

// ------------------------------------------------------------------------

BOOL GraphicFilter::HasImportDialog( USHORT nFormat )
{
    return pConfig->IsImportDialog( nFormat );
}

// ------------------------------------------------------------------------

BOOL GraphicFilter::DoImportDialog( Window* pWindow, USHORT nFormat )
{
    String  aFilterName( pConfig->GetImportFilterName( nFormat ) );
    BOOL    bRet = FALSE;

    if ( pConfig->IsImportInternalFilter( nFormat ) )
    {
        if( ( aFilterName.EqualsIgnoreCaseAscii( IMP_BMP ) ) || ( aFilterName.EqualsIgnoreCaseAscii( IMP_SVMETAFILE ) ) )
        {
            // Bitmaps und SV-MetaFiles
        }
        else if( aFilterName.EqualsIgnoreCaseAscii( IMP_WMF ) )
        {
            // Import-Filter fuer WMF
        }
        else if( ( aFilterName.EqualsIgnoreCaseAscii( IMP_SVSGF ) ) || ( aFilterName.EqualsIgnoreCaseAscii( IMP_SVSGV ) ) )
        {
            // StarDraw- und StarWriter-Filter
        }
    }
    else
    {
        ImpFilterLibCacheEntry* pFilter = NULL;

        // find first filter in filter pathes
        xub_StrLen i, nTokenCount = aFilterPath.GetTokenCount( ';' );
        for( i = 0; ( i < nTokenCount ) && ( pFilter == NULL ); i++ )
            pFilter = aCache.GetFilter( aFilterPath.GetToken( i ), aFilterName );
        if( pFilter )
        {
            PFilterDlgCall pFunc = pFilter->GetImportDlgFunction();
            // Dialog in DLL ausfuehren
            if( pFunc )
            {
                FltCallDialogParameter aFltCallDlgPara( pWindow, NULL, FUNIT_MM );
                bRet = (*pFunc)( aFltCallDlgPara );
            }
        }
    }

    return bRet;
}

// ------------------------------------------------------------------------

BOOL GraphicFilter::HasExportDialog( USHORT nFormat )
{
    return pConfig->IsExportDialog( nFormat );
}

// ------------------------------------------------------------------------

BOOL GraphicFilter::DoExportDialog( Window* pWindow, USHORT nFormat )
{
    return DoExportDialog( pWindow, nFormat, FUNIT_MM );
}

BOOL GraphicFilter::DoExportDialog( Window* pWindow, USHORT nFormat, FieldUnit eFieldUnit )
{
    String  aFilterName( pConfig->GetExportFilterName( nFormat ) );

    sal_Bool bRet = sal_False;

    if ( pConfig->IsExportInternalFilter( nFormat ) )
    {
        // Export-Dialog fuer Bitmap's, SVM's und WMF's
        if( ( aFilterName.EqualsIgnoreCaseAscii( EXP_BMP ) ) ||
            ( aFilterName.EqualsIgnoreCaseAscii( EXP_SVMETAFILE ) ) ||
            ( aFilterName.EqualsIgnoreCaseAscii( EXP_WMF ) ) ||
            ( aFilterName.EqualsIgnoreCaseAscii( EXP_EMF ) ) ||
            ( aFilterName.EqualsIgnoreCaseAscii( EXP_JPEG ) ) )
        {
            ByteString  aResMgrName( "svt", 3 );
            ResMgr*     pResMgr;

            aResMgrName.Append( ByteString::CreateFromInt32( SOLARUPD ) );
            pResMgr = ResMgr::CreateResMgr( aResMgrName.GetBuffer(), Application::GetAppInternational().GetLanguage() );

            FltCallDialogParameter aFltCallDlgPara( pWindow, pResMgr, eFieldUnit );

            // JPEG-Dialog
            if( aFilterName.EqualsIgnoreCaseAscii( EXP_JPEG ) )
                bRet = ( DlgExportEJPG( aFltCallDlgPara ).Execute() == RET_OK );
            // Fuer Bitmaps nehmen wir den Pixel-Dialog
            else if( !aFilterName.EqualsIgnoreCaseAscii( EXP_BMP ) )
            {
                aFltCallDlgPara.aFilterExt = pConfig->GetExportFormatShortName( nFormat );
                bRet = ( DlgExportVec( aFltCallDlgPara ).Execute() == RET_OK );
            }
            // Fuer Vektorformate nehmen wir den Vektor-Dialog
            else
            {
                aFltCallDlgPara.aFilterExt = pConfig->GetExportFormatShortName( nFormat );
                bRet = ( DlgExportPix( aFltCallDlgPara ).Execute() == RET_OK );
            }
            delete pResMgr;
        }
    }
    else    // ladbare Filter
    {
        xub_StrLen i, nTokenCount = aFilterPath.GetTokenCount( ';' );
        for ( i = 0; i < nTokenCount; i++ )
        {
            String aPhysicalName( ImpCreateFullFilterPath( aFilterPath.GetToken( i ), aFilterName ) );
            ::vos::OModule aLibrary( aPhysicalName );
            PFilterDlgCall  pFunc = (PFilterDlgCall) aLibrary.getSymbol( UniString( EXPDLG_FUNCTION_NAME, RTL_TEXTENCODING_UTF8 ) );
            // Dialog in DLL ausfuehren
            if( pFunc )
            {
                FltCallDialogParameter aFltCallDlgPara( pWindow, NULL, eFieldUnit );
                bRet = (*pFunc)( aFltCallDlgPara );
            }
        }
    }
    return bRet;
}

// ------------------------------------------------------------------------

const FilterErrorEx& GraphicFilter::GetLastError() const
{
    return *pErrorEx;
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
        USHORT      nFormat = GRFILTER_FORMAT_DONTKNOW;
        ByteString  aShortName;
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
            nRet = ImportGraphic( pData->maGraphic, String(), pData->mrStm, nFormat ) == 0;
        }
        else if( aShortName.Len() )
        {
            // Export
            nFormat = GetExportFormatNumberForShortName( String( aShortName.GetBuffer(), RTL_TEXTENCODING_UTF8 ) );
            nRet = ExportGraphic( pData->maGraphic, String(), pData->mrStm, nFormat, sal_True ) == 0;
        }
    }
    return nRet;
}
