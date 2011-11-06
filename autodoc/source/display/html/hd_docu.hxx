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



#ifndef ADC_DISPLAY_HTML_HD_DOCU_HXX
#define ADC_DISPLAY_HTML_HD_DOCU_HXX

// BASE CLASSES
#include <ary/ary_disp.hxx>
#include <ary/info/infodisp.hxx>
#include <cosv/tpl/processor.hxx>
#include "hdimpl.hxx"

namespace ary
{
    namespace cpp
    {
        class Namespace;
        class Class;
        class Enum;
        class Typedef;
        class Function;
        class Variable;
    }

    namespace doc
    {
         class Documentation;
    }
    namespace info
    {
        class DocuText;
    }

    class QualifiedName;
}

class OuputPage_Environment;


class Docu_Display : public ary::Display,
                     public csv::ConstProcessor<ary::cpp::Namespace>,
                     public csv::ConstProcessor<ary::cpp::Class>,
                     public csv::ConstProcessor<ary::cpp::Enum>,
                     public csv::ConstProcessor<ary::cpp::Typedef>,
                     public csv::ConstProcessor<ary::cpp::Function>,
                     public csv::ConstProcessor<ary::cpp::Variable>,
                     public csv::ConstProcessor<ary::doc::Documentation>,
                     public ary::info::DocuDisplay,
                     private HtmlDisplay_Impl
{
  public:
                        Docu_Display(
                            OuputPage_Environment &
                                                io_rEnv );
    virtual             ~Docu_Display();

    void                Assign_Out(
                            csi::xml::Element & o_rOut );
    void                Unassign_Out();

    virtual void        Display_StdTag(
                            const ary::info::StdTag &
                                                i_rData );
    virtual void        Display_BaseTag(
                            const ary::info::BaseTag &
                                                i_rData );
    virtual void        Display_ExceptionTag(
                            const ary::info::ExceptionTag &
                                                i_rData );
    virtual void        Display_ImplementsTag(
                            const ary::info::ImplementsTag &
                                                i_rData );
    virtual void        Display_KeywordTag(
                            const ary::info::KeywordTag &
                                                i_rData );
    virtual void        Display_ParameterTag(
                            const ary::info::ParameterTag &
                                                i_rData );
    virtual void        Display_SeeTag(
                            const ary::info::SeeTag &
                                                i_rData );
    virtual void        Display_TemplateTag(
                            const ary::info::TemplateTag &
                                                i_rData );
    virtual void        Display_LabelTag(
                            const ary::info::LabelTag &
                                                i_rData );
    virtual void        Display_SinceTag(
                            const ary::info::SinceTag &
                                                i_rData );

    virtual void        Display_DT_Text(
                            const ary::info::DT_Text &
                                                i_rData );
    virtual void        Display_DT_MaybeLink(
                            const ary::info::DT_MaybeLink &
                                                i_rData );
    virtual void        Display_DT_Whitespace(
                            const ary::info::DT_Whitespace &
                                                i_rData );
    virtual void        Display_DT_Eol(
                            const ary::info::DT_Eol &
                                                i_rData );
    virtual void        Display_DT_Xml(
                            const ary::info::DT_Xml &
                                                i_rData );

    using csv::ConstProcessor<ary::doc::Documentation>::Process;

  private:
    // Interface csv::ConstProcessor<>:
    virtual void        do_Process(
                            const ary::cpp::Namespace &
                                                i_rData );
    virtual void        do_Process(
                            const ary::cpp::Class &
                                                i_rData );
    virtual void        do_Process(
                            const ary::cpp::Enum &
                                                i_rData );
    virtual void        do_Process(
                            const ary::cpp::Typedef &
                                                i_rData );
    virtual void        do_Process(
                            const ary::cpp::Function &
                                                i_rData );
    virtual void        do_Process(
                            const ary::cpp::Variable &
                                                i_rData );
    virtual void        do_Process(
                            const ary::doc::Documentation &
                                                i_rData );
    // Interface ary::Display:
    virtual const ary::cpp::Gate *
                        inq_Get_ReFinder() const;
    // Locals
    void                Start_DocuBlock();
    void                Finish_DocuBlock();

    void                Write_TagTitle(
                            const char *        i_sText,
                            const char *        i_nFontSize = "+0" );
    void                Write_TagContents(
                            const ary::info::DocuText &
                                                i_rDocuText );
    void                Write_Text(
                            const ary::info::DocuText &
                                                i_rDocuText );
    void                Write_TextToken(
                            const String  &     i_sText );
    void                Write_LinkableText(
                            const ary::QualifiedName &
                                                i_sQuName );
    void                Write_QualifiedName(
                            const ary::QualifiedName &
                                                i_sQuName );

    // DATA
    bool                bUseHtmlInDocuTokens;

    /** This is used, if a class documentation is displayed,
        because for links to members then the "current class"
        is not the parent, but this class itself.
    */
    const ary::cpp::Class *
                        pCurClassOverwrite;
};




#endif
