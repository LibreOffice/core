/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vbacondition.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-12-07 10:49:41 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef SC_VBA_CONDITION_HXX
#define SC_VBA_CONDITION_HXX
#include <com/sun/star/sheet/XSheetCondition.hpp>
#include <com/sun/star/sheet/XCellRangeAddressable.hpp>
#include "vbahelperinterface.hxx"
#include <com/sun/star/sheet/ConditionOperator.hpp>

template< typename Ifc1 >
class ScVbaCondition : public InheritedHelperInterfaceImpl1< Ifc1 >
{
typedef InheritedHelperInterfaceImpl1< Ifc1 > ScVbaCondition_BASE;
protected:
    css::uno::Reference< css::sheet::XCellRangeAddressable > mxAddressable;
    css::uno::Reference< css::sheet::XSheetCondition > mxSheetCondition;
public:
    ScVbaCondition(  const css::uno::Reference< oo::vba::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext > & xContext, const css::uno::Reference< css::sheet::XSheetCondition >& _xSheetCondition );

    static css::sheet::ConditionOperator retrieveAPIOperator( const css::uno::Any& _aOperator) throw ( css::script::BasicErrorException );

    virtual rtl::OUString SAL_CALL Formula1( ) throw ( css::script::BasicErrorException, css::uno::RuntimeException );
    virtual rtl::OUString SAL_CALL Formula2( ) throw ( css::script::BasicErrorException, css::uno::RuntimeException );
    virtual void setFormula1( const css::uno::Any& _aFormula1) throw ( css::script::BasicErrorException );
    virtual void setFormula2( const css::uno::Any& _aFormula2) throw ( css::script::BasicErrorException );
    virtual sal_Int32 Operator(sal_Bool _bIncludeFormulaValue) throw ( css::script::BasicErrorException );
    virtual sal_Int32 SAL_CALL Operator() throw ( css::script::BasicErrorException, css::uno::RuntimeException ) = 0;

};
#endif
