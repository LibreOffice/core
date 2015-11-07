/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <editeng/writingmodeitem.hxx>
#include <editeng/eerdll.hxx>
#include <editeng/editrids.hrc>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;


TYPEINIT1_FACTORY(SvxWritingModeItem, SfxUInt16Item, new SvxWritingModeItem(css::text::WritingMode_LR_TB, 0));

SfxPoolItem* SvxWritingModeItem::CreateDefault() { return new  SvxWritingModeItem(css::text::WritingMode_LR_TB, 0);}

SvxWritingModeItem::SvxWritingModeItem( WritingMode eValue, sal_uInt16 _nWhich )
    : SfxUInt16Item( _nWhich, (sal_uInt16)eValue )
{
}

SvxWritingModeItem::~SvxWritingModeItem()
{
}

bool SvxWritingModeItem::operator==( const SfxPoolItem& rCmp ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rCmp), "unequal types" );

    return GetValue() == static_cast<const SvxWritingModeItem&>(rCmp).GetValue();
}

SfxPoolItem* SvxWritingModeItem::Clone( SfxItemPool * ) const
{
    return new SvxWritingModeItem( *this );
}

SfxPoolItem* SvxWritingModeItem::Create( SvStream & , sal_uInt16  ) const
{
    OSL_FAIL("SvxWritingModeItem should not be streamed!");
    return NULL;
}

SvStream& SvxWritingModeItem::Store( SvStream & rStrm, sal_uInt16  ) const
{
    OSL_FAIL("SvxWritingModeItem should not be streamed!");
    return rStrm;
}

sal_uInt16 SvxWritingModeItem::GetVersion( sal_uInt16 /*nFVer*/ ) const
{
    return USHRT_MAX;
}

bool SvxWritingModeItem::GetPresentation( SfxItemPresentation /*ePres*/,
        SfxMapUnit /*eCoreMetric*/,
        SfxMapUnit /*ePresMetric*/,
        OUString &rText,
        const IntlWrapper *  ) const
{
    rText = EE_RESSTR(RID_SVXITEMS_FRMDIR_BEGIN + GetValue());
    return true;
}

bool SvxWritingModeItem::PutValue( const css::uno::Any& rVal, sal_uInt8 )
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

bool SvxWritingModeItem::QueryValue( css::uno::Any& rVal,
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
