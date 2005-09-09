/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fmtrowsplt.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 01:52:46 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _FMTROWSPLT_HXX
#define _FMTROWSPLT_HXX

#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif

#ifndef INCLUDED_SWDLLAPI_H
#include "swdllapi.h"
#endif
#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif
#ifndef _FORMAT_HXX //autogen
#include <format.hxx>
#endif

class IntlWrapper;

class SW_DLLPUBLIC SwFmtRowSplit : public SfxBoolItem
{
public:
    SwFmtRowSplit( BOOL bSplit = TRUE ) : SfxBoolItem( RES_ROW_SPLIT, bSplit ) {}

    // "pure virtual Methoden" vom SfxPoolItem
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText,
                                    const IntlWrapper*    pIntl = 0 ) const;
};

inline const SwFmtRowSplit &SwAttrSet::GetRowSplit(BOOL bInP) const
    { return (const SwFmtRowSplit&)Get( RES_ROW_SPLIT,bInP); }

inline const SwFmtRowSplit &SwFmt::GetRowSplit(BOOL bInP) const
    { return aSet.GetRowSplit(bInP); }

#endif

