/*************************************************************************
 *
 *  $RCSfile: chd_udk2.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: np $ $Date: 2002-03-08 14:45:25 $
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

#ifndef ADC_DISPLAY_HTML_CHD_UDK2_HXX
#define ADC_DISPLAY_HTML_CHD_UDK2_HXX



// USED SERVICES
    // BASE CLASSES
#include <autodoc/dsp_html_std.hxx>
    // COMPONENTS
#include <cosv/ploc.hxx>
    // PARAMETERS
#include <ary/idlists.hxx>

namespace csv
{
    namespace ploc
    {
        class Path;
    }
}

namespace ary
{
    namespace cpp
    {
        class Namespace;
        class ProjectGroup;
    }

    class TopProject;
    class CodeEntity;
}

// class PageDisplay;
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
                            const ary::cpp::DisplayGate &
                                                i_rAryGate,
                            const display::CorporateFrame &
                                                i_rLayout,
                            const StringVector *
                                                i_pProjectList );

    // Local
    typedef std::vector< const ary::cpp::ProjectGroup * >   ProjectList;


    void                SetRunData(
                            const char *        i_sOutputDirectory,
                            const ary::cpp::DisplayGate &
                                                i_rAryGate,
                            const display::CorporateFrame &
                                                i_rLayout,
                            const StringVector *
                                                i_pProjectList );


    void                Create_Overview_File();
    void                Create_Help_File();
    void                Create_AllDefs_File();
    void                CreateFiles_InSubTree_Projects();
    void                CreateFiles_InSubTree_Namespaces();
    void                CreateFiles_InSubTree_Index();

    void                RecursiveDisplay_Namespace(
                            const ary::cpp::Namespace &
                                                i_rNsp );
    void                DisplayFiles_InNamespace(
                            const ary::cpp::Namespace &
                                                i_rNsp );
    bool                CheckFilters(
                            const ary::CodeEntity &
                                                i_rCe ) const
                                                { // KORR_FUTURE Check, if project is included, and maybe other filters.
                                                  return true; }
    const ary::cpp::DisplayGate &
                        Gate() const;
    // DATA
    Dyn<OuputPage_Environment>
                        pCurPageEnv;
    ProjectList         aProjectList;
    bool                bFilterByProjectList;
};



// IMPLEMENTATION


#endif

