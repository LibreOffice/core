/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: chd_udk2.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 16:40:32 $
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
