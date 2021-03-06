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

#pragma once

#include <ooo/vba/excel/XFormatConditions.hpp>
#include <vbahelper/vbacollectionimpl.hxx>

namespace com::sun::star::beans { class XPropertySet; }
namespace com::sun::star::sheet { class XSheetConditionalEntries; }
namespace ooo::vba::excel { class XRange; }
namespace ooo::vba::excel { class XStyle; }
namespace ooo::vba::excel { class XStyles; }

// This class is used only as a target for casting, it seems,
// and no objects of this type are created as such, I think.

class ScVbaFormatConditions: public CollTestImplHelper< ov::excel::XFormatConditions >
{
    css::uno::Reference< css::sheet::XSheetConditionalEntries > mxSheetConditionalEntries;
    css::uno::Reference< ov::excel::XStyles > mxStyles;
    css::uno::Reference< ov::excel::XRange > mxRangeParent;
    css::uno::Reference< css::beans::XPropertySet > mxParentRangePropertySet;
public:
    /// @throws css::script::BasicErrorException
    void notifyRange();
    /// @throws css::script::BasicErrorException
    /// @throws css::uno::RuntimeException
    css::uno::Reference< ov::excel::XFormatCondition > Add( ::sal_Int32 Type, const css::uno::Any& Operator, const css::uno::Any& Formula1, const css::uno::Any& Formula2, const css::uno::Reference< ov::excel::XStyle >& _xCalcStyle );
    /// @throws css::script::BasicErrorException
    static OUString getA1Formula(const css::uno::Any& _aFormula);
    OUString getStyleName();
    /// @throws css::script::BasicErrorException
    void removeFormatCondition( const OUString& _sStyleName, bool _bRemoveStyle);
    const css::uno::Reference< css::sheet::XSheetConditionalEntries >& getSheetConditionalEntries() const { return mxSheetConditionalEntries; }
    // XFormatConditions
    virtual void SAL_CALL Delete(  ) override;
    virtual css::uno::Reference< ov::excel::XFormatCondition > SAL_CALL Add( ::sal_Int32 Type, const css::uno::Any& Operator, const css::uno::Any& Formula1, const css::uno::Any& Formula2 ) override;
    // XEnumerationAccess
    virtual css::uno::Type SAL_CALL getElementType() override;
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() override;
    virtual css::uno::Any createCollectionObject(const css::uno::Any&) override;
    // XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;

    ScVbaFormatConditions() = delete;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
