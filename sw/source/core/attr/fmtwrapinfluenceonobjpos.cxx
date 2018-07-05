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

#include <fmtwrapinfluenceonobjpos.hxx>
#include <unomid.h>
#include <osl/diagnose.h>
#include <libxml/xmlwriter.h>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;


SwFormatWrapInfluenceOnObjPos::SwFormatWrapInfluenceOnObjPos( sal_Int16 _nWrapInfluenceOnPosition )
    : SfxPoolItem( RES_WRAP_INFLUENCE_ON_OBJPOS ),
    mnWrapInfluenceOnPosition( _nWrapInfluenceOnPosition )
{
}

SwFormatWrapInfluenceOnObjPos::~SwFormatWrapInfluenceOnObjPos()
{
}

bool SwFormatWrapInfluenceOnObjPos::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));
    return ( mnWrapInfluenceOnPosition ==
                    static_cast<const SwFormatWrapInfluenceOnObjPos&>(rAttr).
                                                GetWrapInfluenceOnObjPos() );
}

SfxPoolItem* SwFormatWrapInfluenceOnObjPos::Clone( SfxItemPool * ) const
{
    return new SwFormatWrapInfluenceOnObjPos(*this);
}

bool SwFormatWrapInfluenceOnObjPos::QueryValue( Any& rVal, sal_uInt8 nMemberId ) const
{
    nMemberId &= ~CONVERT_TWIPS;
    bool bRet = true;
    if( nMemberId == MID_WRAP_INFLUENCE )
    {
        rVal <<= GetWrapInfluenceOnObjPos();
    }
    else
    {
        OSL_FAIL( "<SwFormatWrapInfluenceOnObjPos::QueryValue()> - unknown MemberId" );
        bRet = false;
    }
    return bRet;
}

bool SwFormatWrapInfluenceOnObjPos::PutValue( const Any& rVal, sal_uInt8 nMemberId )
{
    nMemberId &= ~CONVERT_TWIPS;
    bool bRet = false;

    if( nMemberId == MID_WRAP_INFLUENCE )
    {
        sal_Int16 nNewWrapInfluence = 0;
        rVal >>= nNewWrapInfluence;
        // #i35017# - constant names have changed and <ITERATIVE> has been added
        if ( nNewWrapInfluence == text::WrapInfluenceOnPosition::ONCE_SUCCESSIVE ||
             nNewWrapInfluence == text::WrapInfluenceOnPosition::ONCE_CONCURRENT ||
             nNewWrapInfluence == text::WrapInfluenceOnPosition::ITERATIVE )
        {
            SetWrapInfluenceOnObjPos( nNewWrapInfluence );
            bRet = true;
        }
        else
        {
            OSL_FAIL( "<SwFormatWrapInfluenceOnObjPos::PutValue(..)> - invalid attribute value" );
        }
    }
    else
    {
        OSL_FAIL( "<SwFormatWrapInfluenceOnObjPos::PutValue(..)> - unknown MemberId" );
    }
    return bRet;
}

void SwFormatWrapInfluenceOnObjPos::SetWrapInfluenceOnObjPos( sal_Int16 _nWrapInfluenceOnPosition )
{
    // #i35017# - constant names have changed and consider new value <ITERATIVE>
    if ( _nWrapInfluenceOnPosition == text::WrapInfluenceOnPosition::ONCE_SUCCESSIVE ||
         _nWrapInfluenceOnPosition == text::WrapInfluenceOnPosition::ONCE_CONCURRENT ||
         _nWrapInfluenceOnPosition == text::WrapInfluenceOnPosition::ITERATIVE )
    {
        mnWrapInfluenceOnPosition = _nWrapInfluenceOnPosition;
    }
    else
    {
        OSL_FAIL( "<SwFormatWrapInfluenceOnObjPos::SetWrapInfluenceOnObjPos(..)> - invalid attribute value" );
    }
}

// #i35017# - add parameter <_bIterativeAsOnceConcurrent> to control, if
// value <ITERATIVE> has to be treated as <ONCE_CONCURRENT>
sal_Int16 SwFormatWrapInfluenceOnObjPos::GetWrapInfluenceOnObjPos(
                                const bool _bIterativeAsOnceConcurrent ) const
{
    sal_Int16 nWrapInfluenceOnPosition( mnWrapInfluenceOnPosition );

    if ( _bIterativeAsOnceConcurrent &&
         nWrapInfluenceOnPosition == text::WrapInfluenceOnPosition::ITERATIVE )
    {
        nWrapInfluenceOnPosition = text::WrapInfluenceOnPosition::ONCE_CONCURRENT;
    }

    return nWrapInfluenceOnPosition;
}

void SwFormatWrapInfluenceOnObjPos::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("SwFormatWrapInfluenceOnObjPos"));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"), BAD_CAST(OString::number(Which()).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("nWrapInfluenceOnPosition"), BAD_CAST(OString::number(mnWrapInfluenceOnPosition).getStr()));
    xmlTextWriterEndElement(pWriter);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
