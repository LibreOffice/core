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

using InteractionSupplyName_BASE = cppu::ImplInheritanceHelper<InteractionContinuation,
                                                               css::ucb::XInteractionSupplyName>;
/**
  * This class implements a standard interaction continuation, namely the
  * interface XInteractionSupplyName. Instances of this class can be passed
  * along with an interaction request to indicate the possibility to
  * supply a new name.
  */
class InteractionSupplyName : public InteractionSupplyName_BASE
{
    OUString m_aName;

public:
    explicit InteractionSupplyName( InteractionRequest * pRequest )
    : InteractionSupplyName_BASE( pRequest ) {}

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

    setRequest( uno::Any( aRequest ) );

    // Fill continuations...
    m_xNameSupplier = new InteractionSupplyName( this );

    setContinuations({ new InteractionAbort(this), m_xNameSupplier,
                       new InteractionReplaceExistingData(this) });
}

OUString const & SimpleNameClashResolveRequest::getNewName() const
{
    return m_xNameSupplier->getName();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
