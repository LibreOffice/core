/*************************************************************************
 *
 *  $RCSfile: fontcfg.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:33 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#ifndef _FONTCFG_HXX
#include <fontcfg.hxx>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

using namespace utl;
using namespace rtl;
using namespace com::sun::star::uno;
#define C2S(cChar) String::CreateFromAscii(cChar)
#define C2U(cChar) OUString::createFromAscii(cChar)

/* -----------------------------08.09.00 15:52--------------------------------

 ---------------------------------------------------------------------------*/
Sequence<OUString> SwStdFontConfig::GetPropertyNames()
{
    static const char* aPropNames[] =
    {
        "Standard",    // 0
        "Heading",     // 1
        "List",        // 2
        "Caption",     // 3
        "Index",       // 4
    };
    const int nCount = 5;
    Sequence<OUString> aNames(nCount);
    OUString* pNames = aNames.getArray();
    for(int i = 0; i < nCount; i++)
    {
        pNames[i] = OUString::createFromAscii(aPropNames[i]);
    }
    return aNames;
}
/*-----------------03.09.96 15.00-------------------

--------------------------------------------------*/
SwStdFontConfig::SwStdFontConfig() :
    utl::ConfigItem(C2U("Office.Writer/DefaultFont"))
{
    String sDefFont = C2S("Times");
    sFontStandard = sFontList = sFontCaption = sFontIndex = sDefFont;
    sFontOutline = SwStdFontConfig::GetDefaultFor(FONT_OUTLINE);

    Sequence<OUString> aNames = GetPropertyNames();
    Sequence<Any> aValues = GetProperties(aNames);
//  EnableNotification(aNames);
    const Any* pValues = aValues.getConstArray();
    DBG_ASSERT(aValues.getLength() == aNames.getLength(), "GetProperties failed")
    if(aValues.getLength() == aNames.getLength())
    {
        for(int nProp = 0; nProp < aNames.getLength(); nProp++)
        {
            DBG_ASSERT(pValues[nProp].hasValue(), "property value missing")
            if(pValues[nProp].hasValue())
            {
                OUString sVal;
                pValues[nProp] >>= sVal;
                switch(nProp)
                {
                    case 0: sFontStandard= sVal;  break;
                    case 1: sFontOutline = sVal;  break;
                    case 2: sFontList    = sVal;  break;
                    case 3: sFontCaption = sVal;  break;
                    case 4: sFontIndex   = sVal;  break;
                }
            }
        }
    }
}
/* -----------------------------08.09.00 15:58--------------------------------

 ---------------------------------------------------------------------------*/
void    SwStdFontConfig::Commit()
{
    Sequence<OUString> aNames = GetPropertyNames();
    OUString* pNames = aNames.getArray();
    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();
    for(int nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        switch(nProp)
        {
            case 0: pValues[nProp] <<= OUString(sFontStandard);  break;
            case 1: pValues[nProp] <<= OUString(sFontOutline );  break;
            case 2: pValues[nProp] <<= OUString(sFontList    );  break;
            case 3: pValues[nProp] <<= OUString(sFontCaption );  break;
            case 4: pValues[nProp] <<= OUString(sFontIndex   );  break;
        }
    }
    PutProperties(aNames, aValues);
}
/* -----------------------------08.09.00 15:56--------------------------------

 ---------------------------------------------------------------------------*/
SwStdFontConfig::~SwStdFontConfig()
{}
/*-----------------18.01.97 10.05-------------------

--------------------------------------------------*/
BOOL SwStdFontConfig::IsFontDefault(USHORT nFontType) const
{
    BOOL bSame;
    const String rStd(C2S("Times"));
    switch( nFontType )
    {
        case FONT_STANDARD:
            bSame = sFontStandard == rStd;
        break;
        case FONT_OUTLINE :
#if defined(UNX)
            bSame = sFontOutline == C2S("helvetica");
#elif defined(WNT) || defined(WIN)
            bSame = sFontOutline == C2S("Arial");
#elif defined(MAC)
            bSame = sFontOutline == C2S("Helvetica");
#elif defined(PM20)
            bSame = sFontOutline == C2S("Helvetica");
#else
#error Defaultfont fuer diese Plattform?
#endif
        break;
        case FONT_LIST    :
            bSame = (sFontList == rStd) && (sFontStandard == rStd);
        break;
        case FONT_CAPTION :
            bSame = (sFontCaption == rStd) && (sFontStandard == rStd);
        break;
        case FONT_INDEX   :
            bSame = (sFontIndex == rStd) && (sFontStandard == rStd);
        break;
    }
    return bSame;
}

/* -----------------11.01.99 13:16-------------------
 * Standards auslesen
 * --------------------------------------------------*/
String  SwStdFontConfig::GetDefaultFor(USHORT nFontType)
{
    String sRet(C2S("Times"));
    switch( nFontType )
    {
        case FONT_STANDARD:
        case FONT_LIST    :
        case FONT_CAPTION :
        case FONT_INDEX   :
        break;
        case FONT_OUTLINE :
#if defined(UNX)
            sRet = C2S("helvetica");
#elif defined(WNT) || defined(WIN)
            sRet = C2S("Arial");
#elif defined(MAC)
            sRet = C2S("Helvetica");
#elif defined(PM20)
            sRet = C2S("Helvetica");
#else
#error Defaultfont fuer diese Plattform?
#endif
        break;
    }
    return sRet;
}
