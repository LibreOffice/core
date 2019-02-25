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

#ifndef INCLUDED_SD_SOURCE_UI_ANIMATIONS_CUSTOMANIMATIONDIALOG_HXX
#define INCLUDED_SD_SOURCE_UI_ANIMATIONS_CUSTOMANIMATIONDIALOG_HXX

#include <vcl/svapp.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/weld.hxx>

namespace sd {

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

class PropertySubControl
{
public:
    explicit PropertySubControl( sal_Int32 nType ) : mnType( nType ) {}
    virtual ~PropertySubControl();

    virtual             css::uno::Any getValue() = 0;
    virtual             void setValue( const css::uno::Any& rValue, const OUString& rPresetId ) = 0;

    virtual Control*    getControl() = 0;

    static std::unique_ptr<PropertySubControl>
                        create( sal_Int32 nType,
                                vcl::Window* pParent,
                                const css::uno::Any& rValue,
                                const OUString& rPresetId,
                                const Link<LinkParamNone*,void>& rModifyHdl );

    sal_Int32 getControlType() const { return mnType; }

private:
    sal_Int32 const  mnType;
};

class SdPropertySubControl
{
public:
    explicit SdPropertySubControl(weld::Container* pParent)
        : mxBuilder(Application::CreateBuilder(pParent, "modules/simpress/ui/customanimationfragment.ui"))
        , mxContainer(mxBuilder->weld_container("EffectFragment"))
    {
    }

    virtual ~SdPropertySubControl();

    virtual             css::uno::Any getValue() = 0;
    virtual             void setValue( const css::uno::Any& rValue, const OUString& rPresetId ) = 0;

    static std::unique_ptr<SdPropertySubControl>
                        create( sal_Int32 nType,
                                weld::Label* pLabel,
                                weld::Container* pParent,
                                weld::Window* pTopLevel,
                                const css::uno::Any& rValue,
                                const OUString& rPresetId,
                                const Link<LinkParamNone*,void>& rModifyHdl );

protected:
    std::unique_ptr<weld::Builder> mxBuilder;
    std::unique_ptr<weld::Container> mxContainer;
};

class PropertyControl : public ListBox
{
public:
    explicit PropertyControl( vcl::Window* pParent );
    virtual ~PropertyControl() override;
    virtual void dispose() override;

    void setSubControl( std::unique_ptr<PropertySubControl> pSubControl );
    PropertySubControl* getSubControl() const { return mpSubControl.get(); }

    virtual void Resize() override;

private:
    std::unique_ptr<PropertySubControl> mpSubControl;
};

class CustomAnimationDurationTabPage;
class CustomAnimationEffectTabPage;
class CustomAnimationTextAnimTabPage;
class STLPropertySet;

class CustomAnimationDialog : public weld::GenericDialogController
{
public:
    CustomAnimationDialog(weld::Window* pParent, std::unique_ptr<STLPropertySet> pSet, const OString& Page);
    virtual ~CustomAnimationDialog() override;

    STLPropertySet* getResultSet();
    STLPropertySet* getPropertySet() const { return mxSet.get(); }

    static std::unique_ptr<STLPropertySet> createDefaultSet();

private:
    std::unique_ptr<STLPropertySet> mxSet;
    std::unique_ptr<STLPropertySet> mxResultSet;

    std::unique_ptr<weld::Notebook> mxTabControl;
    std::unique_ptr<CustomAnimationDurationTabPage> mxDurationTabPage;
    std::unique_ptr<CustomAnimationEffectTabPage> mxEffectTabPage;
    std::unique_ptr<CustomAnimationTextAnimTabPage> mxTextAnimTabPage;
};

}

#endif // INCLUDED_SD_SOURCE_UI_ANIMATIONS_CUSTOMANIMATIONDIALOG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
