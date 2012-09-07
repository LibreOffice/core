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
#ifndef SC_VBA_COMMANDBARS_HXX
#define SC_VBA_COMMANDBARS_HXX

#include <ooo/vba/XCommandBar.hpp>
#include <ooo/vba/XCommandBars.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <cppuhelper/implbase1.hxx>
#include <vbahelper/vbahelperinterface.hxx>
#include <vbahelper/vbacollectionimpl.hxx>
#include "vbacommandbarhelper.hxx"

typedef CollTestImplHelper< ov::XCommandBars > CommandBars_BASE;

class ScVbaCommandBars : public CommandBars_BASE
{
private:
    VbaCommandBarHelperRef m_pCBarHelper;

public:
    ScVbaCommandBars( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::container::XIndexAccess >& xIndexAccess, const css::uno::Reference< css::frame::XModel >& xModel ) throw (css::uno::RuntimeException);
    virtual ~ScVbaCommandBars();

    // XCommandBars
    virtual css::uno::Reference< ov::XCommandBar > SAL_CALL Add( const css::uno::Any& Name, const css::uno::Any& Position, const css::uno::Any& MenuBar, const css::uno::Any& Temporary ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    // XEnumerationAccess
    virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw (css::uno::RuntimeException);
    virtual css::uno::Any createCollectionObject( const css::uno::Any& aSource );

    virtual sal_Int32 SAL_CALL getCount() throw(css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL Item( const css::uno::Any& aIndex, const css::uno::Any& /*aIndex2*/ ) throw( css::uno::RuntimeException);
    // XHelperInterface
    virtual OUString getServiceImplName();
    virtual css::uno::Sequence<OUString> getServiceNames();
};

#endif//SC_VBA_COMMANDBARS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
