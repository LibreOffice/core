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
#ifndef INCLUDED_CHART2_SOURCE_INC_DISPOSEHELPER_HXX
#define INCLUDED_CHART2_SOURCE_INC_DISPOSEHELPER_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/lang/XComponent.hpp>

namespace chart
{
namespace DisposeHelper
{

template< class T >
    void Dispose( const T & xIntf )
{
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent > xComp(
        xIntf, ::com::sun::star::uno::UNO_QUERY );
    if( xComp.is())
        xComp->dispose();
}

template< class Intf >
    void DisposeAndClear( ::com::sun::star::uno::Reference< Intf > & rIntf )
{
    Dispose< ::com::sun::star::uno::Reference< Intf > >( rIntf );
    rIntf.set( 0 );
}

template< class Container >
    void DisposeAllElements( Container & rContainer )
{
    for( const auto& rElem : rContainer )
        Dispose< typename Container::value_type >( rElem );
}

} //  namespace DisposeHelper
} //  namespace chart

// INCLUDED_CHART2_SOURCE_INC_DISPOSEHELPER_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
