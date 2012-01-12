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



/* $Id: hfont.h,v 1.4 2008-06-04 09:57:04 vg Exp $ */

#ifndef _HWPFONTS_H_
#define _HWPFONTS_H_

#include <stdlib.h>
#include <string.h>

#define MAXFONTS    256
#define FONTNAMELEN 40

class HWPFile;

/**
 * The HWPFont class has the font list for the document when it's saved.
 * @short Font information
 */
class DLLEXPORT HWPFont
{
    protected:
/**
 * System font count for each language
 * NLanguage is 7 in common case.
 */
        int   nFonts[NLanguage];
/**
 * list of the font family name
 */
        char  *fontnames[NLanguage];

    public:
        HWPFont(void);
        ~HWPFont(void);

/**
 * Adds a font into font list of a language
 * @param lang Language index
 * @param font Name of font family
 */
        int       AddFont( int lang, const char *font );
/**
 * @param lang Language index
 * @param id Index of font
 * @returns Font name
 */
        const char    *GetFontName( int lang, int id );

        bool Read( HWPFile &hwpf );
};
#endif                                            /* _HWPFONTS+H_ */
