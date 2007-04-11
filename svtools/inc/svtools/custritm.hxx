/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: custritm.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 19:15:37 $
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

#ifndef _SVTOOLS_CUSTRITM_HXX
#define _SVTOOLS_CUSTRITM_HXX

#ifndef INCLUDED_SVLDLLAPI_H
#include "svtools/svldllapi.h"
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _SFXPOOLITEM_HXX
#include <svtools/poolitem.hxx>
#endif

//============================================================================
DBG_NAMEEX_VISIBILITY(CntUnencodedStringItem, SVL_DLLPUBLIC)

class SVL_DLLPUBLIC CntUnencodedStringItem: public SfxPoolItem
{
    XubString m_aValue;

public:
    TYPEINFO();

    CntUnencodedStringItem(USHORT which = 0): SfxPoolItem(which)
    { DBG_CTOR(CntUnencodedStringItem, 0); }

    CntUnencodedStringItem(USHORT which, const XubString & rTheValue):
        SfxPoolItem(which), m_aValue(rTheValue)
    { DBG_CTOR(CntUnencodedStringItem, 0); }

    CntUnencodedStringItem(const CntUnencodedStringItem & rItem):
        SfxPoolItem(rItem), m_aValue(rItem.m_aValue)
    { DBG_CTOR(CntUnencodedStringItem, 0); }

    virtual ~CntUnencodedStringItem() { DBG_DTOR(CntUnencodedStringItem, 0); }

    virtual int operator ==(const SfxPoolItem & rItem) const;

    virtual int Compare(const SfxPoolItem & rWith) const;

    virtual int Compare(SfxPoolItem const & rWith,
                        IntlWrapper const & rIntlWrapper) const;

    virtual SfxItemPresentation GetPresentation(SfxItemPresentation,
                                                SfxMapUnit, SfxMapUnit,
                                                XubString & rText,
                                                const IntlWrapper * = 0)
        const;

    virtual BOOL QueryValue(com::sun::star::uno::Any& rVal,
                            BYTE nMemberId = 0) const;

    virtual BOOL PutValue(const com::sun::star::uno::Any& rVal,
                          BYTE nMemberId = 0);

    virtual SfxPoolItem * Clone(SfxItemPool * = 0) const;

    const XubString & GetValue() const { return m_aValue; }

    inline void SetValue(const XubString & rTheValue);
};

inline void CntUnencodedStringItem::SetValue(const XubString & rTheValue)
{
    DBG_ASSERT(GetRefCount() == 0,
               "CntUnencodedStringItem::SetValue(): Pooled item");
    m_aValue = rTheValue;
}

#endif //  _SVTOOLS_CUSTRITM_HXX

