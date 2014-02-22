/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <string.h>

#include <osl/mutex.hxx>
#include <comphelper/servicehelper.hxx>

#include "StyleMap.hxx"

using namespace ::osl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;


StyleMap::StyleMap()
{
}



StyleMap::~StyleMap()
{
}

namespace
{
    class theStyleMapUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theStyleMapUnoTunnelId> {};
}


const Sequence< sal_Int8 > & StyleMap::getUnoTunnelId() throw()
{
    return theStyleMapUnoTunnelId::get().getSeq();
}

StyleMap* StyleMap::getImplementation( Reference< XInterface > xInt ) throw()
{
    Reference< XUnoTunnel > xUT( xInt, UNO_QUERY );
    if( xUT.is() )
        return reinterpret_cast<StyleMap *>(
                xUT->getSomething( StyleMap::getUnoTunnelId() ) );
    else
        return 0;
}


sal_Int64 SAL_CALL StyleMap::getSomething(
        const Sequence< sal_Int8 >& rId )
    throw( RuntimeException )
{
    if( rId.getLength() == 16 &&
        0 == memcmp( getUnoTunnelId().getConstArray(),
                                             rId.getConstArray(), 16 ) )
    {
        return reinterpret_cast<sal_Int64>( this );
    }
    return 0;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
