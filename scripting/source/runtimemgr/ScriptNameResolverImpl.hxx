/*************************************************************************
*
*  $RCSfile: ScriptNameResolverImpl.hxx,v $
*
*  $Revision: 1.11 $
*
*  last change: $Author: npower $ $Date: 2003-03-03 18:39:41 $
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

#ifndef _FRAMEWORK_SCRIPT_SCRIPTNAMERESOLVERIMPL_HXX_
#define  _FRAMEWORK_SCRIPT_SCRIPTNAMERESOLVERIMPL_HXX_

#include <cppuhelper/implbase1.hxx> // helper for XInterface, XTypeProvider etc.

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/script/CannotConvertException.hpp>
#include <com/sun/star/reflection/InvocationTargetException.hpp>

#include <drafts/com/sun/star/script/framework/runtime/XScriptNameResolver.hpp>
#include <drafts/com/sun/star/script/framework/storage/XScriptInfoAccess.hpp>
#include <drafts/com/sun/star/script/framework/storage/XScriptInfo.hpp>

namespace scripting_runtimemgr
{
// for simplification
#define css ::com::sun::star
#define dcsssf ::drafts::com::sun::star::script::framework

class ScriptNameResolverImpl : public
    ::cppu::WeakImplHelper1 < dcsssf::runtime::XScriptNameResolver >
{
public:
    /**********************************************
     ScriptNameResolverImpl Constructor
     @param  the current context
    */
    ScriptNameResolverImpl(
        const css::uno::Reference< css::uno::XComponentContext > & xContext );
    ~ScriptNameResolverImpl();

    // XServiceInfo implementation
    virtual ::rtl::OUString SAL_CALL getImplementationName()
        throw( css::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
        throw( css::uno::RuntimeException );
    virtual css::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
        throw( css::uno::RuntimeException );

    /**********************************************
     resolve method
     @param  scriptURI this is the given ScriptURI
     @param invocationCtx  the invocation context contains the
      documentStorageID and document reference for use in script name
      resolving. On full name resolution it sets the resolvedScriptStorageID to
      the actual storage location of the fully resolved script. May or may not * be the
      same as the documentStorageID.
     @exception CannotResolveScriptNameException
     @exception IllegalArgumentException
     @exception NullPointerException
     @return  the resolved XScriptURI
    */
    css::uno::Reference < dcsssf::storage::XScriptInfo > SAL_CALL resolve(
        const ::rtl::OUString & scriptURI,
        css::uno::Any& invocationCtx )
        throw( css::script::CannotConvertException, css::lang::IllegalArgumentException,
           css::uno::RuntimeException );
private:
    css::uno::Reference < dcsssf::storage::XScriptInfo >
    resolveURIFromStorageID( sal_Int32 sid, const rtl::OUString & docURI,
        const ::rtl::OUString & nameToResolve )
        SAL_THROW ( ( css::lang::IllegalArgumentException, css::uno::RuntimeException ) );
    css::uno::Reference< dcsssf::storage::XScriptInfoAccess >
    getStorageInstance( sal_Int32 sid, const rtl::OUString & docURI)
        SAL_THROW ( ( css::uno::RuntimeException ) );

    /**********************************************
     Reference< XComponentContext > m_xContext
        to obtain other services if needed
    */
    css::uno::Reference< css::uno::XComponentContext > m_xContext;
    css::uno::Reference< css::lang::XMultiComponentFactory > m_xMultiComFac;
    ::osl::Mutex m_mutex;

};
} // scripting_runtimemgr

#endif //_FRAMEWORK_SCRIPT_SCRIPTNAMERESOLVERIMPL_HXX_
