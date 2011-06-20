/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _DBA_COREDATAACESS_COMMANDDEFINITION_HXX_
#define _DBA_COREDATAACESS_COMMANDDEFINITION_HXX_

#include "commandbase.hxx"
#include <comphelper/propertycontainer.hxx>
#include "apitools.hxx"
#include <comphelper/uno3.hxx>
#include <com/sun/star/sdbcx/XRename.hpp>
#include <cppuhelper/implbase1.hxx>
#include <comphelper/proparrhlp.hxx>
#include "datasettings.hxx"
#include <com/sun/star/container/XNameAccess.hpp>
#include "ContentHelper.hxx"
#include "ComponentDefinition.hxx"


//........................................................................
namespace dbaccess
{
//........................................................................

//=========================================================================
//= OCommandDefinition - a database "document" which describes a query
//=========================================================================
    class OCommandDefinition_Impl : public OComponentDefinition_Impl
                                  ,public OCommandBase
    {
    public:
    };

typedef ::cppu::ImplHelper1 <   ::com::sun::star::sdbcx::XRename
                                >   OCommandDefinition_Base;
class OCommandDefinition;
typedef ::comphelper::OPropertyArrayUsageHelper< OCommandDefinition >
                        OCommandDefinition_PROP;

class OCommandDefinition    :public OComponentDefinition
                            ,public OCommandDefinition_Base
                            ,public OCommandDefinition_PROP
{
protected:
    virtual ~OCommandDefinition();

    OCommandDefinition(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&
        ,const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >&   _xParentContainer
        ,const TContentPtr& _pImpl
        );

    inline const OCommandDefinition_Impl& getCommandDefinition() const { return dynamic_cast< const OCommandDefinition_Impl& >( *m_pImpl.get() ); }
    inline       OCommandDefinition_Impl& getCommandDefinition()       { return dynamic_cast<       OCommandDefinition_Impl& >( *m_pImpl.get() ); }

public:

    OCommandDefinition(
             const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxContainer
            ,const ::rtl::OUString& _rElementName
            ,const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&
            ,const TContentPtr& _pImpl
        );

// com::sun::star::lang::XTypeProvider
    DECLARE_TYPEPROVIDER( );

// ::com::sun::star::uno::XInterface
    DECLARE_XINTERFACE( )

// ::com::sun::star::lang::XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::lang::XServiceInfo - static methods
    static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_static(void) throw( ::com::sun::star::uno::RuntimeException );
    static ::rtl::OUString getImplementationName_static(void) throw( ::com::sun::star::uno::RuntimeException );
    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
        Create(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >&);

    // XRename
    virtual void SAL_CALL rename( const ::rtl::OUString& newName ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::container::ElementExistException, ::com::sun::star::uno::RuntimeException);

    // OPropertySetHelper
    DECLARE_PROPERTYCONTAINER_DEFAULTS( );

private:
    // helper
    void registerProperties();
};

//........................................................................
}   // namespace dbaccess
//........................................................................

#endif // _DBA_COREDATAACESS_COMMANDDEFINITION_HXX_


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
