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
#include <editeng/frmdir.hxx>
#include <editeng/eerdll.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;


SvxWritingModeItem::SvxWritingModeItem( WritingMode eValue, TypedWhichId<SvxWritingModeItem> _nWhich )
    : SfxUInt16Item( _nWhich, static_cast<sal_uInt16>(eValue) )
{
}

SvxWritingModeItem::~SvxWritingModeItem()
{
}

SvxWritingModeItem* SvxWritingModeItem::Clone( SfxItemPool * ) const
{
    return new SvxWritingModeItem( *this );
}

bool SvxWritingModeItem::GetPresentation( SfxItemPresentation /*ePres*/,
        MapUnit /*eCoreMetric*/,
        MapUnit /*ePresMetric*/,
        OUString &rText,
        const IntlWrapper& ) const
{
    rText = EditResId(getFrmDirResId(static_cast<int>(GetValue())));
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
            nVal = static_cast<sal_Int32>(eMode);
        }
    }

    if( bRet )
    {
        switch( static_cast<WritingMode>(nVal) )
        {
            case WritingMode_LR_TB:
            case WritingMode_RL_TB:
            case WritingMode_TB_RL:
                SetValue( static_cast<sal_uInt16>(nVal) );
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
    rVal <<= GetValue();
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
