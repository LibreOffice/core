/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vbaformatconditions.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-12-07 10:52:59 $
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
#ifndef SC_VBA_FORMATCONDITIONS_HXX
#define SC_VBA_FORMATCONDITIONS_HXX
#include <org/openoffice/excel/XFormatConditions.hpp>
#include <org/openoffice/excel/XStyles.hpp>
#include <org/openoffice/excel/XRange.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/sheet/XSheetConditionalEntries.hpp>
#include "vbacollectionimpl.hxx"

typedef CollTestImplHelper< oo::excel::XFormatConditions > ScVbaFormatConditions_BASE;
class ScVbaFormatConditions: public ScVbaFormatConditions_BASE
{
    css::table::CellAddress maCellAddress;
    css::uno::Reference< css::sheet::XSheetConditionalEntries > mxSheetConditionalEntries;
    css::uno::Reference< oo::excel::XStyles > mxStyles;
    css::uno::Reference< oo::excel::XRange > mxRangeParent;
    css::uno::Reference< css::beans::XPropertySet > mxParentRangePropertySet;
public:
    ScVbaFormatConditions( const css::uno::Reference< oo::vba::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext > & xContext, const css::uno::Reference< css::sheet::XSheetConditionalEntries >&, const css::uno::Reference< css::frame::XModel >&  );
    void notifyRange() throw ( css::script::BasicErrorException );
    virtual css::uno::Reference< oo::excel::XFormatCondition > Add( ::sal_Int32 Type, const css::uno::Any& Operator, const css::uno::Any& Formula1, const css::uno::Any& Formula2, const css::uno::Reference< oo::excel::XStyle >& _xCalcStyle ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    rtl::OUString getA1Formula(const css::uno::Any& _aFormula) throw ( css::script::BasicErrorException );
    rtl::OUString getStyleName();
    void removeFormatCondition( const rtl::OUString& _sStyleName, sal_Bool _bRemoveStyle) throw ( css::script::BasicErrorException );
    css::uno::Reference< css::sheet::XSheetConditionalEntries > getSheetConditionalEntries() { return mxSheetConditionalEntries; }
    // XFormatConditions
    virtual void SAL_CALL Delete(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual css::uno::Reference< oo::excel::XFormatCondition > SAL_CALL Add( ::sal_Int32 Type, const css::uno::Any& Operator, const css::uno::Any& Formula1, const css::uno::Any& Formula2 ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    // XEnumerationAccess
    virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw (css::uno::RuntimeException);
    virtual css::uno::Any createCollectionObject(const css::uno::Any&);
    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};

#endif //SC_VBA_AXES_HXX
