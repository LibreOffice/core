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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_scripting.hxx"
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
    OSL_TRACE( "< ScriptingContext ctor called >\n" );

    Any nullAny;

    scripting_constants::ScriptingConstantsPool& scriptingConstantsPool =
        scripting_constants::ScriptingConstantsPool::instance();
    registerPropertyNoMember( scriptingConstantsPool.DOC_REF, DOC_REF_PROPID, SCRIPTINGCONTEXT_DEFAULT_ATTRIBS(),::getCppuType(  (const Reference< css::frame::XModel >* ) NULL ), NULL ) ;
    registerPropertyNoMember( scriptingConstantsPool.DOC_STORAGE_ID, DOC_STORAGE_ID_PROPID, SCRIPTINGCONTEXT_DEFAULT_ATTRIBS(), ::getCppuType(  (const sal_Int32* ) NULL ), NULL ) ;
    registerPropertyNoMember( scriptingConstantsPool.DOC_URI, DOC_URI_PROPID, SCRIPTINGCONTEXT_DEFAULT_ATTRIBS(), ::getCppuType(  (const ::rtl::OUString* ) NULL ), NULL ) ;
    registerPropertyNoMember( scriptingConstantsPool.RESOLVED_STORAGE_ID, RESOLVED_STORAGE_ID_PROPID, SCRIPTINGCONTEXT_DEFAULT_ATTRIBS(), ::getCppuType(  (const sal_Int32* ) NULL ), NULL );
    registerPropertyNoMember( scriptingConstantsPool.SCRIPT_INFO, SCRIPT_INFO_PROPID, SCRIPTINGCONTEXT_DEFAULT_ATTRIBS(), ::getCppuType(  (const sal_Int32* ) NULL ), NULL );
}

ScriptingContext::~ScriptingContext()
{
    OSL_TRACE( "< ScriptingContext dtor called >\n" );
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
