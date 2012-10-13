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
#include <boost/unordered_map.hpp>
#include <string.h>

struct THash
{
    size_t operator()( const char* s ) const
    {
        return rtl_str_hashCode(s);
    }
};
struct TCheck
{
    bool operator()( const char* s1, const char* s2 ) const
    {
        return strcmp( s1, s2 ) == 0;
    }
};
typedef boost::unordered_map< const char*, PPPOptimizerTokenEnum, THash, TCheck> TypeNameHashMap;
static TypeNameHashMap* pHashMap = NULL;
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
    { "lnNavSep1",          TK_lnNavSep1 },
    { "lnNavSep2",          TK_lnNavSep2 },
    { "btnNavBack",         TK_btnNavBack },
    { "btnNavNext",         TK_btnNavNext },
    { "btnNavFinish",       TK_btnNavFinish },
    { "btnNavCancel",       TK_btnNavCancel },

    { "FixedText0Pg0",      TK_FixedText0Pg0 },
    { "FixedText1Pg0",      TK_FixedText1Pg0 },
    { "Separator1Pg0",      TK_Separator1Pg0 },
    { "FixedText2Pg0",      TK_FixedText2Pg0 },
    { "ListBox0Pg0",        TK_ListBox0Pg0 },
    { "Button0Pg0",         TK_Button0Pg0 },
    { "RadioButton0Pg1",    TK_RadioButton0Pg1 },
    { "RadioButton1Pg1",    TK_RadioButton1Pg1 },
    { "FixedText0Pg1",      TK_FixedText0Pg1 },
    { "CheckBox1Pg1",       TK_CheckBox1Pg1 },
    { "FixedText1Pg1",      TK_FixedText1Pg1 },
    { "FixedText2Pg1",      TK_FixedText2Pg1 },
    { "FormattedField0Pg1", TK_FormattedField0Pg1 },
    { "ComboBox0Pg1",       TK_ComboBox0Pg1 },
    { "CheckBox2Pg1",       TK_CheckBox2Pg1 },
    { "FixedText0Pg2",      TK_FixedText0Pg2 },
    { "FixedText1Pg2",      TK_FixedText1Pg2 },
    { "CheckBox0Pg2",       TK_CheckBox0Pg2 },
    { "RadioButton0Pg2",    TK_RadioButton0Pg2 },
    { "RadioButton1Pg2",    TK_RadioButton1Pg2 },
    { "FixedText0Pg3",      TK_FixedText0Pg3 },
    { "CheckBox0Pg3",       TK_CheckBox0Pg3 },
    { "FixedText1Pg3",      TK_FixedText1Pg3 },
    { "CheckBox1Pg3",       TK_CheckBox1Pg3 },
    { "FixedText2Pg3",      TK_FixedText2Pg3 },
    { "CheckBox2Pg3",       TK_CheckBox2Pg3 },
    { "CheckBox3Pg3",       TK_CheckBox3Pg3 },
    { "ListBox0Pg3",        TK_ListBox0Pg3 },
    { "Separator0Pg4",      TK_Separator0Pg4 },
    { "RadioButton0Pg4",    TK_RadioButton0Pg4 },
    { "RadioButton1Pg4",    TK_RadioButton1Pg4 },
    { "Separator1Pg4",      TK_Separator1Pg4 },
    { "FixedText0Pg4",      TK_FixedText0Pg4 },
    { "FixedText1Pg4",      TK_FixedText1Pg4 },
    { "FixedText2Pg4",      TK_FixedText2Pg4 },
    { "FixedText3Pg4",      TK_FixedText3Pg4 },
    { "FixedText4Pg4",      TK_FixedText4Pg4 },
    { "FixedText5Pg4",      TK_FixedText5Pg4 },
    { "FixedText6Pg4",      TK_FixedText6Pg4 },
    { "FixedText7Pg4",      TK_FixedText7Pg4 },
    { "FixedText8Pg4",      TK_FixedText8Pg4 },
    { "CheckBox1Pg4",       TK_CheckBox1Pg4 },
    { "ComboBox0Pg4",       TK_ComboBox0Pg4 },

    { "_blank",             TK__blank },
    { "_self",              TK__self },
    { "Activated",          TK_Activated },
    { "Align",              TK_Align },
    { "Alpha",              TK_Alpha },
    { "Animated",           TK_Animated },
    { "Background",         TK_Background },
    { "BitmapPath",         TK_BitmapPath },
    { "Border",             TK_Border },
    { "Closeable",          TK_Closeable },
    { "ColorMode",          TK_ColorMode },
    { "Complete",           TK_Complete },
    { "Compression",        TK_Compression },
    { "CurrentItemID",      TK_CurrentItemID },
    { "DefaultButton",      TK_DefaultButton },
    { "DocumentService",    TK_DocumentService },
    { "Dropdown",           TK_Dropdown },
    { "EffectiveValue",     TK_EffectiveValue },
    { "EffectiveMin",       TK_EffectiveMin },
    { "EffectiveMax",       TK_EffectiveMax },
    { "Enabled",            TK_Enabled },
    { "Extensions",         TK_Extensions },
    { "FileSizeDestination",TK_FileSizeDestination },
    { "FileSizeSource",     TK_FileSizeSource },
    { "FillBitmap",         TK_FillBitmap },
    { "FillBitmapLogicalSize",TK_FillBitmapLogicalSize },
    { "FillBitmapMode",     TK_FillBitmapMode },
    { "FillBitmapSizeX",    TK_FillBitmapSizeX },
    { "FillBitmapSizeY",    TK_FillBitmapSizeY },
    { "FillBitmapURL",      TK_FillBitmapURL },
    { "FillStyle",          TK_FillStyle },
    { "FilterData",         TK_FilterData },
    { "FilterName",         TK_FilterName },
    { "Flags",              TK_Flags },
    { "FontDescriptor",     TK_FontDescriptor },
    { "Graphic",            TK_Graphic },
    { "GraphicCrop",        TK_GraphicCrop },
    { "GraphicCropLogic",   TK_GraphicCropLogic },
    { "GraphicURL",         TK_GraphicURL },
    { "GraphicStreamURL",   TK_GraphicStreamURL },
    { "Height",             TK_Height },
    { "Hidden",             TK_Hidden },
    { "ID",                 TK_ID },
    { "ImageURL",           TK_ImageURL },
    { "InformationDialog",  TK_InformationDialog },
    { "InputStream",        TK_InputStream },
    { "Interlaced",         TK_Interlaced },
    { "IsInternal",         TK_IsInternal },
    { "Label",              TK_Label },
    { "LineCount",          TK_LineCount },
    { "LogicalHeight",      TK_LogicalHeight },
    { "LogicalWidth",       TK_LogicalWidth },
    { "LogicalSize",        TK_LogicalSize },
    { "MimeType",           TK_MimeType },
    { "Moveable",           TK_Moveable },
    { "MultiLine",          TK_MultiLine },
    { "MultiSelection",     TK_MultiSelection },
    { "Name",               TK_Name },
    { "Orientation",        TK_Orientation },
    { "OutputStream",       TK_OutputStream },
    { "PixelHeight",        TK_PixelHeight },
    { "PixelWidth",         TK_PixelWidth },
    { "PositionX",          TK_PositionX },
    { "PositionY",          TK_PositionY },
    { "Progress",           TK_Progress },
    { "ProgressValue",      TK_ProgressValue },
    { "ProgressValueMax",   TK_ProgressValueMax },
    { "ProgressValueMin",   TK_ProgressValueMin },
    { "PushButtonType",     TK_PushButtonType },
    { "Quality",            TK_Quality },
    { "ReadOnly",           TK_ReadOnly },
    { "Repeat",             TK_Repeat },
    { "ScaleImage",         TK_ScaleImage },
    { "SelectedItems",      TK_SelectedItems },
    { "Settings",           TK_Settings },
    { "Size100thMM",        TK_Size100thMM },
    { "SizePixel",          TK_SizePixel },
    { "Spin",               TK_Spin },
    { "Step",               TK_Step },
    { "State",              TK_State },
    { "StatusDispatcher",   TK_StatusDispatcher },
    { "StringItemList",     TK_StringItemList },
    { "Strings",            TK_Strings },
    { "TabIndex",           TK_TabIndex },
    { "Template",           TK_Template },
    { "Text",               TK_Text },
    { "Title",              TK_Title },
    { "Transparent",        TK_Transparent },
    { "Type",               TK_Type },
    { "UIName",             TK_UIName },
    { "Value",              TK_Value },
    { "Width",              TK_Width },
    { "ZOrder",             TK_ZOrder },

    { "LastUsedSettings",   TK_LastUsedSettings },
    { "Settings/Templates", TK_Settings_Templates },
    { "Settings/Templates/",TK_Settings_Templates_ },
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
    { "CurrentPage",        TK_CurrentPage },
    { "GraphicObjects",     TK_GraphicObjects },
    { "CurrentGraphicObject",TK_CurrentGraphicObject },
    { "OLEObjects",         TK_OLEObjects },
    { "CurrentOLEObject",   TK_CurrentOLEObject },

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
    { "STR_MASTER_PAGES",           STR_MASTER_PAGES },
    { "STR_DELETE_MASTER_PAGES",    STR_DELETE_MASTER_PAGES },
    { "STR_DELETE_NOTES_PAGES",     STR_DELETE_NOTES_PAGES },
    { "STR_DELETE_HIDDEN_SLIDES",   STR_DELETE_HIDDEN_SLIDES },
    { "STR_CUSTOM_SHOW",            STR_CUSTOM_SHOW },
    { "STR_SUMMARY",                STR_SUMMARY },
    { "STR_SUMMARY_TITLE",          STR_SUMMARY_TITLE },
    { "STR_PROGRESS",               STR_PROGRESS },
    { "STR_OBJECTS_OPTIMIZED",      STR_OBJECTS_OPTIMIZED },
    { "STR_APPLY_TO_CURRENT",       STR_APPLY_TO_CURRENT },
    { "STR_AUTOMATICALLY_OPEN",     STR_AUTOMATICALLY_OPEN },
    { "STR_SAVE_SETTINGS",          STR_SAVE_SETTINGS },
    { "STR_SAVE_AS",                STR_SAVE_AS },
    { "STR_DELETE_SLIDES",          STR_DELETE_SLIDES },
    { "STR_OPTIMIZE_IMAGES",        STR_OPTIMIZE_IMAGES },
    { "STR_CREATE_REPLACEMENT",     STR_CREATE_REPLACEMENT },
    { "STR_CURRENT_FILESIZE",       STR_CURRENT_FILESIZE },
    { "STR_ESTIMATED_FILESIZE",     STR_ESTIMATED_FILESIZE },
    { "STR_MB",                     STR_MB },
    { "MY_SETTINGS",                STR_MY_SETTINGS },
    { "STR_DEFAULT_SESSION",        STR_DEFAULT_SESSION },
    { "STR_MODIFY_WARNING",         STR_MODIFY_WARNING },
    { "STR_YES",                    STR_YES },
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

    { "Last",               TK_Last },
    { "NotFound",           TK_NotFound }
};

PPPOptimizerTokenEnum TKGet( const rtl::OUString& rToken )
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
    char* pBuf = new char[ nLen + 1 ];
    for ( i = 0; i < nLen; i++ )
        pBuf[ i ] = (char)rToken[ i ];
    pBuf[ i ] = 0;
    TypeNameHashMap::iterator aHashIter( pHashMap->find( pBuf ) );
    delete[] pBuf;
    if ( aHashIter != pHashMap->end() )
        eRetValue = (*aHashIter).second;
    return eRetValue;
}

rtl::OUString TKGet( const PPPOptimizerTokenEnum eToken )
{
    sal_uInt32 i = eToken >= TK_Last
        ? (sal_uInt32)TK_NotFound
        : (sal_uInt32)eToken;
    return rtl::OUString::createFromAscii( pTokenTableArray[ i ].pS );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
