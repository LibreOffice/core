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
#include <svl/zforlist.hxx>
#include <rtl/tencinfo.h>
#include <tools/tenccvt.hxx>

#include <sfx2/sfxhtml.hxx>

#include <com/sun/star/beans/XPropertyContainer.hpp>
#include <comphelper/string.hxx>

#include <vector>


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

SfxHTMLParser::SfxHTMLParser( SvStream& rStream, sal_Bool bIsNewDoc,
                              SfxMedium *pMed ) :
    HTMLParser( rStream, bIsNewDoc ),
    pMedium( pMed ), pDLMedium( 0 )
{
    DBG_ASSERT( RTL_TEXTENCODING_UTF8 == GetSrcEncoding( ),
                "SfxHTMLParser::SfxHTMLParser: From where comes ZS?" );

    DBG_ASSERT( !IsSwitchToUCS2(),
                "SfxHTMLParser::SfxHTMLParser: Switch to UCS2?" );

    // If the file starts with a BOM, switch to UCS2.
    SetSwitchToUCS2( sal_True );
}

SfxHTMLParser::~SfxHTMLParser()
{
    DBG_ASSERT( !pDLMedium, "Here is a File Download that has got stuck" );
    delete pDLMedium;
}

bool SfxHTMLParser::ParseMapOptions(
    ImageMap* pImageMap, const HTMLOptions& rOptions)
{
    DBG_ASSERT( pImageMap, "ParseMapOptions: No Image-Map" );

    String aName;

    for (size_t i = rOptions.size(); i; )
    {
        const HTMLOption& aOption = rOptions[--i];
        switch( aOption.GetToken() )
        {
        case HTML_O_NAME:
            aName = aOption.GetString();
            break;
        }
    }

    if( aName.Len() )
        pImageMap->SetName( aName );

    return aName.Len() > 0;
}

bool SfxHTMLParser::ParseAreaOptions(ImageMap * pImageMap, const String& rBaseURL,
                                     const HTMLOptions& rOptions,
                                     sal_uInt16 nEventMouseOver,
                                     sal_uInt16 nEventMouseOut )
{
    DBG_ASSERT( pImageMap, "ParseAreaOptions: no Image-Map" );

    sal_uInt16 nShape = IMAP_OBJ_RECTANGLE;
    std::vector<sal_uInt32> aCoords;
    String aName, aHRef, aAlt, aTarget, sEmpty;
    sal_Bool bNoHRef = sal_False;
    SvxMacroTableDtor aMacroTbl;

    for (size_t i = rOptions.size(); i; )
    {
        sal_uInt16 nEvent = 0;
        ScriptType eScrpType = STARBASIC;
        const HTMLOption& rOption = rOptions[--i];
        switch( rOption.GetToken() )
        {
        case HTML_O_NAME:
            aName = rOption.GetString();
            break;
        case HTML_O_SHAPE:
            rOption.GetEnum( nShape, aAreaShapeOptEnums );
            break;
        case HTML_O_COORDS:
            rOption.GetNumbers( aCoords, true );
            break;
        case HTML_O_HREF:
            aHRef = INetURLObject::GetAbsURL( rBaseURL, rOption.GetString() );
            break;
        case HTML_O_NOHREF:
            bNoHRef = sal_True;
            break;
        case HTML_O_ALT:
            aAlt = rOption.GetString();
            break;
        case HTML_O_TARGET:
            aTarget = rOption.GetString();
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
                String sTmp( rOption.GetString() );
                if( sTmp.Len() )
                {
                    sTmp = convertLineEnd(sTmp, GetSystemLineEnd());
                    aMacroTbl.Insert( nEvent, SvxMacro( sTmp, sEmpty, eScrpType ));
                }
            }
            break;
        }
    }

    if( bNoHRef )
        aHRef.Erase();

    sal_Bool bNewArea = sal_True;
    switch( nShape )
    {
    case IMAP_OBJ_RECTANGLE:
        if( aCoords.size() >=4 )
        {
            Rectangle aRec( aCoords[0], aCoords[1],
                            aCoords[2], aCoords[3] );
            IMapRectangleObject aMapRObj( aRec, aHRef, aAlt, String(), aTarget, aName,
                                          !bNoHRef );
            if( !aMacroTbl.empty() )
                aMapRObj.SetMacroTable( aMacroTbl );
            pImageMap->InsertIMapObject( aMapRObj );
        }
        break;
    case IMAP_OBJ_CIRCLE:
        if( aCoords.size() >=3 )
        {
            Point aPoint( aCoords[0], aCoords[1] );
            IMapCircleObject aMapCObj( aPoint, aCoords[2],aHRef, aAlt, String(),
                                       aTarget, aName, !bNoHRef );
            if( !aMacroTbl.empty() )
                aMapCObj.SetMacroTable( aMacroTbl );
            pImageMap->InsertIMapObject( aMapCObj );
        }
        break;
    case IMAP_OBJ_POLYGON:
        if( aCoords.size() >=6 )
        {
            sal_uInt16 nCount = aCoords.size() / 2;
            Polygon aPoly( nCount );
            for( sal_uInt16 i=0; i<nCount; i++ )
                aPoly[i] = Point( aCoords[2*i], aCoords[2*i+1] );
            IMapPolygonObject aMapPObj( aPoly, aHRef, aAlt, String(), aTarget, aName,
                                        !bNoHRef );
            if( !aMacroTbl.empty() )
                aMapPObj.SetMacroTable( aMacroTbl );
            pImageMap->InsertIMapObject( aMapPObj );
        }
        break;
    default:
        bNewArea = sal_False;
    }

    return bNewArea;
}

void SfxHTMLParser::StartFileDownload(const OUString& rURL)
{
    DBG_ASSERT( !pDLMedium, "StartFileDownload when active Download" );
    if( pDLMedium )
        return;

    pDLMedium = new SfxMedium( rURL, SFX_STREAM_READONLY );
    pDLMedium->DownLoad();
}

sal_Bool SfxHTMLParser::FinishFileDownload( String& rStr )
{
    String aStr;

    sal_Bool bOK = pDLMedium && pDLMedium->GetErrorCode()==0;
    if( bOK )
    {
        SvStream* pStream = pDLMedium->GetInStream();
        DBG_ASSERT( pStream, "No In-Stream received from Medium" );

        SvMemoryStream aStream;
        if( pStream )
            aStream << *pStream;

        aStream.Seek( STREAM_SEEK_TO_END );
        DBG_ASSERT( aStream.Tell() < STRING_MAXLEN,
                    "File too long for a string, cut off the end" );
        xub_StrLen nLen = aStream.Tell() < STRING_MAXLEN
                        ? (xub_StrLen)aStream.Tell()
                        : STRING_MAXLEN;

        aStream.Seek( 0 );
        rtl::OString sBuffer = read_uInt8s_ToOString(aStream, nLen);
        rStr = S2U(sBuffer);
    }

    delete pDLMedium;
    pDLMedium = 0;

    return bOK;
}

void SfxHTMLParser::GetScriptType_Impl( SvKeyValueIterator *pHTTPHeader )
{
    aScriptType = DEFINE_CONST_UNICODE(SVX_MACRO_LANGUAGE_JAVASCRIPT);
    eScriptType = JAVASCRIPT;
    if( pHTTPHeader )
    {
        SvKeyValue aKV;
        for( sal_Bool bCont = pHTTPHeader->GetFirst( aKV ); bCont;
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
    if ( comphelper::string::getTokenCount(aNumStr, ';') > 2 )
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
