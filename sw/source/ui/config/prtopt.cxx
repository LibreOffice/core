/*************************************************************************
 *
 *  $RCSfile: prtopt.cxx,v $
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

#ifndef _UTL_CONFIGMGR_HXX_
#include <unotools/configmgr.hxx>
#endif
#ifndef _PRTOPT_HXX
#include <prtopt.hxx>
#endif
#ifndef _SWPRTOPT_HXX
#include <swprtopt.hxx>
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

#define C2U(cChar) OUString::createFromAscii(cChar)
/*--------------------------------------------------------------------
     Beschreibung: Ctor
 --------------------------------------------------------------------*/

Sequence<OUString> SwPrintOptions::GetPropertyNames()
{
    static const char* aPropNames[] =
    {
        "Content/Graphic",          // 0
        "Content/Table",            // 1
        "Content/Drawing",          // 2
        "Content/Control",          // 3
        "Content/Background",       // 4
        "Content/PrintBlack",       // 5
        "Content/Note",             // 6
        "Page/LeftPage",            // 7
        "Page/RightPage",           // 8
        "Page/Reversed",            // 9
        "Page/Brochure",            // 10
        "Output/SinglePrintJob",    // 11
        "Output/Fax",               // 12
        "Papertray/FromPrinterSetup"// 13
    };
    const int nCount = 14;
    Sequence<OUString> aNames(nCount);
    OUString* pNames = aNames.getArray();
    for(int i = 0; i < nCount; i++)
    {
        pNames[i] = OUString::createFromAscii(aPropNames[i]);
    }
    return aNames;
}
/* -----------------------------06.09.00 16:44--------------------------------

 ---------------------------------------------------------------------------*/
SwPrintOptions::SwPrintOptions(sal_Bool bWeb) :
    ConfigItem(bWeb ? C2U("Office.WriterWeb/Print") :  C2U("Office.Writer/Print")),
    bPrintGraphic(sal_True),
    bPrintTable(sal_True),
    bPrintDraw(sal_True),
    bPrintControl(sal_True),
    bPrintLeftPage(sal_True),
    bPrintRightPage(sal_True),
    bReverse(sal_False),
    bPaperFromSetup(sal_False),
    bPrintProspect(sal_False),
    bPrintSingleJobs(sal_False),
    bPrintPageBackground(!bWeb),
    bPrintBlackFont(bWeb),
    nPrintPostIts(POSTITS_NONE)
{
    Sequence<OUString> aNames = GetPropertyNames();
    Sequence<Any> aValues = GetProperties(aNames);
    EnableNotification(aNames);
    const Any* pValues = aValues.getConstArray();
    DBG_ASSERT(aValues.getLength() == aNames.getLength(), "GetProperties failed")
    if(aValues.getLength() == aNames.getLength())
    {
        for(int nProp = 0; nProp < aNames.getLength(); nProp++)
        {
            DBG_ASSERT(pValues[nProp].hasValue(), "property value missing")
            if(pValues[nProp].hasValue())
            {
                switch(nProp)
                {
                    case 0: bPrintGraphic       = *(sal_Bool*)pValues[nProp].getValue(); break;
                    case 1: bPrintTable         = *(sal_Bool*)pValues[nProp].getValue();  break;
                    case 2: bPrintDraw          = *(sal_Bool*)pValues[nProp].getValue() ;  break;
                    case 3: bPrintControl       = *(sal_Bool*)pValues[nProp].getValue() ;  break;
                    case 4: bPrintPageBackground= *(sal_Bool*)pValues[nProp].getValue();  break;
                    case 5: bPrintBlackFont     = *(sal_Bool*)pValues[nProp].getValue();  break;
                    case 6: pValues[nProp] >>=  nPrintPostIts       ;  break;
                    case 7: bPrintLeftPage      = *(sal_Bool*)pValues[nProp].getValue();  break;
                    case 8: bPrintRightPage     = *(sal_Bool*)pValues[nProp].getValue();  break;
                    case 9: bReverse            = *(sal_Bool*)pValues[nProp].getValue();  break;
                    case 10:bPrintProspect      = *(sal_Bool*)pValues[nProp].getValue();  break;
                    case 11:bPrintSingleJobs    = *(sal_Bool*)pValues[nProp].getValue();  break;
                    case 12: pValues[nProp] >>= sFaxName;  break;
                    case 13: bPaperFromSetup    = *(sal_Bool*)pValues[nProp].getValue();  break;
                }
            }
        }
    }
}
/* -----------------------------06.09.00 16:50--------------------------------

 ---------------------------------------------------------------------------*/
SwPrintOptions::~SwPrintOptions()
{
}
/* -----------------------------06.09.00 16:43--------------------------------

 ---------------------------------------------------------------------------*/
void    SwPrintOptions::Commit()
{
    Sequence<OUString> aNames = GetPropertyNames();

    OUString* pNames = aNames.getArray();
    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    const Type& rType = ::getBooleanCppuType();
    BOOL bVal;
    for(int nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        switch(nProp)
        {
            case 0: bVal = bPrintGraphic; pValues[nProp].setValue(&bVal, rType);break;
            case 1: bVal = bPrintTable          ;pValues[nProp].setValue(&bVal, rType);  break;
            case 2: bVal = bPrintDraw            ; pValues[nProp].setValue(&bVal, rType);  break;
            case 3: bVal = bPrintControl         ; pValues[nProp].setValue(&bVal, rType);  break;
            case 4: bVal = bPrintPageBackground; pValues[nProp].setValue(&bVal, rType);  break;
            case 5: bVal = bPrintBlackFont      ; pValues[nProp].setValue(&bVal, rType);  break;
            case 6: pValues[nProp] <<=  nPrintPostIts       ; break;
            case 7: bVal = bPrintLeftPage        ; pValues[nProp].setValue(&bVal, rType);  break;
            case 8: bVal = bPrintRightPage      ; pValues[nProp].setValue(&bVal, rType);  break;
            case 9: bVal = bReverse          ; pValues[nProp].setValue(&bVal, rType);  break;
            case 10: bVal = bPrintProspect      ; pValues[nProp].setValue(&bVal, rType);  break;
            case 11: bVal = bPrintSingleJobs     ; pValues[nProp].setValue(&bVal, rType);  break;
            case 12: pValues[nProp] <<= sFaxName;  break;
            case 13: bVal = bPaperFromSetup     ; pValues[nProp].setValue(&bVal, rType);  break;
        }
    }
    PutProperties(aNames, aValues);
}
/* -----------------------------06.09.00 16:46--------------------------------

 ---------------------------------------------------------------------------*/
void SwPrintOptions::Notify( const Sequence<rtl::OUString>& aPropertyNames)
{
    DBG_ERROR("properties have been changed")
}




