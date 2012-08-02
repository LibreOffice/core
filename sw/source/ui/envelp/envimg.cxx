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

#include <hintids.hxx>
#include <comphelper/string.hxx>
#include <tools/stream.hxx>
#include <tools/resid.hxx>
#include <sfx2/app.hxx>
#include <editeng/paperinf.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <unotools/useroptions.hxx>
#include <tools/shl.hxx>
#include <swmodule.hxx>
#include <swtypes.hxx>
#include <envimg.hxx>

#include <cmdid.h>
#include <envelp.hrc>

#include <unomid.h>

#ifdef WNT
#define NEXTLINE  rtl::OUString("\r\n")
#else
#define NEXTLINE  '\n'
#endif

using namespace utl;
using namespace ::com::sun::star::uno;

using ::rtl::OUString;


TYPEINIT1_AUTOFACTORY( SwEnvItem, SfxPoolItem );

SW_DLLPUBLIC String MakeSender()
{
    SvtUserOptions& rUserOpt = SW_MOD()->GetUserOptions();

    String sRet;
    String sSenderToken(SW_RES(STR_SENDER_TOKENS));
    xub_StrLen nSttPos = 0, nTokenCount = comphelper::string::getTokenCount(sSenderToken, ';');
    sal_Bool bLastLength = sal_True;
    for( xub_StrLen i = 0; i < nTokenCount; i++ )
    {
        String sToken = sSenderToken.GetToken( 0, ';', nSttPos );
        if(sToken.EqualsAscii("COMPANY"))
        {
            xub_StrLen nOldLen = sRet.Len();
            sRet += (String)rUserOpt.GetCompany();
            bLastLength = sRet.Len() != nOldLen;
        }
        else if(sToken.EqualsAscii("CR"))
        {
            if(bLastLength)
                sRet +=NEXTLINE;
            bLastLength = sal_True;
        }
        else if(sToken.EqualsAscii("FIRSTNAME"))
            sRet += (String)rUserOpt.GetFirstName();
        else if(sToken.EqualsAscii("LASTNAME"))
            sRet += (String)rUserOpt.GetLastName();
        else if(sToken.EqualsAscii("ADDRESS"))
            sRet += (String)rUserOpt.GetStreet();
        else if(sToken.EqualsAscii("COUNTRY"))
            sRet += (String)rUserOpt.GetCountry();
        else if(sToken.EqualsAscii("POSTALCODE"))
            sRet += (String)rUserOpt.GetZip();
        else if(sToken.EqualsAscii("CITY"))
            sRet += (String)rUserOpt.GetCity();
        else if(sToken.EqualsAscii("STATEPROV"))
            sRet += (String)rUserOpt.GetState();
        else if(sToken.Len()) //spaces
            sRet += sToken;
    }
    return sRet;
}

SwEnvItem::SwEnvItem() :
    SfxPoolItem(FN_ENVELOP)
{
    aAddrText       = aEmptyStr;
    bSend           = sal_True;
    aSendText       = MakeSender();
    lSendFromLeft   = 566; // 1 cm
    lSendFromTop    = 566; // 1 cm
    Size aEnvSz     = SvxPaperInfo::GetPaperSize(PAPER_ENV_C65);
    lWidth          = aEnvSz.Width();
    lHeight         = aEnvSz.Height();
    eAlign          = ENV_HOR_LEFT;
    bPrintFromAbove = sal_True;
    lShiftRight     = 0;
    lShiftDown      = 0;

    lAddrFromLeft   = Max(lWidth, lHeight) / 2;
    lAddrFromTop    = Min(lWidth, lHeight) / 2;
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

int SwEnvItem::operator ==(const SfxPoolItem& rItem) const
{
    const SwEnvItem& rEnv = (const SwEnvItem&) rItem;

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
    ConfigItem(C2U("Office.Writer/Envelope"))
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
                    case  2: aEnvItem.bSend = *(sal_Bool*)pValues[nProp].getValue(); break;// "Inscription/UseSender",
                    case  3:
                        pValues[nProp] >>= aEnvItem.lAddrFromLeft;// "Format/AddresseeFromLeft",
                        aEnvItem.lAddrFromLeft = MM100_TO_TWIP(aEnvItem.lAddrFromLeft);
                    break;
                    case  4:
                        pValues[nProp] >>= aEnvItem.lAddrFromTop;  // "Format/AddresseeFromTop",
                        aEnvItem.lAddrFromTop = MM100_TO_TWIP(aEnvItem.lAddrFromTop);
                    break;
                    case  5:
                        pValues[nProp] >>= aEnvItem.lSendFromLeft; // "Format/SenderFromLeft",
                        aEnvItem.lSendFromLeft = MM100_TO_TWIP(aEnvItem.lSendFromLeft);
                    break;
                    case  6:
                        pValues[nProp] >>= aEnvItem.lSendFromTop;// "Format/SenderFromTop",
                        aEnvItem.lSendFromTop = MM100_TO_TWIP(aEnvItem.lSendFromTop);
                    break;
                    case  7:
                        pValues[nProp] >>= aEnvItem.lWidth; // "Format/Width",
                        aEnvItem.lWidth = MM100_TO_TWIP(aEnvItem.lWidth);
                    break;
                    case  8:
                        pValues[nProp] >>= aEnvItem.lHeight; // "Format/Height",
                        aEnvItem.lHeight = MM100_TO_TWIP(aEnvItem.lHeight);
                    break;
                    case  9:
                    {
                        sal_Int32 nTemp = 0;
                        pValues[nProp] >>= nTemp; aEnvItem.eAlign = (SwEnvAlign)nTemp; break;// "Print/Alignment",
                    }
                    case 10: aEnvItem.bPrintFromAbove = *(sal_Bool*)pValues[nProp].getValue(); break;// "Print/FromAbove",
                    case 11:
                        pValues[nProp] >>= aEnvItem.lShiftRight;
                        aEnvItem.lShiftRight = MM100_TO_TWIP(aEnvItem.lShiftRight);// "Print/Right",
                    break;
                    case 12:
                        pValues[nProp] >>= aEnvItem.lShiftDown;
                        aEnvItem.lShiftDown = MM100_TO_TWIP(aEnvItem.lShiftDown);
                    break;// "Print/Down"
                }
            }
        }
    }
}

SwEnvCfgItem::~SwEnvCfgItem()
{
}

void    SwEnvCfgItem::Commit()
{
    Sequence<OUString> aNames = GetPropertyNames();
    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    const Type& rType = ::getBooleanCppuType();
    for(int nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        switch(nProp)
        {
            case  0: pValues[nProp] <<= aEnvItem.aAddrText; break;// "Inscription/Addressee",
            case  1: pValues[nProp] <<= aEnvItem.aSendText; break;// "Inscription/Sender",
            case  2: pValues[nProp].setValue(&aEnvItem.bSend, rType);break;// "Inscription/UseSender",
            case  3: pValues[nProp] <<= static_cast <sal_Int32>(TWIP_TO_MM100(aEnvItem.lAddrFromLeft)) ; break;// "Format/AddresseeFromLeft",
            case  4: pValues[nProp] <<= static_cast <sal_Int32>(TWIP_TO_MM100(aEnvItem.lAddrFromTop))  ; break;// "Format/AddresseeFromTop",
            case  5: pValues[nProp] <<= static_cast <sal_Int32>(TWIP_TO_MM100(aEnvItem.lSendFromLeft)) ; break;// "Format/SenderFromLeft",
            case  6: pValues[nProp] <<= static_cast <sal_Int32>(TWIP_TO_MM100(aEnvItem.lSendFromTop))  ; break;// "Format/SenderFromTop",
            case  7: pValues[nProp] <<= static_cast <sal_Int32>(TWIP_TO_MM100(aEnvItem.lWidth))  ; break;// "Format/Width",
            case  8: pValues[nProp] <<= static_cast <sal_Int32>(TWIP_TO_MM100(aEnvItem.lHeight)) ; break;// "Format/Height",
            case  9: pValues[nProp] <<= sal_Int32(aEnvItem.eAlign); break;// "Print/Alignment",
            case 10: pValues[nProp].setValue(&aEnvItem.bPrintFromAbove, rType); break;// "Print/FromAbove",
            case 11: pValues[nProp] <<= static_cast <sal_Int32>(TWIP_TO_MM100(aEnvItem.lShiftRight));break; // "Print/Right",
            case 12: pValues[nProp] <<= static_cast <sal_Int32>(TWIP_TO_MM100(aEnvItem.lShiftDown)); break;// "Print/Down"
        }
    }
    PutProperties(aNames, aValues);
}

void SwEnvCfgItem::Notify( const ::com::sun::star::uno::Sequence< rtl::OUString >& ) {}

Sequence<rtl::OUString> SwEnvCfgItem::GetPropertyNames()
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
    sal_Bool bRet = true;
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
