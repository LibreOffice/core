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


#include <sfx2/sfxsids.hrc>
#include <sorgitm.hxx>
#include <osl/diagnose.h>
#include <libxml/xmlwriter.h>

SfxPoolItem* SfxScriptOrganizerItem::CreateDefault() { return new SfxScriptOrganizerItem; }


SfxScriptOrganizerItem::SfxScriptOrganizerItem() : SfxPoolItem(0)
{
}

SfxScriptOrganizerItem* SfxScriptOrganizerItem::Clone( SfxItemPool * ) const
{
    return new SfxScriptOrganizerItem( *this );
}

bool SfxScriptOrganizerItem::operator==( const SfxPoolItem& rItem) const
{
    assert(SfxPoolItem::operator==(rItem));
    return aLanguage == static_cast<const SfxScriptOrganizerItem &>(rItem).aLanguage;
}


bool SfxScriptOrganizerItem::QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    OUString aValue;
    nMemberId &= ~CONVERT_TWIPS;
    switch ( nMemberId )
    {
        case 0:
        case MID_SCRIPT_ORGANIZER_LANGUAGE:
            aValue = aLanguage;
            break;
        default:
            OSL_FAIL("Wrong MemberId!");
               return false;
     }

    rVal <<= aValue;

    return true;
}

bool SfxScriptOrganizerItem::PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId )
{
    OUString aValue;
    bool bRet = false;
    nMemberId &= ~CONVERT_TWIPS;
    switch ( nMemberId )
    {
        case 0:
        case MID_SCRIPT_ORGANIZER_LANGUAGE:
            bRet = (rVal >>= aValue);
            if ( bRet )
                aLanguage = aValue;
            break;
        default:
            OSL_FAIL("Wrong MemberId!");
            return false;
    }

    return bRet;
}

// virtual
bool SfxScriptOrganizerItem::GetPresentation(SfxItemPresentation, MapUnit,
                                        MapUnit, OUString & rText,
                                        const IntlWrapper&) const
{
    rText = aLanguage;
    return true;
}

void SfxScriptOrganizerItem::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SfxScriptOrganizerItem"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"), BAD_CAST(OString::number(Which()).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"), BAD_CAST(aLanguage.toUtf8().getStr()));
    (void)xmlTextWriterEndElement(pWriter);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
