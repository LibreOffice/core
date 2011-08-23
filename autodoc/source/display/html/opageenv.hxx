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

#ifndef ADC_DISPLAY_HTML_OPAGEENV_HXX
#define ADC_DISPLAY_HTML_OPAGEENV_HXX


// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
#include <cosv/ploc.hxx>
    // PARAMETERS

namespace ary
{
    namespace cpp
    {
        class Gate;

        class Namespace;
        class Class;
        class Enum;
        class Typedef;
    }
    namespace loc
    {
        class File;
    }
}
namespace display
{
    class CorporateFrame;
}

class OuputPage_Environment
{
  public:
    // LIFECYCLE
                        OuputPage_Environment(
                            const csv::ploc::Path &
                                                io_rOutputDir,
                            const ary::cpp::Gate &
                                                i_rGate,
                            const display::CorporateFrame &
                                                i_rLayout );
                        ~OuputPage_Environment();

    // OPERATIONS
    void                MoveDir_2Root();
    void                MoveDir_2Names();
    void                MoveDir_Down2(             /// Only one level.
                            const ary::cpp::Namespace &
                                                i_rNsp );
    void                MoveDir_Down2(             /// Only one level.
                            const ary::cpp::Class &
                                                i_rClass );
    void                MoveDir_2Index();
    void                MoveDir_Up();

    void                SetFile_Css();
    void                SetFile_Overview();
    void                SetFile_AllDefs();
    void                SetFile_Index(
                            char                i_cLetter );
    void                SetFile_Help();
    void                SetFile_CurNamespace();
    void                SetFile_Class(
                            const ary::cpp::Class &
                                                i_rClass );
    void                SetFile_Enum(
                            const ary::cpp::Enum &
                                                i_rEnum );
    void                SetFile_Typedef(
                            const ary::cpp::Typedef &
                                                i_typedef );
    void                SetFile_Operations(
                            const ary::loc::File *
                                                i_pFile = 0 );  /// Only needed for global functions.
    void                SetFile_Data(
                            const ary::loc::File *
                                                i_pFile = 0 );  /// Only needed for global variables.
    // INQUIRY
    const ary::cpp::Namespace *
                        CurNamespace() const;
    const ary::cpp::Class *
                        CurClass() const;
    const csv::ploc::Path &
                        CurPath() const;
    const ary::cpp::Gate &
                        Gate() const;
    const display::CorporateFrame &
                        Layout() const;
    uintt               Depth() const;
    const String &      RepositoryTitle() const;

  private:
    struct CheshireCat;
    Dyn<CheshireCat>    pi;
};




#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
