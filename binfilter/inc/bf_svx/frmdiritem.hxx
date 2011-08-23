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
#ifndef _SVX_FRMDIRITEM_HXX
#define _SVX_FRMDIRITEM_HXX

// include ---------------------------------------------------------------

#include <bf_svtools/intitem.hxx>
#include <bf_svx/frmdir.hxx>
#include <bf_svx/svxids.hrc>
namespace binfilter {

// class SvxFrameDirectionItem ----------------------------------------------

/* [Description]

    This item defines a frame direction, which place the content inside
    a frame. It exist different kind of directions which are used to the
    layout text for Western, CJK and CTL languages.
*/

class SvxFrameDirectionItem : public SfxUInt16Item
{
public:
    TYPEINFO();

    SvxFrameDirectionItem( SvxFrameDirection nValue = FRMDIR_HORI_LEFT_TOP,
                            USHORT nWhich = ITEMID_FRAMEDIR );
    virtual ~SvxFrameDirectionItem();

    virtual SfxPoolItem*	Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*	Create(SvStream &, USHORT) const;
    virtual SvStream& 		Store(SvStream & rStrm, USHORT nIVer) const;
    virtual USHORT			GetVersion( USHORT nFileVersion ) const;
    virtual int 			operator==( const SfxPoolItem& ) const;


    virtual bool PutValue( const ::com::sun::star::uno::Any& rVal,
                                    BYTE nMemberId );
    virtual bool QueryValue( ::com::sun::star::uno::Any& rVal,
                                BYTE nMemberId ) const;

    inline SvxFrameDirectionItem& operator=( const SvxFrameDirectionItem& rItem )
    {
        SetValue( rItem.GetValue() );
        return *this;
    }
};

}//end of namespace binfilter
#endif // #ifndef _SVX_FRMDIRITEM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
