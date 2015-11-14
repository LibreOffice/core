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

#include <svx/dialmgr.hxx>
#include <svx/dialogs.hrc>
#include <i18nlangtag/mslangid.hxx>
#include <svtools/valueset.hxx>
#include <svl/languageoptions.hxx>
#include <helpid.hrc>
#include <editeng/numitem.hxx>
#include <svl/eitem.hxx>
#include <vcl/svapp.hxx>
#include <svx/gallery.hxx>
#include <svl/urihelper.hxx>
#include <editeng/brushitem.hxx>
#include <svl/intitem.hxx>
#include <sfx2/objsh.hxx>
#include <vcl/graph.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/settings.hxx>
#include <vcl/builderfactory.hxx>
#include <editeng/flstitem.hxx>
#include <svx/dlgutil.hxx>
#include <svx/xtable.hxx>
#include <svx/drawitem.hxx>
#include <svx/numvset.hxx>
#include <sfx2/htmlmode.hxx>
#include <unotools/pathoptions.hxx>
#include <svtools/ctrltool.hxx>
#include <editeng/unolingu.hxx>
#include <com/sun/star/style/NumberingType.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/text/XDefaultNumberingProvider.hpp>
#include <com/sun/star/text/XNumberingFormatter.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/text/XNumberingTypeInfo.hpp>

#include <algorithm>
#include <sfx2/opengrf.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::i18n;
using namespace com::sun::star::text;
using namespace com::sun::star::container;
using namespace com::sun::star::style;

static const sal_Char cNumberingType[] = "NumberingType";
static const sal_Char cValue[] = "Value";
static const sal_Char cParentNumbering[] = "ParentNumbering";
static const sal_Char cPrefix[] = "Prefix";
static const sal_Char cSuffix[] = "Suffix";
static const sal_Char cBulletChar[] = "BulletChar";
static const sal_Char cBulletFontName[] = "BulletFontName";

// The selection of bullets from the star symbol
static const sal_Unicode aBulletTypes[] =
{
    0x2022,
    0x25cf,
    0xe00c,
    0xe00a,
    0x2794,
    0x27a2,
    0x2717,
    0x2714
};

static vcl::Font& lcl_GetDefaultBulletFont()
{
    static bool bInit = false;
    static vcl::Font aDefBulletFont( "StarSymbol", "", Size( 0, 14 ) );
    if(!bInit)
    {
        aDefBulletFont.SetCharSet( RTL_TEXTENCODING_SYMBOL );
        aDefBulletFont.SetFamily( FAMILY_DONTKNOW );
        aDefBulletFont.SetPitch( PITCH_DONTKNOW );
        aDefBulletFont.SetWeight( WEIGHT_DONTKNOW );
        aDefBulletFont.SetTransparent( true );
        bInit = true;
    }
    return aDefBulletFont;
}

static void lcl_PaintLevel(OutputDevice* pVDev, sal_Int16 nNumberingType,
                        const OUString& rBulletChar, const OUString& rText, const OUString& rFontName,
                        Point& rLeft, vcl::Font& rRuleFont, const vcl::Font& rTextFont)
{

    if(NumberingType::CHAR_SPECIAL == nNumberingType )
    {
        rRuleFont.SetStyleName(rFontName);
        pVDev->SetFont(rRuleFont);
        pVDev->DrawText(rLeft, rBulletChar);
        rLeft.X() += pVDev->GetTextWidth(rBulletChar);
    }
    else
    {
        pVDev->SetFont(rTextFont);
        pVDev->DrawText(rLeft, rText);
        rLeft.X() += pVDev->GetTextWidth(rText);
    }
}
void  SvxNumValueSet::UserDraw( const UserDrawEvent& rUDEvt )
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

    vcl::RenderContext* pDev = rUDEvt.GetRenderContext();
    Rectangle aRect = rUDEvt.GetRect();
    sal_uInt16 nItemId = rUDEvt.GetItemId();

    long nRectWidth = aRect.GetWidth();
    long nRectHeight = aRect.GetHeight();
    Size aRectSize(nRectWidth, aRect.GetHeight());
    Point aBLPos = aRect.TopLeft();
    vcl::Font aOldFont = pDev->GetFont();
    Color aOldColor = pDev->GetLineColor();
    pDev->SetLineColor(aTextColor);
    vcl::Font aFont(OutputDevice::GetDefaultFont(
                DefaultFontType::UI_SANS, MsLangId::getSystemLanguage(), GetDefaultFontFlags::OnlyOne));

    Size aSize = aFont.GetSize();

    vcl::Font aRuleFont( lcl_GetDefaultBulletFont() );
    aSize.Height() = nRectHeight/6;
    aRuleFont.SetSize(aSize);
    aRuleFont.SetColor(aTextColor);
    aRuleFont.SetFillColor(aBackColor);
    if(ePageType == NumberingPageType::BULLET)
        aFont = aRuleFont;
    else if(ePageType == NumberingPageType::OUTLINE)
    {
        aSize.Height() = nRectHeight/8;
    }
    aFont.SetColor(aTextColor);
    aFont.SetFillColor(aBackColor);
    aFont.SetSize( aSize );
    pDev->SetFont(aFont);

    if(!pVDev)
    {
        // The lines are only one time in the virtual device, only the outline
        // page is currently done
        pVDev = VclPtr<VirtualDevice>::Create(*pDev);
        pVDev->SetMapMode(pDev->GetMapMode());
        pVDev->EnableRTL( IsRTLEnabled() );
         pVDev->SetOutputSize( aRectSize );
        aOrgRect = aRect;
        pVDev->SetFillColor( aBackColor );
        pVDev->SetLineColor(COL_LIGHTGRAY);
        // Draw line only once
        if(ePageType != NumberingPageType::OUTLINE)
        {
            Point aStart(aBLPos.X() + nRectWidth *25 / 100,0);
            Point aEnd(aBLPos.X() + nRectWidth * 9 / 10,0);
            for( sal_uInt16 i = 11; i < 100; i += 33)
            {
                aStart.Y() = aEnd.Y() = aBLPos.Y() + nRectHeight  * i / 100;
                pVDev->DrawLine(aStart, aEnd);
                aStart.Y() = aEnd.Y() = aBLPos.Y() + nRectHeight  * (i + 11) / 100;
                pVDev->DrawLine(aStart, aEnd);
            }
        }
    }
    pDev->DrawOutDev(   aRect.TopLeft(), aRectSize,
                        aOrgRect.TopLeft(), aRectSize,
                        *pVDev );
    // Now comes the text
    const OUString sValue(cValue);
    if( NumberingPageType::SINGLENUM == ePageType ||
           NumberingPageType::BULLET == ePageType )
    {
        Point aStart(aBLPos.X() + nRectWidth / 9,0);
        for( sal_uInt16 i = 0; i < 3; i++ )
        {
            sal_uInt16 nY = 11 + i * 33;
            aStart.Y() = aBLPos.Y() + nRectHeight  * nY / 100;
            OUString sText;
            if(ePageType == NumberingPageType::BULLET)
            {
                sText = OUString( aBulletTypes[nItemId - 1] );
                aStart.Y() -= pDev->GetTextHeight()/2;
                aStart.X() = aBLPos.X() + 5;
            }
            else
            {
                if(xFormatter.is() && aNumSettings.getLength() > nItemId - 1)
                {
                    Sequence<PropertyValue> aLevel = aNumSettings.getConstArray()[nItemId - 1];
                    try
                    {
                        aLevel.realloc(aLevel.getLength() + 1);
                        PropertyValue& rValue = aLevel.getArray()[aLevel.getLength() - 1];
                        rValue.Name = sValue;
                        rValue.Value <<= (sal_Int32)(i + 1);
                        sText = xFormatter->makeNumberingString( aLevel, aLocale );
                    }
                    catch(Exception&)
                    {
                        OSL_FAIL("Exception in DefaultNumberingProvider::makeNumberingString");
                    }
                }
                // start just next to the left edge
                aStart.X() = aBLPos.X() + 2;
                aStart.Y() -= pDev->GetTextHeight()/2;
            }
            pDev->DrawText(aStart, sText);
        }
    }
    else if(NumberingPageType::OUTLINE == ePageType )
    {
        // Outline numbering has to be painted into the virtual device
        // to get correct lines
        // has to be made again
        pVDev->SetLineColor(aBackColor);
        pVDev->DrawRect(aOrgRect);
        long nStartX = aOrgRect.TopLeft().X();
        long nStartY = aOrgRect.TopLeft().Y();

        if(xFormatter.is() && aOutlineSettings.getLength() > nItemId - 1)
        {
            Reference<XIndexAccess> xLevel = aOutlineSettings.getArray()[nItemId - 1];
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
                for( sal_Int32 i = 0; i < nLevelCount && i < 5; i++)
                {
                    long nTop = nStartY + nRectHeight * (aLinesArr[2 * i + 11])/100 ;
                    Point aLeft(nStartX + nRectWidth *  (aLinesArr[2 * i + 10])/ 100, nTop );

                    Any aLevelAny = xLevel->getByIndex(i);
                    Sequence<PropertyValue> aLevel;
                    aLevelAny >>= aLevel;
                    const PropertyValue* pValues = aLevel.getConstArray();
                    aNumberingTypes[i] = 0;
                    aParentNumberings[i] = 0;
                    for(sal_Int32 nProperty = 0; nProperty < aLevel.getLength() - 1; nProperty++)
                    {
                        if ( pValues[nProperty].Name == cNumberingType )
                            pValues[nProperty].Value >>= aNumberingTypes[i];
                        else if ( pValues[nProperty].Name == cBulletFontName )
                            pValues[nProperty].Value >>= sFontNames[i];
                        else if ( pValues[nProperty].Name == cBulletChar )
                            pValues[nProperty].Value >>= sBulletChars[i];
                        else if ( pValues[nProperty].Name == cPrefix )
                            pValues[nProperty].Value >>= sPrefixes[i];
                        else if ( pValues[nProperty].Name == cSuffix )
                            pValues[nProperty].Value >>= sSuffixes[i];
                        else if ( pValues[nProperty].Name == cParentNumbering )
                            pValues[nProperty].Value >>= aParentNumberings[i];
                    }
                    Sequence< PropertyValue > aProperties(2);
                    PropertyValue* pProperties = aProperties.getArray();
                    pProperties[0].Name = "NumberingType";
                    pProperties[0].Value <<= aNumberingTypes[i];
                    pProperties[1].Name = "Value";
                    pProperties[1].Value <<= (sal_Int32)1;
                    try
                    {
                        sLevelTexts[i] = xFormatter->makeNumberingString( aProperties, aLocale );
                    }
                    catch(Exception&)
                    {
                        OSL_FAIL("Exception in DefaultNumberingProvider::makeNumberingString");
                    }

                    aLeft.Y() -= (pDev->GetTextHeight()/2);
                    if(!sPrefixes[i].isEmpty() &&
                        sPrefixes[i] != " ")
                    {
                        pVDev->SetFont(aFont);
                        pVDev->DrawText(aLeft, sPrefixes[i]);
                        aLeft.X() += pDev->GetTextWidth(sPrefixes[i]);
                    }
                    if(aParentNumberings[i])
                    {
                        //insert old numberings here
                        sal_Int32 nStartLevel = std::min((sal_Int32)aParentNumberings[i], i);
                        for(sal_Int32 nParentLevel = i - nStartLevel; nParentLevel < i; nParentLevel++)
                        {
                            OUString sTmp(sLevelTexts[nParentLevel]);
                            sTmp += ".";
                            lcl_PaintLevel(pVDev,
                                    aNumberingTypes[nParentLevel],
                                    sBulletChars[nParentLevel],
                                    sTmp,
                                    sFontNames[nParentLevel],
                                    aLeft,
                                    aRuleFont,
                                    aFont);
                        }
                    }
                    lcl_PaintLevel(pVDev,
                                    aNumberingTypes[i],
                                    sBulletChars[i],
                                    sLevelTexts[i],
                                    sFontNames[i],
                                    aLeft,
                                    aRuleFont,
                                    aFont);
                    if(!sSuffixes[i].isEmpty() &&
                       !sSuffixes[i].startsWith(" "))
                    {
                        pVDev->SetFont(aFont);
                        pVDev->DrawText(aLeft, sSuffixes[i]);
                        aLeft.X() += pDev->GetTextWidth(sSuffixes[i]);
                    }

                    long nLineTop = nStartY + nRectHeight * aLinesArr[2 * i + 1]/100 ;
                    Point aLineLeft(aLeft.X(), nLineTop );
                    Point aLineRight(nStartX + nRectWidth * 90 /100, nLineTop );
                    pVDev->SetLineColor(COL_LIGHTGRAY);
                    pVDev->DrawLine(aLineLeft,  aLineRight);
                }

            }
#ifdef DBG_UTIL
            catch(Exception&)
            {
                static bool bAssert = false;
                if(!bAssert)
                {
                    OSL_FAIL("exception in ::UserDraw");
                    bAssert = true;
                }
            }
#else
            catch(Exception&)
            {
            }
#endif
        }
        pDev->DrawOutDev(   aRect.TopLeft(), aRectSize,
                            aOrgRect.TopLeft(), aRectSize,
                            *pVDev );
    }

    pDev->SetFont(aOldFont);
    pDev->SetLineColor(aOldColor);
}

SvxNumValueSet::SvxNumValueSet(vcl::Window* pParent, WinBits nWinBits)
    : ValueSet(pParent, nWinBits)
    , ePageType(NumberingPageType::BULLET)
    , bHTMLMode(false)
    , pVDev(nullptr)
{
}

VCL_BUILDER_FACTORY_ARGS(SvxNumValueSet, WB_TABSTOP)

void SvxNumValueSet::init(NumberingPageType eType)
{
    ePageType = eType;
    bHTMLMode = false;
    pVDev = nullptr;

    SetColCount( 4 );
    SetLineCount( 2 );
    SetStyle( GetStyle() | WB_ITEMBORDER | WB_DOUBLEBORDER );
    if(NumberingPageType::BULLET == eType)
    {
        for ( sal_uInt16 i = 0; i < 8; i++ )
        {
            InsertItem( i + 1, i );
            SetItemText( i + 1, SVX_RESSTR( RID_SVXSTR_BULLET_DESCRIPTIONS + i ) );
        }
    }
}

SvxNumValueSet::~SvxNumValueSet()
{
    disposeOnce();
}

void SvxNumValueSet::dispose()
{
    pVDev.disposeAndClear();
    ValueSet::dispose();
}

void SvxNumValueSet::SetNumberingSettings(
    const Sequence<Sequence<PropertyValue> >& aNum,
    Reference<XNumberingFormatter>& xFormat,
    const Locale& rLocale   )
{
    aNumSettings = aNum;
    xFormatter = xFormat;
    aLocale = rLocale;
    if(aNum.getLength() > 8)
            SetStyle( GetStyle()|WB_VSCROLL);
    for ( sal_Int32 i = 0; i < aNum.getLength(); i++ )
    {
            InsertItem( i + 1, i );
            if( i < 8 )
                SetItemText( i + 1, SVX_RESSTR( RID_SVXSTR_SINGLENUM_DESCRIPTIONS + i ));
    }
}

void SvxNumValueSet::SetOutlineNumberingSettings(
            Sequence<Reference<XIndexAccess> >& rOutline,
            Reference<XNumberingFormatter>& xFormat,
            const Locale& rLocale)
{
    aOutlineSettings = rOutline;
    xFormatter = xFormat;
    aLocale = rLocale;
    if(aOutlineSettings.getLength() > 8)
        SetStyle( GetStyle() | WB_VSCROLL );
    for ( sal_Int32 i = 0; i < aOutlineSettings.getLength(); i++ )
    {
        InsertItem( i + 1, i );
        if( i < 8 )
            SetItemText( i + 1, SVX_RESSTR( RID_SVXSTR_OUTLINENUM_DESCRIPTIONS + i ));
    }
}

SvxBmpNumValueSet::SvxBmpNumValueSet(vcl::Window* pParent, WinBits nWinBits)
    : SvxNumValueSet(pParent, nWinBits)
{
    init();
}

VCL_BUILDER_FACTORY_ARGS(SvxBmpNumValueSet, WB_TABSTOP)

void SvxBmpNumValueSet::init()
{
    SvxNumValueSet::init(NumberingPageType::BITMAP);
    bGrfNotFound = false;
    GalleryExplorer::BeginLocking(GALLERY_THEME_BULLETS);
    SetStyle( GetStyle() | WB_VSCROLL );
    SetLineCount( 3 );
    aFormatIdle.SetPriority(SchedulerPriority::LOWEST);
    aFormatIdle.SetIdleHdl(LINK(this, SvxBmpNumValueSet, FormatHdl_Impl));
}


SvxBmpNumValueSet::~SvxBmpNumValueSet()
{
    disposeOnce();
}

void SvxBmpNumValueSet::dispose()
{
    GalleryExplorer::EndLocking(GALLERY_THEME_BULLETS);
    aFormatIdle.Stop();
    SvxNumValueSet::dispose();
}

void SvxBmpNumValueSet::UserDraw(const UserDrawEvent& rUDEvt)
{
    SvxNumValueSet::UserDraw(rUDEvt);

    Rectangle aRect = rUDEvt.GetRect();
    vcl::RenderContext* pDev = rUDEvt.GetRenderContext();
    sal_uInt16  nItemId = rUDEvt.GetItemId();
    Point aBLPos = aRect.TopLeft();

    long nRectHeight = aRect.GetHeight();
    Size aSize(nRectHeight/8, nRectHeight/8);

    Graphic aGraphic;
    if(!GalleryExplorer::GetGraphicObj( GALLERY_THEME_BULLETS, nItemId - 1,
                        &aGraphic))
    {
        bGrfNotFound = true;
    }
    else
    {
        Point aPos(aBLPos.X() + 5, 0);
        for( sal_uInt16 i = 0; i < 3; i++ )
        {
            sal_uInt16 nY = 11 + i * 33;
            aPos.Y() = aBLPos.Y() + nRectHeight  * nY / 100;
            aGraphic.Draw( pDev, aPos, aSize );
        }
    }
}

IMPL_LINK_NOARG_TYPED(SvxBmpNumValueSet, FormatHdl_Impl, Idle *, void)
{
    // only when a graphics was not there, it needs to be formatted
    if (bGrfNotFound)
    {
        SetFormat();
        bGrfNotFound = false;
    }
    Invalidate();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
