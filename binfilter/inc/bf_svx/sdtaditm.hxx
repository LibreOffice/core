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
#ifndef _SDTADITM_HXX
#define _SDTADITM_HXX

#include <bf_svtools/eitem.hxx>

#include <bf_svx/svddef.hxx>
namespace binfilter {

//------------------------------
// class SdrTextAniDirectionItem
//------------------------------

enum SdrTextAniDirection {SDRTEXTANI_LEFT,
                          SDRTEXTANI_RIGHT,
                          SDRTEXTANI_UP,
                          SDRTEXTANI_DOWN};

class SdrTextAniDirectionItem: public SfxEnumItem {
public:
    TYPEINFO();
    SdrTextAniDirectionItem(SdrTextAniDirection eDir=SDRTEXTANI_LEFT): SfxEnumItem(SDRATTR_TEXT_ANIDIRECTION,eDir) {}
    SdrTextAniDirectionItem(SvStream& rIn)                           : SfxEnumItem(SDRATTR_TEXT_ANIDIRECTION,rIn)  {}
    virtual SfxPoolItem*   Clone(SfxItemPool* pPool=NULL) const;
    virtual SfxPoolItem*   Create(SvStream& rIn, USHORT nVer) const;
    virtual USHORT         GetValueCount() const; // { return 4; }
    SdrTextAniDirection GetValue() const      { return (SdrTextAniDirection)SfxEnumItem::GetValue(); }

    virtual	bool           QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual	bool           PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );


};

}//end of namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
