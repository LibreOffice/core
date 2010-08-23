/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef SW_VBAEVENTS_HXX
#define SW_VBAEVENTS_HXX

#include <vbahelper/vbaeventshelperbase.hxx>

// ============================================================================

class SwVbaEventsHelper : public VbaEventsHelperBase
{
public:
    SwVbaEventsHelper(
        const css::uno::Sequence< css::uno::Any >& rArgs,
        const css::uno::Reference< css::uno::XComponentContext >& rxContext );
    virtual ~SwVbaEventsHelper();

protected:
    virtual bool implPrepareEvent( EventQueue& rEventQueue, const EventHandlerInfo& rInfo, const css::uno::Sequence< css::uno::Any >& rArgs ) throw (css::uno::RuntimeException);
    virtual css::uno::Sequence< css::uno::Any > implBuildArgumentList( const EventHandlerInfo& rInfo, const css::uno::Sequence< css::uno::Any >& rArgs ) throw (css::lang::IllegalArgumentException);
    virtual void implPostProcessEvent( EventQueue& rEventQueue, const EventHandlerInfo& rInfo, bool bSuccess, bool bCancel ) throw (css::uno::RuntimeException);
    virtual ::rtl::OUString implGetDocumentModuleName( const EventHandlerInfo& rInfo, const css::uno::Sequence< css::uno::Any >& rArgs ) const throw (css::lang::IllegalArgumentException);
};

// ============================================================================

#endif

