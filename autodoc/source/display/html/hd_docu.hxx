/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: hd_docu.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:29:08 $
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

#ifndef ADC_DISPLAY_HTML_HD_DOCU_HXX
#define ADC_DISPLAY_HTML_HD_DOCU_HXX



// USED SERVICES
    // BASE CLASSES
#include <ary/cpp/cpp_disp.hxx>
#include <ary/info/infodisp.hxx>
#include "hdimpl.hxx"
    // COMPONENTS
    // PARAMETERS

namespace ary
{
    namespace info
    {
         class DocuText;
    }

    class QualifiedName;
}


class OuputPage_Environmen;


class Docu_Display : public ary::cpp::Display,
                     public ary::info::DocuDisplay,
                     private HtmlDisplay_Impl
{
  public:
                        Docu_Display(
                            OuputPage_Environment &
                                                io_rEnv
                                                 );
    virtual             ~Docu_Display();

    void                Assign_Out(
                            csi::xml::Element & o_rOut );
    void                Unassign_Out();

    // Interface ary::cpp::Display
    virtual void        Display_Namespace(
                            const ary::cpp::Namespace &
                                                i_rData );
    virtual void        Display_Class(
                            const ary::cpp::Class &
                                                i_rData );
    virtual void        Display_Enum(
                            const ary::cpp::Enum &
                                                i_rData );
    virtual void        Display_Typedef(
                            const ary::cpp::Typedef &
                                                i_rData );
    virtual void        Display_Function(
                            const ary::cpp::Function &
                                                i_rData );
    virtual void        Display_Variable(
                            const ary::cpp::Variable &
                                                i_rData );

    // Interface ary::info::DocuDisplay:
    virtual void        Display_CodeInfo(
                            const ary::info::CodeInfo &
                                                i_rData );

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
  private:
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
                            const udmstri &     i_sText );
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

