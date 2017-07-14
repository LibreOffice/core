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
#ifndef INCLUDED_SC_SOURCE_UI_VBA_VBAFILEDIALOG_HXX
#define INCLUDED_SC_SOURCE_UI_VBA_VBAFILEDIALOG_HXX

#include <ooo/vba/excel/XFileDialog.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <vbahelper/vbahelperinterface.hxx>

#include "vbafiledialogitems.hxx"

typedef InheritedHelperInterfaceWeakImpl< ov::excel::XFileDialog > ScVbaFileDialog_BASE;

class ScVbaFileDialog : public ScVbaFileDialog_BASE
{
private:
    sal_Int32 m_nType;
    OUString m_sTitle;
    OUString m_sInitialFileName;
    css::uno::Reference< ov::excel::XFileDialogSelectedItems> m_xItems;
public:
    ScVbaFileDialog( const css::uno::Reference< ov::XHelperInterface >& xParent,  const css::uno::Reference< css::uno::XComponentContext >& xContext, const sal_Int32 nType);

    virtual css::uno::Any SAL_CALL getInitialFileName() override;
    virtual void SAL_CALL setInitialFileName( const css::uno::Any& rName ) override;
    virtual css::uno::Any SAL_CALL getTitle() override;
    virtual void SAL_CALL setTitle( const css::uno::Any& rTitle ) override;
    virtual css::uno::Reference< ov::excel::XFileDialogSelectedItems > SAL_CALL getSelectedItems() override;

    virtual sal_Int32 SAL_CALL Show() override;

    //XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
