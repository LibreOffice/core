/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: prtopt.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 12:47:15 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"



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

#include <unomid.h>

using namespace utl;
using rtl::OUString;
using namespace com::sun::star::uno;

/*--------------------------------------------------------------------
     Beschreibung: Ctor
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
        "Content/PrintPlaceholders",     // 16 not in Sw/Web
        "Content/PrintHiddenText"      // 17
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
/* -----------------------------06.09.00 16:44--------------------------------

 ---------------------------------------------------------------------------*/
SwPrintOptions::SwPrintOptions(sal_Bool bWeb) :
    ConfigItem(bWeb ? C2U("Office.WriterWeb/Print") :  C2U("Office.Writer/Print"),
        CONFIG_MODE_DELAYED_UPDATE|CONFIG_MODE_RELEASE_TREE),
    bIsWeb(bWeb)
{
    bPrintPageBackground = !bWeb;
    bPrintBlackFont = bWeb;
    bPrintTextPlaceholder = bPrintHiddenText = sal_False;

    Sequence<OUString> aNames = GetPropertyNames();
    Sequence<Any> aValues = GetProperties(aNames);
    const Any* pValues = aValues.getConstArray();
    DBG_ASSERT(aValues.getLength() == aNames.getLength(), "GetProperties failed")
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
                    case  8: bPrintProspect_RTL  = *(sal_Bool*)pValues[nProp].getValue();  break;
                    case  9: bPrintSingleJobs   = *(sal_Bool*)pValues[nProp].getValue();  break;
                    case 10: pValues[nProp] >>= sFaxName;  break;
                    case 11: bPaperFromSetup    = *(sal_Bool*)pValues[nProp].getValue();  break;
                    case 12: bPrintDraw         = *(sal_Bool*)pValues[nProp].getValue() ;  break;
                    case 13: bPrintLeftPage     = *(sal_Bool*)pValues[nProp].getValue();  break;
                    case 14: bPrintRightPage        = *(sal_Bool*)pValues[nProp].getValue();  break;
                    case 15: bPrintEmptyPages       = *(sal_Bool*)pValues[nProp].getValue();  break;
                    case 16: bPrintTextPlaceholder = *(sal_Bool*)pValues[nProp].getValue();  break;
                    case 17: bPrintHiddenText = *(sal_Bool*)pValues[nProp].getValue();  break;
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

    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    const Type& rType = ::getBooleanCppuType();
    BOOL bVal;
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
            case  8: bVal = bPrintProspect_RTL      ; pValues[nProp].setValue(&bVal, rType);  break;
            case  9: bVal = bPrintSingleJobs     ; pValues[nProp].setValue(&bVal, rType);  break;
            case 10: pValues[nProp] <<= sFaxName;  break;
            case 11: bVal = bPaperFromSetup     ; pValues[nProp].setValue(&bVal, rType);  break;
            case 12: bVal = bPrintDraw           ; pValues[nProp].setValue(&bVal, rType);  break;
            case 13: bVal = bPrintLeftPage       ; pValues[nProp].setValue(&bVal, rType);  break;
            case 14: bVal = bPrintRightPage     ; pValues[nProp].setValue(&bVal, rType);  break;
            case 15: bVal = bPrintEmptyPages    ; pValues[nProp].setValue(&bVal, rType);  break;
            case 16: bVal = bPrintTextPlaceholder; pValues[nProp].setValue(&bVal, rType);  break;
            case 17: bVal = bPrintHiddenText; pValues[nProp].setValue(&bVal, rType);  break;
        }
    }
    PutProperties(aNames, aValues);
}




