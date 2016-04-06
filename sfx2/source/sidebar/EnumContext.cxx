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
#include <sfx2/sidebar/EnumContext.hxx>

#include <osl/diagnose.h>

#include <map>

namespace sfx2 { namespace sidebar {

namespace {

typedef ::std::map<rtl::OUString,EnumContext::Application> ApplicationMap;

static ApplicationMap maApplicationMap;
static ::std::vector<rtl::OUString> maApplicationVector;

typedef ::std::map<rtl::OUString,EnumContext::Context> ContextMap;

static ContextMap maContextMap;
static ::std::vector<rtl::OUString> maContextVector;

}

const sal_Int32 EnumContext::NoMatch = 4;
const sal_Int32 EnumContext::OptimalMatch = 0;  // Neither application nor context name is "any".

EnumContext::EnumContext()
    : meApplication(Application_None),
      meContext(Context_Unknown)
{
}

EnumContext::EnumContext (
    const Application eApplication,
    const Context eContext)
    : meApplication(eApplication),
      meContext(eContext)
{
}

EnumContext::EnumContext (
    const ::rtl::OUString& rsApplicationName,
    const ::rtl::OUString& rsContextName)
    : meApplication(GetApplicationEnum(rsApplicationName)),
      meContext(GetContextEnum(rsContextName))
{
}

sal_Int32 EnumContext::GetCombinedContext_DI() const
{
    return CombinedEnumContext(GetApplication_DI(), meContext);
}

EnumContext::Application EnumContext::GetApplication_DI() const
{
     switch (meApplication)
     {
         case Application_Draw:
         case Application_Impress:
            return Application_DrawImpress;

         case Application_Writer:
         case Application_WriterGlobal:
         case Application_WriterWeb:
         case Application_WriterXML:
         case Application_WriterForm:
         case Application_WriterReport:
             return Application_WriterVariants;

         default:
             return meApplication;
     }
}

bool EnumContext::operator== (const EnumContext& rOther)
{
    return meApplication==rOther.meApplication
        && meContext==rOther.meContext;
}

bool EnumContext::operator!= (const EnumContext& rOther)
{
    return meApplication!=rOther.meApplication
        || meContext!=rOther.meContext;
}

void EnumContext::AddEntry (const ::rtl::OUString& rsName, const Application eApplication)
{
    maApplicationMap[rsName] = eApplication;
    OSL_ASSERT(eApplication<=LastApplicationEnum);
    if (maApplicationVector.size() <= size_t(eApplication))
        maApplicationVector.resize(eApplication+1);
    maApplicationVector[eApplication]=rsName;
}

void EnumContext::ProvideApplicationContainers()
{
    if (maApplicationMap.empty())
    {
        maApplicationVector.resize(static_cast<size_t>(EnumContext::LastApplicationEnum)+1);
        AddEntry("com.sun.star.text.TextDocument", EnumContext::Application_Writer);
        AddEntry("com.sun.star.text.GlobalDocument", EnumContext::Application_WriterGlobal);
        AddEntry("com.sun.star.text.WebDocument", EnumContext::Application_WriterWeb);
        AddEntry("com.sun.star.xforms.XMLFormDocument", EnumContext::Application_WriterXML);
        AddEntry("com.sun.star.sdb.FormDesign", EnumContext::Application_WriterForm);
        AddEntry("com.sun.star.sdb.TextReportDesign", EnumContext::Application_WriterReport);
        AddEntry("com.sun.star.sheet.SpreadsheetDocument", EnumContext::Application_Calc);
        AddEntry("com.sun.star.chart2.ChartDocument", EnumContext::Application_Chart);
        AddEntry("com.sun.star.drawing.DrawingDocument", EnumContext::Application_Draw);
        AddEntry("com.sun.star.presentation.PresentationDocument", EnumContext::Application_Impress);

        AddEntry("any", EnumContext::Application_Any);
        AddEntry("none", EnumContext::Application_None);
    }
}

EnumContext::Application EnumContext::GetApplicationEnum (const ::rtl::OUString& rsApplicationName)
{
    ProvideApplicationContainers();

    ApplicationMap::const_iterator iApplication(
        maApplicationMap.find(rsApplicationName));
    if (iApplication != maApplicationMap.end())
        return iApplication->second;
    else
        return EnumContext::Application_None;
}

const ::rtl::OUString& EnumContext::GetApplicationName (const Application eApplication)
{
    ProvideApplicationContainers();

    const sal_Int32 nIndex (eApplication);
    if (nIndex<0 || nIndex>= LastApplicationEnum)
        return maApplicationVector[Application_None];
    else
        return maApplicationVector[nIndex];
}

void EnumContext::AddEntry (const ::rtl::OUString& rsName, const Context eApplication)
{
    maContextMap[rsName] = eApplication;
    OSL_ASSERT(eApplication<=LastContextEnum);
    if (maContextVector.size() <= size_t(eApplication))
        maContextVector.resize(eApplication+1);
    maContextVector[eApplication] = rsName;
}

void EnumContext::ProvideContextContainers()
{
    if (maContextMap.empty())
    {
        maContextVector.resize(static_cast<size_t>(LastContextEnum)+1);
        AddEntry("any", Context_Any);
        AddEntry("default", Context_Default);
        AddEntry("empty", Context_Empty);
        AddEntry("3DObject", Context_3DObject);
        AddEntry("Annotation", Context_Annotation);
        AddEntry("Auditing", Context_Auditing);
        AddEntry("Axis", Context_Axis);
        AddEntry("Cell", Context_Cell);
        AddEntry("Chart", Context_Chart);
        AddEntry("ChartElements", Context_ChartElements);
        AddEntry("Draw", Context_Draw);
        AddEntry("DrawLine", Context_DrawLine);
        AddEntry("DrawPage", Context_DrawPage);
        AddEntry("DrawText", Context_DrawText);
        AddEntry("EditCell", Context_EditCell);
        AddEntry("ErrorBar", Context_ErrorBar);
        AddEntry("Form", Context_Form);
        AddEntry("Frame", Context_Frame);
        AddEntry("Graphic", Context_Graphic);
        AddEntry("Grid", Context_Grid);
        AddEntry("HandoutPage", Context_HandoutPage);
        AddEntry("MasterPage", Context_MasterPage);
        AddEntry("Media", Context_Media);
        AddEntry("MultiObject", Context_MultiObject);
        AddEntry("NotesPage", Context_NotesPage);
        AddEntry("OLE", Context_OLE);
        AddEntry("OutlineText", Context_OutlineText);
        AddEntry("Pivot", Context_Pivot);
        AddEntry("Series", Context_Series);
        AddEntry("SlidesorterPage", Context_SlidesorterPage);
        AddEntry("Table", Context_Table);
        AddEntry("Text", Context_Text);
        AddEntry("TextObject", Context_TextObject);
        AddEntry("Trendline", Context_Trendline);
    }
}

EnumContext::Context EnumContext::GetContextEnum (const ::rtl::OUString& rsContextName)
{
    ProvideContextContainers();

    ContextMap::const_iterator iContext( maContextMap.find(rsContextName) );
    if (iContext != maContextMap.end())
        return iContext->second;
    else
        return EnumContext::Context_Unknown;
}

const ::rtl::OUString& EnumContext::GetContextName (const Context eContext)
{
    ProvideContextContainers();

    const sal_Int32 nIndex (eContext);
    if (nIndex<0 || nIndex>= LastContextEnum)
        return maContextVector[Context_Unknown];
    else
        return maContextVector[nIndex];
}

} } // end of namespace sfx2::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
