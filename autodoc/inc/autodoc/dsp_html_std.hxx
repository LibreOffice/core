/*************************************************************************
 *
 *  $RCSfile: dsp_html_std.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: np $ $Date: 2002-03-08 14:45:16 $
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

#ifndef AUTODOC_DSP_HTML_STD_HXX
#define AUTODOC_DSP_HTML_STD_HXX



namespace ary
{
    namespace cpp
    {
        class DisplayGate;
    }
}

namespace display
{
    class CorporateFrame;
}


namespace autodoc
{


class HtmlDisplay_UdkStd
{
  public:
    virtual             ~HtmlDisplay_UdkStd() {}

    /** Displays the names of several C++ code entities within the
        given namespace (or the global namespace as default). All
        subnamespaces are included.

        Output has following format:

        =========================================================================
        OutputDirectory
            index.html
            def-all.html
            prj\
                sal\
                    index.html                      // Overview about project

                    f-FileName[1,2,...]             // Overview about file
                    f-OtherFileName[1,2,...]
                    ...
                    def-FileName[1,2,...]           // #defines and macros in file
                    def-OtherFileName[1,2,...]
                    ...
                rtl\
                    ...
                cppu\
                    ...
                cppuhelper\
                    ...
                ...
            ix\
                ix-a.html
                ix-b.html
                ...
                ix-z.html
                ix-_.html
                ix-other.html

            cpp\
                index.html                          // Overview about global namespace

                Namespace_A\
                    Namespace_C\
                        index.html                  // Overview about namespace C
                        ...
                    ...

                    index.html                      // Overview about namespace A

                    c-ClassName_X.html              // Description of class
                    ...
                    e-EnumName.html                 // Description of enum
                    ...
                    t-TypedefName.html              // Description of typedef
                    ...
                    o-Filename.html                 // Descriptions of operations in this file in this namespace
                    ...
                    d-Filename.html                 // Descriptions of data in this file in this namespace
                    ...

                    ClassName_X\
                        c-ClassName_Y.html
                        e-EnumName.html
                        t-TypedefName.html
                        o.html                      // Descriptions of operations in class X
                        d.html                      // Descriptions of data in class X

                        ClassName_Y\
                            ...
                ...

            idl\
                ...
            java\
                ...
        =========================================================================


        @param i_sOutputDirectory
            Directory for output. Path must be given in correct
            syntax for the actual operating system without final
            path delimiter. If this is 0 or "", the current
            working directory is chosen.
        @param i_rAryGate
            The access to the Autodoc Repository.
        @param i_rLayout
            Gives parameters for the appearance of the HTML output.
        @param i_pProjectList
            If this is != 0, then only code entities which are declared
            in this projects are displayed.
    */
    void                Run(
                            const char *        i_sOutputDirectory,
                            const ary::cpp::DisplayGate &
                                                i_rAryGate,
                            const display::CorporateFrame &
                                                i_rLayout,
                            const StringVector *
                                                i_pProjectList = 0 );
  private:
    virtual void        do_Run(
                            const char *        i_sOutputDirectory,
                            const ary::cpp::DisplayGate &
                                                i_rAryGate,
                            const display::CorporateFrame &
                                                i_rLayout,
                            const StringVector *
                                                i_pProjectList ) = 0;
};

// IMPLEMENTATION

inline void
HtmlDisplay_UdkStd::Run( const char *                    i_sOutputDirectory,
                         const ary::cpp::DisplayGate &   i_rAryGate,
                         const display::CorporateFrame & i_rLayout,
                         const StringVector *  i_pProjectList )
    { do_Run( i_sOutputDirectory, i_rAryGate, i_rLayout, i_pProjectList ); }



} // namespace autodoc


#endif


