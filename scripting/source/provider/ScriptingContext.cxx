/*************************************************************************
 *
 *  $RCSfile: ScriptingContext.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: dfoster $ $Date: 2003-11-04 17:45:28 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
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
        m_xContext( xContext )
{
    OSL_TRACE( "< ScriptingContext ctor called >\n" );

    validateXRef( m_xContext,
                  "ScriptingContext::ScriptingContext: No context available\n" );

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
