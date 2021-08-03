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

#include <memory>
#include <sal/config.h>
#include <sfx2/viewsh.hxx>
#include <sal/types.h>

#include <com/sun/star/view/XPrintable.hpp>
#include <com/sun/star/view/XPrintJobBroadcaster.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/lang/XInitialization.hpp>
#include <osl/mutex.hxx>
#include <cppuhelper/implbase.hxx>

struct  IMPL_PrintListener_DataContainer;
class SfxViewShell;
class SfxPrinter;

class SfxPrintHelper : public cppu::WeakImplHelper
        < css::view::XPrintable
        , css::view::XPrintJobBroadcaster
        , css::lang::XInitialization >
{
public:

    SfxPrintHelper() ;
    virtual ~SfxPrintHelper() override ;

    void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) override;
    virtual void SAL_CALL addPrintJobListener( const css::uno::Reference< css::view::XPrintJobListener >& xListener ) override;
    virtual void SAL_CALL removePrintJobListener( const css::uno::Reference< css::view::XPrintJobListener >& xListener ) override;
    virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL getPrinter() override;
    virtual void SAL_CALL setPrinter( const css::uno::Sequence< css::beans::PropertyValue >& seqPrinter ) override;
    virtual void SAL_CALL print( const css::uno::Sequence< css::beans::PropertyValue >& seqOptions ) override;

private:

    osl::Mutex m_aMutex;
    std::unique_ptr<IMPL_PrintListener_DataContainer> m_pData ;
    void impl_setPrinter(const css::uno::Sequence< css::beans::PropertyValue >& rPrinter,
                         VclPtr<SfxPrinter>& pPrinter,
                         SfxPrinterChangeFlags& nChangeFlags,
                         SfxViewShell*& pViewSh);
} ;



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
