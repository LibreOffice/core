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
#include <ooo/vba/excel/XFormatCondition.hpp>
#include <ooo/vba/excel/XFormatConditions.hpp>
#include <ooo/vba/excel/XStyle.hpp>
#include <com/sun/star/sheet/XSheetConditionalEntries.hpp>
#include <com/sun/star/sheet/XSheetConditionalEntry.hpp>
#include <com/sun/star/sheet/XSheetCondition.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include "vbacondition.hxx"

typedef ScVbaCondition< ov::excel::XFormatCondition >  ScVbaFormatCondition_BASE;
class ScVbaFormatCondition final : public ScVbaFormatCondition_BASE
{
    OUString msStyleName;
    cpo::uno::Reference< css::sheet::XSheetConditionalEntries > mxSheetConditionalEntries;
    cpo::uno::Reference< ov::excel::XFormatConditions> moFormatConditions;
    cpo::uno::Reference< ov::excel::XStyle > mxStyle;
    cpo::uno::Reference< css::beans::XPropertySet > mxParentRangePropertySet;

public:
    /// @throws cpo::uno::RuntimeException
    /// @throws css::script::BasicErrorException
    ScVbaFormatCondition( const cpo::uno::Reference< ov::XHelperInterface >& xParent,
                          const cpo::uno::Reference< cpo::uno::XComponentContext > & xContext,
                          const cpo::uno::Reference< css::sheet::XSheetConditionalEntry >& _xSheetConditionalEntry,
                          cpo::uno::Reference< ov::excel::XStyle > ,
                          cpo::uno::Reference< ov::excel::XFormatConditions >  _xFormatConditions,
                          cpo::uno::Reference< css::beans::XPropertySet >  _xPropertySet );

    /// @throws css::script::BasicErrorException
    void notifyRange();
    /// @throws css::script::BasicErrorException
    static css::sheet::ConditionOperator retrieveAPIType(sal_Int32 _nVBAType, const cpo::uno::Reference< css::sheet::XSheetCondition >& _xSheetCondition );

    //Methods
    virtual void Delete(  ) override;
    virtual void Modify( ::sal_Int32 Type, const cpo::uno::Any& Operator, const cpo::uno::Any& Formula1, const cpo::uno::Any& Formula2 ) override;
    virtual ::sal_Int32 Type(  ) override;
    using ScVbaFormatCondition_BASE::Operator;
    virtual ::sal_Int32 Operator(  ) override;
    virtual cpo::uno::Reference< ::ooo::vba::excel::XInterior > Interior(  ) override;
    virtual cpo::uno::Any Borders( const cpo::uno::Any& Index ) override;
    virtual cpo::uno::Reference< ::ooo::vba::excel::XFont > Font(  ) override;
    // XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual cpo::uno::Sequence<OUString> getServiceNames() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
