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

#ifndef _DBA_COREDATAACCESS_COMMANDCONTAINER_HXX_
#define _DBA_COREDATAACCESS_COMMANDCONTAINER_HXX_

#include "definitioncontainer.hxx"

#include <com/sun/star/lang/XSingleServiceFactory.hpp>

#include <cppuhelper/implbase1.hxx>

namespace dbaccess
{
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

}   // namespace dbaccess


#endif // _DBA_COREDATAACCESS_COMMANDCONTAINER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
