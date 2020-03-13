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
#ifndef INCLUDED_SW_SOURCE_UI_VBA_VBACELL_HXX
#define INCLUDED_SW_SOURCE_UI_VBA_VBACELL_HXX

#include <ooo/vba/word/XCell.hpp>
#include <vbahelper/vbahelperinterface.hxx>
#include <com/sun/star/text/XTextTable.hpp>

typedef InheritedHelperInterfaceWeakImpl< ooo::vba::word::XCell > SwVbaCell_BASE;

class SwVbaCell : public SwVbaCell_BASE
{
private:
    css::uno::Reference< css::text::XTextTable > mxTextTable;
    sal_Int32 mnColumn;
    sal_Int32 mnRow;

public:
    /// @throws css::uno::RuntimeException
    SwVbaCell( const css::uno::Reference< ooo::vba::XHelperInterface >& rParent, const css::uno::Reference< css::uno::XComponentContext >& rContext, const css::uno::Reference< css::text::XTextTable >& xTextTable, sal_Int32 nColumn, sal_Int32 nRow );
    virtual ~SwVbaCell() override;

    // Attributes
    virtual ::sal_Int32 SAL_CALL getWidth() override;
    virtual void SAL_CALL setWidth( ::sal_Int32 _width ) override;
    virtual css::uno::Any SAL_CALL getHeight() override;
    virtual void SAL_CALL setHeight( const css::uno::Any& _height ) override;
    virtual ::sal_Int32 SAL_CALL getHeightRule() override;
    virtual void SAL_CALL setHeightRule( ::sal_Int32 _heightrule ) override;

    // Methods
    virtual void SAL_CALL SetWidth( float width, sal_Int32 rulestyle ) override;
    virtual void SAL_CALL SetHeight( float height, sal_Int32 heightrule ) override;

    // XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;
};
#endif // INCLUDED_SW_SOURCE_UI_VBA_VBACELL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
