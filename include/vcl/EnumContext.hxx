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
#ifndef INCLUDED_VCL_ENUMCONTEXT_HXX
#define INCLUDED_VCL_ENUMCONTEXT_HXX

#include <vcl/dllapi.h>

#include <rtl/ustring.hxx>

#include <vector>


namespace vcl {

class VCL_DLLPUBLIC EnumContext
{
public:
    enum class Application
    {
        Writer,
        WriterGlobal,
        WriterWeb,
        WriterXML,
        WriterForm,
        WriterReport,
        Calc,
        Chart,
        Draw,
        Impress,
        Formula,
        Base,

        // For your convenience to avoid duplicate code in the common
        // case that Draw and Impress use identical context configurations.
        DrawImpress,

        // Also for your convenience for the different variants of Writer documents.
        WriterVariants,

        // Used only by deck or panel descriptors.  Matches any
        // application.
        Any,

        // Use this only in special circumstances.  One might be the
        // wish to disable a deck or panel during debugging.
        NONE,

        LAST = Application::NONE
    };
    enum class Context
    {
        ThreeDObject,
        Annotation,
        Auditing,
        Axis,
        Cell,
        Chart,
        ChartElements,
        Draw,
        DrawLine,
        DrawPage,
        DrawText,
        EditCell,
        ErrorBar,
        Form,
        Frame,
        Graphic,
        Grid,
        HandoutPage,
        MasterPage,
        Media,
        MultiObject,
        NotesPage,
        OLE,
        OutlineText,
        Pivot,
        Printpreview,
        Series,
        SlidesorterPage,
        Table,
        Text,
        TextObject,
        Trendline,

        // Default context of an application.  Do we need this?
        Default,

        // Used only by deck or panel descriptors.  Matches any context.
        Any,

        // Special context name that is only used when a deck would
        // otherwise be empty.
        Empty,

        Unknown,

        LAST = Unknown
    };

    EnumContext();
    EnumContext (
        const Application eApplication,
        const Context eContext);
    EnumContext (
        const ::rtl::OUString& rsApplicationName,
        const ::rtl::OUString& rsContextName);

    /** This variant of the GetCombinedContext() method treats some
        application names as identical to each other.  Replacements
        made are:
            Draw or Impress     -> DrawImpress
            Writer or WriterWeb -> WriterAndWeb
        Use the Application::DrawImpress or Application::WriterAndWeb values in the CombinedEnumContext macro.
    */
    sal_Int32 GetCombinedContext_DI() const;

    Application GetApplication_DI() const;

    bool operator == (const EnumContext& rOther);
    bool operator != (const EnumContext& rOther);

    /** When two contexts are matched against each other, then
        application or context name may have the wildcard value 'any'.
        In order to prefer matches without wildcards over matches with
        wildcards we introduce a integer evaluation for matches.
    */
    const static sal_Int32 NoMatch;
    const static sal_Int32 OptimalMatch;

    static Application GetApplicationEnum (const ::rtl::OUString& rsApplicationName);
    static const ::rtl::OUString& GetApplicationName (const Application eApplication);

    static Context GetContextEnum (const ::rtl::OUString& rsContextName);
    static const ::rtl::OUString& GetContextName (const Context eContext);

private:
    Application meApplication;
    Context meContext;

    static void ProvideApplicationContainers();
    static void ProvideContextContainers();
    static void AddEntry (const ::rtl::OUString& rsName, const Application eApplication);
    static void AddEntry (const ::rtl::OUString& rsName, const Context eContext);
};


#define CombinedEnumContext(a,e) ((static_cast<sal_uInt16>(::vcl::EnumContext::a)<<16)\
        | static_cast<sal_uInt16>(::vcl::EnumContext::e))

} // end of namespace vcl

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
