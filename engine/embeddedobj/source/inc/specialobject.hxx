/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#pragma once

#include <com/sun/star/awt/Size.hpp>

#include "commonembobj.hxx"

class OSpecialEmbeddedObject : public OCommonEmbeddedObject
{
private:
    css::awt::Size         maSize;
public:
    OSpecialEmbeddedObject(
        const css::uno::Reference< css::uno::XComponentContext >& rxContext,
        const cpo::uno::Sequence< css::beans::NamedValue >& aObjectProps );

    // XInterface
    virtual cpo::uno::Any queryInterface( const cpo::uno::Type& rType ) override ;

    // XVisualObject
    virtual css::embed::VisualRepresentation getPreferredVisualRepresentation( ::sal_Int64 nAspect ) override;

    virtual void setVisualAreaSize( sal_Int64 nAspect, const css::awt::Size& aSize ) override;

    virtual css::awt::Size getVisualAreaSize( sal_Int64 nAspect ) override;

    virtual sal_Int32 getMapUnit( sal_Int64 nAspect ) override;

    virtual void changeState( sal_Int32 nNewState ) override;

    virtual void doVerb( sal_Int32 nVerbID ) override;

// XCommonEmbedPersist

    virtual void reload(
                const cpo::uno::Sequence< css::beans::PropertyValue >& lArguments,
                const cpo::uno::Sequence< css::beans::PropertyValue >& lObjArgs ) override;

    // XServiceInfo
    OUString getImplementationName() override;
    bool supportsService( const OUString& ServiceName ) override;
    cpo::uno::Sequence< OUString > getSupportedServiceNames() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
