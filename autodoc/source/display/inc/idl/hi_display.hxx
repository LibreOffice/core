/*************************************************************************
 *
 *  $RCSfile: hi_display.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: np $ $Date: 2002-11-01 17:15:13 $
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

