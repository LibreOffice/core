/*************************************************************************
 *
 *  $RCSfile: ScriptingContext.hxx,v $
 *
 *  $Revision: 1.5 $
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

#ifndef _FRAMEWORK_SCRIPT_PROTOCOLHANDLER_SCRIPTING_CONTEXT_HXX_
#define _FRAMEWORK_SCRIPT_PROTOCOLHANDLER_SCRIPTING_CONTEXT_HXX_


#include <osl/mutex.hxx>
#include <rtl/ustring>
#include <cppuhelper/implbase1.hxx>
#include <comphelper/uno3.hxx>
#include <comphelper/propertycontainer.hxx>
#include <comphelper/proparrhlp.hxx>

#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/weak.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <comphelper/broadcasthelper.hxx>
namespace func_provider
{
// for simplification
#define css ::com::sun::star

//Typedefs
//=============================================================================
//typedef ::cppu::WeakImplHelper1< css::beans::XPropertySet > ScriptingContextImpl_BASE;

class ScriptingContext : public ::comphelper::OMutexAndBroadcastHelper, public ::comphelper::OPropertyContainer,
                         public ::comphelper::OPropertyArrayUsageHelper< ScriptingContext >,  public css::lang::XTypeProvider, public ::cppu::OWeakObject
{

public:
    ScriptingContext( const css::uno::Reference< css::uno::XComponentContext > & xContext );
    ~ScriptingContext();
    // XInterface

    css::uno::Any SAL_CALL queryInterface( const css::uno::Type& rType )
        throw( css::uno::RuntimeException )
    {
        css::uno::Any aRet( OPropertySetHelper::queryInterface( rType ) );
        return (aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType ));
    }
    void SAL_CALL acquire() throw() { ::cppu::OWeakObject::acquire(); }
    void SAL_CALL release() throw() { ::cppu::OWeakObject::release(); }
    // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  )
            throw ( css::uno::RuntimeException );
    //XTypeProvider
    DECLARE_XTYPEPROVIDER( )

protected:

    // OPropertySetHelper
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper(  );

    // OPropertyArrayUsageHelper
    virtual ::cppu::IPropertyArrayHelper* createArrayHelper(  ) const;
private:
    css::uno::Reference< css::uno::XComponentContext > m_xContext;


};
} // func_provider
#endif //_FRAMEWORK_SCRIPT_PROVIDER_XFUNCTIONPROVIDER_HXX_
