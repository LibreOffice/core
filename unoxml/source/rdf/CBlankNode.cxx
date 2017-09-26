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

#include "CNodes.hxx"

#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/rdf/XBlankNode.hpp>

#include <com/sun/star/lang/IllegalArgumentException.hpp>


/// anonymous implementation namespace
namespace {

class CBlankNode:
    public ::cppu::WeakImplHelper<
        css::lang::XServiceInfo,
        css::lang::XInitialization,
        css::rdf::XBlankNode>
{
public:
    CBlankNode();

    // css::lang::XServiceInfo:
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString & ServiceName) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // css::lang::XInitialization:
    virtual void SAL_CALL initialize(const css::uno::Sequence< css::uno::Any > & aArguments) override;

    // css::rdf::XNode:
    virtual OUString SAL_CALL getStringValue() override;

private:
    CBlankNode(CBlankNode const&) = delete;
    CBlankNode& operator=(CBlankNode const&) = delete;

    OUString m_NodeID;
};

CBlankNode::CBlankNode() :
    m_NodeID()
{}

// com.sun.star.uno.XServiceInfo:
OUString SAL_CALL CBlankNode::getImplementationName()
{
    return comp_CBlankNode::_getImplementationName();
}

sal_Bool SAL_CALL CBlankNode::supportsService(OUString const & serviceName)
{
    return cppu::supportsService(this, serviceName);
}

css::uno::Sequence< OUString > SAL_CALL CBlankNode::getSupportedServiceNames()
{
    return comp_CBlankNode::_getSupportedServiceNames();
}

// css::lang::XInitialization:
void SAL_CALL CBlankNode::initialize(const css::uno::Sequence< css::uno::Any > & aArguments)
{
    if (aArguments.getLength() != 1) {
        throw css::lang::IllegalArgumentException(
            "CBlankNode::initialize: must give exactly 1 argument", *this, 1);
    }

    OUString arg;
    if (!(aArguments[0] >>= arg)) {
        throw css::lang::IllegalArgumentException(
            "CBlankNode::initialize: argument must be string", *this, 0);
    }

    //FIXME: what is legal?
    if (arg.isEmpty()) {
        throw css::lang::IllegalArgumentException(
            "CBlankNode::initialize: argument is not valid blank node ID", *this, 0);
    }
    m_NodeID = arg;
}

// css::rdf::XNode:
OUString SAL_CALL CBlankNode::getStringValue()
{
    return m_NodeID;
}

} // closing anonymous implementation namespace


// component helper namespace
namespace comp_CBlankNode {

OUString SAL_CALL _getImplementationName() {
    return OUString( "CBlankNode");
}

css::uno::Sequence< OUString > SAL_CALL _getSupportedServiceNames()
{
    css::uno::Sequence< OUString > s { "com.sun.star.rdf.BlankNode" };
    return s;
}

css::uno::Reference< css::uno::XInterface > SAL_CALL _create(
    const css::uno::Reference< css::uno::XComponentContext > & )
{
    return static_cast< ::cppu::OWeakObject * >(new CBlankNode);
}

} // closing component helper namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
