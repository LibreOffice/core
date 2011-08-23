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

#include "misccfg.hxx"

#include <bf_svtools/zforlist.hxx>

#include <tools/debug.hxx>

#include <rtl/logfile.hxx>


#define DEFAULT_TAB 2000

#define DEF_INCH	2540L
#define DEF_RELTWIP	1440L

using namespace rtl;
using namespace com::sun::star::uno;

namespace binfilter
{

#define C2U(cChar) OUString::createFromAscii(cChar)
/*--------------------------------------------------------------------
     Beschreibung:
 --------------------------------------------------------------------*/
SfxMiscCfg::SfxMiscCfg() :
    ConfigItem(C2U("Office.Common") ),
    bPaperSize(FALSE),
    bPaperOrientation (FALSE),
    bNotFound (FALSE),
    nYear2000( SvNumberFormatter::GetYear2000Default() )
{
    RTL_LOGFILE_CONTEXT(aLog, "svtools SfxMiscCfg::SfxMiscCfg()");
    
    Load();
}
/* -----------------------------02.03.01 15:31--------------------------------

 ---------------------------------------------------------------------------*/
SfxMiscCfg::~SfxMiscCfg()
{
}

/* -----------------------------02.03.01 15:31--------------------------------

 ---------------------------------------------------------------------------*/
const Sequence<OUString>& SfxMiscCfg::GetPropertyNames()
{
    static Sequence<OUString> aNames;
    if(!aNames.getLength())
    {
        static const char* aPropNames[] =
        {
               "Print/Warning/PaperSize",				//  0
               "Print/Warning/PaperOrientation",		//  1
               "Print/Warning/NotFound",				//  2
            "DateFormat/TwoDigitYear",            	//  3
        };
        const int nCount = 4;
        aNames.realloc(nCount);
        OUString* pNames = aNames.getArray();
        for(int i = 0; i < nCount; i++)
            pNames[i] = OUString::createFromAscii(aPropNames[i]);
    }
    return aNames;
}
/* -----------------------------02.03.01 15:31--------------------------------

 ---------------------------------------------------------------------------*/
void SfxMiscCfg::Load()
{
    const Sequence<OUString>& aNames = GetPropertyNames();
    Sequence<Any> aValues = GetProperties(aNames);
    EnableNotification(aNames);
    const Any* pValues = aValues.getConstArray();
    DBG_ASSERT(aValues.getLength() == aNames.getLength(), "GetProperties failed");
    if(aValues.getLength() == aNames.getLength())
    {
        for(int nProp = 0; nProp < aNames.getLength(); nProp++)
        {
            if(pValues[nProp].hasValue())
            {
                switch(nProp)
                {
                    case  0: bPaperSize 	   = *(sal_Bool*)pValues[nProp].getValue(); break;		//"Print/Warning/PaperSize",
                    case  1: bPaperOrientation = *(sal_Bool*)pValues[nProp].getValue();  break;     //"Print/Warning/PaperOrientation",
                    case  2: bNotFound         = *(sal_Bool*)pValues[nProp].getValue()  ;  break;   //"Print/Warning/NotFound",
                    case  3: pValues[nProp] >>= nYear2000;break;                                    //"DateFormat/TwoDigitYear",
                }
            }
        }
    }
}
/* -----------------------------02.03.01 15:31--------------------------------

 ---------------------------------------------------------------------------*/
void SfxMiscCfg::Notify( const com::sun::star::uno::Sequence<rtl::OUString>& )
{
    Load();
}
/* -----------------------------02.03.01 15:31--------------------------------

 ---------------------------------------------------------------------------*/
void SfxMiscCfg::Commit()
{
    const Sequence<OUString>& aNames = GetPropertyNames();
    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    const Type& rType = ::getBooleanCppuType();
    for(int nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        switch(nProp)
        {
            case  0: pValues[nProp].setValue(&bPaperSize, rType);break;	 //"Print/Warning/PaperSize",
            case  1: pValues[nProp].setValue(&bPaperOrientation, rType);break;     //"Print/Warning/PaperOrientation",
            case  2: pValues[nProp].setValue(&bNotFound, rType);break;   //"Print/Warning/NotFound",
            case  3: pValues[nProp] <<= nYear2000;break;                 //"DateFormat/TwoDigitYear",
        }
    }
    PutProperties(aNames, aValues);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
