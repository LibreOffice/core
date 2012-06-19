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
