/*************************************************************************
 *
 *  $RCSfile: fontcfg.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: os $ $Date: 2001-08-15 09:50:39 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _FONTCFG_HXX
#define _FONTCFG_HXX

#ifndef _UTL_CONFIGITEM_HXX_
#include <unotools/configitem.hxx>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _LANG_HXX
#include <tools/lang.hxx>
#endif

#define FONT_STANDARD       0
#define FONT_OUTLINE        1
#define FONT_LIST           2
#define FONT_CAPTION        3
#define FONT_INDEX          4
#define FONT_STANDARD_CJK   5
#define FONT_OUTLINE_CJK    6
#define FONT_LIST_CJK       7
#define FONT_CAPTION_CJK    8
#define FONT_INDEX_CJK      9
#define DEF_FONT_COUNT      10


class SwStdFontConfig : public utl::ConfigItem
{
    String      sDefaultFonts[DEF_FONT_COUNT];

    com::sun::star::uno::Sequence<rtl::OUString>    GetPropertyNames();

    void ChangeString(USHORT nFontType, const String& rSet)
        {
            if(sDefaultFonts[nFontType] != rSet)
            {
                SetModified();
                sDefaultFonts[nFontType] = rSet;
            }
        }
public:
    SwStdFontConfig();
    ~SwStdFontConfig();

    virtual void    Commit();

    const String&   GetFontStandard(sal_Bool bCJK) const {return sDefaultFonts[bCJK ? FONT_STANDARD_CJK : FONT_STANDARD];}
    const String&   GetFontOutline(sal_Bool bCJK)  const {return sDefaultFonts[bCJK ? FONT_OUTLINE_CJK : FONT_OUTLINE];}
    const String&   GetFontList   (sal_Bool bCJK)  const {return sDefaultFonts[bCJK ? FONT_LIST_CJK : FONT_LIST];}
    const String&   GetFontCaption(sal_Bool bCJK)  const {return sDefaultFonts[bCJK ? FONT_CAPTION_CJK : FONT_CAPTION];}
    const String&   GetFontIndex  (sal_Bool bCJK)  const {return sDefaultFonts[bCJK ? FONT_INDEX_CJK : FONT_INDEX];}

    const String&   GetFontFor(USHORT nFontType)  const {return sDefaultFonts[nFontType];}
    BOOL            IsFontDefault(USHORT nFontType) const;

    void     SetFontStandard(const String& rSet, sal_Bool bCJK)
                    {ChangeString(bCJK ? FONT_STANDARD_CJK : FONT_STANDARD, rSet);}

    void     SetFontOutline(const String& rSet, sal_Bool bCJK)
                    {    ChangeString(bCJK ? FONT_OUTLINE_CJK : FONT_OUTLINE, rSet);}
    void     SetFontList   (const String& rSet, sal_Bool bCJK)
                    {    ChangeString(bCJK ? FONT_LIST_CJK : FONT_LIST, rSet);}
    void     SetFontCaption(const String& rSet, sal_Bool bCJK)
                    {    ChangeString(bCJK ? FONT_CAPTION_CJK : FONT_CAPTION, rSet);}
    void     SetFontIndex  (const String& rSet, sal_Bool bCJK)
                    {    ChangeString(bCJK ? FONT_INDEX_CJK : FONT_INDEX, rSet);}

    static String   GetDefaultFor(USHORT nFontType, LanguageType eLang);
};
#endif


