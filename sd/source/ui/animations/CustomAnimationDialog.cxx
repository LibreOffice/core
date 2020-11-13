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

#include <config_features.h>

#include <com/sun/star/presentation/EffectNodeType.hpp>
#include <com/sun/star/animations/Timing.hpp>
#include <com/sun/star/animations/Event.hpp>
#include <com/sun/star/animations/EventTrigger.hpp>
#include <com/sun/star/animations/AnimationFill.hpp>
#include <com/sun/star/presentation/TextAnimationType.hpp>
#include <com/sun/star/animations/ValuePair.hpp>
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/media/XPlayer.hpp>

#include <memory>

#include <comphelper/lok.hxx>
#include <i18nutil/unicode.hxx>
#include <vcl/svapp.hxx>
#include <vcl/stdtext.hxx>
#include <vcl/weld.hxx>
#include <vcl/settings.hxx>

#include <svtools/ctrlbox.hxx>
#include <svtools/ctrltool.hxx>
#include <sfx2/objsh.hxx>
#include <tools/debug.hxx>
#include <tools/urlobj.hxx>

#include <editeng/flstitem.hxx>

#include <svx/colorbox.hxx>
#include <svx/gallery.hxx>

#include <editeng/editids.hrc>
#include <sdresid.hxx>

#include "CustomAnimationDialog.hxx"
#include "CustomAnimationPane.hxx"
#include "STLPropertySet.hxx"
#include <CustomAnimationPreset.hxx>

#include <avmedia/mediawindow.hxx>

#include <filedlg.hxx>
#include <strings.hrc>
#include <helpids.h>

using namespace ::com::sun::star;
using namespace ::com::sun::star::animations;
using namespace ::com::sun::star::presentation;

using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::makeAny;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::drawing::XShape;
using ::com::sun::star::drawing::XDrawPage;
using ::com::sun::star::beans::XPropertySet;

namespace sd {

SdPropertySubControl::SdPropertySubControl(weld::Container* pParent)
    : mxBuilder(Application::CreateBuilder(pParent, "modules/simpress/ui/customanimationfragment.ui"))
    , mxContainer(mxBuilder->weld_container("EffectFragment"))
    , mpParent(pParent)
{
}

SdPropertySubControl::~SdPropertySubControl()
{
    mpParent->move(mxContainer.get(), nullptr);
}

namespace {

class SdPresetPropertyBox  : public SdPropertySubControl
{
public:
    SdPresetPropertyBox(weld::Label* pLabel, weld::Container* pParent, const Any& rValue, const OUString& aPresetId, const Link<LinkParamNone*,void>& rModifyHdl);

    virtual Any getValue() override;
    virtual void setValue( const Any& rValue, const OUString& rPresetId ) override;

private:
    std::vector<OUString> maPropertyValues;
    Link<LinkParamNone*,void> maModifyLink;
    std::unique_ptr<weld::ComboBox> mxControl;

    DECL_LINK(OnSelect, weld::ComboBox&, void);
};

}

SdPresetPropertyBox::SdPresetPropertyBox(weld::Label* pLabel, weld::Container* pParent, const Any& rValue, const OUString& aPresetId, const Link<LinkParamNone*,void>& rModifyHdl)
    : SdPropertySubControl(pParent)
    , maModifyLink(rModifyHdl)
    , mxControl(mxBuilder->weld_combo_box("combo"))
{
    mxControl->connect_changed(LINK(this, SdPresetPropertyBox, OnSelect));
    mxControl->set_help_id(HID_SD_CUSTOMANIMATIONPANE_PRESETPROPERTYBOX);
    mxControl->show();
    pLabel->set_mnemonic_widget(mxControl.get());
    setValue(rValue, aPresetId);
}

IMPL_LINK_NOARG(SdPresetPropertyBox, OnSelect, weld::ComboBox&, void)
{
    maModifyLink.Call(nullptr);
}

void SdPresetPropertyBox::setValue( const Any& rValue, const OUString& rPresetId )
{
    if (!mxControl)
        return;

    mxControl->freeze();
    mxControl->clear();
    maPropertyValues.clear();
    int nPos = -1;

    const CustomAnimationPresets& rPresets = CustomAnimationPresets::getCustomAnimationPresets();
    CustomAnimationPresetPtr pDescriptor = rPresets.getEffectDescriptor( rPresetId );
    if( pDescriptor )
    {

        OUString aPropertyValue;
        rValue >>= aPropertyValue;

        std::vector<OUString> aSubTypes( pDescriptor->getSubTypes() );

        mxControl->set_sensitive(!aSubTypes.empty());

        for( const auto& aSubType : aSubTypes )
        {
            mxControl->append_text(rPresets.getUINameForProperty(aSubType));
            maPropertyValues.push_back(aSubType);
            if (aSubType == aPropertyValue)
                nPos = maPropertyValues.size() - 1;
        }
    }
    else
    {
        mxControl->set_sensitive(false);
    }
    mxControl->thaw();
    if (nPos != -1)
        mxControl->set_active(nPos);
}

Any SdPresetPropertyBox::getValue()
{
    const int nIndex = mxControl->get_active();
    if (nIndex == -1)
        return Any();
    return makeAny(maPropertyValues[nIndex]);
}

namespace {

class SdColorPropertyBox : public SdPropertySubControl
{
public:
    SdColorPropertyBox(weld::Label* pLabel, weld::Container* pParent, weld::Window* pTopLevel, const Any& rValue, const Link<LinkParamNone*,void>& rModifyHdl);

    virtual Any getValue() override;
    virtual void setValue( const Any& rValue, const OUString& rPresetId  ) override;

private:
    Link<LinkParamNone*,void> maModifyLink;
    std::unique_ptr<ColorListBox> mxControl;

    DECL_LINK(OnSelect, ColorListBox&, void);
};

}

SdColorPropertyBox::SdColorPropertyBox(weld::Label* pLabel, weld::Container* pParent, weld::Window* pTopLevel, const Any& rValue, const Link<LinkParamNone*,void>& rModifyHdl)
    : SdPropertySubControl(pParent)
    , maModifyLink(rModifyHdl)
    , mxControl(new ColorListBox(mxBuilder->weld_menu_button("color"), pTopLevel))
{
    mxControl->SetSelectHdl(LINK(this, SdColorPropertyBox, OnSelect));
    mxControl->set_help_id(HID_SD_CUSTOMANIMATIONPANE_COLORPROPERTYBOX);
    pLabel->set_mnemonic_widget(&mxControl->get_widget());
    mxControl->show();

    sal_Int32 nColor = 0;
    rValue >>= nColor;
    mxControl->SelectEntry(Color(nColor));
}

IMPL_LINK_NOARG(SdColorPropertyBox, OnSelect, ColorListBox&, void)
{
    maModifyLink.Call(nullptr);
}

void SdColorPropertyBox::setValue( const Any& rValue, const OUString& )
{
    if (mxControl)
    {
        sal_Int32 nColor = 0;
        rValue >>= nColor;

        mxControl->SetNoSelection();
        mxControl->SelectEntry(Color(nColor));
    }
}

Any SdColorPropertyBox::getValue()
{
    return makeAny(sal_Int32(mxControl->GetSelectEntryColor().GetRGBColor()));
}

namespace {

class SdFontPropertyBox : public SdPropertySubControl
{
public:
    SdFontPropertyBox(weld::Label* pLabel, weld::Container* pParent, const Any& rValue, const Link<LinkParamNone*,void>& rModifyHdl);

    virtual Any getValue() override;
    virtual void setValue(const Any& rValue, const OUString& rPresetId) override;

private:
    Link<LinkParamNone*,void>   maModifyHdl;
    std::unique_ptr<weld::ComboBox> mxControl;

    DECL_LINK(ControlSelectHdl, weld::ComboBox&, void);
};

}

SdFontPropertyBox::SdFontPropertyBox(weld::Label* pLabel, weld::Container* pParent, const Any& rValue, const Link<LinkParamNone*,void>& rModifyHdl)
    : SdPropertySubControl(pParent)
    , maModifyHdl(rModifyHdl)
    , mxControl(mxBuilder->weld_combo_box("fontname"))
{
    mxControl->connect_changed(LINK(this, SdFontPropertyBox, ControlSelectHdl));
    mxControl->set_help_id(HID_SD_CUSTOMANIMATIONPANE_FONTPROPERTYBOX);
    mxControl->show();
    pLabel->set_mnemonic_widget(mxControl.get());

    SfxObjectShell* pDocSh = SfxObjectShell::Current();
    const FontList* pFontList = nullptr;
    bool bMustDelete = false;

    if (pDocSh)
    {
        auto pItem = pDocSh->GetItem( SID_ATTR_CHAR_FONTLIST );
        if (pItem)
            pFontList = static_cast<const SvxFontListItem*>(pItem)->GetFontList();
    }

    if (!pFontList)
    {
        pFontList = new FontList(Application::GetDefaultDevice(), nullptr);
        bMustDelete = true;
    }

    mxControl->freeze();

    sal_uInt16 nFontCount = pFontList->GetFontNameCount();
    for (sal_uInt16 i = 0; i < nFontCount; ++i)
    {
        const FontMetric& rFontMetric = pFontList->GetFontName(i);
        mxControl->append_text(rFontMetric.GetFamilyName());
    }

    mxControl->thaw();

    if( bMustDelete )
        delete pFontList;

    setValue( rValue, OUString() );
}

IMPL_LINK_NOARG(SdFontPropertyBox, ControlSelectHdl, weld::ComboBox&, void)
{
    maModifyHdl.Call(nullptr);
}

void SdFontPropertyBox::setValue( const Any& rValue, const OUString& )
{
    if (mxControl)
    {
        OUString aFontName;
        rValue >>= aFontName;
        mxControl->set_entry_text(aFontName);
    }
}

Any SdFontPropertyBox::getValue()
{
    OUString aFontName(mxControl->get_active_text());
    return makeAny(aFontName);
}

namespace {

class SdCharHeightPropertyBox : public SdPropertySubControl
{
public:
    SdCharHeightPropertyBox(weld::Label* pLabel, weld::Container* pParent, const Any& rValue, const Link<LinkParamNone*,void>& rModifyHdl);

    virtual Any getValue() override;
    virtual void setValue( const Any& rValue, const OUString& ) override;

    DECL_LINK(implMenuSelectHdl, const OString& rIdent, void);

private:
    Link<LinkParamNone*,void> maModifyHdl;
    std::unique_ptr<weld::MetricSpinButton> mxMetric;
    std::unique_ptr<weld::MenuButton> mxControl;

    DECL_LINK(EditModifyHdl, weld::MetricSpinButton&, void);
};

}

SdCharHeightPropertyBox::SdCharHeightPropertyBox(weld::Label* pLabel, weld::Container* pParent, const Any& rValue, const Link<LinkParamNone*,void>& rModifyHdl)
    : SdPropertySubControl(pParent)
    , maModifyHdl(rModifyHdl)
    , mxMetric(mxBuilder->weld_metric_spin_button("fontsize", FieldUnit::PERCENT))
    , mxControl(mxBuilder->weld_menu_button("fontsizemenu"))
{
    mxMetric->connect_value_changed(LINK(this, SdCharHeightPropertyBox, EditModifyHdl));
    mxMetric->set_help_id(HID_SD_CUSTOMANIMATIONPANE_CHARHEIGHTPROPERTYBOX);
    mxMetric->show();
    pLabel->set_mnemonic_widget(&mxMetric->get_widget());

    mxControl->connect_selected(LINK(this, SdCharHeightPropertyBox, implMenuSelectHdl));
    mxControl->set_help_id(HID_SD_CUSTOMANIMATIONPANE_CHARHEIGHTPROPERTYBOX);
    mxControl->show();

    setValue(rValue, OUString());
}

IMPL_LINK_NOARG(SdCharHeightPropertyBox, EditModifyHdl, weld::MetricSpinButton&, void)
{
    maModifyHdl.Call(nullptr);
}

IMPL_LINK(SdCharHeightPropertyBox, implMenuSelectHdl, const OString&, rIdent, void)
{
    sal_Int32 nValue = rIdent.toInt32();
    mxMetric->set_value(nValue, FieldUnit::PERCENT);
    EditModifyHdl(*mxMetric);
}

void SdCharHeightPropertyBox::setValue( const Any& rValue, const OUString& )
{
    if (mxMetric)
    {
        double fValue = 0.0;
        rValue >>= fValue;
        mxMetric->set_value(static_cast<::tools::Long>(fValue * 100.0), FieldUnit::PERCENT);
    }
}

Any SdCharHeightPropertyBox::getValue()
{
    return makeAny(static_cast<double>(mxMetric->get_value(FieldUnit::PERCENT)) / 100.0);
}

namespace {

class SdTransparencyPropertyBox : public SdPropertySubControl
{
public:
    SdTransparencyPropertyBox(weld::Label* pLabel, weld::Container* pParent, const Any& rValue, const Link<LinkParamNone*,void>& rModifyHdl);

    virtual Any getValue() override;
    virtual void setValue( const Any& rValue, const OUString& rPresetId  ) override;

    DECL_LINK(implMenuSelectHdl, const OString&, void);
    DECL_LINK(implModifyHdl, weld::MetricSpinButton&, void);

    void updateMenu();

private:
    Link<LinkParamNone*,void> maModifyHdl;

    std::unique_ptr<weld::MetricSpinButton> mxMetric;
    std::unique_ptr<weld::MenuButton> mxControl;
};

}

SdTransparencyPropertyBox::SdTransparencyPropertyBox(weld::Label* pLabel, weld::Container* pParent, const Any& rValue, const Link<LinkParamNone*,void>& rModifyHdl)
    : SdPropertySubControl(pParent)
    , maModifyHdl(rModifyHdl)
    , mxMetric(mxBuilder->weld_metric_spin_button("transparent", FieldUnit::PERCENT))
    , mxControl(mxBuilder->weld_menu_button("transparentmenu"))
{
    for (sal_Int32 i = 25; i < 101; i += 25)
    {
        OUString aStr(unicode::formatPercent(i,
            Application::GetSettings().GetUILanguageTag()));
        mxControl->append_item_check(OUString::number(i), aStr);
    }

    mxControl->connect_selected(LINK(this, SdTransparencyPropertyBox, implMenuSelectHdl));
    mxControl->set_help_id(HID_SD_CUSTOMANIMATIONPANE_TRANSPARENCYPROPERTYBOX);
    mxControl->show();

    mxMetric->connect_value_changed(LINK(this, SdTransparencyPropertyBox, implModifyHdl));
    mxMetric->set_help_id(HID_SD_CUSTOMANIMATIONPANE_TRANSPARENCYPROPERTYBOX);
    mxMetric->show();
    pLabel->set_mnemonic_widget(&mxMetric->get_widget());

    setValue(rValue, OUString());
}

void SdTransparencyPropertyBox::updateMenu()
{
    sal_Int64 nValue = mxMetric->get_value(FieldUnit::PERCENT);
    for (sal_uInt16 i = 25; i < 101; i += 25)
        mxControl->set_item_active(OString::number(i), nValue == i);
}

IMPL_LINK_NOARG(SdTransparencyPropertyBox, implModifyHdl, weld::MetricSpinButton&, void)
{
    updateMenu();
    maModifyHdl.Call(nullptr);
}

IMPL_LINK(SdTransparencyPropertyBox, implMenuSelectHdl, const OString&, rIdent, void)
{
    auto nValue = rIdent.toInt32();
    if (nValue != mxMetric->get_value(FieldUnit::PERCENT))
    {
        mxMetric->set_value(nValue, FieldUnit::PERCENT);
        implModifyHdl(*mxMetric);
    }
}

void SdTransparencyPropertyBox::setValue(const Any& rValue, const OUString&)
{
    if (mxMetric)
    {
        double fValue = 0.0;
        rValue >>= fValue;
        ::tools::Long nValue = static_cast<::tools::Long>(fValue * 100);
        mxMetric->set_value(nValue, FieldUnit::PERCENT);
        updateMenu();
    }
}

Any SdTransparencyPropertyBox::getValue()
{
    return makeAny(static_cast<double>(mxMetric->get_value(FieldUnit::PERCENT)) / 100.0);
}

namespace {

class SdRotationPropertyBox : public SdPropertySubControl
{
public:
    SdRotationPropertyBox(weld::Label* pLabel, weld::Container* pParent, const Any& rValue, const Link<LinkParamNone*,void>& rModifyHdl);

    virtual Any getValue() override;
    virtual void setValue( const Any& rValue, const OUString& ) override;

    DECL_LINK(implMenuSelectHdl, const OString&, void);
    DECL_LINK(implModifyHdl, weld::MetricSpinButton&, void);

    void updateMenu();

private:
    Link<LinkParamNone*,void> maModifyHdl;

    std::unique_ptr<weld::MetricSpinButton> mxMetric;
    std::unique_ptr<weld::MenuButton> mxControl;
};

}

SdRotationPropertyBox::SdRotationPropertyBox(weld::Label* pLabel, weld::Container* pParent, const Any& rValue, const Link<LinkParamNone*,void>& rModifyHdl)
    : SdPropertySubControl(pParent)
    , maModifyHdl(rModifyHdl)
    , mxMetric(mxBuilder->weld_metric_spin_button("rotate", FieldUnit::DEGREE))
    , mxControl(mxBuilder->weld_menu_button("rotatemenu"))
{
    mxMetric->connect_value_changed(LINK( this, SdRotationPropertyBox, implModifyHdl));
    mxMetric->set_help_id(HID_SD_CUSTOMANIMATIONPANE_ROTATIONPROPERTYBOX);
    mxMetric->show();
    pLabel->set_mnemonic_widget(&mxMetric->get_widget());

    mxControl->connect_selected(LINK(this, SdRotationPropertyBox, implMenuSelectHdl));
    mxControl->set_help_id(HID_SD_CUSTOMANIMATIONPANE_ROTATIONPROPERTYBOX);
    mxControl->show();

    setValue(rValue, OUString());
}

void SdRotationPropertyBox::updateMenu()
{
    sal_Int64 nValue = mxMetric->get_value(FieldUnit::DEGREE);
    bool bDirection = nValue >= 0;
    nValue = (nValue < 0 ? -nValue : nValue);

    mxControl->set_item_active("90", nValue == 90);
    mxControl->set_item_active("180", nValue == 180);
    mxControl->set_item_active("360", nValue == 360);
    mxControl->set_item_active("720", nValue == 720);

    mxControl->set_item_active("closewise", bDirection);
    mxControl->set_item_active("counterclock", !bDirection);
}

IMPL_LINK_NOARG(SdRotationPropertyBox, implModifyHdl, weld::MetricSpinButton&, void)
{
    updateMenu();
    maModifyHdl.Call(nullptr);
}

IMPL_LINK(SdRotationPropertyBox, implMenuSelectHdl, const OString&, rIdent, void)
{
    auto nValue = mxMetric->get_value(FieldUnit::DEGREE);
    bool bDirection = nValue >= 0;
    nValue = (nValue < 0 ? -nValue : nValue);

    if (rIdent == "clockwise")
        bDirection = true;
    else if (rIdent == "counterclock")
        bDirection = false;
    else
        nValue = rIdent.toInt32();

    if( !bDirection )
        nValue = -nValue;

    if (nValue != mxMetric->get_value(FieldUnit::DEGREE))
    {
        mxMetric->set_value(nValue, FieldUnit::DEGREE);
        implModifyHdl(*mxMetric);
    }
}

void SdRotationPropertyBox::setValue( const Any& rValue, const OUString& )
{
    if (mxMetric)
    {
        double fValue = 0.0;
        rValue >>= fValue;
        ::tools::Long nValue = static_cast<::tools::Long>(fValue);
        mxMetric->set_value(nValue, FieldUnit::DEGREE);
        updateMenu();
    }
}

Any SdRotationPropertyBox::getValue()
{
    return makeAny(static_cast<double>(mxMetric->get_value(FieldUnit::DEGREE)));
}

namespace {

class SdScalePropertyBox : public SdPropertySubControl
{
public:
    SdScalePropertyBox(weld::Label* pLabel, weld::Container* pParent, const Any& rValue, const Link<LinkParamNone*,void>& rModifyHdl);

    virtual Any getValue() override;
    virtual void setValue( const Any& rValue, const OUString& ) override;

    DECL_LINK(implMenuSelectHdl, const OString&, void);
    DECL_LINK(implModifyHdl, weld::MetricSpinButton&, void);

    void updateMenu();

private:
    Link<LinkParamNone*,void> maModifyHdl;
    int                       mnDirection;

    std::unique_ptr<weld::MetricSpinButton> mxMetric;
    std::unique_ptr<weld::MenuButton> mxControl;
};

}

SdScalePropertyBox::SdScalePropertyBox(weld::Label* pLabel, weld::Container* pParent, const Any& rValue, const Link<LinkParamNone*,void>& rModifyHdl)
    : SdPropertySubControl(pParent)
    , maModifyHdl( rModifyHdl )
    , mxMetric(mxBuilder->weld_metric_spin_button("scale", FieldUnit::PERCENT))
    , mxControl(mxBuilder->weld_menu_button("scalemenu"))
{
    mxControl->connect_selected(LINK(this, SdScalePropertyBox, implMenuSelectHdl));
    mxControl->set_help_id(HID_SD_CUSTOMANIMATIONPANE_SCALEPROPERTYBOX);
    mxControl->show();

    mxMetric->connect_value_changed(LINK(this, SdScalePropertyBox, implModifyHdl));
    mxMetric->set_help_id(HID_SD_CUSTOMANIMATIONPANE_SCALEPROPERTYBOX);
    mxMetric->show();
    pLabel->set_mnemonic_widget(&mxMetric->get_widget());

    setValue(rValue, OUString());
}

void SdScalePropertyBox::updateMenu()
{
    auto nValue = mxMetric->get_value(FieldUnit::PERCENT);

    mxControl->set_item_active("25scale", nValue == 25);
    mxControl->set_item_active("50scale", nValue == 50);
    mxControl->set_item_active("150scale", nValue == 150);
    mxControl->set_item_active("400scale", nValue == 400);

    mxControl->set_item_active("hori", mnDirection == 1);
    mxControl->set_item_active("vert", mnDirection == 2);
    mxControl->set_item_active("both", mnDirection == 3);
}

IMPL_LINK_NOARG(SdScalePropertyBox, implModifyHdl, weld::MetricSpinButton&, void)
{
    updateMenu();
    maModifyHdl.Call(nullptr);
}

IMPL_LINK(SdScalePropertyBox, implMenuSelectHdl, const OString&, rIdent, void)
{
    auto nValue = mxMetric->get_value(FieldUnit::PERCENT);

    int nDirection = mnDirection;

    if (rIdent == "hori")
        nDirection = 1;
    else if (rIdent == "veri")
        nDirection = 2;
    else if (rIdent == "both")
        nDirection = 3;
    else
        nValue = rIdent.toInt32();

    bool bModified = false;

    if( nDirection != mnDirection )
    {
        mnDirection = nDirection;
        bModified = true;
    }

    if (nValue != mxMetric->get_value(FieldUnit::PERCENT))
    {
        mxMetric->set_value(nValue, FieldUnit::PERCENT);
        bModified = true;
    }

    if(bModified)
    {
        implModifyHdl(*mxMetric);
        updateMenu();
    }
}

void SdScalePropertyBox::setValue(const Any& rValue, const OUString&)
{
    if (!mxMetric)
        return;

    ValuePair aValues;
    rValue >>= aValues;

    double fValue1 = 0.0;
    double fValue2 = 0.0;

    aValues.First >>= fValue1;
    aValues.Second >>= fValue2;

    if( fValue2 == 0.0 )
        mnDirection = 1;
    else if( fValue1 == 0.0 )
        mnDirection = 2;
    else
        mnDirection = 3;

    // Shrink animation is represented by negative value
    // Shrink factor is calculated as (1 + $fValue)
    // e.g 1 + (-0.75) = 0.25 => shrink to 25% of the size
    // 0.25 = -0.75 + 1
    if ( fValue1 < 0.0 )
        fValue1 += 1;
    if ( fValue2 < 0.0 )
        fValue2 += 1;

    ::tools::Long nValue;
    if( fValue1 )
        nValue = static_cast<::tools::Long>(fValue1 * 100.0);
    else
        nValue = static_cast<::tools::Long>(fValue2 * 100.0);
    mxMetric->set_value(nValue, FieldUnit::PERCENT);
    updateMenu();
}

Any SdScalePropertyBox::getValue()
{
    double fValue1 = static_cast<double>(mxMetric->get_value(FieldUnit::PERCENT)) / 100.0;

    // Shrink animation is represented by value < 1 (< 100%)
    // Shrink factor is calculated as (1 + $fValue)
    // e.g shrink to 25% of the size: 0.25 = 1 + $fValue =>
    // $fValue = -0.75; -0.75 = 0.25 -1
    if ( fValue1 < 1.0 )
        fValue1 -= 1;

    double fValue2 = fValue1;

    if( mnDirection == 1 )
        fValue2 = 0.0;
    else if( mnDirection == 2 )
        fValue1 = 0.0;

    ValuePair aValues;
    aValues.First <<= fValue1;
    aValues.Second <<= fValue2;

    return makeAny( aValues );
}

namespace {

class SdFontStylePropertyBox : public SdPropertySubControl
{
public:
    SdFontStylePropertyBox(weld::Label* pLabel, weld::Container* pParent, const Any& rValue, const Link<LinkParamNone*,void>& rModifyHdl);

    virtual Any getValue() override;
    virtual void setValue( const Any& rValue, const OUString& ) override;

    DECL_LINK(implMenuSelectHdl, const OString&, void);

    void update();

private:
    float mfFontWeight;
    awt::FontSlant meFontSlant;
    sal_Int16 mnFontUnderline;
    Link<LinkParamNone*,void> maModifyHdl;

    std::unique_ptr<weld::Entry> mxEdit;
    std::unique_ptr<weld::MenuButton> mxControl;
};

}

SdFontStylePropertyBox::SdFontStylePropertyBox(weld::Label* pLabel, weld::Container* pParent, const Any& rValue, const Link<LinkParamNone*,void>& rModifyHdl )
    : SdPropertySubControl(pParent)
    , maModifyHdl( rModifyHdl )
    , mxEdit(mxBuilder->weld_entry("entry"))
    , mxControl(mxBuilder->weld_menu_button("entrymenu"))
{
    mxEdit->set_text(SdResId(STR_CUSTOMANIMATION_SAMPLE));
    mxEdit->set_help_id(HID_SD_CUSTOMANIMATIONPANE_FONTSTYLEPROPERTYBOX);
    pLabel->set_mnemonic_widget(mxEdit.get());
    mxEdit->show();

    mxControl->connect_selected(LINK(this, SdFontStylePropertyBox, implMenuSelectHdl));
    mxControl->set_help_id(HID_SD_CUSTOMANIMATIONPANE_FONTSTYLEPROPERTYBOX);
    mxControl->show();

    setValue(rValue, OUString());
}

void SdFontStylePropertyBox::update()
{
    // update menu
    mxControl->set_item_active("bold", mfFontWeight == awt::FontWeight::BOLD);
    mxControl->set_item_active("italic", meFontSlant == awt::FontSlant_ITALIC);
    mxControl->set_item_active("underline", mnFontUnderline != awt::FontUnderline::NONE );

    // update sample edit
    vcl::Font aFont(mxEdit->get_font());
    aFont.SetWeight(mfFontWeight == awt::FontWeight::BOLD ? WEIGHT_BOLD : WEIGHT_NORMAL);
    aFont.SetItalic(meFontSlant == awt::FontSlant_ITALIC ? ITALIC_NORMAL : ITALIC_NONE);
    aFont.SetUnderline(mnFontUnderline == awt::FontUnderline::NONE ? LINESTYLE_NONE : LINESTYLE_SINGLE);
    mxEdit->set_font(aFont);
}

IMPL_LINK(SdFontStylePropertyBox, implMenuSelectHdl, const OString&, rIdent, void)
{
    if (rIdent == "bold")
    {
        if( mfFontWeight == awt::FontWeight::BOLD )
            mfFontWeight = awt::FontWeight::NORMAL;
        else
            mfFontWeight = awt::FontWeight::BOLD;
    }
    else if (rIdent == "italic")
    {
        if( meFontSlant == awt::FontSlant_ITALIC )
            meFontSlant = awt::FontSlant_NONE;
        else
            meFontSlant = awt::FontSlant_ITALIC;
    }
    else if (rIdent == "underline")
    {
        if( mnFontUnderline == awt::FontUnderline::SINGLE )
            mnFontUnderline = awt::FontUnderline::NONE;
        else
            mnFontUnderline = awt::FontUnderline::SINGLE;
    }

    update();
    maModifyHdl.Call(nullptr);
}

void SdFontStylePropertyBox::setValue( const Any& rValue, const OUString& )
{
    Sequence<Any> aValues;
    rValue >>= aValues;

    aValues[0] >>= mfFontWeight;
    aValues[1] >>= meFontSlant;
    aValues[2] >>= mnFontUnderline;

    update();
}

Any SdFontStylePropertyBox::getValue()
{
    Sequence<Any> aValues(3);
    aValues[0] <<= mfFontWeight;
    aValues[1] <<= meFontSlant;
    aValues[2] <<= mnFontUnderline;
    return makeAny( aValues );
}

class CustomAnimationEffectTabPage
{
public:
    CustomAnimationEffectTabPage(weld::Container* pParent, weld::Window* pDialog, const STLPropertySet* pSet);

    void update( STLPropertySet* pSet );
    DECL_LINK(implSelectHdl, weld::ComboBox&, void);
    DECL_LINK(implClickHdl, weld::Button&, void);
    void implHdl(const weld::Widget*);

private:
    void updateControlStates();
    void fillSoundListBox();
    void clearSoundListBox();
    sal_Int32 getSoundObject( std::u16string_view rStr );
    void openSoundFileDialog();
    void onSoundPreview();
    weld::Window* GetFrameWeld() const { return mpDialog; }

private:
    ::std::vector< OUString > maSoundList;
    bool mbHasText;
    const STLPropertySet* mpSet;
    css::uno::Reference<css::media::XPlayer> mxPlayer;

    weld::Window* mpDialog;
    std::unique_ptr<weld::Builder> mxBuilder;
    std::unique_ptr<weld::Container> mxContainer;
    std::unique_ptr<weld::Widget> mxSettings;
    std::unique_ptr<weld::Label> mxFTProperty1;
    std::unique_ptr<weld::Container> mxPlaceholderBox;
    std::unique_ptr<weld::CheckButton> mxCBSmoothStart;
    std::unique_ptr<weld::CheckButton> mxCBSmoothEnd;
    std::unique_ptr<weld::Label> mxFTSound;
    std::unique_ptr<weld::ComboBox> mxLBSound;
    std::unique_ptr<weld::Button> mxPBSoundPreview;
    std::unique_ptr<weld::Label> mxFTAfterEffect;
    std::unique_ptr<weld::ComboBox> mxLBAfterEffect;
    std::unique_ptr<weld::Label> mxFTDimColor;
    std::unique_ptr<ColorListBox> mxCLBDimColor;
    std::unique_ptr<weld::Label> mxFTTextAnim;
    std::unique_ptr<weld::ComboBox> mxLBTextAnim;
    std::unique_ptr<weld::MetricSpinButton> mxMFTextDelay;
    std::unique_ptr<weld::Label> mxFTTextDelay;
    std::unique_ptr<SdPropertySubControl> mxLBSubControl;
};

CustomAnimationEffectTabPage::CustomAnimationEffectTabPage(weld::Container* pParent, weld::Window* pDialog, const STLPropertySet* pSet)
    : mbHasText(false)
    , mpSet(pSet)
    , mpDialog(pDialog)
    , mxBuilder(Application::CreateBuilder(pParent, "modules/simpress/ui/customanimationeffecttab.ui"))
    , mxContainer(mxBuilder->weld_container("EffectTab"))
    , mxSettings(mxBuilder->weld_widget("settings"))
    , mxFTProperty1(mxBuilder->weld_label("prop_label1"))
    , mxPlaceholderBox(mxBuilder->weld_container("placeholder"))
    , mxCBSmoothStart(mxBuilder->weld_check_button("smooth_start"))
    , mxCBSmoothEnd(mxBuilder->weld_check_button("smooth_end"))
    , mxFTSound(mxBuilder->weld_label("sound_label"))
    , mxLBSound(mxBuilder->weld_combo_box("sound_list"))
    , mxPBSoundPreview(mxBuilder->weld_button("sound_preview"))
    , mxFTAfterEffect(mxBuilder->weld_label("aeffect_label"))
    , mxLBAfterEffect(mxBuilder->weld_combo_box("aeffect_list"))
    , mxFTDimColor(mxBuilder->weld_label("dim_color_label"))
    , mxCLBDimColor(new ColorListBox(mxBuilder->weld_menu_button("dim_color_list"), pDialog))
    , mxFTTextAnim(mxBuilder->weld_label("text_animation_label"))
    , mxLBTextAnim(mxBuilder->weld_combo_box("text_animation_list"))
    , mxMFTextDelay(mxBuilder->weld_metric_spin_button("text_delay", FieldUnit::PERCENT))
    , mxFTTextDelay(mxBuilder->weld_label("text_delay_label"))
{
    mxCLBDimColor->SelectEntry(COL_BLACK);

    // fill the soundbox
    fillSoundListBox();

    mxLBSound->connect_changed(LINK(this, CustomAnimationEffectTabPage, implSelectHdl));
    mxPBSoundPreview->connect_clicked(LINK(this, CustomAnimationEffectTabPage, implClickHdl));

    // only show settings if all selected effects have the same preset-id
    if( pSet->getPropertyState( nHandlePresetId ) != STLPropertyState::Ambiguous )
    {
        OUString aPresetId;
        pSet->getPropertyValue( nHandlePresetId ) >>= aPresetId;

        // property 1

        if( pSet->getPropertyState( nHandleProperty1Type ) != STLPropertyState::Ambiguous )
        {
            sal_Int32 nType = 0;
            pSet->getPropertyValue( nHandleProperty1Type ) >>= nType;

            if( nType != nPropertyTypeNone )
            {
                // set ui name for property at fixed text
                OUString aPropertyName( getPropertyName( nType ) );

                if( !aPropertyName.isEmpty() )
                {
                    mxSettings->show();
                    mxFTProperty1->set_label(aPropertyName);
                }

                // get property value
                const Any aValue( pSet->getPropertyValue( nHandleProperty1Value ) );

                // create property sub control
                mxLBSubControl = SdPropertySubControl::create(nType, mxFTProperty1.get(), mxPlaceholderBox.get(), mpDialog, aValue, aPresetId, Link<LinkParamNone*,void>());
            }
        }

        mxFTProperty1->set_sensitive(mxPlaceholderBox->get_sensitive());

        // accelerate & decelerate

        if( pSet->getPropertyState( nHandleAccelerate ) == STLPropertyState::Direct )
        {
            mxCBSmoothStart->show();
            mxCBSmoothEnd->show();

            double fTemp = 0.0;
            pSet->getPropertyValue( nHandleAccelerate ) >>= fTemp;
            mxCBSmoothStart->set_active( fTemp > 0.0 );

            pSet->getPropertyValue( nHandleDecelerate ) >>= fTemp;
            mxCBSmoothEnd->set_active( fTemp > 0.0 );
        }
    }

    // init after effect controls

    mxLBAfterEffect->connect_changed(LINK(this, CustomAnimationEffectTabPage, implSelectHdl));
    mxLBTextAnim->connect_changed(LINK(this, CustomAnimationEffectTabPage, implSelectHdl));

    if( (pSet->getPropertyState( nHandleHasAfterEffect ) != STLPropertyState::Ambiguous) &&
        (pSet->getPropertyState( nHandleAfterEffectOnNextEffect ) != STLPropertyState::Ambiguous) &&
        (pSet->getPropertyState( nHandleDimColor ) != STLPropertyState::Ambiguous))
    {
        bool bHasAfterEffect = false;
        pSet->getPropertyValue( nHandleHasAfterEffect ) >>= bHasAfterEffect;

        sal_Int32 nPos = 0;
        if( bHasAfterEffect )
        {
            nPos++;

            bool bAfterEffectOnNextClick = false;
            pSet->getPropertyValue( nHandleAfterEffectOnNextEffect ) >>= bAfterEffectOnNextClick;
            Any aDimColor( pSet->getPropertyValue( nHandleDimColor ) );

            if( aDimColor.hasValue() )
            {
                sal_Int32 nColor = 0;
                aDimColor >>= nColor;
                Color aColor(nColor);
                mxCLBDimColor->SelectEntry(aColor);
            }
            else
            {
                nPos++;
                if( bAfterEffectOnNextClick )
                    nPos++;
            }
        }

        mxLBAfterEffect->set_active(nPos);
    }

    if( pSet->getPropertyState( nHandleHasText ) != STLPropertyState::Ambiguous )
        pSet->getPropertyValue( nHandleHasText ) >>= mbHasText;

    if( mbHasText )
    {
        if( pSet->getPropertyState( nHandleIterateType ) != STLPropertyState::Ambiguous)
        {
            int nPos = -1;

            sal_Int32 nIterateType = 0;
            pSet->getPropertyValue( nHandleIterateType ) >>= nIterateType;
            switch( nIterateType )
            {
            case TextAnimationType::BY_PARAGRAPH:   nPos = 0; break;
            case TextAnimationType::BY_WORD:        nPos = 1; break;
            case TextAnimationType::BY_LETTER:      nPos = 2; break;
            }

            mxLBTextAnim->set_active(nPos);
        }

        if( pSet->getPropertyState( nHandleIterateInterval ) != STLPropertyState::Default )
        {
            double fIterateInterval = 0.0;
            pSet->getPropertyValue( nHandleIterateInterval ) >>= fIterateInterval;
            mxMFTextDelay->set_value(static_cast<::tools::Long>(fIterateInterval*10), FieldUnit::NONE);
        }
    }
    else
    {
        mxFTTextAnim->set_sensitive(false);
        mxLBTextAnim->set_sensitive(false);
        mxMFTextDelay->set_sensitive(false);
        mxFTTextDelay->set_sensitive(false);

    }

    if( pSet->getPropertyState( nHandleSoundURL ) != STLPropertyState::Ambiguous )
    {
        sal_Int32 nPos = 0;

        const Any aValue( pSet->getPropertyValue( nHandleSoundURL ) );

        if( aValue.getValueType() == ::cppu::UnoType<sal_Bool>::get() )
        {
            nPos = 1;
        }
        else
        {
            OUString aSoundURL;
            aValue >>= aSoundURL;

            if( !aSoundURL.isEmpty() )
            {
                sal_uLong i;
                for( i = 0; i < maSoundList.size(); i++ )
                {
                    OUString aString = maSoundList[ i ];
                    if( aString == aSoundURL )
                    {
                        nPos = static_cast<sal_Int32>(i)+2;
                        break;
                    }
                }

                if( nPos == 0 )
                {
                    nPos = static_cast<sal_Int32>(maSoundList.size())+2;
                    maSoundList.push_back( aSoundURL );
                    INetURLObject aURL( aSoundURL );
                    mxLBSound->insert_text(nPos, aURL.GetBase());
                }
            }
        }

        if( nPos != -1)
            mxLBSound->set_active(nPos);
    }

    updateControlStates();

}

void CustomAnimationEffectTabPage::updateControlStates()
{
    auto nPos = mxLBAfterEffect->get_active();
    mxCLBDimColor->set_sensitive( nPos == 1 );
    mxFTDimColor->set_sensitive( nPos == 1 );

    if( mbHasText )
    {
        nPos = mxLBTextAnim->get_active();
        mxMFTextDelay->set_sensitive( nPos != 0 );
        mxFTTextDelay->set_sensitive( nPos != 0 );
    }

    if (comphelper::LibreOfficeKit::isActive())
    {
        mxFTSound->hide();
        mxLBSound->hide();
        mxPBSoundPreview->hide();
    }
    else
    {
        nPos = mxLBSound->get_active();
        mxPBSoundPreview->set_sensitive( nPos >= 2 );
    }

}

IMPL_LINK(CustomAnimationEffectTabPage, implClickHdl, weld::Button&, rBtn, void)
{
    implHdl(&rBtn);
}

IMPL_LINK(CustomAnimationEffectTabPage, implSelectHdl, weld::ComboBox&, rListBox, void)
{
    implHdl(&rListBox);
}

void CustomAnimationEffectTabPage::implHdl(const weld::Widget* pControl)
{
    if (pControl == mxLBTextAnim.get())
    {
        if (mxMFTextDelay->get_value(FieldUnit::NONE) == 0)
            mxMFTextDelay->set_value(100, FieldUnit::NONE);
    }
    else if (pControl == mxLBSound.get())
    {
        auto nPos = mxLBSound->get_active();
        if (nPos == (mxLBSound->get_count() - 1))
        {
            openSoundFileDialog();
        }
    }
    else if (pControl == mxPBSoundPreview.get())
    {
        onSoundPreview();
    }

    updateControlStates();
}

void CustomAnimationEffectTabPage::update( STLPropertySet* pSet )
{
    if (mxLBSubControl)
    {
        Any aNewValue(mxLBSubControl->getValue());
        Any aOldValue;
        if( mpSet->getPropertyState( nHandleProperty1Value ) != STLPropertyState::Ambiguous)
            aOldValue = mpSet->getPropertyValue( nHandleProperty1Value );

        if( aOldValue != aNewValue )
            pSet->setPropertyValue( nHandleProperty1Value, aNewValue );
    }

    if (mxCBSmoothStart->get_visible())
    {
        // set selected value for accelerate if different than in original set

        double fTemp = mxCBSmoothStart->get_active() ? 0.5 : 0.0;

        double fOldTemp = 0.0;
        if(mpSet->getPropertyState( nHandleAccelerate ) != STLPropertyState::Ambiguous)
            mpSet->getPropertyValue( nHandleAccelerate ) >>= fOldTemp;
        else
            fOldTemp = -2.0;

        if( fOldTemp != fTemp )
            pSet->setPropertyValue( nHandleAccelerate, makeAny( fTemp ) );

        // set selected value for decelerate if different than in original set
        fTemp = mxCBSmoothEnd->get_active() ? 0.5 : 0.0;

        if(mpSet->getPropertyState( nHandleDecelerate ) != STLPropertyState::Ambiguous)
            mpSet->getPropertyValue( nHandleDecelerate ) >>= fOldTemp;
        else
            fOldTemp = -2.0;

        if( fOldTemp != fTemp )
            pSet->setPropertyValue( nHandleDecelerate, makeAny( fTemp ) );
    }

    auto nPos = mxLBAfterEffect->get_active();
    if (nPos != -1)
    {
        bool bAfterEffect = nPos != 0;

        bool bOldAfterEffect = false;

        if(mpSet->getPropertyState( nHandleHasAfterEffect ) != STLPropertyState::Ambiguous)
            mpSet->getPropertyValue( nHandleHasAfterEffect ) >>= bOldAfterEffect;
        else
            bOldAfterEffect = !bAfterEffect;

        if( bOldAfterEffect != bAfterEffect )
            pSet->setPropertyValue( nHandleHasAfterEffect, makeAny( bAfterEffect ) );

        Any aDimColor;
        if( nPos == 1 )
        {
            Color aSelectedColor = mxCLBDimColor->GetSelectEntryColor();
            aDimColor <<= aSelectedColor.GetRGBColor();
        }

        if( (mpSet->getPropertyState( nHandleDimColor ) == STLPropertyState::Ambiguous) ||
            (mpSet->getPropertyValue( nHandleDimColor ) != aDimColor) )
            pSet->setPropertyValue( nHandleDimColor, aDimColor );

        bool bAfterEffectOnNextEffect = nPos != 2;
        bool bOldAfterEffectOnNextEffect = !bAfterEffectOnNextEffect;

        if( mpSet->getPropertyState( nHandleAfterEffectOnNextEffect ) != STLPropertyState::Ambiguous)
            mpSet->getPropertyValue( nHandleAfterEffectOnNextEffect ) >>= bOldAfterEffectOnNextEffect;

        if( bAfterEffectOnNextEffect != bOldAfterEffectOnNextEffect )
            pSet->setPropertyValue( nHandleAfterEffectOnNextEffect, makeAny( bAfterEffectOnNextEffect ) );
    }

    nPos = mxLBTextAnim->get_active();
    if (nPos != -1)
    {
        sal_Int16 nIterateType;

        switch( nPos )
        {
        case 1: nIterateType = TextAnimationType::BY_WORD; break;
        case 2: nIterateType = TextAnimationType::BY_LETTER; break;
        default:
            nIterateType = TextAnimationType::BY_PARAGRAPH;
        }

        sal_Int16 nOldIterateType = nIterateType-1;

        if(mpSet->getPropertyState( nHandleIterateType ) != STLPropertyState::Ambiguous)
            mpSet->getPropertyValue( nHandleIterateType ) >>= nOldIterateType;

        if( nIterateType != nOldIterateType )
            pSet->setPropertyValue( nHandleIterateType, makeAny( nIterateType ) );
    }

    {
        double fIterateInterval = static_cast<double>(mxMFTextDelay->get_value(FieldUnit::NONE)) / 10;
        double fOldIterateInterval = -1.0;

        if( mpSet->getPropertyState( nHandleIterateInterval ) != STLPropertyState::Ambiguous )
            mpSet->getPropertyValue( nHandleIterateInterval ) >>= fOldIterateInterval;

        if( fIterateInterval != fOldIterateInterval )
            pSet->setPropertyValue( nHandleIterateInterval, makeAny( fIterateInterval ) );
    }

    nPos = mxLBSound->get_active();
    if (nPos == -1)
        return;

    Any aNewSoundURL, aOldSoundURL( makeAny( sal_Int32(0) ) );

    if( nPos == 0 )
    {
        // 0 means no sound, so leave any empty
    }
    else if( nPos == 1 )
    {
        // this means stop sound
        aNewSoundURL <<= true;
    }
    else
    {
        OUString aSoundURL( maSoundList[ nPos-2 ] );
        aNewSoundURL <<= aSoundURL;
    }

    if( mpSet->getPropertyState( nHandleSoundURL ) != STLPropertyState::Ambiguous )
        aOldSoundURL = mpSet->getPropertyValue( nHandleSoundURL  );

    if( aNewSoundURL != aOldSoundURL )
        pSet->setPropertyValue( nHandleSoundURL, aNewSoundURL );
}

void CustomAnimationEffectTabPage::fillSoundListBox()
{
    GalleryExplorer::FillObjList( GALLERY_THEME_SOUNDS, maSoundList );
    GalleryExplorer::FillObjList( GALLERY_THEME_USERSOUNDS, maSoundList );

    mxLBSound->append_text( SdResId(STR_CUSTOMANIMATION_NO_SOUND) );
    mxLBSound->append_text( SdResId(STR_CUSTOMANIMATION_STOP_PREVIOUS_SOUND) );
    for(const OUString & rString : maSoundList)
    {
        INetURLObject aURL( rString );
        mxLBSound->append_text( aURL.GetBase() );
    }
    mxLBSound->append_text( SdResId(STR_CUSTOMANIMATION_BROWSE_SOUND) );
}

void CustomAnimationEffectTabPage::clearSoundListBox()
{
    maSoundList.clear();
    mxLBSound->clear();
}

sal_Int32 CustomAnimationEffectTabPage::getSoundObject( std::u16string_view rStr )
{
    size_t i;
    const size_t nCount = maSoundList.size();
    for( i = 0; i < nCount; i++ )
    {
        if( maSoundList[ i ].equalsIgnoreAsciiCase(rStr) )
            return i+2;
    }

    return -1;
}

void CustomAnimationEffectTabPage::openSoundFileDialog()
{
    SdOpenSoundFileDialog aFileDialog(GetFrameWeld());

    bool bValidSoundFile = false;
    bool bQuitLoop = false;
    ::tools::Long nPos = 0;

    while( !bQuitLoop && (aFileDialog.Execute() == ERRCODE_NONE) )
    {
        OUString aFile = aFileDialog.GetPath();
        nPos = getSoundObject( aFile );

        if( nPos < 0 ) // not in Soundliste
        {
            // try to insert in Gallery
            if( GalleryExplorer::InsertURL( GALLERY_THEME_USERSOUNDS, aFile ) )
            {
                clearSoundListBox();
                fillSoundListBox();

                nPos = getSoundObject( aFile );
                DBG_ASSERT( nPos >= 0, "sd::CustomAnimationEffectTabPage::openSoundFileDialog(), Recently inserted sound not in list!" );

                bValidSoundFile=true;
                bQuitLoop=true;
            }
            else
            {
                OUString aStrWarning(SdResId(STR_WARNING_NOSOUNDFILE));
                aStrWarning = aStrWarning.replaceFirst("%", aFile);
                std::unique_ptr<weld::MessageDialog> xWarn(Application::CreateMessageDialog(nullptr,
                                                           VclMessageType::Warning, VclButtonsType::NONE,
                                                           aStrWarning));
                xWarn->add_button(GetStandardText(StandardButtonType::Retry), RET_RETRY);
                xWarn->add_button(GetStandardText(StandardButtonType::Cancel), RET_CANCEL);
                bQuitLoop = xWarn->run() != RET_RETRY;

                bValidSoundFile=false;
            }
        }
        else
        {
            bValidSoundFile=true;
            bQuitLoop=true;
        }
    }

    if( !bValidSoundFile )
        nPos = 0;

    mxLBSound->set_active(nPos);
}

void CustomAnimationEffectTabPage::onSoundPreview()
{
#if HAVE_FEATURE_AVMEDIA
    const auto nPos = mxLBSound->get_active();

    if( nPos >= 2 ) try
    {
        const OUString aSoundURL( maSoundList[ nPos-2 ] );
        mxPlayer.set( avmedia::MediaWindow::createPlayer( aSoundURL, "" ), uno::UNO_SET_THROW );
        mxPlayer->start();
    }
    catch( uno::Exception& )
    {
        OSL_FAIL("CustomAnimationEffectTabPage::onSoundPreview(), exception caught!" );
    }
#endif
}

class CustomAnimationDurationTabPage
{
public:
    CustomAnimationDurationTabPage(weld::Container* pParent, const STLPropertySet* pSet);

    void update( STLPropertySet* pSet );

    DECL_LINK(implControlHdl, weld::ComboBox&, void);
    DECL_LINK(implClickHdl, weld::Button&, void);
    DECL_LINK(DurationModifiedHdl, weld::MetricSpinButton&, void);
    void implHdl(const weld::Widget*);

private:
    const STLPropertySet* mpSet;

    std::unique_ptr<weld::Builder> mxBuilder;
    std::unique_ptr<weld::Container> mxContainer;
    std::unique_ptr<weld::Label> mxFTStart;
    std::unique_ptr<weld::ComboBox> mxLBStart;
    std::unique_ptr<weld::Label> mxFTStartDelay;
    std::unique_ptr<weld::MetricSpinButton> mxMFStartDelay;
    std::unique_ptr<weld::Label> mxFTDuration;
    std::unique_ptr<weld::MetricSpinButton> mxCBXDuration;
    std::unique_ptr<weld::Label> mxFTRepeat;
    std::unique_ptr<weld::ComboBox> mxCBRepeat;
    std::unique_ptr<weld::CheckButton> mxCBXRewind;
    std::unique_ptr<weld::RadioButton> mxRBClickSequence;
    std::unique_ptr<weld::RadioButton> mxRBInteractive;
    std::unique_ptr<weld::ComboBox> mxLBTrigger;
};

CustomAnimationDurationTabPage::CustomAnimationDurationTabPage(weld::Container* pParent, const STLPropertySet* pSet)
    : mpSet(pSet)
    , mxBuilder(Application::CreateBuilder(pParent, "modules/simpress/ui/customanimationtimingtab.ui"))
    , mxContainer(mxBuilder->weld_container("TimingTab"))
    , mxFTStart(mxBuilder->weld_label("start_label"))
    , mxLBStart(mxBuilder->weld_combo_box("start_list"))
    , mxFTStartDelay(mxBuilder->weld_label("delay_label"))
    , mxMFStartDelay(mxBuilder->weld_metric_spin_button("delay_value", FieldUnit::SECOND))
    , mxFTDuration(mxBuilder->weld_label("duration_label"))
    , mxCBXDuration(mxBuilder->weld_metric_spin_button("anim_duration", FieldUnit::SECOND))
    , mxFTRepeat(mxBuilder->weld_label("repeat_label"))
    , mxCBRepeat(mxBuilder->weld_combo_box("repeat_list"))
    , mxCBXRewind(mxBuilder->weld_check_button("rewind"))
    , mxRBClickSequence(mxBuilder->weld_radio_button("rb_click_sequence"))
    , mxRBInteractive(mxBuilder->weld_radio_button("rb_interactive"))
    , mxLBTrigger(mxBuilder->weld_combo_box("trigger_list"))
{
    mxLBTrigger->set_size_request(mxLBTrigger->get_approximate_digit_width() * 20, -1);

    fillRepeatComboBox(*mxCBRepeat);

    mxRBClickSequence->connect_clicked(LINK(this, CustomAnimationDurationTabPage, implClickHdl));
    mxLBTrigger->connect_changed(LINK(this, CustomAnimationDurationTabPage, implControlHdl));
    mxCBXDuration->connect_value_changed(LINK( this, CustomAnimationDurationTabPage, DurationModifiedHdl));

    if( pSet->getPropertyState( nHandleStart ) != STLPropertyState::Ambiguous )
    {
        sal_Int16 nStart = 0;
        pSet->getPropertyValue( nHandleStart ) >>= nStart;
        sal_Int32 nPos = 0;
        switch( nStart )
        {
            case EffectNodeType::WITH_PREVIOUS:     nPos = 1; break;
            case EffectNodeType::AFTER_PREVIOUS:    nPos = 2; break;
        }
        mxLBStart->set_active(nPos);
    }

    if( pSet->getPropertyState( nHandleBegin ) != STLPropertyState::Ambiguous )
    {
        double fBegin = 0.0;
        pSet->getPropertyValue( nHandleBegin ) >>= fBegin;
        mxMFStartDelay->set_value(static_cast<::tools::Long>(fBegin*10), FieldUnit::NONE);
    }

    if( pSet->getPropertyState( nHandleDuration ) != STLPropertyState::Ambiguous )
    {
        double fDuration = 0.0;
        pSet->getPropertyValue( nHandleDuration ) >>= fDuration;

        if( fDuration == 0.001 )
        {
            mxFTDuration->set_sensitive(false);
            mxCBXDuration->set_sensitive(false);
            mxFTRepeat->set_sensitive(false);
            mxCBRepeat->set_sensitive(false);
            mxCBXRewind->set_sensitive(false);
        }
        else
        {
            mxCBXDuration->set_value(fDuration * 100.0, FieldUnit::NONE);
        }
    }

    if( pSet->getPropertyState( nHandleRepeat ) != STLPropertyState::Ambiguous )
    {
        Any aRepeatCount( pSet->getPropertyValue( nHandleRepeat ) );
        if( (aRepeatCount.getValueType() == ::cppu::UnoType<double>::get()) || !aRepeatCount.hasValue() )
        {
            double fRepeat = 0.0;
            if( aRepeatCount.hasValue() )
                aRepeatCount >>= fRepeat;

            auto nPos = -1;

            if( fRepeat == 0 )
                nPos = 0;
            else if( fRepeat == 2.0 )
                nPos = 1;
            else if( fRepeat == 3.0 )
                nPos = 2;
            else if( fRepeat == 4.0 )
                nPos = 3;
            else if( fRepeat == 5.0 )
                nPos = 4;
            else if( fRepeat == 10.0 )
                nPos = 5;

            if (nPos != -1)
                mxCBRepeat->set_active(nPos);
            else
                mxCBRepeat->set_entry_text(OUString::number(fRepeat));
        }
        else if( aRepeatCount.getValueType() == ::cppu::UnoType<Timing>::get() )
        {
            Any aEnd;
            if( pSet->getPropertyState( nHandleEnd ) != STLPropertyState::Ambiguous )
                aEnd = pSet->getPropertyValue( nHandleEnd );

            mxCBRepeat->set_active(aEnd.hasValue() ? 6 : 7);
        }
    }

    if( pSet->getPropertyState( nHandleRewind ) != STLPropertyState::Ambiguous )
    {
        sal_Int16 nFill = 0;
        if( pSet->getPropertyValue( nHandleRewind ) >>= nFill )
        {
            mxCBXRewind->set_active(nFill == AnimationFill::REMOVE);
        }
        else
        {
            mxCBXRewind->set_state(TRISTATE_INDET);
        }
    }

    Reference< XShape > xTrigger;

    if( pSet->getPropertyState( nHandleTrigger ) != STLPropertyState::Ambiguous )
    {
        pSet->getPropertyValue( nHandleTrigger ) >>= xTrigger;

        mxRBInteractive->set_active(xTrigger.is());
        mxRBClickSequence->set_active(!xTrigger.is());
    }

    Reference< XDrawPage > xCurrentPage;
    pSet->getPropertyValue( nHandleCurrentPage ) >>= xCurrentPage;
    if( !xCurrentPage.is() )
        return;

    const OUString aStrIsEmptyPresObj( "IsEmptyPresentationObject" );

    sal_Int32 nShape, nCount = xCurrentPage->getCount();
    for( nShape = 0; nShape < nCount; nShape++ )
    {
        Reference< XShape > xShape( xCurrentPage->getByIndex( nShape ), UNO_QUERY );

        if( !xShape.is() )
            continue;

        Reference< XPropertySet > xSet( xShape, UNO_QUERY );
        if( xSet.is() && xSet->getPropertySetInfo()->hasPropertyByName( aStrIsEmptyPresObj ) )
        {
            bool bIsEmpty = false;
            xSet->getPropertyValue( aStrIsEmptyPresObj ) >>= bIsEmpty;
            if( bIsEmpty )
                continue;
        }

        OUString aDescription( getShapeDescription( xShape, true ) );
        mxLBTrigger->append(OUString::number(nShape), aDescription);
        auto nPos = mxLBTrigger->get_count() - 1;
        if (xShape == xTrigger)
            mxLBTrigger->set_active(nPos);
    }
}

IMPL_LINK(CustomAnimationDurationTabPage, implClickHdl, weld::Button&, rBtn, void)
{
    implHdl(&rBtn);
}

IMPL_LINK(CustomAnimationDurationTabPage, implControlHdl, weld::ComboBox&, rListBox, void)
{
    implHdl(&rListBox);
}

IMPL_LINK_NOARG(CustomAnimationDurationTabPage, DurationModifiedHdl, weld::MetricSpinButton&, void)
{
    if (!mxCBXDuration->get_text().isEmpty())
    {
        double duration_value = static_cast<double>(mxCBXDuration->get_value(FieldUnit::NONE));
        if(duration_value <= 0.0)
            mxCBXDuration->set_value(1, FieldUnit::NONE);
        else
            mxCBXDuration->set_value(duration_value, FieldUnit::NONE);
    }
}

void CustomAnimationDurationTabPage::implHdl(const weld::Widget* pControl)
{
    if (pControl == mxLBTrigger.get())
    {
        mxRBClickSequence->set_active(false);
        mxRBInteractive->set_active(true);
    }
}

void CustomAnimationDurationTabPage::update( STLPropertySet* pSet )
{
    auto nPos = mxLBStart->get_active();
    if (nPos != -1)
    {
        sal_Int16 nStart;
        sal_Int16 nOldStart = -1;

        switch( nPos )
        {
        case 1: nStart = EffectNodeType::WITH_PREVIOUS; break;
        case 2: nStart = EffectNodeType::AFTER_PREVIOUS; break;
        default:
            nStart = EffectNodeType::ON_CLICK; break;
        }

        if(mpSet->getPropertyState( nHandleStart ) != STLPropertyState::Ambiguous)
            mpSet->getPropertyValue( nHandleStart ) >>= nOldStart;

        if( nStart != nOldStart )
            pSet->setPropertyValue( nHandleStart, makeAny( nStart ) );
    }

    {
        double fBegin = static_cast<double>(mxMFStartDelay->get_value(FieldUnit::NONE)) / 10.0;
        double fOldBegin = -1.0;

        if( mpSet->getPropertyState( nHandleBegin ) != STLPropertyState::Ambiguous )
            mpSet->getPropertyValue( nHandleBegin ) >>= fOldBegin;

        if( fBegin != fOldBegin )
            pSet->setPropertyValue( nHandleBegin, makeAny( fBegin ) );
    }

    nPos = mxCBRepeat->get_active();
    if (nPos != -1 || !mxCBRepeat->get_active_text().isEmpty())
    {
        Any aRepeatCount;
        Any aEnd;

        switch( nPos )
        {
        case 0:
            break;

        case 6:
            {
                Event aEvent;
                aEvent.Trigger = EventTrigger::ON_NEXT;
                aEvent.Repeat = 0;
                aEnd <<= aEvent;
            }
            [[fallthrough]];
        case 7:
            aRepeatCount <<= Timing_INDEFINITE;
            break;
        default:
            {
                OUString aText(mxCBRepeat->get_text(nPos));
                if( !aText.isEmpty() )
                    aRepeatCount <<= aText.toDouble();
            }
        }

        Any aOldRepeatCount( aRepeatCount );
        if( mpSet->getPropertyState( nHandleRepeat ) != STLPropertyState::Ambiguous )
            aOldRepeatCount = mpSet->getPropertyValue( nHandleRepeat );

        if( aRepeatCount != aOldRepeatCount )
            pSet->setPropertyValue( nHandleRepeat, aRepeatCount );

        Any aOldEnd( aEnd );
        if( mpSet->getPropertyState( nHandleEnd ) != STLPropertyState::Ambiguous )
            aOldEnd = mpSet->getPropertyValue( nHandleEnd );

        if( aEnd != aOldEnd )
            pSet->setPropertyValue( nHandleEnd, aEnd );
    }

    double fDuration = -1.0;

    if (!mxCBXDuration->get_text().isEmpty())
    {
        double duration_value = static_cast<double>(mxCBXDuration->get_value(FieldUnit::NONE));

        if(duration_value > 0)
            fDuration = duration_value/100.0;
    }

    if( fDuration != -1.0 )
    {
        double fOldDuration = -1;

        if( mpSet->getPropertyState( nHandleDuration ) != STLPropertyState::Ambiguous )
            mpSet->getPropertyValue( nHandleDuration ) >>= fOldDuration;

        if( fDuration != fOldDuration )
            pSet->setPropertyValue( nHandleDuration, makeAny( fDuration ) );
    }

    if (mxCBXRewind->get_state() != TRISTATE_INDET)
    {
        sal_Int16 nFill = mxCBXRewind->get_active() ? AnimationFill::REMOVE : AnimationFill::HOLD;

        bool bSet = true;

        if( mpSet->getPropertyState( nHandleRewind ) != STLPropertyState::Ambiguous )
        {
            sal_Int16 nOldFill = 0;
            mpSet->getPropertyValue( nHandleRewind ) >>= nOldFill;
            bSet = nFill != nOldFill;
        }

        if( bSet )
            pSet->setPropertyValue( nHandleRewind, makeAny( nFill ) );
    }

    Reference< XShape > xTrigger;

    if (mxRBInteractive->get_active())
    {
        nPos = mxLBTrigger->get_active();
        if (nPos != -1)
        {
            sal_Int32 nShape = mxLBTrigger->get_id(nPos).toInt32();

            Reference< XDrawPage > xCurrentPage;
            mpSet->getPropertyValue( nHandleCurrentPage ) >>= xCurrentPage;

            if( xCurrentPage.is() && (nShape >= 0) && (nShape < xCurrentPage->getCount()) )
                xCurrentPage->getByIndex( nShape ) >>= xTrigger;
        }
    }

    if (xTrigger.is() || mxRBClickSequence->get_active())
    {
        Any aNewValue( makeAny( xTrigger ) );
        Any aOldValue;

        if( mpSet->getPropertyState( nHandleTrigger ) != STLPropertyState::Ambiguous )
            aOldValue = mpSet->getPropertyValue( nHandleTrigger );

        if( aNewValue != aOldValue )
            pSet->setPropertyValue( nHandleTrigger, aNewValue );
    }
}

class CustomAnimationTextAnimTabPage
{
public:
    CustomAnimationTextAnimTabPage(weld::Container* pParent, const STLPropertySet* pSet);

    void update( STLPropertySet* pSet );

    void updateControlStates();
    DECL_LINK(implSelectHdl, weld::ComboBox&, void);

private:
    const STLPropertySet* mpSet;
    bool mbHasVisibleShapes;

    std::unique_ptr<weld::Builder> mxBuilder;
    std::unique_ptr<weld::Container> mxContainer;
    std::unique_ptr<weld::Label> mxFTGroupText;
    std::unique_ptr<weld::ComboBox> mxLBGroupText;
    std::unique_ptr<weld::CheckButton> mxCBXGroupAuto;
    std::unique_ptr<weld::MetricSpinButton> mxMFGroupAuto;
    std::unique_ptr<weld::CheckButton> mxCBXAnimateForm;
    std::unique_ptr<weld::CheckButton> mxCBXReverse;
};

CustomAnimationTextAnimTabPage::CustomAnimationTextAnimTabPage(weld::Container* pParent, const STLPropertySet* pSet)
    : mpSet(pSet)
    , mbHasVisibleShapes(true)
    , mxBuilder(Application::CreateBuilder(pParent, "modules/simpress/ui/customanimationtexttab.ui"))
    , mxContainer(mxBuilder->weld_container("TextAnimationTab"))
    , mxFTGroupText(mxBuilder->weld_label("group_text_label"))
    , mxLBGroupText(mxBuilder->weld_combo_box("group_text_list"))
    , mxCBXGroupAuto(mxBuilder->weld_check_button("auto_after"))
    , mxMFGroupAuto(mxBuilder->weld_metric_spin_button("auto_after_value",FieldUnit::SECOND))
    , mxCBXAnimateForm(mxBuilder->weld_check_button("animate_shape"))
    , mxCBXReverse(mxBuilder->weld_check_button("reverse_order"))
{
    mxLBGroupText->connect_changed(LINK(this, CustomAnimationTextAnimTabPage, implSelectHdl));

    if( pSet->getPropertyState( nHandleTextGrouping ) != STLPropertyState::Ambiguous )
    {
        sal_Int32 nTextGrouping = 0;
        if( pSet->getPropertyValue( nHandleTextGrouping ) >>= nTextGrouping )
            mxLBGroupText->set_active(nTextGrouping + 1);
    }

    if( pSet->getPropertyState( nHandleHasVisibleShape ) != STLPropertyState::Ambiguous )
        pSet->getPropertyValue( nHandleHasVisibleShape ) >>= mbHasVisibleShapes;

    if( pSet->getPropertyState( nHandleTextGroupingAuto ) != STLPropertyState::Ambiguous )
    {
        double fTextGroupingAuto = 0.0;
        if( pSet->getPropertyValue( nHandleTextGroupingAuto ) >>= fTextGroupingAuto )
        {
            mxCBXGroupAuto->set_active(fTextGroupingAuto >= 0.0);
            if( fTextGroupingAuto >= 0.0 )
                mxMFGroupAuto->set_value(static_cast<::tools::Long>(fTextGroupingAuto*10), FieldUnit::NONE);
        }
    }
    else
    {
        mxCBXGroupAuto->set_state( TRISTATE_INDET );
    }

    mxCBXAnimateForm->set_state( TRISTATE_INDET );
    if( pSet->getPropertyState( nHandleAnimateForm ) != STLPropertyState::Ambiguous )
    {
        bool bAnimateForm = false;
        if( pSet->getPropertyValue( nHandleAnimateForm ) >>= bAnimateForm )
        {
            mxCBXAnimateForm->set_active( bAnimateForm );
        }
    }
    else
    {
        mxCBXAnimateForm->set_sensitive(false);
    }

    mxCBXReverse->set_state(TRISTATE_INDET);
    if( pSet->getPropertyState( nHandleTextReverse ) != STLPropertyState::Ambiguous )
    {
        bool bTextReverse = false;
        if( pSet->getPropertyValue( nHandleTextReverse ) >>= bTextReverse )
        {
            mxCBXReverse->set_active( bTextReverse );
        }
    }

    if( pSet->getPropertyState( nHandleMaxParaDepth ) == STLPropertyState::Direct )
    {
        sal_Int32 nMaxParaDepth = 0;
        pSet->getPropertyValue( nHandleMaxParaDepth ) >>= nMaxParaDepth;
        nMaxParaDepth += 1;

        sal_Int32 nPos = 6;
        while( (nPos > 2) && (nPos > nMaxParaDepth) )
        {
            mxLBGroupText->remove(nPos);
            nPos--;
        }
    }

    updateControlStates();
}

void CustomAnimationTextAnimTabPage::update( STLPropertySet* pSet )
{
    auto nPos = mxLBGroupText->get_active();
    if (nPos != -1)
    {
        sal_Int32 nTextGrouping = nPos - 1;
        sal_Int32 nOldGrouping = -2;

        if(mpSet->getPropertyState( nHandleTextGrouping ) != STLPropertyState::Ambiguous)
            mpSet->getPropertyValue( nHandleTextGrouping ) >>= nOldGrouping;

        if( nTextGrouping != nOldGrouping )
            pSet->setPropertyValue( nHandleTextGrouping, makeAny( nTextGrouping ) );
    }

    if (nPos != 0)
    {
        bool bTextReverse = mxCBXReverse->get_active();
        bool bOldTextReverse = !bTextReverse;

        if(mpSet->getPropertyState( nHandleTextReverse ) != STLPropertyState::Ambiguous)
            mpSet->getPropertyValue( nHandleTextReverse ) >>= bOldTextReverse;

        if( bTextReverse != bOldTextReverse )
            pSet->setPropertyValue( nHandleTextReverse, makeAny( bTextReverse ) );

        if( nPos > 1 )
        {
            double fTextGroupingAuto = mxCBXGroupAuto->get_active() ? mxMFGroupAuto->get_value(FieldUnit::NONE) / 10.0 : -1.0;
            double fOldTextGroupingAuto = -2.0;

            if(mpSet->getPropertyState( nHandleTextGroupingAuto ) != STLPropertyState::Ambiguous)
                mpSet->getPropertyValue( nHandleTextGroupingAuto ) >>= fOldTextGroupingAuto;

            if( fTextGroupingAuto != fOldTextGroupingAuto )
                pSet->setPropertyValue( nHandleTextGroupingAuto, makeAny( fTextGroupingAuto ) );
        }
    }
    //#i120049# impress crashes when modifying the "Random effects" animation
    //effect's trigger condition to "Start effect on click of".
    //If this control is disabled, we should ignore its value
    if (mxCBXAnimateForm->get_sensitive())
    {
        bool bAnimateForm = mxCBXAnimateForm->get_active();
        bool bOldAnimateForm = !bAnimateForm;

        if(mpSet->getPropertyState( nHandleAnimateForm ) != STLPropertyState::Ambiguous)
            mpSet->getPropertyValue( nHandleAnimateForm ) >>= bOldAnimateForm;

        if( bAnimateForm != bOldAnimateForm )
            pSet->setPropertyValue( nHandleAnimateForm, makeAny( bAnimateForm ) );
    }
}

void CustomAnimationTextAnimTabPage::updateControlStates()
{
    auto nPos = mxLBGroupText->get_active();

    mxCBXGroupAuto->set_sensitive( nPos > 1 );
    mxMFGroupAuto->set_sensitive( nPos > 1 );
    mxCBXReverse->set_sensitive( nPos > 0 );

    if( !mbHasVisibleShapes && nPos > 0 )
    {
        mxCBXAnimateForm->set_active(false);
        mxCBXAnimateForm->set_sensitive(false);
    }
    else
    {
        mxCBXAnimateForm->set_sensitive(true);
    }
}

IMPL_LINK_NOARG(CustomAnimationTextAnimTabPage, implSelectHdl, weld::ComboBox&, void)
{
    updateControlStates();
}

CustomAnimationDialog::CustomAnimationDialog(weld::Window* pParent, std::unique_ptr<STLPropertySet> pSet, const OString& rPage)
    : GenericDialogController(pParent, "modules/simpress/ui/customanimationproperties.ui", "CustomAnimationProperties")
    , mxSet(std::move(pSet))
    , mxTabControl(m_xBuilder->weld_notebook("tabcontrol"))
    , mxDurationTabPage(new CustomAnimationDurationTabPage(mxTabControl->get_page("timing"), mxSet.get()))
    , mxEffectTabPage(new CustomAnimationEffectTabPage(mxTabControl->get_page("effect"), m_xDialog.get(), mxSet.get()))
{
    bool bHasText = false;
    if( mxSet->getPropertyState( nHandleHasText ) != STLPropertyState::Ambiguous )
        mxSet->getPropertyValue( nHandleHasText ) >>= bHasText;

    if( bHasText )
    {
        mxTextAnimTabPage.reset(new CustomAnimationTextAnimTabPage(mxTabControl->get_page("textanim"), mxSet.get()));
    }
    else
    {
        mxTabControl->remove_page("textanim");
    }

    if (!rPage.isEmpty())
        mxTabControl->set_current_page(rPage);
}

CustomAnimationDialog::~CustomAnimationDialog()
{
}

STLPropertySet* CustomAnimationDialog::getResultSet()
{
    mxResultSet = createDefaultSet();

    mxEffectTabPage->update( mxResultSet.get() );
    mxDurationTabPage->update( mxResultSet.get() );
    if (mxTextAnimTabPage)
        mxTextAnimTabPage->update( mxResultSet.get() );

    return mxResultSet.get();
}

std::unique_ptr<STLPropertySet> CustomAnimationDialog::createDefaultSet()
{
    Any aEmpty;

    std::unique_ptr<STLPropertySet> pSet(new STLPropertySet());
    pSet->setPropertyDefaultValue( nHandleMaxParaDepth, makeAny( sal_Int32(-1) ) );

    pSet->setPropertyDefaultValue( nHandleHasAfterEffect, makeAny( false ) );
    pSet->setPropertyDefaultValue( nHandleAfterEffectOnNextEffect, makeAny( false ) );
    pSet->setPropertyDefaultValue( nHandleDimColor, aEmpty );
    pSet->setPropertyDefaultValue( nHandleIterateType, makeAny( sal_Int16(0) ) );
    pSet->setPropertyDefaultValue( nHandleIterateInterval, makeAny( 0.0 ) );

    pSet->setPropertyDefaultValue( nHandleStart, makeAny( sal_Int16(EffectNodeType::ON_CLICK) ) );
    pSet->setPropertyDefaultValue( nHandleBegin, makeAny( 0.0 ) );
    pSet->setPropertyDefaultValue( nHandleDuration, makeAny( 2.0 ) );
    pSet->setPropertyDefaultValue( nHandleRepeat, aEmpty );
    pSet->setPropertyDefaultValue( nHandleRewind, makeAny( AnimationFill::HOLD ) );

    pSet->setPropertyDefaultValue( nHandleEnd, aEmpty );

    pSet->setPropertyDefaultValue( nHandlePresetId, aEmpty );
    pSet->setPropertyDefaultValue( nHandleProperty1Type, makeAny( nPropertyTypeNone ) );
    pSet->setPropertyDefaultValue( nHandleProperty1Value, aEmpty );
    pSet->setPropertyDefaultValue( nHandleProperty2Type, makeAny( nPropertyTypeNone ) );
    pSet->setPropertyDefaultValue( nHandleProperty2Value, aEmpty );
    pSet->setPropertyDefaultValue( nHandleAccelerate, aEmpty );
    pSet->setPropertyDefaultValue( nHandleDecelerate, aEmpty );
    pSet->setPropertyDefaultValue( nHandleAutoReverse, aEmpty );
    pSet->setPropertyDefaultValue( nHandleTrigger, aEmpty );

    pSet->setPropertyDefaultValue( nHandleHasText, makeAny( false ) );
    pSet->setPropertyDefaultValue( nHandleHasVisibleShape, makeAny( false ) );
    pSet->setPropertyDefaultValue( nHandleTextGrouping, makeAny( sal_Int32(-1) ) );
    pSet->setPropertyDefaultValue( nHandleAnimateForm, makeAny( true ) );
    pSet->setPropertyDefaultValue( nHandleTextGroupingAuto, makeAny( -1.0 ) );
    pSet->setPropertyDefaultValue( nHandleTextReverse, makeAny( false ) );

    pSet->setPropertyDefaultValue( nHandleCurrentPage, aEmpty );

    pSet->setPropertyDefaultValue( nHandleSoundURL, aEmpty );
    pSet->setPropertyDefaultValue( nHandleSoundVolume, makeAny( 1.0) );
    pSet->setPropertyDefaultValue( nHandleSoundEndAfterSlide, makeAny( sal_Int32(0) ) );

    pSet->setPropertyDefaultValue( nHandleCommand, makeAny( sal_Int16(0) ) );
    return pSet;
}

std::unique_ptr<SdPropertySubControl> SdPropertySubControl::create(sal_Int32 nType, weld::Label* pLabel, weld::Container* pParent, weld::Window* pTopLevel, const Any& rValue, const OUString& rPresetId, const Link<LinkParamNone*,void>& rModifyHdl)
{
    std::unique_ptr<SdPropertySubControl> pSubControl;
    switch( nType )
    {
    case nPropertyTypeDirection:
    case nPropertyTypeSpokes:
    case nPropertyTypeZoom:
        pSubControl.reset( new SdPresetPropertyBox( pLabel, pParent, rValue, rPresetId, rModifyHdl ) );
        break;

    case nPropertyTypeColor:
    case nPropertyTypeFillColor:
    case nPropertyTypeFirstColor:
    case nPropertyTypeCharColor:
    case nPropertyTypeLineColor:
        pSubControl.reset( new SdColorPropertyBox( pLabel, pParent, pTopLevel, rValue, rModifyHdl ) );
        break;

    case nPropertyTypeFont:
        pSubControl.reset( new SdFontPropertyBox( pLabel, pParent, rValue, rModifyHdl ) );
        break;

    case nPropertyTypeCharHeight:
        pSubControl.reset( new SdCharHeightPropertyBox( pLabel, pParent, rValue, rModifyHdl ) );
        break;

    case nPropertyTypeRotate:
        pSubControl.reset( new SdRotationPropertyBox( pLabel, pParent, rValue, rModifyHdl ) );
        break;

    case nPropertyTypeTransparency:
        pSubControl.reset( new SdTransparencyPropertyBox( pLabel, pParent, rValue, rModifyHdl ) );
        break;

    case nPropertyTypeScale:
        pSubControl.reset( new SdScalePropertyBox( pLabel, pParent, rValue, rModifyHdl ) );
        break;

    case nPropertyTypeCharDecoration:
        pSubControl.reset( new SdFontStylePropertyBox( pLabel, pParent, rValue, rModifyHdl ) );
        break;
    }

    return pSubControl;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
