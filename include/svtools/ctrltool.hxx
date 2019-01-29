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

#ifndef INCLUDED_SVTOOLS_CTRLTOOL_HXX
#define INCLUDED_SVTOOLS_CTRLTOOL_HXX

#include <svtools/svtdllapi.h>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <vcl/metric.hxx>
#include <vcl/vclptr.hxx>

#include <vector>
#include <memory>


class ImplFontListNameInfo;
class OutputDevice;

/*

Description
============

class FontList

This class manages all fonts which can be display on one or two output devices.
Additionally, this class offers methods for generating the StyleName from
bold and italics or the missing attributes from a StyleName.
Furthermore, this class can handle synthetically reproduced fonts.
It also works with several standard controls and standard menus.

Links:

class FontNameBox, class FontStyleBox, class FontSizeBox,
class FontNameMenu, class FontSizeMenu

--------------------------------------------------------------------------

FontList::FontList( OutputDevice* pDevice, OutputDevice* pDevice2 = NULL,
                    bool bAll = true );

Constructor of the FontList class. The relevant fonts will be queried from
the OutputDevice. The OutputDevice needs to exist as long as the FontList
class exists. Optionally, a second output device can be given in order to,
e.g., manage the fonts from both, a printer and a screen in a single FontList
and thus also give FontMenus the fonts if both OutputDevices.
The pDevice2 needs to exist as long as the FontList class exists.

The OutputDevice given first should be the preferred one. This is usually
the printer. Because if two different device fonts (one for the printer and
one for the screen) exist, the ones from the "pDevice" are preferred.

The third parameter governs whether only scalable or all fonts shall be queried.
With sal_True bitmap fonts will also be queried.
With sal_False vectorized and scalable fonts will be queried.

--------------------------------------------------------------------------

String FontList::GetStyleName( const FontMetric& rFontMetric ) const;

This method returns the StyleName of a FontMetric.
If no StyleName is set, a name will be generated from the set attributes.

--------------------------------------------------------------------------

OUString FontList::GetFontMapText( const FontMetric& rFontMetric ) const;

This method returns a Matchstring which indicates the problem that could
arise when using a font. This string should be displayed to the user.

--------------------------------------------------------------------------

FontMetric FontList::Get( const String& rName, const String& rStyleName ) const;

This method search a FontMetric for the given name and the given style name.
The Stylename can also be a synthetic one.
In that case the relevant FontMetric fields will be set.
If a StyleName is provided, a FontMetric structure without a Stylename can be
returned. To get a representation of the StyleName for displaying it to the user,
call GetStyleName() on this FontMetric structure.

Links:

FontList::GetStyleName()

--------------------------------------------------------------------------

FontMetric FontList::Get( const String& rName, FontWeight eWeight,
                        FontItalic eItalic ) const;

This method search a FontMetric structure for a provided name and styles.
This method can also return a FontMetric without a Stylename.
To get a representation of the StyleName to be presented to the user
call GetStyleName() with this FontMetric.

Links:

FontList::GetStyleName()

--------------------------------------------------------------------------

const sal_IntPtr* FontList::GetSizeAry( const FontMetric& rFontMetric ) const;

This method returns the available sizes for the given font.
If it is a scalable font, standard sizes are returned.
The array contains the heights of the font in tenth (1/10) point.
The last value of the array is 0.
The returned array will destroyed by the FontList.
You should thus not reference the array after the next method call on the
FontList.
*/

class SVT_DLLPUBLIC FontList
{
private:
    static const sal_IntPtr aStdSizeAry[];

    OUString                maMapBoth;
    OUString                maMapPrinterOnly;
    OUString                maMapStyleNotAvailable;
    mutable OUString        maMapNotAvailable;
    OUString                maLight;
    OUString                maLightItalic;
    OUString                maNormal;
    OUString                maNormalItalic;
    OUString                maBold;
    OUString                maBoldItalic;
    OUString                maBlack;
    OUString                maBlackItalic;
    mutable std::unique_ptr<sal_IntPtr[]>
                            mpSizeAry;
    VclPtr<OutputDevice>    mpDev;
    VclPtr<OutputDevice>    mpDev2;
    std::vector<std::unique_ptr<ImplFontListNameInfo>> m_Entries;

    SVT_DLLPRIVATE ImplFontListNameInfo*    ImplFind( const OUString& rSearchName, sal_uInt32* pIndex ) const;
    SVT_DLLPRIVATE ImplFontListNameInfo*    ImplFindByName( const OUString& rStr ) const;
    SVT_DLLPRIVATE void                     ImplInsertFonts(OutputDevice* pDev, bool bInsertData);

public:
                            FontList( OutputDevice* pDevice,
                                      OutputDevice* pDevice2 = nullptr);
                            ~FontList();

    std::unique_ptr<FontList> Clone() const;

    OUString                GetFontMapText( const FontMetric& rFontMetric ) const;

    const OUString&         GetNormalStr() const { return maNormal; }
    const OUString&         GetItalicStr() const { return maNormalItalic; }
    const OUString&         GetBoldStr() const { return maBold; }
    const OUString&         GetBoldItalicStr() const { return maBoldItalic; }
    const OUString&         GetStyleName( FontWeight eWeight, FontItalic eItalic ) const;
    OUString                GetStyleName( const FontMetric& rFontMetric ) const;

    FontMetric           Get( const OUString& rName,
                                 const OUString& rStyleName ) const;
    FontMetric           Get( const OUString& rName,
                                 FontWeight eWeight,
                                 FontItalic eItalic ) const;

    bool                    IsAvailable( const OUString& rName ) const;
    size_t GetFontNameCount() const
    {
        return m_Entries.size();
    }
    const FontMetric& GetFontName(size_t nFont) const;
    sal_Handle              GetFirstFontMetric( const OUString& rName ) const;
    static sal_Handle           GetNextFontMetric( sal_Handle hFontMetric );
    static const FontMetric& GetFontMetric( sal_Handle hFontMetric );

    const sal_IntPtr*       GetSizeAry( const FontMetric& rFontMetric ) const;
    static const sal_IntPtr* GetStdSizeAry() { return aStdSizeAry; }

private:
                            FontList( const FontList& ) = delete;
    FontList&               operator =( const FontList& ) = delete;
};

class SVT_DLLPUBLIC FontSizeNames
{
private:
    const struct ImplFSNameItem*    mpArray;
    sal_Int32                       mnElem;

public:
                            FontSizeNames( LanguageType eLanguage /* = LANGUAGE_DONTKNOW */ );

    sal_Int32               Count() const { return mnElem; }
    bool                    IsEmpty() const { return !mnElem; }

    sal_Int32               Name2Size( const OUString& ) const;
    OUString                Size2Name( sal_Int32 ) const;

    OUString                GetIndexName( sal_Int32 nIndex ) const;
    sal_Int32               GetIndexSize( sal_Int32 nIndex ) const;
};

#endif // INCLUDED_SVTOOLS_CTRLTOOL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
