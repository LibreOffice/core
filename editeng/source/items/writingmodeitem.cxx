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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_editeng.hxx"

// include ---------------------------------------------------------------


#include <editeng/writingmodeitem.hxx>
#include <editeng/eerdll.hxx>
#include <editeng/editrids.hrc>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;


TYPEINIT1_FACTORY(SvxWritingModeItem, SfxUInt16Item, new SvxWritingModeItem(com::sun::star::text::WritingMode_LR_TB, 0));

SvxWritingModeItem::SvxWritingModeItem( WritingMode eValue, USHORT _nWhich )
    : SfxUInt16Item( _nWhich, (sal_uInt16)eValue )
{
}

SvxWritingModeItem::~SvxWritingModeItem()
{
}

int SvxWritingModeItem::operator==( const SfxPoolItem& rCmp ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rCmp), "unequal types" );

    return GetValue() == ((SvxWritingModeItem&)rCmp).GetValue();
}

SfxPoolItem* SvxWritingModeItem::Clone( SfxItemPool * ) const
{
    return new SvxWritingModeItem( *this );
}

SfxPoolItem* SvxWritingModeItem::Create( SvStream & , USHORT  ) const
{
    OSL_FAIL("SvxWritingModeItem should not be streamed!");
    return NULL;
}

SvStream& SvxWritingModeItem::Store( SvStream & rStrm, USHORT  ) const
{
    OSL_FAIL("SvxWritingModeItem should not be streamed!");
    return rStrm;
}

USHORT SvxWritingModeItem::GetVersion( USHORT /*nFVer*/ ) const
{
    return USHRT_MAX;
}

SfxItemPresentation SvxWritingModeItem::GetPresentation( SfxItemPresentation ePres,
        SfxMapUnit /*eCoreMetric*/,
        SfxMapUnit /*ePresMetric*/,
        String &rText,
        const IntlWrapper *  ) const
{
    SfxItemPresentation eRet = ePres;
    switch( ePres )
    {
    case SFX_ITEM_PRESENTATION_NONE:
        rText.Erase();
        break;

    case SFX_ITEM_PRESENTATION_NAMELESS:
    case SFX_ITEM_PRESENTATION_COMPLETE:
        rText = String( EditResId( RID_SVXITEMS_FRMDIR_BEGIN + GetValue() ) );
        break;

    default:
        eRet = SFX_ITEM_PRESENTATION_NONE;
    }
    return eRet;
}

bool SvxWritingModeItem::PutValue( const com::sun::star::uno::Any& rVal, BYTE )
{
    sal_Int32 nVal = 0;
    bool bRet = ( rVal >>= nVal );

    if( !bRet )
    {
        WritingMode eMode;
        bRet = rVal >>= eMode;

        if( bRet )
        {
            nVal = (sal_Int32)eMode;
        }
    }

    if( bRet )
    {
        switch( nVal )
        {
            case WritingMode_LR_TB:
            case WritingMode_RL_TB:
            case WritingMode_TB_RL:
                SetValue( (sal_uInt16)nVal );
                bRet = true;
                break;
            default:
                bRet = false;
                break;
        }
    }

    return bRet;
}

bool SvxWritingModeItem::QueryValue( com::sun::star::uno::Any& rVal,
                                            BYTE ) const
{
    rVal <<= (WritingMode)GetValue();
    return true;
}

SvxWritingModeItem& SvxWritingModeItem::operator=( const SvxWritingModeItem& rItem )
{
    SetValue( rItem.GetValue() );
    return *this;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
