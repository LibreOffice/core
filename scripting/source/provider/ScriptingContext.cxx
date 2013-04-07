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

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/frame/XModel.hpp>

#include <cppuhelper/implementationentry.hxx>
#include <cppuhelper/factory.hxx>

#include <util/scriptingconstants.hxx>
#include <util/util.hxx>

#include "ScriptingContext.hxx"

using namespace com::sun::star;
using namespace com::sun::star::uno;
#define DOC_REF_PROPID  1
#define DOC_STORAGE_ID_PROPID 2
#define DOC_URI_PROPID   3
#define RESOLVED_STORAGE_ID_PROPID  4
#define SCRIPT_INFO_PROPID  5
#define SCRIPTINGCONTEXT_DEFAULT_ATTRIBS()   beans::PropertyAttribute::TRANSIENT | beans::PropertyAttribute::MAYBEVOID
namespace func_provider
{

//*************************************************************************
// XScriptingContext implementation
//
//*************************************************************************
ScriptingContext::ScriptingContext( const Reference< XComponentContext > & xContext ) : //ScriptingContextImpl_BASE( GetMutex()),
    OPropertyContainer( GetBroadcastHelper() ),
        m_xContext( xContext, UNO_SET_THROW )
{
    OSL_TRACE( "< ScriptingContext ctor called >" );

    Any nullAny;

    scripting_constants::ScriptingConstantsPool& scriptingConstantsPool =
        scripting_constants::ScriptingConstantsPool::instance();
    registerPropertyNoMember( scriptingConstantsPool.DOC_REF, DOC_REF_PROPID, SCRIPTINGCONTEXT_DEFAULT_ATTRIBS(),::getCppuType(  (const Reference< css::frame::XModel >* ) NULL ), NULL ) ;
    registerPropertyNoMember( scriptingConstantsPool.DOC_STORAGE_ID, DOC_STORAGE_ID_PROPID, SCRIPTINGCONTEXT_DEFAULT_ATTRIBS(), ::getCppuType(  (const sal_Int32* ) NULL ), NULL ) ;
    registerPropertyNoMember( scriptingConstantsPool.DOC_URI, DOC_URI_PROPID, SCRIPTINGCONTEXT_DEFAULT_ATTRIBS(), ::getCppuType(  (const OUString* ) NULL ), NULL ) ;
    registerPropertyNoMember( scriptingConstantsPool.RESOLVED_STORAGE_ID, RESOLVED_STORAGE_ID_PROPID, SCRIPTINGCONTEXT_DEFAULT_ATTRIBS(), ::getCppuType(  (const sal_Int32* ) NULL ), NULL );
    registerPropertyNoMember( scriptingConstantsPool.SCRIPT_INFO, SCRIPT_INFO_PROPID, SCRIPTINGCONTEXT_DEFAULT_ATTRIBS(), ::getCppuType(  (const sal_Int32* ) NULL ), NULL );
}

ScriptingContext::~ScriptingContext()
{
    OSL_TRACE( "< ScriptingContext dtor called >" );
}
// -----------------------------------------------------------------------------
// OPropertySetHelper
// -----------------------------------------------------------------------------

::cppu::IPropertyArrayHelper& ScriptingContext::getInfoHelper(  )
{
    return *getArrayHelper();
}

// -----------------------------------------------------------------------------
// OPropertyArrayUsageHelper
// -----------------------------------------------------------------------------

::cppu::IPropertyArrayHelper* ScriptingContext::createArrayHelper(  ) const
{
    Sequence< beans::Property > aProps;
    describeProperties( aProps );
    return new ::cppu::OPropertyArrayHelper( aProps );
}
// -----------------------------------------------------------------------------
// XPropertySet
// -----------------------------------------------------------------------------

Reference< beans::XPropertySetInfo > ScriptingContext::getPropertySetInfo(  ) throw (RuntimeException)
{
    Reference< beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}
// -----------------------------------------------------------------------------// XTypeProvider
// -----------------------------------------------------------------------------
IMPLEMENT_GET_IMPLEMENTATION_ID( ScriptingContext )

css::uno::Sequence< css::uno::Type > SAL_CALL ScriptingContext::getTypes(  ) throw (css::uno::RuntimeException)
{
    return OPropertyContainer::getTypes();
}
} // namespace func_provider

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
