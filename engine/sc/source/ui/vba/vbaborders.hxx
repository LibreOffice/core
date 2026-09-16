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

#include <ooo/vba/excel/XBorders.hpp>

#include <vbahelper/vbacollectionimpl.hxx>

namespace com::sun::star::beans { class XPropertySet; }
namespace cpo::uno { class XComponentContext; }
namespace com::sun::star::table { class XCellRange; }

typedef CollTestImplHelper< ov::excel::XBorders > ScVbaBorders_BASE;
class ScVbaPalette;
class ScVbaBorders : public ScVbaBorders_BASE
{
    // XEnumerationAccess
    virtual cpo::uno::Any getItemByIntIndex( const sal_Int32 nIndex ) override;
    bool bRangeIsSingleCell;
    cpo::uno::Reference< css::beans::XPropertySet > m_xProps;
public:
    ScVbaBorders( const cpo::uno::Reference< ov::XHelperInterface >& xParent,
                  const cpo::uno::Reference< cpo::uno::XComponentContext > & xContext,
                  const cpo::uno::Reference< css::table::XCellRange >& xRange,
                  const ScVbaPalette& rPalette );

    // XEnumerationAccess
    virtual cpo::uno::Type getElementType() override;
    virtual cpo::uno::Reference< css::container::XEnumeration > createEnumeration() override;

    // XBorders

    // ScVbaCollectionBaseImpl
    virtual cpo::uno::Any createCollectionObject( const cpo::uno::Any& aSource ) override;

    virtual cpo::uno::Any getColor() override;
    virtual void setColor( const cpo::uno::Any& _color ) override;
    virtual cpo::uno::Any getColorIndex() override;
    virtual void setColorIndex( const cpo::uno::Any& _colorindex ) override;
    virtual cpo::uno::Any getLineStyle() override;
    virtual void setLineStyle( const cpo::uno::Any& _linestyle ) override;
    virtual cpo::uno::Any getWeight() override;
    virtual void setWeight( const  cpo::uno::Any& ) override;
    virtual cpo::uno::Any getTintAndShade() override;
    virtual void setTintAndShade( const  cpo::uno::Any& ) override;
    // xxxxBASE
    virtual OUString getServiceImplName() override;
    virtual cpo::uno::Sequence<OUString> getServiceNames() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
