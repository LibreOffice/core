/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fontcfg.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2005-11-16 09:51:48 $
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
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef INCLUDED_SWDLLAPI_H
#include "swdllapi.h"
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
#define FONT_STANDARD_CTL   10
#define FONT_OUTLINE_CTL    11
#define FONT_LIST_CTL       12
#define FONT_CAPTION_CTL    13
#define FONT_INDEX_CTL      14
#define DEF_FONT_COUNT      15

#define FONT_PER_GROUP      5

#define FONT_GROUP_DEFAULT  0
#define FONT_GROUP_CJK      1
#define FONT_GROUP_CTL      2

//pt-size of fonts
#define FONTSIZE_DEFAULT            240
#define FONTSIZE_OUTLINE            280


class SW_DLLPUBLIC SwStdFontConfig : public utl::ConfigItem
{
    String      sDefaultFonts[DEF_FONT_COUNT];
    sal_Int32   nDefaultFontHeight[DEF_FONT_COUNT];

    SW_DLLPRIVATE com::sun::star::uno::Sequence<rtl::OUString>    GetPropertyNames();

    void ChangeString(USHORT nFontType, const String& rSet)
        {
            if(sDefaultFonts[nFontType] != rSet)
            {
                SetModified();
                sDefaultFonts[nFontType] = rSet;
            }
        }

    void ChangeInt( USHORT nFontType, sal_Int32 nHeight );

public:
    SwStdFontConfig();
    ~SwStdFontConfig();

    virtual void    Commit();

    const String&   GetFontStandard(sal_uInt8 nFontGroup) const {return sDefaultFonts[FONT_STANDARD + FONT_PER_GROUP * nFontGroup];}
    const String&   GetFontOutline(sal_uInt8 nFontGroup)  const {return sDefaultFonts[FONT_OUTLINE + FONT_PER_GROUP * nFontGroup];}
    const String&   GetFontList   (sal_uInt8 nFontGroup)  const {return sDefaultFonts[FONT_LIST + FONT_PER_GROUP * nFontGroup];}
    const String&   GetFontCaption(sal_uInt8 nFontGroup)  const {return sDefaultFonts[FONT_CAPTION + FONT_PER_GROUP * nFontGroup];}
    const String&   GetFontIndex  (sal_uInt8 nFontGroup)  const {return sDefaultFonts[FONT_INDEX + FONT_PER_GROUP * nFontGroup];}

    const String&   GetFontFor(USHORT nFontType)  const {return sDefaultFonts[nFontType];}
    BOOL            IsFontDefault(USHORT nFontType) const;

    void     SetFontStandard(const String& rSet, sal_uInt8 nFontGroup)
                    {ChangeString(FONT_STANDARD + FONT_PER_GROUP * nFontGroup, rSet);}

    void     SetFontOutline(const String& rSet, sal_uInt8 nFontGroup)
                    {    ChangeString(FONT_OUTLINE + FONT_PER_GROUP * nFontGroup, rSet);}
    void     SetFontList   (const String& rSet, sal_uInt8 nFontGroup)
                    {    ChangeString(FONT_LIST + FONT_PER_GROUP * nFontGroup, rSet);}
    void     SetFontCaption(const String& rSet, sal_uInt8 nFontGroup)
                    {    ChangeString(FONT_CAPTION + FONT_PER_GROUP * nFontGroup, rSet);}
    void     SetFontIndex  (const String& rSet, sal_uInt8 nFontGroup)
                    {    ChangeString(FONT_INDEX + FONT_PER_GROUP * nFontGroup, rSet);}

    void     SetFontHeight( sal_Int32 nHeight, sal_uInt8 nFont, sal_uInt8 nScriptType )
                    {    ChangeInt(nFont + FONT_PER_GROUP * nScriptType, nHeight);}

    sal_Int32 GetFontHeight( sal_uInt8 nFont, sal_uInt8 nScriptType, LanguageType eLang );

    static String    GetDefaultFor(USHORT nFontType, LanguageType eLang);
    static sal_Int32 GetDefaultHeightFor(USHORT nFontType, LanguageType eLang);
};
#endif


