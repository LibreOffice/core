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
#ifndef INCLUDED_SC_SOURCE_UI_VBA_VBABORDERS_HXX
#define INCLUDED_SC_SOURCE_UI_VBA_VBABORDERS_HXX

#include <ooo/vba/excel/XBorders.hpp>

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <vbahelper/vbacollectionimpl.hxx>

typedef CollTestImplHelper< ov::excel::XBorders > ScVbaBorders_BASE;
class ScVbaPalette;
class ScVbaBorders : public ScVbaBorders_BASE
{
    // XEnumerationAccess
    virtual css::uno::Any getItemByIntIndex( const sal_Int32 nIndex ) throw (css::uno::RuntimeException) override;
    bool bRangeIsSingleCell;
    css::uno::Reference< css::beans::XPropertySet > m_xProps;
public:
    ScVbaBorders( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext > & xContext, const css::uno::Reference< css::table::XCellRange >& xRange, ScVbaPalette& rPalette );
    virtual ~ScVbaBorders() {}

    // XEnumerationAccess
    virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException) override;
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw (css::uno::RuntimeException) override;

    // XBorders

    // ScVbaCollectionBaseImpl
    virtual css::uno::Any createCollectionObject( const css::uno::Any& aSource ) override;

    virtual css::uno::Any SAL_CALL getColor() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setColor( const css::uno::Any& _color ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getColorIndex() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setColorIndex( const css::uno::Any& _colorindex ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getLineStyle() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setLineStyle( const css::uno::Any& _linestyle ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getWeight() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setWeight( const  css::uno::Any& ) throw (css::uno::RuntimeException, std::exception) override;
    // xxxxBASE
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;
};

#endif // INCLUDED_SC_SOURCE_UI_VBA_VBABORDERS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
