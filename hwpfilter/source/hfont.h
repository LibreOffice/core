/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: hfont.h,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:34:07 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

/* $Id: hfont.h,v 1.2 2005-09-07 16:34:07 rt Exp $ */

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
 * @param lang Language index
 * @returns Count of the font list for given language
 */
        int       NFonts(int lang);
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
