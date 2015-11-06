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

#include "vbaeventshelper.hxx"
#include <com/sun/star/script/ModuleType.hpp>
#include <com/sun/star/script/vba/VBAEventId.hpp>
#include <vbahelper/helperdecl.hxx>

using namespace ::com::sun::star;
using namespace css::script::vba::VBAEventId;
using namespace ::ooo::vba;

SwVbaEventsHelper::SwVbaEventsHelper( uno::Sequence< css::uno::Any > const& aArgs, uno::Reference< uno::XComponentContext > const& xContext ) :
    VbaEventsHelperBase( aArgs, xContext )
{
    using namespace css::script::ModuleType;
    registerEventHandler( DOCUMENT_NEW,     DOCUMENT,   "Document_New" );
    registerEventHandler( AUTO_NEW,         NORMAL,     "AutoNew" );
    registerEventHandler( DOCUMENT_OPEN,    DOCUMENT,   "Document_Open" );
    registerEventHandler( AUTO_OPEN,        NORMAL,     "AutoOpen" );
    registerEventHandler( DOCUMENT_CLOSE,   DOCUMENT,   "Document_Close" );
    registerEventHandler( AUTO_CLOSE,       NORMAL,     "AutoClose" );
}

SwVbaEventsHelper::~SwVbaEventsHelper()
{
}

bool SwVbaEventsHelper::implPrepareEvent( EventQueue& rEventQueue,
        const EventHandlerInfo& rInfo, const uno::Sequence< uno::Any >& /*rArgs*/ ) throw (uno::RuntimeException)
{
    switch( rInfo.mnEventId )
    {
        case DOCUMENT_NEW:
            rEventQueue.push_back( AUTO_NEW );
        break;
        case DOCUMENT_OPEN:
            rEventQueue.push_back( AUTO_OPEN );
        break;
        case DOCUMENT_CLOSE:
            rEventQueue.push_back( AUTO_CLOSE );
        break;
    }
    return true;
}

uno::Sequence< uno::Any > SwVbaEventsHelper::implBuildArgumentList( const EventHandlerInfo& /*rInfo*/,
        const uno::Sequence< uno::Any >& /*rArgs*/ ) throw (lang::IllegalArgumentException)
{
    // no event handler expects any arguments
    return uno::Sequence< uno::Any >();
}

void SwVbaEventsHelper::implPostProcessEvent( EventQueue& /*rEventQueue*/,
        const EventHandlerInfo& /*rInfo*/, bool /*bCancel*/ ) throw (uno::RuntimeException)
{
    // nothing to do after any event
}

OUString SwVbaEventsHelper::implGetDocumentModuleName( const EventHandlerInfo& /*rInfo*/,
        const uno::Sequence< uno::Any >& /*rArgs*/ ) const throw (lang::IllegalArgumentException)
{
    // TODO: get actual codename from document
    return OUString( "ThisDocument" );
}

namespace vbaeventshelper
{
namespace sdecl = comphelper::service_decl;
sdecl::inheritingClass_<SwVbaEventsHelper, sdecl::with_args<true> > serviceImpl;
extern sdecl::ServiceDecl const serviceDecl(
    serviceImpl,
    "SwVbaEventsHelper",
    "com.sun.star.document.vba.VBATextEventProcessor" );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
