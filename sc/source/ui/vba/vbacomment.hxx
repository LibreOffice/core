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
#ifndef INCLUDED_SC_SOURCE_UI_VBA_VBACOMMENT_HXX
#define INCLUDED_SC_SOURCE_UI_VBA_VBACOMMENT_HXX

#include <ooo/vba/excel/XComment.hpp>
#include <ooo/vba/excel/XApplication.hpp>
#include <ooo/vba/msforms/XShape.hpp>
#include <com/sun/star/sheet/XSheetAnnotations.hpp>
#include <com/sun/star/sheet/XSheetAnnotation.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <vbahelper/vbahelperinterface.hxx>

typedef InheritedHelperInterfaceWeakImpl< ov::excel::XComment > ScVbaComment_BASE;

class ScVbaComment : public ScVbaComment_BASE
{
    css::uno::Reference< css::frame::XModel > mxModel;
    css::uno::Reference< css::table::XCellRange > mxRange;

private:
    css::uno::Reference< css::sheet::XSheetAnnotation > SAL_CALL getAnnotation() throw (css::uno::RuntimeException);
    css::uno::Reference< css::sheet::XSheetAnnotations > SAL_CALL getAnnotations() throw (css::uno::RuntimeException);
    sal_Int32 SAL_CALL getAnnotationIndex() throw (css::uno::RuntimeException);
    css::uno::Reference< ov::excel::XComment > SAL_CALL getCommentByIndex( sal_Int32 Index ) throw (css::uno::RuntimeException);
public:
    ScVbaComment(
        const css::uno::Reference< ov::XHelperInterface >& xParent,
        const css::uno::Reference< css::uno::XComponentContext >& xContext,
        const css::uno::Reference< css::frame::XModel >& xModel,
        const css::uno::Reference< css::table::XCellRange >& xRange ) throw ( css::lang::IllegalArgumentException, css::uno::RuntimeException );

    virtual ~ScVbaComment() {}

    // Attributes
    virtual OUString SAL_CALL getAuthor() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setAuthor( const OUString& _author ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< ov::msforms::XShape > SAL_CALL getShape() throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL getVisible() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setVisible( sal_Bool _visible ) throw (css::uno::RuntimeException, std::exception) override;

    // Methods
    virtual void SAL_CALL Delete() throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< ov::excel::XComment > SAL_CALL Next() throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< ov::excel::XComment > SAL_CALL Previous() throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL Text( const css::uno::Any& Text, const css::uno::Any& Start, const css::uno::Any& Overwrite ) throw (css::uno::RuntimeException, std::exception) override;
    // XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;
};

#endif // INCLUDED_SC_SOURCE_UI_VBA_VBACOMMENT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
