/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ScriptingContext.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 10:22:05 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _FRAMEWORK_SCRIPT_PROTOCOLHANDLER_SCRIPTING_CONTEXT_HXX_
#define _FRAMEWORK_SCRIPT_PROTOCOLHANDLER_SCRIPTING_CONTEXT_HXX_


#include <osl/mutex.hxx>
#include <rtl/ustring.hxx>
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
