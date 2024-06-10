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

#include <sal/config.h>

#include <o3tl/any.hxx>
#include <o3tl/string_view.hxx>
#include <osl/diagnose.h>
#include <editeng/paperinf.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <rtl/ustrbuf.hxx>
#include <tools/UnitConversion.hxx>
#include <unotools/useroptions.hxx>
#include <swmodule.hxx>
#include <swtypes.hxx>
#include <envimg.hxx>
#include <strings.hrc>

#include <cmdid.h>

#include <unomid.h>

using namespace utl;
using namespace ::com::sun::star::uno;


SfxPoolItem* SwEnvItem::CreateDefault() { return new SwEnvItem; }

OUString MakeSender()
{
    SvtUserOptions& rUserOpt = SW_MOD()->GetUserOptions();

    const OUString sSenderToken(SwResId(STR_SENDER_TOKENS));
    if (sSenderToken.isEmpty())
        return OUString();

    OUStringBuffer sRet;
    sal_Int32 nSttPos = 0;
    bool bLastLength = true;
    do {
        std::u16string_view sToken = o3tl::getToken(sSenderToken, 0, ';', nSttPos );
        if (sToken == u"COMPANY")
        {
            sal_Int32 nOldLen = sRet.getLength();
            sRet.append(rUserOpt.GetCompany());
            bLastLength = sRet.getLength() != nOldLen;
        }
        else if (sToken == u"CR")
        {
            if(bLastLength)
                sRet.append(SAL_NEWLINE_STRING);
            bLastLength = true;
        }
        else if (sToken == u"FIRSTNAME")
            sRet.append(rUserOpt.GetFirstName());
        else if (sToken == u"LASTNAME")
            sRet.append(rUserOpt.GetLastName());
        else if (sToken == u"ADDRESS")
            sRet.append(rUserOpt.GetStreet());
        else if (sToken == u"COUNTRY")
            sRet.append(rUserOpt.GetCountry());
        else if (sToken == u"POSTALCODE")
            sRet.append(rUserOpt.GetZip());
        else if (sToken == u"CITY")
            sRet.append(rUserOpt.GetCity());
        else if (sToken == u"STATEPROV")
            sRet.append(rUserOpt.GetState());
        else if (!sToken.empty()) //spaces
            sRet.append(sToken);
    } while (nSttPos>=0);
    return sRet.makeStringAndClear();
}

SwEnvItem::SwEnvItem() :
    SfxPoolItem(FN_ENVELOP)
{
    m_bSend           = true;
    m_aSendText       = MakeSender();
    m_nSendFromLeft   = 566; // 1 cm
    m_nSendFromTop    = 566; // 1 cm
    Size aEnvSz     = SvxPaperInfo::GetPaperSize(PAPER_ENV_C65);
    m_nWidth          = aEnvSz.Width();
    m_nHeight         = aEnvSz.Height();
    m_eAlign          = ENV_HOR_LEFT;
    m_bPrintFromAbove = true;
    m_nShiftRight     = 0;
    m_nShiftDown      = 0;

    m_nAddrFromLeft   = std::max(m_nWidth, m_nHeight) / 2;
    m_nAddrFromTop    = std::min(m_nWidth, m_nHeight) / 2;
}

SwEnvItem& SwEnvItem::operator =(const SwEnvItem& rItem)
{
    m_aAddrText       = rItem.m_aAddrText;
    m_bSend           = rItem.m_bSend;
    m_aSendText       = rItem.m_aSendText;
    m_nSendFromLeft   = rItem.m_nSendFromLeft;
    m_nSendFromTop    = rItem.m_nSendFromTop;
    m_nAddrFromLeft   = rItem.m_nAddrFromLeft;
    m_nAddrFromTop    = rItem.m_nAddrFromTop;
    m_nWidth          = rItem.m_nWidth;
    m_nHeight         = rItem.m_nHeight;
    m_eAlign          = rItem.m_eAlign;
    m_bPrintFromAbove = rItem.m_bPrintFromAbove;
    m_nShiftRight     = rItem.m_nShiftRight;
    m_nShiftDown      = rItem.m_nShiftDown;
    return *this;
}

bool SwEnvItem::operator ==(const SfxPoolItem& rItem) const
{
    assert(SfxPoolItem::operator==(rItem));
    const SwEnvItem& rEnv = static_cast<const SwEnvItem&>( rItem);

    return m_aAddrText       == rEnv.m_aAddrText       &&
           m_bSend           == rEnv.m_bSend           &&
           m_aSendText       == rEnv.m_aSendText       &&
           m_nSendFromLeft   == rEnv.m_nSendFromLeft   &&
           m_nSendFromTop    == rEnv.m_nSendFromTop    &&
           m_nAddrFromLeft   == rEnv.m_nAddrFromLeft   &&
           m_nAddrFromTop    == rEnv.m_nAddrFromTop    &&
           m_nWidth          == rEnv.m_nWidth          &&
           m_nHeight         == rEnv.m_nHeight         &&
           m_eAlign          == rEnv.m_eAlign          &&
           m_bPrintFromAbove == rEnv.m_bPrintFromAbove &&
           m_nShiftRight     == rEnv.m_nShiftRight     &&
           m_nShiftDown      == rEnv.m_nShiftDown;
}

SwEnvItem* SwEnvItem::Clone(SfxItemPool*) const
{
    return new SwEnvItem(*this);
}

SwEnvCfgItem::SwEnvCfgItem() :
    ConfigItem(u"Office.Writer/Envelope"_ustr)
{
    Sequence<OUString> aNames = GetPropertyNames();
    Sequence<Any> aValues = GetProperties(aNames);
    EnableNotification(aNames);
    const Any* pValues = aValues.getConstArray();
    OSL_ENSURE(aValues.getLength() == aNames.getLength(), "GetProperties failed");
    if(aValues.getLength() != aNames.getLength())
        return;

    for(int nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        if(pValues[nProp].hasValue())
        {
            switch(nProp)
            {
                case  0: pValues[nProp] >>= m_aEnvItem.m_aAddrText; break;// "Inscription/Addressee",
                case  1: pValues[nProp] >>= m_aEnvItem.m_aSendText; break;// "Inscription/Sender",
                case  2: m_aEnvItem.m_bSend = *o3tl::doAccess<bool>(pValues[nProp]); break;// "Inscription/UseSender",
                case  3:
                    pValues[nProp] >>= m_aEnvItem.m_nAddrFromLeft;// "Format/AddresseeFromLeft",
                    m_aEnvItem.m_nAddrFromLeft = o3tl::toTwips(m_aEnvItem.m_nAddrFromLeft, o3tl::Length::mm100);
                break;
                case  4:
                    pValues[nProp] >>= m_aEnvItem.m_nAddrFromTop;  // "Format/AddresseeFromTop",
                    m_aEnvItem.m_nAddrFromTop = o3tl::toTwips(m_aEnvItem.m_nAddrFromTop, o3tl::Length::mm100);
                break;
                case  5:
                    pValues[nProp] >>= m_aEnvItem.m_nSendFromLeft; // "Format/SenderFromLeft",
                    m_aEnvItem.m_nSendFromLeft = o3tl::toTwips(m_aEnvItem.m_nSendFromLeft, o3tl::Length::mm100);
                break;
                case  6:
                    pValues[nProp] >>= m_aEnvItem.m_nSendFromTop;// "Format/SenderFromTop",
                    m_aEnvItem.m_nSendFromTop = o3tl::toTwips(m_aEnvItem.m_nSendFromTop, o3tl::Length::mm100);
                break;
                case  7:
                    pValues[nProp] >>= m_aEnvItem.m_nWidth; // "Format/Width",
                    m_aEnvItem.m_nWidth = o3tl::toTwips(m_aEnvItem.m_nWidth, o3tl::Length::mm100);
                break;
                case  8:
                    pValues[nProp] >>= m_aEnvItem.m_nHeight; // "Format/Height",
                    m_aEnvItem.m_nHeight = o3tl::toTwips(m_aEnvItem.m_nHeight, o3tl::Length::mm100);
                break;
                case  9:
                {
                    sal_Int32 nTemp = 0;
                    pValues[nProp] >>= nTemp; m_aEnvItem.m_eAlign = static_cast<SwEnvAlign>(nTemp); break;// "Print/Alignment",
                }
                case 10: m_aEnvItem.m_bPrintFromAbove = *o3tl::doAccess<bool>(pValues[nProp]); break;// "Print/FromAbove",
                case 11:
                    pValues[nProp] >>= m_aEnvItem.m_nShiftRight;
                    m_aEnvItem.m_nShiftRight = o3tl::toTwips(m_aEnvItem.m_nShiftRight, o3tl::Length::mm100);// "Print/Right",
                break;
                case 12:
                    pValues[nProp] >>= m_aEnvItem.m_nShiftDown;
                    m_aEnvItem.m_nShiftDown = o3tl::toTwips(m_aEnvItem.m_nShiftDown, o3tl::Length::mm100);
                break;// "Print/Down"
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
            case  0: pValues[nProp] <<= m_aEnvItem.m_aAddrText; break;// "Inscription/Addressee",
            case  1: pValues[nProp] <<= m_aEnvItem.m_aSendText; break;// "Inscription/Sender",
            case  2: pValues[nProp] <<= m_aEnvItem.m_bSend; break;// "Inscription/UseSender",
            case  3: pValues[nProp] <<= static_cast <sal_Int32>(convertTwipToMm100(m_aEnvItem.m_nAddrFromLeft)) ; break;// "Format/AddresseeFromLeft",
            case  4: pValues[nProp] <<= static_cast <sal_Int32>(convertTwipToMm100(m_aEnvItem.m_nAddrFromTop))  ; break;// "Format/AddresseeFromTop",
            case  5: pValues[nProp] <<= static_cast <sal_Int32>(convertTwipToMm100(m_aEnvItem.m_nSendFromLeft)) ; break;// "Format/SenderFromLeft",
            case  6: pValues[nProp] <<= static_cast <sal_Int32>(convertTwipToMm100(m_aEnvItem.m_nSendFromTop))  ; break;// "Format/SenderFromTop",
            case  7: pValues[nProp] <<= static_cast <sal_Int32>(convertTwipToMm100(m_aEnvItem.m_nWidth))  ; break;// "Format/Width",
            case  8: pValues[nProp] <<= static_cast <sal_Int32>(convertTwipToMm100(m_aEnvItem.m_nHeight)) ; break;// "Format/Height",
            case  9: pValues[nProp] <<= sal_Int32(m_aEnvItem.m_eAlign); break;// "Print/Alignment",
            case 10: pValues[nProp] <<= m_aEnvItem.m_bPrintFromAbove; break;// "Print/FromAbove",
            case 11: pValues[nProp] <<= static_cast <sal_Int32>(convertTwipToMm100(m_aEnvItem.m_nShiftRight));break; // "Print/Right",
            case 12: pValues[nProp] <<= static_cast <sal_Int32>(convertTwipToMm100(m_aEnvItem.m_nShiftDown)); break;// "Print/Down"
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
        case MID_ENV_ADDR_TEXT : rVal <<= m_aAddrText; break;
        case MID_ENV_SEND : rVal <<= m_bSend; break;
        case MID_SEND_TEXT : rVal <<= m_aSendText; break;
        case MID_ENV_ADDR_FROM_LEFT : rVal <<= m_nAddrFromLeft; break;
        case MID_ENV_ADDR_FROM_TOP : rVal <<= m_nAddrFromTop; break;
        case MID_ENV_SEND_FROM_LEFT : rVal <<= m_nSendFromLeft; break;
        case MID_ENV_SEND_FROM_TOP : rVal <<= m_nSendFromTop; break;
        case MID_ENV_WIDTH : rVal <<= m_nWidth; break;
        case MID_ENV_HEIGHT : rVal <<= m_nHeight; break;
        case MID_ENV_ALIGN            : rVal <<= static_cast<sal_Int16>(m_eAlign); break;
        case MID_ENV_PRINT_FROM_ABOVE  : rVal <<= m_bPrintFromAbove; break;
        case MID_ENV_SHIFT_RIGHT      : rVal <<= m_nShiftRight; break;
        case MID_ENV_SHIFT_DOWN       : rVal <<= m_nShiftDown; break;
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
        case MID_ENV_ADDR_TEXT : bRet = (rVal >>= m_aAddrText); break;
        case MID_ENV_SEND : bRet = (rVal >>= m_bSend); break;
        case MID_SEND_TEXT : bRet = (rVal >>= m_aSendText); break;
        case MID_ENV_ADDR_FROM_LEFT : bRet = (rVal >>= m_nAddrFromLeft); break;
        case MID_ENV_ADDR_FROM_TOP : bRet = (rVal >>= m_nAddrFromTop); break;
        case MID_ENV_SEND_FROM_LEFT : bRet = (rVal >>= m_nSendFromLeft); break;
        case MID_ENV_SEND_FROM_TOP : bRet = (rVal >>= m_nSendFromTop); break;
        case MID_ENV_WIDTH : bRet = (rVal >>= m_nWidth); break;
        case MID_ENV_HEIGHT : bRet = (rVal >>= m_nHeight); break;
        case MID_ENV_ALIGN :
        {
            sal_Int16 nTemp = 0;
            bRet = (rVal >>= nTemp);
            if (bRet)
                m_eAlign = SwEnvAlign(nTemp);
            break;
        }
        case MID_ENV_PRINT_FROM_ABOVE  : bRet = (rVal >>= m_bPrintFromAbove); break;
        case MID_ENV_SHIFT_RIGHT      : bRet = (rVal >>= m_nShiftRight); break;
        case MID_ENV_SHIFT_DOWN       : bRet = (rVal >>= m_nShiftDown); break;
        default:
            OSL_FAIL("Wrong memberId");
    }
    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
