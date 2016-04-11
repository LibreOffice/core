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
#include <osl/diagnose.h>
#include <com/sun/star/uno/Sequence.hxx>

#include <unomid.h>

using namespace utl;
using namespace com::sun::star::uno;

// Ctor
Sequence<OUString> SwPrintOptions::GetPropertyNames()
{
    static const char* aPropNames[] =
    {
        "Content/Graphic",              //  0
        "Content/Table",                //  1
        "Content/Control",              //  2
        "Content/Background",           //  3
        "Content/PrintBlack",           //  4
        "Content/Note",                 //  5
        "Page/Reversed",                //  6
        "Page/Brochure",                //  7
        "Page/BrochureRightToLeft",     //  8
        "Output/SinglePrintJob",        //  9
        "Output/Fax",                   // 10
        "Papertray/FromPrinterSetup",   // 11
        "Content/Drawing",              // 12 not in SW/Web
        "Page/LeftPage",                // 13 not in SW/Web
        "Page/RightPage",               // 14 not in SW/Web
        "EmptyPages",                   // 15 not in SW/Web
        "Content/PrintPlaceholders",    // 16 not in Sw/Web
        "Content/PrintHiddenText"       // 17 not in Sw/Web
    };
    const int nCount = bIsWeb ? 12 : 18;
    Sequence<OUString> aNames(nCount);
    OUString* pNames = aNames.getArray();
    for(int i = 0; i < nCount; i++)
    {
        pNames[i] = OUString::createFromAscii(aPropNames[i]);
    }
    return aNames;
}

SwPrintOptions::SwPrintOptions(bool bWeb) :
    ConfigItem(bWeb ? OUString("Office.WriterWeb/Print") : OUString("Office.Writer/Print"),
        ConfigItemMode::DelayedUpdate|ConfigItemMode::ReleaseTree),
    bIsWeb(bWeb)
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
                    case  0: m_bPrintGraphic      = *static_cast<sal_Bool const *>(pValues[nProp].getValue()); break;
                    case  1: m_bPrintTable            = *static_cast<sal_Bool const *>(pValues[nProp].getValue());  break;
                    case  2: m_bPrintControl      = *static_cast<sal_Bool const *>(pValues[nProp].getValue());  break;
                    case  3: m_bPrintPageBackground= *static_cast<sal_Bool const *>(pValues[nProp].getValue());  break;
                    case  4: m_bPrintBlackFont        = *static_cast<sal_Bool const *>(pValues[nProp].getValue());  break;
                    case  5:
                    {
                        sal_Int32 nTmp = 0;
                        pValues[nProp] >>=  nTmp;
                        m_nPrintPostIts = static_cast<SwPostItMode>(nTmp);
                    }
                    break;
                    case  6: m_bPrintReverse      = *static_cast<sal_Bool const *>(pValues[nProp].getValue());  break;
                    case  7: m_bPrintProspect      = *static_cast<sal_Bool const *>(pValues[nProp].getValue());  break;
                    case  8: m_bPrintProspectRTL  = *static_cast<sal_Bool const *>(pValues[nProp].getValue());  break;
                    case  9: m_bPrintSingleJobs   = *static_cast<sal_Bool const *>(pValues[nProp].getValue());  break;
                    case 10: pValues[nProp] >>= m_sFaxName;  break;
                    case 11: m_bPaperFromSetup    = *static_cast<sal_Bool const *>(pValues[nProp].getValue());  break;
                    case 12: m_bPrintDraw         = *static_cast<sal_Bool const *>(pValues[nProp].getValue());  break;
                    case 13: m_bPrintLeftPages    = *static_cast<sal_Bool const *>(pValues[nProp].getValue());  break;
                    case 14: m_bPrintRightPages       = *static_cast<sal_Bool const *>(pValues[nProp].getValue());  break;
                    case 15: m_bPrintEmptyPages       = *static_cast<sal_Bool const *>(pValues[nProp].getValue());  break;
                    case 16: m_bPrintTextPlaceholder = *static_cast<sal_Bool const *>(pValues[nProp].getValue());  break;
                    case 17: m_bPrintHiddenText = *static_cast<sal_Bool const *>(pValues[nProp].getValue());  break;
                }
            }
        }
    }

    // currently there is just one checkbox for print drawings and print graphics
    // In the UI. (File/Print dialog and Tools/Options/.../Print)
    // And since print graphics is the only available in Writer and WrtierWeb ...

    m_bPrintDraw = m_bPrintGraphic;
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
            case  1: pValues[nProp] <<= m_bPrintTable; break;
            case  2: pValues[nProp] <<= m_bPrintControl; break;
            case  3: pValues[nProp] <<= m_bPrintPageBackground; break;
            case  4: pValues[nProp] <<= m_bPrintBlackFont; break;
            case  5: pValues[nProp] <<=  (sal_Int32)m_nPrintPostIts       ; break;
            case  6: pValues[nProp] <<= m_bPrintReverse; break;
            case  7: pValues[nProp] <<= m_bPrintProspect; break;
            case  8: pValues[nProp] <<= m_bPrintProspectRTL; break;
            case  9: pValues[nProp] <<= m_bPrintSingleJobs; break;
            case 10: pValues[nProp] <<= m_sFaxName;  break;
            case 11: pValues[nProp] <<= m_bPaperFromSetup; break;
            case 12: pValues[nProp] <<= m_bPrintDraw; break;
            case 13: pValues[nProp] <<= m_bPrintLeftPages; break;
            case 14: pValues[nProp] <<= m_bPrintRightPages; break;
            case 15: pValues[nProp] <<= m_bPrintEmptyPages; break;
            case 16: pValues[nProp] <<= m_bPrintTextPlaceholder; break;
            case 17: pValues[nProp] <<= m_bPrintHiddenText; break;
        }
    }

    // currently there is just one checkbox for print drawings and print graphics
    // In the UI. (File/Print dialog and Tools/Options/.../Print)
    // And since print graphics is the only available in Writer and WrtierWeb ...
    m_bPrintDraw = m_bPrintGraphic;

    PutProperties(aNames, aValues);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
