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
#ifndef INCLUDED_SC_SOURCE_UI_VBA_VBAFORMATCONDITION_HXX
#define INCLUDED_SC_SOURCE_UI_VBA_VBAFORMATCONDITION_HXX
#include <ooo/vba/excel/XFormatCondition.hpp>
#include <ooo/vba/excel/XFormatConditions.hpp>
#include <ooo/vba/excel/XStyle.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/sheet/XSheetConditionalEntries.hpp>
#include <com/sun/star/sheet/XSheetConditionalEntry.hpp>
#include <com/sun/star/sheet/XSheetCondition.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include "vbacondition.hxx"

typedef ScVbaCondition< ov::excel::XFormatCondition >  ScVbaFormatCondition_BASE;
class ScVbaFormatCondition : public ScVbaFormatCondition_BASE
{
protected:
    OUString msStyleName;
    css::uno::Reference< css::sheet::XSheetConditionalEntry > mxSheetConditionalEntry;
    css::uno::Reference< css::sheet::XSheetConditionalEntries > mxSheetConditionalEntries;
    css::uno::Reference< ov::excel::XFormatConditions> moFormatConditions;
    css::uno::Reference< ov::excel::XStyle > mxStyle;
    css::uno::Reference< css::beans::XPropertySet > mxParentRangePropertySet;
public:
    ScVbaFormatCondition( const css::uno::Reference< ov::XHelperInterface >& xParent,
                          const css::uno::Reference< css::uno::XComponentContext > & xContext,
                          const css::uno::Reference< css::sheet::XSheetConditionalEntry >& _xSheetConditionalEntry,
                          const css::uno::Reference< ov::excel::XStyle >&,
                          const css::uno::Reference< ov::excel::XFormatConditions >& _xFormatConditions,
                          const css::uno::Reference< css::beans::XPropertySet >& _xPropertySet ) throw ( css::uno::RuntimeException, css::script::BasicErrorException );

    void notifyRange() throw ( css::script::BasicErrorException );
    static css::sheet::ConditionOperator retrieveAPIType(sal_Int32 _nVBAType, const css::uno::Reference< css::sheet::XSheetCondition >& _xSheetCondition ) throw( css::script::BasicErrorException );

    //Methods
    virtual void SAL_CALL Delete(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL Modify( ::sal_Int32 Type, const css::uno::Any& Operator, const css::uno::Any& Formula1, const css::uno::Any& Formula2 ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;
    virtual ::sal_Int32 SAL_CALL Type(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;
    virtual ::sal_Int32 Operator( bool  ) throw (css::script::BasicErrorException) override;
    virtual ::sal_Int32 SAL_CALL Operator(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException) override;
    virtual void setFormula1( const css::uno::Any& _aFormula1) throw ( css::script::BasicErrorException ) override;
    virtual css::uno::Reference< ::ooo::vba::excel::XInterior > SAL_CALL Interior(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL Borders( const css::uno::Any& Index ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< ::ooo::vba::excel::XFont > SAL_CALL Font(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;
    // XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
