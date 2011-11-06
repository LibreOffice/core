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



#ifndef _DBA_COREDATAACCESS_COMMANDCONTAINER_HXX_
#define _DBA_COREDATAACCESS_COMMANDCONTAINER_HXX_

#ifndef _DBA_CORE_DEFINITIONCONTAINER_HXX_
#include "definitioncontainer.hxx"
#endif

#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

//........................................................................
namespace dbaccess
{
//........................................................................
//==========================================================================
//= OCommandContainer
//==========================================================================

typedef ::cppu::ImplHelper1 <   ::com::sun::star::lang::XSingleServiceFactory
                            >   OCommandContainer_BASE;

class OCommandContainer : public ODefinitionContainer
                         ,public OCommandContainer_BASE
{
    sal_Bool m_bTables;

public:
    /** constructs the container.<BR>
    */
    OCommandContainer(
         const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _xORB
        ,const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >&   _xParentContainer
        ,const TContentPtr& _pImpl
        ,sal_Bool _bTables
        );

    DECLARE_XINTERFACE( )
    DECLARE_TYPEPROVIDER( );

    // XSingleServiceFactory
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL createInstance( ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL createInstanceWithArguments( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

protected:
    virtual ~OCommandContainer();

    // ODefinitionContainer
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContent > createObject(const ::rtl::OUString& _rName);

protected:
    // OContentHelper overridables
    virtual ::rtl::OUString determineContentType() const;
};

//........................................................................
}   // namespace dbaccess
//........................................................................

#endif // _DBA_COREDATAACCESS_COMMANDCONTAINER_HXX_


