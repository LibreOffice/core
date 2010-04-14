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

#ifndef AUTODOC_DSP_HTML_STD_HXX
#define AUTODOC_DSP_HTML_STD_HXX



namespace ary
{
    namespace cpp
    {
        class Gate;
    }
    namespace idl
    {
        class Gate;
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
                            const ary::cpp::Gate &
                                                i_rAryGate,
                            const display::CorporateFrame &
                                                i_rLayout );
  private:
    virtual void        do_Run(
                            const char *        i_sOutputDirectory,
                            const ary::cpp::Gate &
                                                i_rAryGate,
                            const display::CorporateFrame &
                                                i_rLayout ) = 0;
};

// IMPLEMENTATION

inline void
HtmlDisplay_UdkStd::Run( const char *                    i_sOutputDirectory,
                         const ary::cpp::Gate &          i_rAryGate,
                         const display::CorporateFrame & i_rLayout )
{
    do_Run( i_sOutputDirectory, i_rAryGate, i_rLayout );
}



// class HtmlDisplay_Idl_Ifc

class HtmlDisplay_Idl_Ifc
{
  public:
    virtual             ~HtmlDisplay_Idl_Ifc() {}

    void                Run(
                            const char *        i_sOutputDirectory,
                            const ary::idl::Gate &
                                                i_rAryGate,
                            const display::CorporateFrame &
                                                i_rLayout );
  private:
    virtual void        do_Run(
                            const char *        i_sOutputDirectory,
                            const ary::idl::Gate &
                                                i_rAryGate,
                            const display::CorporateFrame &
                                                i_rLayout ) = 0;
};




// IMPLEMENTATION
inline void
HtmlDisplay_Idl_Ifc::Run( const char *                    i_sOutputDirectory,
                          const ary::idl::Gate &          i_rAryGate,
                          const display::CorporateFrame & i_rLayout )
{
    do_Run( i_sOutputDirectory, i_rAryGate, i_rLayout );
}




} // namespace autodoc
#endif
