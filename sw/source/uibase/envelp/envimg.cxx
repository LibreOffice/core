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

#include <hintids.hxx>
#include <comphelper/string.hxx>
#include <tools/stream.hxx>
#include <tools/resid.hxx>
#include <sfx2/app.hxx>
#include <editeng/paperinf.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <unotools/useroptions.hxx>
#include <swmodule.hxx>
#include <swtypes.hxx>
#include <envimg.hxx>

#include <cmdid.h>
#include <envelp.hrc>

#include <unomid.h>

#ifdef WNT
#define NEXTLINE  "\r\n"
#else
#define NEXTLINE  "\n"
#endif

using namespace utl;
using namespace css::uno;

TYPEINIT1_AUTOFACTORY( SwEnvItem, SfxPoolItem );

OUString MakeSender()
{
    SvtUserOptions& rUserOpt = SW_MOD()->GetUserOptions();

    OUString sRet;
    OUString sSenderToken(SW_RES(STR_SENDER_TOKENS));
    sal_Int32 nTokenCount = comphelper::string::getTokenCount(sSenderToken, ';');
    sal_Int32 nSttPos = 0;
    bool bLastLength = true;
    for( sal_Int32 i = 0; i < nTokenCount; i++ )
    {
        OUString sToken = sSenderToken.getToken( 0, ';', nSttPos );
        if (sToken == "COMPANY")
        {
            sal_Int32 nOldLen = sRet.getLength();
            sRet += rUserOpt.GetCompany();
            bLastLength = sRet.getLength() != nOldLen;
        }
        else if (sToken == "CR")
        {
            if(bLastLength)
                sRet += NEXTLINE;
            bLastLength = true;
        }
        else if (sToken == "FIRSTNAME")
            sRet += rUserOpt.GetFirstName();
        else if (sToken == "LASTNAME")
            sRet += rUserOpt.GetLastName();
        else if (sToken == "ADDRESS")
            sRet += rUserOpt.GetStreet();
        else if (sToken == "COUNTRY")
            sRet += rUserOpt.GetCountry();
        else if (sToken == "POSTALCODE")
            sRet += rUserOpt.GetZip();
        else if (sToken == "CITY")
            sRet += rUserOpt.GetCity();
        else if (sToken == "STATEPROV")
            sRet += rUserOpt.GetState();
        else if (!sToken.isEmpty()) //spaces
            sRet += sToken;
    }
    return sRet;
}

SwEnvItem::SwEnvItem() :
    SfxPoolItem(FN_ENVELOP)
{
    aAddrText       = aEmptyOUStr;
    bSend           = true;
    aSendText       = MakeSender();
    lSendFromLeft   = 566; // 1 cm
    lSendFromTop    = 566; // 1 cm
    Size aEnvSz     = SvxPaperInfo::GetPaperSize(PAPER_ENV_C65);
    lWidth          = aEnvSz.Width();
    lHeight         = aEnvSz.Height();
    eAlign          = ENV_HOR_LEFT;
    bPrintFromAbove = true;
    lShiftRight     = 0;
    lShiftDown      = 0;

    lAddrFromLeft   = std::max(lWidth, lHeight) / 2;
    lAddrFromTop    = std::min(lWidth, lHeight) / 2;
}

SwEnvItem::SwEnvItem(const SwEnvItem& rItem) :
    SfxPoolItem(FN_ENVELOP),
    aAddrText      (rItem.aAddrText),
    bSend          (rItem.bSend),
    aSendText      (rItem.aSendText),
    lAddrFromLeft  (rItem.lAddrFromLeft),
    lAddrFromTop   (rItem.lAddrFromTop),
    lSendFromLeft  (rItem.lSendFromLeft),
    lSendFromTop   (rItem.lSendFromTop),
    lWidth         (rItem.lWidth),
    lHeight        (rItem.lHeight),
    eAlign         (rItem.eAlign),
    bPrintFromAbove(rItem.bPrintFromAbove),
    lShiftRight    (rItem.lShiftRight),
    lShiftDown     (rItem.lShiftDown)
{
}

SwEnvItem& SwEnvItem::operator =(const SwEnvItem& rItem)
{
    aAddrText       = rItem.aAddrText;
    bSend           = rItem.bSend;
    aSendText       = rItem.aSendText;
    lSendFromLeft   = rItem.lSendFromLeft;
    lSendFromTop    = rItem.lSendFromTop;
    lAddrFromLeft   = rItem.lAddrFromLeft;
    lAddrFromTop    = rItem.lAddrFromTop;
    lWidth          = rItem.lWidth;
    lHeight         = rItem.lHeight;
    eAlign          = rItem.eAlign;
    bPrintFromAbove = rItem.bPrintFromAbove;
    lShiftRight     = rItem.lShiftRight;
    lShiftDown      = rItem.lShiftDown;
    return *this;
}

bool SwEnvItem::operator ==(const SfxPoolItem& rItem) const
{
    const SwEnvItem& rEnv = static_cast<const SwEnvItem&>( rItem);

    return aAddrText       == rEnv.aAddrText       &&
           bSend           == rEnv.bSend           &&
           aSendText       == rEnv.aSendText       &&
           lSendFromLeft   == rEnv.lSendFromLeft   &&
           lSendFromTop    == rEnv.lSendFromTop    &&
           lAddrFromLeft   == rEnv.lAddrFromLeft   &&
           lAddrFromTop    == rEnv.lAddrFromTop    &&
           lWidth          == rEnv.lWidth          &&
           lHeight         == rEnv.lHeight         &&
           eAlign          == rEnv.eAlign          &&
           bPrintFromAbove == rEnv.bPrintFromAbove &&
           lShiftRight     == rEnv.lShiftRight     &&
           lShiftDown      == rEnv.lShiftDown;
}

SfxPoolItem* SwEnvItem::Clone(SfxItemPool*) const
{
    return new SwEnvItem(*this);
}

SwEnvCfgItem::SwEnvCfgItem() :
    ConfigItem("Office.Writer/Envelope")
{
    Sequence<OUString> aNames = GetPropertyNames();
    Sequence<Any> aValues = GetProperties(aNames);
    EnableNotification(aNames);
    const Any* pValues = aValues.getConstArray();
    OSL_ENSURE(aValues.getLength() == aNames.getLength(), "GetProperties failed");
    if(aValues.getLength() == aNames.getLength())
    {
        for(int nProp = 0; nProp < aNames.getLength(); nProp++)
        {
            if(pValues[nProp].hasValue())
            {
                switch(nProp)
                {
                    case  0: pValues[nProp] >>= aEnvItem.aAddrText; break;// "Inscription/Addressee",
                    case  1: pValues[nProp] >>= aEnvItem.aSendText; break;// "Inscription/Sender",
                    case  2: aEnvItem.bSend = *static_cast<sal_Bool const *>(pValues[nProp].getValue()); break;// "Inscription/UseSender",
                    case  3:
                        pValues[nProp] >>= aEnvItem.lAddrFromLeft;// "Format/AddresseeFromLeft",
                        aEnvItem.lAddrFromLeft = convertMm100ToTwip(aEnvItem.lAddrFromLeft);
                    break;
                    case  4:
                        pValues[nProp] >>= aEnvItem.lAddrFromTop;  // "Format/AddresseeFromTop",
                        aEnvItem.lAddrFromTop = convertMm100ToTwip(aEnvItem.lAddrFromTop);
                    break;
                    case  5:
                        pValues[nProp] >>= aEnvItem.lSendFromLeft; // "Format/SenderFromLeft",
                        aEnvItem.lSendFromLeft = convertMm100ToTwip(aEnvItem.lSendFromLeft);
                    break;
                    case  6:
                        pValues[nProp] >>= aEnvItem.lSendFromTop;// "Format/SenderFromTop",
                        aEnvItem.lSendFromTop = convertMm100ToTwip(aEnvItem.lSendFromTop);
                    break;
                    case  7:
                        pValues[nProp] >>= aEnvItem.lWidth; // "Format/Width",
                        aEnvItem.lWidth = convertMm100ToTwip(aEnvItem.lWidth);
                    break;
                    case  8:
                        pValues[nProp] >>= aEnvItem.lHeight; // "Format/Height",
                        aEnvItem.lHeight = convertMm100ToTwip(aEnvItem.lHeight);
                    break;
                    case  9:
                    {
                        sal_Int32 nTemp = 0;
                        pValues[nProp] >>= nTemp; aEnvItem.eAlign = (SwEnvAlign)nTemp; break;// "Print/Alignment",
                    }
                    case 10: aEnvItem.bPrintFromAbove = *static_cast<sal_Bool const *>(pValues[nProp].getValue()); break;// "Print/FromAbove",
                    case 11:
                        pValues[nProp] >>= aEnvItem.lShiftRight;
                        aEnvItem.lShiftRight = convertMm100ToTwip(aEnvItem.lShiftRight);// "Print/Right",
                    break;
                    case 12:
                        pValues[nProp] >>= aEnvItem.lShiftDown;
                        aEnvItem.lShiftDown = convertMm100ToTwip(aEnvItem.lShiftDown);
                    break;// "Print/Down"
                }
            }
        }
    }
}

SwEnvCfgItem::~SwEnvCfgItem()
{
}

void SwEnvCfgItem::ImplCommit()
{
    Sequence<OUString> aNames = GetPropertyNames();
    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    for(int nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        switch(nProp)
        {
            case  0: pValues[nProp] <<= aEnvItem.aAddrText; break;// "Inscription/Addressee",
            case  1: pValues[nProp] <<= aEnvItem.aSendText; break;// "Inscription/Sender",
            case  2: pValues[nProp] <<= aEnvItem.bSend; break;// "Inscription/UseSender",
            case  3: pValues[nProp] <<= static_cast <sal_Int32>(convertTwipToMm100(aEnvItem.lAddrFromLeft)) ; break;// "Format/AddresseeFromLeft",
            case  4: pValues[nProp] <<= static_cast <sal_Int32>(convertTwipToMm100(aEnvItem.lAddrFromTop))  ; break;// "Format/AddresseeFromTop",
            case  5: pValues[nProp] <<= static_cast <sal_Int32>(convertTwipToMm100(aEnvItem.lSendFromLeft)) ; break;// "Format/SenderFromLeft",
            case  6: pValues[nProp] <<= static_cast <sal_Int32>(convertTwipToMm100(aEnvItem.lSendFromTop))  ; break;// "Format/SenderFromTop",
            case  7: pValues[nProp] <<= static_cast <sal_Int32>(convertTwipToMm100(aEnvItem.lWidth))  ; break;// "Format/Width",
            case  8: pValues[nProp] <<= static_cast <sal_Int32>(convertTwipToMm100(aEnvItem.lHeight)) ; break;// "Format/Height",
            case  9: pValues[nProp] <<= sal_Int32(aEnvItem.eAlign); break;// "Print/Alignment",
            case 10: pValues[nProp] <<= aEnvItem.bPrintFromAbove; break;// "Print/FromAbove",
            case 11: pValues[nProp] <<= static_cast <sal_Int32>(convertTwipToMm100(aEnvItem.lShiftRight));break; // "Print/Right",
            case 12: pValues[nProp] <<= static_cast <sal_Int32>(convertTwipToMm100(aEnvItem.lShiftDown)); break;// "Print/Down"
        }
    }
    PutProperties(aNames, aValues);
}

void SwEnvCfgItem::Notify( const css::uno::Sequence< OUString >& ) {}

Sequence<OUString> SwEnvCfgItem::GetPropertyNames()
{
    static const char* aPropNames[] =
    {
        "Inscription/Addressee",    //  0
        "Inscription/Sender",       //  1
        "Inscription/UseSender",    //  2
        "Format/AddresseeFromLeft", //  3
        "Format/AddresseeFromTop",  //  4
        "Format/SenderFromLeft",    //  5
        "Format/SenderFromTop",     //  6
        "Format/Width",             //  7
        "Format/Height",            //  8
        "Print/Alignment",          //  9
        "Print/FromAbove",          // 10
        "Print/Right",              // 11
        "Print/Down"                // 12
    };
    const int nCount = 13;
    Sequence<OUString> aNames(nCount);
    OUString* pNames = aNames.getArray();

    for(int i = 0; i < nCount; i++)
        pNames[i] = OUString::createFromAscii(aPropNames[i]);

    return aNames;
}

bool SwEnvItem::QueryValue( Any& rVal, sal_uInt8 nMemberId ) const
{
    bool bRet = true;
    switch(nMemberId & ~CONVERT_TWIPS)
    {
        case MID_ENV_ADDR_TEXT : rVal <<= aAddrText; break;
        case MID_ENV_SEND : rVal <<= bSend; break;
        case MID_SEND_TEXT : rVal <<= aSendText; break;
        case MID_ENV_ADDR_FROM_LEFT : rVal <<= lAddrFromLeft; break;
        case MID_ENV_ADDR_FROM_TOP : rVal <<= lAddrFromTop; break;
        case MID_ENV_SEND_FROM_LEFT : rVal <<= lSendFromLeft; break;
        case MID_ENV_SEND_FROM_TOP : rVal <<= lSendFromTop; break;
        case MID_ENV_WIDTH : rVal <<= lWidth; break;
        case MID_ENV_HEIGHT : rVal <<= lHeight; break;
        case MID_ENV_ALIGN            : rVal <<= (sal_Int16) eAlign; break;
        case MID_ENV_PRINT_FROM_ABOVE  : rVal <<= bPrintFromAbove; break;
        case MID_ENV_SHIFT_RIGHT      : rVal <<= lShiftRight; break;
        case MID_ENV_SHIFT_DOWN       : rVal <<= lShiftDown; break;
        default:
            OSL_FAIL("Wrong memberId");
            bRet = false;
    }
    return bRet;
}

bool SwEnvItem::PutValue(const Any& rVal, sal_uInt8 nMemberId)
{
    bool bRet = false;
    switch(nMemberId  & ~CONVERT_TWIPS)
    {
        case MID_ENV_ADDR_TEXT : bRet = (rVal >>= aAddrText); break;
        case MID_ENV_SEND : bRet = (rVal >>= bSend); break;
        case MID_SEND_TEXT : bRet = (rVal >>= aSendText); break;
        case MID_ENV_ADDR_FROM_LEFT : bRet = (rVal >>= lAddrFromLeft); break;
        case MID_ENV_ADDR_FROM_TOP : bRet = (rVal >>= lAddrFromTop); break;
        case MID_ENV_SEND_FROM_LEFT : bRet = (rVal >>= lSendFromLeft); break;
        case MID_ENV_SEND_FROM_TOP : bRet = (rVal >>= lSendFromTop); break;
        case MID_ENV_WIDTH : bRet = (rVal >>= lWidth); break;
        case MID_ENV_HEIGHT : bRet = (rVal >>= lHeight); break;
        case MID_ENV_ALIGN :
        {
            sal_Int16 nTemp = 0;
            bRet = (rVal >>= nTemp);
            if (bRet)
                eAlign = SwEnvAlign(nTemp);
            break;
        }
        case MID_ENV_PRINT_FROM_ABOVE  : bRet = (rVal >>= bPrintFromAbove); break;
        case MID_ENV_SHIFT_RIGHT      : bRet = (rVal >>= lShiftRight); break;
        case MID_ENV_SHIFT_DOWN       : bRet = (rVal >>= lShiftDown); break;
        default:
            OSL_FAIL("Wrong memberId");
    }
    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
