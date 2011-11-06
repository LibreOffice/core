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
