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

#ifndef _DBA_COREDATAACCESS_COMMANDCONTAINER_HXX_
#define _DBA_COREDATAACCESS_COMMANDCONTAINER_HXX_

#include "definitioncontainer.hxx"

#include <com/sun/star/lang/XSingleServiceFactory.hpp>

#include <cppuhelper/implbase1.hxx>

namespace dbaccess
{
// OCommandContainer

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
         const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _xORB
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
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContent > createObject(const OUString& _rName);

protected:
    // OContentHelper overridables
    virtual OUString determineContentType() const;
};

}   // namespace dbaccess

#endif // _DBA_COREDATAACCESS_COMMANDCONTAINER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
