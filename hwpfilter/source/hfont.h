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
