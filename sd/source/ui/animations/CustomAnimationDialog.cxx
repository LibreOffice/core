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

#include <boost/shared_ptr.hpp>

#include <comphelper/processfactory.hxx>
#include <unotools/pathoptions.hxx>
#include <vcl/tabctrl.hxx>
#include <vcl/tabpage.hxx>
#include <vcl/menubtn.hxx>
#include <vcl/svapp.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/layout.hxx>
#include <vcl/field.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/decoview.hxx>
#include <vcl/combobox.hxx>
#include <vcl/menu.hxx>
#include <svtools/ctrlbox.hxx>
#include <svtools/ctrltool.hxx>
#include <sfx2/objsh.hxx>

#include <svx/svxids.hrc>
#include <svx/dialmgr.hxx>
#include <editeng/flstitem.hxx>
#include <svx/drawitem.hxx>

#include <svx/xtable.hxx>
#include <svx/gallery.hxx>

#include <svx/dialogs.hrc>
#include "sdresid.hxx"

#include "glob.hrc"
#include "CustomAnimationDialog.hxx"
#include "CustomAnimationDialog.hrc"
#include "CustomAnimation.hrc"
#include "STLPropertySet.hxx"

#include <avmedia/mediawindow.hxx>

#include "filedlg.hxx"
#include "strings.hrc"
#include "helpids.h"

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

extern void fillRepeatComboBox( ComboBox* pBox );
extern void fillDurationComboBox( ComboBox* pBox );
extern OUString getShapeDescription( const Reference< XShape >& xShape, bool bWithText = true );
extern OUString getPropertyName( sal_Int32 nPropertyType );

// ====================================================================

class PresetPropertyBox  : public PropertySubControl
{
public:
    PresetPropertyBox( sal_Int32 nControlType, Window* pParent, const Any& rValue, const OUString& aPresetId, const Link& rModifyHdl );
    ~PresetPropertyBox();

    virtual Any getValue();
    virtual void setValue( const Any& rValue, const OUString& rPresetId );
    virtual Control* getControl();

private:
    std::map< sal_uInt16, OUString > maPropertyValues;
    ListBox* mpControl;
};

// --------------------------------------------------------------------

PresetPropertyBox::PresetPropertyBox( sal_Int32 nControlType, Window* pParent, const Any& rValue, const OUString& aPresetId, const Link& rModifyHdl )
: PropertySubControl( nControlType )
{
    mpControl = new ListBox( pParent, WB_BORDER|WB_TABSTOP|WB_DROPDOWN );
    mpControl->SetDropDownLineCount( 10 );
    mpControl->SetSelectHdl( rModifyHdl );
    mpControl->SetHelpId( HID_SD_CUSTOMANIMATIONPANE_PRESETPROPERTYBOX );

    setValue( rValue, aPresetId );

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

            UStringList aSubTypes( pDescriptor->getSubTypes() );
            UStringList::iterator aIter( aSubTypes.begin() );
            const UStringList::iterator aEnd( aSubTypes.end() );

            mpControl->Enable( aIter != aEnd );

            while( aIter != aEnd )
            {
                sal_uInt16 nPos = mpControl->InsertEntry( rPresets.getUINameForProperty( (*aIter) ) );
                if( (*aIter) == aPropertyValue )
                    mpControl->SelectEntryPos( nPos );
                maPropertyValues[nPos] = (*aIter++);
            }
        }
        else
        {
            mpControl->Enable( sal_False );
        }
    }
}

// --------------------------------------------------------------------

PresetPropertyBox::~PresetPropertyBox()
{
    delete mpControl;
}

// --------------------------------------------------------------------

Any PresetPropertyBox::getValue()
{
    return makeAny( maPropertyValues[mpControl->GetSelectEntryPos()] );
}

// --------------------------------------------------------------------

Control* PresetPropertyBox::getControl()
{
    return mpControl;
}

// ====================================================================

class ColorPropertyBox  : public PropertySubControl
{
public:
    ColorPropertyBox( sal_Int32 nControlType, Window* pParent, const Any& rValue, const Link& rModifyHdl );
    ~ColorPropertyBox();

    virtual Any getValue();
    virtual void setValue( const Any& rValue, const OUString& rPresetId  );
    virtual Control* getControl();

private:
    ColorListBox* mpControl;
};

// --------------------------------------------------------------------

ColorPropertyBox::ColorPropertyBox( sal_Int32 nControlType, Window* pParent, const Any& rValue, const Link& rModifyHdl )
: PropertySubControl( nControlType )
{
    mpControl = new ColorListBox( pParent, WB_BORDER|WB_TABSTOP|WB_DROPDOWN );
    mpControl->SetDropDownLineCount( 10 );
    mpControl->SetSelectHdl( rModifyHdl );
    mpControl->SetHelpId( HID_SD_CUSTOMANIMATIONPANE_COLORPROPERTYBOX );

    SfxObjectShell* pDocSh = SfxObjectShell::Current();
    DBG_ASSERT( pDocSh, "DocShell not found!" );
    XColorListRef pColorList;
    const SfxPoolItem* pItem = NULL;

    if ( pDocSh && ( ( pItem = pDocSh->GetItem( SID_COLOR_TABLE ) ) != 0) )
        pColorList = ( (SvxColorListItem*)pItem )->GetColorList();

    if ( !pColorList.is() )
        pColorList = XColorList::CreateStdColorList();

    sal_Int32 nColor = 0;
    rValue >>= nColor;

    for ( long i = 0; i < pColorList->Count(); i++ )
    {
        XColorEntry* pEntry = pColorList->GetColor(i);
        sal_uInt16 nPos = mpControl->InsertEntry( pEntry->GetColor(), pEntry->GetName() );
        if( pEntry->GetColor().GetRGBColor() == (sal_uInt32)nColor )
            mpControl->SelectEntryPos( nPos );
    }
}

// --------------------------------------------------------------------

ColorPropertyBox::~ColorPropertyBox()
{
    delete mpControl;
}

// --------------------------------------------------------------------

void ColorPropertyBox::setValue( const Any& rValue, const OUString& )
{
    if( mpControl )
    {
        sal_Int32 nColor = 0;
        rValue >>= nColor;

        mpControl->SetNoSelection();
        mpControl->SelectEntryPos( mpControl->GetEntryPos( (Color)nColor ) );
    }
}

// --------------------------------------------------------------------

Any ColorPropertyBox::getValue()
{
    return makeAny( (sal_Int32)mpControl->GetSelectEntryColor().GetRGBColor() );
}

// --------------------------------------------------------------------

Control* ColorPropertyBox::getControl()
{
    return mpControl;
}

// ====================================================================

class FontPropertyBox : public PropertySubControl
{
public:
    FontPropertyBox( sal_Int32 nControlType, Window* pParent, const Any& rValue, const Link& rModifyHdl );
    virtual ~FontPropertyBox();

    virtual Any getValue();
    virtual void setValue( const Any& rValue, const OUString& rPresetId  );

    virtual Control* getControl();

private:
    FontNameBox* mpControl;
};

// --------------------------------------------------------------------

FontPropertyBox::FontPropertyBox( sal_Int32 nControlType, Window* pParent, const Any& rValue, const Link& rModifyHdl )
: PropertySubControl( nControlType )
{
    mpControl = new FontNameBox( pParent, WB_BORDER|WB_TABSTOP|WB_DROPDOWN );
    mpControl->SetDropDownLineCount( 10 );
    mpControl->SetSelectHdl( rModifyHdl );
    mpControl->SetHelpId( HID_SD_CUSTOMANIMATIONPANE_FONTPROPERTYBOX );

    SfxObjectShell* pDocSh = SfxObjectShell::Current();
    const SfxPoolItem* pItem;

    const FontList* pFontList = 0;
    bool bMustDelete = false;

    if ( pDocSh && ( (pItem = pDocSh->GetItem( SID_ATTR_CHAR_FONTLIST ) ) != 0) )
        pFontList = ( (SvxFontListItem*)pItem )->GetFontList();

    if(!pFontList)
    {
        pFontList = new FontList( Application::GetDefaultDevice(), NULL, sal_False );
        bMustDelete = true;
    }

    mpControl->Fill( pFontList );

    if( bMustDelete )
        delete pFontList;

    OUString aPresetId;
    setValue( rValue, aPresetId );
}

// --------------------------------------------------------------------

void FontPropertyBox::setValue( const Any& rValue, const OUString& )
{
    if( mpControl )
    {
        OUString aFontName;
        rValue >>= aFontName;
        mpControl->SetText( aFontName );
    }
}

// --------------------------------------------------------------------

FontPropertyBox::~FontPropertyBox()
{
    delete mpControl;
}

// --------------------------------------------------------------------

Any FontPropertyBox::getValue()
{
    OUString aFontName( mpControl->GetText() );
    return makeAny( aFontName );
}

// --------------------------------------------------------------------

Control* FontPropertyBox::getControl()
{
    return mpControl;
}

// ====================================================================

class DropdownMenuBox : public Edit
{
public:
    DropdownMenuBox( Window* pParent, Edit* pSubControl, PopupMenu* pMenu );
    ~DropdownMenuBox();

    void Resize();
    long PreNotify( NotifyEvent& rNEvt );

    void SetMenuSelectHdl( const Link& rLink ) { mpDropdownButton->SetSelectHdl( rLink ); }

private:
    Edit* mpSubControl;
    MenuButton* mpDropdownButton;
    PopupMenu* mpMenu;
};

// --------------------------------------------------------------------

DropdownMenuBox::DropdownMenuBox( Window* pParent, Edit* pSubControl, PopupMenu* pMenu )
:   Edit( pParent, WB_BORDER|WB_TABSTOP| WB_DIALOGCONTROL ),
    mpSubControl(pSubControl),mpDropdownButton(0),mpMenu(pMenu)
{
    mpDropdownButton = new MenuButton( this, WB_NOLIGHTBORDER | WB_RECTSTYLE | WB_NOTABSTOP);
    mpDropdownButton->SetSymbol(SYMBOL_SPIN_DOWN);
    mpDropdownButton->Show();
    mpDropdownButton->SetPopupMenu( pMenu );

    SetSubEdit( mpSubControl );
    mpSubControl->SetParent( this );
    mpSubControl->Show();
}

// --------------------------------------------------------------------

DropdownMenuBox::~DropdownMenuBox()
{
    SetSubEdit( 0 );
    delete mpSubControl;
    delete mpDropdownButton;
    delete mpMenu;
}

// --------------------------------------------------------------------

void DropdownMenuBox::Resize()
{
    Size aOutSz = GetOutputSizePixel();

    long nSBWidth = GetSettings().GetStyleSettings().GetScrollBarSize();
    nSBWidth = CalcZoom( nSBWidth );
    mpSubControl->setPosSizePixel( 0, 1, aOutSz.Width() - nSBWidth, aOutSz.Height()-2 );
    mpDropdownButton->setPosSizePixel( aOutSz.Width() - nSBWidth, 0, nSBWidth, aOutSz.Height() );
}

// --------------------------------------------------------------------

long DropdownMenuBox::PreNotify( NotifyEvent& rNEvt )
{
    long nResult=sal_True;

    sal_uInt16 nSwitch=rNEvt.GetType();
    if (nSwitch==EVENT_KEYINPUT)
    {
        const KeyCode& aKeyCode=rNEvt.GetKeyEvent()->GetKeyCode();
        sal_uInt16 nKey=aKeyCode.GetCode();

        if (nKey==KEY_DOWN && aKeyCode.IsMod2())
        {
            mpDropdownButton->KeyInput( *rNEvt.GetKeyEvent() );
        }
        else
        {
            nResult=Edit::PreNotify(rNEvt);
        }
    }
    else
        nResult=Edit::PreNotify(rNEvt);

    return nResult;
}

// --------------------------------------------------------------------

class CharHeightPropertyBox : public PropertySubControl
{
public:
    CharHeightPropertyBox( sal_Int32 nControlType, Window* pParent, const Any& rValue, const Link& rModifyHdl );
    virtual ~CharHeightPropertyBox();

    virtual Any getValue();
    virtual void setValue( const Any& rValue, const OUString& );

    virtual Control* getControl();

    DECL_LINK( implMenuSelectHdl, MenuButton* );

private:
    DropdownMenuBox* mpControl;
    PopupMenu* mpMenu;
    MetricField* mpMetric;
};

// --------------------------------------------------------------------

CharHeightPropertyBox::CharHeightPropertyBox( sal_Int32 nControlType, Window* pParent, const Any& rValue, const Link& rModifyHdl )
: PropertySubControl( nControlType )
{
    mpMetric = new MetricField( pParent, WB_TABSTOP|WB_IGNORETAB| WB_NOBORDER);
    mpMetric->SetUnit( FUNIT_PERCENT );
    mpMetric->SetMin( 0 );
    mpMetric->SetMax( 1000 );

    mpMenu = new PopupMenu(SdResId( RID_CUSTOMANIMATION_FONTSIZE_POPUP ) );
    mpControl = new DropdownMenuBox( pParent, mpMetric, mpMenu );
    mpControl->SetMenuSelectHdl( LINK( this, CharHeightPropertyBox, implMenuSelectHdl ));
    mpControl->SetModifyHdl( rModifyHdl );
    mpControl->SetHelpId( HID_SD_CUSTOMANIMATIONPANE_CHARHEIGHTPROPERTYBOX );

    OUString aPresetId;
    setValue( rValue, aPresetId );
}

// --------------------------------------------------------------------

CharHeightPropertyBox::~CharHeightPropertyBox()
{
    delete mpControl;
}

// --------------------------------------------------------------------

IMPL_LINK( CharHeightPropertyBox, implMenuSelectHdl, MenuButton*, pPb )
{
    long nValue = 100;
    switch( pPb->GetCurItemId() )
    {
    case CM_SIZE_25: nValue = 25; break;
    case CM_SIZE_50: nValue = 50; break;
    case CM_SIZE_150: nValue = 150; break;
    case CM_SIZE_400: nValue = 400; break;
    }
    mpMetric->SetValue( nValue );
    mpMetric->Modify();
    return 0;
}

// --------------------------------------------------------------------

void CharHeightPropertyBox::setValue( const Any& rValue, const OUString& )
{
    if( mpMetric )
    {
        double fValue = 0.0;
        rValue >>= fValue;
        mpMetric->SetValue( (long)(fValue * 100.0) );
    }
}

// --------------------------------------------------------------------

Any CharHeightPropertyBox::getValue()
{
    return makeAny( (double)((double)mpMetric->GetValue() / 100.0) );
}

// --------------------------------------------------------------------

Control* CharHeightPropertyBox::getControl()
{
    return mpControl;
}

// ====================================================================

class TransparencyPropertyBox : public PropertySubControl
{
public:
    TransparencyPropertyBox( sal_Int32 nControlType, Window* pParent, const Any& rValue, const Link& rModifyHdl );
    ~TransparencyPropertyBox();

    virtual Any getValue();
    virtual void setValue( const Any& rValue, const OUString& rPresetId  );

    virtual Control* getControl();

    DECL_LINK( implMenuSelectHdl, MenuButton* );
    DECL_LINK(implModifyHdl, void *);

    void updateMenu();

private:
    DropdownMenuBox* mpControl;
    PopupMenu* mpMenu;
    MetricField* mpMetric;
    Link maModifyHdl;
};

// --------------------------------------------------------------------

TransparencyPropertyBox::TransparencyPropertyBox( sal_Int32 nControlType, Window* pParent, const Any& rValue, const Link& rModifyHdl )
: PropertySubControl( nControlType )
, maModifyHdl( rModifyHdl )
{
    mpMetric = new MetricField( pParent ,WB_TABSTOP|WB_IGNORETAB| WB_NOBORDER);
    mpMetric->SetUnit( FUNIT_PERCENT );
    mpMetric->SetMin( 0 );
    mpMetric->SetMax( 100 );

    mpMenu = new PopupMenu();
    for( sal_Int32 i = 25; i < 101; i += 25 )
    {
        String aStr(OUString::valueOf(i));
        aStr += sal_Unicode('%');
        mpMenu->InsertItem( i, aStr );
    }

    mpControl = new DropdownMenuBox( pParent, mpMetric, mpMenu );
    mpControl->SetMenuSelectHdl( LINK( this, TransparencyPropertyBox, implMenuSelectHdl ));
    mpControl->SetHelpId( HID_SD_CUSTOMANIMATIONPANE_TRANSPARENCYPROPERTYBOX );

    Link aLink( LINK( this, TransparencyPropertyBox, implModifyHdl ) );
    mpControl->SetModifyHdl( aLink );

    OUString aPresetId;
    setValue( rValue, aPresetId  );
}

// --------------------------------------------------------------------

TransparencyPropertyBox::~TransparencyPropertyBox()
{
    delete mpControl;
}

// --------------------------------------------------------------------

void TransparencyPropertyBox::updateMenu()
{
    sal_Int64 nValue = mpMetric->GetValue();
    for( sal_uInt16 i = 25; i < 101; i += 25 )
        mpMenu->CheckItem( i, nValue == i );
}

// --------------------------------------------------------------------

IMPL_LINK_NOARG(TransparencyPropertyBox, implModifyHdl)
{
    updateMenu();
    maModifyHdl.Call(mpMetric);

    return 0;
}

// --------------------------------------------------------------------

IMPL_LINK( TransparencyPropertyBox, implMenuSelectHdl, MenuButton*, pPb )
{
    if( pPb->GetCurItemId() != mpMetric->GetValue() )
    {
        mpMetric->SetValue( pPb->GetCurItemId() );
        mpMetric->Modify();
    }

    return 0;
}

// --------------------------------------------------------------------

void TransparencyPropertyBox::setValue( const Any& rValue, const OUString& )
{
    if( mpMetric )
    {
        double fValue = 0.0;
        rValue >>= fValue;
        long nValue = (long)(fValue * 100);
        mpMetric->SetValue( nValue );
        updateMenu();
    }
}

// --------------------------------------------------------------------

Any TransparencyPropertyBox::getValue()
{
    return makeAny( (double)((double)mpMetric->GetValue()) / 100.0 );
}

// --------------------------------------------------------------------

Control* TransparencyPropertyBox::getControl()
{
    return mpControl;
}

// --------------------------------------------------------------------

class RotationPropertyBox : public PropertySubControl
{
public:
    RotationPropertyBox( sal_Int32 nControlType, Window* pParent, const Any& rValue, const Link& rModifyHdl );
    ~RotationPropertyBox();

    virtual Any getValue();
    virtual void setValue( const Any& rValue, const OUString& );

    virtual Control* getControl();

    DECL_LINK( implMenuSelectHdl, MenuButton* );
    DECL_LINK(implModifyHdl, void *);

    void updateMenu();

private:
    DropdownMenuBox* mpControl;
    PopupMenu* mpMenu;
    MetricField* mpMetric;
    Link maModifyHdl;
};

// --------------------------------------------------------------------

RotationPropertyBox::RotationPropertyBox( sal_Int32 nControlType, Window* pParent, const Any& rValue, const Link& rModifyHdl )
: PropertySubControl( nControlType )
, maModifyHdl( rModifyHdl )
{
    mpMetric = new MetricField( pParent ,WB_TABSTOP|WB_IGNORETAB| WB_NOBORDER);
    mpMetric->SetUnit( FUNIT_CUSTOM );
    mpMetric->SetCustomUnitText( OUString( sal_Unicode(0xb0)) ); // degree sign
    mpMetric->SetMin( -10000 );
    mpMetric->SetMax( 10000 );

    mpMenu = new PopupMenu(SdResId( RID_CUSTOMANIMATION_ROTATION_POPUP ) );
    mpControl = new DropdownMenuBox( pParent, mpMetric, mpMenu );
    mpControl->SetMenuSelectHdl( LINK( this, RotationPropertyBox, implMenuSelectHdl ));
    mpControl->SetHelpId( HID_SD_CUSTOMANIMATIONPANE_ROTATIONPROPERTYBOX );

    Link aLink( LINK( this, RotationPropertyBox, implModifyHdl ) );
    mpControl->SetModifyHdl( aLink );

    OUString aPresetId;
    setValue( rValue, aPresetId );
}

// --------------------------------------------------------------------

RotationPropertyBox::~RotationPropertyBox()
{
    delete mpControl;
}

// --------------------------------------------------------------------

void RotationPropertyBox::updateMenu()
{
    sal_Int64 nValue = mpMetric->GetValue();
    bool bDirection = nValue >= 0;
    nValue = (nValue < 0 ? -nValue : nValue);

    mpMenu->CheckItem( CM_QUARTER_SPIN, nValue == 90 );
    mpMenu->CheckItem( CM_HALF_SPIN, nValue == 180 );
    mpMenu->CheckItem( CM_FULL_SPIN, nValue == 360 );
    mpMenu->CheckItem( CM_TWO_SPINS, nValue == 720 );

    mpMenu->CheckItem( CM_CLOCKWISE, bDirection );
    mpMenu->CheckItem( CM_COUNTERCLOCKWISE, !bDirection );
}

// --------------------------------------------------------------------

IMPL_LINK_NOARG(RotationPropertyBox, implModifyHdl)
{
    updateMenu();
    maModifyHdl.Call(mpMetric);

    return 0;
}

IMPL_LINK( RotationPropertyBox, implMenuSelectHdl, MenuButton*, pPb )
{
    sal_Int64 nValue = mpMetric->GetValue();
    bool bDirection = nValue >= 0;
    nValue = (nValue < 0 ? -nValue : nValue);

    switch( pPb->GetCurItemId() )
    {
    case CM_QUARTER_SPIN: nValue = 90; break;
    case CM_HALF_SPIN: nValue = 180; break;
    case CM_FULL_SPIN: nValue = 360; break;
    case CM_TWO_SPINS: nValue = 720; break;

    case CM_CLOCKWISE: bDirection = true; break;
    case CM_COUNTERCLOCKWISE: bDirection = false; break;

    }

    if( !bDirection )
        nValue = -nValue;

    if( nValue != mpMetric->GetValue() )
    {
        mpMetric->SetValue( nValue );
        mpMetric->Modify();
    }

    return 0;
}

// --------------------------------------------------------------------

void RotationPropertyBox::setValue( const Any& rValue, const OUString& )
{
    if( mpMetric )
    {
        double fValue = 0.0;
        rValue >>= fValue;
        long nValue = (long)(fValue);
        mpMetric->SetValue( nValue );
        updateMenu();
    }
}

// --------------------------------------------------------------------

Any RotationPropertyBox::getValue()
{
    return makeAny( (double)((double)mpMetric->GetValue()) );
}

// --------------------------------------------------------------------

Control* RotationPropertyBox::getControl()
{
    return mpControl;
}

// --------------------------------------------------------------------

class ScalePropertyBox : public PropertySubControl
{
public:
    ScalePropertyBox( sal_Int32 nControlType, Window* pParent, const Any& rValue, const Link& rModifyHdl );
    ~ScalePropertyBox();

    virtual Any getValue();
    virtual void setValue( const Any& rValue, const OUString& );

    virtual Control* getControl();

    DECL_LINK( implMenuSelectHdl, MenuButton* );
    DECL_LINK(implModifyHdl, void *);

    void updateMenu();

private:
    DropdownMenuBox* mpControl;
    PopupMenu* mpMenu;
    MetricField* mpMetric;
    Link maModifyHdl;
    int mnDirection;
};

// --------------------------------------------------------------------

ScalePropertyBox::ScalePropertyBox( sal_Int32 nControlType, Window* pParent, const Any& rValue, const Link& rModifyHdl )
: PropertySubControl( nControlType )
, maModifyHdl( rModifyHdl )
{
    mpMetric = new MetricField( pParent ,WB_TABSTOP|WB_IGNORETAB| WB_NOBORDER);
    mpMetric->SetUnit( FUNIT_PERCENT );
    mpMetric->SetMin( 0 );
    mpMetric->SetMax( 10000 );

    mpMenu = new PopupMenu(SdResId( RID_CUSTOMANIMATION_SCALE_POPUP ) );
    mpControl = new DropdownMenuBox( pParent, mpMetric, mpMenu );
    mpControl->SetMenuSelectHdl( LINK( this, ScalePropertyBox, implMenuSelectHdl ));
    mpControl->SetHelpId( HID_SD_CUSTOMANIMATIONPANE_SCALEPROPERTYBOX );

    Link aLink( LINK( this, ScalePropertyBox, implModifyHdl ) );
    mpControl->SetModifyHdl( aLink );

    OUString aPresetId;
    setValue( rValue, aPresetId );
}

// --------------------------------------------------------------------

ScalePropertyBox::~ScalePropertyBox()
{
    delete mpControl;
}

// --------------------------------------------------------------------

void ScalePropertyBox::updateMenu()
{
    sal_Int64 nValue = mpMetric->GetValue();

    mpMenu->CheckItem( 25, nValue == 25 );
    mpMenu->CheckItem( 50, nValue == 50 );
    mpMenu->CheckItem( 150, nValue == 150 );
    mpMenu->CheckItem( 400, nValue == 400 );

    mpMenu->CheckItem( CM_HORIZONTAL, mnDirection == 1 );
    mpMenu->CheckItem( CM_VERTICAL, mnDirection == 2 );
    mpMenu->CheckItem( CM_BOTH, mnDirection == 3 );
}

// --------------------------------------------------------------------

IMPL_LINK_NOARG(ScalePropertyBox, implModifyHdl)
{
    updateMenu();
    maModifyHdl.Call(mpMetric);

    return 0;
}

IMPL_LINK( ScalePropertyBox, implMenuSelectHdl, MenuButton*, pPb )
{
    sal_Int64 nValue = mpMetric->GetValue();

    int nDirection = mnDirection;

    switch( pPb->GetCurItemId() )
    {
    case CM_HORIZONTAL: nDirection = 1; break;
    case CM_VERTICAL: nDirection = 2; break;
    case CM_BOTH: nDirection = 3; break;

    default:
        nValue = pPb->GetCurItemId();
    }

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

    return 0;
}

// --------------------------------------------------------------------

void ScalePropertyBox::setValue( const Any& rValue, const OUString& )
{
    if( mpMetric )
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

        long nValue;
        if( fValue1 )
            nValue = (long)(fValue1 * 100.0);
        else
            nValue = (long)(fValue2 * 100.0);
        mpMetric->SetValue( nValue );
        updateMenu();
    }
}

// --------------------------------------------------------------------

Any ScalePropertyBox::getValue()
{
    double fValue1 = (double)((double)mpMetric->GetValue() / 100.0);
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

// --------------------------------------------------------------------

Control* ScalePropertyBox::getControl()
{
    return mpControl;
}

// ====================================================================

class FontStylePropertyBox : public PropertySubControl
{
public:
    FontStylePropertyBox( sal_Int32 nControlType, Window* pParent, const Any& rValue, const Link& rModifyHdl );
    ~FontStylePropertyBox();

    virtual Any getValue();
    virtual void setValue( const Any& rValue, const OUString& );

    virtual Control* getControl();

    DECL_LINK( implMenuSelectHdl, MenuButton* );

    void update();

private:
    DropdownMenuBox* mpControl;
    PopupMenu* mpMenu;
    Edit* mpEdit;
    Link maModifyHdl;

    float mfFontWeight;
    awt::FontSlant meFontSlant;
    sal_Int16 mnFontUnderline;
};

// --------------------------------------------------------------------

FontStylePropertyBox::FontStylePropertyBox( sal_Int32 nControlType, Window* pParent, const Any& rValue, const Link& rModifyHdl )
: PropertySubControl( nControlType )
, maModifyHdl( rModifyHdl )
{
    mpEdit = new Edit( pParent, WB_TABSTOP|WB_IGNORETAB|WB_NOBORDER|WB_READONLY);
    mpEdit->SetText( String( SdResId( STR_CUSTOMANIMATION_SAMPLE ) ) );

    mpMenu = new PopupMenu(SdResId( RID_CUSTOMANIMATION_FONTSTYLE_POPUP ) );
    mpControl = new DropdownMenuBox( pParent, mpEdit, mpMenu );
    mpControl->SetMenuSelectHdl( LINK( this, FontStylePropertyBox, implMenuSelectHdl ));
    mpControl->SetHelpId( HID_SD_CUSTOMANIMATIONPANE_FONTSTYLEPROPERTYBOX );

    OUString aPresetId;
    setValue( rValue, aPresetId );
}

// --------------------------------------------------------------------

FontStylePropertyBox::~FontStylePropertyBox()
{
    delete mpControl;
}

// --------------------------------------------------------------------

void FontStylePropertyBox::update()
{
    // update menu
    mpMenu->CheckItem( CM_BOLD, mfFontWeight == awt::FontWeight::BOLD );
    mpMenu->CheckItem( CM_ITALIC, meFontSlant == awt::FontSlant_ITALIC);
    mpMenu->CheckItem( CM_UNDERLINED, mnFontUnderline != awt::FontUnderline::NONE );

    // update sample edit
    Font aFont( mpEdit->GetFont() );
    aFont.SetWeight( mfFontWeight == awt::FontWeight::BOLD ? WEIGHT_BOLD : WEIGHT_NORMAL );
    aFont.SetItalic( meFontSlant == awt::FontSlant_ITALIC ? ITALIC_NORMAL : ITALIC_NONE  );
    aFont.SetUnderline( mnFontUnderline == awt::FontUnderline::NONE ? UNDERLINE_NONE : UNDERLINE_SINGLE );
    mpEdit->SetFont( aFont );
    mpEdit->Invalidate();
}

// --------------------------------------------------------------------

IMPL_LINK( FontStylePropertyBox, implMenuSelectHdl, MenuButton*, pPb )
{
    switch( pPb->GetCurItemId() )
    {
    case CM_BOLD:
        if( mfFontWeight == awt::FontWeight::BOLD )
            mfFontWeight = awt::FontWeight::NORMAL;
        else
            mfFontWeight = awt::FontWeight::BOLD;
        break;
    case CM_ITALIC:
        if( meFontSlant == awt::FontSlant_ITALIC )
            meFontSlant = awt::FontSlant_NONE;
        else
            meFontSlant = awt::FontSlant_ITALIC;
        break;
    case CM_UNDERLINED:
        if( mnFontUnderline == awt::FontUnderline::SINGLE )
            mnFontUnderline = awt::FontUnderline::NONE;
        else
            mnFontUnderline = awt::FontUnderline::SINGLE;
        break;
    default:
        return 0;
    }

    update();
    maModifyHdl.Call(mpEdit);

    return 0;
}

// --------------------------------------------------------------------

void FontStylePropertyBox::setValue( const Any& rValue, const OUString& )
{
    Sequence<Any> aValues;
    rValue >>= aValues;

    aValues[0] >>= mfFontWeight;
    aValues[1] >>= meFontSlant;
    aValues[2] >>= mnFontUnderline;

    update();
}

// --------------------------------------------------------------------

Any FontStylePropertyBox::getValue()
{
    Sequence<Any> aValues(3);
    aValues[0] <<= mfFontWeight;
    aValues[1] <<= meFontSlant;
    aValues[2] <<= mnFontUnderline;
    return makeAny( aValues );
}

// --------------------------------------------------------------------

Control* FontStylePropertyBox::getControl()
{
    return mpControl;
}

// ====================================================================

class CustomAnimationEffectTabPage : public TabPage
{
public:
    CustomAnimationEffectTabPage( Window* pParent, const STLPropertySet* pSet );
    ~CustomAnimationEffectTabPage();

    void update( STLPropertySet* pSet );
    DECL_LINK( implSelectHdl, Control* );

private:
    void updateControlStates();
    void fillSoundListBox();
    void clearSoundListBox();
    sal_Int32 getSoundObject( const String& rStr );
    void openSoundFileDialog();
    void onSoundPreview();

private:
    ::std::vector< String > maSoundList;
    sal_Bool mbHasText;
    const STLPropertySet* mpSet;

    FixedText*      mpFTProperty1;
    PropertyControl* mpLBProperty1;
    VclHBox*        mpPlaceholderBox;
    FixedText*      mpFTProperty2;
    PropertyControl* mpLBProperty2;
    CheckBox*       mpCBSmoothStart;
    CheckBox*       mpCBSmoothEnd;
    CheckBox*       mpCBAutoRestart;

    FixedText*      mpFTSound;
    ListBox*        mpLBSound;
    PushButton*     mpPBSoundPreview;
    FixedText*      mpFTAfterEffect;
    ListBox*        mpLBAfterEffect;
    FixedText*      mpFTDimColor;
    ColorListBox*   mpCLBDimColor;
    FixedText*      mpFTTextAnim;
    ListBox*        mpLBTextAnim;
    MetricField*    mpMFTextDelay;
    FixedText*      mpFTTextDelay;

    ::com::sun::star::uno::Reference< ::com::sun::star::media::XPlayer > mxPlayer;
};


static void move_down( Control* pControl, int nOffsetX, int nOffsetY )
{
    Point aPos( pControl->GetPosPixel() );
    aPos.X() += nOffsetX;
    aPos.Y() += nOffsetY;
    pControl->SetPosPixel( aPos );
}

CustomAnimationEffectTabPage::CustomAnimationEffectTabPage( Window* pParent, const STLPropertySet* pSet )
: TabPage( pParent, "EffectTab", "modules/simpress/ui/customanimationeffecttab.ui" ), mbHasText( sal_False ), mpSet(pSet )
{
    get(mpFTProperty1, "prop_label1" );
    get(mpLBProperty1, "prop_list1" );
    //get(mpFTProperty2, "prop_label2" );
    //get(mpLBProperty2, "prop_list2" );
    get(mpPlaceholderBox, "placeholder" );
    get(mpCBSmoothStart, "smooth_start" );
    get(mpCBSmoothEnd, "smooth_end" );
    get(mpCBAutoRestart, "auto_restart" );
    get(mpFTSound, "sound_label");
    get(mpLBSound, "sound_list" );
    get(mpPBSoundPreview, "sound_preview" );
    get(mpFTAfterEffect, "aeffect_label" );
    get(mpLBAfterEffect, "aeffect_list" );
    get(mpFTDimColor, "dim_color_label" );
    get(mpCLBDimColor, "dim_color_list" );
    get(mpFTTextAnim, "text_animation_label" );
    get(mpLBTextAnim, "text_animation_list" );
    get(mpMFTextDelay,"text_delay" );
    get(mpFTTextDelay,"text_delay_label" );

    //FreeResource();

    // fill the soundbox
    fillSoundListBox();

    mpLBSound->SetSelectHdl( LINK( this, CustomAnimationEffectTabPage, implSelectHdl ) );

    mpPBSoundPreview->SetClickHdl( LINK( this, CustomAnimationEffectTabPage, implSelectHdl ) );
    mpPBSoundPreview->SetSymbol( SYMBOL_PLAY );

    // fill the color box
    SfxObjectShell* pDocSh = SfxObjectShell::Current();
    DBG_ASSERT( pDocSh, "DocShell not found!" );
    XColorListRef pColorList;
    const SfxPoolItem* pItem = NULL;

    if ( pDocSh && ( (pItem = pDocSh->GetItem( SID_COLOR_TABLE ) ) != 0 ) )
        pColorList = ( (SvxColorListItem*)pItem )->GetColorList();

    if ( !pColorList.is() )
        pColorList = XColorList::CreateStdColorList();

    mpCLBDimColor->SetUpdateMode( sal_False );

    for ( long i = 0; i < pColorList->Count(); i++ )
    {
        XColorEntry* pEntry = pColorList->GetColor(i);
        mpCLBDimColor->InsertEntry( pEntry->GetColor(), pEntry->GetName() );
    }

    mpCLBDimColor->SetUpdateMode( sal_True );

    //
    // init settings controls
    //
    int nOffsetY = 0;
    int nOffsetX = 0;

    Size aSpace( LogicToPixel( Size( 3, 3 ), MAP_APPFONT ) );

    // only show settings if all selected effects have the same preset-id
    if( pSet->getPropertyState( nHandlePresetId ) != STLPropertyState_AMBIGUOUS )
    {
        OUString aPresetId;
        pSet->getPropertyValue( nHandlePresetId ) >>= aPresetId;

        //
        // property 1
        //

        if( pSet->getPropertyState( nHandleProperty1Type ) != STLPropertyState_AMBIGUOUS )
        {
            sal_Int32 nType = 0;
            pSet->getPropertyValue( nHandleProperty1Type ) >>= nType;

            if( nType != nPropertyTypeNone )
            {
                // set ui name for property at fixed text
                OUString aPropertyName( getPropertyName( nType ) );

                if( !aPropertyName.isEmpty() )
                {
                    mpFTProperty1->Show();
                    mpLBProperty1->Show();

                    //nOffsetY += mpLBProperty1->GetSizePixel().Height() + aSpace.Height();

                    mpFTProperty1->SetText( aPropertyName );
                }

                // get property value
                const Any aValue( pSet->getPropertyValue( nHandleProperty1Value ) );

                Link aModifyLink;
                // create property sub control
                mpLBProperty1->setSubControl( PropertySubControl::create( nType, mpPlaceholderBox, aValue, aPresetId, aModifyLink ));
            }
        }

        mpFTProperty1->Enable( mpLBProperty1->IsEnabled() );

        //
        // accelerate & deccelerate
        //

        if( pSet->getPropertyState( nHandleAccelerate ) == STLPropertyState_DIRECT )
        {
            mpCBSmoothStart->Show();
            mpCBSmoothEnd->Show();

            //move_down( mpCBSmoothStart, nOffsetX, nOffsetY );
            //move_down( mpCBSmoothEnd, nOffsetX, nOffsetY );

            nOffsetY += mpCBSmoothStart->GetSizePixel().Height() + aSpace.Height();

            double fTemp = 0.0;
            pSet->getPropertyValue( nHandleAccelerate ) >>= fTemp;
            mpCBSmoothStart->Check( fTemp > 0.0 );

            pSet->getPropertyValue( nHandleDecelerate ) >>= fTemp;
            mpCBSmoothEnd->Check( fTemp > 0.0 );
        }

        //
        // auto reverse
        //


        /*if( nOffsetY )
        {
            nOffsetY += mpFLSettings->GetSizePixel().Height() + aSpace.Height();
            mpFLSettings->Show();

            mpFLEnhancements->Show();
            move_down( mpFLEnhancements, nOffsetX, nOffsetY );

            nOffsetY += mpFLEnhancements->GetSizePixel().Height() + aSpace.Height();

            nOffsetX = 2* aSpace.Width();
        }*/
    }

    /*if( (nOffsetY != 0) || (nOffsetX != 0) )
    {
        move_down( mpFTSound, nOffsetX, nOffsetY );
        move_down( mpLBSound, nOffsetX, nOffsetY );
        move_down( mpPBSoundPreview, nOffsetX, nOffsetY );
        move_down( mpFTAfterEffect, nOffsetX, nOffsetY );
        move_down( mpLBAfterEffect, nOffsetX, nOffsetY );
        move_down( mpFTDimColor, nOffsetX, nOffsetY );
        move_down( mpCLBDimColor, nOffsetX, nOffsetY );
        move_down( mpFTTextAnim, nOffsetX, nOffsetY );
        move_down( mpLBTextAnim, nOffsetX, nOffsetY );
        move_down( mpMFTextDelay, nOffsetX, nOffsetY );
        move_down( mpFTTextDelay, nOffsetX, nOffsetY );
    }*/

    //
    // init after effect controls
    //

    mpLBAfterEffect->SetSelectHdl( LINK( this, CustomAnimationEffectTabPage, implSelectHdl ) );
    mpLBTextAnim->SetSelectHdl( LINK( this, CustomAnimationEffectTabPage, implSelectHdl ) );

    if( (pSet->getPropertyState( nHandleHasAfterEffect ) != STLPropertyState_AMBIGUOUS) &&
        (pSet->getPropertyState( nHandleAfterEffectOnNextEffect ) != STLPropertyState_AMBIGUOUS) &&
        (pSet->getPropertyState( nHandleDimColor ) != STLPropertyState_AMBIGUOUS))
    {
        sal_Bool bHasAfterEffect = sal_False;
        pSet->getPropertyValue( nHandleHasAfterEffect ) >>= bHasAfterEffect;

        sal_uInt16 nPos = 0;
        if( bHasAfterEffect )
        {
            nPos++;

            sal_Bool bAfterEffectOnNextClick = sal_False;
            pSet->getPropertyValue( nHandleAfterEffectOnNextEffect ) >>= bAfterEffectOnNextClick;
            Any aDimColor( pSet->getPropertyValue( nHandleDimColor ) );

            if( aDimColor.hasValue() )
            {
                sal_Int32 nColor = 0;
                aDimColor >>= nColor;
                Color aColor( nColor );
                sal_uInt16 nColorPos = mpCLBDimColor->GetEntryPos( aColor );
                if ( LISTBOX_ENTRY_NOTFOUND != nColorPos )
                    mpCLBDimColor->SelectEntryPos( nColorPos );
                else
                    mpCLBDimColor->SelectEntryPos(
                        mpCLBDimColor->InsertEntry( aColor, String( SVX_RES( RID_SVXSTR_COLOR_USER ) ) ) );
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

    if( pSet->getPropertyState( nHandleHasText ) != STLPropertyState_AMBIGUOUS )
        pSet->getPropertyValue( nHandleHasText ) >>= mbHasText;

    if( mbHasText )
    {
        if( pSet->getPropertyState( nHandleIterateType ) != STLPropertyState_AMBIGUOUS)
        {
            sal_uInt16 nPos = LISTBOX_ENTRY_NOTFOUND;

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

        if( pSet->getPropertyState( nHandleIterateInterval ) )
        {
            double fIterateInterval = 0.0;
            pSet->getPropertyValue( nHandleIterateInterval ) >>= fIterateInterval;
            mpMFTextDelay->SetValue( (long)(fIterateInterval*10) );
        }
    }
    else
    {
        mpFTTextAnim->Enable( sal_False );
        mpLBTextAnim->Enable( sal_False );
        mpMFTextDelay->Enable( sal_False );
        mpFTTextDelay->Enable( sal_False );

    }

    if( pSet->getPropertyState( nHandleSoundURL ) != STLPropertyState_AMBIGUOUS )
    {
        sal_uInt16 nPos = 0;

        const Any aValue( pSet->getPropertyValue( nHandleSoundURL ) );

        if( aValue.getValueType() == ::getCppuType((const sal_Bool*)0) )
        {
            nPos = 1;
        }
        else
        {
            OUString aSoundURL;
            aValue >>= aSoundURL;

            if( !aSoundURL.isEmpty() )
            {
                const String aTmp( aSoundURL );

                sal_uLong i;
                for( i = 0; i < maSoundList.size(); i++ )
                {
                    String aString = maSoundList[ i ];
                    if( aString == aTmp )
                    {
                        nPos = (sal_uInt16)i+2;
                        break;
                    }
                }

                if( nPos == 0 )
                {
                    nPos = (sal_uInt16)maSoundList.size()+2;
                    maSoundList.push_back( String( aTmp ) );
                    INetURLObject aURL( aTmp );
                    nPos = mpLBSound->InsertEntry( aURL.GetBase(), nPos );
                }
            }
        }

        if( nPos != LISTBOX_ENTRY_NOTFOUND )
            mpLBSound->SelectEntryPos( nPos );
    }

    updateControlStates();

    //Size aSize( GetSizePixel() );
    //aSize.Height() += mpMFTextDelay->GetPosPixel().X() + GetSizePixel().Height() + aSpace.Height();
    //SetSizePixel( aSize );
}

CustomAnimationEffectTabPage::~CustomAnimationEffectTabPage()
{
    clearSoundListBox();
}

void CustomAnimationEffectTabPage::updateControlStates()
{
    sal_uInt16 nPos = mpLBAfterEffect->GetSelectEntryPos();
    mpCLBDimColor->Enable( nPos == 1 );
    mpFTDimColor->Enable( nPos == 1 );

    if( mbHasText )
    {
        nPos = mpLBTextAnim->GetSelectEntryPos();
        mpMFTextDelay->Enable( nPos != 0 );
        mpFTTextDelay->Enable( nPos != 0 );
    }

    nPos = mpLBSound->GetSelectEntryPos();
    mpPBSoundPreview->Enable( nPos >= 2 );
}

IMPL_LINK( CustomAnimationEffectTabPage, implSelectHdl, Control*, pControl )
{
    if( pControl == mpLBAfterEffect )
    {
        sal_uInt16 nPos = static_cast<ListBox*>( mpLBAfterEffect )->GetSelectEntryPos();
        if( nPos == 1 )
        {
            if( mpCLBDimColor->GetSelectEntryPos() == LISTBOX_ENTRY_NOTFOUND )
                mpCLBDimColor->SelectEntryPos(0);
        }
    }
    else if( pControl == mpLBTextAnim )
    {
        if( mpMFTextDelay->GetValue() == 0 )
            mpMFTextDelay->SetValue( 100 );
    }
    else if( pControl == mpLBSound )
    {
        sal_uInt16 nPos = mpLBSound->GetSelectEntryPos();
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
    return 0;
}

void CustomAnimationEffectTabPage::update( STLPropertySet* pSet )
{
    if( mpLBProperty1->getSubControl() )
    {
        Any aNewValue( mpLBProperty1->getSubControl()->getValue() );
        Any aOldValue;
        if( mpSet->getPropertyState( nHandleProperty1Value ) != STLPropertyState_AMBIGUOUS)
            aOldValue = mpSet->getPropertyValue( nHandleProperty1Value );

        if( aOldValue != aNewValue )
            pSet->setPropertyValue( nHandleProperty1Value, aNewValue );
    }

    /*if( mpLBProperty2->getSubControl() )
    {
        Any aNewValue( mpLBProperty2->getSubControl()->getValue() );
        Any aOldValue;
        if( mpSet->getPropertyState( nHandleProperty2Value ) != STLPropertyState_AMBIGUOUS)
            aOldValue = mpSet->getPropertyValue( nHandleProperty2Value );

        if( aOldValue != aNewValue )
            pSet->setPropertyValue( nHandleProperty2Value, aNewValue );
    }*/

    if( mpCBSmoothStart->IsVisible() )
    {
        // set selected value for accelerate if different then in original set

        double fTemp = mpCBSmoothStart->IsChecked() ? 0.5 : 0.0;

        double fOldTemp = 0.0;
        if(mpSet->getPropertyState( nHandleAccelerate ) != STLPropertyState_AMBIGUOUS)
            mpSet->getPropertyValue( nHandleAccelerate ) >>= fOldTemp;
        else
            fOldTemp = -2.0;

        if( fOldTemp != fTemp )
            pSet->setPropertyValue( nHandleAccelerate, makeAny( fTemp ) );

        // set selected value for decelerate if different then in original set
        fTemp = mpCBSmoothEnd->IsChecked() ? 0.5 : 0.0;

        if(mpSet->getPropertyState( nHandleDecelerate ) != STLPropertyState_AMBIGUOUS)
            mpSet->getPropertyValue( nHandleDecelerate ) >>= fOldTemp;
        else
            fOldTemp = -2.0;

        if( fOldTemp != fTemp )
            pSet->setPropertyValue( nHandleDecelerate, makeAny( fTemp ) );
    }

    sal_uInt16 nPos = mpLBAfterEffect->GetSelectEntryPos();
    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        sal_Bool bAfterEffect = nPos != 0;

        sal_Bool bOldAfterEffect = sal_False;

        if(mpSet->getPropertyState( nHandleHasAfterEffect ) != STLPropertyState_AMBIGUOUS)
            mpSet->getPropertyValue( nHandleHasAfterEffect ) >>= bOldAfterEffect;
        else
            bOldAfterEffect = !bAfterEffect;

        if( bOldAfterEffect != bAfterEffect )
            pSet->setPropertyValue( nHandleHasAfterEffect, makeAny( bAfterEffect ) );

        Any aDimColor;
        if( nPos == 1 )
        {
            Color aSelectedColor;
            if ( mpCLBDimColor->GetSelectEntryPos() != LISTBOX_ENTRY_NOTFOUND )
                aSelectedColor = mpCLBDimColor->GetSelectEntryColor();

            aDimColor = makeAny( makeAny( (sal_Int32)aSelectedColor.GetRGBColor() ) );
        }

        if( (mpSet->getPropertyState( nHandleDimColor ) == STLPropertyState_AMBIGUOUS) ||
            (mpSet->getPropertyValue( nHandleDimColor ) != aDimColor) )
            pSet->setPropertyValue( nHandleDimColor, makeAny( aDimColor ) );

        sal_Bool bAfterEffectOnNextEffect = nPos != 2 ? sal_True : sal_False;
        sal_Bool bOldAfterEffectOnNextEffect = !bAfterEffectOnNextEffect;

        if( mpSet->getPropertyState( nHandleAfterEffectOnNextEffect ) != STLPropertyState_AMBIGUOUS)
            mpSet->getPropertyValue( nHandleAfterEffectOnNextEffect ) >>= bOldAfterEffectOnNextEffect;

        if( bAfterEffectOnNextEffect != bOldAfterEffectOnNextEffect )
            pSet->setPropertyValue( nHandleAfterEffectOnNextEffect, makeAny( bAfterEffectOnNextEffect ) );
    }

    // ---

    nPos = mpLBTextAnim->GetSelectEntryPos();
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

        if(mpSet->getPropertyState( nHandleIterateType ) != STLPropertyState_AMBIGUOUS)
            mpSet->getPropertyValue( nHandleIterateType ) >>= nOldIterateType;

        if( nIterateType != nOldIterateType )
            pSet->setPropertyValue( nHandleIterateType, makeAny( nIterateType ) );
    }

    // ---

    {
        double fIterateInterval = static_cast< double >( mpMFTextDelay->GetValue() ) / 10;
        double fOldIterateInterval = -1.0;

        if( mpSet->getPropertyState( nHandleIterateInterval ) != STLPropertyState_AMBIGUOUS )
            mpSet->getPropertyValue( nHandleIterateInterval ) >>= fOldIterateInterval;

        if( fIterateInterval != fOldIterateInterval )
            pSet->setPropertyValue( nHandleIterateInterval, makeAny( fIterateInterval ) );
    }

    nPos = mpLBSound->GetSelectEntryPos();
    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        Any aNewSoundURL, aOldSoundURL( makeAny( (sal_Int32) 0 ) );

        if( nPos == 0 )
        {
            // 0 means no sound, so leave any empty
        }
        else if( nPos == 1 )
        {
            // this means stop sound
            aNewSoundURL = makeAny( (sal_Bool)sal_True );
        }
        else
        {
            OUString aSoundURL( maSoundList[ nPos-2 ] );
            aNewSoundURL = makeAny( aSoundURL );
        }

        if( mpSet->getPropertyState( nHandleSoundURL ) != STLPropertyState_AMBIGUOUS )
            mpSet->getPropertyValue( nHandleSoundURL  ) >>= aOldSoundURL;

        if( aNewSoundURL != aOldSoundURL )
            pSet->setPropertyValue( nHandleSoundURL, aNewSoundURL );
    }
}

void CustomAnimationEffectTabPage::fillSoundListBox()
{
    GalleryExplorer::FillObjList( GALLERY_THEME_SOUNDS, maSoundList );
    GalleryExplorer::FillObjList( GALLERY_THEME_USERSOUNDS, maSoundList );

    mpLBSound->InsertEntry( String( SdResId( STR_CUSTOMANIMATION_NO_SOUND ) ) );
    mpLBSound->InsertEntry( String( SdResId( STR_CUSTOMANIMATION_STOP_PREVIOUS_SOUND ) ) );
    for( size_t i = 0; i < maSoundList.size(); i++ )
    {
        String aString = maSoundList[ i ];
        INetURLObject aURL( aString );
        mpLBSound->InsertEntry( aURL.GetBase() );
    }
    mpLBSound->InsertEntry( String( SdResId( STR_CUSTOMANIMATION_BROWSE_SOUND ) ) );
}

void CustomAnimationEffectTabPage::clearSoundListBox()
{
    maSoundList.clear();
    mpLBSound->Clear();
}

sal_Int32 CustomAnimationEffectTabPage::getSoundObject( const String& rStr )
{
    String aStrIn( rStr );
    aStrIn.ToLowerAscii();

    size_t i;
    const size_t nCount = maSoundList.size();
    for( i = 0; i < nCount; i++ )
    {
        String aTmpStr( maSoundList[ i ] );
        aTmpStr.ToLowerAscii();

        if( aTmpStr == aStrIn )
            return i+2;
    }

    return -1;
}

void CustomAnimationEffectTabPage::openSoundFileDialog()
{
    SdOpenSoundFileDialog   aFileDialog;

    String aFile( SvtPathOptions().GetGraphicPath() );
    aFileDialog.SetPath( aFile );

    bool bValidSoundFile = false;
    bool bQuitLoop = false;
    long nPos = 0;

    while( !bQuitLoop && (aFileDialog.Execute() == ERRCODE_NONE) )
    {
        aFile = aFileDialog.GetPath();
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
                String aStrWarning(SdResId(STR_WARNING_NOSOUNDFILE));
                String aStr; aStr += sal_Unicode('%');
                aStrWarning.SearchAndReplace( aStr , aFile );
                WarningBox aWarningBox( NULL, WB_3DLOOK | WB_RETRY_CANCEL, aStrWarning );
                aWarningBox.SetModalInputMode (sal_True);
                bQuitLoop = aWarningBox.Execute()==RET_RETRY ? sal_False : sal_True;

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

    mpLBSound->SelectEntryPos( (sal_uInt16) nPos );
}

void CustomAnimationEffectTabPage::onSoundPreview()
{
    const sal_uInt16 nPos = mpLBSound->GetSelectEntryPos();

    if( nPos >= 2 ) try
    {
        const OUString aSoundURL( maSoundList[ nPos-2 ] );
        mxPlayer.set( avmedia::MediaWindow::createPlayer( aSoundURL ), uno::UNO_QUERY_THROW );
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
    CustomAnimationDurationTabPage( Window* pParent, const STLPropertySet* pSet );
    ~CustomAnimationDurationTabPage();

    void update( STLPropertySet* pSet );

    DECL_LINK( implControlHdl, Control* );

private:
    const STLPropertySet* mpSet;

    FixedText* mpFTStart;
    ListBox* mpLBStart;
    FixedText* mpFTStartDelay;
    MetricField* mpMFStartDelay;
    FixedText* mpFTDuration;
    ListBox* mpCBDuration;
    FixedText* mpFTRepeat;
    ListBox* mpCBRepeat;
    CheckBox* mpCBXRewind;
    RadioButton* mpRBClickSequence;
    RadioButton* mpRBInteractive;
    ListBox* mpLBTrigger;
};

CustomAnimationDurationTabPage::CustomAnimationDurationTabPage(Window* pParent, const STLPropertySet* pSet)
: TabPage( pParent, "TimingTab", "modules/simpress/ui/customanimationtimingtab.ui" ), mpSet( pSet )
{
    get(mpFTStart,"start_label" );
    get(mpLBStart, "start_list" );
    get(mpFTStartDelay, "delay_label" );
    get(mpMFStartDelay, "delay_value" );
    get(mpFTDuration, "duration_label" );
    get(mpCBDuration, "duration_list" );
    get(mpFTRepeat, "repeat_label" );
    get(mpCBRepeat, "repeat_list" );
    get(mpCBXRewind, "rewind" );
    get(mpRBClickSequence, "rb_click_sequence" );
    get(mpRBInteractive, "rb_interactive" );
    get(mpLBTrigger, "trigger_list");

    //fillRepeatComboBox( mpCBRepeat.get() );
    //fillDurationComboBox( mpCBDuration.get() );

    //FreeResource();

    mpRBClickSequence->SetClickHdl( LINK( this, CustomAnimationDurationTabPage, implControlHdl ) );
    mpLBTrigger->SetSelectHdl( LINK( this, CustomAnimationDurationTabPage, implControlHdl ) );

    if( pSet->getPropertyState( nHandleStart ) != STLPropertyState_AMBIGUOUS )
    {
        sal_Int16 nStart = 0;
        pSet->getPropertyValue( nHandleStart ) >>= nStart;
        sal_uInt16 nPos = 0;
        switch( nStart )
        {
            case EffectNodeType::WITH_PREVIOUS:     nPos = 1; break;
            case EffectNodeType::AFTER_PREVIOUS:    nPos = 2; break;
        }
        mpLBStart->SelectEntryPos( nPos );
    }

    if( pSet->getPropertyState( nHandleBegin ) != STLPropertyState_AMBIGUOUS )
    {
        double fBegin = 0.0;
        pSet->getPropertyValue( nHandleBegin ) >>= fBegin;
        mpMFStartDelay->SetValue( (long)(fBegin*10) );
    }

    if( pSet->getPropertyState( nHandleDuration ) != STLPropertyState_AMBIGUOUS )
    {
        double fDuration = 0.0;
        pSet->getPropertyValue( nHandleDuration ) >>= fDuration;

        if( fDuration == 0.001 )
        {
            mpFTDuration->Disable();
            mpCBDuration->Disable();
            mpFTRepeat->Disable();
            mpCBRepeat->Disable();
            mpCBXRewind->Disable();
        }
        else
        {
            sal_uInt16 nPos = LISTBOX_ENTRY_NOTFOUND;

            if( fDuration == 5.0 )
                nPos = 0;
            else if( fDuration == 3.0 )
                nPos = 1;
            else if( fDuration == 2.0 )
                nPos = 2;
            else if( fDuration == 1.0 )
                nPos = 3;
            else if( fDuration == 0.5 )
                nPos = 4;

            if( nPos != LISTBOX_ENTRY_NOTFOUND )
                mpCBDuration->SelectEntryPos( nPos );
            else
                mpCBDuration->SetText(OUString::valueOf(fDuration));
        }
    }

    if( pSet->getPropertyState( nHandleRepeat ) != STLPropertyState_AMBIGUOUS )
    {
        Any aRepeatCount( pSet->getPropertyValue( nHandleRepeat ) );
        if( (aRepeatCount.getValueType() == ::getCppuType((const double*)0)) || !aRepeatCount.hasValue() )
        {
            double fRepeat = 0.0;
            if( aRepeatCount.hasValue() )
                aRepeatCount >>= fRepeat;

            sal_uInt16 nPos = LISTBOX_ENTRY_NOTFOUND;

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
                mpCBRepeat->SetText(OUString::valueOf(fRepeat));
        }
        else if( aRepeatCount.getValueType() == ::getCppuType((const Timing*)0) )
        {
            Any aEnd;
            if( pSet->getPropertyState( nHandleEnd ) != STLPropertyState_AMBIGUOUS )
                aEnd = pSet->getPropertyValue( nHandleEnd );

            mpCBRepeat->SelectEntryPos( aEnd.hasValue() ? 6 : 7 );
        }
    }

    if( pSet->getPropertyState( nHandleRewind ) != STLPropertyState_AMBIGUOUS )
    {
        sal_Int16 nFill = 0;
        if( pSet->getPropertyValue( nHandleRewind ) >>= nFill )
        {
            mpCBXRewind->Check( (nFill == AnimationFill::REMOVE) ? sal_True : sal_False );
        }
        else
        {
            mpCBXRewind->SetState( STATE_DONTKNOW );
        }
    }

    Reference< XShape > xTrigger;

    if( pSet->getPropertyState( nHandleTrigger ) != STLPropertyState_AMBIGUOUS )
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
                sal_Bool bIsEmpty = sal_False;
                xSet->getPropertyValue( aStrIsEmptyPresObj ) >>= bIsEmpty;
                if( bIsEmpty )
                    continue;
            }

            String aDescription( getShapeDescription( xShape, true ) );
            sal_uInt16 nPos = mpLBTrigger->InsertEntry( aDescription );

            mpLBTrigger->SetEntryData( nPos, (void*)(sal_IntPtr)nShape );
            if( xShape == xTrigger )
                mpLBTrigger->SelectEntryPos( nPos );
        }
    }
}

CustomAnimationDurationTabPage::~CustomAnimationDurationTabPage()
{
}

IMPL_LINK( CustomAnimationDurationTabPage, implControlHdl, Control*, pControl )
{
    if( pControl == mpLBTrigger )
    {
        mpRBClickSequence->Check( sal_False );
        mpRBInteractive->Check( sal_True );
    }

    return 0;
}

void CustomAnimationDurationTabPage::update( STLPropertySet* pSet )
{
    sal_uInt16 nPos = mpLBStart->GetSelectEntryPos();
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

        if(mpSet->getPropertyState( nHandleStart ) != STLPropertyState_AMBIGUOUS)
            mpSet->getPropertyValue( nHandleStart ) >>= nOldStart;

        if( nStart != nOldStart )
            pSet->setPropertyValue( nHandleStart, makeAny( nStart ) );
    }

    // ---

    {
        double fBegin = static_cast<double>( mpMFStartDelay->GetValue()) / 10.0;
        double fOldBegin = -1.0;

        if( mpSet->getPropertyState( nHandleBegin ) != STLPropertyState_AMBIGUOUS )
            mpSet->getPropertyValue( nHandleBegin ) >>= fOldBegin;

        if( fBegin != fOldBegin )
            pSet->setPropertyValue( nHandleBegin, makeAny( fBegin ) );
    }

    // ---

    nPos = mpCBRepeat->GetSelectEntryPos();
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
            // ATTENTION: FALL THROUGH INTENDED!
        case 7:
            aRepeatCount <<= Timing_INDEFINITE;
            break;
        default:
            {
                OUString aText( mpCBRepeat->GetText() );
                if( !aText.isEmpty() )
                    aRepeatCount <<= aText.toDouble();
            }
        }

        Any aOldRepeatCount( aRepeatCount );
        if( mpSet->getPropertyState( nHandleRepeat ) != STLPropertyState_AMBIGUOUS )
            aOldRepeatCount = mpSet->getPropertyValue( nHandleRepeat );

        if( aRepeatCount != aOldRepeatCount )
            pSet->setPropertyValue( nHandleRepeat, aRepeatCount );

        Any aOldEnd( aEnd );
        if( mpSet->getPropertyState( nHandleEnd ) != STLPropertyState_AMBIGUOUS )
            aOldEnd = mpSet->getPropertyValue( nHandleEnd );

        if( aEnd != aOldEnd )
            pSet->setPropertyValue( nHandleEnd, aEnd );
    }

    // ---

    double fDuration = -1.0;
    nPos = mpCBDuration->GetSelectEntryPos();
    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        fDuration = *static_cast< const double * >( mpCBDuration->GetEntryData(nPos) );
    }
    else
    {
        OUString aText( mpCBDuration->GetText() );
        if( !aText.isEmpty() )
        {
            fDuration = aText.toDouble();
        }
    }

    if( fDuration != -1.0 )
    {
        double fOldDuration = -1;

        if( mpSet->getPropertyState( nHandleDuration ) != STLPropertyState_AMBIGUOUS )
            mpSet->getPropertyValue( nHandleDuration ) >>= fOldDuration;

        if( fDuration != fOldDuration )
            pSet->setPropertyValue( nHandleDuration, makeAny( fDuration ) );
    }

    // ---

    if( mpCBXRewind->GetState() != STATE_DONTKNOW )
    {
        sal_Int16 nFill = mpCBXRewind->IsChecked() ? AnimationFill::REMOVE : AnimationFill::HOLD;

        bool bSet = true;

        if( mpSet->getPropertyState( nHandleRewind ) != STLPropertyState_AMBIGUOUS )
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
        nPos = mpLBTrigger->GetSelectEntryPos();
        if( nPos != LISTBOX_ENTRY_NOTFOUND )
        {
            sal_Int32 nShape = (sal_Int32)(sal_IntPtr)mpLBTrigger->GetEntryData( nPos );

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

        if( mpSet->getPropertyState( nHandleTrigger ) != STLPropertyState_AMBIGUOUS )
            aOldValue = mpSet->getPropertyValue( nHandleTrigger );

        if( aNewValue != aOldValue )
            pSet->setPropertyValue( nHandleTrigger, aNewValue );
    }
}

class CustomAnimationTextAnimTabPage : public TabPage
{
public:
    CustomAnimationTextAnimTabPage( Window* pParent, const STLPropertySet* pSet );

    void update( STLPropertySet* pSet );

    void updateControlStates();
    DECL_LINK(implSelectHdl, void *);

private:
    FixedText*   maFTGroupText;
    ListBox*     maLBGroupText;
    CheckBox*    maCBXGroupAuto;
    MetricField* maMFGroupAuto;
    CheckBox*    maCBXAnimateForm;
    CheckBox*    maCBXReverse;

    const STLPropertySet* mpSet;

    bool mbHasVisibleShapes;
};

CustomAnimationTextAnimTabPage::CustomAnimationTextAnimTabPage(Window* pParent, const STLPropertySet* pSet)
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

    if( pSet->getPropertyState( nHandleTextGrouping ) != STLPropertyState_AMBIGUOUS )
    {
        sal_Int32 nTextGrouping = 0;
        if( pSet->getPropertyValue( nHandleTextGrouping ) >>= nTextGrouping )
            maLBGroupText->SelectEntryPos( (sal_uInt16)(nTextGrouping + 1) );
    }

    if( pSet->getPropertyState( nHandleHasVisibleShape ) != STLPropertyState_AMBIGUOUS )
        pSet->getPropertyValue( nHandleHasVisibleShape ) >>= mbHasVisibleShapes;

    if( pSet->getPropertyState( nHandleTextGroupingAuto ) != STLPropertyState_AMBIGUOUS )
    {
        double fTextGroupingAuto = 0.0;
        if( pSet->getPropertyValue( nHandleTextGroupingAuto ) >>= fTextGroupingAuto )
        {
            maCBXGroupAuto->Check( fTextGroupingAuto >= 0.0 );
            if( fTextGroupingAuto >= 0.0 )
                maMFGroupAuto->SetValue( (long)(fTextGroupingAuto*10) );
        }
    }
    else
    {
        maCBXGroupAuto->SetState( STATE_DONTKNOW );
    }

    maCBXAnimateForm->SetState( STATE_DONTKNOW );
    if( pSet->getPropertyState( nHandleAnimateForm ) != STLPropertyState_AMBIGUOUS )
    {
        sal_Bool bAnimateForm = sal_False;
        if( pSet->getPropertyValue( nHandleAnimateForm ) >>= bAnimateForm )
        {
            maCBXAnimateForm->Check( bAnimateForm );
        }
    }
    else
    {
        maCBXAnimateForm->Enable( sal_False );
    }

    maCBXReverse->SetState( STATE_DONTKNOW );
    if( pSet->getPropertyState( nHandleTextReverse ) != STLPropertyState_AMBIGUOUS )
    {
        sal_Bool bTextReverse = sal_False;
        if( pSet->getPropertyValue( nHandleTextReverse ) >>= bTextReverse )
        {
            maCBXReverse->Check( bTextReverse );
        }
    }

    if( pSet->getPropertyState( nHandleMaxParaDepth ) == STLPropertyState_DIRECT )
    {
        sal_Int32 nMaxParaDepth = 0;
        pSet->getPropertyValue( nHandleMaxParaDepth ) >>= nMaxParaDepth;
        nMaxParaDepth += 1;

        sal_Int32 nPos = 6;
        while( (nPos > 2) && (nPos > nMaxParaDepth) )
        {
            maLBGroupText->RemoveEntry( (sal_uInt16)nPos );
            nPos--;
        }
    }

    updateControlStates();
}

void CustomAnimationTextAnimTabPage::update( STLPropertySet* pSet )
{
    sal_uInt16 nPos = maLBGroupText->GetSelectEntryPos();
    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        sal_Int32 nTextGrouping = nPos - 1;
        sal_Int32 nOldGrouping = -2;

        if(mpSet->getPropertyState( nHandleTextGrouping ) != STLPropertyState_AMBIGUOUS)
            mpSet->getPropertyValue( nHandleTextGrouping ) >>= nOldGrouping;

        if( nTextGrouping != nOldGrouping )
            pSet->setPropertyValue( nHandleTextGrouping, makeAny( nTextGrouping ) );
    }

    if( nPos > 0 )
    {
        sal_Bool bTextReverse = maCBXReverse->IsChecked();
        sal_Bool bOldTextReverse = !bTextReverse;

        if(mpSet->getPropertyState( nHandleTextReverse ) != STLPropertyState_AMBIGUOUS)
            mpSet->getPropertyValue( nHandleTextReverse ) >>= bOldTextReverse;

        if( bTextReverse != bOldTextReverse )
            pSet->setPropertyValue( nHandleTextReverse, makeAny( bTextReverse ) );

        if( nPos > 1 )
        {
            double fTextGroupingAuto = maCBXGroupAuto->IsChecked() ? maMFGroupAuto->GetValue() / 10.0 : -1.0;
            double fOldTextGroupingAuto = -2.0;

            if(mpSet->getPropertyState( nHandleTextGroupingAuto ) != STLPropertyState_AMBIGUOUS)
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
        sal_Bool bAnimateForm = maCBXAnimateForm->IsChecked();
        sal_Bool bOldAnimateForm = !bAnimateForm;

        if(mpSet->getPropertyState( nHandleAnimateForm ) != STLPropertyState_AMBIGUOUS)
            mpSet->getPropertyValue( nHandleAnimateForm ) >>= bOldAnimateForm;

        if( bAnimateForm != bOldAnimateForm )
            pSet->setPropertyValue( nHandleAnimateForm, makeAny( bAnimateForm ) );
    }
}

void CustomAnimationTextAnimTabPage::updateControlStates()
{
    sal_uInt16 nPos = maLBGroupText->GetSelectEntryPos();

    maCBXGroupAuto->Enable( nPos > 1 );
    maMFGroupAuto->Enable( nPos > 1 );
    maCBXReverse->Enable( nPos > 0 );

    if( !mbHasVisibleShapes && nPos > 0 )
    {
        maCBXAnimateForm->Check(sal_False);
        maCBXAnimateForm->Enable(sal_False);
    }
    else
    {
        maCBXAnimateForm->Enable(sal_True);
    }
}

IMPL_LINK_NOARG(CustomAnimationTextAnimTabPage, implSelectHdl)
{
    updateControlStates();
    return 0;
}

// --------------------------------------------------------------------

CustomAnimationDialog::CustomAnimationDialog( Window* pParent, STLPropertySet* pSet, sal_uInt16 nPage /* = 0 */  )
: TabDialog( pParent, "CustomAnimationProperties", "modules/simpress/ui/customanimationproperties.ui")
, mpSet( pSet )
, mpResultSet( 0 )
{
    get(mpTabControl, "tabs");

    sal_uInt16 mnEffectId = mpTabControl->GetPageId("effect");
    sal_uInt16 mnTimingId = mpTabControl->GetPageId("timing");
    sal_uInt16 mnTextAnimId = mpTabControl->GetPageId("textanim");

    mpEffectTabPage = new CustomAnimationEffectTabPage( mpTabControl, mpSet );
    mpTabControl->SetTabPage( mnEffectId, mpEffectTabPage );
    mpDurationTabPage = new CustomAnimationDurationTabPage( mpTabControl, mpSet );
    mpTabControl->SetTabPage( mnTimingId, mpDurationTabPage );

    sal_Bool bHasText = sal_False;
    if( pSet->getPropertyState( nHandleHasText ) != STLPropertyState_AMBIGUOUS )
        pSet->getPropertyValue( nHandleHasText ) >>= bHasText;

    if( bHasText )
    {
        mpTextAnimTabPage = new CustomAnimationTextAnimTabPage( mpTabControl, mpSet );
        mpTabControl->SetTabPage( mnTextAnimId, mpTextAnimTabPage );
    }
    else
    {
        mpTextAnimTabPage = 0;
        mpTabControl->RemovePage( mnTextAnimId );
    }

    if( nPage )
        mpTabControl->SelectTabPage( nPage );
}

CustomAnimationDialog::~CustomAnimationDialog()
{
    delete mpEffectTabPage;
    delete mpDurationTabPage;
    delete mpTextAnimTabPage;

    /*delete mpTabControl;*/

    delete mpSet;
    delete mpResultSet;
}

STLPropertySet* CustomAnimationDialog::getResultSet()
{
    if( mpResultSet )
        delete mpResultSet;

    mpResultSet = createDefaultSet();

    mpEffectTabPage->update( mpResultSet );
    mpDurationTabPage->update( mpResultSet );
    if( mpTextAnimTabPage )
        mpTextAnimTabPage->update( mpResultSet );

    return mpResultSet;
}

STLPropertySet* CustomAnimationDialog::createDefaultSet()
{
    Any aEmpty;

    STLPropertySet* pSet = new STLPropertySet();
    pSet->setPropertyDefaultValue( nHandleMaxParaDepth, makeAny( (sal_Int32)-1 ) );

    pSet->setPropertyDefaultValue( nHandleHasAfterEffect, makeAny( (sal_Bool)sal_False ) );
    pSet->setPropertyDefaultValue( nHandleAfterEffectOnNextEffect, makeAny( (sal_Bool)sal_False ) );
    pSet->setPropertyDefaultValue( nHandleDimColor, aEmpty );
    pSet->setPropertyDefaultValue( nHandleIterateType, makeAny( (sal_Int16)0 ) );
    pSet->setPropertyDefaultValue( nHandleIterateInterval, makeAny( (double)0.0 ) );

    pSet->setPropertyDefaultValue( nHandleStart, makeAny( (sal_Int16)EffectNodeType::ON_CLICK ) );
    pSet->setPropertyDefaultValue( nHandleBegin, makeAny( (double)0.0 ) );
    pSet->setPropertyDefaultValue( nHandleDuration, makeAny( (double)2.0 ) );
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

    pSet->setPropertyDefaultValue( nHandleHasText, makeAny( sal_False ) );
    pSet->setPropertyDefaultValue( nHandleHasVisibleShape, makeAny( sal_False ) );
    pSet->setPropertyDefaultValue( nHandleTextGrouping, makeAny( (sal_Int32)-1 ) );
    pSet->setPropertyDefaultValue( nHandleAnimateForm, makeAny( sal_True ) );
    pSet->setPropertyDefaultValue( nHandleTextGroupingAuto, makeAny( (double)-1.0 ) );
    pSet->setPropertyDefaultValue( nHandleTextReverse, makeAny( sal_False ) );

    pSet->setPropertyDefaultValue( nHandleCurrentPage, aEmpty );

    pSet->setPropertyDefaultValue( nHandleSoundURL, aEmpty );
    pSet->setPropertyDefaultValue( nHandleSoundVolumne, makeAny( (double)1.0) );
    pSet->setPropertyDefaultValue( nHandleSoundEndAfterSlide, makeAny( (sal_Int32)0 ) );

    pSet->setPropertyDefaultValue( nHandleCommand, makeAny( (sal_Int16)0 ) );
    return pSet;
}

PropertyControl::PropertyControl( Window* pParent, const ResId& rResId )
: ListBox( pParent, rResId ), mpSubControl(0)
{
}

PropertyControl::PropertyControl( Window* pParent )
: ListBox( pParent, WB_TABSTOP | WB_BORDER | WB_DROPDOWN ), mpSubControl(0)
{
}

extern "C" SAL_DLLPUBLIC_EXPORT Window* SAL_CALL makePropertyControl( Window *pParent )
{
    return new PropertyControl( pParent );
}

PropertyControl::~PropertyControl()
{
    if( mpSubControl )
        delete mpSubControl;
}

void PropertyControl::setSubControl( PropertySubControl* pSubControl )
{
    if( mpSubControl && mpSubControl != pSubControl )
        delete mpSubControl;

    mpSubControl = pSubControl;

    Control* pControl = pSubControl ? pSubControl->getControl() : 0;

    if( pControl )
    {
        pControl->SetPosSizePixel( GetPosPixel(), GetSizePixel() );
        pControl->SetZOrder( this, WINDOW_ZORDER_BEFOR );
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
    Control* pControl = mpSubControl ? mpSubControl->getControl() : 0;
    if( pControl )
        pControl->SetPosSizePixel( GetPosPixel(), GetSizePixel() );
    ListBox::Resize();
}

// ====================================================================

PropertySubControl::~PropertySubControl()
{
}

PropertySubControl* PropertySubControl::create( sal_Int32 nType, Window* pParent, const Any& rValue, const OUString& rPresetId, const Link& rModifyHdl )
{
    PropertySubControl* pSubControl = NULL;
    switch( nType )
    {
    case nPropertyTypeDirection:
    case nPropertyTypeSpokes:
    case nPropertyTypeZoom:
        pSubControl = new PresetPropertyBox( nType, pParent, rValue, rPresetId, rModifyHdl );
        break;

    case nPropertyTypeColor:
    case nPropertyTypeFillColor:
    case nPropertyTypeFirstColor:
    case nPropertyTypeCharColor:
    case nPropertyTypeLineColor:
        pSubControl = new ColorPropertyBox( nType, pParent, rValue, rModifyHdl );
        break;

    case nPropertyTypeFont:
        pSubControl = new FontPropertyBox( nType, pParent, rValue, rModifyHdl );
        break;

    case nPropertyTypeCharHeight:
        pSubControl = new CharHeightPropertyBox( nType, pParent, rValue, rModifyHdl );
        break;

    case nPropertyTypeRotate:
        pSubControl = new RotationPropertyBox( nType, pParent, rValue, rModifyHdl );
        break;

    case nPropertyTypeTransparency:
        pSubControl = new TransparencyPropertyBox( nType, pParent, rValue, rModifyHdl );
        break;

    case nPropertyTypeScale:
        pSubControl = new ScalePropertyBox( nType, pParent, rValue, rModifyHdl );
        break;

    case nPropertyTypeCharDecoration:
        pSubControl = new FontStylePropertyBox( nType, pParent, rValue, rModifyHdl );
        break;
    }

    return pSubControl;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
