/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef _SD_CUSTOMANIMATIONDIALOG_HXX
#define _SD_CUSTOMANIMATIONDIALOG_HXX

#include "CustomAnimationEffect.hxx"
#include "CustomAnimationPreset.hxx"
#include <vcl/tabdlg.hxx>
#include <vcl/lstbox.hxx>

class TabControl;
class OKButton;
class CancelButton;
class HelpButton;
namespace sd {


// --------------------------------------------------------------------

// property handles
const sal_Int32 nHandleSound = 0;
const sal_Int32 nHandleHasAfterEffect = 1;
const sal_Int32 nHandleIterateType = 2;
const sal_Int32 nHandleIterateInterval = 3;
const sal_Int32 nHandleStart = 4;
const sal_Int32 nHandleBegin = 5;
const sal_Int32 nHandleDuration = 6;
const sal_Int32 nHandleRepeat = 7;
const sal_Int32 nHandleRewind = 8;
const sal_Int32 nHandleEnd = 9;
const sal_Int32 nHandleAfterEffectOnNextEffect = 10;
const sal_Int32 nHandleDimColor = 11;
const sal_Int32 nHandleMaxParaDepth = 12;
const sal_Int32 nHandlePresetId = 13;
const sal_Int32 nHandleProperty1Type = 14;
const sal_Int32 nHandleProperty1Value = 15;
const sal_Int32 nHandleProperty2Type = 16;
const sal_Int32 nHandleProperty2Value = 17;

const sal_Int32 nHandleAccelerate = 18;
const sal_Int32 nHandleDecelerate = 19;
const sal_Int32 nHandleAutoReverse = 20;
const sal_Int32 nHandleTrigger = 21;

const sal_Int32 nHandleHasText = 22;
const sal_Int32 nHandleTextGrouping = 23;
const sal_Int32 nHandleAnimateForm = 24;
const sal_Int32 nHandleTextGroupingAuto = 25;
const sal_Int32 nHandleTextReverse = 26;

const sal_Int32 nHandleCurrentPage = 27;
const sal_Int32 nHandleSoundURL = 28;
const sal_Int32 nHandleSoundVolumne = 29;
const sal_Int32 nHandleSoundEndAfterSlide = 30;

const sal_Int32 nHandleCommand = 31;

const sal_Int32 nHandleHasVisibleShape = 32;

const sal_Int32 nPropertyTypeNone = 0;
const sal_Int32 nPropertyTypeDirection = 1;
const sal_Int32 nPropertyTypeSpokes = 2;
const sal_Int32 nPropertyTypeFirstColor = 3;
const sal_Int32 nPropertyTypeSecondColor = 4;
const sal_Int32 nPropertyTypeZoom = 5;
const sal_Int32 nPropertyTypeFillColor = 6;
const sal_Int32 nPropertyTypeColorStyle = 7;
const sal_Int32 nPropertyTypeFont = 8;
const sal_Int32 nPropertyTypeCharHeight = 9;
const sal_Int32 nPropertyTypeCharColor = 10;
const sal_Int32 nPropertyTypeCharHeightStyle = 11;
const sal_Int32 nPropertyTypeCharDecoration = 12;
const sal_Int32 nPropertyTypeLineColor = 13;
const sal_Int32 nPropertyTypeRotate = 14;
const sal_Int32 nPropertyTypeColor = 15;
const sal_Int32 nPropertyTypeAccelerate = 16;
const sal_Int32 nPropertyTypeDecelerate = 17;
const sal_Int32 nPropertyTypeAutoReverse = 18;
const sal_Int32 nPropertyTypeTransparency = 19;
const sal_Int32 nPropertyTypeFontStyle = 20;
const sal_Int32 nPropertyTypeScale = 21;

// --------------------------------------------------------------------

class PropertySubControl
{
public:
    PropertySubControl( sal_Int32 nType ) : mnType( nType ) {}
    virtual ~PropertySubControl();

    virtual             ::com::sun::star::uno::Any getValue() = 0;
    virtual             void setValue( const ::com::sun::star::uno::Any& rValue, const rtl::OUString& rPresetId ) = 0;

    virtual Control*    getControl() = 0;

    static PropertySubControl*
                        create( sal_Int32 nType,
                                ::Window* pParent,
                                const ::com::sun::star::uno::Any& rValue,
                                const rtl::OUString& rPresetId,
                                const Link& rModifyHdl );

    sal_Int32 getControlType() const { return mnType; }

protected:
    sal_Int32           mnType;
};

// --------------------------------------------------------------------

class PropertyControl : public ListBox
{
public:
    PropertyControl( Window* pParent, const ResId& rResId );
    ~PropertyControl();

    void setSubControl( PropertySubControl* pSubControl );
    PropertySubControl* getSubControl() const { return mpSubControl; }

    virtual void Resize();

private:
    PropertySubControl* mpSubControl;
};

// --------------------------------------------------------------------

class CustomAnimationDurationTabPage;
class CustomAnimationEffectTabPage;
class CustomAnimationTextAnimTabPage;
class STLPropertySet;

class CustomAnimationDialog : public TabDialog
{
public:
    CustomAnimationDialog( Window* pParent, STLPropertySet* pSet, sal_uInt16 nPage = 0 );
    ~CustomAnimationDialog();

    STLPropertySet* getDefaultSet() { return mpSet; }
    STLPropertySet* getResultSet();

    static STLPropertySet* createDefaultSet();

private:
    STLPropertySet* mpSet;
    STLPropertySet* mpResultSet;

    CustomAnimationEffectPtr mpEffect;
    TabControl* mpTabControl;
    OKButton* mpOKButton;
    CancelButton* mpCancelButton;
    HelpButton* mpHelpButton;

    CustomAnimationDurationTabPage* mpDurationTabPage;
    CustomAnimationEffectTabPage* mpEffectTabPage;
    CustomAnimationTextAnimTabPage* mpTextAnimTabPage;
};

}

#endif // _SD_CUSTOMANIMATIONDIALOG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
