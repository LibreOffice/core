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



#ifndef ADC_DISPLAY_HTML_PAGEMAKE_HXX
#define ADC_DISPLAY_HTML_PAGEMAKE_HXX



// USED SERVICES
    // BASE CLASSES
#include <ary/ary_disp.hxx>
#include <cosv/tpl/processor.hxx>
#include "hdimpl.hxx"
    // COMPONENTS
    // PARAMETERS
#include <ary/cpp/c_namesp.hxx>

namespace ary
{
    namespace cpp
    {
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


class OuputPage_Environment;
class HtmlDocuFile;



class PageDisplay : public ary::Display,
                    public csv::ConstProcessor<ary::cpp::Class>,
                    public csv::ConstProcessor<ary::cpp::Enum>,
                    public csv::ConstProcessor<ary::cpp::Typedef>,
                    public HtmlDisplay_Impl
{
  public:
                        PageDisplay(
                            OuputPage_Environment &
                                                io_rEnv );
    virtual             ~PageDisplay();

    void                Create_OverviewFile();
    void                Create_AllDefsFile();
    void                Create_IndexFiles();
    void                Create_HelpFile();

    void                Create_NamespaceFile();

    void                Setup_OperationsFile_for(
                            const ary::loc::File &
                                                i_rFile );
    void                Setup_OperationsFile_for(
                            const ary::cpp::Class &
                                                i_rClass );
    void                Setup_DataFile_for(
                            const ary::loc::File &
                                                i_rFile );
    void                Setup_DataFile_for(
                            const ary::cpp::Class &
                                                i_rClass );
    /// Used with Setup_OperatonsFile_for().
    void                Create_File();


    // Interface for Children of SpecializedPageMaker:
    void                Write_NameChainWithLinks(
                            const ary::cpp::CodeEntity &
                                                i_rCe );

    // Necessary, to call Process() on this class.
    using csv::ConstProcessor<ary::cpp::Class>::Process;
    using csv::ConstProcessor<ary::cpp::Enum>::Process;
    using csv::ConstProcessor<ary::cpp::Typedef>::Process;

 private:
    // Interface csv::ConstProcessor<>:
    virtual void        do_Process(
                            const ary::cpp::Class &
                                                i_rData );
    virtual void        do_Process(
                            const ary::cpp::Enum &
                                                i_rData );
    virtual void        do_Process(
                            const ary::cpp::Typedef &
                                                i_rData );
    // Interface ary::cpp::Display:
    virtual const ary::cpp::Gate *
                        inq_Get_ReFinder() const;
    // Locals
    HtmlDocuFile &      File()                  { return *pMyFile; }
    void                RecursiveWrite_NamespaceLink(
                            const ary::cpp::Namespace *
                                                i_pNamespace );
    void                RecursiveWrite_ClassLink(
                            const ary::cpp::Class *
                                                i_pClass,
                            uintt               i_nLevelDistance );
    void                SetupFileOnCurEnv(
                            const char *        i_sTitle );
    void                Write_NavBar_Enum(
                            const ary::cpp::Enum &
                                                i_rData );
    void                Write_TopArea_Enum(
                            const ary::cpp::Enum &
                                                i_rData );
    void                Write_DocuArea_Enum(
                            const ary::cpp::Enum &
                                                i_rData );
    void                Write_ChildList_Enum(
                            const ary::cpp::Enum &
                                                i_rData );
    void                Write_NavBar_Typedef(
                            const ary::cpp::Typedef &
                                                i_rData );
    void                Write_TopArea_Typedef(
                            const ary::cpp::Typedef &
                                                i_rData );
    void                Write_DocuArea_Typedef(
                            const ary::cpp::Typedef &
                                                i_rData );
    void                Create_IndexFile(
                            int                 i_nLetter );

    // DATA
    Dyn<HtmlDocuFile>   pMyFile;
};




#endif
