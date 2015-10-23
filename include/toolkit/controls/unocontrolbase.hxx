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

#ifndef INCLUDED_TOOLKIT_CONTROLS_UNOCONTROLBASE_HXX
#define INCLUDED_TOOLKIT_CONTROLS_UNOCONTROLBASE_HXX

#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/util/Time.hpp>

#include <toolkit/controls/unocontrol.hxx>


//  class UnoControlBase


class TOOLKIT_DLLPUBLIC UnoControlBase : public UnoControl
{
protected:
    UnoControlBase() :UnoControl() {}

    bool                    ImplHasProperty( sal_uInt16 nProp );
    bool                    ImplHasProperty( const OUString& aPropertyName );
    void                        ImplSetPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue, bool bUpdateThis );
    void                        ImplSetPropertyValues( const css::uno::Sequence< OUString >& aPropertyNames, const css::uno::Sequence< css::uno::Any >& aValues, bool bUpdateThis );
    css::uno::Any  ImplGetPropertyValue( const OUString& aPropertyName );

    template <typename T> T ImplGetPropertyValuePOD( sal_uInt16 nProp );
    template <typename T> T ImplGetPropertyValueClass( sal_uInt16 nProp );
    bool        ImplGetPropertyValue_BOOL( sal_uInt16 nProp );
    sal_Int16       ImplGetPropertyValue_INT16( sal_uInt16 nProp );
    sal_Int32       ImplGetPropertyValue_INT32( sal_uInt16 nProp );
    double          ImplGetPropertyValue_DOUBLE( sal_uInt16 nProp );
    OUString        ImplGetPropertyValue_UString( sal_uInt16 nProp );
    css::util::Date ImplGetPropertyValue_Date( sal_uInt16 nProp );
    css::util::Time ImplGetPropertyValue_Time( sal_uInt16 nProp );

    // XLayoutConstrains (nur wenn das Control es unterstuetzt!)
    css::awt::Size Impl_getMinimumSize();
    css::awt::Size Impl_getPreferredSize();
    css::awt::Size Impl_calcAdjustedSize( const css::awt::Size& rNewSize );

    // XTextLayoutConstrains (nur wenn das Control es unterstuetzt!)
    css::awt::Size Impl_getMinimumSize( sal_Int16 nCols, sal_Int16 nLines );
    void                        Impl_getColumnsAndLines( sal_Int16& nCols, sal_Int16& nLines );
};



#endif // INCLUDED_TOOLKIT_CONTROLS_UNOCONTROLBASE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
