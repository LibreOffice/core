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
#include <vcl/EnumContext.hxx>

#include <osl/diagnose.h>
#include <o3tl/enumarray.hxx>

#include <map>

namespace vcl {

namespace {

typedef ::std::map<rtl::OUString,EnumContext::Application> ApplicationMap;

static ApplicationMap maApplicationMap;
static o3tl::enumarray<EnumContext::Application, rtl::OUString> maApplicationVector;

typedef ::std::map<rtl::OUString,EnumContext::Context> ContextMap;

static ContextMap maContextMap;
static ::std::vector<rtl::OUString> maContextVector;

}

const sal_Int32 EnumContext::NoMatch = 4;
const sal_Int32 EnumContext::OptimalMatch = 0;  // Neither application nor context name is "any".

EnumContext::EnumContext()
    : meApplication(Application::NONE),
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
         case Application::Draw:
         case Application::Impress:
            return Application::DrawImpress;

         case Application::Writer:
         case Application::WriterGlobal:
         case Application::WriterWeb:
         case Application::WriterXML:
         case Application::WriterForm:
         case Application::WriterReport:
             return Application::WriterVariants;

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
    OSL_ASSERT(eApplication<=Application::LAST);
    maApplicationVector[eApplication]=rsName;
}

void EnumContext::ProvideApplicationContainers()
{
    if (maApplicationMap.empty())
    {
        AddEntry("com.sun.star.text.TextDocument", EnumContext::Application::Writer);
        AddEntry("com.sun.star.text.GlobalDocument", EnumContext::Application::WriterGlobal);
        AddEntry("com.sun.star.text.WebDocument", EnumContext::Application::WriterWeb);
        AddEntry("com.sun.star.xforms.XMLFormDocument", EnumContext::Application::WriterXML);
        AddEntry("com.sun.star.sdb.FormDesign", EnumContext::Application::WriterForm);
        AddEntry("com.sun.star.sdb.TextReportDesign", EnumContext::Application::WriterReport);
        AddEntry("com.sun.star.sheet.SpreadsheetDocument", EnumContext::Application::Calc);
        AddEntry("com.sun.star.chart2.ChartDocument", EnumContext::Application::Chart);
        AddEntry("com.sun.star.drawing.DrawingDocument", EnumContext::Application::Draw);
        AddEntry("com.sun.star.presentation.PresentationDocument", EnumContext::Application::Impress);

        AddEntry("any", EnumContext::Application::Any);
        AddEntry("none", EnumContext::Application::NONE);
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
        return EnumContext::Application::NONE;
}

const ::rtl::OUString& EnumContext::GetApplicationName (const Application eApplication)
{
    ProvideApplicationContainers();
    return maApplicationVector[eApplication];
}

void EnumContext::AddEntry (const ::rtl::OUString& rsName, const Context eContext)
{
    maContextMap[rsName] = eContext;
    OSL_ASSERT(eContext<=LastContextEnum);
    if (maContextVector.size() <= size_t(eContext))
        maContextVector.resize(eContext+1);
    maContextVector[eContext] = rsName;
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

} // end of namespace vcl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
