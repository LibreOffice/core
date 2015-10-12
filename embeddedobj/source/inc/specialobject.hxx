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

#ifndef INCLUDED_EMBEDDEDOBJ_SOURCE_INC_SPECIALOBJECT_HXX
#define INCLUDED_EMBEDDEDOBJ_SOURCE_INC_SPECIALOBJECT_HXX

#include <com/sun/star/awt/Size.hpp>

#include "commonembobj.hxx"

class OSpecialEmbeddedObject : public OCommonEmbeddedObject
{
private:
    css::awt::Size         maSize;
public:
    OSpecialEmbeddedObject(
        const css::uno::Reference< css::uno::XComponentContext >& rxContext,
        const css::uno::Sequence< css::beans::NamedValue >& aObjectProps );

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& rType )
        throw( css::uno::RuntimeException, std::exception ) override ;

    // XVisualObject
    virtual css::embed::VisualRepresentation SAL_CALL getPreferredVisualRepresentation( ::sal_Int64 nAspect )
        throw ( css::lang::IllegalArgumentException,
                css::embed::WrongStateException,
                css::uno::Exception,
                css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL setVisualAreaSize( sal_Int64 nAspect, const css::awt::Size& aSize )
        throw ( css::lang::IllegalArgumentException,
                css::embed::WrongStateException,
                css::uno::Exception,
                css::uno::RuntimeException, std::exception ) override;

    virtual css::awt::Size SAL_CALL getVisualAreaSize( sal_Int64 nAspect )
        throw ( css::lang::IllegalArgumentException,
                css::embed::WrongStateException,
                css::uno::Exception,
                css::uno::RuntimeException, std::exception ) override;

    virtual sal_Int32 SAL_CALL getMapUnit( sal_Int64 nAspect )
        throw ( css::uno::Exception,
                css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL changeState( sal_Int32 nNewState )
        throw ( css::embed::UnreachableStateException,
                css::embed::WrongStateException,
                css::uno::Exception,
                css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL doVerb( sal_Int32 nVerbID )
        throw ( css::lang::IllegalArgumentException,
                css::embed::WrongStateException,
                css::embed::UnreachableStateException,
                css::uno::Exception,
                css::uno::RuntimeException, std::exception ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
