/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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
#pragma once

#include <ooo/vba/excel/XInterior.hpp>
#include <cpo/uno/XComponentContext.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>

#include <vbahelper/vbahelperinterface.hxx>

#include <tools/color.hxx>

class ScDocument;

typedef InheritedHelperInterfaceWeakImpl< ov::excel::XInterior > ScVbaInterior_BASE;

class ScVbaInterior final :  public ScVbaInterior_BASE
{
    cpo::uno::Reference< css::beans::XPropertySet > m_xProps;
    ScDocument* m_pScDoc;
    Color m_aPattColor;
    sal_Int32 m_nPattern;

    cpo::uno::Reference< css::container::XIndexAccess > getPalette() const;
    cpo::uno::Reference< css::container::XNameContainer > GetAttributeContainer();
    static cpo::uno::Any SetAttributeData( sal_Int32 nValue );
    static sal_Int32 GetAttributeData( cpo::uno::Any const & aValue );
    Color GetBackColor();
    static Color GetPatternColor( const Color& rPattColor, const Color& rBackColor, sal_uInt32 nXclPattern );
    static Color GetMixedColor( const Color& rFore, const Color& rBack, sal_uInt8 nTrans );
    static sal_uInt8 GetMixedColorComp( sal_uInt8 nFore, sal_uInt8 nBack, sal_uInt8 nTrans );
    cpo::uno::Any GetIndexColor( sal_Int32 nColorIndex );
    sal_Int32 GetColorIndex( const sal_Int32 nColor );
    cpo::uno::Any GetUserDefinedAttributes( const OUString& sName );
    void SetUserDefinedAttributes( const OUString& sName, const cpo::uno::Any& aValue );
    void SetMixedColor();

public:
        /// @throws css::lang::IllegalArgumentException
        ScVbaInterior( const cpo::uno::Reference< ov::XHelperInterface >& xParent,  const cpo::uno::Reference< cpo::uno::XComponentContext >& xContext,
                 cpo::uno::Reference< css::beans::XPropertySet > xProps, ScDocument* pScDoc = nullptr);

    virtual cpo::uno::Any getColor() override ;
    virtual void setColor( const cpo::uno::Any& _color ) override ;

    virtual cpo::uno::Any getColorIndex() override;
    virtual void setColorIndex( const cpo::uno::Any& _colorindex ) override;
    virtual cpo::uno::Any getPattern() override;
    virtual void setPattern( const cpo::uno::Any& _pattern ) override;
    virtual cpo::uno::Any getPatternColor() override;
    virtual void setPatternColor( const cpo::uno::Any& _patterncolor ) override;
    virtual cpo::uno::Any getPatternColorIndex() override;
    virtual void setPatternColorIndex( const cpo::uno::Any& _patterncolorindex ) override;
    cpo::uno::Any getThemeColor() override;
    void setThemeColor(const cpo::uno::Any& rAny) override;
    cpo::uno::Any getTintAndShade() override;
    void setTintAndShade(const cpo::uno::Any& rAny) override;
    cpo::uno::Any getPatternTintAndShade() override;
    void setPatternTintAndShade(const cpo::uno::Any& rAny) override;
    //XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual cpo::uno::Sequence<OUString> getServiceNames() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
