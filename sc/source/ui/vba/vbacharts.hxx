/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vbacharts.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-12-07 10:47:09 $
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
#ifndef SC_VBA_CHARTS_HXX
#define SC_VBA_CHARTS_HXX
#include <org/openoffice/excel/XCharts.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include"vbacollectionimpl.hxx"
#include "vbahelper.hxx"
#include <hash_map>

typedef CollTestImplHelper< oo::excel::XCharts > Charts_BASE;

class ScVbaCharts : public Charts_BASE
{
    css::uno::Reference< oo::excel::XChart > xActiveChart;
    css::uno::Reference< css::sheet::XSpreadsheetDocument > xSpreadsheetDocument;
    css::uno::Reference< css::lang::XComponent > xComponent;
public:
    ScVbaCharts( const css::uno::Reference< oo::vba::XHelperInterface >& _xParent, const css::uno::Reference< css::uno::XComponentContext >& _xContext );
    // XCharts
    virtual css::uno::Any SAL_CALL Add() throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual css::uno::Reference< oo::excel::XChart > SAL_CALL getActiveChart(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    // XCollection
    ::sal_Int32 SAL_CALL getCount() throw (css::uno::RuntimeException);
    // XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw (css::uno::RuntimeException);
    // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException) { return oo::excel::XChart::static_type(0); }
    // ScVbaCollectionBaseImpl
    virtual css::uno::Any createCollectionObject( const css::uno::Any& aSource );
    // Charts_BASE or HelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};

#endif
