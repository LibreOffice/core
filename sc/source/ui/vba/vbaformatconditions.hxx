/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef SC_VBA_FORMATCONDITIONS_HXX
#define SC_VBA_FORMATCONDITIONS_HXX
#include <ooo/vba/excel/XFormatConditions.hpp>
#include <ooo/vba/excel/XStyles.hpp>
#include <ooo/vba/excel/XRange.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/sheet/XSheetConditionalEntries.hpp>
#include <vbahelper/vbacollectionimpl.hxx>

typedef CollTestImplHelper< ov::excel::XFormatConditions > ScVbaFormatConditions_BASE;
class ScVbaFormatConditions: public ScVbaFormatConditions_BASE
{
    css::table::CellAddress maCellAddress;
    css::uno::Reference< css::sheet::XSheetConditionalEntries > mxSheetConditionalEntries;
    css::uno::Reference< ov::excel::XStyles > mxStyles;
    css::uno::Reference< ov::excel::XRange > mxRangeParent;
    css::uno::Reference< css::beans::XPropertySet > mxParentRangePropertySet;
public:
    ScVbaFormatConditions( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext > & xContext, const css::uno::Reference< css::sheet::XSheetConditionalEntries >&, const css::uno::Reference< css::frame::XModel >&  );
    void notifyRange() throw ( css::script::BasicErrorException );
    virtual css::uno::Reference< ov::excel::XFormatCondition > Add( ::sal_Int32 Type, const css::uno::Any& Operator, const css::uno::Any& Formula1, const css::uno::Any& Formula2, const css::uno::Reference< ov::excel::XStyle >& _xCalcStyle ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    rtl::OUString getA1Formula(const css::uno::Any& _aFormula) throw ( css::script::BasicErrorException );
    rtl::OUString getStyleName();
    void removeFormatCondition( const rtl::OUString& _sStyleName, sal_Bool _bRemoveStyle) throw ( css::script::BasicErrorException );
    css::uno::Reference< css::sheet::XSheetConditionalEntries > getSheetConditionalEntries() { return mxSheetConditionalEntries; }
    // XFormatConditions
    virtual void SAL_CALL Delete(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual css::uno::Reference< ov::excel::XFormatCondition > SAL_CALL Add( ::sal_Int32 Type, const css::uno::Any& Operator, const css::uno::Any& Formula1, const css::uno::Any& Formula2 ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    // XEnumerationAccess
    virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw (css::uno::RuntimeException);
    virtual css::uno::Any createCollectionObject(const css::uno::Any&);
    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};

#endif //SC_VBA_AXES_HXX
