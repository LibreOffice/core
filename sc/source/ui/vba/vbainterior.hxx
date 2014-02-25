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
#ifndef SC_VBA_INTERIOR_HXX
#define SC_VBA_INTERIOR_HXX

#include <ooo/vba/excel/XInterior.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>

#include <com/sun/star/script/XInvocation.hpp>
#include <vbahelper/vbahelperinterface.hxx>

#include <tools/color.hxx>

class ScDocument;

typedef InheritedHelperInterfaceImpl1< ov::excel::XInterior > ScVbaInterior_BASE;

class ScVbaInterior :  public ScVbaInterior_BASE
{
    css::uno::Reference< css::beans::XPropertySet > m_xProps;
    ScDocument* m_pScDoc;
    Color m_aPattColor;
    sal_Int32 m_nPattern;

        css::uno::Reference< css::container::XIndexAccess > getPalette();
    css::uno::Reference< css::container::XNameContainer > GetAttributeContainer();
    css::uno::Any SetAttributeData( sal_Int32 nValue );
    sal_Int32 GetAttributeData( css::uno::Any aValue );
    Color GetBackColor();
protected:
    Color GetPatternColor( const Color& rPattColor, const Color& rBackColor, sal_uInt32 nXclPattern );
    Color GetMixedColor( const Color& rFore, const Color& rBack, sal_uInt8 nTrans );
    sal_uInt8 GetMixedColorComp( sal_uInt8 nFore, sal_uInt8 nBack, sal_uInt8 nTrans ) const;
    css::uno::Any GetIndexColor( const sal_Int32& nColorIndex );
    sal_Int32 GetColorIndex( const sal_Int32 nColor );
    css::uno::Any GetUserDefinedAttributes( const OUString& sName );
    void SetUserDefinedAttributes( const OUString& sName, const css::uno::Any& aValue );
    void SetMixedColor();
public:
        ScVbaInterior( const css::uno::Reference< ov::XHelperInterface >& xParent,  const css::uno::Reference< css::uno::XComponentContext >& xContext,
                 const css::uno::Reference< css::beans::XPropertySet >& xProps, ScDocument* pScDoc = NULL) throw ( css::lang::IllegalArgumentException);

        virtual ~ScVbaInterior(){}

    virtual css::uno::Any SAL_CALL getColor() throw (css::uno::RuntimeException, std::exception) ;
    virtual void SAL_CALL setColor( const css::uno::Any& _color ) throw (css::uno::RuntimeException, std::exception) ;

    virtual css::uno::Any SAL_CALL getColorIndex() throw ( css::uno::RuntimeException, std::exception);
    virtual void SAL_CALL setColorIndex( const css::uno::Any& _colorindex ) throw ( css::uno::RuntimeException, std::exception );
    virtual css::uno::Any SAL_CALL getPattern() throw (css::uno::RuntimeException, std::exception);
    virtual void SAL_CALL setPattern( const css::uno::Any& _pattern ) throw (css::uno::RuntimeException, std::exception);
    virtual css::uno::Any SAL_CALL getPatternColor() throw (css::uno::RuntimeException, std::exception);
    virtual void SAL_CALL setPatternColor( const css::uno::Any& _patterncolor ) throw (css::uno::RuntimeException, std::exception);
    virtual css::uno::Any SAL_CALL getPatternColorIndex() throw (css::uno::RuntimeException, std::exception);
    virtual void SAL_CALL setPatternColorIndex( const css::uno::Any& _patterncolorindex ) throw (css::uno::RuntimeException, std::exception);
    //XHelperInterface
    virtual OUString getServiceImplName();
    virtual css::uno::Sequence<OUString> getServiceNames();
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
