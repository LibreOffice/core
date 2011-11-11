/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _SALLANG_HXX
#include <os2/sallang.hxx>
#endif

// =======================================================================

// -----------------------------------------------------------------------
// English (US/UK/AUS/CAN/NZ/EIRE/SAFRICA/JAMAICA/CARRIBEAN)
static const wchar_t* aImplLangEnglishTab[LSTR_COUNT] =
{
    L"Shift",                                                // LSTR_KEY_SHIFT
    L"Ctrl",                                                 // LSTR_KEY_CTRL
    L"Alt",                                                  // LSTR_KEY_ALT
    L"Up",                                                   // LSTR_KEY_UP
    L"Down",                                                 // LSTR_KEY_DOWN
    L"Left",                                                 // LSTR_KEY_LEFT
    L"Right",                                                // LSTR_KEY_RIGHT
    L"Home",                                                 // LSTR_KEY_HOME
    L"End",                                                  // LSTR_KEY_END
    L"PageUp",                                               // LSTR_KEY_PAGEUP
    L"PageDown",                                             // LSTR_KEY_PAGEDOWN
    L"Enter",                                                // LSTR_KEY_RETURN
    L"Esc",                                                  // LSTR_KEY_ESC
    L"Tab",                                                  // LSTR_KEY_TAB
    L"Backspace",                                            // LSTR_KEY_BACKSPACE
    L"Space",                                                // LSTR_KEY_SPACE
    L"Insert",                                               // LSTR_KEY_INSERT
    L"Del",                                                  // LSTR_KEY_DELETE
};

// =======================================================================

const sal_Unicode** ImplGetLangTab( LanguageType eLang )
{
    // Sprachtabelle ermitteln
    const wchar_t** pLangTab;
    //switch ( International::GetNeutralLanguage( eLang ) )
    switch ( eLang )
    {
#if 0
        case LANGUAGE_DANISH:
            pLangTab = aImplLangDanishTab;
            break;

        case LANGUAGE_DUTCH:
        case LANGUAGE_DUTCH_BELGIAN:
            pLangTab = aImplLangDutchTab;
            break;

        case LANGUAGE_FINNISH:
            pLangTab = aImplLangFinnishTab;
            break;

        case LANGUAGE_FRENCH:
            pLangTab = aImplLangFrenchTab;
            break;

        case LANGUAGE_GERMAN:
            pLangTab = aImplLangGermanTab;
            break;

        case LANGUAGE_ITALIAN:
            pLangTab = aImplLangItalianTab;
            break;

        case LANGUAGE_NORWEGIAN:
        case LANGUAGE_NORWEGIAN_BOKMAL:
            pLangTab = aImplLangNorwegianTab;
            break;

        case LANGUAGE_PORTUGUESE:
        case LANGUAGE_PORTUGUESE_BRAZILIAN:
            pLangTab = aImplLangPortugueseTab;
            break;

        case LANGUAGE_SPANISH:
            pLangTab = aImplLangSpanishTab;
            break;

        case LANGUAGE_SWEDISH:
            pLangTab = aImplLangSwedishTab;
            break;
#endif
        default:
            pLangTab = aImplLangEnglishTab;
            break;
    }

    return (const sal_Unicode**)pLangTab;
}
