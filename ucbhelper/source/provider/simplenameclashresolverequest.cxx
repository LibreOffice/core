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
                              public css::lang::XTypeProvider,
                              public css::ucb::XInteractionSupplyName
{
    OUString m_aName;

public:
    explicit InteractionSupplyName( InteractionRequest * pRequest )
    : InteractionContinuation( pRequest ) {}

    // XInterface
    virtual css::uno::Any SAL_CALL
    queryInterface( const css::uno::Type & rType ) override;
    virtual void SAL_CALL acquire()
        noexcept override;
    virtual void SAL_CALL release()
        noexcept override;

    // XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL
    getTypes() override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL
    getImplementationId() override;

    // XInteractionContinuation
    virtual void SAL_CALL select() override;

    // XInteractionSupplyName
    virtual void SAL_CALL setName( const OUString& Name ) override;

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
    noexcept
{
    OWeakObject::acquire();
}

void SAL_CALL InteractionSupplyName::release()
    noexcept
{
    OWeakObject::release();
}

uno::Any SAL_CALL
InteractionSupplyName::queryInterface( const uno::Type & rType )
{
    uno::Any aRet = cppu::queryInterface( rType,
                static_cast< lang::XTypeProvider * >( this ),
                static_cast< task::XInteractionContinuation * >( this ),
                static_cast< ucb::XInteractionSupplyName * >( this ) );

    return aRet.hasValue()
            ? aRet : InteractionContinuation::queryInterface( rType );
}

uno::Sequence< sal_Int8 > SAL_CALL InteractionSupplyName::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

uno::Sequence< uno::Type > SAL_CALL InteractionSupplyName::getTypes()
{
    static cppu::OTypeCollection s_aCollection(
                cppu::UnoType<lang::XTypeProvider>::get(),
                cppu::UnoType<ucb::XInteractionSupplyName>::get() );

    return s_aCollection.getTypes();
}

void SAL_CALL InteractionSupplyName::select()
{
    recordSelection();
}

void SAL_CALL
InteractionSupplyName::setName( const OUString& Name )
{
    m_aName = Name;
}

SimpleNameClashResolveRequest::~SimpleNameClashResolveRequest() {}

SimpleNameClashResolveRequest::SimpleNameClashResolveRequest(
                                    const OUString & rTargetFolderURL,
                                    const OUString & rClashingName )
{
    // Fill request...
    ucb::NameClashResolveRequest aRequest;
//    aRequest.Message        = // OUString
//    aRequest.Context        = // XInterface
    aRequest.Classification  = task::InteractionClassification_QUERY;
    aRequest.TargetFolderURL = rTargetFolderURL;
    aRequest.ClashingName    = rClashingName;
    aRequest.ProposedNewName = OUString();

    setRequest( uno::makeAny( aRequest ) );

    // Fill continuations...
    m_xNameSupplier = new InteractionSupplyName( this );

    uno::Sequence< uno::Reference< task::XInteractionContinuation > >
                            aContinuations( 3 );
    aContinuations[ 0 ] = new InteractionAbort( this );
    aContinuations[ 1 ] = m_xNameSupplier.get();
    aContinuations[ 2 ] = new InteractionReplaceExistingData( this );

    setContinuations( aContinuations );
}

OUString const & SimpleNameClashResolveRequest::getNewName() const
{
    return m_xNameSupplier->getName();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
