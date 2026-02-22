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

/*
There are two different Autoformat classes for Writer and Calc
(SwTableAutoFormatTable & ScAutoFormat). This SvxAutoFormat unifies the
common functionality of SwTableAutoFormatTable and ScAutoFormat
*/

#include <comphelper/processfactory.hxx>
#include <sfx2/docfile.hxx>
#include <unotools/streamwrap.hxx>
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <config_folders.h>
#include <editeng/memberids.h>
#include <rtl/bootstrap.hxx>
#include <svl/intitem.hxx>
#include <svx/rotmodit.hxx>
#include <svx/svxtableitems.hxx>
#include <com/sun/star/xml/sax/Writer.hpp>
#include <editeng/adjustitem.hxx>
#include <svx/TableAutoFmt.hxx>
#include <editeng/justifyitem.hxx>
#include <editeng/svxenum.hxx>
#include <svl/typedwhich.hxx>
#include <svx/TableStylesParser.hxx>

using namespace com::sun::star::beans;
using namespace com::sun::star;

const SfxItemPropertySet& SvxAutoFormat::GetTablePropertySet()
{
    static const SfxItemPropertyMapEntry aSvxTablePropertyMap[] =

        {
            // SvxBrushItem
            { u"BackColor"_ustr, SVX_TABLE_BACKGROUND, cppu::UnoType<sal_Int32>::get(),
              PROPERTY_NONE, MID_BACK_COLOR },
            // SvxBoxItem
            { u"Border"_ustr, SVX_TABLE_BOX, cppu::UnoType<css::table::BorderLine>::get(),
              PROPERTY_NONE, LEFT_BORDER | CONVERT_TWIPS },
            { u"LeftBorder"_ustr, SVX_TABLE_BOX, cppu::UnoType<css::table::BorderLine>::get(),
              PROPERTY_NONE, LEFT_BORDER | CONVERT_TWIPS },
            { u"RightBorder"_ustr, SVX_TABLE_BOX, cppu::UnoType<css::table::BorderLine>::get(),
              PROPERTY_NONE, RIGHT_BORDER | CONVERT_TWIPS },
            { u"TopBorder"_ustr, SVX_TABLE_BOX, cppu::UnoType<css::table::BorderLine>::get(),
              PROPERTY_NONE, TOP_BORDER | CONVERT_TWIPS },
            { u"BottomBorder"_ustr, SVX_TABLE_BOX, cppu::UnoType<css::table::BorderLine>::get(),
              PROPERTY_NONE, BOTTOM_BORDER | CONVERT_TWIPS },
            { u"BorderDistance"_ustr, SVX_TABLE_BOX, cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE,
              BORDER_DISTANCE | CONVERT_TWIPS },
            { u"LeftBorderDistance"_ustr, SVX_TABLE_BOX, cppu::UnoType<sal_Int32>::get(),
              PROPERTY_NONE, LEFT_BORDER_DISTANCE | CONVERT_TWIPS },
            { u"RightBorderDistance"_ustr, SVX_TABLE_BOX, cppu::UnoType<sal_Int32>::get(),
              PROPERTY_NONE, RIGHT_BORDER_DISTANCE | CONVERT_TWIPS },
            { u"TopBorderDistance"_ustr, SVX_TABLE_BOX, cppu::UnoType<sal_Int32>::get(),
              PROPERTY_NONE, TOP_BORDER_DISTANCE | CONVERT_TWIPS },
            { u"BottomBorderDistance"_ustr, SVX_TABLE_BOX, cppu::UnoType<sal_Int32>::get(),
              PROPERTY_NONE, BOTTOM_BORDER_DISTANCE | CONVERT_TWIPS },
            // SvxFormatOrient
            { u"VertOrient"_ustr, SVX_TABLE_VER_JUSTIFY, cppu::UnoType<sal_Int16>::get(),
              PROPERTY_NONE, 0 },
            // SvxColorItem
            { u"CharColor"_ustr, SVX_TABLE_FONT_COLOR, cppu::UnoType<sal_Int32>::get(),
              PROPERTY_NONE, 0 },
            { u"CharUnderline"_ustr, SVX_TABLE_FONT_UNDERLINE, cppu::UnoType<sal_Int32>::get(),
              PROPERTY_NONE, MID_TL_STYLE },
            //Font
            { u"CharHeight"_ustr, SVX_TABLE_FONT_HEIGHT, cppu::UnoType<float>::get(),
              PropertyAttribute::MAYBEVOID, MID_FONTHEIGHT | CONVERT_TWIPS },
            { u"CharWeight"_ustr, SVX_TABLE_FONT_WEIGHT, cppu::UnoType<float>::get(),
              PropertyAttribute::MAYBEVOID, MID_WEIGHT },
            { u"CharPosture"_ustr, SVX_TABLE_FONT_POSTURE,
              cppu::UnoType<css::awt::FontSlant>::get(), PropertyAttribute::MAYBEVOID,
              MID_POSTURE },
            { u"CharFontName"_ustr, SVX_TABLE_FONT, cppu::UnoType<OUString>::get(),
              PropertyAttribute::MAYBEVOID, MID_FONT_FAMILY_NAME },
            { u"CharFontStyleName"_ustr, SVX_TABLE_FONT, cppu::UnoType<OUString>::get(),
              PropertyAttribute::MAYBEVOID, MID_FONT_STYLE_NAME },
            { u"CharFontFamily"_ustr, SVX_TABLE_FONT, cppu::UnoType<sal_Int16>::get(),
              PropertyAttribute::MAYBEVOID, MID_FONT_FAMILY },
            { u"CharFontCharSet"_ustr, SVX_TABLE_FONT, cppu::UnoType<sal_Int16>::get(),
              PropertyAttribute::MAYBEVOID, MID_FONT_CHAR_SET },
            { u"CharFontPitch"_ustr, SVX_TABLE_FONT, cppu::UnoType<sal_Int16>::get(),
              PropertyAttribute::MAYBEVOID, MID_FONT_PITCH },
            // CJK Font
            { u"CharHeightAsian"_ustr, SVX_TABLE_CJK_FONT_HEIGHT, cppu::UnoType<float>::get(),
              PropertyAttribute::MAYBEVOID, MID_FONTHEIGHT | CONVERT_TWIPS },
            { u"CharWeightAsian"_ustr, SVX_TABLE_CJK_FONT_WEIGHT, cppu::UnoType<float>::get(),
              PropertyAttribute::MAYBEVOID, MID_WEIGHT },
            { u"CharPostureAsian"_ustr, SVX_TABLE_CJK_FONT_POSTURE,
              cppu::UnoType<css::awt::FontSlant>::get(), PropertyAttribute::MAYBEVOID,
              MID_POSTURE },
            { u"CharFontNameAsian"_ustr, SVX_TABLE_CJK_FONT, cppu::UnoType<OUString>::get(),
              PropertyAttribute::MAYBEVOID, MID_FONT_FAMILY_NAME },
            { u"CharFontStyleNameAsian"_ustr, SVX_TABLE_CJK_FONT, cppu::UnoType<OUString>::get(),
              PropertyAttribute::MAYBEVOID, MID_FONT_STYLE_NAME },
            { u"CharFontFamilyAsian"_ustr, SVX_TABLE_CJK_FONT, cppu::UnoType<sal_Int16>::get(),
              PropertyAttribute::MAYBEVOID, MID_FONT_FAMILY },
            { u"CharFontCharSetAsian"_ustr, SVX_TABLE_CJK_FONT, cppu::UnoType<sal_Int16>::get(),
              PropertyAttribute::MAYBEVOID, MID_FONT_CHAR_SET },
            { u"CharFontPitchAsian"_ustr, SVX_TABLE_CJK_FONT, cppu::UnoType<sal_Int16>::get(),
              PropertyAttribute::MAYBEVOID, MID_FONT_PITCH },
            // CTL Font
            { u"CharHeightComplex"_ustr, SVX_TABLE_CTL_FONT_HEIGHT, cppu::UnoType<float>::get(),
              PropertyAttribute::MAYBEVOID, MID_FONTHEIGHT | CONVERT_TWIPS },
            { u"CharWeightComplex"_ustr, SVX_TABLE_CTL_FONT_WEIGHT, cppu::UnoType<float>::get(),
              PropertyAttribute::MAYBEVOID, MID_WEIGHT },
            { u"CharPostureComplex"_ustr, SVX_TABLE_CTL_FONT_POSTURE,
              cppu::UnoType<css::awt::FontSlant>::get(), PropertyAttribute::MAYBEVOID,
              MID_POSTURE },
            { u"CharFontNameComplex"_ustr, SVX_TABLE_CTL_FONT, cppu::UnoType<OUString>::get(),
              PropertyAttribute::MAYBEVOID, MID_FONT_FAMILY_NAME },
            { u"CharFontStyleNameComplex"_ustr, SVX_TABLE_CTL_FONT, cppu::UnoType<OUString>::get(),
              PropertyAttribute::MAYBEVOID, MID_FONT_STYLE_NAME },
            { u"CharFontFamilyComplex"_ustr, SVX_TABLE_CTL_FONT, cppu::UnoType<sal_Int16>::get(),
              PropertyAttribute::MAYBEVOID, MID_FONT_FAMILY },
            { u"CharFontCharSetComplex"_ustr, SVX_TABLE_CTL_FONT, cppu::UnoType<sal_Int16>::get(),
              PropertyAttribute::MAYBEVOID, MID_FONT_CHAR_SET },
            { u"CharFontPitchComplex"_ustr, SVX_TABLE_CTL_FONT, cppu::UnoType<sal_Int16>::get(),
              PropertyAttribute::MAYBEVOID, MID_FONT_PITCH },
            // Paragraph Properties
            { u"ParaAdjust"_ustr, SVX_TABLE_PARA_ADJUST, cppu::UnoType<sal_Int16>::get(),
              PropertyAttribute::MAYBEVOID, MID_PARA_ADJUST },
        };
    static SfxItemPropertySet aSvxTablePropertySet(aSvxTablePropertyMap);
    return aSvxTablePropertySet;
}

SvxAutoFormatDataField::SvxAutoFormatDataField(const SvxAutoFormatDataField& rCopy)
    : AutoFormatBase(rCopy)
    , mSetProperties(rCopy.mSetProperties)
{
}

void SvxAutoFormatDataField::GetValueFormat(OUString& rFormat, LanguageType& rLng,
                                            LanguageType& rSys) const
{
    rFormat = maNumFormatString;
    rLng = meNumFormatLanguage;
    rSys = meSysLanguage;
}

void SvxAutoFormatDataField::SetValueFormat(const OUString& rFormat, LanguageType eLng,
                                            LanguageType eSys)
{
    maNumFormatString = rFormat;
    meNumFormatLanguage = eLng;
    meSysLanguage = eSys;
}

SvxAutoFormatData::SvxAutoFormatData()
    : mbInclFont(true)
    , mbInclJustify(true)
    , mbInclFrame(true)
    , mbInclBackground(true)
    , mbInclValueFormat(true)
    , mbInclWidthHeight(true)
    , mbUseFirstRowStyles(true)
    , mbUseLastRowStyles(true)
    , mbUseFirstColStyles(true)
    , mbUseLastColStyles(true)
    , mbUseBandedRowStyles(true)
    , mbUseBandedColStyles(true)
{
}

SvxAutoFormatData::SvxAutoFormatData(const SvxAutoFormatData& rData)
    : maName(rData.maName)
    , mbInclFont(rData.mbInclFont)
    , mbInclJustify(rData.mbInclJustify)
    , mbInclFrame(rData.mbInclFrame)
    , mbInclBackground(rData.mbInclBackground)
    , mbInclValueFormat(rData.mbInclValueFormat)
    , mbInclWidthHeight(rData.mbInclWidthHeight)
    , mbUseFirstRowStyles(rData.mbUseFirstRowStyles)
    , mbUseLastRowStyles(rData.mbUseLastRowStyles)
    , mbUseFirstColStyles(rData.mbUseFirstColStyles)
    , mbUseLastColStyles(rData.mbUseLastColStyles)
    , mbUseBandedRowStyles(rData.mbUseBandedRowStyles)
    , mbUseBandedColStyles(rData.mbUseBandedColStyles)
    , maParent(rData.maParent)
{
}

SvxAutoFormatData& SvxAutoFormatData::operator=(const SvxAutoFormatData& rCopy)
{
    if (this == &rCopy)
        return *this;

    maName = rCopy.maName;
    mbInclFont = rCopy.mbInclFont;
    mbInclJustify = rCopy.mbInclJustify;
    mbInclFrame = rCopy.mbInclFrame;
    mbInclBackground = rCopy.mbInclBackground;
    mbInclValueFormat = rCopy.mbInclValueFormat;
    mbInclWidthHeight = rCopy.mbInclWidthHeight;
    mbUseFirstRowStyles = rCopy.mbUseFirstRowStyles;
    mbUseLastRowStyles = rCopy.mbUseLastRowStyles;
    mbUseFirstColStyles = rCopy.mbUseFirstColStyles;
    mbUseLastColStyles = rCopy.mbUseLastColStyles;
    mbUseBandedRowStyles = rCopy.mbUseBandedRowStyles;
    mbUseBandedColStyles = rCopy.mbUseBandedColStyles;

    return *this;
}

void SvxAutoFormatData::FillToItemSet(size_t nIndex, SfxItemSet& rItemSet) const
{
    if (nIndex >= ELEMENT_COUNT)
    {
        SAL_WARN("svx", "SvxAutoFormatData::FillToItemSet - index out of bounds: " << nIndex);
        return;
    }

    const SvxAutoFormatDataField& rField = *GetField(nIndex);
    const SvxAutoFormatDataField& rDefault = *GetDefaultField();

    if (IsFont())
    {
        // regular font
        if (rField.GetFont() != rDefault.GetFont())
            rItemSet.Put(rField.GetFont());

        if (rField.GetHeight() != rDefault.GetHeight())
            rItemSet.Put(rField.GetHeight());

        if (rField.GetWeight() != rDefault.GetWeight())
            rItemSet.Put(rField.GetWeight());

        if (rField.GetPosture() != rDefault.GetPosture())
            rItemSet.Put(rField.GetPosture());

        // CJK Font
        if (rField.GetCJKFont() != rDefault.GetCJKFont())
            rItemSet.Put(rField.GetCJKFont());

        if (rField.GetCJKHeight() != rDefault.GetCJKHeight())
            rItemSet.Put(rField.GetCJKHeight());
        if (rField.GetCJKWeight() != rDefault.GetCJKWeight())
            rItemSet.Put(rField.GetCJKWeight());

        if (rField.GetCJKPosture() != rDefault.GetCJKPosture())
            rItemSet.Put(rField.GetCJKPosture());

        // CTL font
        if (rField.GetCTLFont() != rDefault.GetCTLFont())
            rItemSet.Put(rField.GetCTLFont());

        if (rField.GetCTLHeight() != rDefault.GetCTLHeight())
            rItemSet.Put(rField.GetCTLHeight());

        if (rField.GetCTLWeight() != rDefault.GetCTLWeight())
            rItemSet.Put(rField.GetCTLWeight());

        if (rField.GetCTLPosture() != rDefault.GetCTLPosture())
            rItemSet.Put(rField.GetCTLPosture());

        // Font properties
        if (rField.GetUnderline() != rDefault.GetUnderline())
            rItemSet.Put(rField.GetUnderline());

        if (rField.GetColor() != rDefault.GetColor())
            rItemSet.Put(rField.GetColor());
    }

    if (IsJustify())
    {
        if (rField.GetHorJustify() != rDefault.GetHorJustify())
            rItemSet.Put(rField.GetHorJustify());

        if (rField.GetVerJustify() != rDefault.GetVerJustify())
            rItemSet.Put(rField.GetVerJustify());
    }

    if (IsBackground())
    {
        if (rField.GetBackground() != rDefault.GetBackground())
            rItemSet.Put(rField.GetBackground());
    }
}

void SvxAutoFormatData::MergeStyle(const SvxAutoFormatData& pParent)
{
    for (sal_uInt8 i = 0; i < ELEMENT_COUNT; i++)
    {
        if ((!UseFirstRowStyles()
             && (i == FIRST_ROW || i == FIRST_ROW_START_COL || i == FIRST_ROW_END_COL
                 || i == FIRST_ROW_EVEN_COL))
            || (!UseLastRowStyles()
                && (i == LAST_ROW || i == LAST_ROW_START_COL || i == LAST_ROW_END_COL
                    || i == LAST_ROW_EVEN_COL))
            || (!UseFirstColStyles() && i == FIRST_COL) || (!UseLastColStyles() && i == LAST_COL)
            || (!UseBandedRowStyles() && (i == EVEN_ROW || i == ODD_ROW))
            || (!UseBandedColStyles() && (i == EVEN_COL || i == ODD_COL)))
        {
            continue;
        }

        SvxAutoFormatDataField& pField = *GetField(i);
        const SvxAutoFormatDataField& pNew = *pParent.GetField(i);

        // regular font
        if (!pField.IsPropertySet(PROP_FONT))
            pField.SetFont(pNew.GetFont());
        if (!pField.IsPropertySet(PROP_HEIGHT))
            pField.SetHeight(pNew.GetHeight());
        if (!pField.IsPropertySet(PROP_WEIGHT))
            pField.SetWeight(pNew.GetWeight());
        if (!pField.IsPropertySet(PROP_POSTURE))
            pField.SetPosture(pNew.GetPosture());

        // CJK font
        if (!pField.IsPropertySet(PROP_CJK_FONT))
            pField.SetCJKFont(pNew.GetCJKFont());
        if (!pField.IsPropertySet(PROP_CJK_HEIGHT))
            pField.SetCJKHeight(pNew.GetCJKHeight());
        if (!pField.IsPropertySet(PROP_CJK_WEIGHT))
            pField.SetCJKWeight(pNew.GetCJKWeight());
        if (!pField.IsPropertySet(PROP_CJK_POSTURE))
            pField.SetCJKPosture(pNew.GetCJKPosture());

        // CTL font
        if (!pField.IsPropertySet(PROP_CTL_FONT))
            pField.SetCTLFont(pNew.GetCTLFont());
        if (!pField.IsPropertySet(PROP_CTL_HEIGHT))
            pField.SetCTLHeight(pNew.GetCTLHeight());
        if (!pField.IsPropertySet(PROP_CTL_WEIGHT))
            pField.SetCTLWeight(pNew.GetCTLWeight());
        if (!pField.IsPropertySet(PROP_CTL_POSTURE))
            pField.SetCTLPosture(pNew.GetCTLPosture());

        // Font properties
        if (!pField.IsPropertySet(PROP_UNDERLINE))
            pField.SetUnderline(pNew.GetUnderline());

        if (!pField.IsPropertySet(PROP_COLOR))
            pField.SetColor(pNew.GetColor());
        if (!pField.IsPropertySet(PROP_HOR_JUSTIFY))
        {
            pField.SetAdjust(pNew.GetAdjust());
            pField.SetHorJustify(pNew.GetHorJustify());
        }
        if (!pField.IsPropertySet(PROP_VER_JUSTIFY))
            pField.SetVerJustify(pNew.GetVerJustify());

        SvxBoxItem aBox = pField.IsPropertySet(PROP_BOX) ? pField.GetBox() : pNew.GetBox();
        sal_Int16 nPadding = pField.IsPropertySet(PROP_BOX)
                                 ? pField.GetBox().GetDistance(SvxBoxItemLine::RIGHT)
                                 : pNew.GetBox().GetDistance(SvxBoxItemLine::RIGHT);

        aBox.SetDistance(nPadding, SvxBoxItemLine::RIGHT);
        pField.SetBox(aBox);

        if (!pField.IsPropertySet(PROP_BACKGROUND))
            pField.SetBackground(pNew.GetBackground());
    }
}

void SvxAutoFormatData::ResetAutoFormat(SvxAutoFormatData& pOld)
{
    for (int i = 0; i < ELEMENT_COUNT; i++)
    {
        SvxAutoFormatDataField& pField = *GetField(i);
        const SvxAutoFormatDataField& pNew = *pOld.GetField(i);
        pField.SetPropertySet(pNew.GetPropertySet());
        // regular font
        pField.SetFont(pNew.GetFont());
        pField.SetHeight(pNew.GetHeight());
        pField.SetWeight(pNew.GetWeight());
        pField.SetPosture(pNew.GetPosture());

        // CJK font
        pField.SetCJKFont(pNew.GetCJKFont());
        pField.SetCJKHeight(pNew.GetCJKHeight());
        pField.SetCJKWeight(pNew.GetCJKWeight());
        pField.SetCJKPosture(pNew.GetCJKPosture());

        // CTL font
        pField.SetCTLFont(pNew.GetCTLFont());
        pField.SetCTLHeight(pNew.GetCTLHeight());
        pField.SetCTLWeight(pNew.GetCTLWeight());
        pField.SetCTLPosture(pNew.GetCTLPosture());

        // Font properties
        pField.SetUnderline(pNew.GetUnderline());
        pField.SetColor(pNew.GetColor());

        pField.SetAdjust(pNew.GetAdjust());
        pField.SetHorJustify(pNew.GetHorJustify());
        pField.SetVerJustify(pNew.GetVerJustify());

        pField.SetBox(pNew.GetBox());
        pField.SetBackground(pNew.GetBackground());
    }
}

SvxAutoFormat::~SvxAutoFormat() = default;

void SvxAutoFormat::ResetParent(const OUString& rName)
{
    OUString sParent;
    if (size())
        sParent = GetData(0)->GetName();

    for (size_t i = 0; i < size(); i++)
    {
        if (GetData(i)->GetParent() == rName)
        {
            GetData(i)->SetParent(sParent);
        }
    }
}

bool SvxAutoFormat::Load(bool bWriter)
{
    OUString sStyles = "/svx/tablestyles.xml";
    if (!bWriter)
        sStyles = "/calc/tablestyles.xml";

    try
    {
        OUString sPath(u"$BRAND_BASE_DIR/" LIBO_SHARE_FOLDER + sStyles);
        rtl::Bootstrap::expandMacros(sPath);
        SfxMedium aMedium(sPath, StreamMode::STD_READ);

        SvStream* pStream = aMedium.GetInStream();
        if (!pStream || pStream->GetError() != ERRCODE_NONE)
        {
            SAL_WARN("svx", "Cannot open table styles file: " << sPath);
            return false;
        }

        uno::Reference<uno::XComponentContext> xContext = comphelper::getProcessComponentContext();
        if (!xContext.is())
        {
            SAL_WARN("svx", "No component context available");
            return false;
        }

        uno::Reference<io::XInputStream> xInputStream = new utl::OInputStreamWrapper(*pStream);
        rtl::Reference<SvxTableStylesImport> xImport = new SvxTableStylesImport(xContext, *this);

        xml::sax::InputSource aInputSource;
        aInputSource.aInputStream = xInputStream;
        aInputSource.sSystemId = sPath;

        xImport->parseStream(aInputSource);
        return true;
    }
    catch (const uno::Exception& e)
    {
        SAL_WARN("svx", "Error loading auto formats: " << e.Message);
        return false;
    }
}

// This method is only called by calc. In writer tables styles are stored at document level
bool SvxAutoFormat::Save()
{
    try
    {
        OUString sPath(u"$BRAND_BASE_DIR/" LIBO_SHARE_FOLDER "/calc/tablestyles.xml"_ustr);
        rtl::Bootstrap::expandMacros(sPath);
        SfxMedium aMedium(sPath, StreamMode::STD_WRITE);

        SvStream* pStream = aMedium.GetOutStream();
        if (!pStream || pStream->GetError() != ERRCODE_NONE)
        {
            SAL_WARN("svx", "Cannot save table styles to file: " << sPath);
            return false;
        }

        uno::Reference<uno::XComponentContext> xContext = comphelper::getProcessComponentContext();
        if (!xContext.is())
        {
            SAL_WARN("svx", "No component context available");
            return false;
        }

        uno::Reference<io::XOutputStream> const xOutStream(new utl::OOutputStreamWrapper(*pStream));
        uno::Reference<xml::sax::XWriter> const xWriter(xml::sax::Writer::create(xContext));
        rtl::Reference<SvxTableStylesExport> xExport
            = new SvxTableStylesExport(xContext, sPath, xWriter, *this);

        xWriter->setOutputStream(xOutStream);
        xExport->ExportStyles();

        pStream->FlushBuffer();
        aMedium.Commit();
        return true;
    }
    catch (const uno::Exception& e)
    {
        SAL_WARN("svx", "Error saving auto formats: " << e.Message);
        return false;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
