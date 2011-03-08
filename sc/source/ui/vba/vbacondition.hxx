/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef SC_VBA_CONDITION_HXX
#define SC_VBA_CONDITION_HXX
#include <com/sun/star/sheet/XSheetCondition.hpp>
#include <com/sun/star/sheet/XCellRangeAddressable.hpp>
#include <vbahelper/vbahelperinterface.hxx>
#include <com/sun/star/sheet/ConditionOperator.hpp>

template< typename Ifc1 >
class ScVbaCondition : public InheritedHelperInterfaceImpl1< Ifc1 >
{
typedef InheritedHelperInterfaceImpl1< Ifc1 > ScVbaCondition_BASE;
protected:
    css::uno::Reference< css::sheet::XCellRangeAddressable > mxAddressable;
    css::uno::Reference< css::sheet::XSheetCondition > mxSheetCondition;
public:
    ScVbaCondition(  const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext > & xContext, const css::uno::Reference< css::sheet::XSheetCondition >& _xSheetCondition );

    static css::sheet::ConditionOperator retrieveAPIOperator( const css::uno::Any& _aOperator) throw ( css::script::BasicErrorException );

    virtual rtl::OUString SAL_CALL Formula1( ) throw ( css::script::BasicErrorException, css::uno::RuntimeException );
    virtual rtl::OUString SAL_CALL Formula2( ) throw ( css::script::BasicErrorException, css::uno::RuntimeException );
    virtual void setFormula1( const css::uno::Any& _aFormula1) throw ( css::script::BasicErrorException );
    virtual void setFormula2( const css::uno::Any& _aFormula2) throw ( css::script::BasicErrorException );
    virtual sal_Int32 Operator(sal_Bool _bIncludeFormulaValue) throw ( css::script::BasicErrorException );
    virtual sal_Int32 SAL_CALL Operator() throw ( css::script::BasicErrorException, css::uno::RuntimeException ) = 0;

};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
