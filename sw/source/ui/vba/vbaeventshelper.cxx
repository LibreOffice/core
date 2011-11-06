/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#include "vbaeventshelper.hxx"
#include <com/sun/star/script/ModuleType.hpp>
#include <com/sun/star/script/vba/VBAEventId.hpp>
#include <vbahelper/helperdecl.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::script::vba::VBAEventId;
using namespace ::ooo::vba;

// ============================================================================

SwVbaEventsHelper::SwVbaEventsHelper( uno::Sequence< css::uno::Any > const& aArgs, uno::Reference< uno::XComponentContext > const& xContext ) :
    VbaEventsHelperBase( aArgs, xContext )
{
    using namespace ::com::sun::star::script::ModuleType;
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

::rtl::OUString SwVbaEventsHelper::implGetDocumentModuleName( const EventHandlerInfo& /*rInfo*/,
        const uno::Sequence< uno::Any >& /*rArgs*/ ) const throw (lang::IllegalArgumentException)
{
    // TODO: get actual codename from document
    return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ThisDocument" ) );
}

// ============================================================================

namespace vbaeventshelper
{
namespace sdecl = comphelper::service_decl;
sdecl::class_<SwVbaEventsHelper, sdecl::with_args<true> > serviceImpl;
extern sdecl::ServiceDecl const serviceDecl(
    serviceImpl,
    "SwVbaEventsHelper",
    "com.sun.star.document.vba.VBATextEventProcessor" );
}

// ============================================================================
