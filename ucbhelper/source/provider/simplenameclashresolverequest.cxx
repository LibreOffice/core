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

#include <com/sun/star/ucb/NameClashResolveRequest.hpp>
#include <com/sun/star/ucb/XInteractionSupplyName.hpp>
#include <cppuhelper/typeprovider.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <ucbhelper/simplenameclashresolverequest.hxx>

using namespace com::sun::star;

namespace ucbhelper {

/**
  * This class implements a standard interaction continuation, namely the
  * interface XInteractionSupplyName. Instances of this class can be passed
  * along with an interaction request to indicate the possibility to
  * supply a new name.
  */
class InteractionSupplyName : public InteractionContinuation,
                              public com::sun::star::lang::XTypeProvider,
                              public com::sun::star::ucb::XInteractionSupplyName
{
    OUString m_aName;

public:
    explicit InteractionSupplyName( InteractionRequest * pRequest )
    : InteractionContinuation( pRequest ) {}

    // XInterface
    virtual com::sun::star::uno::Any SAL_CALL
    queryInterface( const com::sun::star::uno::Type & rType )
        throw( com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL acquire()
        throw() override;
    virtual void SAL_CALL release()
        throw() override;

    // XTypeProvider
    virtual com::sun::star::uno::Sequence< com::sun::star::uno::Type > SAL_CALL
    getTypes()
        throw( com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL
    getImplementationId()
        throw( com::sun::star::uno::RuntimeException, std::exception ) override;

    // XInteractionContinuation
    virtual void SAL_CALL select()
        throw( com::sun::star::uno::RuntimeException, std::exception ) override;

    // XInteractionSupplyName
    virtual void SAL_CALL setName( const OUString& Name )
        throw ( com::sun::star::uno::RuntimeException, std::exception ) override;

    // Non-interface methods.

    /**
      * This method returns the name that was supplied by the interaction
      * handler.
      *
      * @return the name.
      */
    const OUString & getName() const { return m_aName; }
};

void SAL_CALL InteractionSupplyName::acquire()
    throw()
{
    OWeakObject::acquire();
}

void SAL_CALL InteractionSupplyName::release()
    throw()
{
    OWeakObject::release();
}

uno::Any SAL_CALL
InteractionSupplyName::queryInterface( const uno::Type & rType )
    throw ( uno::RuntimeException, std::exception )
{
    uno::Any aRet = cppu::queryInterface( rType,
                static_cast< lang::XTypeProvider * >( this ),
                static_cast< task::XInteractionContinuation * >( this ),
                static_cast< ucb::XInteractionSupplyName * >( this ) );

    return aRet.hasValue()
            ? aRet : InteractionContinuation::queryInterface( rType );
}

uno::Sequence< sal_Int8 > SAL_CALL InteractionSupplyName::getImplementationId()
    throw( uno::RuntimeException, std::exception )
{
    return css::uno::Sequence<sal_Int8>();
}

uno::Sequence< uno::Type > SAL_CALL InteractionSupplyName::getTypes()
    throw( uno::RuntimeException, std::exception )
{
    static cppu::OTypeCollection* pCollection = 0;
      if ( !pCollection )
      {
        osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );
        if ( !pCollection )
        {
            static cppu::OTypeCollection collection(
                cppu::UnoType<lang::XTypeProvider>::get(),
                cppu::UnoType<ucb::XInteractionSupplyName>::get() );
            pCollection = &collection;
        }
    }
    return (*pCollection).getTypes();
}

void SAL_CALL InteractionSupplyName::select()
    throw( uno::RuntimeException, std::exception )
{
    recordSelection();
}

void SAL_CALL
InteractionSupplyName::setName( const OUString& Name )
    throw( uno::RuntimeException, std::exception )
{
    m_aName = Name;
}

SimpleNameClashResolveRequest::~SimpleNameClashResolveRequest() {}

SimpleNameClashResolveRequest::SimpleNameClashResolveRequest(
                                    const OUString & rTargetFolderURL,
                                    const OUString & rClashingName,
                                    const OUString & rProposedNewName,
                                    bool bSupportsOverwriteData )
{
    // Fill request...
    ucb::NameClashResolveRequest aRequest;
//    aRequest.Message        = // OUString
//    aRequest.Context        = // XInterface
    aRequest.Classification  = task::InteractionClassification_QUERY;
    aRequest.TargetFolderURL = rTargetFolderURL;
    aRequest.ClashingName    = rClashingName;
    aRequest.ProposedNewName = rProposedNewName;

    setRequest( uno::makeAny( aRequest ) );

    // Fill continuations...
    m_xNameSupplier = new InteractionSupplyName( this );

    uno::Sequence< uno::Reference< task::XInteractionContinuation > >
                            aContinuations( bSupportsOverwriteData ? 3 : 2 );
    aContinuations[ 0 ] = new InteractionAbort( this );
    aContinuations[ 1 ] = m_xNameSupplier.get();

    if ( bSupportsOverwriteData )
        aContinuations[ 2 ] = new InteractionReplaceExistingData( this );

    setContinuations( aContinuations );
}

const OUString SimpleNameClashResolveRequest::getNewName() const
{
    return m_xNameSupplier->getName();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
