/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fmteiro.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 01:48:55 $
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
#ifndef _FMTEIRO_HXX
#define _FMTEIRO_HXX


#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif
#ifndef _FORMAT_HXX //autogen
#include <format.hxx>
#endif
#ifndef INCLUDED_SWDLLAPI_H
#include "swdllapi.h"
#endif

class IntlWrapper;

class SW_DLLPUBLIC SwFmtEditInReadonly : public SfxBoolItem
{
public:
    SwFmtEditInReadonly( USHORT nId = RES_EDIT_IN_READONLY,
                     BOOL bPrt = FALSE ) : SfxBoolItem( nId, bPrt ) {}

    // "pure virtual Methoden" vom SfxPoolItem
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText,
                                    const IntlWrapper*    pIntl = 0 ) const;
};

inline const SwFmtEditInReadonly &SwAttrSet::GetEditInReadonly(BOOL bInP) const
    { return (const SwFmtEditInReadonly&)Get( RES_EDIT_IN_READONLY,bInP); }

inline const SwFmtEditInReadonly &SwFmt::GetEditInReadonly(BOOL bInP) const
    { return aSet.GetEditInReadonly(bInP); }

#endif

