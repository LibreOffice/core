/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
