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
#pragma once

#include <svtools/svtdllapi.h>
#include <rtl/ustring.hxx>
#include <tools/color.hxx>
#include <com/sun/star/uno/Sequence.h>
#include <unotools/options.hxx>
#include <memory>

#include <tools/link.hxx>
#include <vcl/vclevent.hxx>

namespace svtools{
enum ColorConfigEntry : int
{
    DOCCOLOR            ,
    DOCBOUNDARIES       ,
    APPBACKGROUND       ,
    TABLEBOUNDARIES     ,
    FONTCOLOR           ,
    LINKS               ,
    LINKSVISITED        ,
    SPELL         ,
    GRAMMAR       ,
    SMARTTAGS     ,
    SHADOWCOLOR         ,
    WRITERTEXTGRID      ,
    WRITERFIELDSHADINGS ,
    WRITERIDXSHADINGS         ,
    WRITERDIRECTCURSOR        , // unused
    WRITERSCRIPTINDICATOR,
    WRITERSECTIONBOUNDARIES,
    WRITERHEADERFOOTERMARK,
    WRITERPAGEBREAKS,
    WRITERNONPRINTCHARS,
    HTMLSGML            ,
    HTMLCOMMENT         ,
    HTMLKEYWORD         ,
    HTMLUNKNOWN         ,
    CALCGRID            ,
    CALCCELLFOCUS       ,
    CALCPAGEBREAK       ,
    CALCPAGEBREAKMANUAL,
    CALCPAGEBREAKAUTOMATIC,
    CALCHIDDENROWCOL    ,
    CALCTEXTOVERFLOW    ,
    CALCCOMMENTS        ,
    CALCDETECTIVE       ,
    CALCDETECTIVEERROR       ,
    CALCREFERENCE       ,
    CALCNOTESBACKGROUND     ,
    CALCVALUE,
    CALCFORMULA,
    CALCTEXT,
    CALCPROTECTEDBACKGROUND,
    DRAWGRID            ,
    AUTHOR1,
    AUTHOR2,
    AUTHOR3,
    AUTHOR4,
    AUTHOR5,
    AUTHOR6,
    AUTHOR7,
    AUTHOR8,
    AUTHOR9,
    BASICEDITOR,
    BASICIDENTIFIER,
    BASICCOMMENT   ,
    BASICNUMBER    ,
    BASICSTRING    ,
    BASICOPERATOR  ,
    BASICKEYWORD   ,
    BASICERROR   ,
    SQLIDENTIFIER,
    SQLNUMBER,
    SQLSTRING,
    SQLOPERATOR,
    SQLKEYWORD,
    SQLPARAMETER,
    SQLCOMMENT,

    // colors for Libreoffice Theme (Application)
    WINDOWCOLOR         ,
    WINDOWTEXTCOLOR     ,
    BASECOLOR           ,
    BUTTONCOLOR         ,
    BUTTONTEXTCOLOR     ,
    ACCENTCOLOR         ,
    DISABLEDCOLOR       ,
    DISABLEDTEXTCOLOR   ,
    SHADECOLOR         ,
    SEPARATORCOLOR      ,
    FACECOLOR           ,
    ACTIVECOLOR         ,
    ACTIVETEXTCOLOR     ,
    ACTIVEBORDERCOLOR   ,
    FIELDCOLOR          ,
    MENUBARCOLOR        ,
    MENUBARTEXTCOLOR    ,
    MENUBARHIGHLIGHTCOLOR ,
    MENUBARHIGHLIGHTTEXTCOLOR ,
    MENUCOLOR           ,
    MENUTEXTCOLOR       ,
    MENUHIGHLIGHTCOLOR  ,
    MENUHIGHLIGHTTEXTCOLOR ,
    MENUBORDERCOLOR     ,
    INACTIVECOLOR       ,
    INACTIVETEXTCOLOR   ,
    INACTIVEBORDERCOLOR ,

    ColorConfigEntryCount
};

constexpr OUString AUTOMATIC_COLOR_SCHEME = u"COLOR_SCHEME_LIBREOFFICE_AUTOMATIC"_ustr;
constexpr OUString LIGHT_COLOR_SCHEME = u"COLOR_SCHEME_LIBREOFFICE_LIGHT"_ustr;
constexpr OUString DARK_COLOR_SCHEME = u"COLOR_SCHEME_LIBREOFFICE_DARK"_ustr;
constexpr size_t THEME_APPLICATION_COLORS_COUNT = ColorConfigEntryCount - WINDOWCOLOR;

namespace {
    struct ColorConfigEntryData_Impl
    {
        std::u16string_view cName;
        bool            bCanBeVisible;
    };
    const ColorConfigEntryData_Impl cNames[] =
    {
        { std::u16string_view(u"DocColor"), false },
        { std::u16string_view(u"DocBoundaries"), false },
        { std::u16string_view(u"AppBackground"), false },
        { std::u16string_view(u"TableBoundaries"), false },
        { std::u16string_view(u"FontColor"), false },
        { std::u16string_view(u"Links"), true },
        { std::u16string_view(u"LinksVisited"), true },
        { std::u16string_view(u"Spell"), false },
        { std::u16string_view(u"Grammar"), false },
        { std::u16string_view(u"SmartTags"), false },
        { std::u16string_view(u"Shadow"), true },
        { std::u16string_view(u"WriterTextGrid"), false },
        { std::u16string_view(u"WriterFieldShadings"), true },
        { std::u16string_view(u"WriterIdxShadings"), true },
        { std::u16string_view(u"WriterDirectCursor"), true },
        { std::u16string_view(u"WriterScriptIndicator"), false },
        { std::u16string_view(u"WriterSectionBoundaries"), false },
        { std::u16string_view(u"WriterHeaderFooterMark"), false },
        { std::u16string_view(u"WriterPageBreaks"), false },
        { std::u16string_view(u"WriterNonPrintChars"), false },
        { std::u16string_view(u"HTMLSGML"), false },
        { std::u16string_view(u"HTMLComment"), false },
        { std::u16string_view(u"HTMLKeyword"), false },
        { std::u16string_view(u"HTMLUnknown"), false },
        { std::u16string_view(u"CalcGrid"), false },
        { std::u16string_view(u"CalcCellFocus"), false },
        { std::u16string_view(u"CalcPageBreak"), false },
        { std::u16string_view(u"CalcPageBreakManual"), false },
        { std::u16string_view(u"CalcPageBreakAutomatic"), false },
        { std::u16string_view(u"CalcHiddenColRow"), true },
        { std::u16string_view(u"CalcTextOverflow"), true },
        { std::u16string_view(u"CalcComments"), false },
        { std::u16string_view(u"CalcDetective"), false },
        { std::u16string_view(u"CalcDetectiveError"), false },
        { std::u16string_view(u"CalcReference"), false },
        { std::u16string_view(u"CalcNotesBackground"), false },
        { std::u16string_view(u"CalcValue"), false },
        { std::u16string_view(u"CalcFormula"), false },
        { std::u16string_view(u"CalcText"), false },
        { std::u16string_view(u"CalcProtectedBackground"), false },
        { std::u16string_view(u"DrawGrid"), true },
        { std::u16string_view(u"Author1"), false },
        { std::u16string_view(u"Author2"), false },
        { std::u16string_view(u"Author3"), false },
        { std::u16string_view(u"Author4"), false },
        { std::u16string_view(u"Author5"), false },
        { std::u16string_view(u"Author6"), false },
        { std::u16string_view(u"Author7"), false },
        { std::u16string_view(u"Author8"), false },
        { std::u16string_view(u"Author9"), false },
        { std::u16string_view(u"BASICEditor"), false },
        { std::u16string_view(u"BASICIdentifier"), false },
        { std::u16string_view(u"BASICComment"), false },
        { std::u16string_view(u"BASICNumber"), false },
        { std::u16string_view(u"BASICString"), false },
        { std::u16string_view(u"BASICOperator"), false },
        { std::u16string_view(u"BASICKeyword"), false },
        { std::u16string_view(u"BASICError"), false },
        { std::u16string_view(u"SQLIdentifier"), false },
        { std::u16string_view(u"SQLNumber"), false },
        { std::u16string_view(u"SQLString"), false },
        { std::u16string_view(u"SQLOperator"), false },
        { std::u16string_view(u"SQLKeyword"), false },
        { std::u16string_view(u"SQLParameter"), false },
        { std::u16string_view(u"SQLComment"), false },

        { std::u16string_view(u"WindowColor"), false },
        { std::u16string_view(u"WindowTextColor"), false },
        { std::u16string_view(u"BaseColor"), false },
        { std::u16string_view(u"ButtonColor"), false },
        { std::u16string_view(u"ButtonTextColor"), false },
        { std::u16string_view(u"AccentColor"), false },
        { std::u16string_view(u"DisabledColor"), false },
        { std::u16string_view(u"DisabledTextColor"), false },
        { std::u16string_view(u"ShadowColor"), false },
        { std::u16string_view(u"SeparatorColor"), false },
        { std::u16string_view(u"FaceColor"), false },
        { std::u16string_view(u"ActiveColor"), false },
        { std::u16string_view(u"ActiveTextColor"), false },
        { std::u16string_view(u"ActiveBorderColor"), false },
        { std::u16string_view(u"FieldColor"), false },
        { std::u16string_view(u"MenuBarColor"), false },
        { std::u16string_view(u"MenuBarTextColor"), false },
        { std::u16string_view(u"MenuBarHighlightColor"), false },
        { std::u16string_view(u"MenuBarHighlightTextColor"), false },
        { std::u16string_view(u"MenuColor"), false },
        { std::u16string_view(u"MenuTextColor"), false },
        { std::u16string_view(u"MenuHighlightColor"), false },
        { std::u16string_view(u"MenuHighlightTextColor"), false },
        { std::u16string_view(u"MenuBorderColor"), false },
        { std::u16string_view(u"InactiveColor"), false },
        { std::u16string_view(u"InactiveTextColor"), false },
        { std::u16string_view(u"InactiveBorderColor"), false }
    };
}

class ColorConfig_Impl;
struct ColorConfigValue
{

    bool        bIsVisible; // validity depends on the element type
    ::Color     nColor; // used as a cache for the current color
    Color       nLightColor;
    Color       nDarkColor;

    bool        bUseBitmapBackground;
    bool        bIsBitmapStretched;
    OUString    sBitmapFileName;

    ColorConfigValue()
        : bIsVisible(false)
        , nColor(0)
        , nLightColor(0)
        , nDarkColor(0)
        , bUseBitmapBackground(false)
        , bIsBitmapStretched(false)
    {
    }

    bool operator!=(const ColorConfigValue& rCmp) const
    {
        return
            nColor != rCmp.nColor
            || nLightColor != rCmp.nLightColor
            || nDarkColor != rCmp.nDarkColor
            || bIsVisible != rCmp.bIsVisible
            || bUseBitmapBackground != rCmp.bUseBitmapBackground
            || bIsBitmapStretched != rCmp.bIsBitmapStretched
            || sBitmapFileName != rCmp.sBitmapFileName;
    }
};

class SAL_WARN_UNUSED SVT_DLLPUBLIC ColorConfig final :
    public utl::detail::Options
{
    friend class ColorConfig_Impl;
private:
    static ColorConfig_Impl* m_pImpl;
public:
    ColorConfig();
    virtual ~ColorConfig() override;

    // get the configured value - if bSmart is set the default color setting is provided
    // instead of the automatic color
    ColorConfigValue        GetColorValue(ColorConfigEntry eEntry, bool bSmart = true) const;
    // -1 gets the default color on current mod.
    //  0 gets the default color on light mod.
    //  1 gets the default color on dark mod.
    static Color            GetDefaultColor(ColorConfigEntry eEntry, int nMod = -1);
    static const OUString& GetCurrentSchemeName();
};

class SVT_DLLPUBLIC EditableColorConfig
{
    std::unique_ptr<ColorConfig_Impl> m_pImpl;
    bool                m_bModified;
public:
    EditableColorConfig();
    ~EditableColorConfig();

    css::uno::Sequence< OUString >  GetSchemeNames() const;
    void                        DeleteScheme(const OUString& rScheme );
    void                        AddScheme(const OUString& rScheme );
    void                        LoadScheme(const OUString& rScheme );
    const OUString&             GetCurrentSchemeName() const;
    void                        SetCurrentSchemeName(const OUString& rScheme);

    const ColorConfigValue&     GetColorValue(ColorConfigEntry eEntry) const;
    void                        SetColorValue(ColorConfigEntry eEntry, const ColorConfigValue& rValue);
    void                        SetModified();
    void                        ClearModified() {m_bModified = false;}
    bool                        IsModified() const {return m_bModified;}
    void                        Commit();

    void                        DisableBroadcast();
    void                        EnableBroadcast();
};
}//namespace svtools

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
