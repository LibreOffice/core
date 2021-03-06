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

#include <rtl/ref.hxx>
#include <vbahelper/vbaeventshelperbase.hxx>

class ScDocShell;
class ScDocument;
class ScVbaEventListener;

class ScVbaEventsHelper : public VbaEventsHelperBase
{
public:
    ScVbaEventsHelper( const css::uno::Sequence< css::uno::Any >& rArgs );
    virtual ~ScVbaEventsHelper() override;

    virtual void SAL_CALL notifyEvent( const css::document::EventObject& rEvent ) override;

    OUString SAL_CALL getImplementationName() override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

protected:
    virtual bool implPrepareEvent( EventQueue& rEventQueue, const EventHandlerInfo& rInfo, const css::uno::Sequence< css::uno::Any >& rArgs ) override;
    virtual css::uno::Sequence< css::uno::Any > implBuildArgumentList( const EventHandlerInfo& rInfo, const css::uno::Sequence< css::uno::Any >& rArgs ) override;
    virtual void implPostProcessEvent( EventQueue& rEventQueue, const EventHandlerInfo& rInfo, bool bCancel ) override;
    virtual OUString implGetDocumentModuleName( const EventHandlerInfo& rInfo, const css::uno::Sequence< css::uno::Any >& rArgs ) const override;

private:
    /** Checks if selection has been changed compared to selection of last call.
        @return true, if the selection has been changed.
        @throws css::lang::IllegalArgumentException
        @throws css::uno::RuntimeException
    */
    bool isSelectionChanged( const css::uno::Sequence< css::uno::Any >& rArgs, sal_Int32 nIndex );

    /** Creates a VBA Worksheet object (the argument must contain a sheet index).
        @throws css::lang::IllegalArgumentException
        @throws css::uno::RuntimeException
    */
    css::uno::Any createWorksheet( const css::uno::Sequence< css::uno::Any >& rArgs, sal_Int32 nIndex ) const;
    /** Creates a VBA Range object (the argument must contain a UNO range or UNO range list).
        @throws css::lang::IllegalArgumentException
        @throws css::uno::RuntimeException
    */
    css::uno::Any createRange( const css::uno::Sequence< css::uno::Any >& rArgs, sal_Int32 nIndex ) const;
    /** Creates a VBA Hyperlink object (the argument must contain a UNO cell).
        @throws css::lang::IllegalArgumentException
        @throws css::uno::RuntimeException
    */
    css::uno::Any createHyperlink( const css::uno::Sequence< css::uno::Any >& rArgs, sal_Int32 nIndex ) const;
    /** Creates a VBA Window object (the argument must contain a model controller).
        @throws css::lang::IllegalArgumentException
        @throws css::uno::RuntimeException
    */
    css::uno::Any createWindow( const css::uno::Sequence< css::uno::Any >& rArgs, sal_Int32 nIndex ) const;

private:
    ::rtl::Reference< ScVbaEventListener > mxListener;
    css::uno::Any maOldSelection;
    ScDocShell* mpDocShell;
    ScDocument* mpDoc;
    bool mbOpened;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
