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
#pragma once

#include <ooo/vba/excel/XBorders.hpp>

#include <vbahelper/vbacollectionimpl.hxx>

namespace com::sun::star::beans { class XPropertySet; }
namespace com::sun::star::uno { class XComponentContext; }
namespace com::sun::star::table { class XCellRange; }

typedef CollTestImplHelper< ov::excel::XBorders > ScVbaBorders_BASE;
class ScVbaPalette;
class ScVbaBorders : public ScVbaBorders_BASE
{
    // XEnumerationAccess
    virtual css::uno::Any getItemByIntIndex( const sal_Int32 nIndex ) override;
    bool bRangeIsSingleCell;
    css::uno::Reference< css::beans::XPropertySet > m_xProps;
public:
    ScVbaBorders( const css::uno::Reference< ov::XHelperInterface >& xParent,
                  const css::uno::Reference< css::uno::XComponentContext > & xContext,
                  const css::uno::Reference< css::table::XCellRange >& xRange,
                  const ScVbaPalette& rPalette );

    // XEnumerationAccess
    virtual css::uno::Type SAL_CALL getElementType() override;
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() override;

    // XBorders

    // ScVbaCollectionBaseImpl
    virtual css::uno::Any createCollectionObject( const css::uno::Any& aSource ) override;

    virtual css::uno::Any SAL_CALL getColor() override;
    virtual void SAL_CALL setColor( const css::uno::Any& _color ) override;
    virtual css::uno::Any SAL_CALL getColorIndex() override;
    virtual void SAL_CALL setColorIndex( const css::uno::Any& _colorindex ) override;
    virtual css::uno::Any SAL_CALL getLineStyle() override;
    virtual void SAL_CALL setLineStyle( const css::uno::Any& _linestyle ) override;
    virtual css::uno::Any SAL_CALL getWeight() override;
    virtual void SAL_CALL setWeight( const  css::uno::Any& ) override;
    virtual css::uno::Any SAL_CALL getTintAndShade() override;
    virtual void SAL_CALL setTintAndShade( const  css::uno::Any& ) override;
    // xxxxBASE
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
