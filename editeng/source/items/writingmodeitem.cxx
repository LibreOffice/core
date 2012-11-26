/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_editeng.hxx"

// include ---------------------------------------------------------------


#include <editeng/writingmodeitem.hxx>
#include <editeng/eerdll.hxx>
#include <editeng/editrids.hrc>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;

// class SvxWritingModeItem -------------------------------------------------

SvxWritingModeItem::SvxWritingModeItem( WritingMode eValue, sal_uInt16 _nWhich )
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

SfxPoolItem* SvxWritingModeItem::Create( SvStream & , sal_uInt16  ) const
{
    DBG_ERROR("SvxWritingModeItem should not be streamed!");
    return NULL;
}

SvStream& SvxWritingModeItem::Store( SvStream & rStrm, sal_uInt16  ) const
{
    DBG_ERROR("SvxWritingModeItem should not be streamed!");
    return rStrm;
}

sal_uInt16 SvxWritingModeItem::GetVersion( sal_uInt16 /*nFVer*/ ) const
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

sal_Bool SvxWritingModeItem::PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 )
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
                                            sal_uInt8 ) const
{
    rVal <<= (WritingMode)GetValue();
    return true;
}

SvxWritingModeItem& SvxWritingModeItem::operator=( const SvxWritingModeItem& rItem )
{
    SetValue( rItem.GetValue() );
    return *this;
}
