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

#ifndef _SFXHTML_HXX
#define _SFXHTML_HXX

#include "sal/config.h"
#include "sfx2/dllapi.h"
#include "sal/types.h"
#include <i18npool/lang.h>
#include <svtools/parhtml.hxx>
#include <svl/macitem.hxx>


class ImageMap;
class SfxMedium;
class SfxObjectShell;

class SFX2_DLLPUBLIC SfxHTMLParser : public HTMLParser
{
    DECL_DLLPRIVATE_STATIC_LINK( SfxHTMLParser, FileDownloadDone, void* );

    String                  aScriptType;

    SfxMedium*              pMedium;
    SfxMedium *pDLMedium;   // Medium for Download Files

    sal_uInt16 nMetaTags;       // Number of previously read Meta-Tags
    ScriptType eScriptType;

    SAL_DLLPRIVATE void GetScriptType_Impl( SvKeyValueIterator* );

protected:

    SfxHTMLParser( SvStream& rStream, sal_Bool bNewDoc=sal_True, SfxMedium *pMedium=0 );

    virtual ~SfxHTMLParser();

public:
    // Read the options of an image map
    // <MAP>: sal_True = Image-Map has a name
    // <AREA>: sal_True = Image-Map has now one Region more
    static sal_Bool ParseMapOptions(ImageMap * pImageMap,
                                const HTMLOptions * pOptions );
    sal_Bool ParseMapOptions(ImageMap * pImageMap)
    { return ParseMapOptions(pImageMap, GetOptions()); }
    static sal_Bool ParseAreaOptions(ImageMap * pImageMap, const String& rBaseURL,
                                 const HTMLOptions * pOptions,
                                 sal_uInt16 nEventMouseOver = 0,
                                 sal_uInt16 nEventMouseOut = 0 );
    inline sal_Bool ParseAreaOptions(ImageMap * pImageMap, const String& rBaseURL,
                                 sal_uInt16 nEventMouseOver = 0,
                                 sal_uInt16 nEventMouseOut = 0);

    // <TD SDVAL="..." SDNUM="...">
    static double GetTableDataOptionsValNum( sal_uInt32& nNumForm,
            LanguageType& eNumLang, const String& aValStr,
            const String& aNumStr, SvNumberFormatter& rFormatter );

protected:
    // Start a file download. This is done asynchronously or synchronously.
    // In the synchronous case, the parser is in the the working state after
    // it has been called. The read file can then be picked up directly with
    // FinishFileDownload. In the asynchronous case, the parser is in the
    // pending state after it hs been called. The parser then has to leave
    // over the Continue (without Reschedule!). If the file is loaded,
    // a Continue is called with passed on token. The file can then be picked
    // up by FinishFileDownload. To cancel the download should a shell be
    // left. It can only exist a single download at the same time, For every
    // started download FinshFileDownload must be called.
    void StartFileDownload( const String& rURL, int nToken,
                            SfxObjectShell *pSh=0 );

    // End of an asynchronous file download. Returns TRUE if the download
    // was successful. The read file is then passed into String.
    sal_Bool FinishFileDownload( String& rStr );

    // Returns TRUE if a file was downloaded and if FileDownloadFinished
    // has not yet been called
    sal_Bool ShouldFinishFileDownload() const { return pDLMedium != 0; }

    SfxMedium *GetMedium() { return pMedium; }
    const SfxMedium *GetMedium() const { return pMedium; }

    // Default (without iterator) is JavaScript
    ScriptType GetScriptType( SvKeyValueIterator* ) const;
    const String& GetScriptTypeString( SvKeyValueIterator* ) const;
};

inline sal_Bool SfxHTMLParser::ParseAreaOptions(ImageMap * pImageMap, const String& rBaseURL,
                                            sal_uInt16 nEventMouseOver,
                                            sal_uInt16 nEventMouseOut)
{
    return ParseAreaOptions( pImageMap, rBaseURL, GetOptions(),
                             nEventMouseOver, nEventMouseOut );
}


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
