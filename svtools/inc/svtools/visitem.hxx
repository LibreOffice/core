/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: visitem.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 19:43:12 $
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

#ifndef _SFXVISIBILITYITEM_HXX
#define _SFXVISIBILITYITEM_HXX

#ifndef INCLUDED_SVLDLLAPI_H
#include "svtools/svldllapi.h"
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _SFXPOOLITEM_HXX
#include <svtools/poolitem.hxx>
#endif
#ifndef _COM_SUN_STAR_FRAME_STATUS_VISIBILITY_HPP_
#include <com/sun/star/frame/status/Visibility.hpp>
#endif

//============================================================================
DBG_NAMEEX_VISIBILITY(SfxVisibilityItem, SVL_DLLPUBLIC)

class SVL_DLLPUBLIC SfxVisibilityItem: public SfxPoolItem
{
    ::com::sun::star::frame::status::Visibility m_nValue;

public:
    TYPEINFO();

    SfxVisibilityItem(USHORT which = 0, sal_Bool bVisible = sal_True):
        SfxPoolItem(which)
    {
        m_nValue.bVisible = bVisible;
        DBG_CTOR(SfxVisibilityItem, 0);
    }

    SfxVisibilityItem(USHORT which, SvStream & rStream);

    SfxVisibilityItem(const SfxVisibilityItem & rItem):
        SfxPoolItem(rItem), m_nValue(rItem.m_nValue)
    { DBG_CTOR(SfxVisibilityItem, 0); }

    virtual ~SfxVisibilityItem() { DBG_DTOR(SfxVisibilityItem, 0); }

    virtual int operator ==(const SfxPoolItem & rItem) const;

    using SfxPoolItem::Compare;
    virtual int Compare(const SfxPoolItem & rWith) const;

    virtual SfxItemPresentation GetPresentation(SfxItemPresentation,
                                                SfxMapUnit, SfxMapUnit,
                                                XubString & rText,
                                                const IntlWrapper * = 0)
        const;

    virtual BOOL QueryValue( com::sun::star::uno::Any& rVal,
                             BYTE nMemberId = 0 ) const;

    virtual BOOL PutValue( const com::sun::star::uno::Any& rVal,
                           BYTE nMemberId = 0 );

    virtual SfxPoolItem * Create(SvStream & rStream, USHORT) const;

    virtual SvStream & Store(SvStream & rStream, USHORT) const;

    virtual SfxPoolItem * Clone(SfxItemPool * = 0) const;

    virtual USHORT GetValueCount() const;

    virtual UniString GetValueTextByVal(BOOL bTheValue) const;

    BOOL GetValue() const { return m_nValue.bVisible; }

    void SetValue(BOOL bVisible) { m_nValue.bVisible = bVisible; }
};

#endif // _SFXVISIBILITYITEM_HXX
