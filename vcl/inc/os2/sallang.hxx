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
#define _SALLANG_HXX

//#ifndef _TOOLS_LANG_HXX
//#include <tools/lang.hxx>
//#endif
#include <i18npool/mslangid.hxx>

// --------------------
// - Language Strings -
// --------------------

// --- Key-Namen ---
#define LSTR_KEY_SHIFT                       0
#define LSTR_KEY_CTRL                        1
#define LSTR_KEY_ALT                         2
#define LSTR_KEY_UP                          3
#define LSTR_KEY_DOWN                        4
#define LSTR_KEY_LEFT                        5
#define LSTR_KEY_RIGHT                       6
#define LSTR_KEY_HOME                        7
#define LSTR_KEY_END                         8
#define LSTR_KEY_PAGEUP                      9
#define LSTR_KEY_PAGEDOWN                   10
#define LSTR_KEY_RETURN                     11
#define LSTR_KEY_ESC                        12
#define LSTR_KEY_TAB                        13
#define LSTR_KEY_BACKSPACE                  14
#define LSTR_KEY_SPACE                      15
#define LSTR_KEY_INSERT                     16
#define LSTR_KEY_DELETE                     17

// --- Anzahl der Texte ---

#define LSTR_COUNT                          18

// --------------------------------------------
// - Methoden zum Abfragen der Sprach-Strings -
// --------------------------------------------

const sal_Unicode** ImplGetLangTab( LanguageType eLang );

#endif // _SALLANG_HXX
