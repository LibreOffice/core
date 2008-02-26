/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sfxhtml.cxx,v $
 *
 *  $Revision: 1.22 $
 *
 *  last change: $Author: obo $ $Date: 2008-02-26 15:06:53 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sfx2.hxx"

#include <tools/urlobj.hxx>

#include <sfx2/objsh.hxx>
#include <sfx2/docfile.hxx>
#include "openflag.hxx"

#include <comphelper/string.hxx>

#ifndef _HTMLKYWD_HXX
#include <svtools/htmlkywd.hxx>
#endif
#ifndef _HTMLTOKN_H
#include <svtools/htmltokn.h>
#endif
#ifndef _IMAP_HXX
#include <svtools/imap.hxx>
#endif
#ifndef _GOODIES_IMAPCIRC_HXX
#include <svtools/imapcirc.hxx>
#endif
#ifndef _GOODIES_IMAPOBJ_HXX
#include <svtools/imapobj.hxx>
#endif
#ifndef _GOODIES_IMAPPOLY_HXX
#include <svtools/imappoly.hxx>
#endif
#ifndef _GOODIES_IMAPRECT_HXX
#include <svtools/imaprect.hxx>
#endif
#ifndef _SVSTDARR_ULONGS_DECL
#define _SVSTDARR_ULONGS
#include <svtools/svstdarr.hxx>
#endif
#ifndef _ZFORLIST_HXX
#include <svtools/zforlist.hxx>
#endif
#ifndef _INETTYPE_HXX
#include <svtools/inettype.hxx>
#endif

#ifndef _RTL_TENCINFO_H
#include <rtl/tencinfo.h>
#endif
#ifndef _TOOLS_TENCCVT_HXX
#include <tools/tenccvt.hxx>
#endif

#include <sfx2/sfxhtml.hxx>

#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/beans/XPropertyContainer.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>

using namespace ::com::sun::star;


sal_Char __FAR_DATA sHTML_MIME_text[] = "text/";
sal_Char __FAR_DATA sHTML_MIME_application[] = "application/";
sal_Char __FAR_DATA sHTML_MIME_experimental[] = "x-";

#define HTML_META_NONE          0
#define HTML_META_AUTHOR        1
#define HTML_META_DESCRIPTION   2
#define HTML_META_KEYWORDS      3
#define HTML_META_REFRESH       4
#define HTML_META_CLASSIFICATION 5
#define HTML_META_CREATED       6
#define HTML_META_CHANGEDBY     7
#define HTML_META_CHANGED       8
#define HTML_META_GENERATOR     9
#define HTML_META_SDFOOTNOTE    10
#define HTML_META_SDENDNOTE     11
#define HTML_META_CONTENT_TYPE  12

// <INPUT TYPE=xxx>
#ifdef __MINGW32__ // for runtime pseudo reloc
static HTMLOptionEnum aAreaShapeOptEnums[] =
#else
static HTMLOptionEnum __READONLY_DATA aAreaShapeOptEnums[] =
#endif
{
    { sHTML_SH_rect,        IMAP_OBJ_RECTANGLE  },
    { sHTML_SH_rectangle,   IMAP_OBJ_RECTANGLE  },
    { sHTML_SH_circ,        IMAP_OBJ_CIRCLE     },
    { sHTML_SH_circle,      IMAP_OBJ_CIRCLE     },
    { sHTML_SH_poly,        IMAP_OBJ_POLYGON    },
    { sHTML_SH_polygon,     IMAP_OBJ_POLYGON    },
    { 0,                    0                   }
};

// <META NAME=xxx>
#ifdef __MINGW32__ // for runtime pseudo reloc
static HTMLOptionEnum aHTMLMetaNameTable[] =
#else
static HTMLOptionEnum __READONLY_DATA aHTMLMetaNameTable[] =
#endif
{
    { sHTML_META_author,        HTML_META_AUTHOR        },
    { sHTML_META_changed,       HTML_META_CHANGED       },
    { sHTML_META_changedby,     HTML_META_CHANGEDBY     },
    { sHTML_META_classification,HTML_META_CLASSIFICATION},
    { sHTML_META_content_type,  HTML_META_CONTENT_TYPE  },
    { sHTML_META_created,       HTML_META_CREATED       },
    { sHTML_META_description,   HTML_META_DESCRIPTION   },
    { sHTML_META_keywords,      HTML_META_KEYWORDS      },
    { sHTML_META_generator,     HTML_META_GENERATOR     },
    { sHTML_META_refresh,       HTML_META_REFRESH       },
    { sHTML_META_sdendnote,     HTML_META_SDENDNOTE     },
    { sHTML_META_sdfootnote,    HTML_META_SDFOOTNOTE    },
    { 0,                        0                       }
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
    // als default encoding.
    SetSrcEncoding( GetExtendedCompatibilityTextEncoding(  RTL_TEXTENCODING_ISO_8859_1 ) );

    // If the file starts with a BOM, switch to UCS2.
    SetSwitchToUCS2( TRUE );
}

__EXPORT SfxHTMLParser::~SfxHTMLParser()
{
    DBG_ASSERT( !pDLMedium, "Da ist ein File-Download stehengeblieben" );
    delete pDLMedium;
}

BOOL SfxHTMLParser::ParseMapOptions(ImageMap * pImageMap,
                                    const HTMLOptions * pOptions)
{
    DBG_ASSERT( pImageMap, "ParseMapOptions: keine Image-Map" );
    DBG_ASSERT( pOptions, "ParseMapOptions: keine Optionen" );

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
    DBG_ASSERT( pImageMap, "ParseAreaOptions: keine Image-Map" );
    DBG_ASSERT( pOptions, "ParseAreaOptions: keine Optionen" );

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

BOOL SfxHTMLParser::ParseMetaOptions(
        const uno::Reference<document::XDocumentProperties> & i_xDocProps,
        SvKeyValueIterator *pHTTPHeader,
        const HTMLOptions *pOptions,
        rtl_TextEncoding& rEnc )
{
    String aName, aContent;
    USHORT nAction = HTML_META_NONE;
    BOOL bHTTPEquiv = FALSE, bChanged = FALSE;

    for( USHORT i = pOptions->Count(); i; )
    {
        const HTMLOption *pOption = (*pOptions)[ --i ];
        switch( pOption->GetToken() )
        {
        case HTML_O_NAME:
            aName = pOption->GetString();
            if( HTML_META_NONE==nAction )
                pOption->GetEnum( nAction, aHTMLMetaNameTable );
            break;
        case HTML_O_HTTPEQUIV:
            aName = pOption->GetString();
            pOption->GetEnum( nAction, aHTMLMetaNameTable );
            bHTTPEquiv = TRUE;
            break;
        case HTML_O_CONTENT:
            aContent = pOption->GetString();
            break;
        }
    }

    if( bHTTPEquiv || HTML_META_DESCRIPTION!=nAction )
    {
        // wenn's keine Description ist CRs und LFs aus dem CONTENT entfernen
        aContent.EraseAllChars( _CR );
        aContent.EraseAllChars( _LF );
    }
    else
    {
        // fuer die Beschreibung die Zeilen-Umbrueche entsprechen wandeln
        aContent.ConvertLineEnd();
    }


    if( bHTTPEquiv && pHTTPHeader )
    {
        // #57232#: Netscape scheint ein abschliessendes " einfach zu
        // ignorieren, also tun wir das auch.
        if( aContent.Len() && '"' == aContent.GetChar( aContent.Len()-1 ) )
            aContent.Erase( aContent.Len() - 1 );
        SvKeyValue aKeyValue( aName, aContent );
        pHTTPHeader->Append( aKeyValue );
    }

    switch( nAction )
    {
    case HTML_META_AUTHOR:
        if (i_xDocProps.is()) {
            i_xDocProps->setAuthor( aContent );
            bChanged = TRUE;
        }
        break;
    case HTML_META_DESCRIPTION:
        if (i_xDocProps.is()) {
            i_xDocProps->setDescription( aContent );
            bChanged = TRUE;
        }
        break;
    case HTML_META_KEYWORDS:
        if (i_xDocProps.is()) {
            i_xDocProps->setKeywords(
                ::comphelper::string::convertCommaSeparated(aContent));
            bChanged = TRUE;
        }
        break;
    case HTML_META_CLASSIFICATION:
        if (i_xDocProps.is()) {
            i_xDocProps->setSubject( aContent );
            bChanged = TRUE;
        }
        break;

    case HTML_META_CHANGEDBY:
        if (i_xDocProps.is()) {
            i_xDocProps->setModifiedBy( aContent );
        }
        break;

    case HTML_META_CREATED:
    case HTML_META_CHANGED:
        if( i_xDocProps.is() && aContent.Len() && aContent.GetTokenCount()==2 )
        {
            Date aDate( (ULONG)aContent.GetToken(0).ToInt32() );
            Time aTime( (ULONG)aContent.GetToken(1).ToInt32() );
            DateTime aDateTime( aDate, aTime );
            ::util::DateTime uDT(aDateTime.Get100Sec(),
                aDateTime.GetSec(), aDateTime.GetMin(),
                aDateTime.GetHour(), aDateTime.GetDay(),
                aDateTime.GetMonth(), aDateTime.GetYear());
            if( HTML_META_CREATED==nAction )
                i_xDocProps->setCreationDate( uDT );
            else
                i_xDocProps->setModificationDate( uDT );
            bChanged = TRUE;
        }
        break;

    case HTML_META_REFRESH:
        DBG_ASSERT( !bHTTPEquiv || pHTTPHeader,
                "Reload-URL aufgrund unterlsassener MUSS-Aenderung verlorengegangen" );
        break;

    case HTML_META_CONTENT_TYPE:
        if( aContent.Len() )
            rEnc = GetEncodingByMIME( aContent );
        break;

    case HTML_META_NONE:
        if( !bHTTPEquiv )
        {
            if (i_xDocProps.is()) {
                uno::Reference<beans::XPropertyContainer> xUDProps
                    = i_xDocProps->getUserDefinedProperties();
                try {
                    xUDProps->addProperty(aName,
                        beans::PropertyAttribute::REMOVEABLE,
                        uno::makeAny(::rtl::OUString(aContent)));
                    bChanged = TRUE;
                } catch (uno::Exception &) {
                    // ignore
                }
            }
        }
        break;
    }

    return bChanged;
}

BOOL SfxHTMLParser::ParseMetaOptions(
        const uno::Reference<document::XDocumentProperties> & i_xDocProps,
                                      SvKeyValueIterator *pHeader )
{
    USHORT nContentOption = HTML_O_CONTENT;
    rtl_TextEncoding eEnc = RTL_TEXTENCODING_DONTKNOW;

    BOOL bRet = ParseMetaOptions( i_xDocProps, pHeader,
                                  GetOptions(&nContentOption),
                                  eEnc );

    // If the encoding is set by a META tag, it may only overwrite the
    // current encoding if both, the current and the new encoding, are 1-BYTE
    // encodings. Everything else cannot lead to reasonable results.
    if( RTL_TEXTENCODING_DONTKNOW != eEnc &&
        rtl_isOctetTextEncoding( eEnc ) &&
        rtl_isOctetTextEncoding( GetSrcEncoding() ) )
        SetSrcEncoding( eEnc );

    return bRet;
}


void SfxHTMLParser::StartFileDownload( const String& rURL, int nToken,
                                       SfxObjectShell *pSh )
{
    DBG_ASSERT( !pDLMedium, "StartFileDwonload bei aktivem Download" );
    if( pDLMedium )
        return;

    pDLMedium = new SfxMedium( rURL, SFX_STREAM_READONLY, FALSE );
    if( pSh )
    {
        // Medium registrieren, damit abgebrochen werden kann
        pSh->RegisterTransfer( *pDLMedium );

        // Target-Frame uebertragen, damit auch javascript:-URLs
        // "geladen" werden koennen.
        //const SfxMedium *pShMedium = pSh->GetMedium();
        //if( pShMedium )
        //  pDLMedium->SetLoadTargetFrame( pShMedium->GetLoadTargetFrame() );
    }

    // Download anstossen (Achtung: Kann auch synchron sein).
    if ( TRUE /*pMedium->GetDoneLink() == Link()*/ )
        pDLMedium->DownLoad();
    else
    {
        // Downloading-Flag auf TRUE setzen. Es werden dann auch
        // Data-Available-Links, wenn wir in den Pending-Staus gelangen.
        SetDownloadingFile( TRUE );
        pDLMedium->DownLoad( STATIC_LINK( this, SfxHTMLParser, FileDownloadDone ) );

        // Wenn das Dowsnloading-Flag noch gesetzt ist erfolgt der Download
        // asynchron. Wir gehen dann in den Pedning-Staus und warten dort.
        // Solange sind alle Aufrufe des Data-Avaialble-Link gesperrt.
        if( IsDownloadingFile() )
        {
            // Den aktuellen Zustand einfrieren und in den Pending-Status gehen.
            // Wenn der Download beendet oder abgebrochen wurde, wird ueber
            // NewDataRead ein Continue mit dem uebergeben Token angesteossen.
            SaveState( nToken );
            eState = SVPAR_PENDING;
        }
    }
}

BOOL SfxHTMLParser::GetFileDownloadMIME( String& rMIME )
{
    return pDLMedium && pDLMedium->GetErrorCode()==0 &&
           pDLMedium->GetMIMEAndRedirect(rMIME)==0;
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
        if( pStream )   // HACK wegen #65563#
            aStream << *pStream;

        aStream.Seek( STREAM_SEEK_TO_END );
        DBG_ASSERT( aStream.Tell() < STRING_MAXLEN,
                    "File zu lang fuer einen String, Ende abgeschnitten" );
        xub_StrLen nLen = aStream.Tell() < STRING_MAXLEN
                        ? (xub_StrLen)aStream.Tell()
                        : STRING_MAXLEN;

        // TODO: untested!!!
        rtl_TextEncoding eEnc =
            GetExtendedCompatibilityTextEncoding( RTL_TEXTENCODING_ISO_8859_1 );
        String sMime;
        if( pDLMedium->GetMIMEAndRedirect( sMime ) == 0 )
        {
            rtl_TextEncoding eMimeEnc = GetEncodingByMIME( sMime );
            if( RTL_TEXTENCODING_DONTKNOW != eMimeEnc )
                eEnc = eMimeEnc;
        }

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
    // Der Download ist jetzt abgeschlossen. Ausserdem muss/darf der
    // Data-Available-Link wieder durchgelassen werden.
    pThis->SetDownloadingFile( FALSE );

    // ... und einmal aufrufen, damit weitergelesen wird.
    pThis->CallAsyncCallLink();

    return 0;
}

rtl_TextEncoding SfxHTMLParser::GetEncodingByMIME( const String& rMime )
{
    ByteString sType;
    ByteString sSubType;
    INetContentTypeParameterList aParameters;
    ByteString sMime( rMime, RTL_TEXTENCODING_ASCII_US );
    if (INetContentTypes::parse(sMime, sType, sSubType, &aParameters))
    {
        const INetContentTypeParameter * pCharset
            = aParameters.find("charset");
        if (pCharset != 0)
        {
            ByteString sValue( pCharset->m_sValue, RTL_TEXTENCODING_ASCII_US );
            return GetExtendedCompatibilityTextEncoding(rtl_getTextEncodingFromMimeCharset( sValue.GetBuffer() ) );
        }
    }
    return RTL_TEXTENCODING_DONTKNOW;
}
rtl_TextEncoding SfxHTMLParser::GetEncodingByHttpHeader( SvKeyValueIterator *pHTTPHeader )
{
    rtl_TextEncoding eRet = RTL_TEXTENCODING_DONTKNOW;
    if( pHTTPHeader )
    {
        SvKeyValue aKV;
        for( BOOL bCont = pHTTPHeader->GetFirst( aKV ); bCont;
             bCont = pHTTPHeader->GetNext( aKV ) )
        {
            if( aKV.GetKey().EqualsIgnoreCaseAscii( sHTML_META_content_type ) )
            {
                if( aKV.GetValue().Len() )
                {
                    eRet = SfxHTMLParser::GetEncodingByMIME( aKV.GetValue() );
                }
            }
        }
    }
    return eRet;
}

BOOL SfxHTMLParser::SetEncodingByHTTPHeader(
                                SvKeyValueIterator *pHTTPHeader )
{
    BOOL bRet = FALSE;
    rtl_TextEncoding eEnc = SfxHTMLParser::GetEncodingByHttpHeader( pHTTPHeader );
    if(RTL_TEXTENCODING_DONTKNOW != eEnc)
    {
        SetSrcEncoding( eEnc );
        bRet = TRUE;
    }
    return bRet;
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
                                    sHTML_META_content_script_type ) )
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

                    if( aTmp.EqualsIgnoreCaseAscii( sHTML_LG_starbasic ) )
                    {
                        eScriptType = STARBASIC;
                        aScriptType = DEFINE_CONST_UNICODE(SVX_MACRO_LANGUAGE_STARBASIC);
                    }
                    if( !aTmp.EqualsIgnoreCaseAscii( sHTML_LG_javascript ) )
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

