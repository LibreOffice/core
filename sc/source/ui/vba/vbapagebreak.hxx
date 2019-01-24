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
#ifndef INCLUDED_SC_SOURCE_UI_VBA_VBAPAGEBREAK_HXX
#define INCLUDED_SC_SOURCE_UI_VBA_VBAPAGEBREAK_HXX

#include <ooo/vba/excel/XHPageBreak.hpp>
#include <ooo/vba/excel/XVPageBreak.hpp>
#include <com/sun/star/sheet/TablePageBreakData.hpp>
#include <vbahelper/vbahelperinterface.hxx>

namespace com { namespace sun { namespace star { namespace beans { class XPropertySet; } } } }
namespace com { namespace sun { namespace star { namespace uno { class XComponentContext; } } } }
namespace ooo { namespace vba { namespace excel { class XRange; } } }

template< typename... Ifc >
class ScVbaPageBreak : public InheritedHelperInterfaceWeakImpl< Ifc... >
{
typedef InheritedHelperInterfaceWeakImpl< Ifc... > ScVbaPageBreak_BASE;
protected:
    css::uno::Reference< css::beans::XPropertySet > mxRowColPropertySet;
    css::sheet::TablePageBreakData maTablePageBreakData;
public:
    /// @throws css::uno::RuntimeException
    ScVbaPageBreak( const css::uno::Reference< ov::XHelperInterface >& xParent,
                    const css::uno::Reference< css::uno::XComponentContext >& xContext,
                    const css::uno::Reference< css::beans::XPropertySet >& xProps,
                    css::sheet::TablePageBreakData aTablePageBreakData);

    virtual sal_Int32 SAL_CALL getType( ) override;
    virtual void SAL_CALL setType(sal_Int32 type) override;

    virtual void SAL_CALL Delete() override;
    virtual css::uno::Reference< ov::excel::XRange> SAL_CALL Location() override;
};

typedef ScVbaPageBreak < ov::excel::XHPageBreak > ScVbaHPageBreak_BASE;

class ScVbaHPageBreak :  public ScVbaHPageBreak_BASE
{
public:
    /// @throws css::uno::RuntimeException
    ScVbaHPageBreak( const css::uno::Reference< ov::XHelperInterface >& xParent,
                    const css::uno::Reference< css::uno::XComponentContext >& xContext,
                    const css::uno::Reference< css::beans::XPropertySet >& xProps,
                    css::sheet::TablePageBreakData aTablePageBreakData):
              ScVbaHPageBreak_BASE( xParent,xContext,xProps,aTablePageBreakData ){}

    // XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;
};

//VPageBreak
typedef ScVbaPageBreak < ov::excel::XVPageBreak > ScVbaVPageBreak_BASE;

class ScVbaVPageBreak :  public ScVbaVPageBreak_BASE
{
public:
    /// @throws css::uno::RuntimeException
    ScVbaVPageBreak( const css::uno::Reference< ov::XHelperInterface >& xParent,
                     const css::uno::Reference< css::uno::XComponentContext >& xContext,
                     const css::uno::Reference< css::beans::XPropertySet >& xProps,
                     css::sheet::TablePageBreakData aTablePageBreakData);

    virtual ~ScVbaVPageBreak() override;

    // XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
