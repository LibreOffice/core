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

#ifndef INCLUDED_SFX2_SOURCE_DOC_PRINTHELPER_HXX
#define INCLUDED_SFX2_SOURCE_DOC_PRINTHELPER_HXX

#include <sal/config.h>
#include <sfx2/dllapi.h>
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
        < com::sun::star::view::XPrintable
        , com::sun::star::view::XPrintJobBroadcaster
        , com::sun::star::lang::XInitialization >
{
public:

    SfxPrintHelper() ;
    virtual ~SfxPrintHelper() ;

    void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addPrintJobListener( const ::com::sun::star::uno::Reference< ::com::sun::star::view::XPrintJobListener >& xListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removePrintJobListener( const ::com::sun::star::uno::Reference< ::com::sun::star::view::XPrintJobListener >& xListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue > SAL_CALL getPrinter() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPrinter( const com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >& seqPrinter )
        throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL print( const com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >& seqOptions )
        throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) override;

private:

    osl::Mutex m_aMutex;
    IMPL_PrintListener_DataContainer* m_pData ;
    void impl_setPrinter(const com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >& rPrinter,
                         SfxPrinter*& pPrinter,
                         SfxPrinterChangeFlags& nChangeFlags,
                         SfxViewShell*& pViewSh);
} ;

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
