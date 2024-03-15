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
#pragma once

#include <com/sun/star/util/XCloneable.hpp>
#include <rtl/ref.hxx>
#include <iterator>
#include <vector>

namespace chart::CloneHelper
{

/// functor that clones a UNO-Reference
template< class Interface >
    struct CreateRefClone
{
    css::uno::Reference<Interface> operator() ( const css::uno::Reference<Interface> & xOther )
    {
        css::uno::Reference<Interface> xResult;
        css::uno::Reference< css::util::XCloneable >
              xCloneable( xOther, css::uno::UNO_QUERY );
        if( xCloneable.is())
            xResult.set( xCloneable->createClone(), css::uno::UNO_QUERY );

        return xResult;
    }
};

/// clones a vector of UNO-References
template< class Interface >
    void CloneRefVector(
        const std::vector< css::uno::Reference< Interface > > & rSource,
        std::vector< css::uno::Reference< Interface > > & rDestination )
{
    std::transform( rSource.begin(), rSource.end(),
                      std::back_inserter( rDestination ),
                      CreateRefClone< Interface >());
}

template< class T >
    void CloneRefVector(
        const std::vector< rtl::Reference< T > > & rSource,
        std::vector< rtl::Reference< T > > & rDestination )
{
    for (const auto & rSourceItem : rSource)
        rDestination.push_back(static_cast<T*>(rSourceItem->createClone().get()));
}

/// clones a UNO-sequence of UNO-References
template< class Interface >
    void CloneRefSequence(
        const css::uno::Sequence< css::uno::Reference<Interface> > & rSource,
        css::uno::Sequence< css::uno::Reference<Interface> > & rDestination )
{
    rDestination.realloc( rSource.getLength());
    std::transform( rSource.begin(), rSource.end(),
                      rDestination.getArray(),
                      CreateRefClone< Interface >());
}

} //  namespace chart::CloneHelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
