/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: writingmodeitem.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: vg $ $Date: 2006-11-21 17:11:29 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

// include ---------------------------------------------------------------


#ifndef _SVX_WRITINGMODEITEM_HXX
#include "writingmodeitem.hxx"
#endif

#ifndef _SVX_DIALMGR_HXX
#include "dialmgr.hxx"
#endif

#ifndef _SVXITEMS_HRC
#include "svxitems.hrc"
#endif

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;

// class SvxWritingModeItem -------------------------------------------------

TYPEINIT1_AUTOFACTORY(SvxWritingModeItem, SfxUInt16Item);

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
    DBG_ERROR("SvxWritingModeItem should not be streamed!");
    return NULL;
}

SvStream& SvxWritingModeItem::Store( SvStream & rStrm, USHORT  ) const
{
    DBG_ERROR("SvxWritingModeItem should not be streamed!");
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
        rText = SVX_RESSTR( RID_SVXITEMS_FRMDIR_BEGIN + GetValue() );
        break;

    default:
        eRet = SFX_ITEM_PRESENTATION_NONE;
    }
    return eRet;
}

sal_Bool SvxWritingModeItem::PutValue( const com::sun::star::uno::Any& rVal, BYTE )
{
    sal_Int32 nVal = 0;
    sal_Bool bRet = ( rVal >>= nVal );

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

sal_Bool SvxWritingModeItem::QueryValue( com::sun::star::uno::Any& rVal,
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
