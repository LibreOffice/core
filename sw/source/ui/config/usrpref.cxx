/*************************************************************************
 *
 *  $RCSfile: usrpref.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: os $ $Date: 2000-10-10 08:31:19 $
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

#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif


#include "swtypes.hxx"
#include "hintids.hxx"
#include "uitool.hxx"
#include "usrpref.hxx"
#include "crstate.hxx"
#ifndef _OFA_OSPLCFG_HXX
#include <offmgr/osplcfg.hxx>
#endif
#ifndef _OFF_APP_HXX //autogen
#include <offmgr/app.hxx>
#endif

#ifndef _LINGU_LNGPROPS_HHX_
#include <lingu/lngprops.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

using namespace utl;
using namespace rtl;
using namespace ::com::sun::star;
using namespace com::sun::star::uno;

#define C2U(cChar) OUString::createFromAscii(cChar)
#define C2S(cChar) String::CreateFromAscii(cChar)

sal_Bool bNotLoadLayout = sal_False;    // fuer MD

void SwMasterUsrPref::SetUsrPref(const SwViewOption &rCopy)
{
    *((SwViewOption*)this) = rCopy;

#ifndef PRODUCT
    bNotLoadLayout = rCopy.IsTest1();       // MD Layout lesen
#endif
}

SwMasterUsrPref::SwMasterUsrPref(BOOL bWeb) :
    aContentConfig(bWeb, *this),
    aLayoutConfig(bWeb, *this)
{
    aContentConfig.Load();
    aLayoutConfig.Load();
}
/*-- 28.09.00 09:55:32---------------------------------------------------

  -----------------------------------------------------------------------*/
Sequence<OUString> SwContentViewConfig::GetPropertyNames()
{
    static const char* aPropNames[] =
    {
        "Display/GraphicObject",                    //  0
        "Display/Table",                            //  1
        "Display/DrawingControl",                   //  2
        "Display/FieldCode",                        //  3
        "Display/Note",                             //  4
        "Highlighting/Field",                       //  5
        "NonprintingCharacter/ParagraphEnd",        //  6
        "NonprintingCharacter/OptionalHyphen",      //  7
        "NonprintingCharacter/Space",               //  8
        "NonprintingCharacter/ProtectedSpace",      //  9
        "NonprintingCharacter/Break",               // 10
             "Highlighting/IndexEntry",             // 11 //not in Writer/Web
            "Highlighting/Index",                   // 12
            "Highlighting/Footnote",                // 13
            "NonprintingCharacter/Tab",             // 14
            "NonprintingCharacter/HiddenText",      // 15
            "NonprintingCharacter/HiddenParagraph", // 16
            "Update/Link",                          // 17
            "Update/Field",                         // 18
            "Update/Chart"                          // 19
    };
    const int nCount = bWeb ? 11 : 20;
    Sequence<OUString> aNames(nCount);
    OUString* pNames = aNames.getArray();
    for(int i = 0; i < nCount; i++)
    {
        pNames[i] = OUString::createFromAscii(aPropNames[i]);
    }
    return aNames;
}
/*-- 28.09.00 09:55:33---------------------------------------------------

  -----------------------------------------------------------------------*/
SwContentViewConfig::SwContentViewConfig(BOOL bIsWeb, SwMasterUsrPref& rPar) :
    ConfigItem(bIsWeb ? C2U("Office.WriterWeb/Content") :  C2U("Office.Writer/Content")),
    rParent(rPar),
    bWeb(bIsWeb)
{
}
/*-- 28.09.00 09:55:33---------------------------------------------------

  -----------------------------------------------------------------------*/
SwContentViewConfig::~SwContentViewConfig()
{
}
/*-- 28.09.00 09:55:33---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwContentViewConfig::Notify( const com::sun::star::uno::Sequence<rtl::OUString>& aPropertyNames)
{
    Load();
}
/*-- 28.09.00 09:55:33---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwContentViewConfig::Commit()
{
    Sequence<OUString> aNames = GetPropertyNames();

    OUString* pNames = aNames.getArray();
    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    const Type& rType = ::getBooleanCppuType();
    for(int nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        sal_Bool bVal = FALSE;
        switch(nProp)
        {
            case  0: bVal = rParent.IsGraphic();    break;// "Display/GraphicObject",
            case  1: bVal = rParent.IsTable();  break;// "Display/Table",
            case  2: bVal = rParent.IsDraw();       break;// "Display/DrawingControl",
            case  3: bVal = rParent.IsFldName();    break;// "Display/FieldCode",
            case  4: bVal = rParent.IsPostIts();    break;// "Display/Note",
            case  5: bVal = rParent.IsField();  break;// "Highlighting/Field",
            case  6: bVal = rParent.IsPageBreak(); break;// "NonprintingCharacter/ParagraphEnd",
            case  7: bVal = rParent.IsSoftHyph(); break;// "NonprintingCharacter/OptionalHyphen",
            case  8: bVal = rParent.IsBlank();  break;// "NonprintingCharacter/Space",
            case  9: bVal = rParent.IsHardBlank(); break;// "NonprintingCharacter/ProtectedSpace",
            case 10: bVal = rParent.IsLineBreak();break;// "NonprintingCharacter/Break",
            case 11: bVal = rParent.IsTox();        break;// "Highlighting/IndexEntry",
            case 12: bVal = rParent.IsIndexBackground(); break;// "Highlighting/Index",
            case 13: bVal = rParent.IsFootNote(); break;// "Highlighting/Footnote",
            case 14: bVal = rParent.IsTab();        break;// "NonprintingCharacter/Tab",
            case 15: bVal = rParent.IsHidden(); break;// "NonprintingCharacter/HiddenText",
            case 16: bVal = rParent.IsShowHiddenPara(); break;// "NonprintingCharacter/HiddenParagraph",
            case 17: pValues[nProp] <<= rParent.GetUpdateLinkMode();    break;// "Update/Link",
            case 18: bVal = rParent.IsUpdateFields(); break;// "Update/Field",
            case 19: bVal = rParent.IsUpdateCharts(); break;// "Update/Chart"
        }
        if(nProp != 17)
            pValues[nProp].setValue(&bVal, ::getBooleanCppuType());
    }
    PutProperties(aNames, aValues);
}
/*-- 28.09.00 09:55:34---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwContentViewConfig::Load()
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
            if(pValues[nProp].hasValue())
            {
                sal_Bool bSet = nProp != 17 ? *(sal_Bool*)pValues[nProp].getValue() : sal_False;
                switch(nProp)
                {
                    case  0: rParent.SetGraphic(bSet);  break;// "Display/GraphicObject",
                    case  1: rParent.SetTable(bSet);    break;// "Display/Table",
                    case  2: rParent.SetDraw(bSet);     break;// "Display/DrawingControl",
                    case  3: rParent.SetFldName(bSet);  break;// "Display/FieldCode",
                    case  4: rParent.SetPostIts(bSet);  break;// "Display/Note",
                    case  5: rParent.SetField(bSet);    break;// "Highlighting/Field",
                    case  6: rParent.SetPageBreak(bSet); break;// "NonprintingCharacter/ParagraphEnd",
                    case  7: rParent.SetSoftHyph(bSet); break;// "NonprintingCharacter/OptionalHyphen",
                    case  8: rParent.SetBlank(bSet);    break;// "NonprintingCharacter/Space",
                    case  9: rParent.SetHardBlank(bSet); break;// "NonprintingCharacter/ProtectedSpace",
                    case 10: rParent.SetLineBreak(bSet);break;// "NonprintingCharacter/Break",
                    case 11: rParent.SetTox(bSet);      break;// "Highlighting/IndexEntry",
                    case 12: rParent.SetIndexBackground(bSet); break;// "Highlighting/Index",
                    case 13: rParent.SetFootNote(bSet); break;// "Highlighting/Footnote",
                    case 14: rParent.SetTab(bSet);      break;// "NonprintingCharacter/Tab",
                    case 15: rParent.SetHidden(bSet);   break;// "NonprintingCharacter/HiddenText",
                    case 16: rParent.SetShowHiddenPara(bSet); break;// "NonprintingCharacter/HiddenParagraph",
                    case 17:
                    {
                        sal_Int32 nSet; pValues[nProp] >>= nSet;
                        rParent.SetUpdateLinkMode(nSet);
                    }
                    break;// "Update/Link",
                    case 18: rParent.SetUpdateFields(bSet); break;// "Update/Field",
                    case 19: rParent.SetUpdateCharts(bSet); break;// "Update/Chart"
                }
            }
        }
    }
}
/*-- 28.09.00 09:55:34---------------------------------------------------

  -----------------------------------------------------------------------*/
Sequence<OUString> SwLayoutViewConfig::GetPropertyNames()
{
    static const char* aPropNames[] =
    {
        "Line/TextBoundary",                // 0
        "Line/TableBoundary",               // 1
        "Line/SectionBoundary",             // 2
        "Line/Guide",                       // 3
        "Line/SimpleControlPoint",          // 4
        "Line/LargeControlPoint",           // 5
        "Window/HorizontalScroll",          // 6
        "Window/VerticalScroll",            // 7
        "Window/HorizontalRuler",           // 8
        "Window/VerticalRuler",             // 9
        "Window/SmoothScroll",              //10
        "Other/MeasureUnit",                //11
        "Other/TabStop",                    //12
    };
    const int nCount = bWeb ? 12 : 13;
    Sequence<OUString> aNames(nCount);
    OUString* pNames = aNames.getArray();
    for(int i = 0; i < nCount; i++)
    {
        pNames[i] = OUString::createFromAscii(aPropNames[i]);
    }
    return aNames;
}
/*-- 28.09.00 09:55:34---------------------------------------------------

  -----------------------------------------------------------------------*/
SwLayoutViewConfig::SwLayoutViewConfig(BOOL bIsWeb, SwMasterUsrPref& rPar) :
    ConfigItem(bIsWeb ? C2U("Office.WriterWeb/Layout") :  C2U("Office.Writer/Layout")),
    rParent(rPar),
    bWeb(bIsWeb)
{
}
/*-- 28.09.00 09:55:35---------------------------------------------------

  -----------------------------------------------------------------------*/
SwLayoutViewConfig::~SwLayoutViewConfig()
{
}
/*-- 28.09.00 09:55:35---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwLayoutViewConfig::Notify( const com::sun::star::uno::Sequence<rtl::OUString>& aPropertyNames)
{
    Load();
}
/*-- 28.09.00 09:55:36---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwLayoutViewConfig::Commit()
{
    Sequence<OUString> aNames = GetPropertyNames();

    OUString* pNames = aNames.getArray();
    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    const Type& rType = ::getBooleanCppuType();
    for(int nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        sal_Bool bSet;
        switch(nProp)
        {
            case  0: bSet = rParent.IsSubsLines(); break;// "Line/TextBoundary",
            case  1: bSet = rParent.IsSubsTable(); break;// "Line/TableBoundary",
            case  2: bSet = rParent.IsSectionBounds(); break;// "Line/SectionBoundary",
            case  3: bSet = rParent.IsCrossHair(); break;// "Line/Guide",
            case  4: bSet = rParent.IsSolidMarkHdl(); break;// "Line/SimpleControlPoint",
            case  5: bSet = rParent.IsBigMarkHdl(); break;// "Line/LargeControlPoint",
            case  6: bSet = rParent.IsViewHScrollBar(); break;// "Window/HorizontalScroll",
            case  7: bSet = rParent.IsViewVScrollBar(); break;// "Window/VerticalScroll",
            case  8: bSet = rParent.IsViewTabwin(); break;// "Window/HorizontalRuler",
            case  9: bSet = rParent.IsViewVLin(); break;// "Window/VerticalRuler",
            case 10: bSet = rParent.IsSmoothScroll(); break;// "Window/SmoothScroll",
            case 11: pValues[nProp] <<= (sal_Int32)rParent.GetMetric(); break;// "Other/MeasureUnit",
            case 12: pValues[nProp] <<= rParent.GetDefTab(); break;// "Other/TabStop",
        }
        if(nProp < 11)
            pValues[nProp].setValue(&bSet, ::getBooleanCppuType());
    }
    PutProperties(aNames, aValues);
}
/*-- 28.09.00 09:55:36---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwLayoutViewConfig::Load()
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
            if(pValues[nProp].hasValue())
            {
                sal_Bool bSet = nProp < 11 ? *(sal_Bool*)pValues[nProp].getValue() : sal_False;
                switch(nProp)
                {
                    case  0: rParent.SetSubsLines(bSet); break;// "Line/TextBoundary",
                    case  1: rParent.SetSubsTable(bSet); break;// "Line/TableBoundary",
                    case  2: rParent.SetSectionBounds(bSet); break;// "Line/SectionBoundary",
                    case  3: rParent.SetCrossHair(bSet); break;// "Line/Guide",
                    case  4: rParent.SetSolidMarkHdl(bSet); break;// "Line/SimpleControlPoint",
                    case  5: rParent.SetBigMarkHdl(bSet); break;// "Line/LargeControlPoint",
                    case  6: rParent.SetViewHScrollBar(bSet); break;// "Window/HorizontalScroll",
                    case  7: rParent.SetViewVScrollBar(bSet); break;// "Window/VerticalScroll",
                    case  8: rParent.SetViewTabwin(bSet); break;// "Window/HorizontalRuler",
                    case  9: rParent.SetViewVLin(bSet); break;// "Window/VerticalRuler",
                    case 10: rParent.SetSmoothScroll(bSet); break;// "Window/SmoothScroll",
                    case 11:
                    {
                        sal_Int32 nUnit; pValues[nProp] >>= nUnit;
                        rParent.SetMetric((FieldUnit)nUnit);
                    }
                    break;// "Other/MeasureUnit",
                    case 12:
                    {
                        sal_Int32 nTab; pValues[nProp] >>= nTab;
                        rParent.SetDefTab(nTab);
                    }
                    break;// "Other/TabStop",
                }
            }
        }
    }
}
