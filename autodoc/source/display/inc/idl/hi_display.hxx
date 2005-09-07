/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: hi_display.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:54:40 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

