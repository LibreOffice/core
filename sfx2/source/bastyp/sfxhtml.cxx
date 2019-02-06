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
#include <tools/debug.hxx>

#include <sfx2/objsh.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/event.hxx>
#include <openflag.hxx>

#include <svtools/htmlkywd.hxx>
#include <svtools/htmltokn.h>
#include <vcl/imap.hxx>
#include <vcl/imapcirc.hxx>
#include <vcl/imapobj.hxx>
#include <vcl/imappoly.hxx>
#include <vcl/imaprect.hxx>
#include <svl/zforlist.hxx>
#include <rtl/tencinfo.h>
#include <tools/tenccvt.hxx>

#include <sfx2/sfxhtml.hxx>

#include <com/sun/star/beans/XPropertyContainer.hpp>
#include <comphelper/string.hxx>

#include <vector>


using namespace ::com::sun::star;


// <INPUT TYPE=xxx>
static HTMLOptionEnum<sal_uInt16> const aAreaShapeOptEnums[] =
{
    { OOO_STRING_SVTOOLS_HTML_SH_rect,      IMAP_OBJ_RECTANGLE  },
    { OOO_STRING_SVTOOLS_HTML_SH_rectangle, IMAP_OBJ_RECTANGLE  },
    { OOO_STRING_SVTOOLS_HTML_SH_circ,      IMAP_OBJ_CIRCLE     },
    { OOO_STRING_SVTOOLS_HTML_SH_circle,    IMAP_OBJ_CIRCLE     },
    { OOO_STRING_SVTOOLS_HTML_SH_poly,      IMAP_OBJ_POLYGON    },
    { OOO_STRING_SVTOOLS_HTML_SH_polygon,   IMAP_OBJ_POLYGON    },
    { nullptr,                              0                   }
};

SfxHTMLParser::SfxHTMLParser( SvStream& rStream, bool bIsNewDoc,
                              SfxMedium *pMed )
    : HTMLParser(rStream, bIsNewDoc)
    , pMedium(pMed)
    , eScriptType(STARBASIC)
{
    DBG_ASSERT( RTL_TEXTENCODING_UTF8 == GetSrcEncoding( ),
                "SfxHTMLParser::SfxHTMLParser: From where comes ZS?" );

    DBG_ASSERT( !IsSwitchToUCS2(),
                "SfxHTMLParser::SfxHTMLParser: Switch to UCS2?" );

    // If the file starts with a BOM, switch to UCS2.
    SetSwitchToUCS2( true );
}

SfxHTMLParser::~SfxHTMLParser()
{
    DBG_ASSERT( !pDLMedium, "Here is a File Download that has got stuck" );
}

bool SfxHTMLParser::ParseMapOptions(
    ImageMap* pImageMap, const HTMLOptions& rOptions)
{
    DBG_ASSERT( pImageMap, "ParseMapOptions: No Image-Map" );

    OUString aName;

    for (size_t i = rOptions.size(); i; )
    {
        const HTMLOption& aOption = rOptions[--i];
        if ( aOption.GetToken() == HtmlOptionId::NAME )
            aName = aOption.GetString();
    }

    if( !aName.isEmpty() )
        pImageMap->SetName( aName );

    return !aName.isEmpty();
}

bool SfxHTMLParser::ParseAreaOptions(ImageMap * pImageMap, const OUString& rBaseURL,
                                     const HTMLOptions& rOptions,
                                     SvMacroItemId nEventMouseOver,
                                     SvMacroItemId nEventMouseOut )
{
    DBG_ASSERT( pImageMap, "ParseAreaOptions: no Image-Map" );

    sal_uInt16 nShape = IMAP_OBJ_RECTANGLE;
    std::vector<sal_uInt32> aCoords;
    OUString aName, aHRef, aAlt, aTarget;
    bool bNoHRef = false;
    SvxMacroTableDtor aMacroTbl;

    for (size_t i = rOptions.size(); i; )
    {
        SvMacroItemId nEvent = SvMacroItemId::NONE;
        ScriptType eScrpType = STARBASIC;
        const HTMLOption& rOption = rOptions[--i];
        switch( rOption.GetToken() )
        {
        case HtmlOptionId::NAME:
            aName = rOption.GetString();
            break;
        case HtmlOptionId::SHAPE:
            rOption.GetEnum( nShape, aAreaShapeOptEnums );
            break;
        case HtmlOptionId::COORDS:
            rOption.GetNumbers( aCoords );
            break;
        case HtmlOptionId::HREF:
            aHRef = INetURLObject::GetAbsURL( rBaseURL, rOption.GetString() );
            break;
        case HtmlOptionId::NOHREF:
            bNoHRef = true;
            break;
        case HtmlOptionId::ALT:
            aAlt = rOption.GetString();
            break;
        case HtmlOptionId::TARGET:
            aTarget = rOption.GetString();
            break;

        case HtmlOptionId::ONMOUSEOVER:
            eScrpType = JAVASCRIPT;
            [[fallthrough]];
        case HtmlOptionId::SDONMOUSEOVER:
            nEvent = nEventMouseOver;
            goto IMAPOBJ_SETEVENT;

        case HtmlOptionId::ONMOUSEOUT:
            eScrpType = JAVASCRIPT;
            [[fallthrough]];
        case HtmlOptionId::SDONMOUSEOUT:
            nEvent = nEventMouseOut;
            goto IMAPOBJ_SETEVENT;
IMAPOBJ_SETEVENT:
            if( nEvent != SvMacroItemId::NONE)
            {
                OUString sTmp( rOption.GetString() );
                if( !sTmp.isEmpty() )
                {
                    sTmp = convertLineEnd(sTmp, GetSystemLineEnd());
                    aMacroTbl.Insert( nEvent, SvxMacro( sTmp, "", eScrpType ));
                }
            }
            break;
        default: break;
        }
    }

    if( bNoHRef )
        aHRef.clear();

    bool bNewArea = true;
    switch( nShape )
    {
    case IMAP_OBJ_RECTANGLE:
        if( aCoords.size() >=4 )
        {
            tools::Rectangle aRect( aCoords[0], aCoords[1],
                             aCoords[2], aCoords[3] );
            std::unique_ptr<IMapRectangleObject> pMapRObj( new IMapRectangleObject(aRect, aHRef, aAlt, OUString(), aTarget, aName,
                                          !bNoHRef ));
            if( !aMacroTbl.empty() )
                pMapRObj->SetMacroTable( aMacroTbl );
            pImageMap->InsertIMapObject( std::move(pMapRObj) );
        }
        break;
    case IMAP_OBJ_CIRCLE:
        if( aCoords.size() >=3 )
        {
            Point aPoint( aCoords[0], aCoords[1] );
            std::unique_ptr<IMapCircleObject> pMapCObj(new IMapCircleObject(aPoint, aCoords[2],aHRef, aAlt, OUString(),
                                       aTarget, aName, !bNoHRef ));
            if( !aMacroTbl.empty() )
                pMapCObj->SetMacroTable( aMacroTbl );
            pImageMap->InsertIMapObject( std::move(pMapCObj) );
        }
        break;
    case IMAP_OBJ_POLYGON:
        if( aCoords.size() >=6 )
        {
            sal_uInt16 nCount = aCoords.size() / 2;
            tools::Polygon aPoly( nCount );
            for( sal_uInt16 i=0; i<nCount; i++ )
                aPoly[i] = Point( aCoords[2*i], aCoords[2*i+1] );
            std::unique_ptr<IMapPolygonObject> pMapPObj(new IMapPolygonObject( aPoly, aHRef, aAlt, OUString(), aTarget, aName,
                                        !bNoHRef ));
            if( !aMacroTbl.empty() )
                pMapPObj->SetMacroTable( aMacroTbl );
            pImageMap->InsertIMapObject( std::move(pMapPObj) );
        }
        break;
    default:
        bNewArea = false;
    }

    return bNewArea;
}

void SfxHTMLParser::StartFileDownload(const OUString& rURL)
{
    DBG_ASSERT( !pDLMedium, "StartFileDownload when active Download" );
    if( pDLMedium )
        return;

    pDLMedium.reset( new SfxMedium( rURL, SFX_STREAM_READONLY ) );
    pDLMedium->Download();
}

bool SfxHTMLParser::FinishFileDownload( OUString& rStr )
{
    bool bOK = pDLMedium && pDLMedium->GetErrorCode() == ERRCODE_NONE;
    if( bOK )
    {
        SvStream* pStream = pDLMedium->GetInStream();
        DBG_ASSERT( pStream, "No In-Stream received from Medium" );

        SvMemoryStream aStream;
        if( pStream )
            aStream.WriteStream( *pStream );

        sal_uInt64 const nLen = aStream.TellEnd();
        aStream.Seek( 0 );
        rStr = read_uInt8s_ToOUString(aStream, nLen, RTL_TEXTENCODING_UTF8);
    }

    pDLMedium.reset();

    return bOK;
}

void SfxHTMLParser::GetScriptType_Impl( SvKeyValueIterator *pHTTPHeader )
{
    aScriptType = SVX_MACRO_LANGUAGE_JAVASCRIPT;
    eScriptType = JAVASCRIPT;
    if( !pHTTPHeader )
        return;

    SvKeyValue aKV;
    for( bool bCont = pHTTPHeader->GetFirst( aKV ); bCont;
         bCont = pHTTPHeader->GetNext( aKV ) )
    {
        if( aKV.GetKey().equalsIgnoreAsciiCase(
                                OOO_STRING_SVTOOLS_HTML_META_content_script_type ) )
        {
            if( !aKV.GetValue().isEmpty() )
            {
                OUString aTmp( aKV.GetValue() );
                if( aTmp.startsWithIgnoreAsciiCase( "text/" ) )
                    aTmp = aTmp.copy( 5 );
                else if( aTmp.startsWithIgnoreAsciiCase( "application/" ) )
                    aTmp = aTmp.copy( 12 );
                else
                    break;

                if( aTmp.startsWithIgnoreAsciiCase( "x-" ) ) // MIME-experimental
                {
                    aTmp = aTmp.copy( 2 );
                }

                if( aTmp.equalsIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_LG_starbasic ) )
                {
                    eScriptType = STARBASIC;
                    aScriptType = SVX_MACRO_LANGUAGE_STARBASIC;
                }
                if( !aTmp.equalsIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_LG_javascript ) )
                {
                    eScriptType = EXTENDED_STYPE;
                    aScriptType = aTmp;
                }
            }
            break;
        }
    }
}

ScriptType SfxHTMLParser::GetScriptType( SvKeyValueIterator *pHTTPHeader ) const
{
    if( aScriptType.isEmpty() )
        const_cast<SfxHTMLParser *>(this)->GetScriptType_Impl( pHTTPHeader );

    return eScriptType;
}

const OUString& SfxHTMLParser::GetScriptTypeString(
                                    SvKeyValueIterator *pHTTPHeader ) const
{
    if( aScriptType.isEmpty() )
        const_cast<SfxHTMLParser *>(this)->GetScriptType_Impl( pHTTPHeader );

    return aScriptType;
}

double SfxHTMLParser::GetTableDataOptionsValNum( sal_uInt32& nNumForm,
        LanguageType& eNumLang, const OUString& aValStr, const OUString& aNumStr,
        SvNumberFormatter& rFormatter )
{
    LanguageType eParseLang(aNumStr.toInt32());
    sal_uInt32 nParseForm = rFormatter.GetFormatForLanguageIfBuiltIn( 0, eParseLang );
    double fVal;
    (void)rFormatter.IsNumberFormat(aValStr, nParseForm, fVal);
    if ( comphelper::string::getTokenCount(aNumStr, ';') > 2 )
    {
        sal_Int32 nIdx {0};
        eNumLang = LanguageType(aNumStr.getToken( 1, ';', nIdx ).toInt32());
        OUString aFormat( aNumStr.copy( nIdx ) );
        sal_Int32 nCheckPos;
        SvNumFormatType nType;
        if ( eNumLang != LANGUAGE_SYSTEM )
            rFormatter.PutEntry( aFormat, nCheckPos, nType, nNumForm, eNumLang );
        else
            rFormatter.PutandConvertEntry( aFormat, nCheckPos, nType, nNumForm,
                                           eParseLang, eNumLang, true);
    }
    else
    {
        eNumLang = LANGUAGE_SYSTEM;
        nNumForm = rFormatter.GetFormatForLanguageIfBuiltIn( 0, eNumLang );
    }
    return fVal;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
