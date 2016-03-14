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
#ifndef INCLUDED_CHART2_SOURCE_CONTROLLER_INC_RANGESELECTIONLISTENER_HXX
#define INCLUDED_CHART2_SOURCE_CONTROLLER_INC_RANGESELECTIONLISTENER_HXX

#include "ControllerLockGuard.hxx"
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/sheet/XRangeSelectionListener.hpp>

namespace chart
{

class RangeSelectionListenerParent
{
public:
    virtual void listeningFinished( const OUString & rNewRange ) = 0;
    virtual void disposingRangeSelection() = 0;

protected:
    ~RangeSelectionListenerParent() {}
};

class RangeSelectionListener : public
    ::cppu::WeakImplHelper<
        css::sheet::XRangeSelectionListener >
{
public:
    explicit RangeSelectionListener(
        RangeSelectionListenerParent & rParent,
        const OUString & rInitialRange,
        const css::uno::Reference< css::frame::XModel >& xModelToLockController );
    virtual ~RangeSelectionListener();

protected:
    // ____ XRangeSelectionListener ____
    virtual void SAL_CALL done( const css::sheet::RangeSelectionEvent& aEvent )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL aborted( const css::sheet::RangeSelectionEvent& aEvent )
        throw (css::uno::RuntimeException, std::exception) override;

    // ____ XEventListener ____
    virtual void SAL_CALL disposing( const css::lang::EventObject& Source )
        throw (css::uno::RuntimeException, std::exception) override;

private:
    RangeSelectionListenerParent & m_rParent;
    OUString m_aRange;
    ControllerLockGuardUNO m_aControllerLockGuard;
};

} //  namespace chart

// INCLUDED_CHART2_SOURCE_CONTROLLER_INC_RANGESELECTIONLISTENER_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
