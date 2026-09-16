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

#include <ooo/vba/excel/XComment.hpp>
#include <ooo/vba/msforms/XShape.hpp>
#include <com/sun/star/sheet/XSheetAnnotations.hpp>
#include <com/sun/star/sheet/XSheetAnnotation.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <cpo/uno/XComponentContext.hpp>

#include <vbahelper/vbahelperinterface.hxx>

typedef InheritedHelperInterfaceWeakImpl< ov::excel::XComment > ScVbaComment_BASE;

class ScVbaComment : public ScVbaComment_BASE
{
    cpo::uno::Reference< css::frame::XModel > mxModel;
    cpo::uno::Reference< css::table::XCellRange > mxRange;

private:
    /// @throws cpo::uno::RuntimeException
    cpo::uno::Reference< css::sheet::XSheetAnnotation > getAnnotation();
    /// @throws cpo::uno::RuntimeException
    cpo::uno::Reference< css::sheet::XSheetAnnotations > getAnnotations() const;
    /// @throws cpo::uno::RuntimeException
    sal_Int32 getAnnotationIndex();
    /// @throws cpo::uno::RuntimeException
    cpo::uno::Reference< ov::excel::XComment > getCommentByIndex( sal_Int32 Index );
public:
    /// @throws css::lang::IllegalArgumentException
    /// @throws cpo::uno::RuntimeException
    ScVbaComment(
        const cpo::uno::Reference< ov::XHelperInterface >& xParent,
        const cpo::uno::Reference< cpo::uno::XComponentContext >& xContext,
        const cpo::uno::Reference< css::frame::XModel >& xModel,
        const cpo::uno::Reference< css::table::XCellRange >& xRange );

    // Attributes
    virtual OUString getAuthor() override;
    virtual void setAuthor( const OUString& _author ) override;
    virtual cpo::uno::Reference< ov::msforms::XShape > getShape() override;
    virtual bool getVisible() override;
    virtual void setVisible( bool _visible ) override;

    // Methods
    virtual void Delete() override;
    virtual cpo::uno::Reference< ov::excel::XComment > Next() override;
    virtual cpo::uno::Reference< ov::excel::XComment > Previous() override;
    virtual OUString Text( const cpo::uno::Any& Text, const cpo::uno::Any& Start, const cpo::uno::Any& Overwrite ) override;
    // XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual cpo::uno::Sequence<OUString> getServiceNames() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
