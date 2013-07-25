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

#ifndef _SFXHTML_HXX
#define _SFXHTML_HXX

#include "sal/config.h"
#include "sfx2/dllapi.h"
#include "sal/types.h"
#include <i18nlangtag/lang.h>
#include <svtools/parhtml.hxx>
#include <svl/macitem.hxx>


class ImageMap;
class SfxMedium;
class SfxObjectShell;

class SFX2_DLLPUBLIC SfxHTMLParser : public HTMLParser
{
    String                  aScriptType;

    SfxMedium*              pMedium;
    SfxMedium *pDLMedium;   // Medium for Download Files

    ScriptType eScriptType;

    SAL_DLLPRIVATE void GetScriptType_Impl( SvKeyValueIterator* );

protected:

    SfxHTMLParser( SvStream& rStream, sal_Bool bNewDoc=sal_True, SfxMedium *pMedium=0 );

    virtual ~SfxHTMLParser();

public:
    // Read the options of an image map
    // <MAP>: sal_True = Image-Map has a name
    // <AREA>: sal_True = Image-Map has now one Region more
    static bool ParseMapOptions(ImageMap* pImageMap, const HTMLOptions& rOptions);
    bool ParseMapOptions(ImageMap * pImageMap)
    { return ParseMapOptions(pImageMap, GetOptions()); }
    static bool ParseAreaOptions(ImageMap * pImageMap, const String& rBaseURL,
                                 const HTMLOptions& rOptions,
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
    // In the synchronous case, the parser is in the working state after
    // it has been called. The read file can then be picked up directly with
    // FinishFileDownload. In the asynchronous case, the parser is in the
    // pending state after it hs been called. The parser then has to leave
    // over the Continue (without Reschedule!). If the file is loaded,
    // a Continue is called with passed on token. The file can then be picked
    // up by FinishFileDownload. To cancel the download should a shell be
    // left. It can only exist a single download at the same time, For every
    // started download FinshFileDownload must be called.
    void StartFileDownload(const OUString& rURL);

    // End of an asynchronous file download. Returns TRUE if the download
    // was successful. The read file is then passed into String.
    sal_Bool FinishFileDownload( OUString& rStr );

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
