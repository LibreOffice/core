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
static o3tl::enumarray<EnumContext::Context, rtl::OUString> maContextVector;

}

const sal_Int32 EnumContext::NoMatch = 4;
const sal_Int32 EnumContext::OptimalMatch = 0;  // Neither application nor context name is "any".

EnumContext::EnumContext()
    : meApplication(Application::NONE),
      meContext(Context::Unknown)
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
    if (!maApplicationMap.empty())
        return;

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
    maContextVector[eContext] = rsName;
}

void EnumContext::ProvideContextContainers()
{
    if (!maContextMap.empty())
        return;

    AddEntry("any", Context::Any);
    AddEntry("default", Context::Default);
    AddEntry("empty", Context::Empty);
    AddEntry("3DObject", Context::ThreeDObject);
    AddEntry("Annotation", Context::Annotation);
    AddEntry("Auditing", Context::Auditing);
    AddEntry("Axis", Context::Axis);
    AddEntry("Cell", Context::Cell);
    AddEntry("Chart", Context::Chart);
    AddEntry("ChartElements", Context::ChartElements);
    AddEntry("Draw", Context::Draw);
    AddEntry("DrawLine", Context::DrawLine);
    AddEntry("DrawPage", Context::DrawPage);
    AddEntry("DrawText", Context::DrawText);
    AddEntry("EditCell", Context::EditCell);
    AddEntry("ErrorBar", Context::ErrorBar);
    AddEntry("Form", Context::Form);
    AddEntry("Frame", Context::Frame);
    AddEntry("Graphic", Context::Graphic);
    AddEntry("Grid", Context::Grid);
    AddEntry("HandoutPage", Context::HandoutPage);
    AddEntry("MasterPage", Context::MasterPage);
    AddEntry("Media", Context::Media);
    AddEntry("MultiObject", Context::MultiObject);
    AddEntry("NotesPage", Context::NotesPage);
    AddEntry("OLE", Context::OLE);
    AddEntry("OutlineText", Context::OutlineText);
    AddEntry("Pivot", Context::Pivot);
    AddEntry("Series", Context::Series);
    AddEntry("SlidesorterPage", Context::SlidesorterPage);
    AddEntry("Table", Context::Table);
    AddEntry("Text", Context::Text);
    AddEntry("TextObject", Context::TextObject);
    AddEntry("Trendline", Context::Trendline);

}

EnumContext::Context EnumContext::GetContextEnum (const ::rtl::OUString& rsContextName)
{
    ProvideContextContainers();

    ContextMap::const_iterator iContext( maContextMap.find(rsContextName) );
    if (iContext != maContextMap.end())
        return iContext->second;
    else
        return EnumContext::Context::Unknown;
}

const ::rtl::OUString& EnumContext::GetContextName (const Context eContext)
{
    ProvideContextContainers();
    return maContextVector[eContext];
}

} // end of namespace vcl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
