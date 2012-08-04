/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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

// XUnoTunnel & co
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

// XUnoTunnel
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
