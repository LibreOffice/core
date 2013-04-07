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
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include <unomid.h>


using namespace utl;
using namespace com::sun::star::uno;

/*--------------------------------------------------------------------
     Description: Ctor
 --------------------------------------------------------------------*/

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

SwPrintOptions::SwPrintOptions(sal_Bool bWeb) :
    ConfigItem(bWeb ? OUString("Office.WriterWeb/Print") : OUString("Office.Writer/Print"),
        CONFIG_MODE_DELAYED_UPDATE|CONFIG_MODE_RELEASE_TREE),
    bIsWeb(bWeb)
{
    bPrintPageBackground = !bWeb;
    bPrintBlackFont = bWeb;
    bPrintTextPlaceholder = bPrintHiddenText = sal_False;
    if (bWeb)
        bPrintEmptyPages = sal_False;

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
                    case  0: bPrintGraphic      = *(sal_Bool*)pValues[nProp].getValue(); break;
                    case  1: bPrintTable            = *(sal_Bool*)pValues[nProp].getValue();  break;
                    case  2: bPrintControl      = *(sal_Bool*)pValues[nProp].getValue() ;  break;
                    case  3: bPrintPageBackground= *(sal_Bool*)pValues[nProp].getValue();  break;
                    case  4: bPrintBlackFont        = *(sal_Bool*)pValues[nProp].getValue();  break;
                    case  5:
                    {
                        sal_Int32 nTmp = 0;
                        pValues[nProp] >>=  nTmp;
                        nPrintPostIts = (sal_Int16)nTmp;
                    }
                    break;
                    case  6: bPrintReverse      = *(sal_Bool*)pValues[nProp].getValue();  break;
                    case  7: bPrintProspect      = *(sal_Bool*)pValues[nProp].getValue();  break;
                    case  8: bPrintProspectRTL  = *(sal_Bool*)pValues[nProp].getValue();  break;
                    case  9: bPrintSingleJobs   = *(sal_Bool*)pValues[nProp].getValue();  break;
                    case 10: pValues[nProp] >>= sFaxName;  break;
                    case 11: bPaperFromSetup    = *(sal_Bool*)pValues[nProp].getValue();  break;
                    case 12: bPrintDraw         = *(sal_Bool*)pValues[nProp].getValue() ;  break;
                    case 13: bPrintLeftPages    = *(sal_Bool*)pValues[nProp].getValue();  break;
                    case 14: bPrintRightPages       = *(sal_Bool*)pValues[nProp].getValue();  break;
                    case 15: bPrintEmptyPages       = *(sal_Bool*)pValues[nProp].getValue();  break;
                    case 16: bPrintTextPlaceholder = *(sal_Bool*)pValues[nProp].getValue();  break;
                    case 17: bPrintHiddenText = *(sal_Bool*)pValues[nProp].getValue();  break;
                }
            }
        }
    }

    // currently there is just one checkbox for print drawings and print graphics
    // In the UI. (File/Print dialog and Tools/Options/.../Print)
    // And since print graphics is the only available in Writer and WrtierWeb ...

    bPrintDraw = bPrintGraphic;
}

SwPrintOptions::~SwPrintOptions()
{
}


void SwPrintOptions::Notify( const ::com::sun::star::uno::Sequence< OUString >& ) {}

void    SwPrintOptions::Commit()
{
    Sequence<OUString> aNames = GetPropertyNames();

    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    const Type& rType = ::getBooleanCppuType();
    sal_Bool bVal;
    for(int nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        switch(nProp)
        {
            case  0: bVal = bPrintGraphic; pValues[nProp].setValue(&bVal, rType);break;
            case  1: bVal = bPrintTable         ;pValues[nProp].setValue(&bVal, rType);  break;
            case  2: bVal = bPrintControl        ; pValues[nProp].setValue(&bVal, rType);  break;
            case  3: bVal = bPrintPageBackground; pValues[nProp].setValue(&bVal, rType);  break;
            case  4: bVal = bPrintBlackFont     ; pValues[nProp].setValue(&bVal, rType);  break;
            case  5: pValues[nProp] <<=  (sal_Int32)nPrintPostIts       ; break;
            case  6: bVal = bPrintReverse       ; pValues[nProp].setValue(&bVal, rType);  break;
            case  7: bVal = bPrintProspect      ; pValues[nProp].setValue(&bVal, rType);  break;
            case  8: bVal = bPrintProspectRTL      ; pValues[nProp].setValue(&bVal, rType);  break;
            case  9: bVal = bPrintSingleJobs     ; pValues[nProp].setValue(&bVal, rType);  break;
            case 10: pValues[nProp] <<= sFaxName;  break;
            case 11: bVal = bPaperFromSetup     ; pValues[nProp].setValue(&bVal, rType);  break;
            case 12: bVal = bPrintDraw           ; pValues[nProp].setValue(&bVal, rType);  break;
            case 13: bVal = bPrintLeftPages      ; pValues[nProp].setValue(&bVal, rType);  break;
            case 14: bVal = bPrintRightPages    ; pValues[nProp].setValue(&bVal, rType);  break;
            case 15: bVal = bPrintEmptyPages    ; pValues[nProp].setValue(&bVal, rType);  break;
            case 16: bVal = bPrintTextPlaceholder; pValues[nProp].setValue(&bVal, rType);  break;
            case 17: bVal = bPrintHiddenText; pValues[nProp].setValue(&bVal, rType);  break;
        }
    }

    // currently there is just one checkbox for print drawings and print graphics
    // In the UI. (File/Print dialog and Tools/Options/.../Print)
    // And since print graphics is the only available in Writer and WrtierWeb ...
    bPrintDraw = bPrintGraphic;

    PutProperties(aNames, aValues);
}




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
