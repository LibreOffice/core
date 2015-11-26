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

#ifndef INCLUDED_SC_SOURCE_UI_VBA_VBAFORMATCONDITIONS_HXX
#define INCLUDED_SC_SOURCE_UI_VBA_VBAFORMATCONDITIONS_HXX

#include <ooo/vba/excel/XFormatConditions.hpp>
#include <ooo/vba/excel/XStyles.hpp>
#include <ooo/vba/excel/XRange.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sheet/XSheetConditionalEntries.hpp>
#include <vbahelper/vbacollectionimpl.hxx>

// This class is used only as a target for casting, it seems,
// and no objects of this type are created as such, I think.
// So avoid MSVC warnings:
// warning C4510: 'ScVbaFormatConditions' : default constructor could not be generated
// warning C4610: class 'ScVbaFormatConditions' can never be instantiated - user defined constructor required

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4510)
#pragma warning(disable: 4610)
#endif

class ScVbaFormatConditions: public CollTestImplHelper< ov::excel::XFormatConditions >
{
    css::uno::Reference< css::sheet::XSheetConditionalEntries > mxSheetConditionalEntries;
    css::uno::Reference< ov::excel::XStyles > mxStyles;
    css::uno::Reference< ov::excel::XRange > mxRangeParent;
    css::uno::Reference< css::beans::XPropertySet > mxParentRangePropertySet;
public:
    void notifyRange() throw ( css::script::BasicErrorException );
    css::uno::Reference< ov::excel::XFormatCondition > Add( ::sal_Int32 Type, const css::uno::Any& Operator, const css::uno::Any& Formula1, const css::uno::Any& Formula2, const css::uno::Reference< ov::excel::XStyle >& _xCalcStyle ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    static OUString getA1Formula(const css::uno::Any& _aFormula) throw ( css::script::BasicErrorException );
    OUString getStyleName();
    void removeFormatCondition( const OUString& _sStyleName, bool _bRemoveStyle) throw ( css::script::BasicErrorException );
    css::uno::Reference< css::sheet::XSheetConditionalEntries > getSheetConditionalEntries() const { return mxSheetConditionalEntries; }
    // XFormatConditions
    virtual void SAL_CALL Delete(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< ov::excel::XFormatCondition > SAL_CALL Add( ::sal_Int32 Type, const css::uno::Any& Operator, const css::uno::Any& Formula1, const css::uno::Any& Formula2 ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) override;
    // XEnumerationAccess
    virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException) override;
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw (css::uno::RuntimeException) override;
    virtual css::uno::Any createCollectionObject(const css::uno::Any&) override;
    // XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;
};

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif // INCLUDED_SC_SOURCE_UI_VBA_VBAFORMATCONDITIONS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
