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

#include "PresenterTextView.hxx"
#include <facreg.hxx>

#include <i18nlangtag/mslangid.hxx>
#include <cppcanvas/vclfactory.hxx>
#include <svl/itempool.hxx>
#include <svl/itemset.hxx>
#include <unotools/linguprops.hxx>
#include <unotools/lingucfg.hxx>
#include <editeng/colritem.hxx>
#include <editeng/editeng.hxx>
#include <editeng/editstat.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/fontitem.hxx>
#include <svx/xflclit.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/svapp.hxx>
#include <vcl/virdev.hxx>
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/rendering/XSpriteCanvas.hpp>
#include <com/sun/star/rendering/XBitmapCanvas.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/util/Color.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;

static const OUStringLiteral gsTextPropertyName("Text");
static const OUStringLiteral gsBitmapPropertyName("Bitmap");
static const OUStringLiteral gsSizePropertyName("Size");
static const OUStringLiteral gsBackgroundColorPropertyName("BackgroundColor");
static const OUStringLiteral gsTextColorPropertyName("TextColor");
static const OUStringLiteral gsFontDescriptorPropertyName("FontDescriptor");
static const OUStringLiteral gsTopPropertyName("Top");
static const OUStringLiteral gsTopRelativePropertyName("RelativeTop");
static const OUStringLiteral gsTotalHeightPropertyName("TotalHeight");

namespace sd { namespace presenter {

// PresenterTextView::Implementation
class PresenterTextView::Implementation
{
public:
    Implementation();
    ~Implementation();

    void SetCanvas (const cppcanvas::CanvasSharedPtr& rCanvas);
    void SetSize (const Size aSize);
    void SetBackgroundColor (const Color aColor);
    void SetTextColor (const Color aColor);
    void SetFontDescriptor (const awt::FontDescriptor& rFontDescriptor);
    sal_Int32 GetTop() const { return mnTop;}
    void SetTop (const sal_Int32 nTop);
    void SetText (const OUString& Text);
    sal_Int32 ParseDistance (const OUString& rsDistance) const;
    Reference<rendering::XBitmap> const & GetBitmap();
    sal_Int32 GetTotalHeight();

private:
    Reference<rendering::XBitmap> mxBitmap;
    cppcanvas::CanvasSharedPtr mpCanvas;
    VclPtr<VirtualDevice> mpOutputDevice;
    std::unique_ptr<EditEngine> mpEditEngine;
    SfxItemPool* mpEditEngineItemPool;
    Size maSize;
    OUString msText;
    sal_Int32 mnTop;
    sal_Int32 mnTotalHeight;

    void CheckTop();
};

// PresenterTextView
PresenterTextView::PresenterTextView ()
    : PresenterTextViewInterfaceBase(),
      mpImplementation(new Implementation())
{
}

PresenterTextView::~PresenterTextView()
{
}

void SAL_CALL PresenterTextView::disposing()
{
    mpImplementation.reset();
}

// XInitialization
void SAL_CALL PresenterTextView::initialize (const Sequence<Any>& rArguments)
{
    ThrowIfDisposed();

    if (rArguments.getLength() != 1)
    {
        throw RuntimeException("PresenterTextView: invalid number of arguments",
                static_cast<XWeak*>(this));
    }

    Reference<rendering::XCanvas> xCanvas (rArguments[0], UNO_QUERY_THROW);
    mpImplementation->SetCanvas(
        cppcanvas::VCLFactory::createCanvas(xCanvas));
}

Any PresenterTextView::GetPropertyValue (const OUString& rsPropertyName)
{
    ThrowIfDisposed();

    if (rsPropertyName == gsBitmapPropertyName)
    {
        return Any(mpImplementation->GetBitmap());
    }
    else if (rsPropertyName == gsTopPropertyName)
    {
        return Any(mpImplementation->GetTop());
    }
    else if (rsPropertyName == gsTotalHeightPropertyName)
    {
        return Any(mpImplementation->GetTotalHeight());
    }

    return Any();
}

Any PresenterTextView::SetPropertyValue (
    const OUString& rsPropertyName,
    const css::uno::Any& rValue)
{
    ThrowIfDisposed();

    Any aOldValue;
    if (rsPropertyName == gsTextPropertyName)
    {
        OUString sText;
        if (rValue >>= sText)
            mpImplementation->SetText(sText);
    }
    else if (rsPropertyName == gsSizePropertyName)
    {
        awt::Size aSize;
        if (rValue >>= aSize)
            mpImplementation->SetSize(Size(aSize.Width,aSize.Height));
    }
    else if (rsPropertyName == gsBackgroundColorPropertyName)
    {
        util::Color aColor = util::Color();
        if (rValue >>= aColor)
            mpImplementation->SetBackgroundColor(Color(aColor));
    }
    else if (rsPropertyName == gsTextColorPropertyName)
    {
        util::Color aColor = util::Color();
        if (rValue >>= aColor)
            mpImplementation->SetTextColor(Color(aColor));
    }
    else if (rsPropertyName == gsFontDescriptorPropertyName)
    {
        awt::FontDescriptor aFontDescriptor;
        if (rValue >>= aFontDescriptor)
            mpImplementation->SetFontDescriptor(aFontDescriptor);
    }
    else if (rsPropertyName == gsTopPropertyName)
    {
        sal_Int32 nTop = 0;
        if (rValue >>= nTop)
            mpImplementation->SetTop(nTop);
    }
    else if (rsPropertyName == gsTopRelativePropertyName)
    {
        OUString sDistance;
        if (rValue >>= sDistance)
            mpImplementation->SetTop(
                mpImplementation->GetTop()
                    + mpImplementation->ParseDistance(sDistance));
    }
    return aOldValue;
}

void PresenterTextView::ThrowIfDisposed()
{
    if (PresenterTextViewInterfaceBase::rBHelper.bDisposed
        || PresenterTextViewInterfaceBase::rBHelper.bInDispose || mpImplementation == nullptr)
    {
        throw lang::DisposedException ("PresenterTextView object has already been disposed",
            static_cast<uno::XWeak*>(this));
    }
}

// PresenterTextView::Implementation
PresenterTextView::Implementation::Implementation()
    : mxBitmap(),
      mpCanvas(),
      mpOutputDevice(VclPtr<VirtualDevice>::Create(*Application::GetDefaultDevice(), DeviceFormat::DEFAULT, DeviceFormat::DEFAULT)),
      mpEditEngineItemPool(EditEngine::CreatePool()),
      maSize(100,100),
      msText(),
      mnTop(0),
      mnTotalHeight(-1)
{
    mpOutputDevice->SetMapMode(MapMode(MapUnit::MapPixel));

    // set fonts to be used
    SvtLinguOptions aOpt;
    SvtLinguConfig().GetOptions( aOpt );

    struct FontDta {
        LanguageType    nFallbackLang;
        LanguageType    nLang;
        DefaultFontType nFontType;
        sal_uInt16      nFontInfoId;
        } aTable[3] =
    {
        // info to get western font to be used
        {   LANGUAGE_ENGLISH_US,    LANGUAGE_NONE,
            DefaultFontType::SERIF,      EE_CHAR_FONTINFO },
        // info to get CJK font to be used
        {   LANGUAGE_JAPANESE,      LANGUAGE_NONE,
            DefaultFontType::CJK_TEXT,   EE_CHAR_FONTINFO_CJK },
        // info to get CTL font to be used
        {   LANGUAGE_ARABIC_SAUDI_ARABIA,  LANGUAGE_NONE,
            DefaultFontType::CTL_TEXT,   EE_CHAR_FONTINFO_CTL }
    };
    aTable[0].nLang = MsLangId::resolveSystemLanguageByScriptType(aOpt.nDefaultLanguage, css::i18n::ScriptType::LATIN);
    aTable[1].nLang = MsLangId::resolveSystemLanguageByScriptType(aOpt.nDefaultLanguage_CJK, css::i18n::ScriptType::ASIAN);
    aTable[2].nLang = MsLangId::resolveSystemLanguageByScriptType(aOpt.nDefaultLanguage_CTL, css::i18n::ScriptType::COMPLEX);

    for (FontDta & rFntDta : aTable)
    {
        LanguageType nLang = (LANGUAGE_NONE == rFntDta.nLang) ?
            rFntDta.nFallbackLang : rFntDta.nLang;
        vcl::Font aFont = OutputDevice::GetDefaultFont(
            rFntDta.nFontType, nLang, GetDefaultFontFlags::OnlyOne);
        mpEditEngineItemPool->SetPoolDefaultItem(
            SvxFontItem(
                aFont.GetFamilyType(),
                aFont.GetFamilyName(),
                aFont.GetStyleName(),
                aFont.GetPitch(),
                aFont.GetCharSet(),
                rFntDta.nFontInfoId));
    }

    mpEditEngine.reset( new EditEngine (mpEditEngineItemPool) );

    mpEditEngine->EnableUndo (true);
    mpEditEngine->SetDefTab (sal_uInt16(
        Application::GetDefaultDevice()->GetTextWidth("XXXX")));

    mpEditEngine->SetControlWord(
            EEControlBits(mpEditEngine->GetControlWord() | EEControlBits::AUTOINDENTING) &
            EEControlBits(~EEControlBits::UNDOATTRIBS) &
            EEControlBits(~EEControlBits::PASTESPECIAL) );

    mpEditEngine->SetWordDelimiters (" .=+-*/(){}[];\"");
    mpEditEngine->SetRefMapMode(MapMode(MapUnit::MapPixel));
    mpEditEngine->SetPaperSize (Size(800, 0));
    mpEditEngine->EraseVirtualDevice();
    mpEditEngine->ClearModifyFlag();
}

PresenterTextView::Implementation::~Implementation()
{
    mpEditEngine.reset();
    SfxItemPool::Free(mpEditEngineItemPool);
    mpOutputDevice.disposeAndClear();
}

void PresenterTextView::Implementation::SetCanvas (const cppcanvas::CanvasSharedPtr& rpCanvas)
{
    mpCanvas = rpCanvas;
    mxBitmap = nullptr;
}

void PresenterTextView::Implementation::SetSize (const Size aSize)
{
    DBG_ASSERT(mpEditEngine!=nullptr, "EditEngine missing");

    maSize = aSize;
    mpEditEngine->SetPaperSize(maSize);
    mnTotalHeight = -1;
    mxBitmap = nullptr;
}

void PresenterTextView::Implementation::SetBackgroundColor (const Color aColor)
{
    mxBitmap = nullptr;

    DBG_ASSERT(mpEditEngine!=nullptr, "EditEngine missing");
    DBG_ASSERT(mpEditEngineItemPool!=nullptr, "EditEngineItemPool missing");
    mpEditEngine->SetBackgroundColor(aColor);
    mpEditEngine->EnableAutoColor(false);
    mpEditEngine->ForceAutoColor(false);
}

void PresenterTextView::Implementation::SetTextColor (const Color aColor)
{
    mxBitmap = nullptr;

    DBG_ASSERT(mpEditEngineItemPool!=nullptr, "EditEngineItemPool missing");
    mpEditEngineItemPool->SetPoolDefaultItem(SvxColorItem(aColor, EE_CHAR_COLOR));
}

void PresenterTextView::Implementation::SetFontDescriptor (
    const awt::FontDescriptor& rFontDescriptor)
{
    mxBitmap = nullptr;

    DBG_ASSERT(mpEditEngineItemPool!=nullptr, "EditEngineItemPool missing");

    const sal_Int32 nFontHeight = rFontDescriptor.Height;

    SvxFontHeightItem aFontHeight(
        Application::GetDefaultDevice()->LogicToPixel(
            Size(0, nFontHeight), MapMode (MapUnit::MapPoint)).Height(),
        100,
        EE_CHAR_FONTHEIGHT);
    mpEditEngineItemPool->SetPoolDefaultItem( aFontHeight);
    aFontHeight.SetWhich (EE_CHAR_FONTHEIGHT_CJK);
    mpEditEngineItemPool->SetPoolDefaultItem( aFontHeight);
    aFontHeight.SetWhich (EE_CHAR_FONTHEIGHT_CTL);
    mpEditEngineItemPool->SetPoolDefaultItem( aFontHeight);

    SvxFontItem aSvxFontItem (EE_CHAR_FONTINFO);
    aSvxFontItem.SetFamilyName( rFontDescriptor.Name );
    mpEditEngineItemPool->SetPoolDefaultItem(aSvxFontItem);

    mnTotalHeight = -1;
    mxBitmap = nullptr;

    CheckTop();
    mnTotalHeight = -1;
}

void PresenterTextView::Implementation::SetTop (const sal_Int32 nTop)
{
    if (nTop == mnTop)
        return;

    mnTop = nTop;
    mxBitmap = nullptr;
    CheckTop();
}

void PresenterTextView::Implementation::SetText (const OUString& rText)
{
    DBG_ASSERT(mpEditEngine!=nullptr, "EditEngine missing");
    msText = rText;
    mpEditEngine->SetPaperSize(maSize);
    mnTotalHeight = -1;
    mxBitmap = nullptr;
}

sal_Int32 PresenterTextView::Implementation::ParseDistance (const OUString& rsDistance) const
{
    DBG_ASSERT(mpEditEngine!=nullptr, "EditEngine missing");
    sal_Int32 nDistance (0);
    if (rsDistance.endsWith("px"))
    {
        nDistance = rsDistance.copy(0,rsDistance.getLength()-2).toInt32();
    }
    else if (rsDistance.endsWith("l"))
    {
        const sal_Int32 nLines (rsDistance.copy(0,rsDistance.getLength()-1).toInt32());
        // Take the height of the first line as the height of every line.
        const sal_uInt32 nFirstLineHeight (mpEditEngine->GetLineHeight(0));
        nDistance = nFirstLineHeight * nLines;
    }

    return nDistance;
}

Reference<rendering::XBitmap> const & PresenterTextView::Implementation::GetBitmap()
{
    DBG_ASSERT(mpEditEngine!=nullptr, "EditEngine missing");

    if ( ! mxBitmap.is())
    {
        mpOutputDevice.disposeAndClear();
        mpOutputDevice = VclPtr<VirtualDevice>::Create(*Application::GetDefaultDevice(),
                                                       DeviceFormat::DEFAULT, DeviceFormat::DEFAULT);
        mpOutputDevice->SetMapMode(MapMode(MapUnit::MapPixel));
        mpOutputDevice->SetOutputSizePixel(maSize);
        mpOutputDevice->SetLineColor();
        mpOutputDevice->SetFillColor();
        mpOutputDevice->SetBackground(Wallpaper());
        mpOutputDevice->Erase();

        MapMode aMapMode (mpOutputDevice->GetMapMode());
        aMapMode.SetOrigin(Point(0,0));
        mpOutputDevice->SetMapMode(aMapMode);
        const ::tools::Rectangle aWindowBox (Point(0,0), maSize);
        mpOutputDevice->DrawRect(aWindowBox);

        mpEditEngine->Clear();
        mpEditEngine->SetText(msText);
        mpEditEngine->SetPaperSize(maSize);

        mpEditEngine->Draw(mpOutputDevice, aWindowBox, Point(0,mnTop));

        const BitmapEx aBitmap (mpOutputDevice->GetBitmapEx(Point(0,0), maSize));
        mxBitmap = cppcanvas::VCLFactory::createBitmap(
            mpCanvas,
            aBitmap
            )->getUNOBitmap();
    }
    return mxBitmap;
}

sal_Int32 PresenterTextView::Implementation::GetTotalHeight()
{
    DBG_ASSERT(mpEditEngine!=nullptr, "EditEngine missing");

    if (mnTotalHeight < 0)
    {
        if ( ! mxBitmap.is())
            GetBitmap();
        mnTotalHeight = mpEditEngine->GetTextHeight();
    }
    return mnTotalHeight;
}

void PresenterTextView::Implementation::CheckTop()
{
    DBG_ASSERT(mpEditEngine!=nullptr, "EditEngine missing");

    if (mpEditEngine!=nullptr && mnTotalHeight < 0)
        mnTotalHeight = mpEditEngine->GetTextHeight();
    if (mpEditEngine!=nullptr && mnTop >= mnTotalHeight)
        mnTop = mnTotalHeight - mpEditEngine->GetLineHeight(0);

    if (mnTotalHeight < maSize.Height())
        mnTop = 0;

    if (mnTotalHeight - mnTop < maSize.Height())
        mnTop = mnTotalHeight - maSize.Height();

    if (mnTop < 0)
        mnTop = 0;
}

} } // end of namespace ::sd::presenter


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_Draw_PresenterTextView_get_implementation(css::uno::XComponentContext*,
                                                            css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new sd::presenter::PresenterTextView);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
