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
#ifndef SC_VBA_XCOLORFORMAT_HXX
#define SC_VBA_XCOLORFORMAT_HXX

#include <com/sun/star/drawing/XShape.hpp>
#include <ooo/vba/msforms/XColorFormat.hpp>
#include <ooo/vba/msforms/XFillFormat.hpp>
#include <vbahelper/vbahelperinterface.hxx>
#include "vbafillformat.hxx"

typedef InheritedHelperInterfaceImpl1< ov::msforms::XColorFormat > ScVbaColorFormat_BASE;

class ColorFormatType
{
public:
    const static sal_Int16 LINEFORMAT_FORECOLOR = 1;
    const static sal_Int16 LINEFORMAT_BACKCOLOR = 2;
    const static sal_Int16 FILLFORMAT_FORECOLOR = 3;
    const static sal_Int16 FILLFORMAT_BACKCOLOR = 4;
    const static sal_Int16 THREEDFORMAT_EXTRUSIONCOLOR = 5;
};

struct MsoColorIndizes
{
    const static sal_Int32 HAPICOLOR_BLACK = 0;
    const static sal_Int32 HAPICOLOR_WITHE = 16777215;
    const static sal_Int32 HAPICOLOR_RED = 16711680;
    const static sal_Int32 HAPICOLOR_BRIGHTGREEN = 65280;
    const static sal_Int32 HAPICOLOR_BLUE = 255;
    const static sal_Int32 HAPICOLOR_YELLOW = 16776960;
    const static sal_Int32 HAPICOLOR_PINK = 16711935;
    const static sal_Int32 HAPICOLOR_TURQUOISE = 65535;
    const static sal_Int32 HAPICOLOR_DARKRED = 8388608;
    const static sal_Int32 HAPICOLOR_GREEN = 32768;
    const static sal_Int32 HAPICOLOR_DARKBLUE = 128;
    const static sal_Int32 HAPICOLOR_DARKYELLOW = 8421376;
    const static sal_Int32 HAPICOLOR_VIOLET = 8388736;
    const static sal_Int32 HAPICOLOR_TEAL = 32896;
    const static sal_Int32 HAPICOLOR_GRAY_25_PERCENT = 12632256;
    const static sal_Int32 HAPICOLOR_GRAY_50_PERCENT = 8421504;
    const static sal_Int32 HAPICOLOR_PERIWINCKLE = 10066431;
    const static sal_Int32 HAPICOLOR_PLUM = 10040166;
    const static sal_Int32 HAPICOLOR_IVORY = 16777164;
    const static sal_Int32 HAPICOLOR_LIGHTTURQUOISE = 13434879;
    const static sal_Int32 HAPICOLOR_DARKPRUPLE = 6684774;
    const static sal_Int32 HAPICOLOR_CORAL = 16744576;
    const static sal_Int32 HAPICOLOR_OCEANBLUE = 26316;
    const static sal_Int32 HAPICOLOR_ICEBLUE = 13421823;
    const static sal_Int32 HAPICOLOR_SKYBLUE = 52479;
    const static sal_Int32 HAPICOLOR_LIGHTGREEN = 13434828;
    const static sal_Int32 HAPICOLOR_LIGHTYELLOW = 16777113;
    const static sal_Int32 HAPICOLOR_PALEBLUE = 10079487;
    const static sal_Int32 HAPICOLOR_ROSE = 16751052;
    const static sal_Int32 HAPICOLOR_LAVENDER = 13408767;
    const static sal_Int32 HAPICOLOR_TAN = 16764057;
    const static sal_Int32 HAPICOLOR_LIGHTBLUE = 3368703;
    const static sal_Int32 HAPICOLOR_AQUA = 3394764;
    const static sal_Int32 HAPICOLOR_LIME = 10079232;
    const static sal_Int32 HAPICOLOR_GOLD = 16763904;
    const static sal_Int32 HAPICOLOR_LIGHTORANGE = 16750848;
    const static sal_Int32 HAPICOLOR_ORANGE = 16737792;
    const static sal_Int32 HAPICOLOR_BLUEGRAY = 6710937;
    const static sal_Int32 HAPICOLOR_GRAY_40_PERCENT = 9868950;
    const static sal_Int32 HAPICOLOR_DARKTEAL = 13158;
    const static sal_Int32 HAPICOLOR_SEAGREEN = 3381606;
    const static sal_Int32 HAPICOLOR_NONAME = 13056;
    const static sal_Int32 HAPICOLOR_OLIVEGREEN = 3355392;
    const static sal_Int32 HAPICOLOR_BROWN = 10040064;
    const static sal_Int32 HAPICOLOR_INDIGO = 3355545;
    const static sal_Int32 HAPICOLOR_GRAY_80_PERCENT = 3355443;

    static sal_Int32 getColorIndex( sal_Int32 nIndex );

};

class ScVbaColorFormat : public ScVbaColorFormat_BASE
{
private:
    css::uno::Reference< ov::XHelperInterface > m_xInternalParent;
    css::uno::Reference< css::drawing::XShape > m_xShape;
    css::uno::Reference< css::beans::XPropertySet > m_xPropertySet;
    ScVbaFillFormat *m_pFillFormat;
    sal_Int16 m_nColorFormatType;
    sal_Int32 m_nFillFormatBackColor;
protected:
    virtual OUString getServiceImplName();
    virtual css::uno::Sequence<OUString> getServiceNames();
public:
    ScVbaColorFormat( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< ov::XHelperInterface > xInternalParent, const css::uno::Reference< css::drawing::XShape > xShape, const sal_Int16 nColorFormatType );

    // Attributes
    virtual sal_Int32 SAL_CALL getRGB() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setRGB( sal_Int32 _rgb ) throw (css::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getSchemeColor() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setSchemeColor( sal_Int32 _schemecolor ) throw (css::uno::RuntimeException);

};

#endif//SC_VBA_XCOLORFORMAT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
