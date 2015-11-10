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
#include <osl/mutex.hxx>
#include <sal/macros.h>
#include <string.h>
#include <unordered_map>
#include <memory>

typedef std::unordered_map< const char*, PPPOptimizerTokenEnum, rtl::CStringHash, rtl::CStringEqual> TypeNameHashMap;
static TypeNameHashMap* pHashMap = nullptr;
static ::osl::Mutex& getHashMapMutex()
{
    static osl::Mutex s_aHashMapProtection;
    return s_aHashMapProtection;
}

struct TokenTable
{
    const char*                         pS;
    PPPOptimizerTokenEnum               pE;
};

static const TokenTable pTokenTableArray[] =
{
    { "rdmNavi",            TK_rdmNavi },
    { "btnNavBack",         TK_btnNavBack },
    { "btnNavNext",         TK_btnNavNext },
    { "btnNavFinish",       TK_btnNavFinish },
    { "btnNavCancel",       TK_btnNavCancel },

    { "Button0Pg0",         TK_Button0Pg0 },
    { "RadioButton0Pg1",    TK_RadioButton0Pg1 },
    { "RadioButton1Pg1",    TK_RadioButton1Pg1 },
    { "CheckBox1Pg1",       TK_CheckBox1Pg1 },
    { "CheckBox2Pg1",       TK_CheckBox2Pg1 },
    { "CheckBox0Pg2",       TK_CheckBox0Pg2 },
    { "RadioButton0Pg2",    TK_RadioButton0Pg2 },
    { "RadioButton1Pg2",    TK_RadioButton1Pg2 },
    { "CheckBox0Pg3",       TK_CheckBox0Pg3 },
    { "CheckBox1Pg3",       TK_CheckBox1Pg3 },
    { "CheckBox2Pg3",       TK_CheckBox2Pg3 },
    { "CheckBox3Pg3",       TK_CheckBox3Pg3 },
    { "RadioButton0Pg4",    TK_RadioButton0Pg4 },
    { "RadioButton1Pg4",    TK_RadioButton1Pg4 },
    { "CheckBox1Pg4",       TK_CheckBox1Pg4 },

    { "DocumentService",    TK_DocumentService },
    { "FileSizeDestination",TK_FileSizeDestination },
    { "FileSizeSource",     TK_FileSizeSource },
    { "FilterName",         TK_FilterName },
    { "Flags",              TK_Flags },
    { "InformationDialog",  TK_InformationDialog },
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
    { "STR_STEPS",                  STR_STEPS },
    { "STR_BACK",                   STR_BACK },
    { "STR_NEXT",                   STR_NEXT },
    { "STR_FINISH",                 STR_FINISH },
    { "STR_CANCEL",                 STR_CANCEL },
    { "STR_INTRODUCTION",           STR_INTRODUCTION },
    { "STR_INTRODUCTION_T",         STR_INTRODUCTION_T },
    { "STR_CHOSE_SETTINGS",         STR_CHOSE_SETTINGS },
    { "STR_REMOVE",                 STR_REMOVE },
    { "STR_GRAPHIC_OPTIMIZATION",   STR_GRAPHIC_OPTIMIZATION },
    { "STR_IMAGE_OPTIMIZATION",     STR_IMAGE_OPTIMIZATION },
    { "STR_LOSSLESS_COMPRESSION",   STR_LOSSLESS_COMPRESSION },
    { "STR_JPEG_COMPRESSION",       STR_JPEG_COMPRESSION },
    { "STR_QUALITY",                STR_QUALITY },
    { "STR_REMOVE_CROP_AREA",       STR_REMOVE_CROP_AREA },
    { "STR_IMAGE_RESOLUTION",       STR_IMAGE_RESOLUTION },
    { "STR_IMAGE_RESOLUTION_0",     STR_IMAGE_RESOLUTION_0 },
    { "STR_IMAGE_RESOLUTION_1",     STR_IMAGE_RESOLUTION_1 },
    { "STR_IMAGE_RESOLUTION_2",     STR_IMAGE_RESOLUTION_2 },
    { "STR_IMAGE_RESOLUTION_3",     STR_IMAGE_RESOLUTION_3 },
    { "STR_EMBED_LINKED_GRAPHICS",  STR_EMBED_LINKED_GRAPHICS },
    { "STR_OLE_OBJECTS",            STR_OLE_OBJECTS },
    { "STR_OLE_OPTIMIZATION",       STR_OLE_OPTIMIZATION },
    { "STR_OLE_REPLACE",            STR_OLE_REPLACE },
    { "STR_ALL_OLE_OBJECTS",        STR_ALL_OLE_OBJECTS },
    { "STR_ALIEN_OLE_OBJECTS_ONLY", STR_ALIEN_OLE_OBJECTS_ONLY },
    { "STR__OLE_OBJECTS_DESC",      STR_OLE_OBJECTS_DESC },
    { "STR_NO_OLE_OBJECTS_DESC",    STR_NO_OLE_OBJECTS_DESC },
    { "STR_SLIDES",                 STR_SLIDES },
    { "STR_CHOOSE_SLIDES",          STR_CHOOSE_SLIDES },
    { "STR_DELETE_MASTER_PAGES",    STR_DELETE_MASTER_PAGES },
    { "STR_DELETE_NOTES_PAGES",     STR_DELETE_NOTES_PAGES },
    { "STR_DELETE_HIDDEN_SLIDES",   STR_DELETE_HIDDEN_SLIDES },
    { "STR_CUSTOM_SHOW",            STR_CUSTOM_SHOW },
    { "STR_SUMMARY",                STR_SUMMARY },
    { "STR_SUMMARY_TITLE",          STR_SUMMARY_TITLE },
    { "STR_APPLY_TO_CURRENT",       STR_APPLY_TO_CURRENT },
    { "STR_AUTOMATICALLY_OPEN",     STR_AUTOMATICALLY_OPEN },
    { "STR_SAVE_SETTINGS",          STR_SAVE_SETTINGS },
    { "STR_SAVE_AS",                STR_SAVE_AS },
    { "STR_DELETE_SLIDES",          STR_DELETE_SLIDES },
    { "STR_OPTIMIZE_IMAGES",        STR_OPTIMIZE_IMAGES },
    { "STR_CREATE_REPLACEMENT",     STR_CREATE_REPLACEMENT },
    { "STR_CURRENT_FILESIZE",       STR_CURRENT_FILESIZE },
    { "STR_ESTIMATED_FILESIZE",     STR_ESTIMATED_FILESIZE },
    { "MY_SETTINGS",                STR_MY_SETTINGS },
    { "STR_OK",                     STR_OK },
    { "STR_INFO_1",                 STR_INFO_1 },
    { "STR_INFO_2",                 STR_INFO_2 },
    { "STR_INFO_3",                 STR_INFO_3 },
    { "STR_INFO_4",                 STR_INFO_4 },
    { "STR_DUPLICATING_PRESENTATION",STR_DUPLICATING_PRESENTATION },
    { "STR_DELETING_SLIDES",        STR_DELETING_SLIDES },
    { "STR_OPTIMIZING_GRAPHICS",    STR_OPTIMIZING_GRAPHICS },
    { "STR_CREATING_OLE_REPLACEMENTS",STR_CREATING_OLE_REPLACEMENTS },
    { "STR_FileSizeSeparator",      STR_FILESIZESEPARATOR },

    { "NotFound",           TK_NotFound }
};

PPPOptimizerTokenEnum TKGet( const OUString& rToken )
{
    if ( !pHashMap )
    {   // init hash map
        ::osl::MutexGuard aGuard( getHashMapMutex() );
        if ( !pHashMap )
        {
            TypeNameHashMap* pH = new TypeNameHashMap;
            const TokenTable* pPtr = pTokenTableArray;
            const TokenTable* pEnd = pPtr + SAL_N_ELEMENTS( pTokenTableArray );
            for ( ; pPtr < pEnd; pPtr++ )
                (*pH)[ pPtr->pS ] = pPtr->pE;
            pHashMap = pH;
        }
    }
    PPPOptimizerTokenEnum eRetValue = TK_NotFound;
    int i, nLen = rToken.getLength();
    std::unique_ptr<char[]> pBuf(new char[ nLen + 1 ]);
    for ( i = 0; i < nLen; i++ )
        pBuf[ i ] = (char)rToken[ i ];
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
