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
#include "precompiled_sfx2.hxx"

#include <tools/urlobj.hxx>

#include <sfx2/objsh.hxx>
#include <sfx2/docfile.hxx>
#include "openflag.hxx"

#include <svtools/htmlkywd.hxx>
#include <svtools/htmltokn.h>
#include <svtools/imap.hxx>
#include <svtools/imapcirc.hxx>
#include <svtools/imapobj.hxx>
#include <svtools/imappoly.hxx>
#include <svtools/imaprect.hxx>
#ifndef _SVSTDARR_ULONGS_DECL
#define _SVSTDARR_ULONGS
#include <svl/svstdarr.hxx>
#endif
#include <svl/zforlist.hxx>
#include <rtl/tencinfo.h>
#include <tools/tenccvt.hxx>

#include <sfx2/sfxhtml.hxx>

#include <com/sun/star/beans/XPropertyContainer.hpp>


using namespace ::com::sun::star;


sal_Char sHTML_MIME_text[] = "text/";
sal_Char sHTML_MIME_application[] = "application/";
sal_Char sHTML_MIME_experimental[] = "x-";

// <INPUT TYPE=xxx>
static HTMLOptionEnum const aAreaShapeOptEnums[] =
{
    { OOO_STRING_SVTOOLS_HTML_SH_rect,      IMAP_OBJ_RECTANGLE  },
    { OOO_STRING_SVTOOLS_HTML_SH_rectangle, IMAP_OBJ_RECTANGLE  },
    { OOO_STRING_SVTOOLS_HTML_SH_circ,      IMAP_OBJ_CIRCLE     },
    { OOO_STRING_SVTOOLS_HTML_SH_circle,    IMAP_OBJ_CIRCLE     },
    { OOO_STRING_SVTOOLS_HTML_SH_poly,          IMAP_OBJ_POLYGON    },
    { OOO_STRING_SVTOOLS_HTML_SH_polygon,       IMAP_OBJ_POLYGON    },
    { 0,                    0                   }
};

SfxHTMLParser::SfxHTMLParser( SvStream& rStream, BOOL bIsNewDoc,
                              SfxMedium *pMed ) :
    HTMLParser( rStream, bIsNewDoc ),
    pMedium( pMed ), pDLMedium( 0 ),
    nMetaTags( 0 )
{
    DBG_ASSERT( RTL_TEXTENCODING_DONTKNOW == GetSrcEncoding( ),
                "SfxHTMLParser::SfxHTMLParser: Wo kommt der ZS her?" );
    DBG_ASSERT( !IsSwitchToUCS2(),
                "SfxHTMLParser::SfxHTMLParser: Switch to UCS2?" );

    // Altough the real default encoding is ISO8859-1, we use MS-1252
    // as default encoding.
    SetSrcEncoding( GetExtendedCompatibilityTextEncoding(  RTL_TEXTENCODING_ISO_8859_1 ) );

    // If the file starts with a BOM, switch to UCS2.
    SetSwitchToUCS2( TRUE );
}

SfxHTMLParser::~SfxHTMLParser()
{
    DBG_ASSERT( !pDLMedium, "Here is a File Download that has got stuck" );
    delete pDLMedium;
}

BOOL SfxHTMLParser::ParseMapOptions(ImageMap * pImageMap,
                                    const HTMLOptions * pOptions)
{
    DBG_ASSERT( pImageMap, "ParseMapOptions: No Image-Map" );
    DBG_ASSERT( pOptions, "ParseMapOptions: No Options" );

    String aName;

    for( USHORT i=pOptions->Count(); i; )
    {
        const HTMLOption *pOption = (*pOptions)[--i];
        switch( pOption->GetToken() )
        {
        case HTML_O_NAME:
            aName = pOption->GetString();
            break;
        }
    }

    if( aName.Len() )
        pImageMap->SetName( aName );

    return aName.Len() > 0;
}

BOOL SfxHTMLParser::ParseAreaOptions(ImageMap * pImageMap, const String& rBaseURL,
                                     const HTMLOptions * pOptions,
                                     USHORT nEventMouseOver,
                                     USHORT nEventMouseOut )
{
    DBG_ASSERT( pImageMap, "ParseAreaOptions: no Image-Map" );
    DBG_ASSERT( pOptions, "ParseAreaOptions: no Options" );

    USHORT nShape = IMAP_OBJ_RECTANGLE;
    SvULongs aCoords;
    String aName, aHRef, aAlt, aTarget, sEmpty;
    BOOL bNoHRef = FALSE;
    SvxMacroTableDtor aMacroTbl;

    for( USHORT i=pOptions->Count(); i; )
    {
        USHORT nEvent = 0;
        ScriptType eScrpType = STARBASIC;
        const HTMLOption *pOption = (*pOptions)[--i];
        switch( pOption->GetToken() )
        {
        case HTML_O_NAME:
            aName = pOption->GetString();
            break;
        case HTML_O_SHAPE:
            pOption->GetEnum( nShape, aAreaShapeOptEnums );
            break;
        case HTML_O_COORDS:
            pOption->GetNumbers( aCoords, TRUE );
            break;
        case HTML_O_HREF:
            aHRef = INetURLObject::GetAbsURL( rBaseURL, pOption->GetString() );
            break;
        case HTML_O_NOHREF:
            bNoHRef = TRUE;
            break;
        case HTML_O_ALT:
            aAlt = pOption->GetString();
            break;
        case HTML_O_TARGET:
            aTarget = pOption->GetString();
            break;

        case HTML_O_ONMOUSEOVER:
            eScrpType = JAVASCRIPT;
        case HTML_O_SDONMOUSEOVER:
            nEvent = nEventMouseOver;
            goto IMAPOBJ_SETEVENT;

        case HTML_O_ONMOUSEOUT:
            eScrpType = JAVASCRIPT;
        case HTML_O_SDONMOUSEOUT:
            nEvent = nEventMouseOut;
            goto IMAPOBJ_SETEVENT;
IMAPOBJ_SETEVENT:
            if( nEvent )
            {
                String sTmp( pOption->GetString() );
                if( sTmp.Len() )
                {
                    sTmp.ConvertLineEnd();
                    aMacroTbl.Insert( nEvent,
                        new SvxMacro( sTmp, sEmpty, eScrpType ));
                }
            }
            break;
        }
    }

    if( bNoHRef )
        aHRef.Erase();

    BOOL bNewArea = TRUE;
    switch( nShape )
    {
    case IMAP_OBJ_RECTANGLE:
        if( aCoords.Count() >=4 )
        {
            Rectangle aRec( aCoords[0], aCoords[1],
                            aCoords[2], aCoords[3] );
            IMapRectangleObject aMapRObj( aRec, aHRef, aAlt, String(), aTarget, aName,
                                          !bNoHRef );
            if( aMacroTbl.Count() )
                aMapRObj.SetMacroTable( aMacroTbl );
            pImageMap->InsertIMapObject( aMapRObj );
        }
        break;
    case IMAP_OBJ_CIRCLE:
        if( aCoords.Count() >=3 )
        {
            Point aPoint( aCoords[0], aCoords[1] );
            IMapCircleObject aMapCObj( aPoint, aCoords[2],aHRef, aAlt, String(),
                                       aTarget, aName, !bNoHRef );
            if( aMacroTbl.Count() )
                aMapCObj.SetMacroTable( aMacroTbl );
            pImageMap->InsertIMapObject( aMapCObj );
        }
        break;
    case IMAP_OBJ_POLYGON:
        if( aCoords.Count() >=6 )
        {
            USHORT nCount = aCoords.Count() / 2;
            Polygon aPoly( nCount );
            for( USHORT i=0; i<nCount; i++ )
                aPoly[i] = Point( aCoords[2*i], aCoords[2*i+1] );
            IMapPolygonObject aMapPObj( aPoly, aHRef, aAlt, String(), aTarget, aName,
                                        !bNoHRef );
            if( aMacroTbl.Count() )
                aMapPObj.SetMacroTable( aMacroTbl );
            pImageMap->InsertIMapObject( aMapPObj );
        }
        break;
    default:
        bNewArea = FALSE;
    }

    return bNewArea;
}


void SfxHTMLParser::StartFileDownload( const String& rURL, int nToken,
                                       SfxObjectShell *pSh )
{
    DBG_ASSERT( !pDLMedium, "StartFileDownload when active Download" );
    if( pDLMedium )
        return;

    pDLMedium = new SfxMedium( rURL, SFX_STREAM_READONLY, FALSE );
    if( pSh )
    {
        // Register the medium, so that it can be stopped.
        pSh->RegisterTransfer( *pDLMedium );
    }

    // Push Download (Note: Can also be synchronous).
    if ( TRUE /*pMedium->GetDoneLink() == Link()*/ )
        pDLMedium->DownLoad();
    else
    {
        // Set Downloading-Flag to TRUE. When we get into the Pending-status
        // we will then also have Data-Available-Links.
        SetDownloadingFile( TRUE );
        pDLMedium->DownLoad( STATIC_LINK( this, SfxHTMLParser, FileDownloadDone ) );

        // If the Downloading-Flag is still set downloading will be done
        // asynchronously. We will go into Pedning-status and wait there.
        // As long as we are there all calls to the Data-Link Avaialble are
        // locked.
        if( IsDownloadingFile() )
        {
            // Unfreeze the current state and go into the Pending-Status.
            // When the download is completed or aborted, a Continue with
            // the transfer token will be pushed by passedNewDataRead.
            SaveState( nToken );
            eState = SVPAR_PENDING;
        }
    }
}

BOOL SfxHTMLParser::FinishFileDownload( String& rStr )
{
    String aStr;

    BOOL bOK = pDLMedium && pDLMedium->GetErrorCode()==0;
    if( bOK )
    {
        SvStream* pStream = pDLMedium->GetInStream();
        DBG_ASSERT( pStream, "Kein In-Stream vom Medium erhalten" );

        SvMemoryStream aStream;
        if( pStream )
            aStream << *pStream;

        aStream.Seek( STREAM_SEEK_TO_END );
        DBG_ASSERT( aStream.Tell() < STRING_MAXLEN,
                    "File zu lang fuer einen String, Ende abgeschnitten" );
        xub_StrLen nLen = aStream.Tell() < STRING_MAXLEN
                        ? (xub_StrLen)aStream.Tell()
                        : STRING_MAXLEN;

        ByteString sBuffer;
        sal_Char* pBuffer = sBuffer.AllocBuffer(nLen);
        aStream.Seek( 0 );
        aStream.Read((void*)pBuffer, nLen);
        rStr = String( S2U(pBuffer) );
    }

    delete pDLMedium;
    pDLMedium = 0;

    return bOK;
}

IMPL_STATIC_LINK( SfxHTMLParser, FileDownloadDone, void*, EMPTYARG )
{
    // The Download is now completed. also the Data-Available-Link
    // must or are allowed to be passed through.
    pThis->SetDownloadingFile( FALSE );

    // ... and call once, thus will continue reading.
    pThis->CallAsyncCallLink();

    return 0;
}

void SfxHTMLParser::GetScriptType_Impl( SvKeyValueIterator *pHTTPHeader )
{
    aScriptType = DEFINE_CONST_UNICODE(SVX_MACRO_LANGUAGE_JAVASCRIPT);
    eScriptType = JAVASCRIPT;
    if( pHTTPHeader )
    {
        SvKeyValue aKV;
        for( BOOL bCont = pHTTPHeader->GetFirst( aKV ); bCont;
             bCont = pHTTPHeader->GetNext( aKV ) )
        {
            if( aKV.GetKey().EqualsIgnoreCaseAscii(
                                    OOO_STRING_SVTOOLS_HTML_META_content_script_type ) )
            {
                if( aKV.GetValue().Len() )
                {
                    String aTmp( aKV.GetValue() );
                    if( aTmp.EqualsIgnoreCaseAscii( sHTML_MIME_text, 0, 5 ) )
                        aTmp.Erase( 0, 5 );
                    else if( aTmp.EqualsIgnoreCaseAscii( sHTML_MIME_application,
                                                         0, 12 ) )
                        aTmp.Erase( 0, 12 );
                    else
                        break;

                    if( aTmp.EqualsIgnoreCaseAscii( sHTML_MIME_experimental, 0,
                                                    2 ) )
                    {
                        aTmp.Erase( 0, 2 );
                    }

                    if( aTmp.EqualsIgnoreCaseAscii( OOO_STRING_SVTOOLS_HTML_LG_starbasic ) )
                    {
                        eScriptType = STARBASIC;
                        aScriptType = DEFINE_CONST_UNICODE(SVX_MACRO_LANGUAGE_STARBASIC);
                    }
                    if( !aTmp.EqualsIgnoreCaseAscii( OOO_STRING_SVTOOLS_HTML_LG_javascript ) )
                    {
                        eScriptType = EXTENDED_STYPE;
                        aScriptType = aTmp;
                    }
                }
                break;
            }
        }
    }
}

ScriptType SfxHTMLParser::GetScriptType( SvKeyValueIterator *pHTTPHeader ) const
{
    if( !aScriptType.Len() )
        ((SfxHTMLParser *)this)->GetScriptType_Impl( pHTTPHeader );

    return eScriptType;
}

const String& SfxHTMLParser::GetScriptTypeString(
                                    SvKeyValueIterator *pHTTPHeader ) const
{
    if( !aScriptType.Len() )
        ((SfxHTMLParser *)this)->GetScriptType_Impl( pHTTPHeader );

    return aScriptType;
}

double SfxHTMLParser::GetTableDataOptionsValNum( sal_uInt32& nNumForm,
        LanguageType& eNumLang, const String& aValStr, const String& aNumStr,
        SvNumberFormatter& rFormatter )
{
    LanguageType eParseLang = (LanguageType )aNumStr.ToInt32();
    sal_uInt32 nParseForm =
        rFormatter.GetFormatForLanguageIfBuiltIn( 0, eParseLang );
    double fVal;
    rFormatter.IsNumberFormat( aValStr, nParseForm, fVal );
    if ( aNumStr.GetTokenCount( ';' ) > 2 )
    {
        eNumLang = (LanguageType)aNumStr.GetToken( 1, ';' ).ToInt32();
        xub_StrLen nPos = aNumStr.Search( ';' );
        nPos = aNumStr.Search( ';', nPos + 1 );
        String aFormat( aNumStr.Copy( nPos + 1 ) );
        xub_StrLen nCheckPos;
        short nType;
        if ( eNumLang != LANGUAGE_SYSTEM )
            rFormatter.PutEntry( aFormat, nCheckPos, nType, nNumForm, eNumLang );
        else
            rFormatter.PutandConvertEntry( aFormat, nCheckPos, nType, nNumForm,
                eParseLang, eNumLang );
    }
    else
    {
        eNumLang = LANGUAGE_SYSTEM;
        nNumForm = rFormatter.GetFormatForLanguageIfBuiltIn( 0, eNumLang );
    }
    return fVal;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
