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

#ifndef _SDGGAITM_HXX
#define _SDGGAITM_HXX

#include <bf_svtools/intitem.hxx>
#include <bf_svx/svddef.hxx>
namespace binfilter {

//----------------------
// SdrGrafGamma100Item -
//----------------------

class SdrGrafGamma100Item : public SfxUInt32Item
{
public:

                            TYPEINFO();

                            SdrGrafGamma100Item( ULONG nGamma100 = 100 ) : SfxUInt32Item( SDRATTR_GRAFGAMMA, nGamma100 ) {}
                            SdrGrafGamma100Item( SvStream& rIn ) : SfxUInt32Item( SDRATTR_GRAFGAMMA, rIn ) {}

    virtual SfxPoolItem*	Clone( SfxItemPool* pPool = NULL ) const;
    virtual SfxPoolItem*	Create( SvStream& rIn, USHORT nVer ) const;

    virtual	bool            QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual	bool            PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );
};

}//end of namespace binfilter
#endif // _SDGGAITM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
