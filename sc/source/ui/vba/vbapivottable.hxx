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
#ifndef INCLUDED_SC_SOURCE_UI_VBA_VBAPIVOTTABLE_HXX
#define INCLUDED_SC_SOURCE_UI_VBA_VBAPIVOTTABLE_HXX
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/sheet/XDataPilotTable.hpp>
#include <ooo/vba/excel/XPivotTable.hpp>
#include "excelvbahelper.hxx"
#include <vbahelper/vbahelperinterface.hxx>

typedef InheritedHelperInterfaceWeakImpl< ov::excel::XPivotTable >  PivotTableImpl_BASE;

class ScVbaPivotTable : public PivotTableImpl_BASE
{
    css::uno::Reference< css::sheet::XDataPilotTable > m_xTable;
public:
    ScVbaPivotTable( const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::sheet::XDataPilotTable >& xTable );
    virtual css::uno::Reference< ov::excel::XPivotCache > SAL_CALL PivotCache(  ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    // XHelperInterface
    virtual OUString getServiceImplName() SAL_OVERRIDE;
    virtual css::uno::Sequence<OUString> getServiceNames() SAL_OVERRIDE;
};

#endif // INCLUDED_SC_SOURCE_UI_VBA_VBAPIVOTTABLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
