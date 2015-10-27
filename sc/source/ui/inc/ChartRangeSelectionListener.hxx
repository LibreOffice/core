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
#ifndef INCLUDED_SC_SOURCE_UI_INC_CHARTRANGESELECTIONLISTENER_HXX
#define INCLUDED_SC_SOURCE_UI_INC_CHARTRANGESELECTIONLISTENER_HXX

#include <cppuhelper/compbase.hxx>
#include <comphelper/broadcasthelper.hxx>
#include <com/sun/star/view/XSelectionChangeListener.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

class ScTabViewShell;

typedef ::cppu::WeakComponentImplHelper<
        css::view::XSelectionChangeListener,
        css::lang::XServiceInfo >
    ScChartRangeSelectionListener_Base;

class ScChartRangeSelectionListener :
    public comphelper::OBaseMutex,
    public ScChartRangeSelectionListener_Base
{
public:
    explicit ScChartRangeSelectionListener( ScTabViewShell * pViewShell );
    virtual ~ScChartRangeSelectionListener();

protected:
    // ____ XSelectionChangeListener ____
    virtual void SAL_CALL selectionChanged(
        const css::lang::EventObject& aEvent )
        throw (css::uno::RuntimeException, std::exception) override;

    // ____ XEventListener (base of XSelectionChangeListener) ____
    virtual void SAL_CALL disposing(
        const css::lang::EventObject& Source )
        throw (css::uno::RuntimeException, std::exception) override;

    // ____ WeakComponentImplHelperBase ____
    // is called when dispose() is called at this component
    virtual void SAL_CALL disposing() override;

    // ____ XServiceInfo ____
    virtual OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService(
        const OUString& ServiceName )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) override;

private:
    ScTabViewShell * m_pViewShell;
};

// INCLUDED_SC_SOURCE_UI_INC_CHARTRANGESELECTIONLISTENER_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
