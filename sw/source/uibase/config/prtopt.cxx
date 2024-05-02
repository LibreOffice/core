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

#include <unotools/configmgr.hxx>
#include <prtopt.hxx>
#include <o3tl/any.hxx>
#include <osl/diagnose.h>
#include <com/sun/star/uno/Sequence.hxx>

using namespace utl;
using namespace com::sun::star::uno;

// Ctor
Sequence<OUString> SwPrintOptions::GetPropertyNames() const
{
    static constexpr OUString aPropNames[] =
    {
        u"Content/Graphic"_ustr,              //  0
        u"Content/Control"_ustr,              //  1
        u"Content/Background"_ustr,           //  2
        u"Content/PrintBlack"_ustr,           //  3
        u"Content/Note"_ustr,                 //  4
        u"Page/Brochure"_ustr,                //  5
        u"Page/BrochureRightToLeft"_ustr,     //  6
        u"Output/Fax"_ustr,                   //  7
        u"Papertray/FromPrinterSetup"_ustr,   //  8
        u"Page/LeftPage"_ustr,                //  9 not in SW/Web
        u"Page/RightPage"_ustr,               // 10 not in SW/Web
        u"EmptyPages"_ustr,                   // 11 not in SW/Web
        u"Content/PrintPlaceholders"_ustr,    // 12 not in Sw/Web
        u"Content/PrintHiddenText"_ustr       // 13 not in Sw/Web
    };
    const int nCount = m_bIsWeb ? 9 : 14;
    Sequence<OUString> aNames(nCount);
    OUString* pNames = aNames.getArray();
    for(int i = 0; i < nCount; i++)
    {
        pNames[i] = aPropNames[i];
    }
    return aNames;
}

SwPrintOptions::SwPrintOptions(bool bWeb) :
    ConfigItem(bWeb ? OUString("Office.WriterWeb/Print") : OUString("Office.Writer/Print"),
        ConfigItemMode::ReleaseTree),
    m_bIsWeb(bWeb)
{
    m_bPrintPageBackground = !bWeb;
    m_bPrintBlackFont = bWeb;
    m_bPrintTextPlaceholder = m_bPrintHiddenText = false;
    if (bWeb)
        m_bPrintEmptyPages = false;

    Sequence<OUString> aNames = GetPropertyNames();
    Sequence<Any> aValues = GetProperties(aNames);
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
                    case  0: m_bPrintGraphic      = *o3tl::doAccess<bool>(pValues[nProp]); break;
                    case  1: m_bPrintControl      = *o3tl::doAccess<bool>(pValues[nProp]);  break;
                    case  2: m_bPrintPageBackground= *o3tl::doAccess<bool>(pValues[nProp]);  break;
                    case  3: m_bPrintBlackFont        = *o3tl::doAccess<bool>(pValues[nProp]);  break;
                    case  4:
                    {
                        sal_Int32 nTmp = 0;
                        pValues[nProp] >>=  nTmp;
                        m_nPrintPostIts = static_cast<SwPostItMode>(nTmp);
                    }
                    break;
                    case  5: m_bPrintProspect      = *o3tl::doAccess<bool>(pValues[nProp]);  break;
                    case  6: m_bPrintProspectRTL  = *o3tl::doAccess<bool>(pValues[nProp]);  break;
                    case  7: pValues[nProp] >>= m_sFaxName;  break;
                    case  8: m_bPaperFromSetup    = *o3tl::doAccess<bool>(pValues[nProp]);  break;
                    case  9: m_bPrintLeftPages    = *o3tl::doAccess<bool>(pValues[nProp]);  break;
                    case 10: m_bPrintRightPages       = *o3tl::doAccess<bool>(pValues[nProp]);  break;
                    case 11: m_bPrintEmptyPages       = *o3tl::doAccess<bool>(pValues[nProp]);  break;
                    case 12: m_bPrintTextPlaceholder = *o3tl::doAccess<bool>(pValues[nProp]);  break;
                    case 13: m_bPrintHiddenText = *o3tl::doAccess<bool>(pValues[nProp]);  break;
                }
            }
        }
    }
}

SwPrintOptions::~SwPrintOptions()
{
}

void SwPrintOptions::Notify( const css::uno::Sequence< OUString >& ) {}

void SwPrintOptions::ImplCommit()
{
    Sequence<OUString> aNames = GetPropertyNames();

    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    for(int nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        switch(nProp)
        {
            case  0: pValues[nProp] <<= m_bPrintGraphic; break;
            case  1: pValues[nProp] <<= m_bPrintControl; break;
            case  2: pValues[nProp] <<= m_bPrintPageBackground; break;
            case  3: pValues[nProp] <<= m_bPrintBlackFont; break;
            case  4: pValues[nProp] <<=  static_cast<sal_Int32>(m_nPrintPostIts)       ; break;
            case  5: pValues[nProp] <<= m_bPrintProspect; break;
            case  6: pValues[nProp] <<= m_bPrintProspectRTL; break;
            case  7: pValues[nProp] <<= m_sFaxName;  break;
            case  8: pValues[nProp] <<= m_bPaperFromSetup; break;
            case  9: pValues[nProp] <<= m_bPrintLeftPages; break;
            case 10: pValues[nProp] <<= m_bPrintRightPages; break;
            case 11: pValues[nProp] <<= m_bPrintEmptyPages; break;
            case 12: pValues[nProp] <<= m_bPrintTextPlaceholder; break;
            case 13: pValues[nProp] <<= m_bPrintHiddenText; break;
        }
    }

    PutProperties(aNames, aValues);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
