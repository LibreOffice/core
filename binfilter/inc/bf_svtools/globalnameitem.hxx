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
#ifndef _GLOBALNAMEITEM_HXX
#define _GLOBALNAMEITEM_HXX

#ifndef INCLUDED_SVTDLLAPI_H
#include "bf_svtools/svtdllapi.h"
#endif

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif
#ifndef _RTTI_HXX
#include <tools/rtti.hxx>
#endif
#ifndef _GLOBNAME_HXX
#include <tools/globname.hxx>
#endif

#ifndef _SFXPOOLITEM_HXX
#include <bf_svtools/poolitem.hxx>
#endif

namespace binfilter
{

// -----------------------------------------------------------------------

class  SfxGlobalNameItem: public SfxPoolItem
{
    SvGlobalName			m_aName;

public:
                            TYPEINFO();
                            SfxGlobalNameItem();
                            ~SfxGlobalNameItem();

    virtual int 			operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*	Clone( SfxItemPool *pPool = 0 ) const;
    SvGlobalName			GetValue() const { return m_aName; }

    virtual	BOOL 			PutValue  ( const ::com::sun::star::uno::Any& rVal,
                                        BYTE nMemberId = 0 );
    virtual	BOOL 			QueryValue( ::com::sun::star::uno::Any& rVal,
                                        BYTE nMemberId = 0 ) const;
};

}

#endif

