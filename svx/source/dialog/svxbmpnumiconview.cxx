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

#include <com/sun/star/text/DefaultNumberingProvider.hpp>
#include <com/sun/star/text/XNumberingFormatter.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/graphic/XGraphic.hpp>

#include <vcl/virdev.hxx>
#include <i18nlangtag/mslangid.hxx>
#include <editeng/svxenum.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <svx/dialmgr.hxx>
#include <svx/strings.hrc>
#include <officecfg/Office/Common.hxx>

#include <svx/svxbmpnumiconview.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::text;
using namespace com::sun::star::container;
using namespace com::sun::star::style;

const TranslateId OUTLINE_DESCRIPTIONS[] = {
    RID_SVXSTR_OUTLINENUM_DESCRIPTION_0,
    RID_SVXSTR_OUTLINENUM_DESCRIPTION_1,
    RID_SVXSTR_OUTLINENUM_DESCRIPTION_2,
    RID_SVXSTR_OUTLINENUM_DESCRIPTION_3,
    RID_SVXSTR_OUTLINENUM_DESCRIPTION_4,
    RID_SVXSTR_OUTLINENUM_DESCRIPTION_5,
    RID_SVXSTR_OUTLINENUM_DESCRIPTION_6,
    RID_SVXSTR_OUTLINENUM_DESCRIPTION_7
};

const TranslateId SINGLENUM_DESCRIPTIONS[] = {
    RID_SVXSTR_SINGLENUM_DESCRIPTION_0,
    RID_SVXSTR_SINGLENUM_DESCRIPTION_1,
    RID_SVXSTR_SINGLENUM_DESCRIPTION_2,
    RID_SVXSTR_SINGLENUM_DESCRIPTION_3,
    RID_SVXSTR_SINGLENUM_DESCRIPTION_4,
    RID_SVXSTR_SINGLENUM_DESCRIPTION_5,
    RID_SVXSTR_SINGLENUM_DESCRIPTION_6,
    RID_SVXSTR_SINGLENUM_DESCRIPTION_7
};

vcl::Font& SvxBmpNumIconView::GetDefaultBulletFont()
{
    static vcl::Font aDefBulletFont = []()
    {
        static vcl::Font tmp(u"OpenSymbol"_ustr, u""_ustr, Size(0, 14));
        tmp.SetCharSet( RTL_TEXTENCODING_SYMBOL );
        tmp.SetFamily( FAMILY_DONTKNOW );
        tmp.SetPitch( PITCH_DONTKNOW );
        tmp.SetWeight( WEIGHT_DONTKNOW );
        tmp.SetTransparent( true );
        return tmp;
    }();
    return aDefBulletFont;
}

void SvxBmpNumIconView::PopulateIconView(
    weld::IconView* pIconView,
    NumberingPageType ePageType,
    Size previewSize,
    const Sequence<Sequence<PropertyValue>>& rNumSettings,
    const Sequence<Reference<XIndexAccess>>& rOutlineSettings,
    Reference<XNumberingFormatter> const& xFormatter,
    const Locale& rLocale)
{
    if (!pIconView)
        return;

    pIconView->clear();

    if (ePageType == NumberingPageType::BULLET)
    {
        Sequence<OUString> aBulletSymbols =
            officecfg::Office::Common::BulletsNumbering::DefaultBullets::get();

        for (sal_Int32 i = 0; i < std::min(aBulletSymbols.getLength(), sal_Int32(8)); ++i)
        {
            VclPtr<VirtualDevice> pVDev = CreatePreviewFromUserDraw(NumberingPageType::BULLET, i, previewSize, rNumSettings, rOutlineSettings, xFormatter, rLocale);
            OUString sId = OUString::number(i);
            OUString sText = GetNumberingDescription(ePageType, i);
            pIconView->insert(-1, &sText, &sId, pVDev, nullptr);
        }
    }
}

VclPtr<VirtualDevice> SvxBmpNumIconView::CreatePreviewFromUserDraw(
    NumberingPageType ePageType,
    sal_Int32 nIndex,
    Size previewSize,
    const Sequence<Sequence<PropertyValue>>& rNumSettings,
    const Sequence<Reference<XIndexAccess>>& rOutlineSettings,
    Reference<XNumberingFormatter> const& xFormatter,
    const Locale& rLocale,
    const std::vector<std::pair<OUString, OUString>>& rCustomBullets)
{
    static const sal_uInt16 aLinesArr[] =
    {
        15, 10,
        20, 30,
        25, 50,
        30, 70,
        35, 90, // up to here line positions
         5, 10, // character positions
        10, 30,
        15, 50,
        20, 70,
        25, 90,
    };

    const Color aBackColor(COL_WHITE);
    const Color aTextColor(COL_BLACK);

    VclPtr<VirtualDevice> pVDev = VclPtr<VirtualDevice>::Create();
    pVDev->SetOutputSizePixel(previewSize);


    tools::Long nRectWidth = previewSize.Width();
    tools::Long nRectHeight = previewSize.Height();
    Point aBLPos(0, 0);

    vcl::Font aFont(OutputDevice::GetDefaultFont(
                DefaultFontType::UI_SANS, MsLangId::getConfiguredSystemLanguage(), GetDefaultFontFlags::OnlyOne));

    Size aSize = aFont.GetFontSize();

    vcl::Font aRuleFont( GetDefaultBulletFont() );
    Sequence<OUString> aBulletSymbols;

    if(ePageType == NumberingPageType::BULLET || ePageType == NumberingPageType::DOCBULLET)
    {
        if(ePageType == NumberingPageType::BULLET)
        {
            aBulletSymbols = officecfg::Office::Common::BulletsNumbering::DefaultBullets::get();
            Sequence< OUString > aBulletFonts =
                officecfg::Office::Common::BulletsNumbering::DefaultBulletsFonts::get();
            if (nIndex < aBulletFonts.getLength())
            {
                aRuleFont.SetFamilyName(aBulletFonts[nIndex]);
            }
        }
        else // DOCBULLET
        {
            if (o3tl::make_unsigned(nIndex) < rCustomBullets.size())
            {
                aRuleFont.SetFamilyName(rCustomBullets[nIndex].second);
            }
        }
        aSize.setHeight(nRectHeight/6);
        aRuleFont.SetFontSize(aSize);
        aRuleFont.SetColor(aTextColor);
        aRuleFont.SetFillColor(aBackColor);
        aFont = aRuleFont;
    }
    else if(ePageType == NumberingPageType::OUTLINE)
    {
        aSize.setHeight( nRectHeight/8 );
    }
    else // SINGLENUM
    {
        aSize.setHeight(nRectHeight/6);
    }

    aFont.SetColor(aTextColor);
    aFont.SetFillColor(aBackColor);
    aFont.SetFontSize( aSize );
    pVDev->SetFont(aFont);

    pVDev->SetFillColor(aBackColor);
    pVDev->SetLineColor(aTextColor);
    pVDev->DrawRect(tools::Rectangle(Point(0,0), previewSize));

    // Draw background lines for non-outline types
    if(ePageType != NumberingPageType::OUTLINE)
    {
        pVDev->SetLineColor(COL_LIGHTGRAY);
        Point aStart(aBLPos.X() + nRectWidth * 25 / 100, 0);
        Point aEnd(aBLPos.X() + nRectWidth * 9 / 10, 0);
        for(sal_uInt16 i = 11; i < 100; i += 33)
        {
            aStart.setY(aBLPos.Y() + nRectHeight * i / 100);
            aEnd.setY(aStart.Y());
            pVDev->DrawLine(aStart, aEnd);
            aStart.setY(aBLPos.Y() + nRectHeight * (i + 11) / 100);
            aEnd.setY(aStart.Y());
            pVDev->DrawLine(aStart, aEnd);
        }
    }

    // Render content based on page type
    if(ePageType == NumberingPageType::SINGLENUM ||
       ePageType == NumberingPageType::BULLET ||
       ePageType == NumberingPageType::DOCBULLET)
    {
        Point aStart(aBLPos.X() + nRectWidth / 9, 0);
        for(sal_uInt16 i = 0; i < 3; i++)
        {
            sal_uInt16 nY = 11 + i * 33;
            aStart.setY(aBLPos.Y() + nRectHeight * nY / 100);
            OUString sText;

            if(ePageType == NumberingPageType::BULLET)
            {
                if (nIndex < aBulletSymbols.getLength())
                    sText = aBulletSymbols[nIndex];
                aStart.AdjustY(-(pVDev->GetTextHeight()/2));
                aStart.setX(aBLPos.X() + 5);
            }
            else if(ePageType == NumberingPageType::DOCBULLET)
            {
                if (o3tl::make_unsigned(nIndex) < rCustomBullets.size())
                    sText = rCustomBullets[nIndex].first;
                aStart.AdjustY(-(pVDev->GetTextHeight()/2));
                aStart.setX(aBLPos.X() + 5);
            }
            else // SINGLENUM
            {
                if(xFormatter.is() && rNumSettings.getLength() > nIndex)
                {
                    Sequence<PropertyValue> aLevel = rNumSettings.getConstArray()[nIndex];
                    try
                    {
                        aLevel.realloc(aLevel.getLength() + 1);
                        PropertyValue& rValue = aLevel.getArray()[aLevel.getLength() - 1];
                        rValue.Name = "Value";
                        rValue.Value <<= static_cast<sal_Int32>(i + 1);
                        sText = xFormatter->makeNumberingString(aLevel, rLocale);
                    }
                    catch(Exception&)
                    {
                        sText = OUString::number(i + 1);
                    }
                }
                aStart.setX(aBLPos.X() + 2);
                aStart.AdjustY(-(pVDev->GetTextHeight()/2));
            }
            pVDev->DrawText(aStart, sText);
        }
    }
    else if(ePageType == NumberingPageType::OUTLINE)
    {
        tools::Long nStartX = 0;
        tools::Long nStartY = 0;

        if(xFormatter.is() && rOutlineSettings.getLength() > nIndex)
        {
            Reference<XIndexAccess> xLevel = rOutlineSettings.getConstArray()[nIndex];
            try
            {
                OUString sLevelTexts[5];
                OUString sFontNames[5];
                OUString sBulletChars[5];
                sal_Int16 aNumberingTypes[5];
                OUString sPrefixes[5];
                OUString sSuffixes[5];
                sal_Int16 aParentNumberings[5];

                sal_Int32 nLevelCount = xLevel->getCount();
                if(nLevelCount > 5)
                    nLevelCount = 5;

                for(sal_Int32 i = 0; i < nLevelCount; i++)
                {
                    tools::Long nTop = nStartY + nRectHeight * (aLinesArr[2 * i + 11])/100;
                    Point aLeft(nStartX + nRectWidth * (aLinesArr[2 * i + 10])/100, nTop);

                    Any aLevelAny = xLevel->getByIndex(i);
                    Sequence<PropertyValue> aLevel;
                    aLevelAny >>= aLevel;

                    aNumberingTypes[i] = 0;
                    aParentNumberings[i] = 0;

                    for (const PropertyValue& rProp : aLevel)
                    {
                        if (rProp.Name == "NumberingType")
                            rProp.Value >>= aNumberingTypes[i];
                        else if (rProp.Name == "BulletFontName")
                            rProp.Value >>= sFontNames[i];
                        else if (rProp.Name == "BulletChar")
                            rProp.Value >>= sBulletChars[i];
                        else if (rProp.Name == "Prefix")
                            rProp.Value >>= sPrefixes[i];
                        else if (rProp.Name == "Suffix")
                            rProp.Value >>= sSuffixes[i];
                        else if (rProp.Name == "ParentNumbering")
                            rProp.Value >>= aParentNumberings[i];
                    }

                    // Generate numbering text using formatter
                    Sequence<PropertyValue> aProperties(2);
                    PropertyValue* pProperties = aProperties.getArray();
                    pProperties[0].Name = "NumberingType";
                    pProperties[0].Value <<= aNumberingTypes[i];
                    pProperties[1].Name = "Value";
                    pProperties[1].Value <<= sal_Int32(1);

                    try
                    {
                        sLevelTexts[i] = xFormatter->makeNumberingString(aProperties, rLocale);
                    }
                    catch(Exception&)
                    {
                        sLevelTexts[i] = "1";
                    }

                    aLeft.AdjustY(-(pVDev->GetTextHeight()/2));

                    // Draw prefix
                    if(!sPrefixes[i].isEmpty() && sPrefixes[i] != " ")
                    {
                        pVDev->SetFont(aFont);
                        pVDev->DrawText(aLeft, sPrefixes[i]);
                        aLeft.AdjustX(pVDev->GetTextWidth(sPrefixes[i]));
                    }

                    // Draw parent numbering
                    if(aParentNumberings[i])
                    {
                        sal_Int32 nStartLevel = std::min(static_cast<sal_Int32>(aParentNumberings[i]), i);
                        for(sal_Int32 nParentLevel = i - nStartLevel; nParentLevel < i; nParentLevel++)
                        {
                            OUString sTmp = sLevelTexts[nParentLevel] + ".";

                            if(aNumberingTypes[nParentLevel] == SVX_NUM_CHAR_SPECIAL && !sBulletChars[nParentLevel].isEmpty())
                            {
                                if(!sFontNames[nParentLevel].isEmpty())
                                {
                                    vcl::Font aBulletFont(sFontNames[nParentLevel], aSize);
                                    aBulletFont.SetColor(aTextColor);
                                    pVDev->SetFont(aBulletFont);
                                }
                                pVDev->DrawText(aLeft, sBulletChars[nParentLevel]);
                                aLeft.AdjustX(pVDev->GetTextWidth(sBulletChars[nParentLevel]));
                            }
                            else
                            {
                                pVDev->SetFont(aFont);
                                pVDev->DrawText(aLeft, sTmp);
                                aLeft.AdjustX(pVDev->GetTextWidth(sTmp));
                            }
                        }
                    }

                    // Draw current level
                    if(aNumberingTypes[i] == SVX_NUM_CHAR_SPECIAL && !sBulletChars[i].isEmpty())
                    {
                        if(!sFontNames[i].isEmpty())
                        {
                            vcl::Font aBulletFont(sFontNames[i], aSize);
                            aBulletFont.SetColor(aTextColor);
                            pVDev->SetFont(aBulletFont);
                        }
                        pVDev->DrawText(aLeft, sBulletChars[i]);
                        aLeft.AdjustX(pVDev->GetTextWidth(sBulletChars[i]));
                    }
                    else
                    {
                        pVDev->SetFont(aFont);
                        pVDev->DrawText(aLeft, sLevelTexts[i]);
                        aLeft.AdjustX(pVDev->GetTextWidth(sLevelTexts[i]));
                    }

                    // Draw suffix
                    if(!sSuffixes[i].isEmpty() && !sSuffixes[i].startsWith(" "))
                    {
                        pVDev->SetFont(aFont);
                        pVDev->DrawText(aLeft, sSuffixes[i]);
                        aLeft.AdjustX(pVDev->GetTextWidth(sSuffixes[i]));
                    }

                    // Draw horizontal line
                    tools::Long nLineTop = nStartY + nRectHeight * aLinesArr[2 * i + 1]/100;
                    Point aLineLeft(aLeft.X(), nLineTop);
                    Point aLineRight(nStartX + nRectWidth * 90/100, nLineTop);
                    pVDev->SetLineColor(COL_LIGHTGRAY);
                    pVDev->DrawLine(aLineLeft, aLineRight);
                }

            }
 #ifdef DBG_UTIL
            catch(Exception&)
            {
                static bool bAssert = false;
                if(!bAssert)
                {
                    TOOLS_WARN_EXCEPTION("svx.dialog", "");
                    bAssert = true;
                }
            }
#else
            catch(Exception&)
            {
            }
#endif
        }
    }

    return pVDev;
}

VclPtr<VirtualDevice> SvxBmpNumIconView::CreateCustomBulletPreview(const OUString& rBulletChar, const OUString& rFontName)
{
    VclPtr<VirtualDevice> pVDev = VclPtr<VirtualDevice>::Create();
    Size aSize(80, 100);
    pVDev->SetOutputSizePixel(aSize);

    const Color aBackColor(COL_WHITE);
    const Color aTextColor(COL_BLACK);

    pVDev->SetFillColor(aBackColor);
    pVDev->SetLineColor(aTextColor);
    pVDev->DrawRect(tools::Rectangle(Point(0,0), aSize));

    // Add horizontal lines
    pVDev->SetLineColor(COL_LIGHTGRAY);
    Point aStart(aSize.Width() * 25 / 100, 0);
    Point aEnd(aSize.Width() * 9 / 10, 0);
    for (sal_uInt16 i = 11; i < 100; i += 33)
    {
        aStart.setY(aSize.Height() * i / 100);
        aEnd.setY(aStart.Y());
        pVDev->DrawLine(aStart, aEnd);
        aStart.setY(aSize.Height() * (i + 11) / 100);
        aEnd.setY(aStart.Y());
        pVDev->DrawLine(aStart, aEnd);
    }

    vcl::Font aRuleFont(rFontName, Size(0, aSize.Height()/6));
    aRuleFont.SetCharSet(RTL_TEXTENCODING_SYMBOL);
    aRuleFont.SetColor(aTextColor);
    aRuleFont.SetFillColor(aBackColor);
    pVDev->SetFont(aRuleFont);

    for (sal_uInt16 i = 0; i < 3; i++) // Show 3 sample lines
    {
        sal_uInt16 nY = 11 + i * 33;
        Point pStart(5, aSize.Height() * nY / 100);
        pStart.AdjustY(-(pVDev->GetTextHeight()/2));
        pVDev->DrawText(pStart, rBulletChar);
    }

    return pVDev;
}

OUString SvxBmpNumIconView::GetNumberingDescription(NumberingPageType ePageType, sal_Int32 nIndex)
{
    if (nIndex < 0)
        return OUString();

    switch (ePageType)
    {
        case NumberingPageType::OUTLINE: {
            constexpr auto size = std::size(OUTLINE_DESCRIPTIONS);
            if (nIndex < static_cast<sal_Int32>(size))
                return SvxResId(OUTLINE_DESCRIPTIONS[nIndex]);
            break;
        }
        case NumberingPageType::SINGLENUM: {
            constexpr auto size = std::size(SINGLENUM_DESCRIPTIONS);
            if (nIndex < static_cast<sal_Int32>(size))
                return SvxResId(SINGLENUM_DESCRIPTIONS[nIndex]);
            break;
        }
        case NumberingPageType::BULLET:
            return u"Bullet " + OUString::number(nIndex + 1); // convert and display 1-based index
        default:
            break;
    }

    return OUString();
}

void SvxBmpNumIconView::SetNumberingSettings(
    weld::IconView* mxIconView,
    Size previewSize,
    const Sequence<Sequence<PropertyValue>>& aNum,
    Reference<XNumberingFormatter> const& xFormat,
    const Locale& rLocale,
    std::vector<std::pair<OUString, OUString>> maCustomBullets)
{
    mxIconView->clear();

    for (sal_Int32 i = 0; i < aNum.getLength(); ++i)
    {
        VclPtr<VirtualDevice> pVDev = CreatePreviewFromUserDraw(
            NumberingPageType::SINGLENUM, i, previewSize, aNum, Sequence<Reference<XIndexAccess>>(),
            xFormat, rLocale, maCustomBullets);

        OUString sId = OUString::number(i);
        OUString sText = GetNumberingDescription(NumberingPageType::SINGLENUM, i);
        mxIconView->insert(-1, &sText, &sId, pVDev, nullptr);
    }
}

void SvxBmpNumIconView::SetOutlineNumberingSettings(
    weld::IconView* mxIconView,
    Size previewSize,
    const Sequence<Reference<XIndexAccess>>& rOutline,
    Reference<XNumberingFormatter> const& xFormat,
    const Locale& rLocale,
    std::vector<std::pair<OUString, OUString>> maCustomBullets)
{
    mxIconView->clear();

    for (sal_Int32 i = 0; i < rOutline.getLength(); ++i)
    {
        VclPtr<VirtualDevice> pVDev = CreatePreviewFromUserDraw(
            NumberingPageType::OUTLINE, i, previewSize, Sequence<Sequence<PropertyValue>>(), rOutline,
            xFormat, rLocale, maCustomBullets);

        OUString sId = OUString::number(i);
        OUString sText = GetNumberingDescription(NumberingPageType::OUTLINE, i);
        mxIconView->insert(-1, &sText, &sId, pVDev, nullptr);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
