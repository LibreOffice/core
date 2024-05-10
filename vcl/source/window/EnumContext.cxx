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

typedef ::std::map<OUString,EnumContext::Application> ApplicationMap;

ApplicationMap maApplicationMap;
o3tl::enumarray<EnumContext::Application, OUString> maApplicationVector;

typedef ::std::map<OUString,EnumContext::Context> ContextMap;

ContextMap maContextMap;
o3tl::enumarray<EnumContext::Context, OUString> maContextVector;

}

const sal_Int32 EnumContext::NoMatch = 4;

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

sal_Int32 EnumContext::GetCombinedContext_DI() const
{
    return CombinedEnumContext(GetApplication_DI(), meContext);
}

EnumContext::Application EnumContext::GetApplication() const
{
     return meApplication;
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

bool EnumContext::operator== (const EnumContext& rOther) const
{
    return meApplication==rOther.meApplication
        && meContext==rOther.meContext;
}

bool EnumContext::operator!= (const EnumContext& rOther) const
{
    return meApplication!=rOther.meApplication
        || meContext!=rOther.meContext;
}

void EnumContext::AddEntry (const OUString& rsName, const Application eApplication)
{
    maApplicationMap[rsName] = eApplication;
    OSL_ASSERT(eApplication<=Application::LAST);
    maApplicationVector[eApplication]=rsName;
}

void EnumContext::ProvideApplicationContainers()
{
    if (!maApplicationMap.empty())
        return;

    AddEntry(u"com.sun.star.text.TextDocument"_ustr, EnumContext::Application::Writer);
    AddEntry(u"com.sun.star.text.GlobalDocument"_ustr, EnumContext::Application::WriterGlobal);
    AddEntry(u"com.sun.star.text.WebDocument"_ustr, EnumContext::Application::WriterWeb);
    AddEntry(u"com.sun.star.xforms.XMLFormDocument"_ustr, EnumContext::Application::WriterXML);
    AddEntry(u"com.sun.star.sdb.FormDesign"_ustr, EnumContext::Application::WriterForm);
    AddEntry(u"com.sun.star.sdb.TextReportDesign"_ustr, EnumContext::Application::WriterReport);
    AddEntry(u"com.sun.star.sheet.SpreadsheetDocument"_ustr, EnumContext::Application::Calc);
    AddEntry(u"com.sun.star.chart2.ChartDocument"_ustr, EnumContext::Application::Chart);
    AddEntry(u"com.sun.star.drawing.DrawingDocument"_ustr, EnumContext::Application::Draw);
    AddEntry(u"com.sun.star.presentation.PresentationDocument"_ustr, EnumContext::Application::Impress);
    AddEntry(u"com.sun.star.formula.FormulaProperties"_ustr, EnumContext::Application::Formula);
    AddEntry(u"com.sun.star.sdb.OfficeDatabaseDocument"_ustr, EnumContext::Application::Base);
    AddEntry(u"any"_ustr, EnumContext::Application::Any);
    AddEntry(u"none"_ustr, EnumContext::Application::NONE);

}

EnumContext::Application EnumContext::GetApplicationEnum (const OUString& rsApplicationName)
{
    ProvideApplicationContainers();

    ApplicationMap::const_iterator iApplication(
        maApplicationMap.find(rsApplicationName));
    if (iApplication != maApplicationMap.end())
        return iApplication->second;
    else
        return EnumContext::Application::NONE;
}

const OUString& EnumContext::GetApplicationName (const Application eApplication)
{
    ProvideApplicationContainers();
    return maApplicationVector[eApplication];
}

void EnumContext::AddEntry (const OUString& rsName, const Context eContext)
{
    maContextMap[rsName] = eContext;
    maContextVector[eContext] = rsName;
}

void EnumContext::ProvideContextContainers()
{
    if (!maContextMap.empty())
        return;

    AddEntry(u"3DObject"_ustr, Context::ThreeDObject);
    AddEntry(u"Annotation"_ustr, Context::Annotation);
    AddEntry(u"Auditing"_ustr, Context::Auditing);
    AddEntry(u"Axis"_ustr, Context::Axis);
    AddEntry(u"Cell"_ustr, Context::Cell);
    AddEntry(u"Chart"_ustr, Context::Chart);
    AddEntry(u"ChartElements"_ustr, Context::ChartElements);
    AddEntry(u"Draw"_ustr, Context::Draw);
    AddEntry(u"DrawFontwork"_ustr, Context::DrawFontwork);
    AddEntry(u"DrawLine"_ustr, Context::DrawLine);
    AddEntry(u"DrawPage"_ustr, Context::DrawPage);
    AddEntry(u"DrawText"_ustr, Context::DrawText);
    AddEntry(u"EditCell"_ustr, Context::EditCell);
    AddEntry(u"ErrorBar"_ustr, Context::ErrorBar);
    AddEntry(u"Form"_ustr, Context::Form);
    AddEntry(u"Frame"_ustr, Context::Frame);
    AddEntry(u"Graphic"_ustr, Context::Graphic);
    AddEntry(u"Grid"_ustr, Context::Grid);
    AddEntry(u"HandoutPage"_ustr, Context::HandoutPage);
    AddEntry(u"MasterPage"_ustr, Context::MasterPage);
    AddEntry(u"Math"_ustr, Context::Math);
    AddEntry(u"Media"_ustr, Context::Media);
    AddEntry(u"MultiObject"_ustr, Context::MultiObject);
    AddEntry(u"NotesPage"_ustr, Context::NotesPage);
    AddEntry(u"OLE"_ustr, Context::OLE);
    AddEntry(u"OutlineText"_ustr, Context::OutlineText);
    AddEntry(u"Pivot"_ustr, Context::Pivot);
    AddEntry(u"Printpreview"_ustr, Context::Printpreview);
    AddEntry(u"Series"_ustr, Context::Series);
    AddEntry(u"SlidesorterPage"_ustr, Context::SlidesorterPage);
    AddEntry(u"Table"_ustr, Context::Table);
    AddEntry(u"Text"_ustr, Context::Text);
    AddEntry(u"TextObject"_ustr, Context::TextObject);
    AddEntry(u"Trendline"_ustr, Context::Trendline);
    AddEntry(u"Sparkline"_ustr, Context::Sparkline);

    // other general contexts
    AddEntry(u"any"_ustr, Context::Any);
    AddEntry(u"default"_ustr, Context::Default);
    AddEntry(u"empty"_ustr, Context::Empty);
}

EnumContext::Context EnumContext::GetContextEnum (const OUString& rsContextName)
{
    ProvideContextContainers();

    ContextMap::const_iterator iContext( maContextMap.find(rsContextName) );
    if (iContext != maContextMap.end())
        return iContext->second;
    else
        return EnumContext::Context::Unknown;
}

const OUString& EnumContext::GetContextName (const Context eContext)
{
    ProvideContextContainers();
    return maContextVector[eContext];
}

} // end of namespace vcl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
