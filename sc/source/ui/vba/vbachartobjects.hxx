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

#include <ooo/vba/excel/XChartObjects.hpp>
#include <vbahelper/vbacollectionimpl.hxx>

namespace com::sun::star::container { class XEnumeration; }
namespace com::sun::star::drawing { class XDrawPageSupplier; }
namespace com::sun::star::table { class XTableCharts; }
namespace com::sun::star::uno { class XComponentContext; }

typedef CollTestImplHelper< ov::excel::XChartObjects > ChartObjects_BASE;

class ScVbaChartObjects : public ChartObjects_BASE
{

    css::uno::Reference< css::table::XTableCharts > xTableCharts;
    css::uno::Reference< css::drawing::XDrawPageSupplier > xDrawPageSupplier;
    // method associated with populating the hashmap ( I'm not convinced this is necessary )
    //css::uno::Reference< ov::excel::XChartObject > putByPersistName( const rtl:::OUString& _sPersistChartName );
public:
    ScVbaChartObjects( const css::uno::Reference< ov::XHelperInterface >& _xParent, const css::uno::Reference< css::uno::XComponentContext >& _xContext, const css::uno::Reference< css::table::XTableCharts >& _xTableCharts, const css::uno::Reference< css::drawing::XDrawPageSupplier >&  _xDrawPageSupplier );

    /// @throws css::script::BasicErrorException
    css::uno::Sequence< OUString > getChartObjectNames() const;
    void removeByName(const OUString& _sChartName);

    // XChartObjects
    virtual css::uno::Any SAL_CALL Add( double Left, double Top, double Width, double Height ) override;
    virtual void SAL_CALL Delete(  ) override;
    // XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() override;
    // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() override;
    // ScVbaCollectionBaseImpl
    virtual css::uno::Any createCollectionObject( const css::uno::Any& aSource ) override;
    // ChartObjects_BASE
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
