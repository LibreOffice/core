/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: eitem.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 19:17:14 $
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

#ifndef _SFXENUMITEM_HXX
#define _SFXENUMITEM_HXX

#ifndef INCLUDED_SVTDLLAPI_H
#include "svtools/svtdllapi.h"
#endif

#ifndef _SVTOOLS_CENUMITM_HXX
#include <svtools/cenumitm.hxx>
#endif

//============================================================================
class SVT_DLLPUBLIC SfxEnumItem: public CntEnumItem
{
protected:
    SfxEnumItem(USHORT which = 0, USHORT nValue = 0):
        CntEnumItem(which, nValue) {}

    SfxEnumItem(USHORT which, SvStream & rStream):
        CntEnumItem(which, rStream) {}

public:
    TYPEINFO();

};

//============================================================================
class SVT_DLLPUBLIC SfxBoolItem: public CntBoolItem
{
public:
    TYPEINFO();

    SfxBoolItem(USHORT which = 0, BOOL bValue = FALSE):
        CntBoolItem(which, bValue) {}

    SfxBoolItem(USHORT which, SvStream & rStream):
        CntBoolItem(which, rStream) {}

    virtual SfxPoolItem * Create(SvStream & rStream, USHORT) const
    { return new SfxBoolItem(Which(), rStream); }

    virtual SfxPoolItem * Clone(SfxItemPool * = 0) const
    { return new SfxBoolItem(*this); }
};

#endif //  _SFXENUMITEM_HXX

