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
#ifndef _SXCIKITM_HXX
#define _SXCIKITM_HXX

#ifndef _SVDDEF_HXX //autogen
#include <bf_svx/svddef.hxx>
#endif

#ifndef _SFXENUMITEM_HXX //autogen
#include <bf_svtools/eitem.hxx>
#endif
namespace binfilter {

enum SdrCircKind {SDRCIRC_FULL,
                  SDRCIRC_SECT,
                  SDRCIRC_CUT,
                  SDRCIRC_ARC};

class SdrCircKindItem: public SfxEnumItem {
public:
    TYPEINFO();
    SdrCircKindItem(SdrCircKind eKind=SDRCIRC_FULL): SfxEnumItem(SDRATTR_CIRCKIND,eKind) {}
    SdrCircKindItem(SvStream& rIn)                 : SfxEnumItem(SDRATTR_CIRCKIND,rIn)   {}
    virtual SfxPoolItem* Clone(SfxItemPool* pPool=NULL) const;
    virtual SfxPoolItem* Create(SvStream& rIn, USHORT nVer) const;
    virtual USHORT       GetValueCount() const; // { return 4; }
            SdrCircKind  GetValue() const      { return (SdrCircKind)SfxEnumItem::GetValue(); }

    virtual	sal_Bool			 PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );

};

}//end of namespace binfilter
#endif
