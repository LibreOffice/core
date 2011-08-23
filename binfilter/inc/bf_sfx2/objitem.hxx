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
#ifndef _SFX_OBJITEM_HXX
#define _SFX_OBJITEM_HXX

#include <bf_svtools/poolitem.hxx>

#include <bf_sfx2/shell.hxx>
namespace binfilter {

//------------------------------------------------------------------------

class SfxObjectItem: public SfxPoolItem
{
    SfxShell*       		 _pSh;

public:
                             TYPEINFO();
                             SfxObjectItem( USHORT nWhich=0, SfxShell *pSh=0 );

    virtual int 			 operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*	 Clone( SfxItemPool *pPool = 0 ) const;

    SfxShell*                GetShell() const
                             { return _pSh; }
};

}//end of namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
