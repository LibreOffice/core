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


#include "pppoptimizertoken.hxx"

#include <sal/macros.h>
#include <unordered_map>
#include <memory>

typedef std::unordered_map< OUString, PPPOptimizerTokenEnum> TypeNameHashMap;

namespace {

struct TokenTable
{
    OUString                            pS;
    PPPOptimizerTokenEnum               pE;
};

}

constexpr TokenTable pTokenTableArray[] =
{
    { u"btnNavFinish"_ustr,       TK_btnNavFinish },

    { u"DocumentService"_ustr,    TK_DocumentService },
    { u"FileSizeDestination"_ustr,TK_FileSizeDestination },
    { u"FileSizeSource"_ustr,     TK_FileSizeSource },
    { u"FilterName"_ustr,         TK_FilterName },
    { u"Flags"_ustr,              TK_Flags },
    { u"DocumentFrame"_ustr,      TK_DocumentFrame },
    { u"DialogParentWindow"_ustr, TK_DialogParentWindow },
    { u"Name"_ustr,               TK_Name },
    { u"Progress"_ustr,           TK_Progress },
    { u"Settings"_ustr,           TK_Settings },
    { u"StatusDispatcher"_ustr,   TK_StatusDispatcher },
    { u"Type"_ustr,               TK_Type },
    { u"UIName"_ustr,             TK_UIName },

    { u"JPEGCompression"_ustr,    TK_JPEGCompression },
    { u"JPEGQuality"_ustr,        TK_JPEGQuality },
    { u"RemoveCropArea"_ustr,     TK_RemoveCropArea },
    { u"ImageResolution"_ustr,    TK_ImageResolution },
    { u"EmbedLinkedGraphics"_ustr,TK_EmbedLinkedGraphics },
    { u"OLEOptimization"_ustr,    TK_OLEOptimization },
    { u"OLEOptimizationType"_ustr,TK_OLEOptimizationType },
    { u"DeleteUnusedMasterPages"_ustr, TK_DeleteUnusedMasterPages },
    { u"DeleteHiddenSlides"_ustr, TK_DeleteHiddenSlides },
    { u"DeleteNotesPages"_ustr,   TK_DeleteNotesPages },
    { u"CustomShowName"_ustr,     TK_CustomShowName },
    { u"SaveAs"_ustr,             TK_SaveAs },
    { u"SaveAsURL"_ustr,          TK_SaveAsURL },
    { u"OpenNewDocument"_ustr,    TK_OpenNewDocument },
    { u"EstimatedFileSize"_ustr,  TK_EstimatedFileSize },

    { u"Status"_ustr,             TK_Status },
    { u"Pages"_ustr,              TK_Pages },


    { u"NotFound"_ustr,           TK_NotFound }
};

static const TypeNameHashMap& createHashMap()
{
    static TypeNameHashMap aMap = []()
    {
        TypeNameHashMap map;
        for ( const auto& rToken : pTokenTableArray )
        {
            map[ rToken.pS ] = rToken.pE;
        }
        return map;
    }();
    return aMap;
}

PPPOptimizerTokenEnum TKGet( const OUString& rToken )
{
    const TypeNameHashMap& rHashMap = createHashMap();
    PPPOptimizerTokenEnum eRetValue = TK_NotFound;
    auto aHashIter = rHashMap.find( rToken );
    if ( aHashIter != rHashMap.end() )
        eRetValue = (*aHashIter).second;
    return eRetValue;
}

OUString TKGet( const PPPOptimizerTokenEnum eToken )
{
    return pTokenTableArray[ eToken ].pS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
