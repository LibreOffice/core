/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <tools/stream.hxx>
#include <unotools/syslocale.hxx>

#include "swtypes.hxx"
#include "hintids.hxx"
#include "uitool.hxx"
#include "usrpref.hxx"
#include "crstate.hxx"
#include <linguistic/lngprops.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <unotools/localedatawrapper.hxx>

#include <unomid.h>

using namespace utl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;



void SwMasterUsrPref::SetUsrPref(const SwViewOption &rCopy)
{
    *((SwViewOption*)this) = rCopy;
}

SwMasterUsrPref::SwMasterUsrPref(sal_Bool bWeb) :
    eFldUpdateFlags(AUTOUPD_OFF),
    nLinkUpdateMode(0),
    bIsHScrollMetricSet(sal_False),
    bIsVScrollMetricSet(sal_False),
    nDefTab( MM50 * 4 ),
    bIsSquaredPageMode(false),
    bIsAlignMathObjectsToBaseline(false),
    aContentConfig(bWeb, *this),
    aLayoutConfig(bWeb, *this),
    aGridConfig(bWeb, *this),
    aCursorConfig(*this),
    pWebColorConfig(bWeb ? new SwWebColorConfig(*this) : 0),
    bApplyCharUnit(sal_False)
{
    MeasurementSystem eSystem = SvtSysLocale().GetLocaleData().getMeasurementSystemEnum();
    eUserMetric = MEASURE_METRIC == eSystem ? FUNIT_CM : FUNIT_INCH;
    eHScrollMetric = eVScrollMetric = eUserMetric;

    aContentConfig.Load();
    aLayoutConfig.Load();
    aGridConfig.Load();
    aCursorConfig.Load();
    if(pWebColorConfig)
        pWebColorConfig->Load();
}

SwMasterUsrPref::~SwMasterUsrPref()
{
    delete pWebColorConfig;
}

Sequence<OUString> SwContentViewConfig::GetPropertyNames()
{
    static const char* aPropNames[] =
    {
        "Display/GraphicObject",                    
        "Display/Table",                            
        "Display/DrawingControl",                   
        "Display/FieldCode",                        
        "Display/Note",                             
        "Display/ShowContentTips",                      
        "NonprintingCharacter/MetaCharacters",     
        "NonprintingCharacter/ParagraphEnd",        
        "NonprintingCharacter/OptionalHyphen",      
        "NonprintingCharacter/Space",               
        "NonprintingCharacter/Break",               
        "NonprintingCharacter/ProtectedSpace",      
        "NonprintingCharacter/Tab",             
        "NonprintingCharacter/HiddenText",      
        "NonprintingCharacter/HiddenParagraph", 
        "NonprintingCharacter/HiddenCharacter",      
        "Update/Link",                          
        "Update/Field",                         
        "Update/Chart"                          


    };
    const int nCount = bWeb ? 12 : 19;
    Sequence<OUString> aNames(nCount);
    OUString* pNames = aNames.getArray();
    for(int i = 0; i < nCount; i++)
    {
        pNames[i] = OUString::createFromAscii(aPropNames[i]);
    }
    return aNames;
}

SwContentViewConfig::SwContentViewConfig(sal_Bool bIsWeb, SwMasterUsrPref& rPar) :
    ConfigItem(bIsWeb ? OUString("Office.WriterWeb/Content") :  OUString("Office.Writer/Content")),
    rParent(rPar),
    bWeb(bIsWeb)
{
    Load();
    EnableNotification( GetPropertyNames() );
}

SwContentViewConfig::~SwContentViewConfig()
{
}

void SwContentViewConfig::Notify( const Sequence< OUString > & /*rPropertyNames*/ )
{
    Load();
}

void SwContentViewConfig::Commit()
{
    Sequence<OUString> aNames = GetPropertyNames();

    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    for(int nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        sal_Bool bVal = sal_False;
        switch(nProp)
        {
            case  0: bVal = rParent.IsGraphic();    break;
            case  1: bVal = rParent.IsTable();  break;
            case  2: bVal = rParent.IsDraw();       break;
            case  3: bVal = rParent.IsFldName();    break;
            case  4: bVal = rParent.IsPostIts();    break;
            case  5: bVal = rParent.IsShowContentTips(); break; 
            case  6: bVal = rParent.IsViewMetaChars(); break; 
            case  7: bVal = rParent.IsParagraph(sal_True); break;
            case  8: bVal = rParent.IsSoftHyph(); break;
            case  9: bVal = rParent.IsBlank(sal_True);  break;
            case 10: bVal = rParent.IsLineBreak(sal_True);break;
            case 11: bVal = rParent.IsHardBlank(); break;
            case 12: bVal = rParent.IsTab(sal_True);        break;
            case 13: bVal = rParent.IsShowHiddenField(); break;
            case 14: bVal = rParent.IsShowHiddenPara(); break;
            case 15: bVal = rParent.IsShowHiddenChar(sal_True);    break;
            case 16: pValues[nProp] <<= rParent.GetUpdateLinkMode();    break;
            case 17: bVal = rParent.IsUpdateFields(); break;
            case 18: bVal = rParent.IsUpdateCharts(); break;
        }
        if(nProp != 16)
            pValues[nProp].setValue(&bVal, ::getBooleanCppuType());
    }
    PutProperties(aNames, aValues);
}

void SwContentViewConfig::Load()
{
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
                sal_Bool bSet = nProp != 16 ? *(sal_Bool*)pValues[nProp].getValue() : sal_False;
                switch(nProp)
                {
                    case  0: rParent.SetGraphic(bSet);  break;
                    case  1: rParent.SetTable(bSet);    break;
                    case  2: rParent.SetDraw(bSet);     break;
                    case  3: rParent.SetFldName(bSet);  break;
                    case  4: rParent.SetPostIts(bSet);  break;
                    case  5: rParent.SetShowContentTips(bSet);  break;
                    case  6: rParent.SetViewMetaChars(bSet); break; 
                    case  7: rParent.SetParagraph(bSet); break;
                    case  8: rParent.SetSoftHyph(bSet); break;
                    case  9: rParent.SetBlank(bSet);    break;
                    case 10: rParent.SetLineBreak(bSet);break;
                    case 11: rParent.SetHardBlank(bSet); break;
                    case 12: rParent.SetTab(bSet);      break;
                    case 13: rParent.SetShowHiddenField(bSet);   break;
                    case 14: rParent.SetShowHiddenPara(bSet); break;
                    case 15: rParent.SetShowHiddenChar(bSet); break;
                    case 16:
                    {
                        sal_Int32 nSet = 0;
                        pValues[nProp] >>= nSet;
                        rParent.SetUpdateLinkMode(nSet, sal_True);
                    }
                    break;
                    case 17: rParent.SetUpdateFields(bSet, sal_True); break;
                    case 18: rParent.SetUpdateCharts(bSet, sal_True); break;
                }
            }
        }
    }
}

Sequence<OUString> SwLayoutViewConfig::GetPropertyNames()
{
    static const char* aPropNames[] =
    {
        "Line/Guide",                           
        "Window/HorizontalScroll",              
        "Window/VerticalScroll",                
        "Window/ShowRulers",                    
        "Window/HorizontalRuler",               
        "Window/VerticalRuler",                 
        "Window/HorizontalRulerUnit",           
        "Window/VerticalRulerUnit",             
        "Window/SmoothScroll",                  
        "Zoom/Value",                           
        "Zoom/Type",                            
        "Other/IsAlignMathObjectsToBaseline",   
        "Other/MeasureUnit",                    
        
        "Other/TabStop",                        
        "Window/IsVerticalRulerRight",          
        "ViewLayout/Columns",                   
        "ViewLayout/BookMode",                  
        "Other/IsSquaredPageMode",              
        "Other/ApplyCharUnit",                  
        "Window/ShowScrollBarTips"              
    };
    const int nCount = bWeb ? 13 : 20;
    Sequence<OUString> aNames(nCount);
    OUString* pNames = aNames.getArray();
    for(int i = 0; i < nCount; i++)
    {
        pNames[i] = OUString::createFromAscii(aPropNames[i]);
    }
    return aNames;
}

SwLayoutViewConfig::SwLayoutViewConfig(sal_Bool bIsWeb, SwMasterUsrPref& rPar) :
    ConfigItem(bIsWeb ? OUString("Office.WriterWeb/Layout") :  OUString("Office.Writer/Layout"),
        CONFIG_MODE_DELAYED_UPDATE|CONFIG_MODE_RELEASE_TREE),
    rParent(rPar),
    bWeb(bIsWeb)
{
}

SwLayoutViewConfig::~SwLayoutViewConfig()
{
}

void SwLayoutViewConfig::Commit()
{
    Sequence<OUString> aNames = GetPropertyNames();

    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    for(int nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        Any &rVal = pValues[nProp];
        switch(nProp)
        {
            case  0: rVal <<= (sal_Bool) rParent.IsCrossHair(); break;              
            case  1: rVal <<= (sal_Bool) rParent.IsViewHScrollBar(); break;         
            case  2: rVal <<= (sal_Bool) rParent.IsViewVScrollBar(); break;         
            case  3: rVal <<= (sal_Bool) rParent.IsViewAnyRuler(); break;           
            
            
            case  4: rVal <<= (sal_Bool) rParent.IsViewHRuler(sal_True); break;         
            case  5: rVal <<= (sal_Bool) rParent.IsViewVRuler(sal_True); break;         
            case  6:
                if(rParent.bIsHScrollMetricSet)
                    rVal <<= (sal_Int32)rParent.eHScrollMetric;                     
            break;
            case  7:
                if(rParent.bIsVScrollMetricSet)
                    rVal <<= (sal_Int32)rParent.eVScrollMetric;                     
            break;
            case  8: rVal <<= (sal_Bool) rParent.IsSmoothScroll(); break;           
            case  9: rVal <<= (sal_Int32)rParent.GetZoom(); break;                  
            case 10: rVal <<= (sal_Int32)rParent.GetZoomType(); break;              
            case 11: rVal <<= (sal_Bool) rParent.IsAlignMathObjectsToBaseline(); break;      
            case 12: rVal <<= (sal_Int32)rParent.GetMetric(); break;                
            case 13: rVal <<= static_cast<sal_Int32>(TWIP_TO_MM100(rParent.GetDefTab())); break;
            case 14: rVal <<= (sal_Bool) rParent.IsVRulerRight(); break;            
            case 15: rVal <<= (sal_Int32)rParent.GetViewLayoutColumns(); break;     
            case 16: rVal <<= (sal_Bool) rParent.IsViewLayoutBookMode(); break;     
            case 17: rVal <<= (sal_Bool) rParent.IsSquaredPageMode(); break;        
            case 18: rVal <<= (sal_Bool) rParent.IsApplyCharUnit(); break;        
            case 19: rVal <<= (sal_Bool) rParent.IsShowScrollBarTips(); break;      
        }
    }
    PutProperties(aNames, aValues);
}

void SwLayoutViewConfig::Load()
{
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
                sal_Int32   nInt32Val   = 0;
                sal_Bool    bSet        = sal_False;
                pValues[nProp] >>= nInt32Val;
                pValues[nProp] >>= bSet;

                switch(nProp)
                {
                    case  0: rParent.SetCrossHair(bSet); break;
                    case  1: rParent.SetViewHScrollBar(bSet); break;
                    case  2: rParent.SetViewVScrollBar(bSet); break;
                    case  3: rParent.SetViewAnyRuler(bSet);break; 
                    case  4: rParent.SetViewHRuler(bSet); break;
                    case  5: rParent.SetViewVRuler(bSet); break;
                    case  6:
                    {
                        rParent.bIsHScrollMetricSet = sal_True;
                        rParent.eHScrollMetric = ((FieldUnit)nInt32Val);  
                    }
                    break;
                    case  7:
                    {
                        rParent.bIsVScrollMetricSet = sal_True;
                        rParent.eVScrollMetric = ((FieldUnit)nInt32Val); 
                    }
                    break;
                    case  8: rParent.SetSmoothScroll(bSet); break;
                    case  9: rParent.SetZoom( static_cast< sal_uInt16 >(nInt32Val) ); break;
                    case 10: rParent.SetZoomType( static_cast< SvxZoomType >(nInt32Val) ); break;
                    case 11: rParent.SetAlignMathObjectsToBaseline(bSet); break;
                    case 12: rParent.SetMetric((FieldUnit)nInt32Val, sal_True); break;
                    case 13: rParent.SetDefTab(MM100_TO_TWIP(nInt32Val), sal_True); break;
                    case 14: rParent.SetVRulerRight(bSet); break;
                    case 15: rParent.SetViewLayoutColumns( static_cast<sal_uInt16>(nInt32Val) ); break;
                    case 16: rParent.SetViewLayoutBookMode(bSet); break;
                    case 17: rParent.SetDefaultPageMode(bSet,sal_True); break;
                    case 18: rParent.SetApplyCharUnit(bSet); break;
                    case 19: rParent.SetShowScrollBarTips(bSet); break;
                }
            }
        }
    }
}

void SwLayoutViewConfig::Notify( const ::com::sun::star::uno::Sequence< OUString >& ) {}

Sequence<OUString> SwGridConfig::GetPropertyNames()
{
    static const char* aPropNames[] =
    {
        "Option/SnapToGrid",            
        "Option/VisibleGrid",           
        "Option/Synchronize",           
        "Resolution/XAxis",             
        "Resolution/YAxis",             
        "Subdivision/XAxis",            
        "Subdivision/YAxis"             
    };
    const int nCount = 7;
    Sequence<OUString> aNames(nCount);
    OUString* pNames = aNames.getArray();
    for(int i = 0; i < nCount; i++)
    {
        pNames[i] = OUString::createFromAscii(aPropNames[i]);
    }
    return aNames;
}

SwGridConfig::SwGridConfig(sal_Bool bIsWeb, SwMasterUsrPref& rPar) :
    ConfigItem(bIsWeb ? OUString("Office.WriterWeb/Grid") :  OUString("Office.Writer/Grid"),
        CONFIG_MODE_DELAYED_UPDATE|CONFIG_MODE_RELEASE_TREE),
    rParent(rPar)
{
}

SwGridConfig::~SwGridConfig()
{
}

void SwGridConfig::Commit()
{
    Sequence<OUString> aNames = GetPropertyNames();

    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    for(int nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        sal_Bool bSet;
        switch(nProp)
        {
            case  0: bSet = rParent.IsSnap(); break;
            case  1: bSet = rParent.IsGridVisible(); break;
            case  2: bSet = rParent.IsSynchronize(); break;
            case  3: pValues[nProp] <<= (sal_Int32)TWIP_TO_MM100(rParent.GetSnapSize().Width()); break;
            case  4: pValues[nProp] <<= (sal_Int32)TWIP_TO_MM100(rParent.GetSnapSize().Height()); break;
            case  5: pValues[nProp] <<= (sal_Int16)rParent.GetDivisionX(); break;
            case  6: pValues[nProp] <<= (sal_Int16)rParent.GetDivisionY(); break;
        }
        if(nProp < 3)
              pValues[nProp].setValue(&bSet, ::getBooleanCppuType());
    }
    PutProperties(aNames, aValues);
}

void SwGridConfig::Load()
{
    Sequence<OUString> aNames = GetPropertyNames();
    Sequence<Any> aValues = GetProperties(aNames);
    const Any* pValues = aValues.getConstArray();
    OSL_ENSURE(aValues.getLength() == aNames.getLength(), "GetProperties failed");
    if(aValues.getLength() == aNames.getLength())
    {
        Size aSnap(rParent.GetSnapSize());
        for(int nProp = 0; nProp < aNames.getLength(); nProp++)
        {
            if(pValues[nProp].hasValue())
            {
                sal_Bool bSet = nProp < 3 ? *(sal_Bool*)pValues[nProp].getValue() : sal_False;
                sal_Int32 nSet = 0;
                if(nProp >= 3)
                    pValues[nProp] >>= nSet;
                switch(nProp)
                {
                    case  0: rParent.SetSnap(bSet); break;
                    case  1: rParent.SetGridVisible(bSet); break;
                    case  2: rParent.SetSynchronize(bSet); break;
                    case  3: aSnap.Width() = MM100_TO_TWIP(nSet); break;
                    case  4: aSnap.Height() = MM100_TO_TWIP(nSet); break;
                    case  5: rParent.SetDivisionX((short)nSet); break;
                    case  6: rParent.SetDivisionY((short)nSet); break;
                }
            }
        }
        rParent.SetSnapSize(aSnap);
    }
}

void SwGridConfig::Notify( const ::com::sun::star::uno::Sequence< OUString >& ) {}

Sequence<OUString> SwCursorConfig::GetPropertyNames()
{
    static const char* aPropNames[] =
    {
        "DirectCursor/UseDirectCursor", 
        "DirectCursor/Insert",          
        "Option/ProtectedArea"          
    };
    const int nCount = 3;
    Sequence<OUString> aNames(nCount);
    OUString* pNames = aNames.getArray();
    for(int i = 0; i < nCount; i++)
        pNames[i] = OUString::createFromAscii(aPropNames[i]);
    return aNames;
}

SwCursorConfig::SwCursorConfig(SwMasterUsrPref& rPar) :
    ConfigItem("Office.Writer/Cursor",
        CONFIG_MODE_DELAYED_UPDATE|CONFIG_MODE_RELEASE_TREE),
    rParent(rPar)
{
}

SwCursorConfig::~SwCursorConfig()
{
}

void SwCursorConfig::Commit()
{
    Sequence<OUString> aNames = GetPropertyNames();

    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    for(int nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        sal_Bool bSet;
        switch(nProp)
        {
            case  0: bSet = rParent.IsShadowCursor();       break;
            case  1: pValues[nProp] <<= (sal_Int32)rParent.GetShdwCrsrFillMode();   break;
            case  2: bSet = rParent.IsCursorInProtectedArea(); break;
        }
        if(nProp != 1 )
              pValues[nProp].setValue(&bSet, ::getBooleanCppuType());
    }
    PutProperties(aNames, aValues);
}

void SwCursorConfig::Load()
{
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
                sal_Bool bSet = sal_False;
                sal_Int32 nSet = 0;
                if(nProp != 1 )
                    bSet = *(sal_Bool*)pValues[nProp].getValue();
                else
                    pValues[nProp] >>= nSet;
                switch(nProp)
                {
                    case  0: rParent.SetShadowCursor(bSet);         break;
                    case  1: rParent.SetShdwCrsrFillMode((sal_uInt8)nSet); break;
                    case  2: rParent.SetCursorInProtectedArea(bSet); break;
                }
            }
        }

    }
}

void SwCursorConfig::Notify( const ::com::sun::star::uno::Sequence< OUString >& ) {}

SwWebColorConfig::SwWebColorConfig(SwMasterUsrPref& rPar) :
    ConfigItem("Office.WriterWeb/Background",
        CONFIG_MODE_DELAYED_UPDATE|CONFIG_MODE_RELEASE_TREE),
    rParent(rPar),
    aPropNames(1)
{
    aPropNames.getArray()[0] = "Color";
}

SwWebColorConfig::~SwWebColorConfig()
{
}

void SwWebColorConfig::Commit()
{
    Sequence<Any> aValues(aPropNames.getLength());
    Any* pValues = aValues.getArray();
    for(int nProp = 0; nProp < aPropNames.getLength(); nProp++)
    {
        switch(nProp)
        {
            case  0: pValues[nProp] <<= (sal_Int32)rParent.GetRetoucheColor().GetColor();   break;
        }
    }
    PutProperties(aPropNames, aValues);
}

void SwWebColorConfig::Notify( const ::com::sun::star::uno::Sequence< OUString >& ) {}

void SwWebColorConfig::Load()
{
    Sequence<Any> aValues = GetProperties(aPropNames);
    const Any* pValues = aValues.getConstArray();
    OSL_ENSURE(aValues.getLength() == aPropNames.getLength(), "GetProperties failed");
    if(aValues.getLength() == aPropNames.getLength())
    {
        for(int nProp = 0; nProp < aPropNames.getLength(); nProp++)
        {
            if(pValues[nProp].hasValue())
            {
                switch(nProp)
                {
                    case  0:
                        sal_Int32 nSet = 0;
                        pValues[nProp] >>= nSet; rParent.SetRetoucheColor(nSet);
                    break;
                }
            }
        }
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
