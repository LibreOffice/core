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
constexpr size_t THEME_APPLICATION_COLORS_COUNT = ColorConfigEntryCount - WINDOWCOLOR;

namespace {
    struct ColorConfigEntryData_Impl
    {
        std::u16string_view cName;
        bool            bCanBeVisible;
        bool            bCanHaveBitmap;
    };
    const ColorConfigEntryData_Impl cNames[] =
    {
        { std::u16string_view(u"DocColor"), false, false },
        { std::u16string_view(u"DocBoundaries"), false, false },
        { std::u16string_view(u"AppBackground"), false, true },
        { std::u16string_view(u"TableBoundaries"), false, false },
        { std::u16string_view(u"FontColor"), false, false },
        { std::u16string_view(u"Links"), true, false },
        { std::u16string_view(u"LinksVisited"), true, false },
        { std::u16string_view(u"Spell"), false, false },
        { std::u16string_view(u"Grammar"), false, false },
        { std::u16string_view(u"SmartTags"), false, false },
        { std::u16string_view(u"Shadow"), true, false },
        { std::u16string_view(u"WriterTextGrid"), false, false },
        { std::u16string_view(u"WriterFieldShadings"), true, false },
        { std::u16string_view(u"WriterIdxShadings"), true, false },
        { std::u16string_view(u"WriterDirectCursor"), true, false },
        { std::u16string_view(u"WriterScriptIndicator"), false, false },
        { std::u16string_view(u"WriterSectionBoundaries"), false, false },
        { std::u16string_view(u"WriterHeaderFooterMark"), false, false },
        { std::u16string_view(u"WriterPageBreaks"), false, false },
        { std::u16string_view(u"WriterNonPrintChars"), false, false },
        { std::u16string_view(u"HTMLSGML"), false, false },
        { std::u16string_view(u"HTMLComment"), false, false },
        { std::u16string_view(u"HTMLKeyword"), false, false },
        { std::u16string_view(u"HTMLUnknown"), false, false },
        { std::u16string_view(u"CalcGrid"), false, false },
        { std::u16string_view(u"CalcCellFocus"), false, false },
        { std::u16string_view(u"CalcPageBreak"), false, false },
        { std::u16string_view(u"CalcPageBreakManual"), false, false },
        { std::u16string_view(u"CalcPageBreakAutomatic"), false, false },
        { std::u16string_view(u"CalcHiddenColRow"), true, false },
        { std::u16string_view(u"CalcTextOverflow"), true, false },
        { std::u16string_view(u"CalcComments"), false, false },
        { std::u16string_view(u"CalcDetective"), false, false },
        { std::u16string_view(u"CalcDetectiveError"), false, false },
        { std::u16string_view(u"CalcReference"), false, false },
        { std::u16string_view(u"CalcNotesBackground"), false, false },
        { std::u16string_view(u"CalcValue"), false, false },
        { std::u16string_view(u"CalcFormula"), false, false },
        { std::u16string_view(u"CalcText"), false, false },
        { std::u16string_view(u"CalcProtectedBackground"), false, false },
        { std::u16string_view(u"DrawGrid"), true, false },
        { std::u16string_view(u"Author1"), false, false },
        { std::u16string_view(u"Author2"), false, false },
        { std::u16string_view(u"Author3"), false, false },
        { std::u16string_view(u"Author4"), false, false },
        { std::u16string_view(u"Author5"), false, false },
        { std::u16string_view(u"Author6"), false, false },
        { std::u16string_view(u"Author7"), false, false },
        { std::u16string_view(u"Author8"), false, false },
        { std::u16string_view(u"Author9"), false, false },
        { std::u16string_view(u"BASICEditor"), false, false },
        { std::u16string_view(u"BASICIdentifier"), false, false },
        { std::u16string_view(u"BASICComment"), false, false },
        { std::u16string_view(u"BASICNumber"), false, false },
        { std::u16string_view(u"BASICString"), false, false },
        { std::u16string_view(u"BASICOperator"), false, false },
        { std::u16string_view(u"BASICKeyword"), false, false },
        { std::u16string_view(u"BASICError"), false, false },
        { std::u16string_view(u"SQLIdentifier"), false, false },
        { std::u16string_view(u"SQLNumber"), false, false },
        { std::u16string_view(u"SQLString"), false, false },
        { std::u16string_view(u"SQLOperator"), false, false },
        { std::u16string_view(u"SQLKeyword"), false, false },
        { std::u16string_view(u"SQLParameter"), false, false },
        { std::u16string_view(u"SQLComment"), false, false },

        { std::u16string_view(u"WindowColor"), false, false },
        { std::u16string_view(u"WindowTextColor"), false, false },
        { std::u16string_view(u"BaseColor"), false, false },
        { std::u16string_view(u"ButtonColor"), false, false },
        { std::u16string_view(u"ButtonTextColor"), false, false },
        { std::u16string_view(u"AccentColor"), false, false },
        { std::u16string_view(u"DisabledColor"), false, false },
        { std::u16string_view(u"DisabledTextColor"), false, false },
        { std::u16string_view(u"ShadowColor"), false, false },
        { std::u16string_view(u"SeparatorColor"), false, false },
        { std::u16string_view(u"FaceColor"), false, false },
        { std::u16string_view(u"ActiveColor"), false, false },
        { std::u16string_view(u"ActiveTextColor"), false, false },
        { std::u16string_view(u"ActiveBorderColor"), false, false },
        { std::u16string_view(u"FieldColor"), false, false },
        { std::u16string_view(u"MenuBarColor"), false, false },
        { std::u16string_view(u"MenuBarTextColor"), false, false },
        { std::u16string_view(u"MenuBarHighlightColor"), false, false },
        { std::u16string_view(u"MenuBarHighlightTextColor"), false, false },
        { std::u16string_view(u"MenuColor"), false, false },
        { std::u16string_view(u"MenuTextColor"), false, false },
        { std::u16string_view(u"MenuHighlightColor"), false, false },
        { std::u16string_view(u"MenuHighlightTextColor"), false, false },
        { std::u16string_view(u"MenuBorderColor"), false, false },
        { std::u16string_view(u"InactiveColor"), false, false },
        { std::u16string_view(u"InactiveTextColor"), false, false },
        { std::u16string_view(u"InactiveBorderColor"), false, false }
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
