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
#include <com/sun/star/util/Color.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;

namespace sd { namespace presenter {

//===== Service ===============================================================

Reference<XInterface> SAL_CALL PresenterTextViewService_createInstance (
    const Reference<XComponentContext>& rxContext)
{
    return Reference<XInterface>(static_cast<XWeak*>(new PresenterTextView(rxContext)));
}




OUString PresenterTextViewService_getImplementationName (void) throw(RuntimeException)
{
    return OUString("com.sun.star.comp.Draw.PresenterTextView");
}




Sequence<OUString> SAL_CALL PresenterTextViewService_getSupportedServiceNames (void)
    throw (RuntimeException)
{
    static const OUString sServiceName("com.sun.star.drawing.PresenterTextView");
    return Sequence<OUString>(&sServiceName, 1);
}



//===== PresenterTextView::Implementation =====================================

class PresenterTextView::Implementation
{
public:
    const OUString msTextPropertyName;
    const OUString msBitmapPropertyName;
    const OUString msSizePropertyName;
    const OUString msBackgroundColorPropertyName;
    const OUString msTextColorPropertyName;
    const OUString msFontDescriptorPropertyName;
    const OUString msTopPropertyName;
    const OUString msTopRelativePropertyName;
    const OUString msTotalHeightPropertyName;

    Implementation (void);
    ~Implementation (void);

    void SetCanvas (const cppcanvas::CanvasSharedPtr& rCanvas);
    void SetSize (const Size aSize);
    void SetBackgroundColor (const Color aColor);
    void SetTextColor (const Color aColor);
    void SetFontDescriptor (const awt::FontDescriptor& rFontDescriptor);
    sal_Int32 GetTop (void) const;
    void SetTop (const sal_Int32 nTop);
    void SetText (const OUString& Text);
    sal_Int32 ParseDistance (const OUString& rsDistance) const;
    Reference<rendering::XBitmap> GetBitmap (void);
    sal_Int32 GetTotalHeight (void);

private:
    Reference<rendering::XBitmap> mxBitmap;
    cppcanvas::CanvasSharedPtr mpCanvas;
    VirtualDevice* mpOutputDevice;
    EditEngine* mpEditEngine;
    SfxItemPool* mpEditEngineItemPool;
    Size maSize;
    Color maBackgroundColor;
    Color maTextColor;
    String msText;
    sal_Int32 mnTop;
    sal_Int32 mnTotalHeight;

    EditEngine * GetEditEngine (void);
    EditEngine* CreateEditEngine (void);
    void CheckTop (void);
};




//===== PresenterTextView =====================================================

PresenterTextView::PresenterTextView (const Reference<XComponentContext>& rxContext)
    : PresenterTextViewInterfaceBase(),
      mpImplementation(new Implementation())
{
    (void)rxContext;
}




PresenterTextView::~PresenterTextView (void)
{
}




void SAL_CALL PresenterTextView::disposing (void)
{
    mpImplementation.reset();
}




//----- XInitialization -------------------------------------------------------

void SAL_CALL PresenterTextView::initialize (const Sequence<Any>& rArguments)
    throw (Exception, RuntimeException)
{
    ThrowIfDisposed();

    if (rArguments.getLength() == 1)
    {
        try
        {
            Reference<rendering::XBitmapCanvas> xCanvas (rArguments[0], UNO_QUERY_THROW);
            if (xCanvas.is())
            {
                mpImplementation->SetCanvas(
                    cppcanvas::VCLFactory::getInstance().createCanvas(xCanvas));
            }
        }
        catch (RuntimeException&)
        {
            throw;
        }
    }
    else
    {
        throw RuntimeException("PresenterTextView: invalid number of arguments",
                static_cast<XWeak*>(this));
    }
}




//-----------------------------------------------------------------------------

Any PresenterTextView::GetPropertyValue (const OUString& rsPropertyName)
{
    ThrowIfDisposed();

    if (rsPropertyName == mpImplementation->msBitmapPropertyName)
    {
        return Any(mpImplementation->GetBitmap());
    }
    else if (rsPropertyName == mpImplementation->msTopPropertyName)
    {
        return Any(mpImplementation->GetTop());
    }
    else if (rsPropertyName == mpImplementation->msTotalHeightPropertyName)
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
    if (rsPropertyName == mpImplementation->msTextPropertyName)
    {
        OUString sText;
        if (rValue >>= sText)
            mpImplementation->SetText(sText);
    }
    else if (rsPropertyName == mpImplementation->msSizePropertyName)
    {
        awt::Size aSize;
        if (rValue >>= aSize)
            mpImplementation->SetSize(Size(aSize.Width,aSize.Height));
    }
    else if (rsPropertyName == mpImplementation->msBackgroundColorPropertyName)
    {
        util::Color aColor = util::Color();
        if (rValue >>= aColor)
            mpImplementation->SetBackgroundColor(Color(aColor));
    }
    else if (rsPropertyName == mpImplementation->msTextColorPropertyName)
    {
        util::Color aColor = util::Color();
        if (rValue >>= aColor)
            mpImplementation->SetTextColor(Color(aColor));
    }
    else if (rsPropertyName == mpImplementation->msFontDescriptorPropertyName)
    {
        awt::FontDescriptor aFontDescriptor;
        if (rValue >>= aFontDescriptor)
            mpImplementation->SetFontDescriptor(aFontDescriptor);
    }
    else if (rsPropertyName == mpImplementation->msTopPropertyName)
    {
        sal_Int32 nTop = 0;
        if (rValue >>= nTop)
            mpImplementation->SetTop(nTop);
    }
    else if (rsPropertyName == mpImplementation->msTopRelativePropertyName)
    {
        OUString sDistance;
        if (rValue >>= sDistance)
            mpImplementation->SetTop(
                mpImplementation->GetTop()
                    + mpImplementation->ParseDistance(sDistance));
    }
    return aOldValue;
}




void PresenterTextView::ThrowIfDisposed (void)
    throw (::com::sun::star::lang::DisposedException)
{
    if (PresenterTextViewInterfaceBase::rBHelper.bDisposed
        || PresenterTextViewInterfaceBase::rBHelper.bInDispose
        || mpImplementation.get()==NULL)
    {
        throw lang::DisposedException ("PresenterTextView object has already been disposed",
            static_cast<uno::XWeak*>(this));
    }
}




//===== PresenterTextView::Implementation =====================================

PresenterTextView::Implementation::Implementation (void)
    : msTextPropertyName("Text"),
      msBitmapPropertyName("Bitmap"),
      msSizePropertyName("Size"),
      msBackgroundColorPropertyName("BackgroundColor"),
      msTextColorPropertyName("TextColor"),
      msFontDescriptorPropertyName("FontDescriptor"),
      msTopPropertyName("Top"),
      msTopRelativePropertyName("RelativeTop"),
      msTotalHeightPropertyName("TotalHeight"),
      mxBitmap(),
      mpCanvas(),
      mpOutputDevice(new VirtualDevice(*Application::GetDefaultDevice(), 0, 0)),
      mpEditEngine(NULL),
      mpEditEngineItemPool(EditEngine::CreatePool()),
      maSize(100,100),
      maBackgroundColor(0xffffffff),
      maTextColor(0x00000000),
      msText(),
      mnTop(0),
      mnTotalHeight(-1)
{
    mpOutputDevice->SetMapMode(MAP_PIXEL);

    GetEditEngine();
}




PresenterTextView::Implementation::~Implementation (void)
{
    delete mpEditEngine;
    SfxItemPool::Free(mpEditEngineItemPool);
    delete mpOutputDevice;
}




EditEngine * PresenterTextView::Implementation::GetEditEngine (void)
{
    if (mpEditEngine == NULL)
        mpEditEngine = CreateEditEngine ();
    return mpEditEngine;
}




EditEngine* PresenterTextView::Implementation::CreateEditEngine (void)
{
    EditEngine* pEditEngine = mpEditEngine;
    if (pEditEngine == NULL)
    {
        //
        // set fonts to be used
        //
        SvtLinguOptions aOpt;
        SvtLinguConfig().GetOptions( aOpt );

        struct FontDta {
            sal_Int16       nFallbackLang;
            sal_Int16       nLang;
            sal_uInt16      nFontType;
            sal_uInt16      nFontInfoId;
            } aTable[3] =
        {
            // info to get western font to be used
            {   LANGUAGE_ENGLISH_US,    LANGUAGE_NONE,
                DEFAULTFONT_SERIF,      EE_CHAR_FONTINFO },
            // info to get CJK font to be used
            {   LANGUAGE_JAPANESE,      LANGUAGE_NONE,
                DEFAULTFONT_CJK_TEXT,   EE_CHAR_FONTINFO_CJK },
            // info to get CTL font to be used
            {   LANGUAGE_ARABIC_SAUDI_ARABIA,  LANGUAGE_NONE,
                DEFAULTFONT_CTL_TEXT,   EE_CHAR_FONTINFO_CTL }
        };
        aTable[0].nLang = MsLangId::resolveSystemLanguageByScriptType(aOpt.nDefaultLanguage, ::com::sun::star::i18n::ScriptType::LATIN);
        aTable[1].nLang = MsLangId::resolveSystemLanguageByScriptType(aOpt.nDefaultLanguage_CJK, ::com::sun::star::i18n::ScriptType::ASIAN);
        aTable[2].nLang = MsLangId::resolveSystemLanguageByScriptType(aOpt.nDefaultLanguage_CTL, ::com::sun::star::i18n::ScriptType::COMPLEX);

        for (int i = 0;  i < 3;  ++i)
        {
            const FontDta &rFntDta = aTable[i];
            LanguageType nLang = (LANGUAGE_NONE == rFntDta.nLang) ?
                rFntDta.nFallbackLang : rFntDta.nLang;
            Font aFont = Application::GetDefaultDevice()->GetDefaultFont(
                rFntDta.nFontType, nLang, DEFAULTFONT_FLAGS_ONLYONE);
            mpEditEngineItemPool->SetPoolDefaultItem(
                SvxFontItem(
                    aFont.GetFamily(),
                    aFont.GetName(),
                    aFont.GetStyleName(),
                    aFont.GetPitch(),
                    aFont.GetCharSet(),
                    rFntDta.nFontInfoId));
        }


        pEditEngine = new EditEngine (mpEditEngineItemPool);

        pEditEngine->EnableUndo (sal_True);
        pEditEngine->SetDefTab (sal_uInt16(
            Application::GetDefaultDevice()->GetTextWidth(OUString("XXXX"))));

        pEditEngine->SetControlWord(
                (pEditEngine->GetControlWord()
                    | EE_CNTRL_AUTOINDENTING) &
                (~EE_CNTRL_UNDOATTRIBS) &
                (~EE_CNTRL_PASTESPECIAL));

        pEditEngine->SetWordDelimiters (" .=+-*/(){}[];\"");
        pEditEngine->SetRefMapMode (MAP_PIXEL);
        pEditEngine->SetPaperSize (Size(800, 0));
        pEditEngine->EraseVirtualDevice();
        pEditEngine->ClearModifyFlag();
    }

    return pEditEngine;
}




void PresenterTextView::Implementation::SetCanvas (const cppcanvas::CanvasSharedPtr& rpCanvas)
{
    mpCanvas = rpCanvas;
    mxBitmap = NULL;
}




void PresenterTextView::Implementation::SetSize (const Size aSize)
{
    DBG_ASSERT(mpEditEngine!=NULL, "EditEngine missing");

    maSize = aSize;
    mpEditEngine->SetPaperSize(maSize);
    mnTotalHeight = -1;
    mxBitmap = NULL;
}




void PresenterTextView::Implementation::SetBackgroundColor (const Color aColor)
{
    maBackgroundColor = aColor;
    mxBitmap = NULL;

    DBG_ASSERT(mpEditEngine!=NULL, "EditEngine missing");
    DBG_ASSERT(mpEditEngineItemPool!=NULL, "EditEngineItemPool missing");
    mpEditEngine->SetBackgroundColor(aColor);
    mpEditEngine->EnableAutoColor(sal_False);
    mpEditEngine->ForceAutoColor(sal_False);
}




void PresenterTextView::Implementation::SetTextColor (const Color aColor)
{
    maTextColor = aColor;
    mxBitmap = NULL;

    DBG_ASSERT(mpEditEngineItemPool!=NULL, "EditEngineItemPool missing");
    mpEditEngineItemPool->SetPoolDefaultItem(SvxColorItem(aColor, EE_CHAR_COLOR));
}




void PresenterTextView::Implementation::SetFontDescriptor (
    const awt::FontDescriptor& rFontDescriptor)
{
    mxBitmap = NULL;

    DBG_ASSERT(mpEditEngineItemPool!=NULL, "EditEngineItemPool missing");

    const sal_Int32 nFontHeight = rFontDescriptor.Height;

    SvxFontHeightItem aFontHeight(
        Application::GetDefaultDevice()->LogicToPixel(
            Size(0, nFontHeight), MapMode (MAP_POINT)).Height(),
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
    mxBitmap = NULL;

    CheckTop();
    mnTotalHeight = -1;
}




sal_Int32 PresenterTextView::Implementation::GetTop (void) const
{
    return mnTop;
}




void PresenterTextView::Implementation::SetTop (const sal_Int32 nTop)
{
    if (nTop == mnTop)
        return;

    mnTop = nTop;
    mxBitmap = NULL;
    CheckTop();
}




void PresenterTextView::Implementation::SetText (const OUString& rText)
{
    DBG_ASSERT(mpEditEngine!=NULL, "EditEngine missing");
    msText = rText;
    mpEditEngine->SetPaperSize(maSize);
    mnTotalHeight = -1;
    mxBitmap = NULL;
}




sal_Int32 PresenterTextView::Implementation::ParseDistance (const OUString& rsDistance) const
{
    DBG_ASSERT(mpEditEngine!=NULL, "EditEngine missing");
    sal_Int32 nDistance (0);
    if (rsDistance.endsWithAsciiL("px", 2))
    {
        nDistance = rsDistance.copy(0,rsDistance.getLength()-2).toInt32();
    }
    else if (rsDistance.endsWithAsciiL("l", 1))
    {
        const sal_Int32 nLines (rsDistance.copy(0,rsDistance.getLength()-1).toInt32());
        // Take the height of the first line as the height of every line.
        const sal_uInt32 nFirstLineHeight (mpEditEngine->GetLineHeight(0,0));
        nDistance = nFirstLineHeight * nLines;
    }

    return nDistance;
}




Reference<rendering::XBitmap> PresenterTextView::Implementation::GetBitmap (void)
{
    DBG_ASSERT(mpEditEngine!=NULL, "EditEngine missing");

    if ( ! mxBitmap.is())
    {
        if (mpOutputDevice != NULL)
            delete mpOutputDevice;
        mpOutputDevice = new VirtualDevice(*Application::GetDefaultDevice(), 0, 0);
        mpOutputDevice->SetMapMode(MAP_PIXEL);
        mpOutputDevice->SetOutputSizePixel(maSize, sal_True);
        mpOutputDevice->SetLineColor();
        mpOutputDevice->SetFillColor();
        mpOutputDevice->SetBackground(Wallpaper());
        mpOutputDevice->Erase();

        MapMode aMapMode (mpOutputDevice->GetMapMode());
        aMapMode.SetOrigin(Point(0,0));
        mpOutputDevice->SetMapMode(aMapMode);
        const Rectangle aWindowBox (Point(0,0), maSize);
        mpOutputDevice->DrawRect(aWindowBox);

        mpEditEngine->Clear();
        mpEditEngine->SetText(msText);
        mpEditEngine->SetPaperSize(maSize);

        mpEditEngine->Draw(mpOutputDevice, aWindowBox, Point(0,mnTop));

        const BitmapEx aBitmap (mpOutputDevice->GetBitmapEx(Point(0,0), maSize));
        mxBitmap = cppcanvas::VCLFactory::getInstance().createBitmap(
            mpCanvas,
            aBitmap
            )->getUNOBitmap();
    }
    return mxBitmap;
}




sal_Int32 PresenterTextView::Implementation::GetTotalHeight (void)
{
    DBG_ASSERT(mpEditEngine!=NULL, "EditEngine missing");

    if (mnTotalHeight < 0)
    {
        if ( ! mxBitmap.is())
            GetBitmap();
        mnTotalHeight = mpEditEngine->GetTextHeight();
    }
    return mnTotalHeight;
}




void PresenterTextView::Implementation::CheckTop (void)
{
    DBG_ASSERT(mpEditEngine!=NULL, "EditEngine missing");

    if (mnTotalHeight < 0)
        mnTotalHeight = mpEditEngine->GetTextHeight();
    if (mpEditEngine!=NULL && mnTop >= mnTotalHeight)
        mnTop = mnTotalHeight - mpEditEngine->GetLineHeight(0,0);

    if (mnTotalHeight < maSize.Height())
        mnTop = 0;

    if (mnTotalHeight - mnTop < maSize.Height())
        mnTop = mnTotalHeight - maSize.Height();

    if (mnTop < 0)
        mnTop = 0;
}


} } // end of namespace ::sd::presenter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
