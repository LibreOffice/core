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

#ifndef INCLUDED_SC_SOURCE_UI_VBA_VBAEVENTSHELPER_HXX
#define INCLUDED_SC_SOURCE_UI_VBA_VBAEVENTSHELPER_HXX

#include <rtl/ref.hxx>
#include <vbahelper/vbaeventshelperbase.hxx>
#include "excelvbahelper.hxx"
#include "rangelst.hxx"

class ScVbaEventListener;

class ScVbaEventsHelper : public VbaEventsHelperBase
{
public:
    ScVbaEventsHelper(
        const css::uno::Sequence< css::uno::Any >& rArgs,
        const css::uno::Reference< css::uno::XComponentContext >& rxContext );
    virtual ~ScVbaEventsHelper();

    virtual void SAL_CALL notifyEvent( const css::document::EventObject& rEvent ) throw (css::uno::RuntimeException, std::exception) override;

    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) override;

protected:
    virtual bool implPrepareEvent( EventQueue& rEventQueue, const EventHandlerInfo& rInfo, const css::uno::Sequence< css::uno::Any >& rArgs ) throw (css::uno::RuntimeException) override;
    virtual css::uno::Sequence< css::uno::Any > implBuildArgumentList( const EventHandlerInfo& rInfo, const css::uno::Sequence< css::uno::Any >& rArgs ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
    virtual void implPostProcessEvent( EventQueue& rEventQueue, const EventHandlerInfo& rInfo, bool bCancel ) throw (css::uno::RuntimeException) override;
    virtual OUString implGetDocumentModuleName( const EventHandlerInfo& rInfo, const css::uno::Sequence< css::uno::Any >& rArgs ) const throw (css::lang::IllegalArgumentException, css::uno::RuntimeException) override;

private:
    /** Checks if selection has been changed compared to selection of last call.
        @return true, if the selection has been changed. */
    bool isSelectionChanged( const css::uno::Sequence< css::uno::Any >& rArgs, sal_Int32 nIndex ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException);

    /** Creates a VBA Worksheet object (the argument must contain a sheet index). */
    css::uno::Any createWorksheet( const css::uno::Sequence< css::uno::Any >& rArgs, sal_Int32 nIndex ) const throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception);
    /** Creates a VBA Range object (the argument must contain a UNO range or UNO range list). */
    css::uno::Any createRange( const css::uno::Sequence< css::uno::Any >& rArgs, sal_Int32 nIndex ) const throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception);
    /** Creates a VBA Hyperlink object (the argument must contain a UNO cell). */
    css::uno::Any createHyperlink( const css::uno::Sequence< css::uno::Any >& rArgs, sal_Int32 nIndex ) const throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception);
    /** Creates a VBA Window object (the argument must contain a model controller). */
    css::uno::Any createWindow( const css::uno::Sequence< css::uno::Any >& rArgs, sal_Int32 nIndex ) const throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception);

private:
    ::rtl::Reference< ScVbaEventListener > mxListener;
    css::uno::Any maOldSelection;
    ScDocShell* mpDocShell;
    ScDocument* mpDoc;
    bool mbOpened;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
