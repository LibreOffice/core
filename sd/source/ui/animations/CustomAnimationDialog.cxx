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
#include <com/sun/star/media/XManager.hpp>
#include <com/sun/star/media/XPlayer.hpp>

#include <memory>

#include <i18nutil/unicode.hxx>
#include <unotools/pathoptions.hxx>
#include <vcl/tabctrl.hxx>
#include <vcl/tabpage.hxx>
#include <vcl/menubtn.hxx>
#include <vcl/svapp.hxx>
#include <vcl/fixed.hxx>
#include <vcl/field.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/layout.hxx>
#include <vcl/weld.hxx>
#include <vcl/decoview.hxx>
#include <vcl/combobox.hxx>
#include <vcl/menu.hxx>
#include <vcl/settings.hxx>
#include <vcl/builderfactory.hxx>

#include <svtools/ctrlbox.hxx>
#include <svtools/ctrltool.hxx>
#include <sfx2/objsh.hxx>

#include <svx/svxids.hrc>
#include <editeng/flstitem.hxx>
#include <svx/drawitem.hxx>

#include <svx/colorbox.hxx>
#include <svx/xtable.hxx>
#include <svx/gallery.hxx>

#include <svx/dialogs.hrc>
#include <sdresid.hxx>

#include "CustomAnimationDialog.hxx"
#include "CustomAnimationPane.hxx"
#include "STLPropertySet.hxx"

#include <avmedia/mediawindow.hxx>

#include <filedlg.hxx>
#include <strings.hrc>
#include <helpids.h>

using namespace ::com::sun::star;
using namespace ::com::sun::star::animations;
using namespace ::com::sun::star::presentation;

using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::makeAny;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::drawing::XShape;
using ::com::sun::star::drawing::XDrawPage;
using ::com::sun::star::beans::XPropertySet;

namespace sd {

class PresetPropertyBox  : public PropertySubControl
{
public:
    PresetPropertyBox( sal_Int32 nControlType, vcl::Window* pParent, const Any& rValue, const OUString& aPresetId, const Link<LinkParamNone*,void>& rModifyHdl );
    virtual ~PresetPropertyBox() override;

    virtual Any getValue() override;
    virtual void setValue( const Any& rValue, const OUString& rPresetId ) override;
    virtual Control* getControl() override;

private:
    std::map< sal_uInt16, OUString > maPropertyValues;
    VclPtr<ListBox> mpControl;
    DECL_LINK(OnSelect, ListBox&, void);
    Link<LinkParamNone*,void> maModifyLink;
};

PresetPropertyBox::PresetPropertyBox( sal_Int32 nControlType, vcl::Window* pParent, const Any& rValue, const OUString& aPresetId, const Link<LinkParamNone*,void>& rModifyHdl )
: PropertySubControl( nControlType ), maModifyLink(rModifyHdl)
{
    mpControl = VclPtr<ListBox>::Create( pParent, WB_BORDER|WB_TABSTOP|WB_DROPDOWN );
    mpControl->set_hexpand(true);
    mpControl->SetDropDownLineCount( 10 );
    mpControl->SetSelectHdl( LINK(this, PresetPropertyBox, OnSelect) );
    mpControl->SetHelpId( HID_SD_CUSTOMANIMATIONPANE_PRESETPROPERTYBOX );

    setValue( rValue, aPresetId );
}

IMPL_LINK_NOARG(PresetPropertyBox, OnSelect, ListBox&, void)
{
    maModifyLink.Call(nullptr);
}

void PresetPropertyBox::setValue( const Any& rValue, const OUString& rPresetId )
{
    if( mpControl )
    {
        mpControl->Clear();

        const CustomAnimationPresets& rPresets = CustomAnimationPresets::getCustomAnimationPresets();
        CustomAnimationPresetPtr pDescriptor = rPresets.getEffectDescriptor( rPresetId );
        if( pDescriptor.get() )
        {

            OUString aPropertyValue;
            rValue >>= aPropertyValue;

            std::vector<OUString> aSubTypes( pDescriptor->getSubTypes() );
            std::vector<OUString>::iterator aIter( aSubTypes.begin() );
            const std::vector<OUString>::iterator aEnd( aSubTypes.end() );

            mpControl->Enable( aIter != aEnd );

            while( aIter != aEnd )
            {
                sal_Int32 nPos = mpControl->InsertEntry( rPresets.getUINameForProperty( *aIter ) );
                if( (*aIter) == aPropertyValue )
                    mpControl->SelectEntryPos( nPos );
                maPropertyValues[nPos] = (*aIter++);
            }
        }
        else
        {
            mpControl->Enable( false );
        }
    }
}

PresetPropertyBox::~PresetPropertyBox()
{
    mpControl.disposeAndClear();
}

Any PresetPropertyBox::getValue()
{
    return makeAny( maPropertyValues[mpControl->GetSelectedEntryPos()] );
}

Control* PresetPropertyBox::getControl()
{
    return mpControl;
}

class ColorPropertyBox  : public PropertySubControl
{
public:
    ColorPropertyBox( sal_Int32 nControlType, vcl::Window* pParent, const Any& rValue, const Link<LinkParamNone*,void>& rModifyHdl );
    virtual ~ColorPropertyBox() override;

    virtual Any getValue() override;
    virtual void setValue( const Any& rValue, const OUString& rPresetId  ) override;
    virtual Control* getControl() override;

private:
    VclPtr<SvxColorListBox> mpControl;
    DECL_LINK(OnSelect, SvxColorListBox&, void);
    Link<LinkParamNone*,void> maModifyLink;
};

ColorPropertyBox::ColorPropertyBox( sal_Int32 nControlType, vcl::Window* pParent, const Any& rValue, const Link<LinkParamNone*,void>& rModifyHdl )
: PropertySubControl( nControlType ), maModifyLink(rModifyHdl)
{
    mpControl = VclPtr<SvxColorListBox>::Create(pParent);
    mpControl->set_hexpand(true);
    mpControl->SetSelectHdl( LINK(this, ColorPropertyBox, OnSelect) );
    mpControl->SetHelpId( HID_SD_CUSTOMANIMATIONPANE_COLORPROPERTYBOX );

    sal_Int32 nColor = 0;
    rValue >>= nColor;
    mpControl->SelectEntry(Color(nColor));
}

IMPL_LINK_NOARG(ColorPropertyBox, OnSelect, SvxColorListBox&, void)
{
    maModifyLink.Call(nullptr);
}

ColorPropertyBox::~ColorPropertyBox()
{
    mpControl.disposeAndClear();
}

void ColorPropertyBox::setValue( const Any& rValue, const OUString& )
{
    if( mpControl )
    {
        sal_Int32 nColor = 0;
        rValue >>= nColor;

        mpControl->SetNoSelection();
        mpControl->SelectEntry(Color(nColor));
    }
}

Any ColorPropertyBox::getValue()
{
    return makeAny( sal_Int32(mpControl->GetSelectEntryColor().GetRGBColor()) );
}

Control* ColorPropertyBox::getControl()
{
    return mpControl;
}

class FontPropertyBox : public PropertySubControl
{
public:
    FontPropertyBox( sal_Int32 nControlType, vcl::Window* pParent, const Any& rValue, const Link<LinkParamNone*,void>& rModifyHdl );
    virtual ~FontPropertyBox() override;

    virtual Any getValue() override;
    virtual void setValue( const Any& rValue, const OUString& rPresetId  ) override;

    virtual Control* getControl() override;

private:
    VclPtr<FontNameBox>         mpControl;
    Link<LinkParamNone*,void>   maModifyHdl;
    DECL_LINK(ControlSelectHdl, ComboBox&, void);
};

FontPropertyBox::FontPropertyBox( sal_Int32 nControlType, vcl::Window* pParent, const Any& rValue, const Link<LinkParamNone*,void>& rModifyHdl )
: PropertySubControl( nControlType ), maModifyHdl(rModifyHdl)
{
    mpControl = VclPtr<FontNameBox>::Create( pParent, WB_BORDER|WB_TABSTOP|WB_DROPDOWN );
    mpControl->set_hexpand(true);
    mpControl->SetDropDownLineCount( 10 );
    mpControl->SetSelectHdl( LINK(this, FontPropertyBox, ControlSelectHdl) );
    mpControl->SetHelpId( HID_SD_CUSTOMANIMATIONPANE_FONTPROPERTYBOX );

    SfxObjectShell* pDocSh = SfxObjectShell::Current();
    const SfxPoolItem* pItem;

    const FontList* pFontList = nullptr;
    bool bMustDelete = false;

    if ( pDocSh && ( (pItem = pDocSh->GetItem( SID_ATTR_CHAR_FONTLIST ) ) != nullptr) )
        pFontList = static_cast<const SvxFontListItem*>(pItem)->GetFontList();

    if(!pFontList)
    {
        pFontList = new FontList(Application::GetDefaultDevice(), nullptr);
        bMustDelete = true;
    }

    mpControl->Fill( pFontList );

    if( bMustDelete )
        delete pFontList;

    setValue( rValue, OUString() );
}

IMPL_LINK_NOARG(FontPropertyBox, ControlSelectHdl, ComboBox&, void)
{
    maModifyHdl.Call(nullptr);
}

void FontPropertyBox::setValue( const Any& rValue, const OUString& )
{
    if( mpControl )
    {
        OUString aFontName;
        rValue >>= aFontName;
        mpControl->SetText( aFontName );
    }
}

FontPropertyBox::~FontPropertyBox()
{
    mpControl.disposeAndClear();
}

Any FontPropertyBox::getValue()
{
    OUString aFontName( mpControl->GetText() );
    return makeAny( aFontName );
}

Control* FontPropertyBox::getControl()
{
    return mpControl;
}

class DropdownMenuBox : public Edit
{
public:
    DropdownMenuBox( vcl::Window* pParent, Edit* pSubControl, PopupMenu* pMenu );
    virtual ~DropdownMenuBox() override;
    virtual void dispose() override;

    void Resize() override;
    bool PreNotify( NotifyEvent& rNEvt ) override;

    void SetMenuSelectHdl( const Link<MenuButton *, void>& rLink ) { mpDropdownButton->SetSelectHdl( rLink ); }

private:
    VclPtr<Edit>        mpSubControl;
    VclPtr<MenuButton>  mpDropdownButton;
    VclPtr<PopupMenu>   mpMenu;
};

DropdownMenuBox::DropdownMenuBox( vcl::Window* pParent, Edit* pSubControl, PopupMenu* pMenu )
:   Edit( pParent, WB_BORDER|WB_TABSTOP| WB_DIALOGCONTROL ),
    mpSubControl(pSubControl),mpDropdownButton(nullptr),mpMenu(pMenu)
{
    mpDropdownButton = VclPtr<MenuButton>::Create( this, WB_NOLIGHTBORDER | WB_RECTSTYLE | WB_NOTABSTOP);
    mpDropdownButton->SetSymbol(SymbolType::SPIN_DOWN);
    mpDropdownButton->Show();
    mpDropdownButton->SetPopupMenu( pMenu );

    SetSubEdit( mpSubControl );
    set_hexpand(true);
    mpSubControl->SetParent( this );
    mpSubControl->Show();
}

DropdownMenuBox::~DropdownMenuBox()
{
    disposeOnce();
}

void DropdownMenuBox::dispose()
{
    SetSubEdit(nullptr);
    mpDropdownButton.disposeAndClear();
    mpMenu.disposeAndClear();
    mpSubControl.disposeAndClear();
    Edit::dispose();
}

void DropdownMenuBox::Resize()
{
    Size aOutSz = GetOutputSizePixel();

    long nSBWidth = GetSettings().GetStyleSettings().GetScrollBarSize();
    nSBWidth = CalcZoom( nSBWidth );
    mpSubControl->setPosSizePixel( 0, 1, aOutSz.Width() - nSBWidth, aOutSz.Height()-2 );
    mpDropdownButton->setPosSizePixel( aOutSz.Width() - nSBWidth, 0, nSBWidth, aOutSz.Height() );
}

bool DropdownMenuBox::PreNotify( NotifyEvent& rNEvt )
{
    bool bResult = true;

    MouseNotifyEvent nSwitch=rNEvt.GetType();
    if (nSwitch==MouseNotifyEvent::KEYINPUT)
    {
        const vcl::KeyCode& aKeyCode=rNEvt.GetKeyEvent()->GetKeyCode();
        sal_uInt16 nKey=aKeyCode.GetCode();

        if (nKey==KEY_DOWN && aKeyCode.IsMod2())
        {
            mpDropdownButton->KeyInput( *rNEvt.GetKeyEvent() );
        }
        else
        {
            bResult=Edit::PreNotify(rNEvt);
        }
    }
    else
        bResult=Edit::PreNotify(rNEvt);

    return bResult;
}

class CharHeightPropertyBox : public PropertySubControl
{
public:
    CharHeightPropertyBox( sal_Int32 nControlType, vcl::Window* pParent, const Any& rValue, const Link<LinkParamNone*,void>& rModifyHdl );
    virtual ~CharHeightPropertyBox() override;

    virtual Any getValue() override;
    virtual void setValue( const Any& rValue, const OUString& ) override;

    virtual Control* getControl() override;

    DECL_LINK( implMenuSelectHdl, MenuButton*, void );

private:
    DECL_LINK( EditModifyHdl, Edit&, void );
    VclBuilder maBuilder;
    VclPtr<DropdownMenuBox> mpControl;
    VclPtr<PopupMenu> mpMenu;
    VclPtr<MetricField> mpMetric;
    Link<LinkParamNone*,void> maModifyHdl;
};

CharHeightPropertyBox::CharHeightPropertyBox(sal_Int32 nControlType, vcl::Window* pParent, const Any& rValue, const Link<LinkParamNone*,void>& rModifyHdl)
    : PropertySubControl(nControlType)
    , maBuilder(nullptr, VclBuilderContainer::getUIRootDir(), "modules/simpress/ui/fontsizemenu.ui", "")
    , maModifyHdl(rModifyHdl)
{
    mpMetric.set( VclPtr<MetricField>::Create( pParent, WB_TABSTOP|WB_IGNORETAB| WB_NOBORDER) );
    mpMetric->SetUnit( FUNIT_PERCENT );
    mpMetric->SetMin( 0 );
    mpMetric->SetMax( 1000 );

    mpMenu = maBuilder.get_menu("menu");
    mpControl = VclPtr<DropdownMenuBox>::Create( pParent, mpMetric, mpMenu );
    mpControl->SetMenuSelectHdl( LINK( this, CharHeightPropertyBox, implMenuSelectHdl ));
    mpControl->SetModifyHdl( LINK( this, CharHeightPropertyBox, EditModifyHdl ) );
    mpControl->SetHelpId( HID_SD_CUSTOMANIMATIONPANE_CHARHEIGHTPROPERTYBOX );

    setValue( rValue, OUString() );
}

CharHeightPropertyBox::~CharHeightPropertyBox()
{
    maBuilder.disposeBuilder();
    mpControl.disposeAndClear();
}

IMPL_LINK_NOARG( CharHeightPropertyBox, EditModifyHdl, Edit&, void )
{
    maModifyHdl.Call(nullptr);
}

IMPL_LINK( CharHeightPropertyBox, implMenuSelectHdl, MenuButton*, pPb, void )
{
    sal_Int32 nValue = pPb->GetCurItemIdent().toInt32();
    mpMetric->SetValue(nValue);
    mpMetric->Modify();
}

void CharHeightPropertyBox::setValue( const Any& rValue, const OUString& )
{
    if( mpMetric.get() )
    {
        double fValue = 0.0;
        rValue >>= fValue;
        mpMetric->SetValue( static_cast<long>(fValue * 100.0) );
    }
}

Any CharHeightPropertyBox::getValue()
{
    return makeAny( static_cast<double>(mpMetric->GetValue()) / 100.0 );
}

Control* CharHeightPropertyBox::getControl()
{
    return mpControl;
}

class TransparencyPropertyBox : public PropertySubControl
{
public:
    TransparencyPropertyBox( sal_Int32 nControlType, vcl::Window* pParent, const Any& rValue, const Link<LinkParamNone*,void>& rModifyHdl );
    virtual ~TransparencyPropertyBox() override;

    virtual Any getValue() override;
    virtual void setValue( const Any& rValue, const OUString& rPresetId  ) override;

    virtual Control* getControl() override;

    DECL_LINK( implMenuSelectHdl, MenuButton*, void );
    DECL_LINK( implModifyHdl, Edit&, void );

    void updateMenu();

private:
    VclPtr<DropdownMenuBox>   mpControl;
    VclPtr<PopupMenu>         mpMenu;
    VclPtr<MetricField>       mpMetric;
    Link<LinkParamNone*,void> maModifyHdl;
};

TransparencyPropertyBox::TransparencyPropertyBox( sal_Int32 nControlType, vcl::Window* pParent, const Any& rValue, const Link<LinkParamNone*,void>& rModifyHdl )
: PropertySubControl( nControlType )
, maModifyHdl( rModifyHdl )
{
    mpMetric.set( VclPtr<MetricField>::Create( pParent ,WB_TABSTOP|WB_IGNORETAB| WB_NOBORDER) );
    mpMetric->SetUnit( FUNIT_PERCENT );
    mpMetric->SetMin( 0 );
    mpMetric->SetMax( 100 );

    mpMenu = VclPtr<PopupMenu>::Create();
    for( sal_Int32 i = 25; i < 101; i += 25 )
    {
        OUString aStr(unicode::formatPercent(i,
            Application::GetSettings().GetUILanguageTag()));
        mpMenu->InsertItem( i, aStr );
    }

    mpControl = VclPtr<DropdownMenuBox>::Create( pParent, mpMetric, mpMenu );
    mpControl->SetMenuSelectHdl( LINK( this, TransparencyPropertyBox, implMenuSelectHdl ));
    mpControl->SetHelpId( HID_SD_CUSTOMANIMATIONPANE_TRANSPARENCYPROPERTYBOX );

    Link<Edit&,void> aLink( LINK( this, TransparencyPropertyBox, implModifyHdl ) );
    mpControl->SetModifyHdl( aLink );

    setValue( rValue, OUString()  );
}

TransparencyPropertyBox::~TransparencyPropertyBox()
{
    mpControl.disposeAndClear();
}

void TransparencyPropertyBox::updateMenu()
{
    sal_Int64 nValue = mpMetric->GetValue();
    for( sal_uInt16 i = 25; i < 101; i += 25 )
        mpMenu->CheckItem( i, nValue == i );
}

IMPL_LINK_NOARG(TransparencyPropertyBox, implModifyHdl, Edit&, void)
{
    updateMenu();
    maModifyHdl.Call(nullptr);
}

IMPL_LINK( TransparencyPropertyBox, implMenuSelectHdl, MenuButton*, pPb, void )
{
    if( pPb->GetCurItemId() != mpMetric->GetValue() )
    {
        mpMetric->SetValue( pPb->GetCurItemId() );
        mpMetric->Modify();
    }
}

void TransparencyPropertyBox::setValue( const Any& rValue, const OUString& )
{
    if( mpMetric.get() )
    {
        double fValue = 0.0;
        rValue >>= fValue;
        long nValue = static_cast<long>(fValue * 100);
        mpMetric->SetValue( nValue );
        updateMenu();
    }
}

Any TransparencyPropertyBox::getValue()
{
    return makeAny( static_cast<double>(mpMetric->GetValue()) / 100.0 );
}

Control* TransparencyPropertyBox::getControl()
{
    return mpControl;
}

class RotationPropertyBox : public PropertySubControl
{
public:
    RotationPropertyBox( sal_Int32 nControlType, vcl::Window* pParent, const Any& rValue, const Link<LinkParamNone*,void>& rModifyHdl );
    virtual ~RotationPropertyBox() override;

    virtual Any getValue() override;
    virtual void setValue( const Any& rValue, const OUString& ) override;

    virtual Control* getControl() override;

    DECL_LINK( implMenuSelectHdl, MenuButton*, void );
    DECL_LINK( implModifyHdl, Edit&, void );

    void updateMenu();

private:
    VclBuilder                maBuilder;
    VclPtr<DropdownMenuBox>   mpControl;
    VclPtr<PopupMenu>         mpMenu;
    VclPtr<MetricField>       mpMetric;
    Link<LinkParamNone*,void> maModifyHdl;
};

RotationPropertyBox::RotationPropertyBox( sal_Int32 nControlType, vcl::Window* pParent, const Any& rValue, const Link<LinkParamNone*,void>& rModifyHdl )
    : PropertySubControl(nControlType)
    , maBuilder(nullptr, VclBuilderContainer::getUIRootDir(), "modules/simpress/ui/rotatemenu.ui", "")
    , maModifyHdl(rModifyHdl)
{
    mpMetric.set( VclPtr<MetricField>::Create( pParent ,WB_TABSTOP|WB_IGNORETAB| WB_NOBORDER) );
    mpMetric->SetUnit( FUNIT_CUSTOM );
    mpMetric->SetCustomUnitText( OUString( u'\x00b0') ); // degree sign
    mpMetric->SetMin( -10000 );
    mpMetric->SetMax( 10000 );

    mpMenu = maBuilder.get_menu("menu");
    mpControl = VclPtr<DropdownMenuBox>::Create( pParent, mpMetric, mpMenu );
    mpControl->SetMenuSelectHdl( LINK( this, RotationPropertyBox, implMenuSelectHdl ));
    mpControl->SetHelpId( HID_SD_CUSTOMANIMATIONPANE_ROTATIONPROPERTYBOX );

    Link<Edit&,void> aLink( LINK( this, RotationPropertyBox, implModifyHdl ) );
    mpControl->SetModifyHdl( aLink );

    setValue( rValue, OUString() );
}

RotationPropertyBox::~RotationPropertyBox()
{
    maBuilder.disposeBuilder();
    mpControl.disposeAndClear();
}

void RotationPropertyBox::updateMenu()
{
    sal_Int64 nValue = mpMetric->GetValue();
    bool bDirection = nValue >= 0;
    nValue = (nValue < 0 ? -nValue : nValue);

    mpMenu->CheckItem("90", nValue == 90);
    mpMenu->CheckItem("180", nValue == 180);
    mpMenu->CheckItem("360", nValue == 360);
    mpMenu->CheckItem("720", nValue == 720);

    mpMenu->CheckItem("closewise", bDirection);
    mpMenu->CheckItem("counterclock", !bDirection);
}

IMPL_LINK_NOARG(RotationPropertyBox, implModifyHdl, Edit&, void)
{
    updateMenu();
    maModifyHdl.Call(nullptr);
}

IMPL_LINK( RotationPropertyBox, implMenuSelectHdl, MenuButton*, pPb, void )
{
    sal_Int64 nValue = mpMetric->GetValue();
    bool bDirection = nValue >= 0;
    nValue = (nValue < 0 ? -nValue : nValue);

    OString sIdent = pPb->GetCurItemIdent();
    if (sIdent == "clockwise")
        bDirection = true;
    else if (sIdent == "counterclock")
        bDirection = false;
    else
        nValue = sIdent.toInt32();

    if( !bDirection )
        nValue = -nValue;

    if( nValue != mpMetric->GetValue() )
    {
        mpMetric->SetValue( nValue );
        mpMetric->Modify();
    }
}

void RotationPropertyBox::setValue( const Any& rValue, const OUString& )
{
    if( mpMetric.get() )
    {
        double fValue = 0.0;
        rValue >>= fValue;
        long nValue = static_cast<long>(fValue);
        mpMetric->SetValue( nValue );
        updateMenu();
    }
}

Any RotationPropertyBox::getValue()
{
    return makeAny( static_cast<double>(mpMetric->GetValue()) );
}

Control* RotationPropertyBox::getControl()
{
    return mpControl;
}

class ScalePropertyBox : public PropertySubControl
{
public:
    ScalePropertyBox( sal_Int32 nControlType, vcl::Window* pParent, const Any& rValue, const Link<LinkParamNone*,void>& rModifyHdl );
    virtual ~ScalePropertyBox() override;

    virtual Any getValue() override;
    virtual void setValue( const Any& rValue, const OUString& ) override;

    virtual Control* getControl() override;

    DECL_LINK( implMenuSelectHdl, MenuButton*, void );
    DECL_LINK( implModifyHdl, Edit&, void );

    void updateMenu();

private:
    VclBuilder maBuilder;
    VclPtr<DropdownMenuBox>   mpControl;
    VclPtr<PopupMenu>         mpMenu;
    VclPtr<MetricField>       mpMetric;
    Link<LinkParamNone*,void> maModifyHdl;
    int                       mnDirection;
};

ScalePropertyBox::ScalePropertyBox(sal_Int32 nControlType, vcl::Window* pParent, const Any& rValue, const Link<LinkParamNone*,void>& rModifyHdl)
    : PropertySubControl( nControlType )
    , maBuilder(nullptr, VclBuilderContainer::getUIRootDir(), "modules/simpress/ui/scalemenu.ui", "")
    , maModifyHdl( rModifyHdl )
{
    mpMetric.set( VclPtr<MetricField>::Create( pParent ,WB_TABSTOP|WB_IGNORETAB| WB_NOBORDER) );
    mpMetric->SetUnit( FUNIT_PERCENT );
    mpMetric->SetMin( 0 );
    mpMetric->SetMax( 10000 );

    mpMenu = maBuilder.get_menu("menu");
    mpControl = VclPtr<DropdownMenuBox>::Create( pParent, mpMetric, mpMenu );
    mpControl->SetMenuSelectHdl( LINK( this, ScalePropertyBox, implMenuSelectHdl ));
    mpControl->SetHelpId( HID_SD_CUSTOMANIMATIONPANE_SCALEPROPERTYBOX );

    Link<Edit&,void> aLink( LINK( this, ScalePropertyBox, implModifyHdl ) );
    mpControl->SetModifyHdl( aLink );

    setValue( rValue, OUString() );
}

ScalePropertyBox::~ScalePropertyBox()
{
    maBuilder.disposeBuilder();
    mpControl.disposeAndClear();
}

void ScalePropertyBox::updateMenu()
{
    sal_Int64 nValue = mpMetric->GetValue();

    mpMenu->CheckItem("25", nValue == 25);
    mpMenu->CheckItem("50", nValue == 50);
    mpMenu->CheckItem("150", nValue == 150);
    mpMenu->CheckItem("400", nValue == 400);

    mpMenu->CheckItem("hori", mnDirection == 1);
    mpMenu->CheckItem("vert", mnDirection == 2);
    mpMenu->CheckItem("both", mnDirection == 3);
}

IMPL_LINK_NOARG(ScalePropertyBox, implModifyHdl, Edit&, void)
{
    updateMenu();
    maModifyHdl.Call(nullptr);
}

IMPL_LINK( ScalePropertyBox, implMenuSelectHdl, MenuButton*, pPb, void )
{
    sal_Int64 nValue = mpMetric->GetValue();

    int nDirection = mnDirection;

    OString sIdent(pPb->GetCurItemIdent());
    if (sIdent == "hori")
        nDirection = 1;
    else if (sIdent == "veri")
        nDirection = 2;
    else if (sIdent == "both")
        nDirection = 3;
    else
        nValue = sIdent.toInt32();

    bool bModified = false;

    if( nDirection != mnDirection )
    {
        mnDirection = nDirection;
        bModified = true;
    }

    if( nValue != mpMetric->GetValue() )
    {
        mpMetric->SetValue( nValue );
        bModified = true;
    }

    if( bModified )
    {
        mpMetric->Modify();
        updateMenu();
    }
}

void ScalePropertyBox::setValue( const Any& rValue, const OUString& )
{
    if( mpMetric.get() )
    {
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

        long nValue;
        if( fValue1 )
            nValue = static_cast<long>(fValue1 * 100.0);
        else
            nValue = static_cast<long>(fValue2 * 100.0);
        mpMetric->SetValue( nValue );
        updateMenu();
    }
}

Any ScalePropertyBox::getValue()
{
    double fValue1 = static_cast<double>(mpMetric->GetValue()) / 100.0;

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

Control* ScalePropertyBox::getControl()
{
    return mpControl;
}

class FontStylePropertyBox : public PropertySubControl
{
public:
    FontStylePropertyBox( sal_Int32 nControlType, vcl::Window* pParent, const Any& rValue, const Link<LinkParamNone*,void>& rModifyHdl );
    virtual ~FontStylePropertyBox() override;

    virtual Any getValue() override;
    virtual void setValue( const Any& rValue, const OUString& ) override;

    virtual Control* getControl() override;

    DECL_LINK( implMenuSelectHdl, MenuButton*, void );

    void update();

private:
    VclBuilder                maBuilder;
    VclPtr<DropdownMenuBox>   mpControl;
    VclPtr<PopupMenu>         mpMenu;
    VclPtr<Edit>              mpEdit;
    Link<LinkParamNone*,void> maModifyHdl;

    float mfFontWeight;
    awt::FontSlant meFontSlant;
    sal_Int16 mnFontUnderline;
};

FontStylePropertyBox::FontStylePropertyBox( sal_Int32 nControlType, vcl::Window* pParent, const Any& rValue, const Link<LinkParamNone*,void>& rModifyHdl )
    : PropertySubControl(nControlType)
    , maBuilder(nullptr, VclBuilderContainer::getUIRootDir(), "modules/simpress/ui/fontstylemenu.ui", "")
    , maModifyHdl( rModifyHdl )
{
    mpEdit.set( VclPtr<Edit>::Create( pParent, WB_TABSTOP|WB_IGNORETAB|WB_NOBORDER|WB_READONLY) );
    mpEdit->SetText( SdResId(STR_CUSTOMANIMATION_SAMPLE) );

    mpMenu = maBuilder.get_menu("menu");
    mpControl = VclPtr<DropdownMenuBox>::Create( pParent, mpEdit, mpMenu );
    mpControl->SetMenuSelectHdl( LINK( this, FontStylePropertyBox, implMenuSelectHdl ));
    mpControl->SetHelpId( HID_SD_CUSTOMANIMATIONPANE_FONTSTYLEPROPERTYBOX );

    setValue( rValue, OUString() );
}

FontStylePropertyBox::~FontStylePropertyBox()
{
    maBuilder.disposeBuilder();
    mpControl.disposeAndClear();
}

void FontStylePropertyBox::update()
{
    // update menu
    mpMenu->CheckItem("bold", mfFontWeight == awt::FontWeight::BOLD);
    mpMenu->CheckItem("italic", meFontSlant == awt::FontSlant_ITALIC);
    mpMenu->CheckItem("underline", mnFontUnderline != awt::FontUnderline::NONE );

    // update sample edit
    vcl::Font aFont( mpEdit->GetFont() );
    aFont.SetWeight( mfFontWeight == awt::FontWeight::BOLD ? WEIGHT_BOLD : WEIGHT_NORMAL );
    aFont.SetItalic( meFontSlant == awt::FontSlant_ITALIC ? ITALIC_NORMAL : ITALIC_NONE  );
    aFont.SetUnderline( mnFontUnderline == awt::FontUnderline::NONE ? LINESTYLE_NONE : LINESTYLE_SINGLE );
    mpEdit->SetFont( aFont );
    mpEdit->Invalidate();
}

IMPL_LINK( FontStylePropertyBox, implMenuSelectHdl, MenuButton*, pPb, void )
{
    OString sIdent = pPb->GetCurItemIdent();
    if (sIdent == "bold")
    {
        if( mfFontWeight == awt::FontWeight::BOLD )
            mfFontWeight = awt::FontWeight::NORMAL;
        else
            mfFontWeight = awt::FontWeight::BOLD;
    }
    else if (sIdent == "italic")
    {
        if( meFontSlant == awt::FontSlant_ITALIC )
            meFontSlant = awt::FontSlant_NONE;
        else
            meFontSlant = awt::FontSlant_ITALIC;
    }
    else if (sIdent == "underline")
    {
        if( mnFontUnderline == awt::FontUnderline::SINGLE )
            mnFontUnderline = awt::FontUnderline::NONE;
        else
            mnFontUnderline = awt::FontUnderline::SINGLE;
    }

    update();
    maModifyHdl.Call(nullptr);
}

void FontStylePropertyBox::setValue( const Any& rValue, const OUString& )
{
    Sequence<Any> aValues;
    rValue >>= aValues;

    aValues[0] >>= mfFontWeight;
    aValues[1] >>= meFontSlant;
    aValues[2] >>= mnFontUnderline;

    update();
}

Any FontStylePropertyBox::getValue()
{
    Sequence<Any> aValues(3);
    aValues[0] <<= mfFontWeight;
    aValues[1] <<= meFontSlant;
    aValues[2] <<= mnFontUnderline;
    return makeAny( aValues );
}

Control* FontStylePropertyBox::getControl()
{
    return mpControl;
}

class CustomAnimationEffectTabPage : public TabPage
{
public:
    CustomAnimationEffectTabPage( vcl::Window* pParent, const STLPropertySet* pSet );
    virtual ~CustomAnimationEffectTabPage() override;
    virtual void dispose() override;

    void update( STLPropertySet* pSet );
    DECL_LINK( implSelectHdl, ListBox&, void );
    DECL_LINK( implClickHdl, Button*, void );
    void implHdl(Control const *);

private:
    void updateControlStates();
    void fillSoundListBox();
    void clearSoundListBox();
    sal_Int32 getSoundObject( const OUString& rStr );
    void openSoundFileDialog();
    void onSoundPreview();

private:
    ::std::vector< OUString > maSoundList;
    bool mbHasText;
    const STLPropertySet* mpSet;

    VclPtr<VclFrame>       mpSettings;
    VclPtr<FixedText>      mpFTProperty1;
    VclPtr<PropertyControl> mpLBProperty1;
    VclPtr<VclHBox>        mpPlaceholderBox;
    VclPtr<CheckBox>       mpCBSmoothStart;
    VclPtr<CheckBox>       mpCBSmoothEnd;

    VclPtr<FixedText>      mpFTSound;
    VclPtr<ListBox>        mpLBSound;
    VclPtr<PushButton>     mpPBSoundPreview;
    VclPtr<FixedText>      mpFTAfterEffect;
    VclPtr<ListBox>        mpLBAfterEffect;
    VclPtr<FixedText>      mpFTDimColor;
    VclPtr<SvxColorListBox> mpCLBDimColor;
    VclPtr<FixedText>      mpFTTextAnim;
    VclPtr<ListBox>        mpLBTextAnim;
    VclPtr<MetricField>    mpMFTextDelay;
    VclPtr<FixedText>      mpFTTextDelay;

    css::uno::Reference< css::media::XPlayer > mxPlayer;
};

CustomAnimationEffectTabPage::CustomAnimationEffectTabPage( vcl::Window* pParent, const STLPropertySet* pSet )
: TabPage( pParent, "EffectTab", "modules/simpress/ui/customanimationeffecttab.ui" ), mbHasText( false ), mpSet(pSet )
{
    get(mpSettings, "settings" );
    get(mpFTProperty1, "prop_label1" );
    get(mpLBProperty1, "prop_list1" );
    get(mpPlaceholderBox, "placeholder" );
    get(mpCBSmoothStart, "smooth_start" );
    get(mpCBSmoothEnd, "smooth_end" );
    get(mpFTSound, "sound_label");
    get(mpLBSound, "sound_list" );
    get(mpPBSoundPreview, "sound_preview" );
    get(mpFTAfterEffect, "aeffect_label" );
    get(mpLBAfterEffect, "aeffect_list" );
    get(mpFTDimColor, "dim_color_label" );
    get(mpCLBDimColor, "dim_color_list" );
    mpCLBDimColor->SelectEntry(COL_BLACK);
    get(mpFTTextAnim, "text_animation_label" );
    get(mpLBTextAnim, "text_animation_list" );
    get(mpMFTextDelay,"text_delay" );
    get(mpFTTextDelay,"text_delay_label" );

    // fill the soundbox
    fillSoundListBox();

    mpLBSound->SetSelectHdl( LINK( this, CustomAnimationEffectTabPage, implSelectHdl ) );

    mpPBSoundPreview->SetClickHdl( LINK( this, CustomAnimationEffectTabPage, implClickHdl ) );

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
                    mpSettings->Show();
                    mpFTProperty1->SetText( aPropertyName );
                }

                // get property value
                const Any aValue( pSet->getPropertyValue( nHandleProperty1Value ) );

                // create property sub control
                mpLBProperty1->setSubControl( PropertySubControl::create( nType, mpPlaceholderBox, aValue, aPresetId, Link<LinkParamNone*,void>() ));
            }
        }

        mpFTProperty1->Enable( mpLBProperty1->IsEnabled() );

        // accelerate & decelerate

        if( pSet->getPropertyState( nHandleAccelerate ) == STLPropertyState::Direct )
        {
            mpCBSmoothStart->Show();
            mpCBSmoothEnd->Show();

            double fTemp = 0.0;
            pSet->getPropertyValue( nHandleAccelerate ) >>= fTemp;
            mpCBSmoothStart->Check( fTemp > 0.0 );

            pSet->getPropertyValue( nHandleDecelerate ) >>= fTemp;
            mpCBSmoothEnd->Check( fTemp > 0.0 );
        }
    }

    // init after effect controls

    mpLBAfterEffect->SetSelectHdl( LINK( this, CustomAnimationEffectTabPage, implSelectHdl ) );
    mpLBTextAnim->SetSelectHdl( LINK( this, CustomAnimationEffectTabPage, implSelectHdl ) );

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
                Color aColor = Color(nColor);
                mpCLBDimColor->SelectEntry(aColor);
            }
            else
            {
                nPos++;
                if( bAfterEffectOnNextClick )
                    nPos++;
            }
        }

        mpLBAfterEffect->SelectEntryPos( nPos );
    }

    if( pSet->getPropertyState( nHandleHasText ) != STLPropertyState::Ambiguous )
        pSet->getPropertyValue( nHandleHasText ) >>= mbHasText;

    if( mbHasText )
    {
        if( pSet->getPropertyState( nHandleIterateType ) != STLPropertyState::Ambiguous)
        {
            sal_Int32 nPos = LISTBOX_ENTRY_NOTFOUND;

            sal_Int32 nIterateType = 0;
            pSet->getPropertyValue( nHandleIterateType ) >>= nIterateType;
            switch( nIterateType )
            {
            case TextAnimationType::BY_PARAGRAPH:   nPos = 0; break;
            case TextAnimationType::BY_WORD:        nPos = 1; break;
            case TextAnimationType::BY_LETTER:      nPos = 2; break;
            }

            mpLBTextAnim->SelectEntryPos( nPos );
        }

        if( pSet->getPropertyState( nHandleIterateInterval ) != STLPropertyState::Default )
        {
            double fIterateInterval = 0.0;
            pSet->getPropertyValue( nHandleIterateInterval ) >>= fIterateInterval;
            mpMFTextDelay->SetValue( static_cast<long>(fIterateInterval*10) );
        }
    }
    else
    {
        mpFTTextAnim->Enable( false );
        mpLBTextAnim->Enable( false );
        mpMFTextDelay->Enable( false );
        mpFTTextDelay->Enable( false );

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
                    nPos = mpLBSound->InsertEntry( aURL.GetBase(), nPos );
                }
            }
        }

        if( nPos != LISTBOX_ENTRY_NOTFOUND )
            mpLBSound->SelectEntryPos( nPos );
    }

    updateControlStates();

}

CustomAnimationEffectTabPage::~CustomAnimationEffectTabPage()
{
    disposeOnce();
}

void CustomAnimationEffectTabPage::dispose()
{
    clearSoundListBox();
    mpSettings.clear();
    mpFTProperty1.clear();
    mpLBProperty1.clear();
    mpPlaceholderBox.clear();
    mpCBSmoothStart.clear();
    mpCBSmoothEnd.clear();
    mpFTSound.clear();
    mpLBSound.clear();
    mpPBSoundPreview.clear();
    mpFTAfterEffect.clear();
    mpLBAfterEffect.clear();
    mpFTDimColor.clear();
    mpCLBDimColor.clear();
    mpFTTextAnim.clear();
    mpLBTextAnim.clear();
    mpMFTextDelay.clear();
    mpFTTextDelay.clear();
    TabPage::dispose();
}

void CustomAnimationEffectTabPage::updateControlStates()
{
    sal_Int32 nPos = mpLBAfterEffect->GetSelectedEntryPos();
    mpCLBDimColor->Enable( nPos == 1 );
    mpFTDimColor->Enable( nPos == 1 );

    if( mbHasText )
    {
        nPos = mpLBTextAnim->GetSelectedEntryPos();
        mpMFTextDelay->Enable( nPos != 0 );
        mpFTTextDelay->Enable( nPos != 0 );
    }

    nPos = mpLBSound->GetSelectedEntryPos();
    mpPBSoundPreview->Enable( nPos >= 2 );
}

IMPL_LINK( CustomAnimationEffectTabPage, implClickHdl, Button*, pBtn, void )
{
    implHdl(pBtn);
}
IMPL_LINK( CustomAnimationEffectTabPage, implSelectHdl, ListBox&, rListBox, void )
{
    implHdl(&rListBox);
}

void CustomAnimationEffectTabPage::implHdl(Control const * pControl )
{
    if( pControl == mpLBTextAnim )
    {
        if( mpMFTextDelay->GetValue() == 0 )
            mpMFTextDelay->SetValue( 100 );
    }
    else if( pControl == mpLBSound )
    {
        sal_Int32 nPos = mpLBSound->GetSelectedEntryPos();
        if( nPos == (mpLBSound->GetEntryCount() - 1) )
        {
            openSoundFileDialog();
        }
    }
    else if( pControl == mpPBSoundPreview )
    {
        onSoundPreview();
    }

    updateControlStates();
}

void CustomAnimationEffectTabPage::update( STLPropertySet* pSet )
{
    if( mpLBProperty1->getSubControl() )
    {
        Any aNewValue( mpLBProperty1->getSubControl()->getValue() );
        Any aOldValue;
        if( mpSet->getPropertyState( nHandleProperty1Value ) != STLPropertyState::Ambiguous)
            aOldValue = mpSet->getPropertyValue( nHandleProperty1Value );

        if( aOldValue != aNewValue )
            pSet->setPropertyValue( nHandleProperty1Value, aNewValue );
    }

    if( mpCBSmoothStart->IsVisible() )
    {
        // set selected value for accelerate if different than in original set

        double fTemp = mpCBSmoothStart->IsChecked() ? 0.5 : 0.0;

        double fOldTemp = 0.0;
        if(mpSet->getPropertyState( nHandleAccelerate ) != STLPropertyState::Ambiguous)
            mpSet->getPropertyValue( nHandleAccelerate ) >>= fOldTemp;
        else
            fOldTemp = -2.0;

        if( fOldTemp != fTemp )
            pSet->setPropertyValue( nHandleAccelerate, makeAny( fTemp ) );

        // set selected value for decelerate if different than in original set
        fTemp = mpCBSmoothEnd->IsChecked() ? 0.5 : 0.0;

        if(mpSet->getPropertyState( nHandleDecelerate ) != STLPropertyState::Ambiguous)
            mpSet->getPropertyValue( nHandleDecelerate ) >>= fOldTemp;
        else
            fOldTemp = -2.0;

        if( fOldTemp != fTemp )
            pSet->setPropertyValue( nHandleDecelerate, makeAny( fTemp ) );
    }

    sal_Int32 nPos = mpLBAfterEffect->GetSelectedEntryPos();
    if( nPos != LISTBOX_ENTRY_NOTFOUND )
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
            Color aSelectedColor = mpCLBDimColor->GetSelectEntryColor();
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

    nPos = mpLBTextAnim->GetSelectedEntryPos();
    if( nPos != LISTBOX_ENTRY_NOTFOUND )
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
        double fIterateInterval = static_cast< double >( mpMFTextDelay->GetValue() ) / 10;
        double fOldIterateInterval = -1.0;

        if( mpSet->getPropertyState( nHandleIterateInterval ) != STLPropertyState::Ambiguous )
            mpSet->getPropertyValue( nHandleIterateInterval ) >>= fOldIterateInterval;

        if( fIterateInterval != fOldIterateInterval )
            pSet->setPropertyValue( nHandleIterateInterval, makeAny( fIterateInterval ) );
    }

    nPos = mpLBSound->GetSelectedEntryPos();
    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
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
}

void CustomAnimationEffectTabPage::fillSoundListBox()
{
    GalleryExplorer::FillObjList( GALLERY_THEME_SOUNDS, maSoundList );
    GalleryExplorer::FillObjList( GALLERY_THEME_USERSOUNDS, maSoundList );

    mpLBSound->InsertEntry( SdResId(STR_CUSTOMANIMATION_NO_SOUND) );
    mpLBSound->InsertEntry( SdResId(STR_CUSTOMANIMATION_STOP_PREVIOUS_SOUND) );
    for(const OUString & rString : maSoundList)
    {
        INetURLObject aURL( rString );
        mpLBSound->InsertEntry( aURL.GetBase() );
    }
    mpLBSound->InsertEntry( SdResId(STR_CUSTOMANIMATION_BROWSE_SOUND) );
}

void CustomAnimationEffectTabPage::clearSoundListBox()
{
    maSoundList.clear();
    mpLBSound->Clear();
}

sal_Int32 CustomAnimationEffectTabPage::getSoundObject( const OUString& rStr )
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
    long nPos = 0;

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
                xWarn->add_button(Button::GetStandardText(StandardButtonType::Retry), RET_RETRY);
                xWarn->add_button(Button::GetStandardText(StandardButtonType::Cancel), RET_CANCEL);
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

    mpLBSound->SelectEntryPos( nPos );
}

void CustomAnimationEffectTabPage::onSoundPreview()
{
    const sal_Int32 nPos = mpLBSound->GetSelectedEntryPos();

    if( nPos >= 2 ) try
    {
        const OUString aSoundURL( maSoundList[ nPos-2 ] );
        mxPlayer.set( avmedia::MediaWindow::createPlayer( aSoundURL, "" ), uno::UNO_QUERY_THROW );
        mxPlayer->start();
    }
    catch( uno::Exception& )
    {
        OSL_FAIL("CustomAnimationEffectTabPage::onSoundPreview(), exception caught!" );
    }
}

class CustomAnimationDurationTabPage : public TabPage
{
public:
    CustomAnimationDurationTabPage( vcl::Window* pParent, const STLPropertySet* pSet );
    virtual ~CustomAnimationDurationTabPage() override;
    virtual void dispose() override;

    void update( STLPropertySet* pSet );

    DECL_LINK( implControlHdl, ListBox&, void );
    DECL_LINK( implClickHdl, Button*, void );
    DECL_LINK( DurationModifiedHdl, Edit&, void );
    void implHdl(Control const *);

private:
    const STLPropertySet* mpSet;

    VclPtr<FixedText> mpFTStart;
    VclPtr<ListBox> mpLBStart;
    VclPtr<FixedText> mpFTStartDelay;
    VclPtr<MetricField> mpMFStartDelay;
    VclPtr<FixedText> mpFTDuration;
    VclPtr<MetricBox> mpCBXDuration;
    VclPtr<FixedText> mpFTRepeat;
    VclPtr<ListBox> mpCBRepeat;
    VclPtr<CheckBox> mpCBXRewind;
    VclPtr<RadioButton> mpRBClickSequence;
    VclPtr<RadioButton> mpRBInteractive;
    VclPtr<ListBox> mpLBTrigger;
};

CustomAnimationDurationTabPage::CustomAnimationDurationTabPage(vcl::Window* pParent, const STLPropertySet* pSet)
: TabPage( pParent, "TimingTab", "modules/simpress/ui/customanimationtimingtab.ui" ), mpSet( pSet )
{
    get(mpFTStart,"start_label" );
    get(mpLBStart, "start_list" );
    get(mpFTStartDelay, "delay_label" );
    get(mpMFStartDelay, "delay_value" );
    get(mpFTDuration, "duration_label" );
    get(mpCBXDuration, "anim_duration" );
    get(mpFTRepeat, "repeat_label" );
    get(mpCBRepeat, "repeat_list" );
    get(mpCBXRewind, "rewind" );
    get(mpRBClickSequence, "rb_click_sequence" );
    get(mpRBInteractive, "rb_interactive" );
    get(mpLBTrigger, "trigger_list");
    mpLBTrigger->set_width_request(approximate_char_width() * 40);

    fillRepeatComboBox( mpCBRepeat );

    //fillDurationMetricComboBox
    mpCBXDuration->InsertValue(50, FUNIT_CUSTOM);
    mpCBXDuration->InsertValue(100, FUNIT_CUSTOM);
    mpCBXDuration->InsertValue(200, FUNIT_CUSTOM);
    mpCBXDuration->InsertValue(300, FUNIT_CUSTOM);
    mpCBXDuration->InsertValue(500, FUNIT_CUSTOM);
    mpCBXDuration->AdaptDropDownLineCountToMaximum();

    mpRBClickSequence->SetClickHdl( LINK( this, CustomAnimationDurationTabPage, implClickHdl ) );
    mpLBTrigger->SetSelectHdl( LINK( this, CustomAnimationDurationTabPage, implControlHdl ) );
    mpCBXDuration->SetModifyHdl(LINK( this, CustomAnimationDurationTabPage, DurationModifiedHdl));

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
        mpLBStart->SelectEntryPos( nPos );
    }

    if( pSet->getPropertyState( nHandleBegin ) != STLPropertyState::Ambiguous )
    {
        double fBegin = 0.0;
        pSet->getPropertyValue( nHandleBegin ) >>= fBegin;
        mpMFStartDelay->SetValue( static_cast<long>(fBegin*10) );
    }

    if( pSet->getPropertyState( nHandleDuration ) != STLPropertyState::Ambiguous )
    {
        double fDuration = 0.0;
        pSet->getPropertyValue( nHandleDuration ) >>= fDuration;

        if( fDuration == 0.001 )
        {
            mpFTDuration->Disable();
            mpCBXDuration->Disable();
            mpFTRepeat->Disable();
            mpCBRepeat->Disable();
            mpCBXRewind->Disable();
        }
        else
        {
            mpCBXDuration->SetValue( fDuration*100.0 );
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

            sal_Int32 nPos = LISTBOX_ENTRY_NOTFOUND;

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

            if( nPos != LISTBOX_ENTRY_NOTFOUND )
                mpCBRepeat->SelectEntryPos( nPos );
            else
                mpCBRepeat->SetText(OUString::number(fRepeat));
        }
        else if( aRepeatCount.getValueType() == ::cppu::UnoType<Timing>::get() )
        {
            Any aEnd;
            if( pSet->getPropertyState( nHandleEnd ) != STLPropertyState::Ambiguous )
                aEnd = pSet->getPropertyValue( nHandleEnd );

            mpCBRepeat->SelectEntryPos( aEnd.hasValue() ? 6 : 7 );
        }
    }

    if( pSet->getPropertyState( nHandleRewind ) != STLPropertyState::Ambiguous )
    {
        sal_Int16 nFill = 0;
        if( pSet->getPropertyValue( nHandleRewind ) >>= nFill )
        {
            mpCBXRewind->Check( nFill == AnimationFill::REMOVE );
        }
        else
        {
            mpCBXRewind->SetState( TRISTATE_INDET );
        }
    }

    Reference< XShape > xTrigger;

    if( pSet->getPropertyState( nHandleTrigger ) != STLPropertyState::Ambiguous )
    {
        pSet->getPropertyValue( nHandleTrigger ) >>= xTrigger;

        mpRBInteractive->Check( xTrigger.is() );
        mpRBClickSequence->Check( !xTrigger.is() );
    }

    Reference< XDrawPage > xCurrentPage;
    pSet->getPropertyValue( nHandleCurrentPage ) >>= xCurrentPage;
    if( xCurrentPage.is() )
    {
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
            sal_Int32 nPos = mpLBTrigger->InsertEntry( aDescription );

            mpLBTrigger->SetEntryData( nPos, reinterpret_cast<void*>(static_cast<sal_IntPtr>(nShape)) );
            if( xShape == xTrigger )
                mpLBTrigger->SelectEntryPos( nPos );
        }
    }
}

CustomAnimationDurationTabPage::~CustomAnimationDurationTabPage()
{
    disposeOnce();
}

void CustomAnimationDurationTabPage::dispose()
{
    mpFTStart.clear();
    mpLBStart.clear();
    mpFTStartDelay.clear();
    mpMFStartDelay.clear();
    mpFTDuration.clear();
    mpCBXDuration.clear();
    mpFTRepeat.clear();
    mpCBRepeat.clear();
    mpCBXRewind.clear();
    mpRBClickSequence.clear();
    mpRBInteractive.clear();
    mpLBTrigger.clear();
    TabPage::dispose();
}

IMPL_LINK( CustomAnimationDurationTabPage, implClickHdl, Button*, pBtn, void )
{
    implHdl(pBtn);
}

IMPL_LINK( CustomAnimationDurationTabPage, implControlHdl, ListBox&, rListBox, void )
{
    implHdl(&rListBox);
}

IMPL_LINK_NOARG(CustomAnimationDurationTabPage, DurationModifiedHdl, Edit&, void)
{
    if(!(mpCBXDuration->GetText()).isEmpty() )
    {
        double duration_value = static_cast<double>(mpCBXDuration->GetValue());
        if(duration_value <= 0.0)
            mpCBXDuration->SetValue(1);
        else
            mpCBXDuration->SetValue(duration_value);
    }
}

void CustomAnimationDurationTabPage::implHdl( Control const * pControl )
{
    if( pControl == mpLBTrigger )
    {
        mpRBClickSequence->Check( false );
        mpRBInteractive->Check();
    }
}

void CustomAnimationDurationTabPage::update( STLPropertySet* pSet )
{
    sal_Int32 nPos = mpLBStart->GetSelectedEntryPos();
    if( nPos != LISTBOX_ENTRY_NOTFOUND )
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
        double fBegin = static_cast<double>( mpMFStartDelay->GetValue()) / 10.0;
        double fOldBegin = -1.0;

        if( mpSet->getPropertyState( nHandleBegin ) != STLPropertyState::Ambiguous )
            mpSet->getPropertyValue( nHandleBegin ) >>= fOldBegin;

        if( fBegin != fOldBegin )
            pSet->setPropertyValue( nHandleBegin, makeAny( fBegin ) );
    }

    nPos = mpCBRepeat->GetSelectedEntryPos();
    if( (nPos != LISTBOX_ENTRY_NOTFOUND) || (!mpCBRepeat->GetText().isEmpty()) )
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
            SAL_FALLTHROUGH;
        case 7:
            aRepeatCount <<= Timing_INDEFINITE;
            break;
        default:
            {
                OUString aText(mpCBRepeat->GetEntry(nPos));
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

    if(!(mpCBXDuration->GetText()).isEmpty() )
    {
        double duration_value = static_cast<double>(mpCBXDuration->GetValue());

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

    if( mpCBXRewind->GetState() != TRISTATE_INDET )
    {
        sal_Int16 nFill = mpCBXRewind->IsChecked() ? AnimationFill::REMOVE : AnimationFill::HOLD;

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

    if( mpRBInteractive->IsChecked() )
    {
        nPos = mpLBTrigger->GetSelectedEntryPos();
        if( nPos != LISTBOX_ENTRY_NOTFOUND )
        {
            sal_Int32 nShape = static_cast<sal_Int32>(reinterpret_cast<sal_IntPtr>(mpLBTrigger->GetEntryData( nPos )));

            Reference< XDrawPage > xCurrentPage;
            mpSet->getPropertyValue( nHandleCurrentPage ) >>= xCurrentPage;

            if( xCurrentPage.is() && (nShape >= 0) && (nShape < xCurrentPage->getCount()) )
                xCurrentPage->getByIndex( nShape ) >>= xTrigger;
        }
    }

    if( xTrigger.is() || mpRBClickSequence->IsChecked() )
    {
        Any aNewValue( makeAny( xTrigger ) );
        Any aOldValue;

        if( mpSet->getPropertyState( nHandleTrigger ) != STLPropertyState::Ambiguous )
            aOldValue = mpSet->getPropertyValue( nHandleTrigger );

        if( aNewValue != aOldValue )
            pSet->setPropertyValue( nHandleTrigger, aNewValue );
    }
}

class CustomAnimationTextAnimTabPage : public TabPage
{
public:
    CustomAnimationTextAnimTabPage( vcl::Window* pParent, const STLPropertySet* pSet );
    virtual ~CustomAnimationTextAnimTabPage() override;
    virtual void dispose() override;

    void update( STLPropertySet* pSet );

    void updateControlStates();
    DECL_LINK(implSelectHdl, ListBox&, void);

private:
    VclPtr<FixedText>   maFTGroupText;
    VclPtr<ListBox>     maLBGroupText;
    VclPtr<CheckBox>    maCBXGroupAuto;
    VclPtr<MetricField> maMFGroupAuto;
    VclPtr<CheckBox>    maCBXAnimateForm;
    VclPtr<CheckBox>    maCBXReverse;

    const STLPropertySet* mpSet;

    bool mbHasVisibleShapes;
};

CustomAnimationTextAnimTabPage::CustomAnimationTextAnimTabPage(vcl::Window* pParent, const STLPropertySet* pSet)
:   TabPage( pParent, "TextAnimationTab", "modules/simpress/ui/customanimationtexttab.ui" ),
    mpSet( pSet ),
    mbHasVisibleShapes(true)
{
    get( maFTGroupText, "group_text_label" );
    get( maLBGroupText, "group_text_list" );
    get( maCBXGroupAuto, "auto_after" );
    get( maMFGroupAuto, "auto_after_value" );
    get( maCBXAnimateForm, "animate_shape" );
    get( maCBXReverse, "reverse_order" );

    maLBGroupText->SetSelectHdl( LINK( this, CustomAnimationTextAnimTabPage, implSelectHdl ) );

    if( pSet->getPropertyState( nHandleTextGrouping ) != STLPropertyState::Ambiguous )
    {
        sal_Int32 nTextGrouping = 0;
        if( pSet->getPropertyValue( nHandleTextGrouping ) >>= nTextGrouping )
            maLBGroupText->SelectEntryPos( nTextGrouping + 1 );
    }

    if( pSet->getPropertyState( nHandleHasVisibleShape ) != STLPropertyState::Ambiguous )
        pSet->getPropertyValue( nHandleHasVisibleShape ) >>= mbHasVisibleShapes;

    if( pSet->getPropertyState( nHandleTextGroupingAuto ) != STLPropertyState::Ambiguous )
    {
        double fTextGroupingAuto = 0.0;
        if( pSet->getPropertyValue( nHandleTextGroupingAuto ) >>= fTextGroupingAuto )
        {
            maCBXGroupAuto->Check( fTextGroupingAuto >= 0.0 );
            if( fTextGroupingAuto >= 0.0 )
                maMFGroupAuto->SetValue( static_cast<long>(fTextGroupingAuto*10) );
        }
    }
    else
    {
        maCBXGroupAuto->SetState( TRISTATE_INDET );
    }

    maCBXAnimateForm->SetState( TRISTATE_INDET );
    if( pSet->getPropertyState( nHandleAnimateForm ) != STLPropertyState::Ambiguous )
    {
        bool bAnimateForm = false;
        if( pSet->getPropertyValue( nHandleAnimateForm ) >>= bAnimateForm )
        {
            maCBXAnimateForm->Check( bAnimateForm );
        }
    }
    else
    {
        maCBXAnimateForm->Enable( false );
    }

    maCBXReverse->SetState( TRISTATE_INDET );
    if( pSet->getPropertyState( nHandleTextReverse ) != STLPropertyState::Ambiguous )
    {
        bool bTextReverse = false;
        if( pSet->getPropertyValue( nHandleTextReverse ) >>= bTextReverse )
        {
            maCBXReverse->Check( bTextReverse );
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
            maLBGroupText->RemoveEntry( nPos );
            nPos--;
        }
    }

    updateControlStates();
}

CustomAnimationTextAnimTabPage::~CustomAnimationTextAnimTabPage()
{
    disposeOnce();
}

void CustomAnimationTextAnimTabPage::dispose()
{
    maFTGroupText.clear();
    maLBGroupText.clear();
    maCBXGroupAuto.clear();
    maMFGroupAuto.clear();
    maCBXAnimateForm.clear();
    maCBXReverse.clear();
    TabPage::dispose();
}

void CustomAnimationTextAnimTabPage::update( STLPropertySet* pSet )
{
    sal_Int32 nPos = maLBGroupText->GetSelectedEntryPos();
    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        sal_Int32 nTextGrouping = nPos - 1;
        sal_Int32 nOldGrouping = -2;

        if(mpSet->getPropertyState( nHandleTextGrouping ) != STLPropertyState::Ambiguous)
            mpSet->getPropertyValue( nHandleTextGrouping ) >>= nOldGrouping;

        if( nTextGrouping != nOldGrouping )
            pSet->setPropertyValue( nHandleTextGrouping, makeAny( nTextGrouping ) );
    }

    if( nPos > 0 )
    {
        bool bTextReverse = maCBXReverse->IsChecked();
        bool bOldTextReverse = !bTextReverse;

        if(mpSet->getPropertyState( nHandleTextReverse ) != STLPropertyState::Ambiguous)
            mpSet->getPropertyValue( nHandleTextReverse ) >>= bOldTextReverse;

        if( bTextReverse != bOldTextReverse )
            pSet->setPropertyValue( nHandleTextReverse, makeAny( bTextReverse ) );

        if( nPos > 1 )
        {
            double fTextGroupingAuto = maCBXGroupAuto->IsChecked() ? maMFGroupAuto->GetValue() / 10.0 : -1.0;
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
    if (maCBXAnimateForm->IsEnabled())
    {
        bool bAnimateForm = maCBXAnimateForm->IsChecked();
        bool bOldAnimateForm = !bAnimateForm;

        if(mpSet->getPropertyState( nHandleAnimateForm ) != STLPropertyState::Ambiguous)
            mpSet->getPropertyValue( nHandleAnimateForm ) >>= bOldAnimateForm;

        if( bAnimateForm != bOldAnimateForm )
            pSet->setPropertyValue( nHandleAnimateForm, makeAny( bAnimateForm ) );
    }
}

void CustomAnimationTextAnimTabPage::updateControlStates()
{
    sal_Int32 nPos = maLBGroupText->GetSelectedEntryPos();

    maCBXGroupAuto->Enable( nPos > 1 );
    maMFGroupAuto->Enable( nPos > 1 );
    maCBXReverse->Enable( nPos > 0 );

    if( !mbHasVisibleShapes && nPos > 0 )
    {
        maCBXAnimateForm->Check(false);
        maCBXAnimateForm->Enable(false);
    }
    else
    {
        maCBXAnimateForm->Enable();
    }
}

IMPL_LINK_NOARG(CustomAnimationTextAnimTabPage, implSelectHdl, ListBox&, void)
{
    updateControlStates();
}

CustomAnimationDialog::CustomAnimationDialog(vcl::Window* pParent, STLPropertySet* pSet, const OString& sPage)
: TabDialog( pParent, "CustomAnimationProperties", "modules/simpress/ui/customanimationproperties.ui")
, mpSet( pSet )
, mpResultSet( nullptr )
{
    get(mpTabControl, "tabs");

    sal_uInt16 nEffectId = mpTabControl->GetPageId("effect");
    sal_uInt16 nTimingId = mpTabControl->GetPageId("timing");
    sal_uInt16 nTextAnimId = mpTabControl->GetPageId("textanim");

    mpEffectTabPage = VclPtr<CustomAnimationEffectTabPage>::Create( mpTabControl, mpSet.get() );
    mpTabControl->SetTabPage( nEffectId, mpEffectTabPage );
    mpDurationTabPage = VclPtr<CustomAnimationDurationTabPage>::Create( mpTabControl, mpSet.get() );
    mpTabControl->SetTabPage( nTimingId, mpDurationTabPage );

    bool bHasText = false;
    if( pSet->getPropertyState( nHandleHasText ) != STLPropertyState::Ambiguous )
        pSet->getPropertyValue( nHandleHasText ) >>= bHasText;

    if( bHasText )
    {
        mpTextAnimTabPage = VclPtr<CustomAnimationTextAnimTabPage>::Create( mpTabControl, mpSet.get() );
        mpTabControl->SetTabPage( nTextAnimId, mpTextAnimTabPage );
    }
    else
    {
        mpTextAnimTabPage = nullptr;
        mpTabControl->RemovePage( nTextAnimId );
    }

    if (!sPage.isEmpty())
        mpTabControl->SelectTabPage(mpTabControl->GetPageId(sPage));
}

CustomAnimationDialog::~CustomAnimationDialog()
{
    disposeOnce();
}

void CustomAnimationDialog::dispose()
{
    mpEffectTabPage.disposeAndClear();
    mpDurationTabPage.disposeAndClear();
    mpTextAnimTabPage.disposeAndClear();

    mpSet.reset();
    mpResultSet.reset();

    mpTabControl.clear();
    TabDialog::dispose();
}

STLPropertySet* CustomAnimationDialog::getResultSet()
{
    mpResultSet = createDefaultSet();

    mpEffectTabPage->update( mpResultSet.get() );
    mpDurationTabPage->update( mpResultSet.get() );
    if( mpTextAnimTabPage )
        mpTextAnimTabPage->update( mpResultSet.get() );

    return mpResultSet.get();
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
    pSet->setPropertyDefaultValue( nHandleSoundVolumne, makeAny( 1.0) );
    pSet->setPropertyDefaultValue( nHandleSoundEndAfterSlide, makeAny( sal_Int32(0) ) );

    pSet->setPropertyDefaultValue( nHandleCommand, makeAny( sal_Int16(0) ) );
    return pSet;
}

PropertyControl::PropertyControl( vcl::Window* pParent )
: ListBox( pParent, WB_TABSTOP | WB_BORDER | WB_DROPDOWN ), mpSubControl(nullptr)
{
}

VCL_BUILDER_FACTORY(PropertyControl)

PropertyControl::~PropertyControl()
{
    disposeOnce();
}

void PropertyControl::dispose()
{
    mpSubControl.reset();
    ListBox::dispose();
}

void PropertyControl::setSubControl( std::unique_ptr<PropertySubControl> pSubControl )
{
    mpSubControl = std::move(pSubControl);

    Control* pControl = mpSubControl ? mpSubControl->getControl() : nullptr;

    if( pControl )
    {
        pControl->SetPosSizePixel( GetPosPixel(), GetSizePixel() );
        pControl->SetZOrder( this, ZOrderFlags::Before );
        pControl->Show();
        Hide();
    }
    else
    {
        Show();
    }
}

void PropertyControl::Resize()
{
    Control* pControl = mpSubControl ? mpSubControl->getControl() : nullptr;
    if( pControl )
        pControl->SetPosSizePixel( GetPosPixel(), GetSizePixel() );
    ListBox::Resize();
}

PropertySubControl::~PropertySubControl()
{
}

std::unique_ptr<PropertySubControl> PropertySubControl::create( sal_Int32 nType, vcl::Window* pParent, const Any& rValue, const OUString& rPresetId, const Link<LinkParamNone*,void>& rModifyHdl )
{
    std::unique_ptr<PropertySubControl> pSubControl;
    switch( nType )
    {
    case nPropertyTypeDirection:
    case nPropertyTypeSpokes:
    case nPropertyTypeZoom:
        pSubControl.reset( new PresetPropertyBox( nType, pParent, rValue, rPresetId, rModifyHdl ) );
        break;

    case nPropertyTypeColor:
    case nPropertyTypeFillColor:
    case nPropertyTypeFirstColor:
    case nPropertyTypeCharColor:
    case nPropertyTypeLineColor:
        pSubControl.reset( new ColorPropertyBox( nType, pParent, rValue, rModifyHdl ) );
        break;

    case nPropertyTypeFont:
        pSubControl.reset( new FontPropertyBox( nType, pParent, rValue, rModifyHdl ) );
        break;

    case nPropertyTypeCharHeight:
        pSubControl.reset( new CharHeightPropertyBox( nType, pParent, rValue, rModifyHdl ) );
        break;

    case nPropertyTypeRotate:
        pSubControl.reset( new RotationPropertyBox( nType, pParent, rValue, rModifyHdl ) );
        break;

    case nPropertyTypeTransparency:
        pSubControl.reset( new TransparencyPropertyBox( nType, pParent, rValue, rModifyHdl ) );
        break;

    case nPropertyTypeScale:
        pSubControl.reset( new ScalePropertyBox( nType, pParent, rValue, rModifyHdl ) );
        break;

    case nPropertyTypeCharDecoration:
        pSubControl.reset( new FontStylePropertyBox( nType, pParent, rValue, rModifyHdl ) );
        break;
    }

    return pSubControl;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
