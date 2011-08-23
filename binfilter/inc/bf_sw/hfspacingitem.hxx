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
#ifndef _SW_HF_EAT_SPACINGITEM_HXX
#define _SW_HF_EAT_SPACINGITEM_HXX


#ifndef _SFXENUMITEM_HXX //autogen
#include <bf_svtools/eitem.hxx>
#endif
#ifndef _FORMAT_HXX //autogen
#include <format.hxx>
#endif
class IntlWrapper; 
namespace binfilter {



class SwHeaderAndFooterEatSpacingItem : public SfxBoolItem
{
public:
    SwHeaderAndFooterEatSpacingItem( USHORT nId = RES_HEADER_FOOTER_EAT_SPACING,
                                     BOOL bPrt = FALSE ) : SfxBoolItem( nId, bPrt ) {}

    // "pure virtual Methoden" vom SfxPoolItem
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*    Create(SvStream &, USHORT) const;
    virtual SvStream&		Store(SvStream &, USHORT nItemVersion ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText,
                                    const ::IntlWrapper*    pIntl = 0 ) const;
    virtual USHORT			 GetVersion( USHORT nFFVer ) const;
};

inline const SwHeaderAndFooterEatSpacingItem &SwAttrSet::GetHeaderAndFooterEatSpacing(BOOL bInP) const
    { return (const SwHeaderAndFooterEatSpacingItem&)Get( RES_HEADER_FOOTER_EAT_SPACING,bInP); }

inline const SwHeaderAndFooterEatSpacingItem &SwFmt::GetHeaderAndFooterEatSpacing(BOOL bInP) const
    { return aSet.GetHeaderAndFooterEatSpacing(bInP); }

} //namespace binfilter
#endif

