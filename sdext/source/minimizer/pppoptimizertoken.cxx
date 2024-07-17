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

typedef std::unordered_map< const char*, PPPOptimizerTokenEnum, rtl::CStringHash, rtl::CStringEqual> TypeNameHashMap;

namespace {

struct TokenTable
{
    const char*                         pS;
    PPPOptimizerTokenEnum               pE;
};

}

const TokenTable pTokenTableArray[] =
{
    { "btnNavFinish",       TK_btnNavFinish },

    { "DocumentService",    TK_DocumentService },
    { "FileSizeDestination",TK_FileSizeDestination },
    { "FileSizeSource",     TK_FileSizeSource },
    { "FilterName",         TK_FilterName },
    { "Flags",              TK_Flags },
    { "DocumentFrame",      TK_DocumentFrame },
    { "DialogParentWindow", TK_DialogParentWindow },
    { "Name",               TK_Name },
    { "Progress",           TK_Progress },
    { "Settings",           TK_Settings },
    { "StatusDispatcher",   TK_StatusDispatcher },
    { "Type",               TK_Type },
    { "UIName",             TK_UIName },

    { "JPEGCompression",    TK_JPEGCompression },
    { "JPEGQuality",        TK_JPEGQuality },
    { "RemoveCropArea",     TK_RemoveCropArea },
    { "ImageResolution",    TK_ImageResolution },
    { "EmbedLinkedGraphics",TK_EmbedLinkedGraphics },
    { "OLEOptimization",    TK_OLEOptimization },
    { "OLEOptimizationType",TK_OLEOptimizationType },
    { "DeleteUnusedMasterPages", TK_DeleteUnusedMasterPages },
    { "DeleteHiddenSlides", TK_DeleteHiddenSlides },
    { "DeleteNotesPages",   TK_DeleteNotesPages },
    { "CustomShowName",     TK_CustomShowName },
    { "SaveAs",             TK_SaveAs },
    { "SaveAsURL",          TK_SaveAsURL },
    { "OpenNewDocument",    TK_OpenNewDocument },
    { "EstimatedFileSize",  TK_EstimatedFileSize },

    { "Status",             TK_Status },
    { "Pages",              TK_Pages },

    { "STR_SUN_OPTIMIZATION_WIZARD2",STR_SUN_OPTIMIZATION_WIZARD2 },
    { "STR_INTRODUCTION",           STR_INTRODUCTION },
    { "STR_IMAGE_OPTIMIZATION",     STR_IMAGE_OPTIMIZATION },
    { "STR_OLE_OBJECTS",            STR_OLE_OBJECTS },
    { "STR_OLE_OBJECTS_DESC",       STR_OLE_OBJECTS_DESC },
    { "STR_NO_OLE_OBJECTS_DESC",    STR_NO_OLE_OBJECTS_DESC },
    { "STR_SLIDES",                 STR_SLIDES },
    { "STR_SUMMARY",                STR_SUMMARY },
    { "STR_DELETE_SLIDES",          STR_DELETE_SLIDES },
    { "STR_OPTIMIZE_IMAGES",        STR_OPTIMIZE_IMAGES },
    { "STR_CREATE_REPLACEMENT",     STR_CREATE_REPLACEMENT },
    { "MY_SETTINGS",                STR_MY_SETTINGS },
    { "STR_OK",                     STR_OK },
    { "STR_INFO_PRIMARY",           STR_INFO_PRIMARY },
    { "STR_INFO_SECONDARY_1",       STR_INFO_SECONDARY_1 },
    { "STR_INFO_SECONDARY_2",       STR_INFO_SECONDARY_2 },
    { "STR_INFO_SECONDARY_3",       STR_INFO_SECONDARY_3 },
    { "STR_INFO_SECONDARY_4",       STR_INFO_SECONDARY_4 },
    { "STR_DUPLICATING_PRESENTATION",STR_DUPLICATING_PRESENTATION },
    { "STR_DELETING_SLIDES",        STR_DELETING_SLIDES },
    { "STR_OPTIMIZING_GRAPHICS",    STR_OPTIMIZING_GRAPHICS },
    { "STR_CREATING_OLE_REPLACEMENTS",STR_CREATING_OLE_REPLACEMENTS },
    { "STR_FileSizeSeparator",      STR_FILESIZESEPARATOR },
    { "STR_FILENAME_SUFFIX",        STR_FILENAME_SUFFIX },
    { "STR_WARN_UNSAVED_PRESENTATION", STR_WARN_UNSAVED_PRESENTATION },


    { "NotFound",           TK_NotFound }
};

static TypeNameHashMap* createHashMap()
{
    TypeNameHashMap* pH = new TypeNameHashMap;
    const TokenTable* pPtr = pTokenTableArray;
    const TokenTable* pEnd = pPtr + SAL_N_ELEMENTS( pTokenTableArray );
    for ( ; pPtr < pEnd; pPtr++ )
        (*pH)[ pPtr->pS ] = pPtr->pE;
    return pH;
}

PPPOptimizerTokenEnum TKGet( std::u16string_view rToken )
{
    static TypeNameHashMap* pHashMap = createHashMap();
    PPPOptimizerTokenEnum eRetValue = TK_NotFound;
    size_t i, nLen = rToken.size();
    std::unique_ptr<char[]> pBuf(new char[ nLen + 1 ]);
    for ( i = 0; i < nLen; i++ )
        pBuf[ i ] = static_cast<char>(rToken[ i ]);
    pBuf[ i ] = 0;
    TypeNameHashMap::iterator aHashIter( pHashMap->find( pBuf.get() ) );
    if ( aHashIter != pHashMap->end() )
        eRetValue = (*aHashIter).second;
    return eRetValue;
}

OUString TKGet( const PPPOptimizerTokenEnum eToken )
{
    return OUString::createFromAscii( pTokenTableArray[ eToken ].pS );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
