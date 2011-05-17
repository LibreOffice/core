/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef ADC_DISPLAY_HTML_CHD_UDK2_HXX
#define ADC_DISPLAY_HTML_CHD_UDK2_HXX

// BASE CLASSES
#include <autodoc/dsp_html_std.hxx>
// USED SERVICES
#include <cosv/ploc.hxx>

namespace ary
{
namespace cpp
{
    class Namespace;
    class Gate;
}
}

class OuputPage_Environment;




class CppHtmlDisplay_Udk2 : public autodoc::HtmlDisplay_UdkStd
{
  public:
                        CppHtmlDisplay_Udk2();
                        ~CppHtmlDisplay_Udk2();
  private:
    // Interface CppHtmlDisplay_UdkStd:
    virtual void        do_Run(
                            const char *        i_sOutputDirectory,
                            const ary::cpp::Gate &
                                                i_rAryGate,
                            const display::CorporateFrame &
                                                i_rLayout );

    // Local
    void                SetRunData(
                            const char *        i_sOutputDirectory,
                            const ary::cpp::Gate &
                                                i_rAryGate,
                            const display::CorporateFrame &
                                                i_rLayout );

    void                Create_Css_File();
    void                Create_Overview_File();
    void                Create_Help_File();
    void                Create_AllDefs_File();
    void                CreateFiles_InSubTree_Namespaces();
    void                CreateFiles_InSubTree_Index();

    void                RecursiveDisplay_Namespace(
                            const ary::cpp::Namespace &
                                                i_rNsp );
    void                DisplayFiles_InNamespace(
                            const ary::cpp::Namespace &
                                                i_rNsp );
    const ary::cpp::Gate &
                        Gate() const;
    // DATA
    Dyn<OuputPage_Environment>
                        pCurPageEnv;
};




#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
