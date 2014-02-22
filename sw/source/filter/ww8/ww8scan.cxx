/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "ww8scan.hxx"

#include <functional>
#include <algorithm>

#include <string.h>
#include <i18nlangtag/mslangid.hxx>
#include <filter/msfilter/sprmids.hxx>
#include <rtl/tencinfo.h>
#include <sal/macros.h>

#ifdef DUMP

#define ERR_SWG_READ_ERROR 1234
#define OSL_ENSURE( a, b )

#else                       
#include <swerror.h>
#include <swtypes.hxx>

#endif                      
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <unotools/localedatawrapper.hxx>
#include <i18nlangtag/lang.h>
#include <editeng/unolingu.hxx>

#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>

#include <stdio.h>

#define ASSERT_RET_ON_FAIL( aCon, aError, aRet ) \
    OSL_ENSURE(aCon, aError); \
    if (!(aCon)) \
        return aRet;

using namespace ::com::sun::star::lang;


namespace SL
{
#   define IMPLCONSTSTRINGARRAY(X) const char a##X[] = "" #X ""
    IMPLCONSTSTRINGARRAY(ObjectPool);
    IMPLCONSTSTRINGARRAY(1Table);
    IMPLCONSTSTRINGARRAY(0Table);
    IMPLCONSTSTRINGARRAY(Data);
    IMPLCONSTSTRINGARRAY(CheckBox);
    IMPLCONSTSTRINGARRAY(ListBox);
    IMPLCONSTSTRINGARRAY(TextBox);
    IMPLCONSTSTRINGARRAY(TextField);
    IMPLCONSTSTRINGARRAY(MSMacroCmds);
}

namespace
{
    /**
        winword strings are typically Belt and Braces strings preceeded with a
        pascal style count, and ending with a c style 0 terminator. 16bit chars
        and count for ww8+ and 8bit chars and count for ww7-. The count and 0
        can be checked for integrity to catch errors (e.g. lotus created
        documents) where in error 8bit strings are used instead of 16bits
        strings for style names.
    */
    bool TestBeltAndBraces(SvStream& rStrm)
    {
        bool bRet = false;
        sal_uInt32 nOldPos = rStrm.Tell();
        sal_Unicode nBelt(0);
        rStrm.ReadUInt16( nBelt );
        nBelt *= sizeof(sal_Unicode);
        if (rStrm.good() && (rStrm.remainingSize() >= (nBelt + sizeof(sal_Unicode))))
        {
            rStrm.SeekRel(nBelt);
            if (rStrm.good())
            {
                sal_Unicode cBraces(0);
                rStrm.ReadUInt16( cBraces );
                if (rStrm.good() && cBraces == 0)
                    bRet = true;
            }
        }
        rStrm.Seek(nOldPos);
        return bRet;
    }
}

inline bool operator==(const SprmInfo &rFirst, const SprmInfo &rSecond)
{
    return (rFirst.nId == rSecond.nId);
}

const wwSprmSearcher *wwSprmParser::GetWW2SprmSearcher()
{
    
    
    static const SprmInfo aSprms[] =
    {
        {  0, 0, L_FIX}, 
        {  2, 1, L_FIX}, 
        {  3, 0, L_VAR}, 
        {  4, 1, L_FIX}, 
        {  5, 1, L_FIX}, 
        {  6, 1, L_FIX}, 
        {  7, 1, L_FIX}, 
        {  8, 1, L_FIX}, 
        {  9, 1, L_FIX}, 
        { 10, 1, L_FIX}, 
        { 11, 1, L_FIX}, 
        { 12, 1, L_FIX}, 
        { 13, 1, L_FIX}, 
        { 14, 1, L_FIX}, 
        { 15, 0, L_VAR}, 
        { 16, 2, L_FIX}, 
        { 17, 2, L_FIX}, 
        { 18, 2, L_FIX}, 
        { 19, 2, L_FIX}, 
        { 20, 2, L_FIX}, 
        { 21, 2, L_FIX}, 
        { 22, 2, L_FIX}, 
        { 23, 0, L_VAR}, 
        { 24, 1, L_FIX}, 
        { 25, 1, L_FIX}, 
        { 26, 2, L_FIX}, 
        { 27, 2, L_FIX}, 
        { 28, 2, L_FIX}, 
        { 29, 1, L_FIX}, 
        { 30, 2, L_FIX}, 
        { 31, 2, L_FIX}, 
        { 32, 2, L_FIX}, 
        { 33, 2, L_FIX}, 
        { 34, 2, L_FIX}, 
        { 35, 2, L_FIX}, 
        { 36, 2, L_FIX}, 
        { 37, 1, L_FIX}, 
        { 38, 2, L_FIX}, 
        { 39, 2, L_FIX}, 
        { 40, 2, L_FIX}, 
        { 41, 2, L_FIX}, 
        { 42, 2, L_FIX}, 
        { 43, 2, L_FIX}, 
        { 44, 1, L_FIX}, 
        { 45, 2, L_FIX}, 
        { 46, 2, L_FIX}, 
        { 47, 2, L_FIX}, 
        { 48, 2, L_FIX}, 
        { 49, 2, L_FIX}, 
        { 50, 1, L_FIX}, 
        { 51, 1, L_FIX}, 
        { 52, 0, L_FIX}, 
        { 53, 1, L_FIX}, 
        { 54, 1, L_FIX}, 
        { 55, 1, L_FIX}, 
        { 57, 0, L_VAR}, 
        { 58, 0, L_FIX}, 
        { 60, 1, L_FIX}, 
        { 61, 1, L_FIX}, 
        { 62, 1, L_FIX}, 
        { 63, 1, L_FIX}, 
        { 64, 1, L_FIX}, 
        { 65, 1, L_FIX}, 
        { 66, 1, L_FIX}, 
        { 67, 1, L_FIX}, 
        { 68, 2, L_FIX}, 
        { 69, 1, L_FIX}, 
        { 70, 3, L_FIX}, 
        { 71, 2, L_FIX}, 
        { 72, 2, L_FIX}, 
        { 73, 1, L_FIX}, 
        { 74, 1, L_FIX}, 
        { 75, 1, L_FIX}, 
        { 76, 1, L_FIX}, 
        { 77, 1, L_FIX}, 
        { 78, 0, L_VAR}, 
        { 80, 1, L_FIX}, 
        { 81, 1, L_FIX}, 
        { 82, 2, L_FIX}, 
        { 83, 2, L_FIX}, 
        { 84, 1, L_FIX}, 
        { 85, 1, L_FIX}, 
        { 86, 1, L_FIX}, 
        { 87, 1, L_FIX}, 
        { 94, 1, L_FIX}, 
        { 95,12, L_VAR}, 
        { 96, 2, L_FIX}, 
        { 97, 2, L_FIX}, 
        { 98, 2, L_FIX}, 
        { 99, 2, L_FIX}, 
        {112, 1, L_FIX}, 
        {114, 1, L_FIX}, 
        {115, 2, L_FIX}, 
        {116, 2, L_FIX}, 
        {117, 1, L_FIX}, 
        {118, 1, L_FIX}, 
        {119, 2, L_FIX}, 
        {120, 2, L_FIX}, 
        {121, 1, L_FIX}, 
        {122, 1, L_FIX}, 
        {123, 2, L_FIX}, 
        {124, 2, L_FIX}, 
        {125, 1, L_FIX}, 
        {126, 1, L_FIX}, 
        {127, 1, L_FIX}, 
        {128, 1, L_FIX}, 
        {129, 2, L_FIX}, 
        {130, 2, L_FIX}, 
        {131, 2, L_FIX}, 
        {132, 2, L_FIX}, 
        {133, 1, L_FIX}, 
        {134, 1, L_FIX}, 
        {135, 2, L_FIX}, 
        {136, 2, L_FIX}, 
        {137, 1, L_FIX}, 
        {138, 1, L_FIX}, 
        {139, 2, L_FIX}, 
        {140, 2, L_FIX}, 
        {141, 2, L_FIX}, 
        {142, 2, L_FIX}, 
        {143, 2, L_FIX}, 
        {144, 2, L_FIX}, 
        {145, 2, L_FIX}, 
        {146, 2, L_FIX}, 
        {147, 2, L_FIX}, 
        {148, 2, L_FIX}, 
        {149, 1, L_FIX}, 
        {152, 0, L_VAR}, 
        {153, 2, L_FIX}, 
        {154, 0, L_VAR2},
        {155, 1, L_VAR}, 
        {157, 5, L_FIX}, 
        {158, 4, L_FIX}, 
        {159, 2, L_FIX}, 
        {160, 4, L_FIX}, 
        {161, 2, L_FIX}, 
        {162, 2, L_FIX}, 
        {163, 5, L_FIX}, 
        {164, 4, L_FIX}, 
    };

    static wwSprmSearcher aSprmSrch(aSprms, sizeof(aSprms) / sizeof(aSprms[0]));
    return &aSprmSrch;
};


const wwSprmSearcher *wwSprmParser::GetWW6SprmSearcher()
{
    
    
    static const SprmInfo aSprms[] =
    {
        {  0, 0, L_FIX}, 
        {  2, 2, L_FIX}, 
        {  3, 3, L_VAR}, 
        {  4, 1, L_FIX}, 
        {  5, 1, L_FIX}, 
        {  6, 1, L_FIX}, 
        {  7, 1, L_FIX}, 
        {  8, 1, L_FIX}, 
        {  9, 1, L_FIX}, 
        { 10, 1, L_FIX}, 
        { 11, 1, L_FIX}, 
        { 12, 0, L_VAR}, 
        { 13, 1, L_FIX}, 
        { 14, 1, L_FIX}, 
        { 15, 0, L_VAR}, 
        { 16, 2, L_FIX}, 
        { 17, 2, L_FIX}, 
        { 18, 2, L_FIX}, 
        { 19, 2, L_FIX}, 
        { 20, 4, L_FIX}, 
        { 21, 2, L_FIX}, 
        { 22, 2, L_FIX}, 
        { 23, 0, L_VAR}, 
        { 24, 1, L_FIX}, 
        { 25, 1, L_FIX}, 
        { 26, 2, L_FIX}, 
        { 27, 2, L_FIX}, 
        { 28, 2, L_FIX}, 
        { 29, 1, L_FIX}, 
        { 30, 2, L_FIX}, 
        { 31, 2, L_FIX}, 
        { 32, 2, L_FIX}, 
        { 33, 2, L_FIX}, 
        { 34, 2, L_FIX}, 
        { 35, 2, L_FIX}, 
        { 36, 2, L_FIX}, 
        { 37, 1, L_FIX}, 
        { 38, 2, L_FIX}, 
        { 39, 2, L_FIX}, 
        { 40, 2, L_FIX}, 
        { 41, 2, L_FIX}, 
        { 42, 2, L_FIX}, 
        { 43, 2, L_FIX}, 
        { 44, 1, L_FIX}, 
        { 45, 2, L_FIX}, 
        { 46, 2, L_FIX}, 
        { 47, 2, L_FIX}, 
        { 48, 2, L_FIX}, 
        { 49, 2, L_FIX}, 
        { 50, 1, L_FIX}, 
        { 51, 1, L_FIX}, 
        { 52, 0, L_FIX}, 
        { 64, 0, L_VAR}, 
        { 65, 1, L_FIX}, 
        { 66, 1, L_FIX}, 
        { 67, 1, L_FIX}, 
        { 68, 0, L_VAR}, 
        { 69, 2, L_FIX}, 
        { 70, 4, L_FIX}, 
        { 71, 1, L_FIX}, 
        { 72, 2, L_FIX}, 
        { 73, 3, L_FIX}, 
        { 74, 0, L_VAR}, 
        { 75, 1, L_FIX}, 
        { 77, 0, L_VAR}, 
        { 79, 0, L_VAR}, 
        { 80, 2, L_FIX}, 
        { 81, 0, L_VAR}, 
        { 82, 0, L_VAR}, 
        { 83, 0, L_FIX}, 
        { 85, 1, L_FIX}, 
        { 86, 1, L_FIX}, 
        { 87, 1, L_FIX}, 
        { 88, 1, L_FIX}, 
        { 89, 1, L_FIX}, 
        { 90, 1, L_FIX}, 
        { 91, 1, L_FIX}, 
        { 92, 1, L_FIX}, 
        { 93, 2, L_FIX}, 
        { 94, 1, L_FIX}, 
        { 95, 3, L_FIX}, 
        { 96, 2, L_FIX}, 
        { 97, 2, L_FIX}, 
        { 98, 1, L_FIX}, 
        { 99, 2, L_FIX}, 
        {100, 1, L_FIX}, 
        {101, 2, L_FIX}, 
        {102, 1, L_FIX}, 
        {103, 0, L_VAR}, 
        {104, 1, L_FIX}, 
        {105, 0, L_VAR}, 
        {106, 0, L_VAR}, 
        {107, 2, L_FIX}, 
        {108, 0, L_VAR}, 
        {109, 2, L_FIX}, 
        {110, 2, L_FIX}, 
        {111, 2, L_FIX}, 
        {112, 2, L_FIX}, 
        {113, 0, L_VAR}, 
        {115, 0, L_VAR}, 
        {116, 0, L_VAR}, 
        {117, 1, L_FIX}, 
        {118, 1, L_FIX}, 
        {119, 1, L_FIX}, 
        {120,12, L_VAR}, 
        {121, 2, L_FIX}, 
        {122, 2, L_FIX}, 
        {123, 2, L_FIX}, 
        {124, 2, L_FIX}, 
        {131, 1, L_FIX}, 
        {132, 1, L_FIX}, 
        {133, 0, L_VAR}, 
        {136, 3, L_FIX}, 
        {137, 3, L_FIX}, 
        {138, 1, L_FIX}, 
        {139, 1, L_FIX}, 
        {140, 2, L_FIX}, 
        {141, 2, L_FIX}, 
        {142, 1, L_FIX}, 
        {143, 1, L_FIX}, 
        {144, 2, L_FIX}, 
        {145, 2, L_FIX}, 
        {146, 1, L_FIX}, 
        {147, 1, L_FIX}, 
        {148, 2, L_FIX}, 
        {149, 2, L_FIX}, 
        {150, 1, L_FIX}, 
        {151, 1, L_FIX}, 
        {152, 1, L_FIX}, 
        {153, 1, L_FIX}, 
        {154, 2, L_FIX}, 
        {155, 2, L_FIX}, 
        {156, 2, L_FIX}, 
        {157, 2, L_FIX}, 
        {158, 1, L_FIX}, 
        {159, 1, L_FIX}, 
        {160, 2, L_FIX}, 
        {161, 2, L_FIX}, 
        {162, 1, L_FIX}, 
        {163, 0, L_FIX}, 
        {164, 2, L_FIX}, 
        {165, 2, L_FIX}, 
        {166, 2, L_FIX}, 
        {167, 2, L_FIX}, 
        {168, 2, L_FIX}, 
        {169, 2, L_FIX}, 
        {170, 2, L_FIX}, 
        {171, 2, L_FIX}, 
        {179, 0, L_VAR}, 
        {181, 0, L_VAR}, 
        {182, 2, L_FIX}, 
        {183, 2, L_FIX}, 
        {184, 2, L_FIX}, 
        {185, 1, L_FIX}, 
        {186, 1, L_FIX}, 
        {187,12, L_FIX}, 
        {188, 0, L_VAR}, 
        {189, 2, L_FIX}, 
        {190, 0, L_VAR2},
        {191, 1, L_VAR}, 
        {192, 4, L_FIX}, 
        {193, 5, L_FIX}, 
        {194, 4, L_FIX}, 
        {195, 2, L_FIX}, 
        {196, 4, L_FIX}, 
        {197, 2, L_FIX}, 
        {198, 2, L_FIX}, 
        {199, 5, L_FIX}, 
        {200, 4, L_FIX}, 
        {207, 0, L_VAR}  
    };

    static wwSprmSearcher aSprmSrch(aSprms, sizeof(aSprms) / sizeof(aSprms[0]));
    return &aSprmSrch;
};

const wwSprmSearcher *wwSprmParser::GetWW8SprmSearcher()
{
    
    
    static const SprmInfo aSprms[] =
    {
        {     0, 0, L_FIX}, 
        {0x4600, 2, L_FIX}, 
        {0xC601, 0, L_VAR}, 
        {0x2602, 1, L_FIX}, 
                            
                            
        {0x2403, 1, L_FIX}, 
        {0x2404, 1, L_FIX}, 
        {0x2405, 1, L_FIX}, 
        {0x2406, 1, L_FIX}, 
        {0x2407, 1, L_FIX}, 
                            
        {0x2408, 1, L_FIX}, 
        {0x2409, 1, L_FIX}, 
        {0x260A, 1, L_FIX}, 
        {0x460B, 2, L_FIX}, 
        {0x240C, 1, L_FIX}, 
        {0xC60D, 0, L_VAR}, 
                            
        {0x840E, 2, L_FIX}, 
        {0x840F, 2, L_FIX}, 
        {0x4610, 2, L_FIX}, 
        {0x8411, 2, L_FIX}, 
        {0x6412, 4, L_FIX}, 
                            
                            
        {0xA413, 2, L_FIX}, 
        {0xA414, 2, L_FIX}, 
        {0xC615, 0, L_VAR}, 
                            
        {0x2416, 1, L_FIX}, 
        {0x2417, 1, L_FIX}, 
        {0x8418, 2, L_FIX}, 
        {0x8419, 2, L_FIX}, 
        {0x841A, 2, L_FIX}, 
        {0x261B, 1, L_FIX}, 
        {0x461C, 2, L_FIX}, 
        {0x461D, 2, L_FIX}, 
        {0x461E, 2, L_FIX}, 
        {0x461F, 2, L_FIX}, 
        {0x4620, 2, L_FIX}, 
        {0x4621, 2, L_FIX}, 
        {0x4622, 2, L_FIX}, 
        {0x2423, 1, L_FIX}, 
        {0x6424, 4, L_FIX}, 
        {0x6425, 4, L_FIX}, 
        {0x6426, 4, L_FIX}, 
        {0x6427, 4, L_FIX}, 
        {0x6428, 4, L_FIX}, 
        {0x6629, 4, L_FIX}, 
        {0x242A, 1, L_FIX}, 
        {0x442B, 2, L_FIX}, 
        {0x442C, 2, L_FIX}, 
        {0x442D, 2, L_FIX}, 
        {0x842E, 2, L_FIX}, 
        {0x842F, 2, L_FIX}, 
        {0x2430, 1, L_FIX}, 
        {0x2431, 1, L_FIX}, 
        {0xC632, 0, L_VAR}, 
        {0x2433, 1, L_FIX}, 
        {0x2434, 1, L_FIX}, 
        {0x2435, 1, L_FIX}, 
        {0x2436, 1, L_FIX}, 
        {0x2437, 1, L_FIX}, 
        {0x2438, 1, L_FIX}, 
        {NS_sprm::LN_PWAlignFont, 2, L_FIX}, 
        {0x443A, 2, L_FIX}, 
                            
        {0x243B, 1, L_FIX}, 
                            
        {0xC63E, 0, L_VAR}, 
        {0xC63F, 0, L_VAR}, 
        {0x2640, 1, L_FIX}, 
                            
        {0x2441, 1, L_FIX}, 
        {0x2443, 1, L_FIX}, 
        {0x2444, 1, L_FIX}, 
        {0xC645, 0, L_VAR}, 
        {0x6645, 4, L_FIX}, 
                            
        {0x6646, 4, L_FIX}, 
                            
        {0x2447, 1, L_FIX}, 
                            
        {0x2448, 1, L_FIX}, 
        {0x0800, 1, L_FIX}, 
        {0x0801, 1, L_FIX}, 
        {0x0802, 1, L_FIX}, 
        {0x6A03, 4, L_FIX}, 
        {0x4804, 2, L_FIX}, 
                            
        {0x6805, 4, L_FIX}, 
        {0x0806, 1, L_FIX}, 
        {0x4807, 2, L_FIX}, 
                            
                            
        {0xEA08, 1, L_FIX}, 
        {0x6A09, 4, L_FIX}, 
                            
        {0x080A, 1, L_FIX}, 
        {0x480B, 0, L_FIX}, 
                            
        {0x2A0C, 1, L_FIX}, 
                            
                            
        {0x680E, 4, L_FIX}, 
        {0x2A10, 0, L_FIX}, 
        {0x4A30, 2, L_FIX}, 
        {0xCA31, 0, L_VAR}, 
        {0x2A32, 0, L_VAR}, 
        {0x2A33, 0, L_FIX}, 
        {0x2A34, 1, L_FIX}, 
        {0x0835, 1, L_FIX}, 
        {0x0836, 1, L_FIX}, 
        {0x0837, 1, L_FIX}, 
        {0x0838, 1, L_FIX}, 
        {0x0839, 1, L_FIX}, 
        {0x083A, 1, L_FIX}, 
        {0x083B, 1, L_FIX}, 
        {0x083C, 1, L_FIX}, 
        {0x4A3D, 2, L_FIX}, 
        {0x2A3E, 1, L_FIX}, 
        {0xEA3F, 3, L_FIX}, 
        {0x8840, 2, L_FIX}, 
        {0x4A41, 2, L_FIX}, 
        {0x2A42, 1, L_FIX}, 
        {0x4A43, 2, L_FIX}, 
        {0x2A44, 1, L_FIX}, 
        {0x4845, 2, L_FIX}, 
        {0x2A46, 1, L_FIX}, 
        {0xCA47, 0, L_VAR}, 
                            
                            
                            
                            
                            
        {0x2A48, 1, L_FIX}, 
        {0xCA49, 0, L_VAR}, 
        {0xCA4A, 0, L_VAR}, 
        {0x484B, 2, L_FIX}, 
        {0xCA4C, 2, L_FIX}, 
                            
                            
                            
        {0x4A4D, 2, L_FIX}, 
        {0x484E, 2, L_FIX}, 
        {0x4A4F, 2, L_FIX}, 
        {0x4A50, 2, L_FIX}, 
        {0x4A51, 2, L_FIX}, 
        {0x4852, 2, L_FIX}, 
        {0x2A53, 1, L_FIX}, 
        {0x0854, 1, L_FIX}, 
        {0x0855, 1, L_FIX}, 
        {0x0856, 1, L_FIX}, 
        {0xCA57, 0, L_VAR}, 
                            
        {0x0858, 1, L_FIX}, 
        {0x2859, 1, L_FIX}, 
        {0x085A, 1, L_FIX}, 
        {0x085B, 1, L_FIX}, 
        {0x085C, 1, L_FIX}, 
        {0x085D, 1, L_FIX}, 
        {0x4A5E, 2, L_FIX},
        {0x485F, 2, L_FIX}, 
        {0x4A60, 1, L_FIX}, 
        {0x4A61, 2, L_FIX}, 
        {0xCA62, 0, L_VAR}, 
                            
        {0x4863, 2, L_FIX}, 
                            
        {NS_sprm::LN_CDttmRMarkDel, 4, L_FIX}, 
        {0x6865, 4, L_FIX}, 
        {0x4866, 2, L_FIX}, 
        {0x4867, 2, L_FIX}, 
                            
                            
        {0x0868, 1, L_FIX}, 
                            
        {0x486B, 2, L_FIX}, 
        {0x486D, 2, L_FIX}, 
        {0x486E, 2, L_FIX}, 
        {0x286F, 1, L_FIX}, 
        {0x2E00, 1, L_FIX}, 
        {0xCE01, 0, L_VAR}, 
                            
                            
        {0x6C02, 4, L_FIX}, 
        {0x6C03, 4, L_FIX}, 
        {0x6C04, 4, L_FIX}, 
        {0x6C05, 4, L_FIX}, 
        {0x3000, 1, L_FIX}, 
        {0x3001, 1, L_FIX}, 
                            
        {0xD202, 0, L_VAR}, 
        {0xF203, 3, L_FIX}, 
        {0xF204, 3, L_FIX}, 
                            
        {0x3005, 1, L_FIX}, 
        {0x3006, 1, L_FIX}, 
        {0x5007, 2, L_FIX}, 
        {0x5008, 2, L_FIX}, 
        {0x3009, 1, L_FIX}, 
        {0x300A, 1, L_FIX}, 
        {0x500B, 2, L_FIX}, 
        {0x900C, 2, L_FIX}, 
        {0x300D, 1, L_FIX}, 
        {0x300E, 1, L_FIX}, 
        {0xB00F, 2, L_FIX}, 
        {0xB010, 2, L_FIX}, 
        {0x3011, 1, L_FIX}, 
        {0x3012, 1, L_FIX}, 
        {0x3013, 1, L_FIX}, 
        {0x3014, 1, L_FIX}, 
        {0x5015, 2, L_FIX}, 
        {0x9016, 2, L_FIX}, 
        {0xB017, 2, L_FIX}, 
        {0xB018, 2, L_FIX}, 
        {0x3019, 1, L_FIX}, 
        {0x301A, 1, L_FIX}, 
        {0x501B, 2, L_FIX}, 
        {0x501C, 2, L_FIX}, 
        {0x301D, 1, L_FIX}, 
        {0x301E, 1, L_FIX}, 
        {0xB01F, 2, L_FIX}, 
        {0xB020, 2, L_FIX}, 
        {0xB021, 2, L_FIX}, 
        {0xB022, 2, L_FIX}, 
        {0x9023, 2, L_FIX}, 
        {0x9024, 2, L_FIX}, 
        {0xB025, 2, L_FIX}, 
        {0x5026, 2, L_FIX}, 
        {0xD227, 0, L_VAR}, 
                            
        {0x3228, 1, L_FIX}, 
        {0x3229, 1, L_FIX}, 
        {0x322A, 1, L_FIX}, 
                            
        {0x702B, 4, L_FIX}, 
        {0x702C, 4, L_FIX}, 
        {0x702D, 4, L_FIX}, 
        {0x702E, 4, L_FIX}, 
        {0x522F, 2, L_FIX}, 
        {0x7030, 4, L_FIX}, 
        {0x9031, 2, L_FIX}, 
                            
        {0x5032, 2, L_FIX}, 
        {0x5033, 2, L_FIX}, 
        {0x5400, 2, L_FIX}, 
                            
        {0x9601, 2, L_FIX}, 
        {0x9602, 2, L_FIX}, 
                            
        {0x3403, 1, L_FIX}, 
        {0x3404, 1, L_FIX}, 
        {0x3466, 1, L_FIX}, 
        {0xD605, 0, L_VAR}, 
        {0xD606, 0, L_VAR}, 
                            
        {0x9407, 2, L_FIX}, 
        {0xD608, 0, L_VAR}, 
        {0xD609, 0, L_VAR}, 
        {0x740A, 4, L_FIX}, 
        {0x560B, 2, L_FIX}, 
        {0x740C, 1, L_FIX}, 
        {0xD620, 0, L_VAR}, 
        {0x7621, 4, L_FIX}, 
        {0x5622, 2, L_FIX}, 
        {0x7623, 4, L_FIX}, 
        {0x5624, 0, L_VAR}, 
        {0x5625, 0, L_VAR}, 
        {0xD626, 0, L_VAR}, 
        {0x7627, 0, L_VAR}, 
        {0x7628, 0, L_VAR}, 
        {0x7629, 4, L_FIX}, 
                            
                            
        {0xD62A, 1, L_FIX}, 
        {0xD62B, 0, L_VAR}, 
        {0xD62C, 0, L_VAR}, 
        {0xCA78, 0, L_VAR}, 
        {0x6649, 4, L_FIX}, 
        {0xF614, 3, L_FIX}, 
        {0xD612, 0, L_VAR}, 
        {0xD613, 0, L_VAR}, 
        {0xD61A, 0, L_VAR}, 
        {0xD61B, 0, L_VAR}, 
        {0xD61C, 0, L_VAR}, 
        {0xD61D, 0, L_VAR}, 
        {0xD632, 0, L_VAR}, 
        {0xD634, 0, L_VAR}, 
        {0xD238, 0, L_VAR}, 
        {0xC64E, 0, L_VAR}, 
        {0xC64F, 0, L_VAR}, 
        {0xC650, 0, L_VAR}, 
        {0xC651, 0, L_VAR}, 
        {0xF661, 3, L_FIX}, 
        {0x4873, 2, L_FIX}, 
        {0x4874, 2, L_FIX}, 
        {0x6463, 4, L_FIX}, 
        {0x2461, 1, L_FIX}, 
        {0x845D, 2, L_FIX}, 
        {0x845E, 2, L_FIX}, 
        {0x8460, 2, L_FIX}, 
        {0x3615, 1, L_FIX}, 
        {0x360D, 1, L_FIX}, 
        {0x703A, 4, L_FIX}, 
        {0x303B, 1, L_FIX}, 
        {0x244B, 1, L_FIX}, 
        {0x244C, 1, L_FIX}, 
        {0x940E, 2, L_FIX}, 
        {0x940F, 2, L_FIX}, 
        {0x9410, 2, L_FIX}, 
        {0x6815, 4, L_FIX}, 
        {0x6816, 4, L_FIX}, 
        {NS_sprm::LN_CCv, 4, L_FIX}, 
        {0xC64D, 0, L_VAR}, 
        {0x6467, 4, L_FIX}, 
        {0x646B, 4, L_FIX}, 
        {0xF617, 3, L_FIX}, 
        {0xD660, 0, L_VAR}, 
        {0xD670, 0, L_VAR}, 
        {0xCA71, 0, L_VAR}, 
        {0x303C, 1, L_FIX}, 
        {0x245B, 1, L_FIX}, 
        {0x245C, 1, L_FIX}, 
        
        {0x246D, 1, L_FIX}
    };

    static wwSprmSearcher aSprmSrch(aSprms, sizeof(aSprms) / sizeof(aSprms[0]));
    return &aSprmSrch;
};

wwSprmParser::wwSprmParser(ww::WordVersion eVersion) : meVersion(eVersion)
{
   OSL_ENSURE((meVersion >= ww::eWW2 && meVersion <= ww::eWW8),
        "Impossible value for version");

    mnDelta = (ww::IsSevenMinus(meVersion)) ? 0 : 1;

    if (meVersion <= ww::eWW2)
        mpKnownSprms = GetWW2SprmSearcher();
    else if (meVersion < ww::eWW8)
        mpKnownSprms = GetWW6SprmSearcher();
    else
        mpKnownSprms = GetWW8SprmSearcher();
}

SprmInfo wwSprmParser::GetSprmInfo(sal_uInt16 nId) const
{
    
    SprmInfo aSrch={0,0,0};
    aSrch.nId = nId;
    const SprmInfo* pFound = mpKnownSprms->search(aSrch);
    if (pFound == 0)
    {
        OSL_ENSURE(ww::IsEightPlus(meVersion),
           "Unknown ww7- sprm, dangerous, report to development");

        aSrch.nId = 0;
        aSrch.nLen = 0;
        
        aSrch.nVari = L_VAR;

        if (ww::IsEightPlus(meVersion)) 
        {
            aSrch.nVari = L_FIX;
            switch (nId >> 13)
            {
                case 0:
                case 1:
                    aSrch.nLen = 1;
                    break;
                case 2:
                    aSrch.nLen = 2;
                    break;
                case 3:
                    aSrch.nLen = 4;
                    break;
                case 4:
                case 5:
                    aSrch.nLen = 2;
                    break;
                case 6:
                    aSrch.nLen = 0;
                    aSrch.nVari =  L_VAR;
                    break;
                case 7:
                default:
                    aSrch.nLen = 3;
                    break;
            }
        }

        pFound = &aSrch;
    }
    return *pFound;
}



inline sal_uInt8 Get_Byte( sal_uInt8 *& p )
{
    sal_uInt8 n = *p;
    p += 1;
    return n;
}

inline sal_uInt16 Get_UShort( sal_uInt8 *& p )
{
    sal_uInt16 n = SVBT16ToShort( *(SVBT16*)p );
    p += 2;
    return n;
}

inline short Get_Short( sal_uInt8 *& p )
{
    return Get_UShort(p);
}

inline sal_uLong Get_ULong( sal_uInt8 *& p )
{
    sal_uLong n = SVBT32ToUInt32( *(SVBT32*)p );
    p += 4;
    return n;
}

inline long Get_Long( sal_uInt8 *& p )
{
    return Get_ULong(p);
}

WW8SprmIter::WW8SprmIter(const sal_uInt8* pSprms_, long nLen_,
    const wwSprmParser &rParser)
    :  mrSprmParser(rParser), pSprms( pSprms_), nRemLen( nLen_)
{
    UpdateMyMembers();
}

void WW8SprmIter::SetSprms(const sal_uInt8* pSprms_, long nLen_)
{
    pSprms = pSprms_;
    nRemLen = nLen_;
    UpdateMyMembers();
}

void WW8SprmIter::advance()
{
    if (nRemLen > 0 )
    {
        sal_uInt16 nSize = nAktSize;
        if (nSize > nRemLen)
            nSize = nRemLen;
        pSprms += nSize;
        nRemLen -= nSize;
        UpdateMyMembers();
    }
}

void WW8SprmIter::UpdateMyMembers()
{
    bool bValid = (pSprms && nRemLen >= mrSprmParser.MinSprmLen());

    if (bValid)
    {
        nAktId = mrSprmParser.GetSprmId(pSprms);
        nAktSize = mrSprmParser.GetSprmSize(nAktId, pSprms);
        pAktParams = pSprms + mrSprmParser.DistanceToData(nAktId);
        bValid = nAktSize <= nRemLen;
        SAL_WARN_IF(!bValid, "sw.ww8", "sprm longer than remaining bytes, doc or parser is wrong");
    }

    if (!bValid)
    {
        nAktId = 0;
        pAktParams = 0;
        nAktSize = 0;
        nRemLen = 0;
    }
}

const sal_uInt8* WW8SprmIter::FindSprm(sal_uInt16 nId)
{
    while (GetSprms())
    {
        if (GetAktId() == nId)
            return GetAktParams();              
        advance();
    }

    return 0;                                   
}




WW8PLCFx_PCDAttrs::WW8PLCFx_PCDAttrs(ww::WordVersion eVersion,
    WW8PLCFx_PCD* pPLCFx_PCD, const WW8ScannerBase* pBase)
    : WW8PLCFx(eVersion, true), pPcdI(pPLCFx_PCD->GetPLCFIter()),
    pPcd(pPLCFx_PCD), pGrpprls(pBase->pPieceGrpprls),
    nGrpprls(pBase->nPieceGrpprls)
{
}

sal_uLong WW8PLCFx_PCDAttrs::GetIdx() const
{
    return 0;
}

void WW8PLCFx_PCDAttrs::SetIdx( sal_uLong )
{
}

bool WW8PLCFx_PCDAttrs::SeekPos(WW8_CP )
{
    return true;
}

void WW8PLCFx_PCDAttrs::advance()
{
}

WW8_CP WW8PLCFx_PCDAttrs::Where()
{
    return ( pPcd ) ? pPcd->Where() : WW8_CP_MAX;
}

void WW8PLCFx_PCDAttrs::GetSprms(WW8PLCFxDesc* p)
{
    void* pData;

    p->bRealLineEnd = false;
    if ( !pPcdI || !pPcdI->Get(p->nStartPos, p->nEndPos, pData) )
    {
        
        p->nStartPos = p->nEndPos = WW8_CP_MAX;
        p->pMemPos = 0;
        p->nSprmsLen = 0;
        return;
    }

    sal_uInt16 nPrm = SVBT16ToShort( ( (WW8_PCD*)pData )->prm );
    if ( nPrm & 1 )
    {
        
        sal_uInt16 nSprmIdx = nPrm >> 1;

        if( nSprmIdx >= nGrpprls )
        {
            
            p->nStartPos = p->nEndPos = WW8_CP_MAX;
            p->pMemPos = 0;
            p->nSprmsLen = 0;
            return;
        }
        const sal_uInt8* pSprms = pGrpprls[ nSprmIdx ];

        p->nSprmsLen = SVBT16ToShort( pSprms ); 
        pSprms += 2;
        p->pMemPos = pSprms;                    
    }
    else
    {
        
        /*
            These are the attr that are in the piece-table instead of in the text!
        */

        if (IsSevenMinus(GetFIBVersion()))
        {
            aShortSprm[0] = (sal_uInt8)( ( nPrm & 0xfe) >> 1 );
            aShortSprm[1] = (sal_uInt8)(   nPrm         >> 8 );
            p->nSprmsLen = ( nPrm ) ? 2 : 0;        

            
            p->pMemPos = aShortSprm;
        }
        else
        {
            p->pMemPos = 0;
            p->nSprmsLen = 0;
            sal_uInt8 nSprmListIdx = (sal_uInt8)((nPrm & 0xfe) >> 1);
            if( nSprmListIdx )
            {
                
                //
                
                
                //
                
                static const sal_uInt16 aSprmId[0x80] =
                {
                    
                    0x0000,0x0000,0x0000,0x0000,
                    
                    0x2402,0x2403,0x2404,0x2405,
                    
                    
                    0x2406,0x2407,0x2408,0x2409,
                    
                    0x260A,0x0000,0x240C,0x0000,
                    
                    0x0000,0x0000,0x0000,0x0000,
                    
                    0x0000,0x0000,0x0000,0x0000,
                    
                    0x2416,0x2417,0x0000,0x0000,
                    
                    0x0000,0x261B,0x0000,0x0000,
                    
                    0x0000,0x0000,0x0000,0x0000,
                    
                    0x0000,0x2423,0x0000,0x0000,
                    
                    0x0000,0x0000,0x0000,0x0000,
                    
                    0x242A,0x0000,0x0000,0x0000,
                    
                    0x0000,0x0000,0x2430,0x2431,
                    
                    
                    0x0000,0x2433,0x2434,0x2435,
                    
                    
                    0x2436,0x2437,0x2438,0x0000,
                    
                    0x0000,0x243B,0x000,0x0000,
                    
                    0x0000,0x0800,0x0801,0x0802,
                    
                    0x0000,0x0000,0x0000,0x0806,
                    
                    0x0000,0x0000,0x0000,0x080A,
                    
                    0x0000,0x2A0C,0x0858,0x2859,
                    
                    0x0000,0x0000,0x0000,0x2A33,
                    
                    0x0000,0x0835,0x0836,0x0837,
                    
                    0x0838,0x0839,0x083a,0x083b,
                    
                    0x083C,0x0000,0x2A3E,0x0000,
                    
                    0x0000,0x0000,0x2A42,0x0000,
                    
                    0x2A44,0x0000,0x2A46,0x0000,
                    
                    0x2A48,0x0000,0x0000,0x0000,
                    
                    0x0000,0x0000,0x0000,0x0000,
                    
                    0x0000,0x0000,0x0000,0x2A53,
                    
                    0x0854,0x0855,0x0856,0x2E00,
                    
                    0x2640,0x2441,0x0000,0x0000,
                    
                    0x0000,0x0000,0x0000,0x0000
                };

                
                sal_uInt16 nSprmId = aSprmId[ nSprmListIdx ];

                if( nSprmId )
                {
                    
                    aShortSprm[0] = (sal_uInt8)( ( nSprmId & 0x00ff)      );
                    aShortSprm[1] = (sal_uInt8)( ( nSprmId & 0xff00) >> 8 );
                    aShortSprm[2] = (sal_uInt8)( nPrm >> 8 );

                    
                    p->nSprmsLen = ( nPrm ) ? 3 : 0;

                    
                    p->pMemPos = aShortSprm;
                }
            }
        }
    }
}

WW8PLCFx_PCD::WW8PLCFx_PCD(ww::WordVersion eVersion, WW8PLCFpcd* pPLCFpcd,
    WW8_CP nStartCp, bool bVer67P)
    : WW8PLCFx(eVersion, false), nClipStart(-1)
{
    
    pPcdI = new WW8PLCFpcd_Iter(*pPLCFpcd, nStartCp);
    bVer67= bVer67P;
}

WW8PLCFx_PCD::~WW8PLCFx_PCD()
{
    
    delete pPcdI;
}

sal_uLong WW8PLCFx_PCD::GetIMax() const
{
    return pPcdI ? pPcdI->GetIMax() : 0;
}

sal_uLong WW8PLCFx_PCD::GetIdx() const
{
    return pPcdI ? pPcdI->GetIdx() : 0;
}

void WW8PLCFx_PCD::SetIdx( sal_uLong nIdx )
{
    if (pPcdI)
        pPcdI->SetIdx( nIdx );
}

bool WW8PLCFx_PCD::SeekPos(WW8_CP nCpPos)
{
    return pPcdI ? pPcdI->SeekPos( nCpPos ) : false;
}

WW8_CP WW8PLCFx_PCD::Where()
{
    return pPcdI ? pPcdI->Where() : WW8_CP_MAX;
}

long WW8PLCFx_PCD::GetNoSprms( WW8_CP& rStart, WW8_CP& rEnd, sal_Int32& rLen )
{
    void* pData;
    rLen = 0;

    if ( !pPcdI || !pPcdI->Get(rStart, rEnd, pData) )
    {
        rStart = rEnd = WW8_CP_MAX;
        return -1;
    }
    return pPcdI->GetIdx();
}

void WW8PLCFx_PCD::advance()
{
    OSL_ENSURE(pPcdI , "missing pPcdI");
    if (pPcdI)
        pPcdI->advance();
}

WW8_FC WW8PLCFx_PCD::AktPieceStartCp2Fc( WW8_CP nCp )
{
    WW8_CP nCpStart, nCpEnd;
    void* pData;

    if ( !pPcdI->Get(nCpStart, nCpEnd, pData) )
    {
        OSL_ENSURE( !this, "AktPieceStartCp2Fc() with false Cp found (1)" );
        return WW8_FC_MAX;
    }

    OSL_ENSURE( nCp >= nCpStart && nCp < nCpEnd,
        "AktPieceCp2Fc() with false Cp found (2)" );

    if( nCp < nCpStart )
        nCp = nCpStart;
    if( nCp >= nCpEnd )
        nCp = nCpEnd - 1;

    bool bIsUnicode = false;
    WW8_FC nFC = SVBT32ToUInt32( ((WW8_PCD*)pData)->fc );
    if( !bVer67 )
        nFC = WW8PLCFx_PCD::TransformPieceAddress( nFC, bIsUnicode );

    return nFC + (nCp - nCpStart) * (bIsUnicode ? 2 : 1);
}


void WW8PLCFx_PCD::AktPieceFc2Cp( WW8_CP& rStartPos, WW8_CP& rEndPos,
    const WW8ScannerBase *pSBase )
{
    
    if ((rStartPos == WW8_CP_MAX) && (rEndPos == WW8_CP_MAX))
        return;

    rStartPos = pSBase->WW8Fc2Cp( rStartPos );
    rEndPos = pSBase->WW8Fc2Cp( rEndPos );
}

WW8_CP WW8PLCFx_PCD::AktPieceStartFc2Cp( WW8_FC nStartPos )
{
    WW8_CP nCpStart, nCpEnd;
    void* pData;
    if ( !pPcdI->Get( nCpStart, nCpEnd, pData ) )
    {
        OSL_ENSURE( !this, "AktPieceStartFc2Cp() - error" );
        return WW8_CP_MAX;
    }
    bool bIsUnicode = false;
    sal_Int32 nFcStart  = SVBT32ToUInt32( ((WW8_PCD*)pData)->fc );
    if( !bVer67 )
        nFcStart = WW8PLCFx_PCD::TransformPieceAddress( nFcStart, bIsUnicode );

    sal_Int32 nUnicodeFactor = bIsUnicode ? 2 : 1;

    if( nStartPos < nFcStart )
        nStartPos = nFcStart;

    if( nStartPos >= nFcStart + (nCpEnd - nCpStart)     * nUnicodeFactor )
        nStartPos  = nFcStart + (nCpEnd - nCpStart - 1) * nUnicodeFactor;

    return nCpStart + (nStartPos - nFcStart) / nUnicodeFactor;
}



short WW8_BRC::DetermineBorderProperties(bool bVer67, short *pSpace,
    sal_uInt8 *pCol, short *pIdx) const
{
    /*
        Word does not factor the width of the border into the width/height
        stored in the information for graphic/table/object widths, so we need
        to figure out this extra width here and utilize the returned size in
        our calculations
    */
    short nMSTotalWidth;
    sal_uInt8 nCol;
    short nIdx,nSpace;
    if( bVer67 )
    {
        sal_uInt16 aBrc1 = SVBT16ToShort(aBits1);
        nCol = static_cast< sal_uInt8 >((aBrc1 >> 6) & 0x1f);   
        nSpace = (aBrc1 & 0xF800) >> 11;

        nMSTotalWidth = aBrc1 & 0x07;
        nIdx = (aBrc1 & 0x18) >> 3;
        
        if (nMSTotalWidth > 5)
        {
            nIdx = nMSTotalWidth;
            nMSTotalWidth=1;
        }
        nMSTotalWidth = nMSTotalWidth * nIdx * 15;
    }
    else
    {
        nIdx = aBits1[1];
        nCol = aBits2[0];   
        nSpace = aBits2[1] & 0x1F; 

        
        nMSTotalWidth  = aBits1[ 0 ] * 20 / 8;

        
        switch (nIdx)
        {
            
            
            case 2:
            case 4:
            case 5:
            case 22:
                OSL_FAIL("Can't create these from the menus, please report");
            default:
            case 23:    
                break;
            case 10:
                /*
                triple line is five times the width of an ordinary line,
                except that the smallest 1/4 point size appears to have
                exactly the same total border width as a 3/4 point size
                ordinary line, i.e. three times the nominal line width.  The
                second smallest 1/2 point size appears to have exactly the
                total border width as a 2 1/4 border, i.e 4.5 times the size.
                */
                if (nMSTotalWidth == 5)
                    nMSTotalWidth*=3;
                else if (nMSTotalWidth == 10)
                    nMSTotalWidth = nMSTotalWidth*9/2;
                else
                    nMSTotalWidth*=5;
                break;
            case 20:
                /*
                wave, the dimensions appear to be created by the drawing of
                the wave, so we have only two possibilites in the menus, 3/4
                point is equal to solid 3 point. This calculation seems to
                match well to results.
                */
                nMSTotalWidth +=45;
                break;
            case 21:
                /*
                double wave, the dimensions appear to be created by the
                drawing of the wave, so we have only one possibilites in the
                menus, that of 3/4 point is equal to solid 3 point. This
                calculation seems to match well to results.
                */
                nMSTotalWidth += 45*2;
                break;
        }
    }

    if (pIdx)
        *pIdx = nIdx;
    if (pSpace)
        *pSpace = nSpace*20;
    if (pCol)
        *pCol = nCol;
    return nMSTotalWidth;
}

/*
 * WW8Cp2Fc is a good method, a CP always maps to a FC
 * WW8Fc2Cp on the other hand is more dubious, a random FC
 * may not map to a valid CP. Try and avoid WW8Fc2Cp where
 * possible
 */
WW8_CP WW8ScannerBase::WW8Fc2Cp( WW8_FC nFcPos ) const
{
    WW8_CP nFallBackCpEnd = WW8_CP_MAX;
    if( nFcPos == WW8_FC_MAX )
        return nFallBackCpEnd;

    bool bIsUnicode;
    if (pWw8Fib->nVersion >= 8)
        bIsUnicode = false;
    else
        bIsUnicode = pWw8Fib->fExtChar ? true : false;

    if( pPieceIter )    
    {
        sal_uLong nOldPos = pPieceIter->GetIdx();

        for (pPieceIter->SetIdx(0);
            pPieceIter->GetIdx() < pPieceIter->GetIMax(); pPieceIter->advance())
        {
            WW8_CP nCpStart, nCpEnd;
            void* pData;
            if( !pPieceIter->Get( nCpStart, nCpEnd, pData ) )
            {   
                OSL_ENSURE( !this, "PLCFpcd-WW8Fc2Cp() went wrong" );
                break;
            }
            sal_Int32 nFcStart  = SVBT32ToUInt32( ((WW8_PCD*)pData)->fc );
            if (pWw8Fib->nVersion >= 8)
            {
                nFcStart = WW8PLCFx_PCD::TransformPieceAddress( nFcStart,
                                                                bIsUnicode );
            }
            else
            {
                bIsUnicode = pWw8Fib->fExtChar ? true : false;
            }
            sal_Int32 nLen = (nCpEnd - nCpStart) * (bIsUnicode ? 2 : 1);

            /*
            If this cp is inside this piece, or its the last piece and we are
            on the very last cp of that piece
            */
            if (nFcPos >= nFcStart)
            {
                
                WW8_CP nTempCp =
                    nCpStart + ((nFcPos - nFcStart) / (bIsUnicode ? 2 : 1));
                if (nFcPos < nFcStart + nLen)
                {
                    pPieceIter->SetIdx( nOldPos );
                    return nTempCp;
                }
                else if (nFcPos == nFcStart + nLen)
                {
                    
                    
                    nFallBackCpEnd = nTempCp;
                }
            }
        }
        
        pPieceIter->SetIdx( nOldPos );      
        /*
        If it was not found, then this is because it has fallen between two
        stools, i.e. either it is the last cp/fc of the last piece, or it is
        the last cp/fc of a disjoint piece.
        */
        return nFallBackCpEnd;
    }

    
    if (!bIsUnicode)
        nFallBackCpEnd = (nFcPos - pWw8Fib->fcMin);
    else
        nFallBackCpEnd = (nFcPos - pWw8Fib->fcMin + 1) / 2;

    return nFallBackCpEnd;
}

WW8_FC WW8ScannerBase::WW8Cp2Fc(WW8_CP nCpPos, bool* pIsUnicode,
    WW8_CP* pNextPieceCp, bool* pTestFlag) const
{
    if( pTestFlag )
        *pTestFlag = true;
    if( WW8_CP_MAX == nCpPos )
        return WW8_CP_MAX;

    bool bIsUnicode;
    if( !pIsUnicode )
        pIsUnicode = &bIsUnicode;

    if (pWw8Fib->nVersion >= 8)
        *pIsUnicode = false;
    else
        *pIsUnicode = pWw8Fib->fExtChar ? true : false;

    if( pPieceIter )
    {
        
        if( pNextPieceCp )
            *pNextPieceCp = WW8_CP_MAX;

        if( !pPieceIter->SeekPos( nCpPos ) )
        {
            if( pTestFlag )
                *pTestFlag = false;
            else {
                OSL_ENSURE( !this, "Handed over wrong CP to WW8Cp2Fc()" );
            }
            return WW8_FC_MAX;
        }
        WW8_CP nCpStart, nCpEnd;
        void* pData;
        if( !pPieceIter->Get( nCpStart, nCpEnd, pData ) )
        {
            if( pTestFlag )
                *pTestFlag = false;
            else {
                OSL_ENSURE( !this, "PLCFfpcd-Get went wrong" );
            }
            return WW8_FC_MAX;
        }
        if( pNextPieceCp )
            *pNextPieceCp = nCpEnd;

        WW8_FC nRet = SVBT32ToUInt32( ((WW8_PCD*)pData)->fc );
        if (pWw8Fib->nVersion >= 8)
            nRet = WW8PLCFx_PCD::TransformPieceAddress( nRet, *pIsUnicode );
        else
            *pIsUnicode = pWw8Fib->fExtChar ? true : false;

        nRet += (nCpPos - nCpStart) * (*pIsUnicode ? 2 : 1);

        return nRet;
    }

    
    return pWw8Fib->fcMin + nCpPos * (*pIsUnicode ? 2 : 1);
}


WW8PLCFpcd* WW8ScannerBase::OpenPieceTable( SvStream* pStr, const WW8Fib* pWwF )
{
    if ( ((8 > pWw8Fib->nVersion) && !pWwF->fComplex) || !pWwF->lcbClx )
        return NULL;

    WW8_FC nClxPos = pWwF->fcClx;
    sal_Int32 nClxLen = pWwF->lcbClx;
    sal_Int32 nLeft = nClxLen;
    sal_Int16 nGrpprl = 0;

    if (!checkSeek(*pStr, nClxPos))
        return NULL;

    while( true ) 
    {
        sal_uInt8 clxt(2);
        pStr->ReadUChar( clxt );
        nLeft--;
        if( 2 == clxt )                         
            break;                              
        if( 1 == clxt )                         
            nGrpprl++;
        sal_uInt16 nLen(0);
        pStr->ReadUInt16( nLen );
        nLeft -= 2 + nLen;
        if( nLeft < 0 )
            return NULL;                        
        pStr->SeekRel( nLen );                  
    }

    if (!checkSeek(*pStr, nClxPos))
        return NULL;

    nLeft = nClxLen;
    pPieceGrpprls = new sal_uInt8*[nGrpprl + 1];
    memset( pPieceGrpprls, 0, ( nGrpprl + 1 ) * sizeof(sal_uInt8 *) );
    nPieceGrpprls = nGrpprl;
    sal_Int16 nAktGrpprl = 0;                       
    while( true )
    {
        sal_uInt8 clxt(2);
        pStr->ReadUChar( clxt );
        nLeft--;
        if( 2 == clxt)                          
            break;                              
        sal_uInt16 nLen(0);
        pStr->ReadUInt16( nLen );
        nLeft -= 2 + nLen;
        if( nLeft < 0 )
            return NULL;                        
        if( 1 == clxt )                         
        {
            sal_uInt8* p = new sal_uInt8[nLen+2];         
            ShortToSVBT16(nLen, p);             
            if (!checkRead(*pStr, p+2, nLen))   
            {
                delete[] p;
                return NULL;
            }
            pPieceGrpprls[nAktGrpprl++] = p;    
        }
        else
            pStr->SeekRel( nLen );              
    }
    
    sal_Int32 nPLCFfLen(0);
    if (pWwF->GetFIBVersion() <= ww::eWW2)
    {
        sal_Int16 nWordTwoLen(0);
        pStr->ReadInt16( nWordTwoLen );
        nPLCFfLen = nWordTwoLen;
    }
    else
        pStr->ReadInt32( nPLCFfLen );
    OSL_ENSURE( 65536 > nPLCFfLen, "PLCFfpcd above 64 k" );
    return new WW8PLCFpcd( pStr, pStr->Tell(), nPLCFfLen, 8 );
}

void WW8ScannerBase::DeletePieceTable()
{
    if( pPieceGrpprls )
    {
        for( sal_uInt8** p = pPieceGrpprls; *p; p++ )
            delete[] (*p);
        delete[] pPieceGrpprls;
        pPieceGrpprls = 0;
    }
}

WW8ScannerBase::WW8ScannerBase( SvStream* pSt, SvStream* pTblSt,
    SvStream* pDataSt, WW8Fib* pWwFib )
    : pWw8Fib(pWwFib), pMainFdoa(0), pHdFtFdoa(0), pMainTxbx(0),
    pMainTxbxBkd(0), pHdFtTxbx(0), pHdFtTxbxBkd(0), pMagicTables(0),
    pSubdocs(0), pExtendedAtrds(0), pPieceGrpprls(0)
{
    pPiecePLCF = OpenPieceTable( pTblSt, pWw8Fib );             
    if( pPiecePLCF )
    {
        pPieceIter = new WW8PLCFpcd_Iter( *pPiecePLCF );
        pPLCFx_PCD = new WW8PLCFx_PCD(pWwFib->GetFIBVersion(), pPiecePLCF, 0,
            IsSevenMinus(pWw8Fib->GetFIBVersion()));
        pPLCFx_PCDAttrs = new WW8PLCFx_PCDAttrs(pWwFib->GetFIBVersion(),
            pPLCFx_PCD, this);
    }
    else
    {
        pPieceIter = 0;
        pPLCFx_PCD = 0;
        pPLCFx_PCDAttrs = 0;
    }

    
    pChpPLCF = new WW8PLCFx_Cp_FKP( pSt, pTblSt, pDataSt, *this, CHP ); 
    pPapPLCF = new WW8PLCFx_Cp_FKP( pSt, pTblSt, pDataSt, *this, PAP ); 

    pSepPLCF = new WW8PLCFx_SEPX(   pSt, pTblSt, *pWwFib, 0 );          

    
    pFtnPLCF = new WW8PLCFx_SubDoc( pTblSt, pWwFib->GetFIBVersion(), 0,
        pWwFib->fcPlcffndRef, pWwFib->lcbPlcffndRef, pWwFib->fcPlcffndTxt,
        pWwFib->lcbPlcffndTxt, 2 );
    
    pEdnPLCF = new WW8PLCFx_SubDoc( pTblSt, pWwFib->GetFIBVersion(), 0,
        pWwFib->fcPlcfendRef, pWwFib->lcbPlcfendRef, pWwFib->fcPlcfendTxt,
        pWwFib->lcbPlcfendTxt, 2 );
    
    pAndPLCF = new WW8PLCFx_SubDoc( pTblSt, pWwFib->GetFIBVersion(), 0,
        pWwFib->fcPlcfandRef, pWwFib->lcbPlcfandRef, pWwFib->fcPlcfandTxt,
        pWwFib->lcbPlcfandTxt, IsSevenMinus(pWwFib->GetFIBVersion()) ? 20 : 30);

    
    pFldPLCF    = new WW8PLCFx_FLD(pTblSt, *pWwFib, MAN_MAINTEXT);
    
    pFldHdFtPLCF= new WW8PLCFx_FLD(pTblSt, *pWwFib, MAN_HDFT);
    
    pFldFtnPLCF = new WW8PLCFx_FLD(pTblSt, *pWwFib, MAN_FTN);
    
    pFldEdnPLCF = new WW8PLCFx_FLD(pTblSt, *pWwFib, MAN_EDN);
    
    pFldAndPLCF = new WW8PLCFx_FLD(pTblSt, *pWwFib, MAN_AND);
    
    pFldTxbxPLCF= new WW8PLCFx_FLD(pTblSt, *pWwFib, MAN_TXBX);
    
    pFldTxbxHdFtPLCF = new WW8PLCFx_FLD(pTblSt,*pWwFib,MAN_TXBX_HDFT);

    
    switch( pWw8Fib->nVersion )
    {
        case 6:
        case 7:
            if( pWwFib->fcPlcfdoaMom && pWwFib->lcbPlcfdoaMom )
            {
                pMainFdoa = new WW8PLCFspecial( pTblSt, pWwFib->fcPlcfdoaMom,
                    pWwFib->lcbPlcfdoaMom, 6 );
            }
            if( pWwFib->fcPlcfdoaHdr && pWwFib->lcbPlcfdoaHdr )
            {
                pHdFtFdoa = new WW8PLCFspecial( pTblSt, pWwFib->fcPlcfdoaHdr,
                pWwFib->lcbPlcfdoaHdr, 6 );
            }
            break;
        case 8:
            if( pWwFib->fcPlcfspaMom && pWwFib->lcbPlcfspaMom )
            {
                pMainFdoa = new WW8PLCFspecial( pTblSt, pWwFib->fcPlcfspaMom,
                    pWwFib->lcbPlcfspaMom, 26 );
            }
            if( pWwFib->fcPlcfspaHdr && pWwFib->lcbPlcfspaHdr )
            {
                pHdFtFdoa = new WW8PLCFspecial( pTblSt, pWwFib->fcPlcfspaHdr,
                    pWwFib->lcbPlcfspaHdr, 26 );
            }
            
            if( pWwFib->fcPlcftxbxBkd && pWwFib->lcbPlcftxbxBkd )
            {
                pMainTxbxBkd = new WW8PLCFspecial( pTblSt,
                    pWwFib->fcPlcftxbxBkd, pWwFib->lcbPlcftxbxBkd, 0);
            }
            
            if( pWwFib->fcPlcfHdrtxbxBkd && pWwFib->lcbPlcfHdrtxbxBkd )
            {
                pHdFtTxbxBkd = new WW8PLCFspecial( pTblSt,
                    pWwFib->fcPlcfHdrtxbxBkd, pWwFib->lcbPlcfHdrtxbxBkd, 0);
            }
            
            if (pWwFib->fcPlcfTch && pWwFib->lcbPlcfTch)
            {
                pMagicTables = new WW8PLCFspecial( pTblSt,
                    pWwFib->fcPlcfTch, pWwFib->lcbPlcfTch, 4);
            }
            
            if (pWwFib->fcPlcfwkb && pWwFib->lcbPlcfwkb)
            {
                pSubdocs = new WW8PLCFspecial( pTblSt,
                    pWwFib->fcPlcfwkb, pWwFib->lcbPlcfwkb, 12);
            }
            
            if (pWwFib->fcAtrdExtra && pWwFib->lcbAtrdExtra)
            {
                sal_Size nOldPos = pTblSt->Tell();
                if (checkSeek(*pTblSt, pWwFib->fcAtrdExtra) && (pTblSt->remainingSize() >= pWwFib->lcbAtrdExtra))
                {
                    pExtendedAtrds = new sal_uInt8[pWwFib->lcbAtrdExtra];
                    pWwFib->lcbAtrdExtra = pTblSt->Read(pExtendedAtrds, pWwFib->lcbAtrdExtra);
                }
                else
                    pWwFib->lcbAtrdExtra = 0;
                pTblSt->Seek(nOldPos);
            }

            break;
        default:
            OSL_ENSURE( !this, "Es wurde vergessen, nVersion zu kodieren!" );
            break;
    }

    
    sal_uInt32 nLenTxBxS = (8 > pWw8Fib->nVersion) ? 0 : 22;
    if( pWwFib->fcPlcftxbxTxt && pWwFib->lcbPlcftxbxTxt )
    {
        pMainTxbx = new WW8PLCFspecial( pTblSt, pWwFib->fcPlcftxbxTxt,
            pWwFib->lcbPlcftxbxTxt, nLenTxBxS );
    }

    
    if( pWwFib->fcPlcfHdrtxbxTxt && pWwFib->lcbPlcfHdrtxbxTxt )
    {
        pHdFtTxbx = new WW8PLCFspecial( pTblSt, pWwFib->fcPlcfHdrtxbxTxt,
            pWwFib->lcbPlcfHdrtxbxTxt, nLenTxBxS );
    }

    pBook = new WW8PLCFx_Book(pTblSt, *pWwFib);
}

WW8ScannerBase::~WW8ScannerBase()
{
    DeletePieceTable();
    delete pPLCFx_PCDAttrs;
    delete pPLCFx_PCD;
    delete pPieceIter;
    delete pPiecePLCF;
    delete pBook;
    delete pFldEdnPLCF;
    delete pFldFtnPLCF;
    delete pFldAndPLCF;
    delete pFldHdFtPLCF;
    delete pFldPLCF;
    delete pFldTxbxPLCF;
    delete pFldTxbxHdFtPLCF;
    delete pEdnPLCF;
    delete pFtnPLCF;
    delete pAndPLCF;
    delete pSepPLCF;
    delete pPapPLCF;
    delete pChpPLCF;
    
    delete pMainFdoa;
    delete pHdFtFdoa;
    delete pMainTxbx;
    delete pMainTxbxBkd;
    delete pHdFtTxbx;
    delete pHdFtTxbxBkd;
    delete pMagicTables;
    delete pSubdocs;
    delete [] pExtendedAtrds;
}



static bool WW8SkipField(WW8PLCFspecial& rPLCF)
{
    void* pData;
    WW8_CP nP;

    if (!rPLCF.Get(nP, pData))              
        return false;

    rPLCF.advance();

    if((((sal_uInt8*)pData)[0] & 0x1f ) != 0x13 )    
        return true;                            

    if( !rPLCF.Get( nP, pData ) )
        return false;


    while((((sal_uInt8*)pData)[0] & 0x1f ) == 0x13 )
    {
        
        WW8SkipField( rPLCF );              
        if( !rPLCF.Get( nP, pData ) )
            return false;
    }

    if((((sal_uInt8*)pData)[0] & 0x1f ) == 0x14 )
    {

        
        rPLCF.advance();

        if( !rPLCF.Get( nP, pData ) )
            return false;

        while ((((sal_uInt8*)pData)[0] & 0x1f ) == 0x13)
        {
            
            WW8SkipField( rPLCF );          
            if( !rPLCF.Get( nP, pData ) )
                return false;
        }
    }
    rPLCF.advance();

    return true;
}

static bool WW8GetFieldPara(WW8PLCFspecial& rPLCF, WW8FieldDesc& rF)
{
    void* pData;
    sal_uLong nOldIdx = rPLCF.GetIdx();

    rF.nLen = rF.nId = rF.nOpt = rF.bCodeNest = rF.bResNest = false;

    if( !rPLCF.Get( rF.nSCode, pData ) )             
        goto Err;

    rPLCF.advance();

    if((((sal_uInt8*)pData)[0] & 0x1f ) != 0x13 )        
        goto Err;

    rF.nId = ((sal_uInt8*)pData)[1];

    if( !rPLCF.Get( rF.nLCode, pData ) )
        goto Err;

    rF.nSRes = rF.nLCode;                           
    rF.nSCode++;                                    
    rF.nLCode -= rF.nSCode;                         

    while((((sal_uInt8*)pData)[0] & 0x1f ) == 0x13 )
    {
        
        WW8SkipField( rPLCF );              
        rF.bCodeNest = true;
        if( !rPLCF.Get( rF.nSRes, pData ) )
            goto Err;
    }

    if ((((sal_uInt8*)pData)[0] & 0x1f ) == 0x14 )       
    {
        rPLCF.advance();

        if( !rPLCF.Get( rF.nLRes, pData ) )
            goto Err;

        while((((sal_uInt8*)pData)[0] & 0x1f ) == 0x13 )
        {
            
            WW8SkipField( rPLCF );              
            rF.bResNest = true;
            if( !rPLCF.Get( rF.nLRes, pData ) )
                goto Err;
        }
        rF.nLen = rF.nLRes - rF.nSCode + 2;         
        rF.nLRes -= rF.nSRes;                       
        rF.nSRes++;                                 
        rF.nLRes--;
    }else{
        rF.nLRes = 0;                               
        rF.nLen = rF.nSRes - rF.nSCode + 2;         
    }

    rPLCF.advance();
    if((((sal_uInt8*)pData)[0] & 0x1f ) == 0x15 )
    {
        
        
        rF.nOpt = ((sal_uInt8*)pData)[1];                
    }else{
        rF.nId = 0;                                      
    }

    rPLCF.SetIdx( nOldIdx );
    return true;
Err:
    rPLCF.SetIdx( nOldIdx );
    return false;
}

OUString read_uInt8_BeltAndBracesString(SvStream& rStrm, rtl_TextEncoding eEnc)
{
    OUString aRet = read_uInt8_lenPrefixed_uInt8s_ToOUString(rStrm, eEnc);
    rStrm.SeekRel(sizeof(sal_uInt8)); 
    return aRet;
}

OUString read_uInt16_BeltAndBracesString(SvStream& rStrm)
{
    OUString aRet = read_uInt16_PascalString(rStrm);
    rStrm.SeekRel(sizeof(sal_Unicode)); 
    return aRet;
}

sal_Int32 WW8ScannerBase::WW8ReadString( SvStream& rStrm, OUString& rStr,
    WW8_CP nAktStartCp, long nTotalLen, rtl_TextEncoding eEnc ) const
{
    
    rStr = OUString();

    long nTotalRead = 0;
    WW8_CP nBehindTextCp = nAktStartCp + nTotalLen;
    WW8_CP nNextPieceCp  = nBehindTextCp; 
    do
    {
        bool bIsUnicode, bPosOk;
        WW8_FC fcAct = WW8Cp2Fc(nAktStartCp,&bIsUnicode,&nNextPieceCp,&bPosOk);

        
        if( !bPosOk )
            break;

        rStrm.Seek( fcAct );

        long nLen = ( (nNextPieceCp < nBehindTextCp) ? nNextPieceCp
            : nBehindTextCp ) - nAktStartCp;

        if( 0 >= nLen )
            break;

        if( nLen > USHRT_MAX - 1 )
            nLen = USHRT_MAX - 1;

        rStr += bIsUnicode
             ? read_uInt16s_ToOUString(rStrm, nLen)
             : read_uInt8s_ToOUString(rStrm, nLen, eEnc);

        nTotalRead  += nLen;
        nAktStartCp += nLen;
        if ( nTotalRead != rStr.getLength() )
            break;
    }
    while( nTotalRead < nTotalLen );

    return rStr.getLength();
}

WW8PLCFspecial::WW8PLCFspecial(SvStream* pSt, sal_uInt32 nFilePos,
    sal_uInt32 nPLCF, sal_uInt32 nStruct)
    : nIdx(0), nStru(nStruct)
{
    const sal_uInt32 nValidMin=4;

    sal_Size nOldPos = pSt->Tell();

    bool bValid = checkSeek(*pSt, nFilePos);
    sal_Size nRemainingSize = pSt->remainingSize();
    if( !(nRemainingSize >= nValidMin && nPLCF >= nValidMin ))
        bValid = false;
    nPLCF = bValid ? std::min(nRemainingSize, static_cast<sal_Size>(nPLCF)) : nValidMin;

    
    pPLCF_PosArray = new sal_Int32[ ( nPLCF + 3 ) / 4 ];
    pPLCF_PosArray[0] = 0;

    nPLCF = bValid ? pSt->Read(pPLCF_PosArray, nPLCF) : nValidMin;

    nPLCF = std::max(nPLCF, nValidMin);

    nIMax = ( nPLCF - 4 ) / ( 4 + nStruct );
#ifdef OSL_BIGENDIAN
    for( nIdx = 0; nIdx <= nIMax; nIdx++ )
        pPLCF_PosArray[nIdx] = OSL_SWAPDWORD( pPLCF_PosArray[nIdx] );
    nIdx = 0;
#endif 
    if( nStruct ) 
        pPLCF_Contents = (sal_uInt8*)&pPLCF_PosArray[nIMax + 1];
    else
        pPLCF_Contents = 0;                         

    pSt->Seek(nOldPos);
}





bool WW8PLCFspecial::SeekPos(long nP)
{
    if( nP < pPLCF_PosArray[0] )
    {
        nIdx = 0;
        return false;   
    }

    
    if( (1 > nIdx) || (nP < pPLCF_PosArray[ nIdx-1 ]) )
        nIdx = 1;

    long nI   = nIdx ? nIdx : 1;
    long nEnd = nIMax;

    for(int n = (1==nIdx ? 1 : 2); n; --n )
    {
        for( ; nI <=nEnd; ++nI)
        {                                   
            if( nP < pPLCF_PosArray[nI] )
            {                               
                nIdx = nI - 1;              
                return true;                
            }
        }
        nI   = 1;
        nEnd = nIdx-1;
    }
    nIdx = nIMax;               
    return false;
}




bool WW8PLCFspecial::SeekPosExact(long nP)
{
    if( nP < pPLCF_PosArray[0] )
    {
        nIdx = 0;
        return false;       
    }
    
    if( nP <=pPLCF_PosArray[nIdx] )
        nIdx = 0;

    long nI   = nIdx ? nIdx-1 : 0;
    long nEnd = nIMax;

    for(int n = (0==nIdx ? 1 : 2); n; --n )
    {
        for( ; nI < nEnd; ++nI)
        {
            if( nP <=pPLCF_PosArray[nI] )
            {                           
                nIdx = nI;              
                return true;            
            }
        }
        nI   = 0;
        nEnd = nIdx;
    }
    nIdx = nIMax;               
    return false;
}

bool WW8PLCFspecial::Get(WW8_CP& rPos, void*& rpValue) const
{
    return GetData( nIdx, rPos, rpValue );
}

bool WW8PLCFspecial::GetData(long nInIdx, WW8_CP& rPos, void*& rpValue) const
{
    if ( nInIdx >= nIMax )
    {
        rPos = WW8_CP_MAX;
        return false;
    }
    rPos = pPLCF_PosArray[nInIdx];
    rpValue = pPLCF_Contents ? (void*)&pPLCF_Contents[nInIdx * nStru] : 0;
    return true;
}




WW8PLCF::WW8PLCF(SvStream& rSt, WW8_FC nFilePos, sal_Int32 nPLCF, int nStruct,
    WW8_CP nStartPos) : pPLCF_PosArray(0), nIdx(0), nStru(nStruct)
{
    OSL_ENSURE( nPLCF, "WW8PLCF: nPLCF is zero!" );

    nIMax = ( nPLCF - 4 ) / ( 4 + nStruct );

    ReadPLCF(rSt, nFilePos, nPLCF);

    if( nStartPos >= 0 )
        SeekPos( nStartPos );
}






WW8PLCF::WW8PLCF(SvStream& rSt, WW8_FC nFilePos, sal_Int32 nPLCF, int nStruct,
    WW8_CP nStartPos, sal_Int32 nPN, sal_Int32 ncpN): pPLCF_PosArray(0), nIdx(0),
    nStru(nStruct)
{
    nIMax = ( nPLCF - 4 ) / ( 4 + nStruct );

    if( nIMax >= ncpN )
        ReadPLCF(rSt, nFilePos, nPLCF);
    else
        GeneratePLCF(rSt, nPN, ncpN);

    if( nStartPos >= 0 )
        SeekPos( nStartPos );
}

void WW8PLCF::ReadPLCF(SvStream& rSt, WW8_FC nFilePos, sal_uInt32 nPLCF)
{
    sal_Size nOldPos = rSt.Tell();
    bool bValid = checkSeek(rSt, nFilePos) && (rSt.remainingSize() >= nPLCF);

    if (bValid)
    {
        
        pPLCF_PosArray = new WW8_CP[ ( nPLCF + 3 ) / 4 ];
        bValid = checkRead(rSt, pPLCF_PosArray, nPLCF);
    }

    if (bValid)
    {
#ifdef OSL_BIGENDIAN
        for( nIdx = 0; nIdx <= nIMax; nIdx++ )
            pPLCF_PosArray[nIdx] = OSL_SWAPDWORD( pPLCF_PosArray[nIdx] );
        nIdx = 0;
#endif 
        
        pPLCF_Contents = (sal_uInt8*)&pPLCF_PosArray[nIMax + 1];
    }

    OSL_ENSURE(bValid, "Document has corrupt PLCF, ignoring it");

    if (!bValid)
        MakeFailedPLCF();

    rSt.Seek(nOldPos);
}

void WW8PLCF::MakeFailedPLCF()
{
    nIMax = 0;
    delete[] pPLCF_PosArray;
    pPLCF_PosArray = new sal_Int32[2];
    pPLCF_PosArray[0] = pPLCF_PosArray[1] = WW8_CP_MAX;
    pPLCF_Contents = (sal_uInt8*)&pPLCF_PosArray[nIMax + 1];
}

void WW8PLCF::GeneratePLCF(SvStream& rSt, sal_Int32 nPN, sal_Int32 ncpN)
{
    OSL_ENSURE( nIMax < ncpN, "Pcl.Fkp: Why is PLCF too big?" );

    bool failure = false;
    nIMax = ncpN;

    if ((nIMax < 1) || (nIMax > (WW8_CP_MAX - 4)/6) || ((nPN + ncpN) > USHRT_MAX))
        failure = true;

    if (!failure)
    {
        size_t nSiz = 6 * nIMax + 4;
        size_t nElems = ( nSiz + 3 ) / 4;
        pPLCF_PosArray = new sal_Int32[ nElems ]; 

        for (sal_Int32 i = 0; i < ncpN && !failure; ++i)
        {
            failure = true;
            
            
            if (checkSeek(rSt, ( nPN + i ) << 9 ))
                continue;
            WW8_CP nFc(0);
            rSt.ReadInt32( nFc );
            pPLCF_PosArray[i] = nFc;
            failure = rSt.GetError();
        }
    }

    if (!failure)
    {
        do
        {
            failure = true;

            sal_Size nLastFkpPos = ( ( nPN + nIMax - 1 ) << 9 );
            
            if (!checkSeek(rSt, nLastFkpPos + 511))
                break;

            sal_uInt8 nb(0);
            rSt.ReadUChar( nb );
            
            if (!checkSeek(rSt, nLastFkpPos + nb * 4))
                break;

            WW8_CP nFc(0);
            rSt.ReadInt32( nFc );
            pPLCF_PosArray[nIMax] = nFc;        

            failure = rSt.GetError();
        } while(false);
    }

    if (!failure)
    {
        
        pPLCF_Contents = (sal_uInt8*)&pPLCF_PosArray[nIMax + 1];
        sal_uInt8* p = pPLCF_Contents;

        for (sal_Int32 i = 0; i < ncpN; ++i)         
        {
            ShortToSVBT16(static_cast<sal_uInt16>(nPN + i), p);
            p+=2;
        }
    }

    OSL_ENSURE( !failure, "Document has corrupt PLCF, ignoring it" );

    if (failure)
        MakeFailedPLCF();
}

bool WW8PLCF::SeekPos(WW8_CP nPos)
{
    WW8_CP nP = nPos;

    if( nP < pPLCF_PosArray[0] )
    {
        nIdx = 0;
        
        return false;
    }

    
    if( (1 > nIdx) || (nP < pPLCF_PosArray[ nIdx-1 ]) )
        nIdx = 1;

    sal_Int32 nI   = nIdx ? nIdx : 1;
    sal_Int32 nEnd = nIMax;

    for(int n = (1==nIdx ? 1 : 2); n; --n )
    {
        for( ; nI <=nEnd; ++nI)             
        {
            if( nP < pPLCF_PosArray[nI] )   
            {
                nIdx = nI - 1;              
                return true;                
            }
        }
        nI   = 1;
        nEnd = nIdx-1;
    }

    nIdx = nIMax;               
    return false;
}

bool WW8PLCF::Get(WW8_CP& rStart, WW8_CP& rEnd, void*& rpValue) const
{
    if ( nIdx >= nIMax )
    {
        rStart = rEnd = WW8_CP_MAX;
        return false;
    }
    rStart = pPLCF_PosArray[ nIdx ];
    rEnd   = pPLCF_PosArray[ nIdx + 1 ];
    rpValue = (void*)&pPLCF_Contents[nIdx * nStru];
    return true;
}

WW8_CP WW8PLCF::Where() const
{
    if ( nIdx >= nIMax )
        return WW8_CP_MAX;

    return pPLCF_PosArray[nIdx];
}

WW8PLCFpcd::WW8PLCFpcd(SvStream* pSt, sal_uInt32 nFilePos,
    sal_uInt32 nPLCF, sal_uInt32 nStruct)
    : nStru( nStruct )
{
    const sal_uInt32 nValidMin=4;

    sal_Size nOldPos = pSt->Tell();

    bool bValid = checkSeek(*pSt, nFilePos);
    sal_Size nRemainingSize = pSt->remainingSize();
    if( !(nRemainingSize >= nValidMin && nPLCF >= nValidMin ))
        bValid = false;
    nPLCF = bValid ? std::min(nRemainingSize, static_cast<sal_Size>(nPLCF)) : nValidMin;

    pPLCF_PosArray = new sal_Int32[ ( nPLCF + 3 ) / 4 ];    
    pPLCF_PosArray[0] = 0;

    nPLCF = bValid ? pSt->Read(pPLCF_PosArray, nPLCF) : nValidMin;
    nPLCF = std::max(nPLCF, nValidMin);

    nIMax = ( nPLCF - 4 ) / ( 4 + nStruct );
#ifdef OSL_BIGENDIAN
    for( long nI = 0; nI <= nIMax; nI++ )
      pPLCF_PosArray[nI] = OSL_SWAPDWORD( pPLCF_PosArray[nI] );
#endif 

    
    pPLCF_Contents = (sal_uInt8*)&pPLCF_PosArray[nIMax + 1];

    pSt->Seek( nOldPos );
}


WW8PLCFpcd_Iter::WW8PLCFpcd_Iter( WW8PLCFpcd& rPLCFpcd, long nStartPos )
    :rPLCF( rPLCFpcd ), nIdx( 0 )
{
    if( nStartPos >= 0 )
        SeekPos( nStartPos );
}

bool WW8PLCFpcd_Iter::SeekPos(long nPos)
{
    long nP = nPos;

    if( nP < rPLCF.pPLCF_PosArray[0] )
    {
        nIdx = 0;
        return false;       
    }
    
    if( (1 > nIdx) || (nP < rPLCF.pPLCF_PosArray[ nIdx-1 ]) )
        nIdx = 1;

    long nI   = nIdx ? nIdx : 1;
    long nEnd = rPLCF.nIMax;

    for(int n = (1==nIdx ? 1 : 2); n; --n )
    {
        for( ; nI <=nEnd; ++nI)
        {                               
            if( nP < rPLCF.pPLCF_PosArray[nI] )
            {                           
                nIdx = nI - 1;          
                return true;            
            }
        }
        nI   = 1;
        nEnd = nIdx-1;
    }
    nIdx = rPLCF.nIMax;         
    return false;
}

bool WW8PLCFpcd_Iter::Get(WW8_CP& rStart, WW8_CP& rEnd, void*& rpValue) const
{
    if( nIdx >= rPLCF.nIMax )
    {
        rStart = rEnd = WW8_CP_MAX;
        return false;
    }
    rStart = rPLCF.pPLCF_PosArray[nIdx];
    rEnd = rPLCF.pPLCF_PosArray[nIdx + 1];
    rpValue = (void*)&rPLCF.pPLCF_Contents[nIdx * rPLCF.nStru];
    return true;
}

sal_Int32 WW8PLCFpcd_Iter::Where() const
{
    if ( nIdx >= rPLCF.nIMax )
        return SAL_MAX_INT32;

    return rPLCF.pPLCF_PosArray[nIdx];
}


bool WW8PLCFx_Fc_FKP::WW8Fkp::Entry::operator<
    (const WW8PLCFx_Fc_FKP::WW8Fkp::Entry& rSecond) const
{
    return (mnFC < rSecond.mnFC);
}

bool IsReplaceAllSprm(sal_uInt16 nSpId)
{
    return (0x6645 == nSpId || 0x6646 == nSpId);
}

bool IsExpandableSprm(sal_uInt16 nSpId)
{
    return 0x646B == nSpId;
}

void WW8PLCFx_Fc_FKP::WW8Fkp::FillEntry(WW8PLCFx_Fc_FKP::WW8Fkp::Entry &rEntry,
    sal_Size nDataOffset, sal_uInt16 nLen)
{
    bool bValidPos = (nDataOffset < sizeof(maRawData));

    OSL_ENSURE(bValidPos, "sprm sequence offset is out of range, ignoring");

    if (!bValidPos)
    {
        rEntry.mnLen = 0;
        return;
    }

    sal_uInt16 nAvailableData = sizeof(maRawData)-nDataOffset;
    OSL_ENSURE(nLen <= nAvailableData, "srpm sequence len is out of range, clipping");
    rEntry.mnLen = std::min(nLen, nAvailableData);
    rEntry.mpData = maRawData + nDataOffset;
}

WW8PLCFx_Fc_FKP::WW8Fkp::WW8Fkp(ww::WordVersion eVersion, SvStream* pSt,
    SvStream* pDataSt, long _nFilePos, long nItemSiz, ePLCFT ePl,
    WW8_FC nStartFc)
    : nItemSize(nItemSiz), nFilePos(_nFilePos),  mnIdx(0), ePLCF(ePl),
    maSprmParser(eVersion)
{
    memset(maRawData, 0, 512);

    sal_Size nOldPos = pSt->Tell();

    bool bCouldSeek = checkSeek(*pSt, nFilePos);
    bool bCouldRead = bCouldSeek ? checkRead(*pSt, maRawData, 512) : false;

    mnIMax = bCouldRead ? maRawData[511] : 0;

    sal_uInt8 *pStart = maRawData;
    
    const size_t nRawDataStart = (mnIMax + 1) * 4;

    for (mnIdx = 0; mnIdx < mnIMax; ++mnIdx)
    {
        const size_t nRawDataOffset = nRawDataStart + mnIdx * nItemSize;

        
        if (nRawDataOffset >= 511)
        {
            mnIMax = mnIdx;
            break;
        }

        unsigned int nOfs = maRawData[nRawDataOffset] * 2;

        
        if (nOfs >= 511)
        {
            mnIMax = mnIdx;
            break;
        }

        Entry aEntry(Get_Long(pStart));

        if (nOfs)
        {
            switch (ePLCF)
            {
                case CHP:
                {
                    aEntry.mnLen = maRawData[nOfs];

                    
                    sal_Size nDataOffset = nOfs + 1;

                    FillEntry(aEntry, nDataOffset, aEntry.mnLen);

                    if (aEntry.mnLen && eVersion == ww::eWW2)
                    {
                        Word2CHPX aChpx = ReadWord2Chpx(*pSt, nFilePos + nOfs + 1, static_cast< sal_uInt8 >(aEntry.mnLen));
                        std::vector<sal_uInt8> aSprms = ChpxToSprms(aChpx);
                        aEntry.mnLen = static_cast< sal_uInt16 >(aSprms.size());
                        if (aEntry.mnLen)
                        {
                            aEntry.mpData = new sal_uInt8[aEntry.mnLen];
                            memcpy(aEntry.mpData, &(aSprms[0]), aEntry.mnLen);
                            aEntry.mbMustDelete = true;
                        }
                    }
                    break;
                }
                case PAP:
                    {
                        sal_uInt8 nDelta = 0;

                        aEntry.mnLen = maRawData[nOfs];
                        if (IsEightPlus(eVersion) && !aEntry.mnLen)
                        {
                            aEntry.mnLen = maRawData[nOfs+1];
                            nDelta++;
                        }
                        aEntry.mnLen *= 2;

                        
                        if (eVersion == ww::eWW2)
                        {
                            if (aEntry.mnLen >= 1)
                            {
                                aEntry.mnIStd = *(maRawData+nOfs+1+nDelta);
                                aEntry.mnLen--;  
                                if (aEntry.mnLen >= 6)
                                {
                                    aEntry.mnLen-=6; 
                                    
                                    unsigned int nOffset = nOfs + 8;
                                    if (nOffset >= 511) 
                                        aEntry.mnLen=0;
                                    if (aEntry.mnLen)   
                                    {
                                        if (nOffset + aEntry.mnLen > 512)   
                                            aEntry.mnLen = 512 - nOffset;
                                        aEntry.mpData = maRawData + nOffset;
                                    }
                                }
                                else
                                    aEntry.mnLen=0; 
                            }
                        }
                        else
                        {
                            if (aEntry.mnLen >= 2)
                            {
                                
                                sal_Size nDataOffset = nOfs + 1 + nDelta;
                                aEntry.mnIStd = nDataOffset <= sizeof(maRawData)-sizeof(aEntry.mnIStd) ?
                                    SVBT16ToShort(maRawData+nDataOffset) : 0;
                                aEntry.mnLen-=2; 
                                if (aEntry.mnLen)
                                {
                                    
                                    nDataOffset += sizeof(aEntry.mnIStd);

                                    FillEntry(aEntry, nDataOffset, aEntry.mnLen);
                                }
                            }
                            else
                                aEntry.mnLen=0; 
                        }

                        sal_uInt16 nSpId = aEntry.mnLen ? maSprmParser.GetSprmId(aEntry.mpData) : 0;

                        /*
                         If we replace then we throw away the old data, if we
                         are expanding, then we tack the old data onto the end
                         of the new data
                        */
                        bool bExpand = IsExpandableSprm(nSpId);
                        if (IsReplaceAllSprm(nSpId) || bExpand)
                        {
                            sal_uInt32 nCurr = pDataSt->Tell();
                            sal_uInt32 nPos = SVBT32ToUInt32(aEntry.mpData + 2);
                            if (checkSeek(*pDataSt, nPos))
                            {
                                sal_uInt16 nOrigLen = bExpand ? aEntry.mnLen : 0;
                                sal_uInt8 *pOrigData = bExpand ? aEntry.mpData : 0;

                                pDataSt->ReadUInt16( aEntry.mnLen );
                                aEntry.mpData =
                                    new sal_uInt8[aEntry.mnLen + nOrigLen];
                                aEntry.mbMustDelete = true;
                                aEntry.mnLen =
                                    pDataSt->Read(aEntry.mpData, aEntry.mnLen);

                                pDataSt->Seek( nCurr );

                                if (pOrigData)
                                {
                                    memcpy(aEntry.mpData + aEntry.mnLen,
                                        pOrigData, nOrigLen);
                                    aEntry.mnLen = aEntry.mnLen + nOrigLen;
                                }
                            }
                        }
                    }
                    break;
                default:
                    OSL_FAIL("sweet god, what have you done!");
                    break;
            }
        }

        maEntries.push_back(aEntry);

#ifdef DEBUGSPRMREADER
        {
            sal_Int32 nLen;
            sal_uInt8* pSprms = GetLenAndIStdAndSprms( nLen );

            WW8SprmIter aIter(pSprms, nLen, maSprmParser);
            while (aIter.GetSprms())
            {
                fprintf(stderr, "id is %x\n", aIter.GetAktId());
                aIter.advance();
            }
        }
#endif
    }

    
    maEntries.push_back(Entry(Get_Long(pStart)));

    
    std::sort(maEntries.begin(), maEntries.end());

    mnIdx = 0;

    if (nStartFc >= 0)
        SeekPos(nStartFc);

    pSt->Seek(nOldPos);
}

WW8PLCFx_Fc_FKP::WW8Fkp::Entry::Entry(const Entry &rEntry)
    : mnFC(rEntry.mnFC), mnLen(rEntry.mnLen), mnIStd(rEntry.mnIStd),
    mbMustDelete(rEntry.mbMustDelete)
{
    if (mbMustDelete)
    {
        mpData = new sal_uInt8[mnLen];
        memcpy(mpData, rEntry.mpData, mnLen);
    }
    else
        mpData = rEntry.mpData;
}

WW8PLCFx_Fc_FKP::WW8Fkp::Entry&
    WW8PLCFx_Fc_FKP::WW8Fkp::Entry::operator=(const Entry &rEntry)
{
    if (this == &rEntry)
        return *this;

    if (mbMustDelete)
        delete[] mpData;

    mnFC = rEntry.mnFC;
    mnLen = rEntry.mnLen;
    mnIStd = rEntry.mnIStd;
    mbMustDelete = rEntry.mbMustDelete;

    if (mbMustDelete)
    {
        mpData = new sal_uInt8[mnLen];
        memcpy(mpData, rEntry.mpData, mnLen);
    }
    else
        mpData = rEntry.mpData;
    return *this;
}

WW8PLCFx_Fc_FKP::WW8Fkp::Entry::~Entry()
{
    if (mbMustDelete)
        delete[] mpData;
}

void WW8PLCFx_Fc_FKP::WW8Fkp::Reset(WW8_FC nFc)
{
    SetIdx(0);
    if (nFc >= 0)
        SeekPos(nFc);
}

bool WW8PLCFx_Fc_FKP::WW8Fkp::SeekPos(WW8_FC nFc)
{
    if (nFc < maEntries[0].mnFC)
    {
        mnIdx = 0;
        return false;       
    }

    
    if ((1 > mnIdx) || (nFc < maEntries[mnIdx-1].mnFC))
        mnIdx = 1;

    sal_uInt8 nI   = mnIdx ? mnIdx : 1;
    sal_uInt8 nEnd = mnIMax;

    for(sal_uInt8 n = (1==mnIdx ? 1 : 2); n; --n )
    {
        for( ; nI <=nEnd; ++nI)
        {                               
            if (nFc < maEntries[nI].mnFC)
            {                           
                mnIdx = nI - 1;          
                return true;            
            }
        }
        nI = 1;
        nEnd = mnIdx-1;
    }
    mnIdx = mnIMax;               
    return false;
}

sal_uInt8* WW8PLCFx_Fc_FKP::WW8Fkp::Get(WW8_FC& rStart, WW8_FC& rEnd, sal_Int32& rLen)
    const
{
    rLen = 0;

    if (mnIdx >= mnIMax)
    {
        rStart = WW8_FC_MAX;
        return 0;
    }

    rStart = maEntries[mnIdx].mnFC;
    rEnd   = maEntries[mnIdx + 1].mnFC;

    sal_uInt8* pSprms = GetLenAndIStdAndSprms( rLen );
    return pSprms;
}

bool WW8PLCFx_Fc_FKP::WW8Fkp::SetIdx(sal_uInt8 nI)
{
    if (nI < mnIMax)
    {
        mnIdx = nI;
        return true;
    }
    return false;
}

sal_uInt8* WW8PLCFx_Fc_FKP::WW8Fkp::GetLenAndIStdAndSprms(sal_Int32& rLen) const
{
    rLen = maEntries[mnIdx].mnLen;
    return maEntries[mnIdx].mpData;
}

const sal_uInt8* WW8PLCFx_Fc_FKP::WW8Fkp::HasSprm( sal_uInt16 nId )
{
    if (mnIdx >= mnIMax)
        return 0;

    sal_Int32 nLen;
    sal_uInt8* pSprms = GetLenAndIStdAndSprms( nLen );

    WW8SprmIter aIter(pSprms, nLen, maSprmParser);
    return aIter.FindSprm(nId);
}

bool WW8PLCFx_Fc_FKP::WW8Fkp::HasSprm(sal_uInt16 nId,
    std::vector<const sal_uInt8 *> &rResult)
{
    if (mnIdx >= mnIMax)
       return false;

    sal_Int32 nLen;
    sal_uInt8* pSprms = GetLenAndIStdAndSprms( nLen );

    WW8SprmIter aIter(pSprms, nLen, maSprmParser);

    while(aIter.GetSprms())
    {
        if (aIter.GetAktId() == nId)
            rResult.push_back(aIter.GetAktParams());
        aIter.advance();
    };
    return !rResult.empty();
}


void WW8PLCFx::GetSprms( WW8PLCFxDesc* p )
{
    OSL_ENSURE( !this, "Called wrong GetSprms" );
    p->nStartPos = p->nEndPos = WW8_CP_MAX;
    p->pMemPos = 0;
    p->nSprmsLen = 0;
    p->bRealLineEnd = false;
    return;
}

long WW8PLCFx::GetNoSprms( WW8_CP& rStart, WW8_CP& rEnd, sal_Int32& rLen )
{
    OSL_ENSURE( !this, "Called wrong GetNoSprms" );
    rStart = rEnd = WW8_CP_MAX;
    rLen = 0;
    return 0;
}


sal_uLong WW8PLCFx::GetIdx2() const
{
    return 0;
}

void WW8PLCFx::SetIdx2(sal_uLong )
{
}

class SamePos :
    public std::unary_function<const WW8PLCFx_Fc_FKP::WW8Fkp *, bool>
{
private:
    long mnPo;
public:
    SamePos(long nPo) : mnPo(nPo) {}
    bool operator()(const WW8PLCFx_Fc_FKP::WW8Fkp *pFkp)
        {return mnPo == pFkp->GetFilePos();}
};


bool WW8PLCFx_Fc_FKP::NewFkp()
{
    WW8_CP nPLCFStart, nPLCFEnd;
    void* pPage;

    static const int WW8FkpSizeTabVer2[ PLCF_END ] =
    {
        1,  1, 0 /*, 0, 0, 0*/
    };
    static const int WW8FkpSizeTabVer6[ PLCF_END ] =
    {
        1,  7, 0 /*, 0, 0, 0*/
    };
    static const int WW8FkpSizeTabVer8[ PLCF_END ] =
    {
        1, 13, 0 /*, 0, 0, 0*/
    };
    const int* pFkpSizeTab;

    switch (GetFIBVersion())
    {
        case ww::eWW2:
            pFkpSizeTab = WW8FkpSizeTabVer2;
            break;
        case ww::eWW6:
        case ww::eWW7:
            pFkpSizeTab = WW8FkpSizeTabVer6;
            break;
        case ww::eWW8:
            pFkpSizeTab = WW8FkpSizeTabVer8;
            break;
        default:
            
            OSL_ENSURE( !this, "Someone forgot to encode nVersion!" );
            return false;
    }

    if (!pPLCF->Get( nPLCFStart, nPLCFEnd, pPage ))
    {
        pFkp = 0;
        return false;                           
    }
    pPLCF->advance();
    long nPo = SVBT16ToShort( (sal_uInt8 *)pPage );
    nPo <<= 9;                                  

    long nAktFkpFilePos = pFkp ? pFkp->GetFilePos() : -1;
    if (nAktFkpFilePos == nPo)
        pFkp->Reset(GetStartFc());
    else
    {
        myiter aIter =
            std::find_if(maFkpCache.begin(), maFkpCache.end(), SamePos(nPo));
        if (aIter != maFkpCache.end())
        {
            pFkp = *aIter;
            pFkp->Reset(GetStartFc());
        }
        else if (0 != (pFkp = new WW8Fkp(GetFIBVersion(), pFKPStrm, pDataStrm, nPo,
            pFkpSizeTab[ ePLCF ], ePLCF, GetStartFc())))
        {
            maFkpCache.push_back(pFkp);

            if (maFkpCache.size() > eMaxCache)
            {
                delete maFkpCache.front();
                maFkpCache.pop_front();
            }
        }
    }

    SetStartFc( -1 );                                   
    return true;
}

WW8PLCFx_Fc_FKP::WW8PLCFx_Fc_FKP(SvStream* pSt, SvStream* pTblSt,
    SvStream* pDataSt, const WW8Fib& rFib, ePLCFT ePl, WW8_FC nStartFcL)
    : WW8PLCFx(rFib.GetFIBVersion(), true), pFKPStrm(pSt), pDataStrm(pDataSt),
    pFkp(0), ePLCF(ePl), pPCDAttrs(0)
{
    SetStartFc(nStartFcL);
    long nLenStruct = (8 > rFib.nVersion) ? 2 : 4;
    if (ePl == CHP)
    {
        pPLCF = new WW8PLCF(*pTblSt, rFib.fcPlcfbteChpx, rFib.lcbPlcfbteChpx,
            nLenStruct, GetStartFc(), rFib.pnChpFirst, rFib.cpnBteChp);
    }
    else
    {
        pPLCF = new WW8PLCF(*pTblSt, rFib.fcPlcfbtePapx, rFib.lcbPlcfbtePapx,
            nLenStruct, GetStartFc(), rFib.pnPapFirst, rFib.cpnBtePap);
    }
}

WW8PLCFx_Fc_FKP::~WW8PLCFx_Fc_FKP()
{
    myiter aEnd = maFkpCache.end();
    for (myiter aIter = maFkpCache.begin(); aIter != aEnd; ++aIter)
        delete *aIter;
    delete pPLCF;
    delete pPCDAttrs;
}

sal_uLong WW8PLCFx_Fc_FKP::GetIdx() const
{
    sal_uLong u = pPLCF->GetIdx() << 8;
    if (pFkp)
        u |= pFkp->GetIdx();
    return u;
}

void WW8PLCFx_Fc_FKP::SetIdx( sal_uLong nIdx )
{
    if( !( nIdx & 0xffffff00L ) )
    {
        pPLCF->SetIdx( nIdx >> 8 );
        pFkp = 0;
    }
    else
    {                                   
        
        pPLCF->SetIdx( ( nIdx >> 8 ) - 1 );
        if (NewFkp())                       
        {
            sal_uInt8 nFkpIdx = static_cast<sal_uInt8>(nIdx & 0xff);
            pFkp->SetIdx(nFkpIdx);          
        }
    }
}

bool WW8PLCFx_Fc_FKP::SeekPos(WW8_FC nFcPos)
{
    
    SetStartFc( nFcPos );

    
    bool bRet = pPLCF->SeekPos(nFcPos);

    
    WW8_CP nPLCFStart, nPLCFEnd;
    void* pPage;
    if( pFkp && pPLCF->Get( nPLCFStart, nPLCFEnd, pPage ) )
    {
        long nPo = SVBT16ToShort( (sal_uInt8 *)pPage );
        nPo <<= 9;                                          
        if (nPo != pFkp->GetFilePos())
            pFkp = 0;
        else
            pFkp->SeekPos( nFcPos );
    }
    return bRet;
}

WW8_FC WW8PLCFx_Fc_FKP::Where()
{
    if( !pFkp )
    {
        if( !NewFkp() )
            return WW8_FC_MAX;
    }
    WW8_FC nP = pFkp->Where();
    if( nP != WW8_FC_MAX )
        return nP;

    pFkp = 0;                   
    return Where();                     
}

sal_uInt8* WW8PLCFx_Fc_FKP::GetSprmsAndPos(WW8_FC& rStart, WW8_FC& rEnd, sal_Int32& rLen)
{
    rLen = 0;                               
    rStart = rEnd = WW8_FC_MAX;

    if( !pFkp )     
    {
        if( !NewFkp() )
            return 0;
    }

    sal_uInt8* pPos = pFkp->Get( rStart, rEnd, rLen );
    if( rStart == WW8_FC_MAX )    
        return 0;
    return pPos;
}

void WW8PLCFx_Fc_FKP::advance()
{
    if( !pFkp )
    {
        if( !NewFkp() )
            return;
    }

    pFkp->advance();
    if( pFkp->Where() == WW8_FC_MAX )
        NewFkp();
}

sal_uInt16 WW8PLCFx_Fc_FKP::GetIstd() const
{
    return pFkp ? pFkp->GetIstd() : 0xFFFF;
}

void WW8PLCFx_Fc_FKP::GetPCDSprms( WW8PLCFxDesc& rDesc )
{
    rDesc.pMemPos   = 0;
    rDesc.nSprmsLen = 0;
    if( pPCDAttrs )
    {
        if( !pFkp )
        {
            OSL_FAIL("+Problem: GetPCDSprms: NewFkp necessary (not possible!)" );
            if( !NewFkp() )
                return;
        }
        pPCDAttrs->GetSprms(&rDesc);
    }
}

const sal_uInt8* WW8PLCFx_Fc_FKP::HasSprm( sal_uInt16 nId )
{
    
    if( !pFkp )
    {
        OSL_FAIL( "+Motz: HasSprm: NewFkp needed ( no const possible )" );
        
        if( !NewFkp() )
            return 0;
    }

    const sal_uInt8* pRes = pFkp->HasSprm( nId );

    if( !pRes )
    {
        WW8PLCFxDesc aDesc;
        GetPCDSprms( aDesc );

        if (aDesc.pMemPos)
        {
            WW8SprmIter aIter(aDesc.pMemPos, aDesc.nSprmsLen,
                pFkp->GetSprmParser());
            pRes = aIter.FindSprm(nId);
        }
    }

    return pRes;
}

bool WW8PLCFx_Fc_FKP::HasSprm(sal_uInt16 nId, std::vector<const sal_uInt8 *> &rResult)
{
    
    if (!pFkp)
    {
       OSL_FAIL( "+Motz: HasSprm: NewFkp needed ( no const possible )" );
       
       if( !NewFkp() )
           return false;
    }

    pFkp->HasSprm(nId, rResult);

    WW8PLCFxDesc aDesc;
    GetPCDSprms( aDesc );

    if (aDesc.pMemPos)
    {
        WW8SprmIter aIter(aDesc.pMemPos, aDesc.nSprmsLen,
            pFkp->GetSprmParser());
        while(aIter.GetSprms())
        {
            if (aIter.GetAktId() == nId)
                rResult.push_back(aIter.GetAktParams());
            aIter.advance();
        };
    }
    return !rResult.empty();
}

WW8PLCFx_Cp_FKP::WW8PLCFx_Cp_FKP( SvStream* pSt, SvStream* pTblSt,
    SvStream* pDataSt, const WW8ScannerBase& rBase, ePLCFT ePl )
    : WW8PLCFx_Fc_FKP(pSt, pTblSt, pDataSt, *rBase.pWw8Fib, ePl,
    rBase.WW8Cp2Fc(0)), rSBase(rBase), nAttrStart(-1), nAttrEnd(-1),
    bLineEnd(false),
    bComplex( (7 < rBase.pWw8Fib->nVersion) || rBase.pWw8Fib->fComplex )
{
    ResetAttrStartEnd();

    pPcd = rSBase.pPiecePLCF ? new WW8PLCFx_PCD(GetFIBVersion(),
        rBase.pPiecePLCF, 0, IsSevenMinus(GetFIBVersion())) : 0;

    /*
    Make a copy of the piece attributes for so that the calls to HasSprm on a
    Fc_FKP will be able to take into account the current piece attributes,
    despite the fact that such attributes can only be found through a cp based
    mechanism.
    */
    if (pPcd)
    {
        pPCDAttrs = rSBase.pPLCFx_PCDAttrs ? new WW8PLCFx_PCDAttrs(
            rSBase.pWw8Fib->GetFIBVersion(), pPcd, &rSBase) : 0;
    }

    pPieceIter = rSBase.pPieceIter;
}

WW8PLCFx_Cp_FKP::~WW8PLCFx_Cp_FKP()
{
    delete pPcd;
}

void WW8PLCFx_Cp_FKP::ResetAttrStartEnd()
{
    nAttrStart = -1;
    nAttrEnd   = -1;
    bLineEnd   = false;
}

sal_uLong WW8PLCFx_Cp_FKP::GetPCDIMax() const
{
    return pPcd ? pPcd->GetIMax() : 0;
}

sal_uLong WW8PLCFx_Cp_FKP::GetPCDIdx() const
{
    return pPcd ? pPcd->GetIdx() : 0;
}

void WW8PLCFx_Cp_FKP::SetPCDIdx( sal_uLong nIdx )
{
    if( pPcd )
        pPcd->SetIdx( nIdx );
}

bool WW8PLCFx_Cp_FKP::SeekPos(WW8_CP nCpPos)
{
    if( pPcd )  
    {
        if( !pPcd->SeekPos( nCpPos ) )  
            return false;
        if (pPCDAttrs && !pPCDAttrs->GetIter()->SeekPos(nCpPos))
            return false;
        return WW8PLCFx_Fc_FKP::SeekPos(pPcd->AktPieceStartCp2Fc(nCpPos));
    }
                                    
    return WW8PLCFx_Fc_FKP::SeekPos( rSBase.WW8Cp2Fc(nCpPos) );
}

WW8_CP WW8PLCFx_Cp_FKP::Where()
{
    WW8_FC nFc = WW8PLCFx_Fc_FKP::Where();
    if( pPcd )
        return pPcd->AktPieceStartFc2Cp( nFc ); 
    return rSBase.WW8Fc2Cp( nFc );      
}

void WW8PLCFx_Cp_FKP::GetSprms(WW8PLCFxDesc* p)
{
    WW8_CP nOrigCp = p->nStartPos;

    if (!GetDirty())        
    {
        p->pMemPos = WW8PLCFx_Fc_FKP::GetSprmsAndPos(p->nStartPos, p->nEndPos,
            p->nSprmsLen);
    }
    else
    {
        /*
        For the odd case where we have a location in a fastsaved file which
        does not have an entry in the FKP, perhaps its para end is in the next
        piece, or perhaps the cp just doesn't exist at all in this document.
        AdvSprm doesn't know so it sets the PLCF as dirty and we figure out
        in this method what the situation is

        It doesn't exist then the piece iterator will not be able to find it.
        Otherwise our cool fastsave algorithm can be brought to bear on the
        problem.
        */
        if( !pPieceIter )
            return;
        sal_uLong nOldPos = pPieceIter->GetIdx();
        bool bOk = pPieceIter->SeekPos(nOrigCp);
        pPieceIter->SetIdx( nOldPos );
        if (!bOk)
            return;
    }

    if( pPcd )  
    {
        
        if( (nAttrStart >  nAttrEnd) || (nAttrStart == -1) )
        {
            p->bRealLineEnd = (ePLCF == PAP);

            if ( ((ePLCF == PAP ) || (ePLCF == CHP)) && (nOrigCp != WW8_CP_MAX) )
            {
                bool bIsUnicode=false;
                /*
                To find the end of a paragraph for a character in a
                complex format file.

                It is necessary to know the piece that contains the
                character and the FC assigned to the character.
                */

                
                
                sal_uLong nOldPos = pPieceIter->GetIdx();
                p->nStartPos = nOrigCp;
                pPieceIter->SeekPos( p->nStartPos);

                
                
                

                /*
                Using the FC of the character, first search the FKP that
                describes the character to find the smallest FC in the rgfc
                that is larger than the character FC.
                */
                
                
                WW8_FC nOldEndPos = p->nEndPos;

                /*
                If the FC found in the FKP is less than or equal to the limit
                FC of the piece, the end of the paragraph that contains the
                character is at the FKP FC minus 1.
                */
                WW8_CP nCpStart, nCpEnd;
                void* pData=NULL;
                pPieceIter->Get(nCpStart, nCpEnd, pData);

                WW8_FC nLimitFC = SVBT32ToUInt32( ((WW8_PCD*)pData)->fc );
                WW8_FC nBeginLimitFC = nLimitFC;
                if (IsEightPlus(GetFIBVersion()))
                {
                    nBeginLimitFC =
                        WW8PLCFx_PCD::TransformPieceAddress(nLimitFC,
                        bIsUnicode);
                }

                nLimitFC = nBeginLimitFC +
                    (nCpEnd - nCpStart) * (bIsUnicode ? 2 : 1);

                if (nOldEndPos <= nLimitFC)
                {
                    p->nEndPos = nCpEnd -
                        (nLimitFC-nOldEndPos) / (bIsUnicode ? 2 : 1);
                }
                else
                {
                    if (ePLCF == CHP)
                        p->nEndPos = nCpEnd;
                    else
                    {
                        /*
                        If the FKP FC that was found was greater than the FC
                        of the end of the piece, scan piece by piece toward
                        the end of the document until a piece is found that
                        contains a  paragraph end mark.
                        */

                        /*
                        It's possible to check if a piece contains a paragraph
                        mark by using the FC of the beginning of the piece to
                        search in the FKPs for the smallest FC in the FKP rgfc
                        that is greater than the FC of the beginning of the
                        piece. If the FC found is less than or equal to the
                        limit FC of the piece, then the character that ends
                        the paragraph is the character immediately before the
                        FKP fc
                        */

                        pPieceIter->advance();

                        for (;pPieceIter->GetIdx() < pPieceIter->GetIMax();
                            pPieceIter->advance())
                        {
                            if( !pPieceIter->Get( nCpStart, nCpEnd, pData ) )
                            {
                                OSL_ENSURE( !this, "piece iter broken!" );
                                break;
                            }
                            bIsUnicode = false;
                            sal_Int32 nFcStart=SVBT32ToUInt32(((WW8_PCD*)pData)->fc);

                            if (IsEightPlus(GetFIBVersion()))
                            {
                                nFcStart =
                                    WW8PLCFx_PCD::TransformPieceAddress(
                                    nFcStart,bIsUnicode );
                            }

                            nLimitFC = nFcStart + (nCpEnd - nCpStart) *
                                (bIsUnicode ? 2 : 1);

                            
                            if (!SeekPos(nCpStart))
                                continue;

                            WW8_FC nOne,nSmallest;
                            p->pMemPos = WW8PLCFx_Fc_FKP::GetSprmsAndPos(nOne,
                                nSmallest, p->nSprmsLen);

                            if (nSmallest <= nLimitFC)
                            {
                                WW8_CP nEndPos = nCpEnd -
                                    (nLimitFC-nSmallest) / (bIsUnicode ? 2 : 1);

                                OSL_ENSURE(nEndPos >= p->nStartPos, "EndPos before StartPos");

                                if (nEndPos >= p->nStartPos)
                                    p->nEndPos = nEndPos;

                                break;
                            }
                        }
                    }
                }
                pPieceIter->SetIdx( nOldPos );
            }
            else
                pPcd->AktPieceFc2Cp( p->nStartPos, p->nEndPos,&rSBase );
        }
        else
        {
            p->nStartPos = nAttrStart;
            p->nEndPos = nAttrEnd;
            p->bRealLineEnd = bLineEnd;
        }
    }
    else        
    {
        p->nStartPos = rSBase.WW8Fc2Cp( p->nStartPos );
        p->nEndPos   = rSBase.WW8Fc2Cp( p->nEndPos );
        p->bRealLineEnd = ePLCF == PAP;
    }
}

void WW8PLCFx_Cp_FKP::advance()
{
    WW8PLCFx_Fc_FKP::advance();
    
    if ( !bComplex || !pPcd )
        return;

    if( GetPCDIdx() >= GetPCDIMax() )           
    {
        nAttrStart = nAttrEnd = WW8_CP_MAX;
        return;
    }

    sal_Int32 nFkpLen;                               
    
    WW8PLCFx_Fc_FKP::GetSprmsAndPos(nAttrStart, nAttrEnd, nFkpLen);

    pPcd->AktPieceFc2Cp( nAttrStart, nAttrEnd, &rSBase );
    bLineEnd = (ePLCF == PAP);
}

WW8PLCFx_SEPX::WW8PLCFx_SEPX(SvStream* pSt, SvStream* pTblSt,
    const WW8Fib& rFib, WW8_CP nStartCp)
    : WW8PLCFx(rFib.GetFIBVersion(), true), maSprmParser(rFib.GetFIBVersion()),
    pStrm(pSt), nArrMax(256), nSprmSiz(0)
{
    pPLCF =   rFib.lcbPlcfsed
            ? new WW8PLCF(*pTblSt, rFib.fcPlcfsed, rFib.lcbPlcfsed,
              GetFIBVersion() <= ww::eWW2 ? 6 : 12, nStartCp)
            : 0;

    pSprms = new sal_uInt8[nArrMax];     
}

WW8PLCFx_SEPX::~WW8PLCFx_SEPX()
{
    delete pPLCF;
    delete[] pSprms;
}

sal_uLong WW8PLCFx_SEPX::GetIdx() const
{
    return pPLCF ? pPLCF->GetIdx() : 0;
}

void WW8PLCFx_SEPX::SetIdx( sal_uLong nIdx )
{
    if( pPLCF ) pPLCF->SetIdx( nIdx );
}

bool WW8PLCFx_SEPX::SeekPos(WW8_CP nCpPos)
{
    return pPLCF && pPLCF->SeekPos( nCpPos );
}

WW8_CP WW8PLCFx_SEPX::Where()
{
    return pPLCF ? pPLCF->Where() : 0;
}

void WW8PLCFx_SEPX::GetSprms(WW8PLCFxDesc* p)
{
    if( !pPLCF ) return;

    void* pData;

    p->bRealLineEnd = false;
    if (!pPLCF->Get( p->nStartPos, p->nEndPos, pData ))
    {
        p->nStartPos = p->nEndPos = WW8_CP_MAX;       
        p->pMemPos = 0;
        p->nSprmsLen = 0;
    }
    else
    {
        sal_uInt32 nPo =  SVBT32ToUInt32( (sal_uInt8*)pData+2 );
        if (nPo == 0xFFFFFFFF)
        {
            p->nStartPos = p->nEndPos = WW8_CP_MAX;   
            p->pMemPos = 0;
            p->nSprmsLen = 0;
        }
        else
        {
            pStrm->Seek( nPo );

            
            if (GetFIBVersion() <= ww::eWW2)    
            {
                sal_uInt8 nSiz(0);
                pStrm->ReadUChar( nSiz );
                nSprmSiz = nSiz;
            }
            else
                pStrm->ReadUInt16( nSprmSiz );

            if( nSprmSiz > nArrMax )
            {               
                delete[] pSprms;
                nArrMax = nSprmSiz;                 
                pSprms = new sal_uInt8[nArrMax];
            }
            nSprmSiz = pStrm->Read(pSprms, nSprmSiz); 

            p->nSprmsLen = nSprmSiz;
            p->pMemPos = pSprms;                    
        }
    }
}

void WW8PLCFx_SEPX::advance()
{
    if (pPLCF)
        pPLCF->advance();
}

const sal_uInt8* WW8PLCFx_SEPX::HasSprm( sal_uInt16 nId ) const
{
    return HasSprm( nId, pSprms, nSprmSiz);
}

const sal_uInt8* WW8PLCFx_SEPX::HasSprm( sal_uInt16 nId, const sal_uInt8*  pOtherSprms,
    long nOtherSprmSiz ) const
{
    const sal_uInt8 *pRet = 0;
    if (pPLCF)
    {
        WW8SprmIter aIter(pOtherSprms, nOtherSprmSiz, maSprmParser);
        pRet = aIter.FindSprm(nId);
    }
    return pRet;
}

bool WW8PLCFx_SEPX::Find4Sprms(sal_uInt16 nId1,sal_uInt16 nId2,sal_uInt16 nId3,sal_uInt16 nId4,
    sal_uInt8*& p1, sal_uInt8*& p2, sal_uInt8*& p3, sal_uInt8*& p4) const
{
    if( !pPLCF )
        return false;

    bool bFound = false;
    p1 = 0;
    p2 = 0;
    p3 = 0;
    p4 = 0;

    sal_uInt8* pSp = pSprms;
    sal_uInt16 i=0;
    while (i + maSprmParser.MinSprmLen() <= nSprmSiz)
    {
        
        sal_uInt16 nAktId = maSprmParser.GetSprmId(pSp);
        bool bOk = true;
        if( nAktId  == nId1 )
            p1 = pSp + maSprmParser.DistanceToData(nId1);
        else if( nAktId  == nId2 )
            p2 = pSp + maSprmParser.DistanceToData(nId2);
        else if( nAktId  == nId3 )
            p3 = pSp + maSprmParser.DistanceToData(nId3);
        else if( nAktId  == nId4 )
            p4 = pSp + maSprmParser.DistanceToData(nId4);
        else
            bOk = false;
        bFound |= bOk;
        
        sal_uInt16 x = maSprmParser.GetSprmSize(nAktId, pSp);
        i = i + x;
        pSp += x;
    }
    return bFound;
}

const sal_uInt8* WW8PLCFx_SEPX::HasSprm( sal_uInt16 nId, sal_uInt8 n2nd ) const
{
    if( !pPLCF )
        return 0;

    sal_uInt8* pSp = pSprms;

    sal_uInt16 i=0;
    while (i + maSprmParser.MinSprmLen() <= nSprmSiz)
    {
        
        sal_uInt16 nAktId = maSprmParser.GetSprmId(pSp);
        if (nAktId == nId)
        {
            sal_uInt8 *pRet = pSp + maSprmParser.DistanceToData(nId);
            if (*pRet == n2nd)
                return pRet;
        }
        
        sal_uInt16 x = maSprmParser.GetSprmSize(nAktId, pSp);
        i = i + x;
        pSp += x;
    }

    return 0;   
}


WW8PLCFx_SubDoc::WW8PLCFx_SubDoc(SvStream* pSt, ww::WordVersion eVersion,
    WW8_CP nStartCp, long nFcRef, long nLenRef, long nFcTxt, long nLenTxt,
    long nStruct)
    : WW8PLCFx(eVersion, true), pRef(0), pTxt(0)
{
    if( nLenRef && nLenTxt )
    {
        pRef = new WW8PLCF(*pSt, nFcRef, nLenRef, nStruct, nStartCp);
        pTxt = new WW8PLCF(*pSt, nFcTxt, nLenTxt, 0, nStartCp);
    }
}

WW8PLCFx_SubDoc::~WW8PLCFx_SubDoc()
{
    delete pRef;
    delete pTxt;
}

sal_uLong WW8PLCFx_SubDoc::GetIdx() const
{
    
    if( pRef )
        return ( pRef->GetIdx() << 16 | pTxt->GetIdx() );
    return 0;
}

void WW8PLCFx_SubDoc::SetIdx( sal_uLong nIdx )
{
    if( pRef )
    {
        pRef->SetIdx( nIdx >> 16 );
        
        pTxt->SetIdx( nIdx & 0xFFFF );
    }
}

bool WW8PLCFx_SubDoc::SeekPos( WW8_CP nCpPos )
{
    return ( pRef ) ? pRef->SeekPos( nCpPos ) : false;
}

WW8_CP WW8PLCFx_SubDoc::Where()
{
    return ( pRef ) ? pRef->Where() : WW8_CP_MAX;
}

void WW8PLCFx_SubDoc::GetSprms(WW8PLCFxDesc* p)
{
    p->nStartPos = p->nEndPos = WW8_CP_MAX;
    p->pMemPos = 0;
    p->nSprmsLen = 0;
    p->bRealLineEnd = false;

    if (!pRef)
        return;

    sal_uLong nNr = pRef->GetIdx();

    void *pData;
    WW8_CP nFoo;
    if (!pRef->Get(p->nStartPos, nFoo, pData))
    {
        p->nEndPos = p->nStartPos = WW8_CP_MAX;
        return;
    }

    p->nEndPos = p->nStartPos + 1;

    if (!pTxt)
        return;

    pTxt->SetIdx(nNr);

    if (!pTxt->Get(p->nCp2OrIdx, p->nSprmsLen, pData))
    {
        p->nEndPos = p->nStartPos = WW8_CP_MAX;
        p->nSprmsLen = 0;
        return;
    }

    p->nSprmsLen -= p->nCp2OrIdx;
}

void WW8PLCFx_SubDoc::advance()
{
    if (pRef && pTxt)
    {
        pRef->advance();
        pTxt->advance();
    }
}


WW8PLCFx_FLD::WW8PLCFx_FLD( SvStream* pSt, const WW8Fib& rMyFib, short nType)
    : WW8PLCFx(rMyFib.GetFIBVersion(), true), pPLCF(0), rFib(rMyFib)
{
    long nFc, nLen;

    switch( nType )
    {
    case MAN_HDFT:
        nFc = rFib.fcPlcffldHdr;
        nLen = rFib.lcbPlcffldHdr;
        break;
    case MAN_FTN:
        nFc = rFib.fcPlcffldFtn;
        nLen = rFib.lcbPlcffldFtn;
        break;
    case MAN_EDN:
        nFc = rFib.fcPlcffldEdn;
        nLen = rFib.lcbPlcffldEdn;
        break;
    case MAN_AND:
        nFc = rFib.fcPlcffldAtn;
        nLen = rFib.lcbPlcffldAtn;
        break;
    case MAN_TXBX:
        nFc = rFib.fcPlcffldTxbx;
        nLen = rFib.lcbPlcffldTxbx;
        break;
    case MAN_TXBX_HDFT:
        nFc = rFib.fcPlcffldHdrTxbx;
        nLen = rFib.lcbPlcffldHdrTxbx;
        break;
    default:
        nFc = rFib.fcPlcffldMom;
        nLen = rFib.lcbPlcffldMom;
        break;
    }

    if( nLen )
        pPLCF = new WW8PLCFspecial( pSt, nFc, nLen, 2 );
}

WW8PLCFx_FLD::~WW8PLCFx_FLD()
{
    delete pPLCF;
}

sal_uLong WW8PLCFx_FLD::GetIdx() const
{
    return pPLCF ? pPLCF->GetIdx() : 0;
}

void WW8PLCFx_FLD::SetIdx( sal_uLong nIdx )
{
    if( pPLCF )
        pPLCF->SetIdx( nIdx );
}

bool WW8PLCFx_FLD::SeekPos(WW8_CP nCpPos)
{
    return pPLCF ? pPLCF->SeekPosExact( nCpPos ) : false;
}

WW8_CP WW8PLCFx_FLD::Where()
{
    return pPLCF ? pPLCF->Where() : WW8_CP_MAX;
}

bool WW8PLCFx_FLD::StartPosIsFieldStart()
{
    void* pData;
    sal_Int32 nTest;
    if (
         (!pPLCF || !pPLCF->Get(nTest, pData) ||
         ((((sal_uInt8*)pData)[0] & 0x1f) != 0x13))
       )
        return false;
    return true;
}

bool WW8PLCFx_FLD::EndPosIsFieldEnd(WW8_CP& nCP)
{
    bool bRet = false;

    if (pPLCF)
    {
        long n = pPLCF->GetIdx();

        pPLCF->advance();

        void* pData;
        sal_Int32 nTest;
        if ( pPLCF->Get(nTest, pData) && ((((sal_uInt8*)pData)[0] & 0x1f) == 0x15) )
        {
            nCP = nTest;
            bRet = true;
        }

        pPLCF->SetIdx(n);
    }

    return bRet;
}

void WW8PLCFx_FLD::GetSprms(WW8PLCFxDesc* p)
{
    p->nStartPos = p->nEndPos = WW8_CP_MAX;
    p->pMemPos = 0;
    p->nSprmsLen = 0;
    p->bRealLineEnd = false;

    if (!pPLCF)
    {
        p->nStartPos = WW8_CP_MAX;                    
        return;
    }

    long n = pPLCF->GetIdx();

    sal_Int32 nP;
    void *pData;
    if (!pPLCF->Get(nP, pData))             
    {
        p->nStartPos = WW8_CP_MAX;            
        return;
    }

    p->nStartPos = nP;

    pPLCF->advance();
    if (!pPLCF->Get(nP, pData))             
    {
        p->nStartPos = WW8_CP_MAX;            
        return;
    }

    p->nEndPos = nP;

    pPLCF->SetIdx(n);

    p->nCp2OrIdx = pPLCF->GetIdx();
}

void WW8PLCFx_FLD::advance()
{
    pPLCF->advance();
}

bool WW8PLCFx_FLD::GetPara(long nIdx, WW8FieldDesc& rF)
{
    OSL_ENSURE( pPLCF, "Call without PLCFspecial field" );
    if( !pPLCF )
        return false;

    long n = pPLCF->GetIdx();
    pPLCF->SetIdx(nIdx);

    bool bOk = WW8GetFieldPara(*pPLCF, rF);

    pPLCF->SetIdx(n);
    return bOk;
}



/*  to be optimized like this:    */
void WW8ReadSTTBF(bool bVer8, SvStream& rStrm, sal_uInt32 nStart, sal_Int32 nLen,
    sal_uInt16 nExtraLen, rtl_TextEncoding eCS, std::vector<OUString> &rArray,
    std::vector<ww::bytes>* pExtraArray, ::std::vector<OUString>* pValueArray)
{
    if (nLen==0)     
        return;

    sal_Size nOldPos = rStrm.Tell();
    if (checkSeek(rStrm, nStart))
    {
        sal_uInt16 nLen2(0);
        rStrm.ReadUInt16( nLen2 ); 
                        

        if( bVer8 )
        {
            sal_uInt16 nStrings(0);
            bool bUnicode = (0xFFFF == nLen2);
            if (bUnicode)
                rStrm.ReadUInt16( nStrings );
            else
                nStrings = nLen2;

            rStrm.ReadUInt16( nExtraLen );

            for (sal_uInt16 i=0; i < nStrings; ++i)
            {
                if (bUnicode)
                    rArray.push_back(read_uInt16_PascalString(rStrm));
                else
                {
                    OString aTmp = read_uInt8_lenPrefixed_uInt8s_ToOString(rStrm);
                    rArray.push_back(OStringToOUString(aTmp, eCS));
                }

                
                if (nExtraLen)
                {
                    if (pExtraArray)
                    {
                        ww::bytes extraData;
                        for (sal_uInt16 j = 0; j < nExtraLen; ++j)
                        {
                            sal_uInt8 iTmp(0);
                            rStrm.ReadUChar( iTmp );
                            extraData.push_back(iTmp);
                        }
                        pExtraArray->push_back(extraData);
                    }
                    else
                        rStrm.SeekRel( nExtraLen );
                }
            }
            
            if (pValueArray)
            {
                for (sal_uInt16 i=0; i < nStrings; ++i)
                {
                    if( bUnicode )
                        pValueArray->push_back(read_uInt16_PascalString(rStrm));
                    else
                    {
                        OString aTmp = read_uInt8_lenPrefixed_uInt8s_ToOString(rStrm);
                        pValueArray->push_back(OStringToOUString(aTmp, eCS));
                    }
                }
            }
        }
        else
        {
            if( nLen2 != nLen )
            {
                OSL_ENSURE(nLen2 == nLen,
                    "Fib length and read length are different");
                if (nLen > USHRT_MAX)
                    nLen = USHRT_MAX;
                else if (nLen < 2 )
                    nLen = 2;
                nLen2 = static_cast<sal_uInt16>(nLen);
            }
            sal_uLong nRead = 0;
            for( nLen2 -= 2; nRead < nLen2;  )
            {
                sal_uInt8 nBChar(0);
                rStrm.ReadUChar( nBChar );
                ++nRead;
                if (nBChar)
                {
                    OString aTmp = read_uInt8s_ToOString(rStrm, nBChar);
                    nRead += aTmp.getLength();
                    rArray.push_back(OStringToOUString(aTmp, eCS));
                }
                else
                    rArray.push_back(OUString());

                
                
                if (nExtraLen)
                {
                    if (pExtraArray)
                    {
                        ww::bytes extraData;
                        for (sal_uInt16 i=0;i < nExtraLen;++i)
                        {
                            sal_uInt8 iTmp(0);
                            rStrm.ReadUChar( iTmp );
                            extraData.push_back(iTmp);
                        }
                        pExtraArray->push_back(extraData);
                    }
                    else
                        rStrm.SeekRel( nExtraLen );
                    nRead+=nExtraLen;
                }
            }
        }
    }
    rStrm.Seek(nOldPos);
}

WW8PLCFx_Book::WW8PLCFx_Book(SvStream* pTblSt, const WW8Fib& rFib)
    : WW8PLCFx(rFib.GetFIBVersion(), false), pStatus(0), nIsEnd(0), nBookmarkId(1)
{
    if( !rFib.fcPlcfbkf || !rFib.lcbPlcfbkf || !rFib.fcPlcfbkl ||
        !rFib.lcbPlcfbkl || !rFib.fcSttbfbkmk || !rFib.lcbSttbfbkmk )
    {
        pBook[0] = pBook[1] = 0;
        nIMax = 0;
    }
    else
    {
        pBook[0] = new WW8PLCFspecial(pTblSt,rFib.fcPlcfbkf,rFib.lcbPlcfbkf,4);

        pBook[1] = new WW8PLCFspecial(pTblSt,rFib.fcPlcfbkl,rFib.lcbPlcfbkl,0);

        rtl_TextEncoding eStructChrSet = WW8Fib::GetFIBCharset(rFib.chseTables);

        WW8ReadSTTBF( (7 < rFib.nVersion), *pTblSt, rFib.fcSttbfbkmk,
            rFib.lcbSttbfbkmk, 0, eStructChrSet, aBookNames );

        nIMax = aBookNames.size();

        if( pBook[0]->GetIMax() < nIMax )   
            nIMax = pBook[0]->GetIMax();
        if( pBook[1]->GetIMax() < nIMax )
            nIMax = pBook[1]->GetIMax();
        pStatus = new eBookStatus[ nIMax ];
        memset( pStatus, 0, nIMax * sizeof( eBookStatus ) );
    }
}

WW8PLCFx_Book::~WW8PLCFx_Book()
{
    delete[] pStatus;
    delete pBook[1];
    delete pBook[0];
}

sal_uLong WW8PLCFx_Book::GetIdx() const
{
    return nIMax ? pBook[0]->GetIdx() : 0;
}

void WW8PLCFx_Book::SetIdx( sal_uLong nI )
{
    if( nIMax )
        pBook[0]->SetIdx( nI );
}

sal_uLong WW8PLCFx_Book::GetIdx2() const
{
    return nIMax ? ( pBook[1]->GetIdx() | ( ( nIsEnd ) ? 0x80000000 : 0 ) ) : 0;
}

void WW8PLCFx_Book::SetIdx2( sal_uLong nI )
{
    if( nIMax )
    {
        pBook[1]->SetIdx( nI & 0x7fffffff );
        nIsEnd = (sal_uInt16)( ( nI >> 31 ) & 1 );  
    }
}

bool WW8PLCFx_Book::SeekPos(WW8_CP nCpPos)
{
    if( !pBook[0] )
        return false;

    bool bOk = pBook[0]->SeekPosExact( nCpPos );
    bOk &= pBook[1]->SeekPosExact( nCpPos );
    nIsEnd = 0;

    return bOk;
}

WW8_CP WW8PLCFx_Book::Where()
{
    return pBook[nIsEnd]->Where();
}

long WW8PLCFx_Book::GetNoSprms( WW8_CP& rStart, WW8_CP& rEnd, sal_Int32& rLen )
{
    void* pData;
    rEnd = WW8_CP_MAX;
    rLen = 0;

    if (!pBook[0] || !pBook[1] || !nIMax || (pBook[nIsEnd]->GetIdx()) >= nIMax)
    {
        rStart = rEnd = WW8_CP_MAX;
        return -1;
    }

    pBook[nIsEnd]->Get( rStart, pData );    
    return pBook[nIsEnd]->GetIdx();
}












void WW8PLCFx_Book::advance()
{
    if( pBook[0] && pBook[1] && nIMax )
    {
        (*pBook[nIsEnd]).advance();

        sal_uLong l0 = pBook[0]->Where();
        sal_uLong l1 = pBook[1]->Where();
        if( l0 < l1 )
            nIsEnd = 0;
        else if( l1 < l0 )
            nIsEnd = 1;
        else
        {
            const void * p = pBook[0]->GetData(pBook[0]->GetIdx());
            long nPairFor = (p == NULL)? 0L : SVBT16ToShort(*((SVBT16*) p));
            if (nPairFor == pBook[1]->GetIdx())
                nIsEnd = 0;
            else
                nIsEnd = ( nIsEnd ) ? 0 : 1;
        }
    }
}

long WW8PLCFx_Book::GetLen() const
{
    if( nIsEnd )
    {
        OSL_ENSURE( !this, "Incorrect call (1) of PLCF_Book::GetLen()" );
        return 0;
    }
    void * p;
    WW8_CP nStartPos;
    if( !pBook[0]->Get( nStartPos, p ) )
    {
        OSL_ENSURE( !this, "Incorrect call (2) of PLCF_Book::GetLen()" );
        return 0;
    }
    sal_uInt16 nEndIdx = SVBT16ToShort( *((SVBT16*)p) );
    long nNum = pBook[1]->GetPos( nEndIdx );
    nNum -= nStartPos;
    return nNum;
}

void WW8PLCFx_Book::SetStatus(sal_uInt16 nIndex, eBookStatus eStat )
{
    OSL_ENSURE(nIndex < nIMax, "set status of non existing bookmark!");
    pStatus[nIndex] = (eBookStatus)( pStatus[nIndex] | eStat );
}

eBookStatus WW8PLCFx_Book::GetStatus() const
{
    if( !pStatus )
        return BOOK_NORMAL;
    long nEndIdx = GetHandle();
    return ( nEndIdx < nIMax ) ? pStatus[nEndIdx] : BOOK_NORMAL;
}

long WW8PLCFx_Book::GetHandle() const
{
    if( !pBook[0] || !pBook[1] )
        return LONG_MAX;

    if( nIsEnd )
        return pBook[1]->GetIdx();
    else
    {
        if (const void* p = pBook[0]->GetData(pBook[0]->GetIdx()))
            return SVBT16ToShort( *((SVBT16*)p) );
        else
            return LONG_MAX;
    }
}

OUString WW8PLCFx_Book::GetBookmark(long nStart,long nEnd, sal_uInt16 &nIndex)
{
    bool bFound = false;
    sal_uInt16 i = 0;
    if( pBook[0] && pBook[1] )
    {
        WW8_CP nStartAkt, nEndAkt;
        do
        {
            void* p;
            sal_uInt16 nEndIdx;

            if( pBook[0]->GetData( i, nStartAkt, p ) && p )
                nEndIdx = SVBT16ToShort( *((SVBT16*)p) );
            else
            {
                OSL_ENSURE( !this, "Bookmark-EndIdx not readable" );
                nEndIdx = i;
            }

            nEndAkt = pBook[1]->GetPos( nEndIdx );

            if ((nStartAkt >= nStart) && (nEndAkt <= nEnd))
            {
                nIndex = i;
                bFound=true;
                break;
            }
            ++i;
        }
        while (i < pBook[0]->GetIMax());
    }
    return bFound ? aBookNames[i] : OUString();
}

OUString WW8PLCFx_Book::GetUniqueBookmarkName(const OUString &rSuggestedName)
{
    OUString aRet(rSuggestedName.isEmpty() ? OUString("Unnamed") : rSuggestedName);
    size_t i = 0;
    while (i < aBookNames.size())
    {
        if (aRet.equals(aBookNames[i]))
        {
            sal_Int32 len = aRet.getLength();
            sal_Int32 p = len - 1;
            while (p > 0 && aRet[p] >= '0' && aRet[p] <= '9')
                --p;
            aRet = aRet.copy(0, p+1) + OUString::number(nBookmarkId++);
            i = 0; 
        }
        else
            ++i;
    }
    return aRet;
}

bool WW8PLCFx_Book::MapName(OUString& rName)
{
    if( !pBook[0] || !pBook[1] )
        return false;

    bool bFound = false;
    sal_uInt16 i = 0;
    do
    {
        if (rName.equalsIgnoreAsciiCase(aBookNames[i]))
        {
            rName = aBookNames[i];
            bFound = true;
        }
        ++i;
    }
    while (!bFound && i < pBook[0]->GetIMax());
    return bFound;
}

const OUString* WW8PLCFx_Book::GetName() const
{
    const OUString *pRet = 0;
    if (!nIsEnd && (pBook[0]->GetIdx() < nIMax))
        pRet = &(aBookNames[pBook[0]->GetIdx()]);
    return pRet;
}

#ifndef DUMP




void WW8PLCFMan::AdjustEnds( WW8PLCFxDesc& rDesc )
{
    
    
    rDesc.nOrigEndPos = rDesc.nEndPos;
    rDesc.nOrigStartPos = rDesc.nStartPos;

    /*
     Normally given ^XXX{para end}^ we don't actually insert a para end
     character into the document, so we clip the para end property one to the
     left to make the para properties end when the paragraph text does. In a
     drawing textbox we actually do insert a para end character, so we don't
     clip it. Making the para end properties end after the para end char.
    */
    if (GetDoingDrawTextBox())
        return;

    if ( (&rDesc == pPap) && rDesc.bRealLineEnd )
    {
        if ( pPap->nEndPos != WW8_CP_MAX )    
        {
            nLineEnd = pPap->nEndPos;
            pPap->nEndPos--;        

            
            
            if (pChp->nEndPos == nLineEnd)
                pChp->nEndPos--;

            
            
            if( pSep->nEndPos == nLineEnd )
                pSep->nEndPos--;
        }
    }
    else if ( (&rDesc == pChp) || (&rDesc == pSep) )
    {
        
        if( (rDesc.nEndPos == nLineEnd) && (rDesc.nEndPos > rDesc.nStartPos) )
            rDesc.nEndPos--;            
    }
}

void WW8PLCFxDesc::ReduceByOffset()
{
   OSL_ENSURE((WW8_CP_MAX == nStartPos) || (nStartPos <= nEndPos),
            "Attr-Anfang und -Ende ueber Kreuz" );

    if( nStartPos != WW8_CP_MAX )
    {
        /*
        ##516##,##517##
        Force the property change to happen at the beginning of this
        subdocument, same as in GetNewNoSprms, except that the target type is
        attributes attached to a piece that might span subdocument boundaries
        */
        if (nCpOfs > nStartPos)
            nStartPos = 0;
        else
            nStartPos -= nCpOfs;
    }
    if( nEndPos != WW8_CP_MAX )
    {
        OSL_ENSURE(nCpOfs <= nEndPos,
            "oh oh, so much for the subdocument piece theory");
        nEndPos   -= nCpOfs;
    }
}

void WW8PLCFMan::GetNewSprms( WW8PLCFxDesc& rDesc )
{
    rDesc.pPLCFx->GetSprms(&rDesc);
    rDesc.ReduceByOffset();

    rDesc.bFirstSprm = true;
    AdjustEnds( rDesc );
    rDesc.nOrigSprmsLen = rDesc.nSprmsLen;
}

void WW8PLCFMan::GetNewNoSprms( WW8PLCFxDesc& rDesc )
{
    rDesc.nCp2OrIdx = rDesc.pPLCFx->GetNoSprms(rDesc.nStartPos, rDesc.nEndPos,
        rDesc.nSprmsLen);

   OSL_ENSURE((WW8_CP_MAX == rDesc.nStartPos) || (rDesc.nStartPos <= rDesc.nEndPos),
            "Attr-Anfang und -Ende ueber Kreuz" );

    rDesc.ReduceByOffset();

    rDesc.bFirstSprm = true;
    rDesc.nOrigSprmsLen = rDesc.nSprmsLen;
}

sal_uInt16 WW8PLCFMan::GetId(const WW8PLCFxDesc* p) const
{
    sal_uInt16 nId = 0;        

    if (p == pFld)
        nId = eFLD;
    else if (p == pFtn)
        nId = eFTN;
    else if (p == pEdn)
        nId = eEDN;
    else if (p == pAnd)
        nId = eAND;
    else if (p->nSprmsLen >= maSprmParser.MinSprmLen())
        nId = maSprmParser.GetSprmId(p->pMemPos);

    return nId;
}

WW8PLCFMan::WW8PLCFMan(WW8ScannerBase* pBase, ManTypes nType, long nStartCp,
    bool bDoingDrawTextBox)
    : maSprmParser(pBase->pWw8Fib->GetFIBVersion()),
    mbDoingDrawTextBox(bDoingDrawTextBox)
{
    pWwFib = pBase->pWw8Fib;

    nLastWhereIdxCp = 0;
    memset( aD, 0, sizeof( aD ) );
    nLineEnd = WW8_CP_MAX;
    nManType = nType;
    sal_uInt16 i;

    if( MAN_MAINTEXT == nType )
    {
        
        nPLCF = MAN_ANZ_PLCF;
        pFld = &aD[0];
        pBkm = &aD[1];
        pEdn = &aD[2];
        pFtn = &aD[3];
        pAnd = &aD[4];

        pPcd = ( pBase->pPLCFx_PCD ) ? &aD[5] : 0;
        
        pPcdA = ( pBase->pPLCFx_PCDAttrs ) ? &aD[6] : 0;

        pChp = &aD[7];
        pPap = &aD[8];
        pSep = &aD[9];

        pSep->pPLCFx = pBase->pSepPLCF;
        pFtn->pPLCFx = pBase->pFtnPLCF;
        pEdn->pPLCFx = pBase->pEdnPLCF;
        pBkm->pPLCFx = pBase->pBook;
        pAnd->pPLCFx = pBase->pAndPLCF;

    }
    else
    {
        
        nPLCF = 7;
        pFld = &aD[0];
        pBkm = ( pBase->pBook ) ? &aD[1] : 0;

        pPcd = ( pBase->pPLCFx_PCD ) ? &aD[2] : 0;
        
        pPcdA= ( pBase->pPLCFx_PCDAttrs ) ? &aD[3] : 0;

        pChp = &aD[4];
        pPap = &aD[5];
        pSep = &aD[6]; 

        pAnd = pFtn = pEdn = 0;     
    }

    pChp->pPLCFx = pBase->pChpPLCF;
    pPap->pPLCFx = pBase->pPapPLCF;
    if( pPcd )
        pPcd->pPLCFx = pBase->pPLCFx_PCD;
    if( pPcdA )
        pPcdA->pPLCFx= pBase->pPLCFx_PCDAttrs;
    if( pBkm )
        pBkm->pPLCFx = pBase->pBook;

    pMagicTables = pBase->pMagicTables;
    pSubdocs = pBase->pSubdocs;
    pExtendedAtrds = pBase->pExtendedAtrds;

    switch( nType )                 
    {
        case MAN_HDFT:
            pFld->pPLCFx = pBase->pFldHdFtPLCF;
            pFdoa = pBase->pHdFtFdoa;
            pTxbx = pBase->pHdFtTxbx;
            pTxbxBkd = pBase->pHdFtTxbxBkd;
            break;
        case MAN_FTN:
            pFld->pPLCFx = pBase->pFldFtnPLCF;
            pFdoa = pTxbx = pTxbxBkd = 0;
            break;
        case MAN_EDN:
            pFld->pPLCFx = pBase->pFldEdnPLCF;
            pFdoa = pTxbx = pTxbxBkd = 0;
            break;
        case MAN_AND:
            pFld->pPLCFx = pBase->pFldAndPLCF;
            pFdoa = pTxbx = pTxbxBkd = 0;
            break;
        case MAN_TXBX:
            pFld->pPLCFx = pBase->pFldTxbxPLCF;
            pTxbx = pBase->pMainTxbx;
            pTxbxBkd = pBase->pMainTxbxBkd;
            pFdoa = 0;
            break;
        case MAN_TXBX_HDFT:
            pFld->pPLCFx = pBase->pFldTxbxHdFtPLCF;
            pTxbx = pBase->pHdFtTxbx;
            pTxbxBkd = pBase->pHdFtTxbxBkd;
            pFdoa = 0;
            break;
        default:
            pFld->pPLCFx = pBase->pFldPLCF;
            pFdoa = pBase->pMainFdoa;
            pTxbx = pBase->pMainTxbx;
            pTxbxBkd = pBase->pMainTxbxBkd;
            break;
    }

    nCpO = pWwFib->GetBaseCp(nType);

    if( nStartCp || nCpO )
        SeekPos( nStartCp );    

    
    GetChpPLCF()->ResetAttrStartEnd();
    GetPapPLCF()->ResetAttrStartEnd();
    for( i=0; i < nPLCF; i++)
    {
        WW8PLCFxDesc* p = &aD[i];

        /*
        ##516##,##517##
        For subdocuments we modify the cp of properties to be relative to
        the beginning of subdocuments, we should also do the same for
        piecetable changes, and piecetable properties, otherwise a piece
        change that happens in a subdocument is lost.
        */
        p->nCpOfs = ( p == pChp || p == pPap || p == pBkm || p == pPcd ||
            p == pPcdA ) ? nCpO : 0;

        p->nCp2OrIdx = 0;
        p->bFirstSprm = false;
        p->pIdStk = 0;

        if ((p == pChp) || (p == pPap))
            p->nStartPos = p->nEndPos = nStartCp;
        else
            p->nStartPos = p->nEndPos = WW8_CP_MAX;
    }

    
    for( i=0; i<nPLCF; i++){
        WW8PLCFxDesc* p = &aD[i];

        if( !p->pPLCFx )
        {
            p->nStartPos = p->nEndPos = WW8_CP_MAX;
            continue;
        }

        if( p->pPLCFx->IsSprm() )
        {
            
            p->pIdStk = new std::stack<sal_uInt16>;
            if ((p == pChp) || (p == pPap))
            {
                WW8_CP nTemp = p->nEndPos+p->nCpOfs;
                p->pMemPos = 0;
                p->nSprmsLen = 0;
                p->nStartPos = nTemp;
                if (!(*p->pPLCFx).SeekPos(p->nStartPos))
                    p->nEndPos = p->nStartPos = WW8_CP_MAX;
                else
                    GetNewSprms( *p );
            }
            else
                GetNewSprms( *p );      
        }
        else if( p->pPLCFx )
            GetNewNoSprms( *p );
    }
}

WW8PLCFMan::~WW8PLCFMan()
{
    for( sal_uInt16 i=0; i<nPLCF; i++)
        delete aD[i].pIdStk;
}




sal_uInt16 WW8PLCFMan::WhereIdx(bool* pbStart, long* pPos) const
{
    OSL_ENSURE(nPLCF,"What the hell");
    long nNext = LONG_MAX;  
    sal_uInt16 nNextIdx = nPLCF;
    bool bStart = true;     
    sal_uInt16 i;
    const WW8PLCFxDesc* pD;
    for (i=0; i < nPLCF; i++)
    {
        pD = &aD[i];
        if (pD != pPcdA)
        {
            if( (pD->nEndPos < nNext) && (pD->nStartPos == WW8_CP_MAX) )
            {
                
                nNext = pD->nEndPos;
                nNextIdx = i;
                bStart = false;
            }
        }
    }
    for (i=nPLCF; i > 0; i--)
    {
        pD = &aD[i-1];
        if (pD != pPcdA)
        {
            if( pD->nStartPos < nNext )
            {
                nNext = pD->nStartPos;
                nNextIdx = i-1;
                bStart = true;
            }
        }
    }
    if( pPos )
        *pPos = nNext;
    if( pbStart )
        *pbStart = bStart;
    return nNextIdx;
}


WW8_CP WW8PLCFMan::Where() const
{
    long l;
    WhereIdx(0, &l);
    return l;
}

void WW8PLCFMan::SeekPos( long nNewCp )
{
    pChp->pPLCFx->SeekPos( nNewCp + nCpO ); 
    pPap->pPLCFx->SeekPos( nNewCp + nCpO ); 
    pFld->pPLCFx->SeekPos( nNewCp );
    if( pPcd )
        pPcd->pPLCFx->SeekPos( nNewCp + nCpO );
    if( pBkm )
        pBkm->pPLCFx->SeekPos( nNewCp + nCpO );
}

void WW8PLCFMan::SaveAllPLCFx( WW8PLCFxSaveAll& rSave ) const
{
    sal_uInt16 i, n=0;
    if( pPcd )
        pPcd->Save(  rSave.aS[n++] );
    if( pPcdA )
        pPcdA->Save( rSave.aS[n++] );

    for(i=0; i<nPLCF; ++i)
        if( pPcd != &aD[i] && pPcdA != &aD[i] )
            aD[i].Save( rSave.aS[n++] );
}

void WW8PLCFMan::RestoreAllPLCFx( const WW8PLCFxSaveAll& rSave )
{
    sal_uInt16 i, n=0;
    if( pPcd )
        pPcd->Restore(  rSave.aS[n++] );
    if( pPcdA )
        pPcdA->Restore( rSave.aS[n++] );

    for(i=0; i<nPLCF; ++i)
        if( pPcd != &aD[i] && pPcdA != &aD[i] )
            aD[i].Restore( rSave.aS[n++] );
}

void WW8PLCFMan::GetSprmStart( short nIdx, WW8PLCFManResult* pRes ) const
{
    memset( pRes, 0, sizeof( WW8PLCFManResult ) );

    

    pRes->nMemLen = 0;

    const WW8PLCFxDesc* p = &aD[nIdx];

    
    if( p->bFirstSprm )
    {
        if( p == pPap )
            pRes->nFlags |= MAN_MASK_NEW_PAP;
        else if( p == pSep )
            pRes->nFlags |= MAN_MASK_NEW_SEP;
    }
    pRes->pMemPos = p->pMemPos;
    pRes->nSprmId = GetId(p);
    pRes->nCp2OrIdx = p->nCp2OrIdx;
    if ((p == pFtn) || (p == pEdn) || (p == pAnd))
        pRes->nMemLen = p->nSprmsLen;
    else if (p->nSprmsLen >= maSprmParser.MinSprmLen()) 
    {
        
        pRes->nMemLen = maSprmParser.GetSprmSize(pRes->nSprmId, pRes->pMemPos);
    }
}

void WW8PLCFMan::GetSprmEnd( short nIdx, WW8PLCFManResult* pRes ) const
{
    memset( pRes, 0, sizeof( WW8PLCFManResult ) );

    const WW8PLCFxDesc* p = &aD[nIdx];

    if (!(p->pIdStk->empty()))
        pRes->nSprmId = p->pIdStk->top();       
    else
    {
        OSL_ENSURE( !this, "No Id on the Stack" );
        pRes->nSprmId = 0;
    }
}

void WW8PLCFMan::GetNoSprmStart( short nIdx, WW8PLCFManResult* pRes ) const
{
    const WW8PLCFxDesc* p = &aD[nIdx];

    pRes->nCpPos = p->nStartPos;
    pRes->nMemLen = p->nSprmsLen;
    pRes->nCp2OrIdx = p->nCp2OrIdx;

    if( p == pFld )
        pRes->nSprmId = eFLD;
    else if( p == pFtn )
        pRes->nSprmId = eFTN;
    else if( p == pEdn )
        pRes->nSprmId = eEDN;
    else if( p == pBkm )
        pRes->nSprmId = eBKN;
    else if( p == pAnd )
        pRes->nSprmId = eAND;
    else if( p == pPcd )
    {
        
        
        GetSprmStart( nIdx+1, pRes );
    }
    else
        pRes->nSprmId = 0;          
}

void WW8PLCFMan::GetNoSprmEnd( short nIdx, WW8PLCFManResult* pRes ) const
{
    pRes->nMemLen = -1;     

    if( &aD[nIdx] == pBkm )
        pRes->nSprmId = eBKN;
    else if( &aD[nIdx] == pPcd )
    {
        
        
        GetSprmEnd( nIdx+1, pRes );
    }
    else
        pRes->nSprmId = 0;
}

bool WW8PLCFMan::TransferOpenSprms(std::stack<sal_uInt16> &rStack)
{
    for (int i = 0; i < nPLCF; ++i)
    {
        WW8PLCFxDesc* p = &aD[i];
        if (!p || !p->pIdStk)
            continue;
        while (!p->pIdStk->empty())
        {
            rStack.push(p->pIdStk->top());
            p->pIdStk->pop();
        }
    }
    return rStack.empty();
}

void WW8PLCFMan::AdvSprm(short nIdx, bool bStart)
{
    WW8PLCFxDesc* p = &aD[nIdx];    

    p->bFirstSprm = false;
    if( bStart )
    {
        sal_uInt16 nLastId = GetId(p);
        p->pIdStk->push(nLastId);   

        if( p->nSprmsLen )
        {   /*
                Pruefe, ob noch Sprm(s) abzuarbeiten sind
            */
            if( p->pMemPos )
            {
                
                sal_uInt16 nSprmL = maSprmParser.GetSprmSize(nLastId, p->pMemPos);

                
                p->nSprmsLen -= nSprmL;

                
                if (p->nSprmsLen < maSprmParser.MinSprmLen())
                {
                    
                    p->pMemPos = 0;
                    p->nSprmsLen = 0;
                }
                else
                    p->pMemPos += nSprmL;
            }
            else
                p->nSprmsLen = 0;
        }
        if (p->nSprmsLen < maSprmParser.MinSprmLen())
            p->nStartPos = WW8_CP_MAX;    
    }
    else
    {
        if (!(p->pIdStk->empty()))
            p->pIdStk->pop();
        if (p->pIdStk->empty())
        {
            if ( (p == pChp) || (p == pPap) )
            {
                p->pMemPos = 0;
                p->nSprmsLen = 0;
                p->nStartPos = p->nOrigEndPos+p->nCpOfs;

                /*
                On failed seek we have run out of sprms, probably.  But if its
                a fastsaved file (has pPcd) then we may be just in a sprm free
                gap between pieces that have them, so set dirty flag in sprm
                finder to consider than.
                */
                if (!(*p->pPLCFx).SeekPos(p->nStartPos))
                {
                    p->nEndPos = WW8_CP_MAX;
                    p->pPLCFx->SetDirty(true);
                }
                if (!p->pPLCFx->GetDirty() || pPcd)
                    GetNewSprms( *p );
                p->pPLCFx->SetDirty(false);

                /*
                #i2325#
                To get the character and paragraph properties you first get
                the pap and chp and then apply the fastsaved pPcd properties
                to the range. If a pap or chp starts inside the pPcd range
                then we must bring the current pPcd range to a halt so as to
                end those sprms, then the pap/chp will be processed, and then
                we must force a restart of the pPcd on that pap/chp starting
                boundary. Doing that effectively means that the pPcd sprms will
                be applied to the new range. Not doing it means that the pPcd
                sprms will only be applied to the first pap/chp set of
                properties contained in the pap/chp range.

                So we bring the pPcd to a halt on this location here, by
                settings its end to the current start, then store the starting
                position of the current range to clipstart. The pPcd sprms
                will end as normal (albeit earlier than originally expected),
                and the existance of a clipstart will force the pPcd iterater
                to reread the current set of sprms instead of advancing to its
                next set. Then the clipstart will be set as the starting
                position which will force them to be applied directly after
                the pap and chps.
                */
                if (pPcd && ((p->nStartPos > pPcd->nStartPos) ||
                    (pPcd->nStartPos == WW8_CP_MAX)) &&
                    (pPcd->nEndPos != p->nStartPos))
                {
                    pPcd->nEndPos = p->nStartPos;
                    ((WW8PLCFx_PCD *)(pPcd->pPLCFx))->SetClipStart(
                        p->nStartPos);
                }

            }
            else
            {
                p->pPLCFx->advance(); 
                p->pMemPos = 0;       
                p->nSprmsLen = 0;
                GetNewSprms( *p );
            }
            OSL_ENSURE( p->nStartPos <= p->nEndPos, "Attribut ueber Kreuz" );
        }
    }
}

void WW8PLCFMan::AdvNoSprm(short nIdx, bool bStart)
{
    /*
    For the case of a piece table we slave the piece table attribute iterator
    to the piece table and access it through that only. They are two separate
    structures, but act together as one logical one. The attributes only go
    to the next entry when the piece changes
    */
    WW8PLCFxDesc* p = &aD[nIdx];

    if( p == pPcd )
    {
        AdvSprm(nIdx+1,bStart);
        if( bStart )
            p->nStartPos = aD[nIdx+1].nStartPos;
        else
        {
            if (aD[nIdx+1].pIdStk->empty())
            {
                WW8PLCFx_PCD *pTemp = (WW8PLCFx_PCD*)(pPcd->pPLCFx);
                /*
                #i2325#
                As per normal, go on to the next set of properties, i.e. we
                have traversed over to the next piece.  With a clipstart set
                we are being told to reread the current piece sprms so as to
                reapply them to a new chp or pap range.
                */
                if (pTemp->GetClipStart() == -1)
                    p->pPLCFx->advance();
                p->pMemPos = 0;
                p->nSprmsLen = 0;
                GetNewSprms( aD[nIdx+1] );
                GetNewNoSprms( *p );
                if (pTemp->GetClipStart() != -1)
                {
                    /*
                    #i2325#, now we will force our starting position to the
                    clipping start so as to force the application of these
                    sprms after the current pap/chp sprms so as to apply the
                    fastsave sprms to the current range.
                    */
                    p->nStartPos = pTemp->GetClipStart();
                    pTemp->SetClipStart(-1);
                }
            }
        }
    }
    else
    {                                  
        p->pPLCFx->advance();
        p->pMemPos = 0;                     
        p->nSprmsLen = 0;
        GetNewNoSprms( *p );
    }
}

void WW8PLCFMan::advance()
{
    bool bStart;
    sal_uInt16 nIdx = WhereIdx(&bStart);
    if (nIdx < nPLCF)
    {
        WW8PLCFxDesc* p = &aD[nIdx];

        p->bFirstSprm = true;                       

        if( p->pPLCFx->IsSprm() )
            AdvSprm( nIdx, bStart );
        else                                        
            AdvNoSprm( nIdx, bStart );
    }
}





bool WW8PLCFMan::Get(WW8PLCFManResult* pRes) const
{
    memset( pRes, 0, sizeof( WW8PLCFManResult ) );
    bool bStart;
    sal_uInt16 nIdx = WhereIdx(&bStart);

    if( nIdx >= nPLCF )
    {
        OSL_ENSURE( !this, "Position not found" );
        return true;
    }

    if( aD[nIdx].pPLCFx->IsSprm() )
    {
        if( bStart )
        {
            GetSprmStart( nIdx, pRes );
            return true;
        }
        else
        {
            GetSprmEnd( nIdx, pRes );
            return false;
        }
    }
    else
    {
        if( bStart )
        {
            GetNoSprmStart( nIdx, pRes );
            return true;
        }
        else
        {
            GetNoSprmEnd( nIdx, pRes );
            return false;
        }
    }
}

sal_uInt16 WW8PLCFMan::GetColl() const
{
    if( pPap->pPLCFx )
        return  pPap->pPLCFx->GetIstd();
    else
    {
        OSL_ENSURE( !this, "GetColl ohne PLCF_Pap" );
        return 0;
    }
}

WW8PLCFx_FLD* WW8PLCFMan::GetFld() const
{
    return (WW8PLCFx_FLD*)pFld->pPLCFx;
}

const sal_uInt8* WW8PLCFMan::HasParaSprm( sal_uInt16 nId ) const
{
    return ((WW8PLCFx_Cp_FKP*)pPap->pPLCFx)->HasSprm( nId );
}

const sal_uInt8* WW8PLCFMan::HasCharSprm( sal_uInt16 nId ) const
{
    return ((WW8PLCFx_Cp_FKP*)pChp->pPLCFx)->HasSprm( nId );
}

bool WW8PLCFMan::HasCharSprm(sal_uInt16 nId,
    std::vector<const sal_uInt8 *> &rResult) const
{
    return ((WW8PLCFx_Cp_FKP*)pChp->pPLCFx)->HasSprm(nId, rResult);
}

#endif 

void WW8PLCFx::Save( WW8PLCFxSave1& rSave ) const
{
    rSave.nPLCFxPos    = GetIdx();
    rSave.nPLCFxPos2   = GetIdx2();
    rSave.nPLCFxMemOfs = 0;
    rSave.nStartFC     = GetStartFc();
}

void WW8PLCFx::Restore( const WW8PLCFxSave1& rSave )
{
    SetIdx(     rSave.nPLCFxPos  );
    SetIdx2(    rSave.nPLCFxPos2 );
    SetStartFc( rSave.nStartFC   );
}

sal_uLong WW8PLCFx_Cp_FKP::GetIdx2() const
{
    return GetPCDIdx();
}

void WW8PLCFx_Cp_FKP::SetIdx2( sal_uLong nIdx )
{
    SetPCDIdx( nIdx );
}

void WW8PLCFx_Cp_FKP::Save( WW8PLCFxSave1& rSave ) const
{
    WW8PLCFx::Save( rSave );

    rSave.nAttrStart = nAttrStart;
    rSave.nAttrEnd   = nAttrEnd;
    rSave.bLineEnd   = bLineEnd;
}

void WW8PLCFx_Cp_FKP::Restore( const WW8PLCFxSave1& rSave )
{
    WW8PLCFx::Restore( rSave );

    nAttrStart = rSave.nAttrStart;
    nAttrEnd   = rSave.nAttrEnd;
    bLineEnd   = rSave.bLineEnd;
}

void WW8PLCFxDesc::Save( WW8PLCFxSave1& rSave ) const
{
    if( pPLCFx )
    {
        pPLCFx->Save( rSave );
        if( pPLCFx->IsSprm() )
        {
            WW8PLCFxDesc aD;
            aD.nStartPos = nOrigStartPos+nCpOfs;
            aD.nCpOfs = rSave.nCpOfs = nCpOfs;
            if (!(pPLCFx->SeekPos(aD.nStartPos)))
            {
                aD.nEndPos = WW8_CP_MAX;
                pPLCFx->SetDirty(true);
            }
            pPLCFx->GetSprms(&aD);
            pPLCFx->SetDirty(false);
            aD.ReduceByOffset();
            rSave.nStartCp = aD.nStartPos;
            rSave.nPLCFxMemOfs = nOrigSprmsLen - nSprmsLen;
        }
    }
}

void WW8PLCFxDesc::Restore( const WW8PLCFxSave1& rSave )
{
    if( pPLCFx )
    {
        pPLCFx->Restore( rSave );
        if( pPLCFx->IsSprm() )
        {
            WW8PLCFxDesc aD;
            aD.nStartPos = rSave.nStartCp+rSave.nCpOfs;
            nCpOfs = aD.nCpOfs = rSave.nCpOfs;
            if (!(pPLCFx->SeekPos(aD.nStartPos)))
            {
                aD.nEndPos = WW8_CP_MAX;
                pPLCFx->SetDirty(true);
            }
            pPLCFx->GetSprms(&aD);
            pPLCFx->SetDirty(false);
            aD.ReduceByOffset();
            pMemPos = aD.pMemPos + rSave.nPLCFxMemOfs;
        }
    }
}



namespace
{
    sal_uInt32 Readcb(SvStream& rSt, ww::WordVersion eVer)
    {
        if (eVer <= ww::eWW2)
        {
            sal_uInt16 nShort;
            rSt.ReadUInt16( nShort );
            return nShort;
        }
        else
        {
            sal_uInt32 nLong;
            rSt.ReadUInt32( nLong );
            return nLong;
        }
    }
}

WW8_CP WW8Fib::GetBaseCp(ManTypes nType) const
{
    WW8_CP nOffset = 0;

    switch( nType )
    {
        default:
        case MAN_MAINTEXT:
            break;
        case MAN_FTN:
            nOffset = ccpText;
            break;
        case MAN_HDFT:
            nOffset = ccpText + ccpFtn;
            break;
        /*
         A subdocument of this kind (MAN_MACRO) probably exists in some defunct
         version of MSWord, but now ccpMcr is always 0. If some example that
         uses this comes to light, this is the likely calculation required

        case MAN_MACRO:
            nOffset = ccpText + ccpFtn + ccpHdr;
            break;

        */
        case MAN_AND:
            nOffset = ccpText + ccpFtn + ccpHdr + ccpMcr;
            break;
        case MAN_EDN:
            nOffset = ccpText + ccpFtn + ccpHdr + ccpMcr + ccpAtn;
            break;
        case MAN_TXBX:
            nOffset = ccpText + ccpFtn + ccpHdr + ccpMcr + ccpAtn + ccpEdn;
            break;
        case MAN_TXBX_HDFT:
            nOffset = ccpText + ccpFtn + ccpHdr + ccpMcr + ccpAtn + ccpEdn +
                ccpTxbx;
            break;
    }
    return nOffset;
}

ww::WordVersion WW8Fib::GetFIBVersion() const
{
    ww::WordVersion eVer = ww::eWW8;
    /*
     * Word for Windows 2 I think (1.X might work too if anyone has an example.
     * Various pages claim that the fileformats of Word 1 and 2 for Windows are
     * equivalent to Word for Macintosh 4 and 5. On the other hand
     *
     * wIdents for Word for Mac versions...
     * 0xFE32 for Word 1
     * 0xFE34 for Word 3
     * 0xFE37 for Word 4 et 5.
     *
     * and this document
     * http:
     * claimed to be "Word 5 for Mac" by Office etc and has that wIdent, but
     * its format isn't the same as that of Word 2 for windows. Nor is it
     * the same as that of Word for DOS/PCWord 5
     */
    if (wIdent == 0xa5db)
        eVer = ww::eWW2;
    else
    {
        switch (nVersion)
        {
            case 6:
                eVer = ww::eWW6;
                break;
            case 7:
                eVer = ww::eWW7;
                break;
            case 8:
                eVer = ww::eWW8;
                break;
        }
    }
    return eVer;
}

WW8Fib::WW8Fib(SvStream& rSt, sal_uInt8 nWantedVersion, sal_uInt32 nOffset)
    : nFibError( 0 )
{
    memset(this, 0, sizeof(*this));
    sal_uInt8 aBits1;
    sal_uInt8 aBits2;
    sal_uInt8 aVer8Bits1;    
    rSt.Seek( nOffset );
    /*
        Wunsch-Nr vermerken, File-Versionsnummer ermitteln
        und gegen Wunsch-Nr. checken !
    */
    nVersion = nWantedVersion;
    rSt.ReadUInt16( wIdent );
    rSt.ReadUInt16( nFib );
    rSt.ReadUInt16( nProduct );
    if( 0 != rSt.GetError() )
    {
        sal_Int16 nFibMin;
        sal_Int16 nFibMax;
        
        switch( nVersion )
        {
            case 6:
                nFibMin = 0x0065;   
                                    
                                    
                                    
                nFibMax = 0x0069;   
                break;
            case 7:
                nFibMin = 0x0069;   
                nFibMax = 0x0069;   
                break;
            case 8:
                nFibMin = 0x006A;   
                nFibMax = 0x00c1;   
                break;
            default:
                nFibMin = 0;            
                nFibMax = 0;
                nFib    = 1;
                OSL_ENSURE( !this, "Es wurde vergessen, nVersion zu kodieren!" );
                break;
        }
        if ( (nFib < nFibMin) || (nFib > nFibMax) )
        {
            nFibError = ERR_SWG_READ_ERROR; 
            return;                         
        }
    }

    ww::WordVersion eVer = GetFIBVersion();

    
    sal_Int16 pnChpFirst_Ver67=0;
    sal_Int16 pnPapFirst_Ver67=0;
    sal_Int16 cpnBteChp_Ver67=0;
    sal_Int16 cpnBtePap_Ver67=0;

    
    rSt.ReadInt16( lid );
    rSt.ReadInt16( pnNext );
    rSt.ReadUChar( aBits1 );
    rSt.ReadUChar( aBits2 );
    rSt.ReadUInt16( nFibBack );
    rSt.ReadUInt16( nHash );
    rSt.ReadUInt16( nKey );
    rSt.ReadUChar( envr );
    rSt.ReadUChar( aVer8Bits1 );      
                            
                            //
                            
                            
                            
                            
                            
                            
    rSt.ReadUInt16( chse );
    rSt.ReadUInt16( chseTables );
    rSt.ReadInt32( fcMin );
    rSt.ReadInt32( fcMac );


    if (IsEightPlus(eVer))
    {
        rSt.ReadUInt16( csw );

        
        rSt.ReadUInt16( wMagicCreated );
        rSt.ReadUInt16( wMagicRevised );
        rSt.ReadUInt16( wMagicCreatedPrivate );
        rSt.ReadUInt16( wMagicRevisedPrivate );
        rSt.SeekRel( 9 * sizeof( sal_Int16 ) );

        /*
        
        && (bVer67 || WW8ReadINT16(  rSt, pnFbpChpFirst_W6          ))  
        && (bVer67 || WW8ReadINT16(  rSt, pnChpFirst_W6                 ))  
        && (bVer67 || WW8ReadINT16(  rSt, cpnBteChp_W6                  ))  
        && (bVer67 || WW8ReadINT16(  rSt, pnFbpPapFirst_W6          ))  
        && (bVer67 || WW8ReadINT16(  rSt, pnPapFirst_W6                 ))  
        && (bVer67 || WW8ReadINT16(  rSt, cpnBtePap_W6                  ))  
        && (bVer67 || WW8ReadINT16(  rSt, pnFbpLvcFirst_W6          ))  
        && (bVer67 || WW8ReadINT16(  rSt, pnLvcFirst_W6                 ))  
        && (bVer67 || WW8ReadINT16(  rSt, cpnBteLvc_W6                  ))  
        */
        rSt.ReadInt16( lidFE );
        rSt.ReadUInt16( clw );
    }



        
    rSt.ReadInt32( cbMac );

        
    rSt.SeekRel( 2 * sizeof( sal_Int32) );

        
    if (IsSevenMinus(eVer))
        rSt.SeekRel( 2 * sizeof( sal_Int32) );

    rSt.ReadInt32( ccpText );
    rSt.ReadInt32( ccpFtn );
    rSt.ReadInt32( ccpHdr );
    rSt.ReadInt32( ccpMcr );
    rSt.ReadInt32( ccpAtn );
    rSt.ReadInt32( ccpEdn );
    rSt.ReadInt32( ccpTxbx );
    rSt.ReadInt32( ccpHdrTxbx );

        
    if (IsSevenMinus(eVer))
        rSt.SeekRel( 1 * sizeof( sal_Int32) );
    else
    {

        rSt.ReadInt32( pnFbpChpFirst );
        rSt.ReadInt32( pnChpFirst );
        rSt.ReadInt32( cpnBteChp );
        rSt.ReadInt32( pnFbpPapFirst );
        rSt.ReadInt32( pnPapFirst );
        rSt.ReadInt32( cpnBtePap );
        rSt.ReadInt32( pnFbpLvcFirst );
        rSt.ReadInt32( pnLvcFirst );
        rSt.ReadInt32( cpnBteLvc );
        rSt.ReadInt32( fcIslandFirst );
        rSt.ReadInt32( fcIslandLim );
        rSt.ReadUInt16( cfclcb );
    }



    
    rSt.ReadInt32( fcStshfOrig );
    lcbStshfOrig = Readcb(rSt, eVer);
    rSt.ReadInt32( fcStshf );
    lcbStshf = Readcb(rSt, eVer);
    rSt.ReadInt32( fcPlcffndRef );
    lcbPlcffndRef = Readcb(rSt, eVer);
    rSt.ReadInt32( fcPlcffndTxt );
    lcbPlcffndTxt = Readcb(rSt, eVer);
    rSt.ReadInt32( fcPlcfandRef );
    lcbPlcfandRef = Readcb(rSt, eVer);
    rSt.ReadInt32( fcPlcfandTxt );
    lcbPlcfandTxt = Readcb(rSt, eVer);
    rSt.ReadInt32( fcPlcfsed );
    lcbPlcfsed = Readcb(rSt, eVer);
    rSt.ReadInt32( fcPlcfpad );
    lcbPlcfpad = Readcb(rSt, eVer);
    rSt.ReadInt32( fcPlcfphe );
    lcbPlcfphe = Readcb(rSt, eVer);
    rSt.ReadInt32( fcSttbfglsy );
    lcbSttbfglsy = Readcb(rSt, eVer);
    rSt.ReadInt32( fcPlcfglsy );
    lcbPlcfglsy = Readcb(rSt, eVer);
    rSt.ReadInt32( fcPlcfhdd );
    lcbPlcfhdd = Readcb(rSt, eVer);
    rSt.ReadInt32( fcPlcfbteChpx );
    lcbPlcfbteChpx = Readcb(rSt, eVer);
    rSt.ReadInt32( fcPlcfbtePapx );
    lcbPlcfbtePapx = Readcb(rSt, eVer);
    rSt.ReadInt32( fcPlcfsea );
    lcbPlcfsea = Readcb(rSt, eVer);
    rSt.ReadInt32( fcSttbfffn );
    lcbSttbfffn = Readcb(rSt, eVer);
    rSt.ReadInt32( fcPlcffldMom );
    lcbPlcffldMom = Readcb(rSt, eVer);
    rSt.ReadInt32( fcPlcffldHdr );
    lcbPlcffldHdr = Readcb(rSt, eVer);
    rSt.ReadInt32( fcPlcffldFtn );
    lcbPlcffldFtn = Readcb(rSt, eVer);
    rSt.ReadInt32( fcPlcffldAtn );
    lcbPlcffldAtn = Readcb(rSt, eVer);
    rSt.ReadInt32( fcPlcffldMcr );
    lcbPlcffldMcr = Readcb(rSt, eVer);
    rSt.ReadInt32( fcSttbfbkmk );
    lcbSttbfbkmk = Readcb(rSt, eVer);
    rSt.ReadInt32( fcPlcfbkf );
    lcbPlcfbkf = Readcb(rSt, eVer);
    rSt.ReadInt32( fcPlcfbkl );
    lcbPlcfbkl = Readcb(rSt, eVer);
    rSt.ReadInt32( fcCmds );
    lcbCmds = Readcb(rSt, eVer);
    rSt.ReadInt32( fcPlcfmcr );
    lcbPlcfmcr = Readcb(rSt, eVer);
    rSt.ReadInt32( fcSttbfmcr );
    lcbSttbfmcr = Readcb(rSt, eVer);
    rSt.ReadInt32( fcPrDrvr );
    lcbPrDrvr = Readcb(rSt, eVer);
    rSt.ReadInt32( fcPrEnvPort );
    lcbPrEnvPort = Readcb(rSt, eVer);
    rSt.ReadInt32( fcPrEnvLand );
    lcbPrEnvLand = Readcb(rSt, eVer);
    rSt.ReadInt32( fcWss );
    lcbWss = Readcb(rSt, eVer);
    rSt.ReadInt32( fcDop );
    lcbDop = Readcb(rSt, eVer);
    rSt.ReadInt32( fcSttbfAssoc );
    lcbSttbfAssoc = Readcb(rSt, eVer);
    rSt.ReadInt32( fcClx );
    lcbClx = Readcb(rSt, eVer);
    rSt.ReadInt32( fcPlcfpgdFtn );
    lcbPlcfpgdFtn = Readcb(rSt, eVer);
    rSt.ReadInt32( fcAutosaveSource );
    lcbAutosaveSource = Readcb(rSt, eVer);
    rSt.ReadInt32( fcGrpStAtnOwners );
    lcbGrpStAtnOwners = Readcb(rSt, eVer);
    rSt.ReadInt32( fcSttbfAtnbkmk );
    lcbSttbfAtnbkmk = Readcb(rSt, eVer);

    
    if (IsSevenMinus(eVer))
    {
        rSt.SeekRel( 1*sizeof( sal_Int16) );

        
        rSt.ReadInt16( pnChpFirst_Ver67 );
        rSt.ReadInt16( pnPapFirst_Ver67 );
        rSt.ReadInt16( cpnBteChp_Ver67 );
        rSt.ReadInt16( cpnBtePap_Ver67 );
    }

    if (eVer > ww::eWW2)
    {
        rSt.ReadInt32( fcPlcfdoaMom );
        rSt.ReadInt32( lcbPlcfdoaMom );
        rSt.ReadInt32( fcPlcfdoaHdr );
        rSt.ReadInt32( lcbPlcfdoaHdr );
        rSt.ReadInt32( fcPlcfspaMom );
        rSt.ReadInt32( lcbPlcfspaMom );
        rSt.ReadInt32( fcPlcfspaHdr );
        rSt.ReadInt32( lcbPlcfspaHdr );

        rSt.ReadInt32( fcPlcfAtnbkf );
        rSt.ReadInt32( lcbPlcfAtnbkf );
        rSt.ReadInt32( fcPlcfAtnbkl );
        rSt.ReadInt32( lcbPlcfAtnbkl );
        rSt.ReadInt32( fcPms );
        rSt.ReadInt32( lcbPMS );
        rSt.ReadInt32( fcFormFldSttbf );
        rSt.ReadInt32( lcbFormFldSttbf );
        rSt.ReadInt32( fcPlcfendRef );
        rSt.ReadInt32( lcbPlcfendRef );
        rSt.ReadInt32( fcPlcfendTxt );
        rSt.ReadInt32( lcbPlcfendTxt );
        rSt.ReadInt32( fcPlcffldEdn );
        rSt.ReadInt32( lcbPlcffldEdn );
        rSt.ReadInt32( fcPlcfpgdEdn );
        rSt.ReadInt32( lcbPlcfpgdEdn );
        rSt.ReadInt32( fcDggInfo );
        rSt.ReadInt32( lcbDggInfo );
        rSt.ReadInt32( fcSttbfRMark );
        rSt.ReadInt32( lcbSttbfRMark );
        rSt.ReadInt32( fcSttbfCaption );
        rSt.ReadInt32( lcbSttbfCaption );
        rSt.ReadInt32( fcSttbAutoCaption );
        rSt.ReadInt32( lcbSttbAutoCaption );
        rSt.ReadInt32( fcPlcfwkb );
        rSt.ReadInt32( lcbPlcfwkb );
        rSt.ReadInt32( fcPlcfspl );
        rSt.ReadInt32( lcbPlcfspl );
        rSt.ReadInt32( fcPlcftxbxTxt );
        rSt.ReadInt32( lcbPlcftxbxTxt );
        rSt.ReadInt32( fcPlcffldTxbx );
        rSt.ReadInt32( lcbPlcffldTxbx );
        rSt.ReadInt32( fcPlcfHdrtxbxTxt );
        rSt.ReadInt32( lcbPlcfHdrtxbxTxt );
        rSt.ReadInt32( fcPlcffldHdrTxbx );
        rSt.ReadInt32( lcbPlcffldHdrTxbx );
        rSt.ReadInt32( fcStwUser );
        rSt.ReadUInt32( lcbStwUser );
        rSt.ReadInt32( fcSttbttmbd );
        rSt.ReadUInt32( lcbSttbttmbd );
    }

    if( 0 == rSt.GetError() )
    {
        
        fDot        =   aBits1 & 0x01       ;
        fGlsy       = ( aBits1 & 0x02 ) >> 1;
        fComplex    = ( aBits1 & 0x04 ) >> 2;
        fHasPic     = ( aBits1 & 0x08 ) >> 3;
        cQuickSaves = ( aBits1 & 0xf0 ) >> 4;
        fEncrypted  =   aBits2 & 0x01       ;
        fWhichTblStm= ( aBits2 & 0x02 ) >> 1;
        fReadOnlyRecommended = (aBits2 & 0x4) >> 2;
        fWriteReservation = (aBits2 & 0x8) >> 3;
        fExtChar    = ( aBits2 & 0x10 ) >> 4;
        
        fFarEast    = ( aBits2 & 0x40 ) >> 6; 
        

        /*
            ggfs. Ziel-Varaiblen, aus xxx_Ver67 fuellen
            oder Flags setzen
        */
        if (IsSevenMinus(eVer))
        {
            pnChpFirst = pnChpFirst_Ver67;
            pnPapFirst = pnPapFirst_Ver67;
            cpnBteChp = cpnBteChp_Ver67;
            cpnBtePap = cpnBtePap_Ver67;
        }
        else if (IsEightPlus(eVer))
        {
          fMac              =   aVer8Bits1  & 0x01           ;
          fEmptySpecial     = ( aVer8Bits1  & 0x02 ) >> 1;
          fLoadOverridePage = ( aVer8Bits1  & 0x04 ) >> 2;
          fFuturesavedUndo  = ( aVer8Bits1  & 0x08 ) >> 3;
          fWord97Saved      = ( aVer8Bits1  & 0x10 ) >> 4;
          fWord2000Saved    = ( aVer8Bits1  & 0x20 ) >> 5;

            /*
                speziell fuer WW8:
                ermittle die Werte fuer PLCF LST und PLF LFO
                und PLCF fuer TextBox-Break-Deskriptoren
            */
            long nOldPos = rSt.Tell();

            rSt.Seek( 0x02da );
            rSt.ReadInt32( fcSttbFnm );
            rSt.ReadInt32( lcbSttbFnm );
            rSt.ReadInt32( fcPlcfLst );
            rSt.ReadInt32( lcbPlcfLst );
            rSt.ReadInt32( fcPlfLfo );
            rSt.ReadInt32( lcbPlfLfo );
            rSt.ReadInt32( fcPlcftxbxBkd );
            rSt.ReadInt32( lcbPlcftxbxBkd );
            rSt.ReadInt32( fcPlcfHdrtxbxBkd );
            rSt.ReadInt32( lcbPlcfHdrtxbxBkd );
            if( 0 != rSt.GetError() )
            {
                nFibError = ERR_SWG_READ_ERROR;
            }

            rSt.Seek( 0x372 );          
            rSt.ReadInt32( fcSttbListNames );
            rSt.ReadInt32( lcbSttbListNames );

            if (cfclcb > 93)
            {
                rSt.Seek( 0x382 );          
                rSt.ReadInt32( fcPlcfTch );
                rSt.ReadInt32( lcbPlcfTch );
            }

            if (cfclcb > 113)
            {
                rSt.Seek( 0x41A );          
                rSt.ReadInt32( fcAtrdExtra );
                rSt.ReadUInt32( lcbAtrdExtra );
            }

            if( 0 != rSt.GetError() )
                nFibError = ERR_SWG_READ_ERROR;

            rSt.Seek( 0x5bc );          
            rSt.ReadUInt16( nFib_actual );

            rSt.Seek( nOldPos );
        }
    }
    else
    {
        nFibError = ERR_SWG_READ_ERROR;     
    }
}


WW8Fib::WW8Fib(sal_uInt8 nVer, bool bDot)
{
    memset(this, 0, sizeof(*this));
    nVersion = nVer;
    if (8 == nVer)
    {
        fcMin = 0x800;
        wIdent = 0xa5ec;
        nFib = 0x0101;
        nFibBack = 0xbf;
        nProduct = 0x204D;
        fDot = bDot;

        csw = 0x0e;     
        cfclcb = 0x88;  
        clw = 0x16;     
        pnFbpChpFirst = pnFbpPapFirst = pnFbpLvcFirst = 0x000fffff;
        fExtChar = true;
        fWord97Saved = fWord2000Saved = true;

        wMagicCreated = 0x6143;
        wMagicRevised = 0x6C6F;
        wMagicCreatedPrivate = 0x6E61;
        wMagicRevisedPrivate = 0x3038;
    }
    else
    {
        fcMin = 0x300;
        wIdent = 0xa5dc;
        nFib = nFibBack = 0x65;
        nProduct = 0xc02d;
    }

    
    cQuickSaves = nFib >= 0x00D9 ? 0xF : 0;

    
    lid = 0x409; 

    LanguageType nLang = Application::GetSettings().GetLanguageTag().getLanguageType();
    fFarEast = MsLangId::isCJK(nLang);
    if (fFarEast)
        lidFE = nLang;
    else
        lidFE = lid;

    LanguageTag aLanguageTag( lid );
    LocaleDataWrapper aLocaleWrapper( aLanguageTag );
    nNumDecimalSep = aLocaleWrapper.getNumDecimalSep()[0];
}

sal_Unicode WW8Fib::getNumDecimalSep() const
{
    return nNumDecimalSep;
}

bool WW8Fib::WriteHeader(SvStream& rStrm)
{
    bool bVer8 = 8 == nVersion;

    size_t nUnencryptedHdr = bVer8 ? 0x44 : 0x24;
    sal_uInt8 *pDataPtr = new sal_uInt8[ nUnencryptedHdr ];
    sal_uInt8 *pData = pDataPtr;
    memset( pData, 0, nUnencryptedHdr );

    sal_uLong nPos = rStrm.Tell();
    cbMac = rStrm.Seek( STREAM_SEEK_TO_END );
    rStrm.Seek( nPos );

    Set_UInt16( pData, wIdent );
    Set_UInt16( pData, nFib );
    Set_UInt16( pData, nProduct );
    Set_UInt16( pData, lid );
    Set_UInt16( pData, pnNext );

    sal_uInt16 nBits16 = 0;
    if( fDot )          nBits16 |= 0x0001;
    if( fGlsy)          nBits16 |= 0x0002;
    if( fComplex )      nBits16 |= 0x0004;
    if( fHasPic )       nBits16 |= 0x0008;
    nBits16 |= (0xf0 & ( cQuickSaves << 4 ));
    if( fEncrypted )    nBits16 |= 0x0100;
    if( fWhichTblStm )  nBits16 |= 0x0200;

    if (fReadOnlyRecommended)
        nBits16 |= 0x0400;
    if (fWriteReservation)
        nBits16 |= 0x0800;

    if( fExtChar )      nBits16 |= 0x1000;
    if( fFarEast )      nBits16 |= 0x4000;  
    if( fObfuscated )   nBits16 |= 0x8000;
    Set_UInt16( pData, nBits16 );

    Set_UInt16( pData, nFibBack );
    Set_UInt16( pData, nHash );
    Set_UInt16( pData, nKey );
    Set_UInt8( pData, envr );

    sal_uInt8 nBits8 = 0;
    if( bVer8 )
    {
        if( fMac )                  nBits8 |= 0x0001;
        if( fEmptySpecial )         nBits8 |= 0x0002;
        if( fLoadOverridePage )     nBits8 |= 0x0004;
        if( fFuturesavedUndo )      nBits8 |= 0x0008;
        if( fWord97Saved )          nBits8 |= 0x0010;
        if( fWord2000Saved )        nBits8 |= 0x0020;
    }
    
    Set_UInt8( pData, nBits8  );

    Set_UInt16( pData, chse );
    Set_UInt16( pData, chseTables );
    Set_UInt32( pData, fcMin );
    Set_UInt32( pData, fcMac );



    
    if( bVer8 )
    {
        Set_UInt16( pData, csw );
        Set_UInt16( pData, wMagicCreated );
        Set_UInt16( pData, wMagicRevised );
        Set_UInt16( pData, wMagicCreatedPrivate );
        Set_UInt16( pData, wMagicRevisedPrivate );
        pData += 9 * sizeof( sal_Int16 );
        Set_UInt16( pData, lidFE );
        Set_UInt16( pData, clw );
    }



    
    Set_UInt32( pData, cbMac );

    rStrm.Write( pDataPtr, nUnencryptedHdr );
    delete[] pDataPtr;
    return 0 == rStrm.GetError();
}

bool WW8Fib::Write(SvStream& rStrm)
{
    bool bVer8 = 8 == nVersion;

    WriteHeader( rStrm );

    size_t nUnencryptedHdr = bVer8 ? 0x44 : 0x24;

    sal_uInt8 *pDataPtr = new sal_uInt8[ fcMin - nUnencryptedHdr ];
    sal_uInt8 *pData = pDataPtr;
    memset( pData, 0, fcMin - nUnencryptedHdr );

    sal_uLong nPos = rStrm.Tell();
    cbMac = rStrm.Seek( STREAM_SEEK_TO_END );
    rStrm.Seek( nPos );

    
    pData += 2 * sizeof( sal_Int32);

    
    if( !bVer8 )
        pData += 2 * sizeof( sal_Int32);

    Set_UInt32( pData, ccpText );
    Set_UInt32( pData, ccpFtn );
    Set_UInt32( pData, ccpHdr );
    Set_UInt32( pData, ccpMcr );
    Set_UInt32( pData, ccpAtn );
    Set_UInt32( pData, ccpEdn );
    Set_UInt32( pData, ccpTxbx );
    Set_UInt32( pData, ccpHdrTxbx );

        
    if( !bVer8 )
        pData += 1 * sizeof( sal_Int32);


    if( bVer8 )
    {
        Set_UInt32( pData, pnFbpChpFirst );
        Set_UInt32( pData, pnChpFirst );
        Set_UInt32( pData, cpnBteChp );
        Set_UInt32( pData, pnFbpPapFirst );
        Set_UInt32( pData, pnPapFirst );
        Set_UInt32( pData, cpnBtePap );
        Set_UInt32( pData, pnFbpLvcFirst );
        Set_UInt32( pData, pnLvcFirst );
        Set_UInt32( pData, cpnBteLvc );
        Set_UInt32( pData, fcIslandFirst );
        Set_UInt32( pData, fcIslandLim );
        Set_UInt16( pData, cfclcb );
    }


    
    Set_UInt32( pData, fcStshfOrig );
    Set_UInt32( pData, lcbStshfOrig );
    Set_UInt32( pData, fcStshf );
    Set_UInt32( pData, lcbStshf );
    Set_UInt32( pData, fcPlcffndRef );
    Set_UInt32( pData, lcbPlcffndRef );
    Set_UInt32( pData, fcPlcffndTxt );
    Set_UInt32( pData, lcbPlcffndTxt );
    Set_UInt32( pData, fcPlcfandRef );
    Set_UInt32( pData, lcbPlcfandRef );
    Set_UInt32( pData, fcPlcfandTxt );
    Set_UInt32( pData, lcbPlcfandTxt );
    Set_UInt32( pData, fcPlcfsed );
    Set_UInt32( pData, lcbPlcfsed );
    Set_UInt32( pData, fcPlcfpad );
    Set_UInt32( pData, lcbPlcfpad );
    Set_UInt32( pData, fcPlcfphe );
    Set_UInt32( pData, lcbPlcfphe );
    Set_UInt32( pData, fcSttbfglsy );
    Set_UInt32( pData, lcbSttbfglsy );
    Set_UInt32( pData, fcPlcfglsy );
    Set_UInt32( pData, lcbPlcfglsy );
    Set_UInt32( pData, fcPlcfhdd );
    Set_UInt32( pData, lcbPlcfhdd );
    Set_UInt32( pData, fcPlcfbteChpx );
    Set_UInt32( pData, lcbPlcfbteChpx );
    Set_UInt32( pData, fcPlcfbtePapx );
    Set_UInt32( pData, lcbPlcfbtePapx );
    Set_UInt32( pData, fcPlcfsea );
    Set_UInt32( pData, lcbPlcfsea );
    Set_UInt32( pData, fcSttbfffn );
    Set_UInt32( pData, lcbSttbfffn );
    Set_UInt32( pData, fcPlcffldMom );
    Set_UInt32( pData, lcbPlcffldMom );
    Set_UInt32( pData, fcPlcffldHdr );
    Set_UInt32( pData, lcbPlcffldHdr );
    Set_UInt32( pData, fcPlcffldFtn );
    Set_UInt32( pData, lcbPlcffldFtn );
    Set_UInt32( pData, fcPlcffldAtn );
    Set_UInt32( pData, lcbPlcffldAtn );
    Set_UInt32( pData, fcPlcffldMcr );
    Set_UInt32( pData, lcbPlcffldMcr );
    Set_UInt32( pData, fcSttbfbkmk );
    Set_UInt32( pData, lcbSttbfbkmk );
    Set_UInt32( pData, fcPlcfbkf );
    Set_UInt32( pData, lcbPlcfbkf );
    Set_UInt32( pData, fcPlcfbkl );
    Set_UInt32( pData, lcbPlcfbkl );
    Set_UInt32( pData, fcCmds );
    Set_UInt32( pData, lcbCmds );
    Set_UInt32( pData, fcPlcfmcr );
    Set_UInt32( pData, lcbPlcfmcr );
    Set_UInt32( pData, fcSttbfmcr );
    Set_UInt32( pData, lcbSttbfmcr );
    Set_UInt32( pData, fcPrDrvr );
    Set_UInt32( pData, lcbPrDrvr );
    Set_UInt32( pData, fcPrEnvPort );
    Set_UInt32( pData, lcbPrEnvPort );
    Set_UInt32( pData, fcPrEnvLand );
    Set_UInt32( pData, lcbPrEnvLand );
    Set_UInt32( pData, fcWss );
    Set_UInt32( pData, lcbWss );
    Set_UInt32( pData, fcDop );
    Set_UInt32( pData, lcbDop );
    Set_UInt32( pData, fcSttbfAssoc );
    Set_UInt32( pData, lcbSttbfAssoc );
    Set_UInt32( pData, fcClx );
    Set_UInt32( pData, lcbClx );
    Set_UInt32( pData, fcPlcfpgdFtn );
    Set_UInt32( pData, lcbPlcfpgdFtn );
    Set_UInt32( pData, fcAutosaveSource );
    Set_UInt32( pData, lcbAutosaveSource );
    Set_UInt32( pData, fcGrpStAtnOwners );
    Set_UInt32( pData, lcbGrpStAtnOwners );
    Set_UInt32( pData, fcSttbfAtnbkmk );
    Set_UInt32( pData, lcbSttbfAtnbkmk );

    
    if( !bVer8 )
    {
        pData += 1*sizeof( sal_Int16);
        Set_UInt16( pData, (sal_uInt16)pnChpFirst );
        Set_UInt16( pData, (sal_uInt16)pnPapFirst );
        Set_UInt16( pData, (sal_uInt16)cpnBteChp );
        Set_UInt16( pData, (sal_uInt16)cpnBtePap );
    }

    Set_UInt32( pData, fcPlcfdoaMom ); 
    Set_UInt32( pData, lcbPlcfdoaMom ); 
    Set_UInt32( pData, fcPlcfdoaHdr ); 
    Set_UInt32( pData, lcbPlcfdoaHdr ); 

    Set_UInt32( pData, fcPlcfspaMom ); 
    Set_UInt32( pData, lcbPlcfspaMom ); 
    Set_UInt32( pData, fcPlcfspaHdr ); 
    Set_UInt32( pData, lcbPlcfspaHdr ); 

    Set_UInt32( pData, fcPlcfAtnbkf );
    Set_UInt32( pData, lcbPlcfAtnbkf );
    Set_UInt32( pData, fcPlcfAtnbkl );
    Set_UInt32( pData, lcbPlcfAtnbkl );
    Set_UInt32( pData, fcPms );
    Set_UInt32( pData, lcbPMS );
    Set_UInt32( pData, fcFormFldSttbf );
    Set_UInt32( pData, lcbFormFldSttbf );
    Set_UInt32( pData, fcPlcfendRef );
    Set_UInt32( pData, lcbPlcfendRef );
    Set_UInt32( pData, fcPlcfendTxt );
    Set_UInt32( pData, lcbPlcfendTxt );
    Set_UInt32( pData, fcPlcffldEdn );
    Set_UInt32( pData, lcbPlcffldEdn );
    Set_UInt32( pData, fcPlcfpgdEdn );
    Set_UInt32( pData, lcbPlcfpgdEdn );
    Set_UInt32( pData, fcDggInfo ); 
    Set_UInt32( pData, lcbDggInfo ); 
    Set_UInt32( pData, fcSttbfRMark );
    Set_UInt32( pData, lcbSttbfRMark );
    Set_UInt32( pData, fcSttbfCaption );
    Set_UInt32( pData, lcbSttbfCaption );
    Set_UInt32( pData, fcSttbAutoCaption );
    Set_UInt32( pData, lcbSttbAutoCaption );
    Set_UInt32( pData, fcPlcfwkb );
    Set_UInt32( pData, lcbPlcfwkb );
    Set_UInt32( pData, fcPlcfspl ); 
    Set_UInt32( pData, lcbPlcfspl ); 
    Set_UInt32( pData, fcPlcftxbxTxt );
    Set_UInt32( pData, lcbPlcftxbxTxt );
    Set_UInt32( pData, fcPlcffldTxbx );
    Set_UInt32( pData, lcbPlcffldTxbx );
    Set_UInt32( pData, fcPlcfHdrtxbxTxt );
    Set_UInt32( pData, lcbPlcfHdrtxbxTxt );
    Set_UInt32( pData, fcPlcffldHdrTxbx );
    Set_UInt32( pData, lcbPlcffldHdrTxbx );

    if( bVer8 )
    {
        pData += 0x2da - 0x27a;         
        Set_UInt32( pData, fcSttbFnm);
        Set_UInt32( pData, lcbSttbFnm);
        Set_UInt32( pData, fcPlcfLst );
        Set_UInt32( pData, lcbPlcfLst );
        Set_UInt32( pData, fcPlfLfo );
        Set_UInt32( pData, lcbPlfLfo );
        Set_UInt32( pData, fcPlcftxbxBkd );
        Set_UInt32( pData, lcbPlcftxbxBkd );
        Set_UInt32( pData, fcPlcfHdrtxbxBkd );
        Set_UInt32( pData, lcbPlcfHdrtxbxBkd );

        pData += 0x372 - 0x302; 
        Set_UInt32( pData, fcSttbListNames );
        Set_UInt32( pData, lcbSttbListNames );

        pData += 0x382 - 0x37A;
        Set_UInt32( pData, fcPlcfTch );
        Set_UInt32( pData, lcbPlcfTch );

        pData += 0x3FA - 0x38A;
        Set_UInt16( pData, (sal_uInt16)0x0002);
        Set_UInt16( pData, (sal_uInt16)0x00D9);

        pData += 0x41A - 0x3FE;
        Set_UInt32( pData, fcAtrdExtra );
        Set_UInt32( pData, lcbAtrdExtra );

        pData += 0x4DA - 0x422;
        Set_UInt32( pData, fcHplxsdr );
        Set_UInt32( pData, 0);
    }

    rStrm.Write( pDataPtr, fcMin - nUnencryptedHdr );
    delete[] pDataPtr;
    return 0 == rStrm.GetError();
}

rtl_TextEncoding WW8Fib::GetFIBCharset(sal_uInt16 chs)
{
    OSL_ENSURE(chs <= 0x100, "overflowed winword charset set");
    rtl_TextEncoding eCharSet =
        (0x0100 == chs)
        ? RTL_TEXTENCODING_APPLE_ROMAN
        : rtl_getTextEncodingFromWindowsCharset( static_cast<sal_uInt8>(chs) );
    return eCharSet;
}

WW8Style::WW8Style(SvStream& rStream, WW8Fib& rFibPara)
    : rFib(rFibPara), rSt(rStream), cstd(0), cbSTDBaseInFile(0),
    stiMaxWhenSaved(0), istdMaxFixedWhenSaved(0), nVerBuiltInNamesWhenSaved(0),
    ftcAsci(0), ftcFE(0), ftcOther(0), ftcBi(0)
{
    if (!checkSeek(rSt, rFib.fcStshf))
        return;

    sal_uInt16 cbStshi = 0; 
    sal_uInt32 nRemaining = rFib.lcbStshf;
    const sal_uInt32 nMinValidStshi = 4;

    if (rFib.GetFIBVersion() <= ww::eWW2)
    {
        cbStshi = 0;
        cstd = 256;
    }
    else
    {
        if (rFib.nFib < 67) 
            cbStshi = nMinValidStshi;
        else    
        {
            if (nRemaining < sizeof(cbStshi))
                return;
            
            rSt.ReadUInt16( cbStshi );
            nRemaining-=2;
        }
    }

    cbStshi = std::min(static_cast<sal_uInt32>(cbStshi), nRemaining);
    if (cbStshi < nMinValidStshi)
        return;

    sal_uInt16 nRead = cbStshi;
    do
    {
        sal_uInt16 a16Bit;

        rSt.ReadUInt16( cstd );

        rSt.ReadUInt16( cbSTDBaseInFile );

        if(  6 > nRead ) break;
        rSt.ReadUInt16( a16Bit );
        fStdStylenamesWritten = a16Bit & 0x0001;

        if(  8 > nRead ) break;
        rSt.ReadUInt16( stiMaxWhenSaved );

        if( 10 > nRead ) break;
        rSt.ReadUInt16( istdMaxFixedWhenSaved );

        if( 12 > nRead ) break;
        rSt.ReadUInt16( nVerBuiltInNamesWhenSaved );

        if( 14 > nRead ) break;
        rSt.ReadUInt16( ftcAsci );

        if( 16 > nRead ) break;
        rSt.ReadUInt16( ftcFE );

        if ( 18 > nRead ) break;
        rSt.ReadUInt16( ftcOther );

        ftcBi = ftcOther;

        if ( 20 > nRead ) break;
        rSt.ReadUInt16( ftcBi );

        
        if( 20 < nRead )
            rSt.SeekRel( nRead-20 );
    }
    while( false ); 
                

    nRemaining -= cbStshi;

    
    
    const sal_uInt32 nMinRecordSize = sizeof(sal_uInt16);
    sal_uInt16 nMaxPossibleRecords = nRemaining/nMinRecordSize;

    OSL_ENSURE(cstd <= nMaxPossibleRecords,
        "allegedly more styles that available data\n");
    cstd = std::min(cstd, nMaxPossibleRecords);
}





WW8_STD* WW8Style::Read1STDFixed( short& rSkip, short* pcbStd )
{
    WW8_STD* pStd = 0;

    sal_uInt16 cbStd(0);
    rSt.ReadUInt16( cbStd );   

    sal_uInt16 nRead = cbSTDBaseInFile;
    if( cbStd >= cbSTDBaseInFile )
    {
        

        
        pStd = new WW8_STD;
        memset( pStd, 0, sizeof( *pStd ) );

        do
        {
            sal_uInt16 a16Bit;

            if( 2 > nRead ) break;
            a16Bit = 0;
            rSt.ReadUInt16( a16Bit );
            pStd->sti          =        a16Bit & 0x0fff  ;
            pStd->fScratch     = sal_uInt16(0 != ( a16Bit & 0x1000 ));
            pStd->fInvalHeight = sal_uInt16(0 != ( a16Bit & 0x2000 ));
            pStd->fHasUpe      = sal_uInt16(0 != ( a16Bit & 0x4000 ));
            pStd->fMassCopy    = sal_uInt16(0 != ( a16Bit & 0x8000 ));

            if( 4 > nRead ) break;
            a16Bit = 0;
            rSt.ReadUInt16( a16Bit );
            pStd->sgc      =   a16Bit & 0x000f       ;
            pStd->istdBase = ( a16Bit & 0xfff0 ) >> 4;

            if( 6 > nRead ) break;
            a16Bit = 0;
            rSt.ReadUInt16( a16Bit );
            pStd->cupx     =   a16Bit & 0x000f       ;
            pStd->istdNext = ( a16Bit & 0xfff0 ) >> 4;

            if( 8 > nRead ) break;
            a16Bit = 0;
            rSt.ReadUInt16( pStd->bchUpe );

            
            if(10 > nRead ) break;
            a16Bit = 0;
            rSt.ReadUInt16( a16Bit );
            pStd->fAutoRedef =   a16Bit & 0x0001       ;
            pStd->fHidden    = ( a16Bit & 0x0002 ) >> 1;

            
            
            if( 10 < nRead )
                rSt.SeekRel( nRead-10 );
        }
        while( false ); 
                    

        if( (0 != rSt.GetError()) || !nRead )
            DELETEZ( pStd );        

      rSkip = cbStd - cbSTDBaseInFile;
    }
    else
    {           
        if( cbStd )
            rSt.SeekRel( cbStd );           
        rSkip = 0;
    }
    if( pcbStd )
        *pcbStd = cbStd;
    return pStd;
}

WW8_STD* WW8Style::Read1Style( short& rSkip, OUString* pString, short* pcbStd )
{
    
    

    WW8_STD* pStd = Read1STDFixed( rSkip, pcbStd );         

    
    if( pString )
    {   
        if ( pStd )
        {
            switch( rFib.nVersion )
            {
                case 6:
                case 7:
                    
                    *pString = read_uInt8_BeltAndBracesString(rSt, RTL_TEXTENCODING_MS_1252);
                    
                    rSkip -= pString->getLength() + 2;
                    break;
                case 8:
                    
                    
                    if (TestBeltAndBraces(rSt))
                    {
                        *pString = read_uInt16_BeltAndBracesString(rSt);
                        rSkip -= (pString->getLength() + 2) * 2;
                    }
                    else
                    {
                        /*
                        #i8114#
                        This is supposed to be impossible, its just supposed
                        to be 16 bit count followed by the string and ending
                        in a 0 short. But "Lotus SmartSuite Product: Word Pro"
                        is creating invalid style names in ww7- format. So we
                        use the belt and braces of the ms strings to see if
                        they are not corrupt. If they are then we try them as
                        8bit ones
                        */
                        *pString = read_uInt8_BeltAndBracesString(rSt,RTL_TEXTENCODING_MS_1252);
                        
                        rSkip -= pString->getLength() + 2;
                    }
                    break;
                default:
                    OSL_ENSURE(!this, "Es wurde vergessen, nVersion zu kodieren!");
                    break;
            }
        }
        else
            *pString = OUString();   
    }
    return pStd;
}





struct WW8_FFN_Ver6 : public WW8_FFN_BASE
{
    
    sal_Char szFfn[65]; 
                        
                        
                        
                        
                        
                        
};
struct WW8_FFN_Ver8 : public WW8_FFN_BASE
{
    
    
    sal_Char panose[ 10 ];  
    sal_Char fs[ 24     ];  

    
    sal_uInt16 szFfn[65];   
                        
                        
                        
                        
                        
                        
};


static void lcl_checkFontname( OUString& sString )
{
    
    
    
    
    
    

    
    OUStringBuffer aBuf(sString);
    const sal_Int32 nLen = aBuf.getLength();
    bool bFound = false;
    for ( sal_Int32 n = 0; n < nLen; ++n )
    {
        if ( aBuf[n] < 0x20 )
        {
            aBuf[n] = 1;
            bFound = true;
        }
    }
    sString = aBuf.makeStringAndClear();

    
    if( bFound )
    {
        sString = comphelper::string::strip(sString.replaceAll("\001", ""), ';');
    }
}

namespace
{
    sal_uInt16 calcMaxFonts(sal_uInt8 *p, sal_Int32 nFFn)
    {
        
        sal_uInt16 nMax = 0;
        sal_Int32 nRemaining = nFFn;
        while (nRemaining)
        {
            
            
            sal_uInt16 cbFfnM1 = *p++;
            --nRemaining;

            if (cbFfnM1 > nRemaining)
                break;

            nMax++;
            nRemaining -= cbFfnM1;
            p += cbFfnM1;
        }
        return nMax;
    }
}

WW8Fonts::WW8Fonts( SvStream& rSt, WW8Fib& rFib )
    : pFontA(0), nMax(0)
{
    
    
    if( rFib.lcbSttbfffn <= 2 )
    {
        OSL_ENSURE( !this, "Fonttabelle kaputt! (rFib.lcbSttbfffn < 2)" );
        return;
    }

    if (!checkSeek(rSt, rFib.fcSttbfffn))
        return;

    sal_Int32 nFFn = rFib.lcbSttbfffn - 2;

    
    sal_uInt8* pA = new sal_uInt8[nFFn];
    memset(pA, 0, nFFn);

    ww::WordVersion eVersion = rFib.GetFIBVersion();

    if( eVersion >= ww::eWW8 )
    {
        
        rSt.ReadUInt16( nMax );
    }

    
    
    
    rSt.SeekRel( 2 );

    
    nFFn = rSt.Read(pA, nFFn);
    sal_uInt16 nCalcMax = calcMaxFonts(pA, nFFn);

    if (eVersion < ww::eWW8)
        nMax = nCalcMax;
    else
    {
        
        
        nMax = std::min(nMax, nCalcMax);
    }

    if( nMax )
    {
        
        pFontA = new WW8_FFN[ nMax ];
        WW8_FFN* p = pFontA;

        if( eVersion <= ww::eWW2 )
        {
            WW8_FFN_BASE* pVer2 = (WW8_FFN_BASE*)pA;
            for(sal_uInt16 i=0; i<nMax; ++i, ++p)
            {
                p->cbFfnM1   = pVer2->cbFfnM1;

                p->prg       =  0;
                p->fTrueType = 0;
                p->ff        = 0;

                p->wWeight   = ( *(((sal_uInt8*)pVer2) + 1) );
                p->chs   = ( *(((sal_uInt8*)pVer2) + 2) );
                /*
                 #i8726# 7- seems to encode the name in the same encoding as
                 the font, e.g load the doc in 97 and save to see the unicode
                 ver of the asian fontnames in that example to confirm.
                */
                rtl_TextEncoding eEnc = WW8Fib::GetFIBCharset(p->chs);
                if ((eEnc == RTL_TEXTENCODING_SYMBOL) || (eEnc == RTL_TEXTENCODING_DONTKNOW))
                    eEnc = RTL_TEXTENCODING_MS_1252;

                p->sFontname = OUString ( (((const sal_Char*)pVer2) + 1 + 2), strlen((((const sal_Char*)pVer2) + 1 + 2)), eEnc);
                pVer2 = (WW8_FFN_BASE*)( ((sal_uInt8*)pVer2) + pVer2->cbFfnM1 + 1 );
            }
        }
        else if( eVersion < ww::eWW8 )
        {
            WW8_FFN_Ver6* pVer6 = (WW8_FFN_Ver6*)pA;
            sal_uInt8 c2;
            for(sal_uInt16 i=0; i<nMax; ++i, ++p)
            {
                p->cbFfnM1   = pVer6->cbFfnM1;
                c2           = *(((sal_uInt8*)pVer6) + 1);

                p->prg       =  c2 & 0x02;
                p->fTrueType = (c2 & 0x04) >> 2;
                
                p->ff        = (c2 & 0x70) >> 4;

                p->wWeight   = SVBT16ToShort( *(SVBT16*)&pVer6->wWeight );
                p->chs       = pVer6->chs;
                p->ibszAlt   = pVer6->ibszAlt;
                /*
                 #i8726# 7- seems to encode the name in the same encoding as
                 the font, e.g load the doc in 97 and save to see the unicode
                 ver of the asian fontnames in that example to confirm.
                 */
                rtl_TextEncoding eEnc = WW8Fib::GetFIBCharset(p->chs);
                if ((eEnc == RTL_TEXTENCODING_SYMBOL) || (eEnc == RTL_TEXTENCODING_DONTKNOW))
                    eEnc = RTL_TEXTENCODING_MS_1252;
                p->sFontname = OUString(pVer6->szFfn, rtl_str_getLength(pVer6->szFfn), eEnc);
                const sal_uInt16 maxStrSize = sizeof (pVer6->szFfn) / sizeof (pVer6->szFfn[0]);
                if (p->ibszAlt && p->ibszAlt < maxStrSize) 
                {
                    const sal_Char *pAlt = pVer6->szFfn+p->ibszAlt;
                    p->sFontname += ";" + OUString(pAlt, rtl_str_getLength(pAlt), eEnc);
                }
                else
                {
                    
                    if (
                         RTL_TEXTENCODING_SYMBOL == WW8Fib::GetFIBCharset(p->chs)
                         && p->sFontname!="Symbol"
                       )
                    {
                        p->sFontname += ";Symbol";
                    }
                }
                pVer6 = (WW8_FFN_Ver6*)( ((sal_uInt8*)pVer6) + pVer6->cbFfnM1 + 1 );
            }
        }
        else
        {
            
            const sal_uInt8 cbMinFFNPayload = 41;
            sal_uInt16 nValidFonts = 0;
            sal_Int32 nRemainingFFn = nFFn;
            sal_uInt8* pRaw = pA;
            for (sal_uInt16 i=0; i < nMax && nRemainingFFn; ++i, ++p)
            {
                
                
                sal_uInt8 cbFfnM1 = *pRaw++;
                --nRemainingFFn;

                if (cbFfnM1 > nRemainingFFn)
                    break;

                if (cbFfnM1 < cbMinFFNPayload)
                    break;

                p->cbFfnM1 = cbFfnM1;

                sal_uInt8 *pVer8 = pRaw;

                sal_uInt8 c2 = *pVer8++;
                --cbFfnM1;

                p->prg = c2 & 0x02;
                p->fTrueType = (c2 & 0x04) >> 2;
                
                p->ff = (c2 & 0x70) >> 4;

                p->wWeight = SVBT16ToShort(*(SVBT16*)pVer8);
                pVer8+=2;
                cbFfnM1-=2;

                p->chs = *pVer8++;
                --cbFfnM1;

                p->ibszAlt = *pVer8++;
                --cbFfnM1;

                pVer8 += 10; 
                cbFfnM1-=10;
                pVer8 += 24; 
                cbFfnM1-=24;

                OSL_ASSERT(cbFfnM1 >= 2);

                sal_uInt8 nMaxNullTerminatedPossible = cbFfnM1/2 - 1;
                sal_Unicode *pPrimary = reinterpret_cast<sal_Unicode*>(pVer8);
                pPrimary[nMaxNullTerminatedPossible] = 0;
#ifdef OSL_BIGENDIAN
                swapEndian(pPrimary);
#endif
                p->sFontname = pPrimary;
                if (p->ibszAlt && p->ibszAlt < nMaxNullTerminatedPossible)
                {
                    sal_Unicode *pSecondary = pPrimary + p->ibszAlt;
#ifdef OSL_BIGENDIAN
                    swapEndian(pSecondary);
#endif
                    p->sFontname += ";" + OUString(pSecondary);
                }

                
                lcl_checkFontname( p->sFontname );

                
                pRaw += p->cbFfnM1;
                nRemainingFFn -= p->cbFfnM1;
                ++nValidFonts;
            }
            OSL_ENSURE(nMax == nValidFonts, "Font count differs with availability");
            nMax = std::min(nMax, nValidFonts);
        }
    }
    delete[] pA;
}

const WW8_FFN* WW8Fonts::GetFont( sal_uInt16 nNum ) const
{
    if( !pFontA || nNum >= nMax )
        return 0;

    return &pFontA[ nNum ];
}







//











WW8PLCF_HdFt::WW8PLCF_HdFt( SvStream* pSt, WW8Fib& rFib, WW8Dop& rDop )
    : aPLCF(*pSt, rFib.fcPlcfhdd , rFib.lcbPlcfhdd , 0)
{
    nIdxOffset = 0;

     /*
      cmc 23/02/2000: This dop.grpfIhdt has a bit set for each special
      footnote *and endnote!!* separator,continuation separator, and
      continuation notice entry, the documentation does not mention the
      endnote separators, the documentation also gets the index numbers
      backwards when specifiying which bits to test. The bottom six bits
      of this value must be tested and skipped over. Each section's
      grpfIhdt is then tested for the existence of the appropriate headers
      and footers, at the end of each section the nIdxOffset must be updated
      to point to the beginning of the next section's group of headers and
      footers in this PLCF, UpdateIndex does that task.
      */
    for( sal_uInt8 nI = 0x1; nI <= 0x20; nI <<= 1 )
        if( nI & rDop.grpfIhdt )                
            nIdxOffset++;

    nTextOfs = rFib.ccpText + rFib.ccpFtn;  
                                            
}

bool WW8PLCF_HdFt::GetTextPos(sal_uInt8 grpfIhdt, sal_uInt8 nWhich, WW8_CP& rStart,
    long& rLen)
{
    sal_uInt8 nI = 0x01;
    short nIdx = nIdxOffset;
    while (true)
    {
        if( nI & nWhich )
            break;                      
        if( grpfIhdt & nI )
            nIdx++;                     
        nI <<= 1;                       
        if( nI > 0x20 )
            return false;               
    }
                                        
    WW8_CP nEnd;
    void* pData;

    aPLCF.SetIdx( nIdx );               
    aPLCF.Get( rStart, nEnd, pData );
    rLen = nEnd - rStart;
    aPLCF.advance();

    return true;
}

bool WW8PLCF_HdFt::GetTextPosExact(short nIdx, WW8_CP& rStart, long& rLen)
{
    WW8_CP nEnd;
    void* pData;

    aPLCF.SetIdx( nIdx );               
    aPLCF.Get( rStart, nEnd, pData );
    rLen = nEnd - rStart;
    return true;
}

void WW8PLCF_HdFt::UpdateIndex( sal_uInt8 grpfIhdt )
{
    
    for( sal_uInt8 nI = 0x01; nI <= 0x20; nI <<= 1 )
        if( nI & grpfIhdt )
            nIdxOffset++;
}





WW8Dop::WW8Dop(SvStream& rSt, sal_Int16 nFib, sal_Int32 nPos, sal_uInt32 nSize) : bUseThaiLineBreakingRules(false)
{
    memset( &nDataStart, 0, (&nDataEnd - &nDataStart) );
    fDontUseHTMLAutoSpacing = true; 
    fAcetateShowAtn = true; 
    const sal_uInt32 nMaxDopSize = 0x268;
    sal_uInt8* pDataPtr = new sal_uInt8[ nMaxDopSize ];
    sal_uInt8* pData = pDataPtr;

    sal_uInt32 nRead = nMaxDopSize < nSize ? nMaxDopSize : nSize;
    rSt.Seek( nPos );
    if (2 > nSize || nRead != rSt.Read(pData, nRead))
        nDopError = ERR_SWG_READ_ERROR;     
    else
    {
        if (nMaxDopSize > nRead)
            memset( pData + nRead, 0, nMaxDopSize - nRead );

        
        sal_uInt32 a32Bit;
        sal_uInt16 a16Bit;
        sal_uInt8   a8Bit;

        a16Bit = Get_UShort( pData );        
        fFacingPages        = 0 != ( a16Bit  &  0x0001 )     ;
        fWidowControl       = 0 != ( a16Bit  &  0x0002 )     ;
        fPMHMainDoc         = 0 != ( a16Bit  &  0x0004 )     ;
        grfSuppression      =      ( a16Bit  &  0x0018 ) >> 3;
        fpc                 =      ( a16Bit  &  0x0060 ) >> 5;
        grpfIhdt            =      ( a16Bit  &  0xff00 ) >> 8;

        a16Bit = Get_UShort( pData );        
        rncFtn              =   a16Bit  &  0x0003        ;
        nFtn                = ( a16Bit  & ~0x0003 ) >> 2 ;

        a8Bit = Get_Byte( pData );           
        fOutlineDirtySave      = 0 != ( a8Bit  &  0x01   );

        a8Bit = Get_Byte( pData );           
        fOnlyMacPics           = 0 != ( a8Bit  &  0x01   );
        fOnlyWinPics           = 0 != ( a8Bit  &  0x02   );
        fLabelDoc              = 0 != ( a8Bit  &  0x04   );
        fHyphCapitals          = 0 != ( a8Bit  &  0x08   );
        fAutoHyphen            = 0 != ( a8Bit  &  0x10   );
        fFormNoFields          = 0 != ( a8Bit  &  0x20   );
        fLinkStyles            = 0 != ( a8Bit  &  0x40   );
        fRevMarking            = 0 != ( a8Bit  &  0x80   );

        a8Bit = Get_Byte( pData );           
        fBackup                = 0 != ( a8Bit  &  0x01   );
        fExactCWords           = 0 != ( a8Bit  &  0x02   );
        fPagHidden             = 0 != ( a8Bit  &  0x04   );
        fPagResults            = 0 != ( a8Bit  &  0x08   );
        fLockAtn               = 0 != ( a8Bit  &  0x10   );
        fMirrorMargins         = 0 != ( a8Bit  &  0x20   );
        fReadOnlyRecommended   = 0 != ( a8Bit  &  0x40   );
        fDfltTrueType          = 0 != ( a8Bit  &  0x80   );

        a8Bit = Get_Byte( pData );           
        fPagSuppressTopSpacing = 0 != ( a8Bit  &  0x01   );
        fProtEnabled           = 0 != ( a8Bit  &  0x02   );
        fDispFormFldSel        = 0 != ( a8Bit  &  0x04   );
        fRMView                = 0 != ( a8Bit  &  0x08   );
        fRMPrint               = 0 != ( a8Bit  &  0x10   );
        fWriteReservation      = 0 != ( a8Bit  &  0x20   );
        fLockRev               = 0 != ( a8Bit  &  0x40   );
        fEmbedFonts            = 0 != ( a8Bit  &  0x80   );


        a8Bit = Get_Byte( pData );           
        copts_fNoTabForInd           = 0 != ( a8Bit  &  0x01   );
        copts_fNoSpaceRaiseLower     = 0 != ( a8Bit  &  0x02   );
        copts_fSupressSpbfAfterPgBrk = 0 != ( a8Bit  &  0x04   );
        copts_fWrapTrailSpaces       = 0 != ( a8Bit  &  0x08   );
        copts_fMapPrintTextColor     = 0 != ( a8Bit  &  0x10   );
        copts_fNoColumnBalance       = 0 != ( a8Bit  &  0x20   );
        copts_fConvMailMergeEsc      = 0 != ( a8Bit  &  0x40   );
        copts_fSupressTopSpacing     = 0 != ( a8Bit  &  0x80   );

        a8Bit = Get_Byte( pData );           
        copts_fOrigWordTableRules    = 0 != ( a8Bit  &  0x01   );
        copts_fTransparentMetafiles  = 0 != ( a8Bit  &  0x02   );
        copts_fShowBreaksInFrames    = 0 != ( a8Bit  &  0x04   );
        copts_fSwapBordersFacingPgs  = 0 != ( a8Bit  &  0x08   );
        copts_fExpShRtn              = 0 != ( a8Bit  &  0x20   );  

        dxaTab = Get_Short( pData );         
        wSpare = Get_UShort( pData );        
        dxaHotZ = Get_UShort( pData );       
        cConsecHypLim = Get_UShort( pData ); 
        wSpare2 = Get_UShort( pData );       
        dttmCreated = Get_Long( pData );     
        dttmRevised = Get_Long( pData );     
        dttmLastPrint = Get_Long( pData );   
        nRevision = Get_Short( pData );      
        tmEdited = Get_Long( pData );        
        cWords = Get_Long( pData );          
        cCh = Get_Long( pData );             
        cPg = Get_Short( pData );            
        cParas = Get_Long( pData );          

        a16Bit = Get_UShort( pData );        
        rncEdn =   a16Bit &  0x0003       ;
        nEdn   = ( a16Bit & ~0x0003 ) >> 2;

        a16Bit = Get_UShort( pData );        
        epc            =   a16Bit &  0x0003       ;
        nfcFtnRef      = ( a16Bit &  0x003c ) >> 2;
        nfcEdnRef      = ( a16Bit &  0x03c0 ) >> 6;
        fPrintFormData = 0 != ( a16Bit &  0x0400 );
        fSaveFormData  = 0 != ( a16Bit &  0x0800 );
        fShadeFormData = 0 != ( a16Bit &  0x1000 );
        fWCFtnEdn      = 0 != ( a16Bit &  0x8000 );

        cLines = Get_Long( pData );          
        cWordsFtnEnd = Get_Long( pData );    
        cChFtnEdn = Get_Long( pData );       
        cPgFtnEdn = Get_Short( pData );      
        cParasFtnEdn = Get_Long( pData );    
        cLinesFtnEdn = Get_Long( pData );    
        lKeyProtDoc = Get_Long( pData );     

        a16Bit = Get_UShort( pData );        
        wvkSaved    =   a16Bit & 0x0007        ;
        wScaleSaved = ( a16Bit & 0x0ff8 ) >> 3 ;
        zkSaved     = ( a16Bit & 0x3000 ) >> 12;
        fRotateFontW6 = ( a16Bit & 0x4000 ) >> 14;
        iGutterPos = ( a16Bit &  0x8000 ) >> 15;
        /*
            bei nFib >= 103 gehts weiter:
        */
        if (nFib >= 103) 
        {
            a32Bit = Get_ULong( pData );     
            SetCompatabilityOptions(a32Bit);
        }

        
        if (nFib <= 104) 
            fUsePrinterMetrics = true;

        /*
            bei nFib > 105 gehts weiter:
        */
        if (nFib > 105) 
        {
            adt = Get_Short( pData );            

            doptypography.ReadFromMem(pData);    

            memcpy( &dogrid, pData, sizeof( WW8_DOGRID )); 
            pData += sizeof( WW8_DOGRID );

            a16Bit = Get_UShort( pData );        
            
            fHtmlDoc                = ( a16Bit &  0x0200 ) >>  9 ;
            fSnapBorder             = ( a16Bit &  0x0800 ) >> 11 ;
            fIncludeHeader          = ( a16Bit &  0x1000 ) >> 12 ;
            fIncludeFooter          = ( a16Bit &  0x2000 ) >> 13 ;
            fForcePageSizePag       = ( a16Bit &  0x4000 ) >> 14 ;
            fMinFontSizePag         = ( a16Bit &  0x8000 ) >> 15 ;

            a16Bit = Get_UShort( pData );        
            fHaveVersions   = 0 != ( a16Bit  &  0x0001 );
            fAutoVersion    = 0 != ( a16Bit  &  0x0002 );

            pData += 12;                         

            cChWS = Get_Long( pData );           
            cChWSFtnEdn = Get_Long( pData );     
            grfDocEvents = Get_Long( pData );    

            pData += 4+30+8;  

            cDBC = Get_Long( pData );            
            cDBCFtnEdn = Get_Long( pData );      

            pData += 1 * sizeof( sal_Int32);         

            nfcFtnRef = Get_Short( pData );      
            nfcEdnRef = Get_Short( pData );      
            hpsZoonFontPag = Get_Short( pData ); 
            dywDispPag = Get_Short( pData );     

            if (nRead >= 516)
            {
                
                pData += 8;                      
                a32Bit = Get_Long( pData );      
                SetCompatabilityOptions(a32Bit);
                a32Bit = Get_Long( pData );      

                
                SetCompatabilityOptions2(a32Bit);
            }
            if (nRead >= 550)
            {
                pData += 32;
                a16Bit = Get_UShort( pData );
                fDoNotEmbedSystemFont = ( a16Bit &  0x0001 );
                fWordCompat = ( a16Bit &  0x0002 ) >> 1;
                fLiveRecover = ( a16Bit &  0x0004 ) >> 2;
                fEmbedFactoids = ( a16Bit &  0x0008 ) >> 3;
                fFactoidXML = ( a16Bit &  0x00010 ) >> 4;
                fFactoidAllDone = ( a16Bit &  0x0020 ) >> 5;
                fFolioPrint = ( a16Bit &  0x0040 ) >> 6;
                fReverseFolio = ( a16Bit &  0x0080 ) >> 7;
                iTextLineEnding = ( a16Bit &  0x0700 ) >> 8;
                fHideFcc = ( a16Bit &  0x0800 ) >> 11;
                fAcetateShowMarkup = ( a16Bit &  0x1000 ) >> 12;
                fAcetateShowAtn = ( a16Bit &  0x2000 ) >> 13;
                fAcetateShowInsDel = ( a16Bit &  0x4000 ) >> 14;
                fAcetateShowProps = ( a16Bit &  0x8000 ) >> 15;
            }
            if (nRead >= 600)
            {
                pData += 48;
                a16Bit = Get_Short(pData);
                fUseBackGroundInAllmodes = (a16Bit & 0x0080) >> 7;
            }
        }
    }
    delete[] pDataPtr;
}

WW8Dop::WW8Dop() : bUseThaiLineBreakingRules(false)
{
    
    memset( &nDataStart, 0, (&nDataEnd - &nDataStart) );

    fWidowControl = true;
    fpc = 1;
    nFtn = 1;
    fOutlineDirtySave = true;
    fHyphCapitals = true;
    fBackup = true;
    fPagHidden = true;
    fPagResults = true;
    fDfltTrueType = true;

    /*
    Writer acts like this all the time at the moment, ideally we need an
    option for these two as well to import word docs that are not like
    this by default
    */
    fNoLeading = true;
    fUsePrinterMetrics = true;

    fRMView = true;
    fRMPrint = true;
    dxaTab = 0x2d0;
    dxaHotZ = 0x168;
    nRevision = 1;
    nEdn = 1;

    epc = 3;
    nfcEdnRef = 2;
    fShadeFormData = true;

    wvkSaved = 2;
    wScaleSaved = 100;
    zkSaved = 0;

    lvl = 9;
    fIncludeHeader = true;
    fIncludeFooter = true;

    cChWS = /**!!**/ 0;
    cChWSFtnEdn = /**!!**/ 0;

    cDBC = /**!!**/ 0;
    cDBCFtnEdn = /**!!**/ 0;

    fAcetateShowAtn = true;
}

void WW8Dop::SetCompatabilityOptions(sal_uInt32 a32Bit)
{
    fNoTabForInd                = ( a32Bit &  0x00000001 )       ;
    fNoSpaceRaiseLower          = ( a32Bit &  0x00000002 ) >>  1 ;
    fSupressSpbfAfterPageBreak  = ( a32Bit &  0x00000004 ) >>  2 ;
    fWrapTrailSpaces            = ( a32Bit &  0x00000008 ) >>  3 ;
    fMapPrintTextColor          = ( a32Bit &  0x00000010 ) >>  4 ;
    fNoColumnBalance            = ( a32Bit &  0x00000020 ) >>  5 ;
    fConvMailMergeEsc           = ( a32Bit &  0x00000040 ) >>  6 ;
    fSupressTopSpacing          = ( a32Bit &  0x00000080 ) >>  7 ;
    fOrigWordTableRules         = ( a32Bit &  0x00000100 ) >>  8 ;
    fTransparentMetafiles       = ( a32Bit &  0x00000200 ) >>  9 ;
    fShowBreaksInFrames         = ( a32Bit &  0x00000400 ) >> 10 ;
    fSwapBordersFacingPgs       = ( a32Bit &  0x00000800 ) >> 11 ;
    fCompatabilityOptions_Unknown1_13       = ( a32Bit &  0x00001000 ) >> 12 ;
    fExpShRtn                   = ( a32Bit &  0x00002000 ) >> 13 ; 
    fCompatabilityOptions_Unknown1_15       = ( a32Bit &  0x00004000 ) >> 14 ;
    fCompatabilityOptions_Unknown1_16       = ( a32Bit &  0x00008000 ) >> 15 ;
    fSuppressTopSpacingMac5     = ( a32Bit &  0x00010000 ) >> 16 ;
    fTruncDxaExpand             = ( a32Bit &  0x00020000 ) >> 17 ;
    fPrintBodyBeforeHdr         = ( a32Bit &  0x00040000 ) >> 18 ;
    fNoLeading                  = ( a32Bit &  0x00080000 ) >> 19 ;
    fCompatabilityOptions_Unknown1_21       = ( a32Bit &  0x00100000 ) >> 20 ;
    fMWSmallCaps                = ( a32Bit &  0x00200000 ) >> 21 ;
    fCompatabilityOptions_Unknown1_23       = ( a32Bit &  0x00400000 ) >> 22 ;
    fCompatabilityOptions_Unknown1_24       = ( a32Bit &  0x00800800 ) >> 23 ;
    fCompatabilityOptions_Unknown1_25       = ( a32Bit &  0x01000000 ) >> 24 ;
    fCompatabilityOptions_Unknown1_26       = ( a32Bit &  0x02000000 ) >> 25 ;
    fCompatabilityOptions_Unknown1_27       = ( a32Bit &  0x04000000 ) >> 26 ;
    fCompatabilityOptions_Unknown1_28       = ( a32Bit &  0x08000000 ) >> 27 ;
    fCompatabilityOptions_Unknown1_29       = ( a32Bit &  0x10000000 ) >> 28 ;
    fCompatabilityOptions_Unknown1_30       = ( a32Bit &  0x20000000 ) >> 29 ;
    fCompatabilityOptions_Unknown1_31       = ( a32Bit &  0x40000000 ) >> 30 ;

    fUsePrinterMetrics          = ( a32Bit &  0x80000000 ) >> 31 ;
}

sal_uInt32 WW8Dop::GetCompatabilityOptions() const
{
    sal_uInt32 a32Bit = 0;
    if (fNoTabForInd)                   a32Bit |= 0x00000001;
    if (fNoSpaceRaiseLower)             a32Bit |= 0x00000002;
    if (fSupressSpbfAfterPageBreak)     a32Bit |= 0x00000004;
    if (fWrapTrailSpaces)               a32Bit |= 0x00000008;
    if (fMapPrintTextColor)             a32Bit |= 0x00000010;
    if (fNoColumnBalance)               a32Bit |= 0x00000020;
    if (fConvMailMergeEsc)              a32Bit |= 0x00000040;
    if (fSupressTopSpacing)             a32Bit |= 0x00000080;
    if (fOrigWordTableRules)            a32Bit |= 0x00000100;
    if (fTransparentMetafiles)          a32Bit |= 0x00000200;
    if (fShowBreaksInFrames)            a32Bit |= 0x00000400;
    if (fSwapBordersFacingPgs)          a32Bit |= 0x00000800;
    if (fCompatabilityOptions_Unknown1_13)          a32Bit |= 0x00001000;
    if (fExpShRtn)                      a32Bit |= 0x00002000; 
    if (fCompatabilityOptions_Unknown1_15)          a32Bit |= 0x00004000;
    if (fCompatabilityOptions_Unknown1_16)          a32Bit |= 0x00008000;
    if (fSuppressTopSpacingMac5)        a32Bit |= 0x00010000;
    if (fTruncDxaExpand)                a32Bit |= 0x00020000;
    if (fPrintBodyBeforeHdr)            a32Bit |= 0x00040000;
    if (fNoLeading)                     a32Bit |= 0x00080000;
    if (fCompatabilityOptions_Unknown1_21)          a32Bit |= 0x00100000;
    if (fMWSmallCaps)                   a32Bit |= 0x00200000;
    if (fCompatabilityOptions_Unknown1_23)          a32Bit |= 0x00400000;
    if (fCompatabilityOptions_Unknown1_24)          a32Bit |= 0x00800000;
    if (fCompatabilityOptions_Unknown1_25)          a32Bit |= 0x01000000;
    if (fCompatabilityOptions_Unknown1_26)          a32Bit |= 0x02000000;
    if (fCompatabilityOptions_Unknown1_27)          a32Bit |= 0x04000000;
    if (fCompatabilityOptions_Unknown1_28)          a32Bit |= 0x08000000;
    if (fCompatabilityOptions_Unknown1_29)          a32Bit |= 0x10000000;
    if (fCompatabilityOptions_Unknown1_30)          a32Bit |= 0x20000000;
    if (fCompatabilityOptions_Unknown1_31)          a32Bit |= 0x40000000;
    if (fUsePrinterMetrics)             a32Bit |= 0x80000000;
    return a32Bit;
}


void WW8Dop::SetCompatabilityOptions2(sal_uInt32 a32Bit)
{
    fCompatabilityOptions_Unknown2_1                        = ( a32Bit &  0x00000001 );
    fCompatabilityOptions_Unknown2_2                        = ( a32Bit &  0x00000002 ) >>  1 ;
    fDontUseHTMLAutoSpacing     = ( a32Bit &  0x00000004 ) >>  2 ;
    fCompatabilityOptions_Unknown2_4                    = ( a32Bit &  0x00000008 ) >>  3 ;
       fCompatabilityOptions_Unknown2_5                 = ( a32Bit &  0x00000010 ) >>  4 ;
       fCompatabilityOptions_Unknown2_6                 = ( a32Bit &  0x00000020 ) >>  5 ;
       fCompatabilityOptions_Unknown2_7                 = ( a32Bit &  0x00000040 ) >>  6 ;
       fCompatabilityOptions_Unknown2_8                 = ( a32Bit &  0x00000080 ) >>  7 ;
       fCompatabilityOptions_Unknown2_9                 = ( a32Bit &  0x00000100 ) >>  8 ;
       fCompatabilityOptions_Unknown2_10                    = ( a32Bit &  0x00000200 ) >>  9 ;
       fCompatabilityOptions_Unknown2_11                    = ( a32Bit &  0x00000400 ) >> 10 ;
       fCompatabilityOptions_Unknown2_12                    = ( a32Bit &  0x00000800 ) >> 11 ;
    fCompatabilityOptions_Unknown2_13                   = ( a32Bit &  0x00001000 ) >> 12 ;
    fCompatabilityOptions_Unknown2_14                   = ( a32Bit &  0x00002000 ) >> 13 ;
    fCompatabilityOptions_Unknown2_15                   = ( a32Bit &  0x00004000 ) >> 14 ;
    fCompatabilityOptions_Unknown2_16                   = ( a32Bit &  0x00008000 ) >> 15 ;
       fCompatabilityOptions_Unknown2_17                    = ( a32Bit &  0x00010000 ) >> 16 ;
       fCompatabilityOptions_Unknown2_18                    = ( a32Bit &  0x00020000 ) >> 17 ;
       fCompatabilityOptions_Unknown2_19                    = ( a32Bit &  0x00040000 ) >> 18 ;
       fCompatabilityOptions_Unknown2_20                    = ( a32Bit &  0x00080000 ) >> 19 ;
    fCompatabilityOptions_Unknown2_21                   = ( a32Bit &  0x00100000 ) >> 20 ;
       fCompatabilityOptions_Unknown2_22                    = ( a32Bit &  0x00200000 ) >> 21 ;
    fCompatabilityOptions_Unknown2_23                   = ( a32Bit &  0x00400000 ) >> 22 ;
    fCompatabilityOptions_Unknown2_24                   = ( a32Bit &  0x00800800 ) >> 23 ;
    fCompatabilityOptions_Unknown2_25                   = ( a32Bit &  0x01000800 ) >> 24 ;
    fCompatabilityOptions_Unknown2_26                   = ( a32Bit &  0x02000800 ) >> 25 ;
    fCompatabilityOptions_Unknown2_27                   = ( a32Bit &  0x04000800 ) >> 26 ;
    fCompatabilityOptions_Unknown2_28                   = ( a32Bit &  0x08000800 ) >> 27 ;
    fCompatabilityOptions_Unknown2_29                   = ( a32Bit &  0x10000800 ) >> 28 ;
    fCompatabilityOptions_Unknown2_30                   = ( a32Bit &  0x20000800 ) >> 29 ;
    fCompatabilityOptions_Unknown2_31                   = ( a32Bit &  0x40000800 ) >> 30 ;
       fCompatabilityOptions_Unknown2_32                    = ( a32Bit &  0x80000000 ) >> 31 ;
}

sal_uInt32 WW8Dop::GetCompatabilityOptions2() const
{
    sal_uInt32 a32Bit = 0;
    if (fCompatabilityOptions_Unknown2_1)           a32Bit |= 0x00000001;
    if (fCompatabilityOptions_Unknown2_2)           a32Bit |= 0x00000002;
    if (fDontUseHTMLAutoSpacing)     a32Bit |= 0x00000004;
    if (fCompatabilityOptions_Unknown2_4)           a32Bit |= 0x00000008;
    if (fCompatabilityOptions_Unknown2_5)           a32Bit |= 0x00000010;
    if (fCompatabilityOptions_Unknown2_6)           a32Bit |= 0x00000020;
    if (fCompatabilityOptions_Unknown2_7)           a32Bit |= 0x00000040;
    if (fCompatabilityOptions_Unknown2_8)           a32Bit |= 0x00000080;
    if (fCompatabilityOptions_Unknown2_9)           a32Bit |= 0x00000100;
    if (fCompatabilityOptions_Unknown2_10)          a32Bit |= 0x00000200;
    if (fCompatabilityOptions_Unknown2_11)          a32Bit |= 0x00000400;
    if (fCompatabilityOptions_Unknown2_12)          a32Bit |= 0x00000800;
    if (fCompatabilityOptions_Unknown2_13)          a32Bit |= 0x00001000;
    
    
    
    if (bUseThaiLineBreakingRules)          a32Bit |= 0x00002000;
    else if (fCompatabilityOptions_Unknown2_14)         a32Bit |= 0x00002000;
    if (fCompatabilityOptions_Unknown2_15)          a32Bit |= 0x00004000;
    if (fCompatabilityOptions_Unknown2_16)          a32Bit |= 0x00008000;
    if (fCompatabilityOptions_Unknown2_17)          a32Bit |= 0x00010000;
    if (fCompatabilityOptions_Unknown2_18)          a32Bit |= 0x00020000;
    if (fCompatabilityOptions_Unknown2_19)          a32Bit |= 0x00040000;
    if (fCompatabilityOptions_Unknown2_20)          a32Bit |= 0x00080000;
    if (fCompatabilityOptions_Unknown2_21)          a32Bit |= 0x00100000;
    if (fCompatabilityOptions_Unknown2_22)          a32Bit |= 0x00200000;
    if (fCompatabilityOptions_Unknown2_23)          a32Bit |= 0x00400000;
    if (fCompatabilityOptions_Unknown2_24)          a32Bit |= 0x00800000;
    if (fCompatabilityOptions_Unknown2_25)          a32Bit |= 0x01000000;
    if (fCompatabilityOptions_Unknown2_26)          a32Bit |= 0x02000000;
    if (fCompatabilityOptions_Unknown2_27)          a32Bit |= 0x04000000;
    if (fCompatabilityOptions_Unknown2_28)          a32Bit |= 0x08000000;
    if (fCompatabilityOptions_Unknown2_29)          a32Bit |= 0x10000000;
    if (fCompatabilityOptions_Unknown2_30)          a32Bit |= 0x20000000;
    if (fCompatabilityOptions_Unknown2_31)          a32Bit |= 0x40000000;
    if (fCompatabilityOptions_Unknown2_32)          a32Bit |= 0x80000000;
    return a32Bit;
}

bool WW8Dop::Write(SvStream& rStrm, WW8Fib& rFib) const
{
    const int nMaxDopLen = 610;
    sal_uInt32 nLen = 8 == rFib.nVersion ? nMaxDopLen : 84;
    rFib.fcDop =  rStrm.Tell();
    rFib.lcbDop = nLen;

    sal_uInt8 aData[ nMaxDopLen ];
    memset( aData, 0, nMaxDopLen );
    sal_uInt8* pData = aData;

    
    sal_uInt16 a16Bit;
    sal_uInt8   a8Bit;

    a16Bit = 0;                         
    if (fFacingPages)
        a16Bit |= 0x0001;
    if (fWidowControl)
        a16Bit |= 0x0002;
    if (fPMHMainDoc)
        a16Bit |= 0x0004;
    a16Bit |= ( 0x0018 & (grfSuppression << 3));
    a16Bit |= ( 0x0060 & (fpc << 5));
    a16Bit |= ( 0xff00 & (grpfIhdt << 8));
    Set_UInt16( pData, a16Bit );

    a16Bit = 0;                         
    a16Bit |= ( 0x0003 & rncFtn );
    a16Bit |= ( ~0x0003 & (nFtn << 2));
    Set_UInt16( pData, a16Bit );

    a8Bit = 0;                          
    if( fOutlineDirtySave ) a8Bit |= 0x01;
    Set_UInt8( pData, a8Bit );

    a8Bit = 0;                          
    if( fOnlyMacPics )  a8Bit |= 0x01;
    if( fOnlyWinPics )  a8Bit |= 0x02;
    if( fLabelDoc )     a8Bit |= 0x04;
    if( fHyphCapitals ) a8Bit |= 0x08;
    if( fAutoHyphen )   a8Bit |= 0x10;
    if( fFormNoFields ) a8Bit |= 0x20;
    if( fLinkStyles )   a8Bit |= 0x40;
    if( fRevMarking )   a8Bit |= 0x80;
    Set_UInt8( pData, a8Bit );

    a8Bit = 0;                          
    if( fBackup )               a8Bit |= 0x01;
    if( fExactCWords )          a8Bit |= 0x02;
    if( fPagHidden )            a8Bit |= 0x04;
    if( fPagResults )           a8Bit |= 0x08;
    if( fLockAtn )              a8Bit |= 0x10;
    if( fMirrorMargins )        a8Bit |= 0x20;
    if( fReadOnlyRecommended )  a8Bit |= 0x40;
    if( fDfltTrueType )         a8Bit |= 0x80;
    Set_UInt8( pData, a8Bit );

    a8Bit = 0;                          
    if( fPagSuppressTopSpacing )    a8Bit |= 0x01;
    if( fProtEnabled )              a8Bit |= 0x02;
    if( fDispFormFldSel )           a8Bit |= 0x04;
    if( fRMView )                   a8Bit |= 0x08;
    if( fRMPrint )                  a8Bit |= 0x10;
    if( fWriteReservation )         a8Bit |= 0x20;
    if( fLockRev )                  a8Bit |= 0x40;
    if( fEmbedFonts )               a8Bit |= 0x80;
    Set_UInt8( pData, a8Bit );


    a8Bit = 0;                          
    if( copts_fNoTabForInd )            a8Bit |= 0x01;
    if( copts_fNoSpaceRaiseLower )      a8Bit |= 0x02;
    if( copts_fSupressSpbfAfterPgBrk )  a8Bit |= 0x04;
    if( copts_fWrapTrailSpaces )        a8Bit |= 0x08;
    if( copts_fMapPrintTextColor )      a8Bit |= 0x10;
    if( copts_fNoColumnBalance )        a8Bit |= 0x20;
    if( copts_fConvMailMergeEsc )       a8Bit |= 0x40;
    if( copts_fSupressTopSpacing )      a8Bit |= 0x80;
    Set_UInt8( pData, a8Bit );

    a8Bit = 0;                          
    if( copts_fOrigWordTableRules )     a8Bit |= 0x01;
    if( copts_fTransparentMetafiles )   a8Bit |= 0x02;
    if( copts_fShowBreaksInFrames )     a8Bit |= 0x04;
    if( copts_fSwapBordersFacingPgs )   a8Bit |= 0x08;
    if( copts_fExpShRtn )               a8Bit |= 0x20;  
    Set_UInt8( pData, a8Bit );

    Set_UInt16( pData, dxaTab );        
    Set_UInt16( pData, wSpare );        
    Set_UInt16( pData, dxaHotZ );       
    Set_UInt16( pData, cConsecHypLim ); 
    Set_UInt16( pData, wSpare2 );       
    Set_UInt32( pData, dttmCreated );   
    Set_UInt32( pData, dttmRevised );   
    Set_UInt32( pData, dttmLastPrint ); 
    Set_UInt16( pData, nRevision );     
    Set_UInt32( pData, tmEdited );      
    Set_UInt32( pData, cWords );        
    Set_UInt32( pData, cCh );           
    Set_UInt16( pData, cPg );           
    Set_UInt32( pData, cParas );        

    a16Bit = 0;                         
    a16Bit |= ( 0x0003 & rncEdn );
    a16Bit |= (~0x0003 & ( nEdn << 2));
    Set_UInt16( pData, a16Bit );

    a16Bit = 0;                         
    a16Bit |= (0x0003 & epc );
    a16Bit |= (0x003c & (nfcFtnRef << 2));
    a16Bit |= (0x03c0 & (nfcEdnRef << 6));
    if( fPrintFormData )    a16Bit |= 0x0400;
    if( fSaveFormData )     a16Bit |= 0x0800;
    if( fShadeFormData )    a16Bit |= 0x1000;
    if( fWCFtnEdn )         a16Bit |= 0x8000;
    Set_UInt16( pData, a16Bit );

    Set_UInt32( pData, cLines );        
    Set_UInt32( pData, cWordsFtnEnd );  
    Set_UInt32( pData, cChFtnEdn );     
    Set_UInt16( pData, cPgFtnEdn );     
    Set_UInt32( pData, cParasFtnEdn );  
    Set_UInt32( pData, cLinesFtnEdn );  
    Set_UInt32( pData, lKeyProtDoc );   

    a16Bit = 0;                         
    if (wvkSaved)
        a16Bit |= 0x0007;
    a16Bit |= (0x0ff8 & (wScaleSaved << 3));
    a16Bit |= (0x3000 & (zkSaved << 12));
    Set_UInt16( pData, a16Bit );

    if( 8 == rFib.nVersion )
    {
        Set_UInt32(pData, GetCompatabilityOptions());  

        Set_UInt16( pData, adt );                      

        doptypography.WriteToMem(pData);               

        memcpy( pData, &dogrid, sizeof( WW8_DOGRID ));
        pData += sizeof( WW8_DOGRID );

        a16Bit = 0x12;      
        if( fHtmlDoc )          a16Bit |= 0x0200;
        if( fSnapBorder )       a16Bit |= 0x0800;
        if( fIncludeHeader )    a16Bit |= 0x1000;
        if( fIncludeFooter )    a16Bit |= 0x2000;
        if( fForcePageSizePag ) a16Bit |= 0x4000;
        if( fMinFontSizePag )   a16Bit |= 0x8000;
        Set_UInt16( pData, a16Bit );

        a16Bit = 0;                                    
        if( fHaveVersions ) a16Bit |= 0x0001;
        if( fAutoVersion )  a16Bit |= 0x0002;
        Set_UInt16( pData, a16Bit );

        pData += 12;                                   

        Set_UInt32( pData, cChWS );                    
        Set_UInt32( pData, cChWSFtnEdn );              
        Set_UInt32( pData, grfDocEvents );             

        pData += 4+30+8;  

        Set_UInt32( pData, cDBC );                     
        Set_UInt32( pData, cDBCFtnEdn );               

        pData += 1 * sizeof( sal_Int32);                   

        Set_UInt16( pData, nfcFtnRef );                
        Set_UInt16( pData, nfcEdnRef );                
        Set_UInt16( pData, hpsZoonFontPag );           
        Set_UInt16( pData, dywDispPag );               

        
        pData += 8;
        Set_UInt32(pData, GetCompatabilityOptions());
        Set_UInt32(pData, GetCompatabilityOptions2());
        pData += 32;

        a16Bit = 0;
        if (fAcetateShowMarkup)
            a16Bit |= 0x1000;
        
        if (fAcetateShowAtn)
        {
            a16Bit |= 0x1000;
            a16Bit |= 0x2000;
        }
        Set_UInt16(pData, a16Bit);

        pData += 48;
        a16Bit = 0x0080;
        Set_UInt16(pData, a16Bit);
    }
    rStrm.Write( aData, nLen );
    return 0 == rStrm.GetError();
}

void WW8DopTypography::ReadFromMem(sal_uInt8 *&pData)
{
    sal_uInt16 a16Bit = Get_UShort(pData);
    fKerningPunct = (a16Bit & 0x0001);
    iJustification = (a16Bit & 0x0006) >>  1;
    iLevelOfKinsoku = (a16Bit & 0x0018) >>  3;
    f2on1 = (a16Bit & 0x0020) >>  5;
    reserved1 = (a16Bit & 0x03C0) >>  6;
    reserved2 = (a16Bit & 0xFC00) >>  10;

    cchFollowingPunct = Get_Short(pData);
    cchLeadingPunct = Get_Short(pData);

    sal_Int16 i;
    for (i=0; i < nMaxFollowing; ++i)
        rgxchFPunct[i] = Get_Short(pData);
    for (i=0; i < nMaxLeading; ++i)
        rgxchLPunct[i] = Get_Short(pData);

    if (cchFollowingPunct >= 0 && cchFollowingPunct < nMaxFollowing)
        rgxchFPunct[cchFollowingPunct]=0;
    else
        rgxchFPunct[nMaxFollowing - 1]=0;

    if (cchLeadingPunct >= 0 && cchLeadingPunct < nMaxLeading)
        rgxchLPunct[cchLeadingPunct]=0;
    else
        rgxchLPunct[nMaxLeading - 1]=0;

}

void WW8DopTypography::WriteToMem(sal_uInt8 *&pData) const
{
    sal_uInt16 a16Bit = sal_uInt16(fKerningPunct);
    a16Bit |= (iJustification << 1) & 0x0006;
    a16Bit |= (iLevelOfKinsoku << 3) & 0x0018;
    a16Bit |= (int(f2on1) << 5) & 0x0020;
    a16Bit |= (reserved1 << 6) & 0x03C0;
    a16Bit |= (reserved2 << 10) & 0xFC00;
    Set_UInt16(pData,a16Bit);

    Set_UInt16(pData,cchFollowingPunct);
    Set_UInt16(pData,cchLeadingPunct);

    sal_Int16 i;
    for (i=0; i < nMaxFollowing; ++i)
        Set_UInt16(pData,rgxchFPunct[i]);
    for (i=0; i < nMaxLeading; ++i)
        Set_UInt16(pData,rgxchLPunct[i]);
}

sal_uInt16 WW8DopTypography::GetConvertedLang() const
{
    sal_uInt16 nLang;
    

    
    

    /*
    One example of 3 for reserved1 which was really Japanese, perhaps last bit
    is for some other use ?, or redundant. If more examples trigger the assert
    we might be able to figure it out.
    */
    switch(reserved1 & 0xE)
    {
        case 2:     
            nLang = LANGUAGE_JAPANESE;
            break;
        case 4:     
            nLang = LANGUAGE_CHINESE_SIMPLIFIED;
            break;
        case 6:     
            nLang = LANGUAGE_KOREAN;
            break;
        case 8:     
            nLang = LANGUAGE_CHINESE_TRADITIONAL;
            break;
        default:
            OSL_ENSURE(!this, "Unknown MS Asian Typography language, report");
            nLang = LANGUAGE_CHINESE_SIMPLIFIED_LEGACY;
            break;
        case 0:
            
            
            
            nLang = LANGUAGE_JAPANESE;
            break;
    }
    return nLang;
}




sal_uInt16 wwSprmParser::GetSprmTailLen(sal_uInt16 nId, const sal_uInt8* pSprm)
    const
{
    SprmInfo aSprm = GetSprmInfo(nId);
    sal_uInt16 nL = 0;                      

    
    switch( nId )
    {
        case 23:
        case 0xC615:
            if( pSprm[1 + mnDelta] != 255 )
                nL = static_cast< sal_uInt16 >(pSprm[1 + mnDelta] + aSprm.nLen);
            else
            {
                sal_uInt8 nDel = pSprm[2 + mnDelta];
                sal_uInt8 nIns = pSprm[3 + mnDelta + 4 * nDel];

                nL = 2 + 4 * nDel + 3 * nIns;
            }
            break;
        case 0xD608:
            nL = SVBT16ToShort( &pSprm[1 + mnDelta] );
            break;
        default:
            switch (aSprm.nVari)
            {
                case L_FIX:
                    nL = aSprm.nLen;        
                    break;
                case L_VAR:
                    
                    
                    nL = static_cast< sal_uInt16 >(pSprm[1 + mnDelta] + aSprm.nLen);
                    break;
                case L_VAR2:
                    
                    
                    nL = static_cast< sal_uInt16 >(SVBT16ToShort( &pSprm[1 + mnDelta] ) + aSprm.nLen - 1);
                    break;
                default:
                    OSL_ENSURE(!this, "Unknown sprm varient");
                    break;
            }
            break;
    }
    return nL;
}


sal_uInt16 wwSprmParser::GetSprmId(const sal_uInt8* pSp) const
{
    ASSERT_RET_ON_FAIL(pSp, "Why GetSprmId with pSp of 0", 0);

    sal_uInt16 nId = 0;

    if (ww::IsSevenMinus(meVersion))
    {
        nId = *pSp;
        if (0x0100 < nId)
            nId = 0;
    }
    else
    {
        nId = SVBT16ToShort(pSp);
        if (0x0800 > nId)
            nId = 0;
    }

    return nId;
}


sal_uInt16 wwSprmParser::GetSprmSize(sal_uInt16 nId, const sal_uInt8* pSprm) const
{
    return GetSprmTailLen(nId, pSprm) + 1 + mnDelta + SprmDataOfs(nId);
}

sal_uInt8 wwSprmParser::SprmDataOfs(sal_uInt16 nId) const
{
    return GetSprmInfo(nId).nVari;
}

sal_uInt16 wwSprmParser::DistanceToData(sal_uInt16 nId) const
{
    return 1 + mnDelta + SprmDataOfs(nId);
}

sal_uInt8* wwSprmParser::findSprmData(sal_uInt16 nId, sal_uInt8* pSprms,
    sal_uInt16 nLen) const
{
    while (nLen >= MinSprmLen())
    {
        sal_uInt16 nAktId = GetSprmId(pSprms);
        
        sal_uInt16 nSize = GetSprmSize(nAktId, pSprms);

        bool bValid = nSize <= nLen;

        SAL_WARN_IF(!bValid, "sw.ww8",
            "sprm 0x" << std::hex << nAktId << std::dec << " longer than remaining bytes, " <<
            nSize << " vs " << nLen << "doc or parser is wrong");

        if (nAktId == nId && bValid) 
            return pSprms + DistanceToData(nId);

        
        nSize = std::min(nSize, nLen);
        pSprms += nSize;
        nLen -= nSize;
    }
    
    return 0;
}

SEPr::SEPr() :
    bkc(2), fTitlePage(0), fAutoPgn(0), nfcPgn(0), fUnlocked(0), cnsPgn(0),
    fPgnRestart(0), fEndNote(1), lnc(0), grpfIhdt(0), nLnnMod(0), dxaLnn(0),
    dxaPgn(720), dyaPgn(720), fLBetween(0), vjc(0), dmBinFirst(0),
    dmBinOther(0), dmPaperReq(0), fPropRMark(0), ibstPropRMark(0),
    dttmPropRMark(0), dxtCharSpace(0), dyaLinePitch(0), clm(0), reserved1(0),
    dmOrientPage(0), iHeadingPgn(0), pgnStart(1), lnnMin(0), wTextFlow(0),
    reserved2(0), pgbApplyTo(0), pgbPageDepth(0), pgbOffsetFrom(0),
    xaPage(lLetterWidth), yaPage(lLetterHeight), xaPageNUp(lLetterWidth), yaPageNUp(lLetterHeight),
    dxaLeft(1800), dxaRight(1800), dyaTop(1440), dyaBottom(1440), dzaGutter(0),
    dyaHdrTop(720), dyaHdrBottom(720), ccolM1(0), fEvenlySpaced(1),
    reserved3(0), fBiDi(0), fFacingCol(0), fRTLGutter(0), fRTLAlignment(0),
    dxaColumns(720), dxaColumnWidth(0), dmOrientFirst(0), fLayout(0),
    reserved4(0)
{
    memset(rgdxaColumnWidthSpacing, 0, sizeof(rgdxaColumnWidthSpacing));
}

bool checkSeek(SvStream &rSt, sal_uInt32 nOffset)
{
    return (rSt.Seek(nOffset) == static_cast<sal_Size>(nOffset));
}

bool checkRead(SvStream &rSt, void *pDest, sal_uInt32 nLength)
{
    return (rSt.Read(pDest, nLength) == static_cast<sal_Size>(nLength));
}

#ifdef OSL_BIGENDIAN
void swapEndian(sal_Unicode *pString)
{
    for (sal_Unicode *pWork = pString; *pWork; ++pWork)
        *pWork = OSL_SWAPWORD(*pWork);
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
