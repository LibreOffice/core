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
#ifndef INCLUDED_SC_SOURCE_UI_VBA_VBACONDITION_HXX
#define INCLUDED_SC_SOURCE_UI_VBA_VBACONDITION_HXX
#include <com/sun/star/sheet/XSheetCondition.hpp>
#include <com/sun/star/sheet/XCellRangeAddressable.hpp>
#include <vbahelper/vbahelperinterface.hxx>
#include <com/sun/star/sheet/ConditionOperator.hpp>

template< typename... Ifc >
class ScVbaCondition : public InheritedHelperInterfaceWeakImpl< Ifc... >
{
typedef InheritedHelperInterfaceWeakImpl< Ifc... > ScVbaCondition_BASE;
protected:
    css::uno::Reference< css::sheet::XCellRangeAddressable > mxAddressable;
    css::uno::Reference< css::sheet::XSheetCondition > mxSheetCondition;
public:
    ScVbaCondition(  const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext > & xContext, const css::uno::Reference< css::sheet::XSheetCondition >& _xSheetCondition );

    static css::sheet::ConditionOperator retrieveAPIOperator( const css::uno::Any& _aOperator) throw ( css::script::BasicErrorException );

    virtual OUString SAL_CALL Formula1( ) throw ( css::script::BasicErrorException, css::uno::RuntimeException );
    virtual OUString SAL_CALL Formula2( ) throw ( css::script::BasicErrorException, css::uno::RuntimeException );
    virtual void setFormula1( const css::uno::Any& _aFormula1) throw ( css::script::BasicErrorException );
    virtual void setFormula2( const css::uno::Any& _aFormula2) throw ( css::script::BasicErrorException );
    virtual sal_Int32 Operator(bool _bIncludeFormulaValue) throw ( css::script::BasicErrorException );
    virtual sal_Int32 SAL_CALL Operator() throw ( css::script::BasicErrorException, css::uno::RuntimeException ) = 0;

};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
