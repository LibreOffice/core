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

#ifndef ADC_DISPLAY_HI_DISPLAY_HXX
#define ADC_DISPLAY_HI_DISPLAY_HXX



// USED SERVICES
    // BASE CLASSES
#include <autodoc/dsp_html_std.hxx>
    // COMPONENTS
#include <cosv/ploc.hxx>
    // PARAMETERS



namespace ary
{
    namespace idl
    {
        class Module;
        class CodeEntity;
    }   // namspace idl
}   // namspace csi


class MainDisplay_Idl;
class HtmlEnvironment_Idl;

class HtmlDisplay_Idl : public autodoc::HtmlDisplay_Idl_Ifc
{
  public:
                        HtmlDisplay_Idl();
                        ~HtmlDisplay_Idl();
  private:
    // Interface HtmlDisplay_Idl_Ifc:
    virtual void        do_Run(
                            const char *        i_sOutputDirectory,
                            const ary::idl::Gate &
                                                i_rAryGate,
                            const display::CorporateFrame &
                                                i_rLayout );
    void                SetRunData(
                            const char *        i_sOutputDirectory,
                            const ary::idl::Gate &
                                                i_rAryGate,
                            const display::CorporateFrame &
                                                i_rLayout );
    void                Create_StartFile();
    void                Create_FilesInNameTree();
    void                Create_IndexFiles();
    void                Create_FilesInProjectTree();
    void                Create_PackageList();
    void                Create_HelpFile();
    void                Create_CssFile();

    /** @descr
        - makes sure, the module's directory exists
        - creates the module's docu file
        - creates docu files for all members of the module
        - does the same recursive for all sub-modules.
    */
    void                RecursiveDisplay_Module(
                            const ary::idl::Module &
                                                i_rNamespace );
    bool                IsModule(
                            const ary::idl::CodeEntity &
                                                i_ce ) const;
    const ary::idl::Module &
                        Module_Cast(            /// @precond Cast must be valid.
                            const ary::idl::CodeEntity &
                                                i_ce ) const;
    // DATA
    Dyn<HtmlEnvironment_Idl>
                        pCurPageEnv;
    Dyn<MainDisplay_Idl>
                        pMainDisplay;
};



// IMPLEMENTATION


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
