/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: SlideSorterService.hxx,v $
 *
 * $Revision: 1.3 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include "SlsDebugDialog.hxx"
#include "view/SlideSorterView.hxx"
#include "view/SlsTheme.hxx"
#include "view/SlsPageObjectPainter.hxx"

#include <vcl/window.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/slider.hxx>
#include <boost/function.hpp>
#include <boost/bind.hpp>

namespace css = ::com::sun::star;
using namespace ::sd::slidesorter::view;

namespace sd { namespace slidesorter {

//===== TextButton ============================================================

class TextButton : public PushButton
{
public:
    typedef ::boost::function<void(TextButton*)> Action;

    TextButton (
        ::Window* pParent,
        const char* pText,
        const Rectangle& rBoundingBox,
        const Action& rAction)
        : PushButton(pParent),
          maAction(rAction)
    {
        SetText(::rtl::OUString::createFromAscii(pText));
        SetPosSizePixel(
            rBoundingBox.Top(), rBoundingBox.Left(),
            rBoundingBox.GetHeight(), rBoundingBox.GetWidth());
        Show();
    }

    virtual void Click (void)
    {
        if (maAction)
            maAction(this);
    }

private:
    Action maAction;
};




//===== ColorControl ==========================================================

class ColorControl
{
public:
    typedef ::boost::function<ColorData(void)> ColorGetter;
    typedef ::boost::function<void(ColorData)> ColorSetter;
    typedef ::boost::function<void(void)> Updater;
    ColorControl (
        ::Window* pParent,
        const char* pTitle,
        const Rectangle& rBoundingBox,
        const ColorGetter& rGetter,
        const ColorSetter& rSetter,
        const Updater& rUpdater)
        : maGetter(rGetter),
          maSetter(rSetter),
          maUpdater(rUpdater),
          mpContainer(new ::Window(pParent, WB_BORDER)),
          mpTitle(new FixedText(mpContainer)),
          mpRedSlider(new Slider(mpContainer)),
          mpGreenSlider(new Slider(mpContainer)),
          mpBlueSlider(new Slider(mpContainer)),
          mpTextValue(new FixedText(mpContainer)),
          mpColorValue(new ::Window(mpContainer))
    {
        const double nWidth (rBoundingBox.GetWidth());
        const double nLeft (0);
        const double nTop (0);
        const ColorData aStartColor (rGetter ? rGetter() : 0);

        mpContainer->SetPosSizePixel(rBoundingBox.TopLeft(), rBoundingBox.GetSize());
        mpContainer->Show();

        mpTitle->SetText(::rtl::OUString::createFromAscii(pTitle));
        mpTitle->SetPosSizePixel(nLeft, nTop, nWidth,20);
        mpTitle->Show();

        mpRedSlider->SetPosSizePixel(nLeft, nTop+30, nWidth, 10);
        mpRedSlider->SetRange(Range(0,255));
        mpRedSlider->Show();

        mpGreenSlider->SetPosSizePixel(nLeft, nTop+40, nWidth, 10);
        mpGreenSlider->SetRange(Range(0,255));
        mpGreenSlider->Show();

        mpBlueSlider->SetPosSizePixel(nLeft, nTop+50, nWidth, 10);
        mpBlueSlider->SetRange(Range(0,255));
        mpBlueSlider->Show();

        SetColor(aStartColor);

        mpRedSlider->SetSlideHdl(LINK(this, ColorControl, UpdateColor));
        mpGreenSlider->SetSlideHdl(LINK(this, ColorControl, UpdateColor));
        mpBlueSlider->SetSlideHdl(LINK(this, ColorControl, UpdateColor));

        mpTextValue->SetText(::rtl::OUString::createFromAscii("x000000"));
        mpTextValue->SetPosSizePixel(nLeft, nTop+70, 150, 20);
        mpTextValue->Show();

        mpColorValue->SetPosSizePixel(nLeft + 150, nTop+70, nWidth - 160, 20);
        mpColorValue->SetBackground(Wallpaper(Color(aStartColor)));
        mpColorValue->Show();

        UpdateDisplay();
    }

    void SetColor (const ColorData aColorData)
    {
        const Color aColor (aColorData);
        mpRedSlider->SetThumbPos(aColor.GetRed());
        mpGreenSlider->SetThumbPos(aColor.GetGreen());
        mpBlueSlider->SetThumbPos(aColor.GetBlue());
        UpdateDisplay();
    }


    ColorData GetColor (void) const
    {
        return Color(
            mpRedSlider->GetThumbPos(),
            mpGreenSlider->GetThumbPos(),
            mpBlueSlider->GetThumbPos()).GetColor();
    }

    DECL_LINK(UpdateColor, void*);

private:
    ColorGetter maGetter;
    ColorSetter maSetter;
    Updater maUpdater;
    ::Window* mpContainer;
    FixedText* mpTitle;
    Slider* mpRedSlider;
    Slider* mpGreenSlider;
    Slider* mpBlueSlider;
    FixedText* mpTextValue;
    ::Window* mpColorValue;

    Color UpdateDisplay (void)
    {
        const int nRed (mpRedSlider->GetThumbPos());
        const int nGreen (mpGreenSlider->GetThumbPos());
        const int nBlue (mpBlueSlider->GetThumbPos());

        Color aColor (nRed, nGreen, nBlue);
        mpColorValue->SetBackground(Wallpaper(aColor));
        mpColorValue->Invalidate();

        const int nMaxLength(10);
        char aBuffer[nMaxLength];
        snprintf(aBuffer, nMaxLength, "x%02x%02x%02x", nRed,nGreen,nBlue);
        mpTextValue->SetText(::rtl::OUString::createFromAscii(aBuffer));

        return aColor;
    }

};

IMPL_LINK(ColorControl, UpdateColor, void*, EMPTYARG)
{
    const Color aColor (UpdateDisplay());
    if (maSetter)
    {
        maSetter(aColor.GetColor());
        if (maUpdater)
            maUpdater();
    }

    return 0;
}




//===== GradientControl =======================================================

class GradientControl
{
public:
    typedef ::boost::function<void(void)> Updater;

    GradientControl (
        ::Window* pParent,
        const Theme::GradientColorType eType,
        const Rectangle& rBoundingBox,
        SlideSorter& rSlideSorter)
        : mpTheme(rSlideSorter.GetTheme()),
          mrSlideSorter(rSlideSorter),
          meType(eType),
          mpContainer(new ::Window(pParent, WB_BORDER)),
          mpColorControl(new ColorControl(mpContainer, "Base Color",
                  Rectangle(0,0, rBoundingBox.GetWidth()-4, 90),
                  ::boost::bind(&Theme::GetGradientColor, mpTheme,
                      ::boost::bind(&GradientControl::GetType, this), Theme::Base),
                  ::boost::bind(&GradientControl::SetBaseColor, this, _1),
                  ::boost::bind(&GradientControl::Update, this, (void*)0))),
          mpFillOffset1Slider(new Slider(mpContainer)),
          mpFillOffset1Text(new FixedText(mpContainer)),
          mpFillOffset1Color(new ::Window(mpContainer)),
          mpFillOffset2Slider(new Slider(mpContainer)),
          mpFillOffset2Text(new FixedText(mpContainer)),
          mpFillOffset2Color(new ::Window(mpContainer)),
          mpBorderOffset1Slider(new Slider(mpContainer)),
          mpBorderOffset1Text(new FixedText(mpContainer)),
          mpBorderOffset1Color(new ::Window(mpContainer)),
          mpBorderOffset2Slider(new Slider(mpContainer)),
          mpBorderOffset2Text(new FixedText(mpContainer)),
          mpBorderOffset2Color(new ::Window(mpContainer))
    {
        const double nWidth (rBoundingBox.GetWidth());
        ::boost::shared_ptr<Theme> pTheme (mrSlideSorter.GetTheme());

        mpContainer->SetPosSizePixel(rBoundingBox.TopLeft(), rBoundingBox.GetSize());
        mpContainer->Show();

        Initialize(mpFillOffset1Slider, mpFillOffset1Text, mpFillOffset1Color,
            mpTheme->GetGradientOffset(eType, Theme::Fill1), 100, nWidth);
        Initialize(mpFillOffset2Slider, mpFillOffset2Text, mpFillOffset2Color,
            mpTheme->GetGradientOffset(eType, Theme::Fill2), 120, nWidth);
        Initialize(mpBorderOffset1Slider, mpBorderOffset1Text, mpBorderOffset1Color,
            mpTheme->GetGradientOffset(eType, Theme::Border1), 140, nWidth);
        Initialize(mpBorderOffset2Slider, mpBorderOffset2Text, mpBorderOffset2Color,
            mpTheme->GetGradientOffset(eType, Theme::Border2), 160, nWidth);

        Update(0);
    }

    void Initialize (
        Slider* pSlider,
        FixedText* pText,
        ::Window* pWindow,
        const sal_Int32 nOffset,
        const sal_Int32 nY,
        const sal_Int32 nWidth)
    {
        pSlider->SetPosSizePixel(10,nY,nWidth/2,15);
        pSlider->Show();
        pSlider->SetRange(Range(-100,+100));
        pSlider->SetThumbPos(nOffset);
        pSlider->SetSlideHdl(LINK(this, GradientControl, Update));

        pText->SetPosSizePixel(nWidth/2+10, nY, nWidth/2-60, 15);
        pText->Show();

        pWindow->SetPosSizePixel(nWidth-40, nY, 40, 15);
        pWindow->Show();
    }

    void SetBaseColor (const ColorData aColor)
    {
        mpColorControl->SetColor(aColor);
    }

    ::Window* GetContainer (void)
    {
        return mpContainer;
    }

    Theme::GradientColorType GetType (void) const
    {
        return meType;
    }

    void SetType (const Theme::GradientColorType eType)
    {
        meType = eType;
        mpColorControl->SetColor(mpTheme->GetGradientColor(meType, Theme::Base));
        mpFillOffset1Slider->SetThumbPos(mpTheme->GetGradientOffset(meType, Theme::Fill1));
        mpFillOffset2Slider->SetThumbPos(mpTheme->GetGradientOffset(meType, Theme::Fill2));
        mpBorderOffset1Slider->SetThumbPos(mpTheme->GetGradientOffset(meType, Theme::Border1));
        mpBorderOffset2Slider->SetThumbPos(mpTheme->GetGradientOffset(meType, Theme::Border2));
        UpdateDisplay();
    }

private:
    ::boost::shared_ptr<view::Theme> mpTheme;
    SlideSorter& mrSlideSorter;
    Theme::GradientColorType meType;
    ::Window* mpContainer;
    ColorControl* mpColorControl;
    Slider* mpFillOffset1Slider;
    FixedText* mpFillOffset1Text;
    ::Window* mpFillOffset1Color;
    sal_Int32 mnFillOffset1;
    Slider* mpFillOffset2Slider;
    FixedText* mpFillOffset2Text;
    ::Window* mpFillOffset2Color;
    sal_Int32 mnFillOffset2;
    Slider* mpBorderOffset1Slider;
    FixedText* mpBorderOffset1Text;
    ::Window* mpBorderOffset1Color;
    sal_Int32 mnBorderOffset1;
    Slider* mpBorderOffset2Slider;
    FixedText* mpBorderOffset2Text;
    ::Window* mpBorderOffset2Color;
    sal_Int32 mnBorderOffset2;

    DECL_LINK(Update, void*);

    void UpdateDisplay (void)
    {
        const sal_Int32 nFillOffset1 (mpFillOffset1Slider->GetThumbPos());
        const sal_Int32 nFillOffset2 (mpFillOffset2Slider->GetThumbPos());
        const sal_Int32 nBorderOffset1 (mpBorderOffset1Slider->GetThumbPos());
        const sal_Int32 nBorderOffset2 (mpBorderOffset2Slider->GetThumbPos());

        mpFillOffset1Text->SetText(::rtl::OUString::valueOf(nFillOffset1));
        mpFillOffset1Color->SetBackground(Wallpaper(
            mpTheme->GetGradientColor(meType, Theme::Fill1)));
        mpFillOffset1Color->Invalidate();

        mpFillOffset2Text->SetText(::rtl::OUString::valueOf(nFillOffset2));
        mpFillOffset2Color->SetBackground(Wallpaper(
            mpTheme->GetGradientColor(meType, Theme::Fill2)));
        mpFillOffset2Color->Invalidate();

        mpBorderOffset1Text->SetText(::rtl::OUString::valueOf(nBorderOffset1));
        mpBorderOffset1Color->SetBackground(Wallpaper(
            mpTheme->GetGradientColor(meType, Theme::Border1)));
        mpBorderOffset1Color->Invalidate();

        mpBorderOffset2Text->SetText(::rtl::OUString::valueOf(nBorderOffset2));
        mpBorderOffset2Color->SetBackground(Wallpaper(
            mpTheme->GetGradientColor(meType, Theme::Border2)));
        mpBorderOffset2Color->Invalidate();

        mrSlideSorter.GetView().GetPageObjectPainter()->NotifyResize(true);
        mrSlideSorter.GetView().RequestRepaint();
    }
};


IMPL_LINK(GradientControl, Update, void*, EMPTYARG)
{
    UpdateDisplay();


    mpTheme->SetGradient(meType,
        mpColorControl->GetColor(),
        mpFillOffset1Slider->GetThumbPos(),
        mpFillOffset2Slider->GetThumbPos(),
        mpBorderOffset1Slider->GetThumbPos(),
        mpBorderOffset2Slider->GetThumbPos());

    return 0;
}




//===== SliderControl =========================================================

class SliderControl
{
public:
    typedef ::boost::function<sal_Int32(void)> ValueGetter;
    typedef ::boost::function<void(sal_Int32)> ValueSetter;
    typedef ::boost::function<void(void)> Updater;
    SliderControl (
        ::Window* pParent,
        const char* pTitle,
        const Rectangle& rBoundingBox,
        const Range aRange,
        const ValueGetter& rGetter,
        const ValueSetter& rSetter,
        const Updater& rUpdater)
        : maGetter(rGetter),
          maSetter(rSetter),
          maUpdater(rUpdater),
          mpContainer(new ::Window(pParent, WB_BORDER)),
          mpTitle(new FixedText(mpContainer)),
          mpSlider(new Slider(mpContainer)),
          mpTextValue(new FixedText(mpContainer))
    {
        const double nWidth (rBoundingBox.GetWidth());
        const double nLeft (0);
        const double nTop (0);
        const sal_Int32 nStartValue (maGetter ? maGetter() : 0);

        mpContainer->SetPosSizePixel(rBoundingBox.TopLeft(), rBoundingBox.GetSize());
        mpContainer->Show();

        mpTitle->SetText(::rtl::OUString::createFromAscii(pTitle));
        mpTitle->SetPosSizePixel(nLeft, nTop, nWidth,20);
        mpTitle->Show();

        mpSlider->SetPosSizePixel(nLeft, nTop+30, nWidth, 10);
        mpSlider->SetRange(aRange);
        mpSlider->SetSlideHdl(LINK(this, SliderControl, UpdateValue));
        mpSlider->SetThumbPos(nStartValue);
        mpSlider->Show();

        mpTextValue->SetText(::rtl::OUString::valueOf(nStartValue));
        mpTextValue->SetPosSizePixel(nLeft, nTop+50, 150, 20);
        mpTextValue->Show();

        UpdateValue(0);
    }

private:
    ValueGetter maGetter;
    ValueSetter maSetter;
    Updater maUpdater;
    ::Window* mpContainer;
    FixedText* mpTitle;
    Slider* mpSlider;
    FixedText* mpTextValue;

    DECL_LINK(UpdateValue, void*);
};

IMPL_LINK(SliderControl, UpdateValue, void*, EMPTYARG)
{
    const sal_Int32 nValue (mpSlider->GetThumbPos());

    const sal_Int32 nMaxLength (30);
    char aBuffer[nMaxLength];
    snprintf(aBuffer, nMaxLength, "%ld  %f", nValue, nValue/255.0);
    mpTextValue->SetText(::rtl::OUString::createFromAscii(aBuffer));

    if (maSetter)
    {
        maSetter(nValue);
        if (maUpdater)
            maUpdater();
    }

    return 0;
}




//===== ChoiceControl =========================================================

class ChoiceControl
{
public:
    typedef ::boost::function<Theme::GradientColorType(void)> ValueGetter;
    typedef ::boost::function<void(Theme::GradientColorType)> ValueSetter;
    ChoiceControl (
        ::Window* pParent,
        const Rectangle& rBoundingBox,
        const char** aValues,
        const int nValueCount,
        GradientControl* pSubControl,
        const ValueGetter& rGetter,
        const ValueSetter& rSetter)
        : maValues(),
          maGetter(rGetter),
          maSetter(rSetter),
          mpContainer(new ::Window(pParent, WB_BORDER)),
          mpComboBox(new ComboBox(mpContainer, WB_DROPDOWN)),
          mpSubControl(pSubControl)
    {
        const double nWidth (rBoundingBox.GetWidth());
        const double nLeft (0);
        const double nTop (0);

        mpContainer->SetPosSizePixel(rBoundingBox.TopLeft(), rBoundingBox.GetSize());
        mpContainer->Show();

        mpComboBox->SetPosSizePixel(nLeft, nTop, nWidth-5, 20);
        mpComboBox->ToggleDropDown();
        mpComboBox->SetSelectHdl(LINK(this, ChoiceControl, UpdateValue));
        mpComboBox->SetDoubleClickHdl(LINK(this, ChoiceControl, UpdateValue));
        for (int nIndex=0; nIndex<nValueCount; ++nIndex)
        {
            const USHORT nId (
                mpComboBox->InsertEntry(::rtl::OUString::createFromAscii(aValues[nIndex])));
            maValues[nId] = nIndex;
        }
        mpComboBox->Show();

        mpSubControl->GetContainer()->SetParent(mpContainer);
        mpSubControl->GetContainer()->SetPosSizePixel(
            nLeft, nTop+20, nWidth, rBoundingBox.GetHeight()-20);

        UpdateValue(0);
    }

private:
    ::std::map<USHORT, int> maValues;
    ValueGetter maGetter;
    ValueSetter maSetter;
    ::Window* mpContainer;
    ComboBox* mpComboBox;
    GradientControl* mpSubControl;

    DECL_LINK(UpdateValue, void*);
};

IMPL_LINK(ChoiceControl, UpdateValue, void*, EMPTYARG)
{
    USHORT nId (mpComboBox->GetSelectEntryPos());
    const sal_Int32 nIndex (maValues[nId]);
    const Theme::GradientColorType nValue ((Theme::GradientColorType)nIndex);

    if (maSetter)
        maSetter(nValue);

    return 0;
}




//===== BoolControl ===========================================================

class BoolControl
{
public:
    typedef ::boost::function<sal_Int32(void)> ValueGetter;
    typedef ::boost::function<void(sal_Int32)> ValueSetter;
    typedef ::boost::function<void(void)> Updater;
    BoolControl (
        ::Window* pParent,
        const char* pTitle,
        const Rectangle& rBoundingBox,
        const sal_Int32 nOnValue,
        const sal_Int32 nOffValue,
        const ValueGetter& rGetter,
        const ValueSetter& rSetter,
        const Updater& rUpdater)
        : maGetter(rGetter),
          maSetter(rSetter),
          maUpdater(rUpdater),
          mpContainer(new ::Window(pParent, WB_BORDER)),
          mpButton(new CheckBox(mpContainer)),
          mnOnValue(nOnValue),
          mnOffValue(nOffValue)
    {
        const double nWidth (rBoundingBox.GetWidth());
        const double nLeft (0);
        const double nTop (0);

        mpContainer->SetPosSizePixel(rBoundingBox.TopLeft(), rBoundingBox.GetSize());
        mpContainer->Show();

        mpButton->SetText(::rtl::OUString::createFromAscii(pTitle));
        mpButton->SetPosSizePixel(nLeft, nTop, nWidth,20);
        mpButton->Check(maGetter() == mnOnValue ? TRUE : FALSE);
        mpButton->SetToggleHdl(LINK(this, BoolControl, UpdateValue));
        mpButton->Show();
    }

private:
    ValueGetter maGetter;
    ValueSetter maSetter;
    Updater maUpdater;
    ::Window* mpContainer;
    CheckBox* mpButton;
    const sal_Int32 mnOnValue;
    const sal_Int32 mnOffValue;

    DECL_LINK(UpdateValue, void*);
};

IMPL_LINK(BoolControl, UpdateValue, void*, EMPTYARG)
{
    const bool bValue (mpButton->IsChecked());

    if (maSetter)
    {
        maSetter(bValue ? mnOnValue : mnOffValue);
        if (maUpdater)
            maUpdater();
    }

    return 0;
}




//===== SlideSorterDebugDialog ================================================

SlideSorterDebugDialog* SlideSorterDebugDialog::CreateDebugDialog (SlideSorter& rSlideSorter)
{
    static const char* pText = getenv("SD_SHOW_DEBUG_DIALOG");
    if (pText!=NULL && pText[0]!='0')
        return new SlideSorterDebugDialog(rSlideSorter);
    else
        return NULL;
}




SlideSorterDebugDialog::SlideSorterDebugDialog (SlideSorter& rSlideSorter)
    : mpTopLevelWindow(new WorkWindow(NULL, WB_STDWORK))
{
    ::boost::shared_ptr<view::Theme> pTheme(rSlideSorter.GetTheme());

    mpTopLevelWindow->SetSizePixel(Size(300,510));

    new TextButton(
        mpTopLevelWindow,
        "Close",
        Rectangle(195,480,100,25),
        ::boost::bind(&WorkWindow::Close, mpTopLevelWindow));

    new ColorControl(
        mpTopLevelWindow,
        "Unhide Button Background",
        Rectangle(10,10,290,110),
        ::boost::bind(&view::Theme::GetColor, pTheme, view::Theme::ButtonBackground),
        ::boost::bind(&view::Theme::SetColor, pTheme, view::Theme::ButtonBackground, _1),
        ::boost::bind(&view::SlideSorterView::RequestRepaint,
            ::boost::ref(rSlideSorter.GetView())));

    new SliderControl(
        mpTopLevelWindow,
        "Max Button Alpha",
        Rectangle(10,120,290,200),
        Range(0,255),
        ::boost::bind(&view::Theme::GetIntegerValue, pTheme, view::Theme::ButtonMaxAlpha),
        ::boost::bind(&view::Theme::SetIntegerValue, pTheme, view::Theme::ButtonMaxAlpha, _1),
        ::boost::bind(&view::SlideSorterView::RequestRepaint,
            ::boost::ref(rSlideSorter.GetView())));

    GradientControl* pControl = new GradientControl(
        mpTopLevelWindow,
        Theme::SelectedPage,
        Rectangle(10,210,285,450),
        rSlideSorter);
    const char* aValues[] = {
        "Normal",
        "Selected",
        "SelectedAndFocused",
        "MouseOver"
    };
    new ChoiceControl(
        mpTopLevelWindow,
        Rectangle(10,210,290,470),
        aValues,
        4,
        pControl,
        ::boost::bind(&GradientControl::GetType, pControl),
        ::boost::bind(&GradientControl::SetType, pControl, _1));

    new BoolControl(
        mpTopLevelWindow,
        "Alternative Button Paint Style",
        Rectangle(10,480,290,500),
        1,0,
        ::boost::bind(&view::Theme::GetIntegerValue, pTheme, view::Theme::ButtonPaintType),
        ::boost::bind(&view::Theme::SetIntegerValue, pTheme, view::Theme::ButtonPaintType, _1),
        ::boost::bind(&view::SlideSorterView::RequestRepaint,
            ::boost::ref(rSlideSorter.GetView())));

    mpTopLevelWindow->Show(true);
}




SlideSorterDebugDialog::~SlideSorterDebugDialog (void)
{
    DeleteWindow(mpTopLevelWindow);
}




void SlideSorterDebugDialog::DeleteWindow (::Window* pWindow)
{
    if (pWindow == NULL)
        return;

    while (pWindow->GetChildCount() > 0)
        DeleteWindow(pWindow->GetChild(0));

    delete pWindow;
}


} } // end of namespace ::sd::slidesorter
