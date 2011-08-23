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
#ifndef _FMTTSPLT_HXX
#define _FMTTSPLT_HXX

#ifndef _SFXENUMITEM_HXX //autogen
#include <bf_svtools/eitem.hxx>
#endif
#ifndef _FORMAT_HXX //autogen
#include <format.hxx>
#endif
class IntlWrapper; 
namespace binfilter {


class SwFmtLayoutSplit : public SfxBoolItem
{
public:
    SwFmtLayoutSplit( BOOL bSplit = TRUE ) : SfxBoolItem( RES_LAYOUT_SPLIT, bSplit ) {}

    // "pure virtual Methoden" vom SfxPoolItem
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*    Create(SvStream &, USHORT) const;
    virtual SvStream&       Store(SvStream &, USHORT nItemVersion ) const;
    virtual USHORT			 GetVersion( USHORT nFFVer ) const;
};

#if !(defined(MACOSX) && ( __GNUC__ < 3 ))
// GrP moved to gcc_outl.cxx; revisit with gcc3
inline const SwFmtLayoutSplit &SwAttrSet::GetLayoutSplit(BOOL bInP) const
    { return (const SwFmtLayoutSplit&)Get( RES_LAYOUT_SPLIT,bInP); }

inline const SwFmtLayoutSplit &SwFmt::GetLayoutSplit(BOOL bInP) const
    { return aSet.GetLayoutSplit(bInP); }
#endif

} //namespace binfilter
#endif

