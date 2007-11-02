/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: hd_docu.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 16:27:04 $
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

#include <precomp.h>
#include "hd_docu.hxx"


// NOT FULLY DEFINED SERVICES
#include <ary/cpp/c_gate.hxx>
#include <ary/cpp/c_namesp.hxx>
#include <ary/cpp/c_class.hxx>
#include <ary/cpp/c_enum.hxx>
#include <ary/cpp/c_tydef.hxx>
#include <ary/cpp/c_funct.hxx>
#include <ary/cpp/c_vari.hxx>
#include <ary/cpp/c_enuval.hxx>
#include <ary/doc/d_oldcppdocu.hxx>
#include <ary/info/all_tags.hxx>
#include <ary/info/all_dts.hxx>
#include <adc_cl.hxx>
#include "html_kit.hxx"
#include "opageenv.hxx"



using namespace ary::info;
using namespace csi;

using html::DefList;
using html::DefListTerm;
using html::DefListDefinition;
using html::Headline;
using html::Link;


const char *
C_sTagHeadlines[ ary::info::C_eAtTag_NrOfClasses ] =
    {
        "ATTENTION!",   "Author",       "Changes",      "Collaborators",
                                                                        "Contact",  // Contact may be unused
        "Copyright",    "Deprecated",   "Description",  "Date of Documentation",
                                                                        "How to Derive from this Class",
        "Heap object - owner is responsible for deletion.",
                        "Important Instances",
                                        "Interface Only",
                                                        "Invariant",    "Lifecycle",
        "Multiplicity", "On Error",     "Persistency",  "Postcondition",
                                                                        "Precondition",
        "Responsibilities",
                        "Return",       "Summary",      "Todos",        "Version",
        "Base Classes", "Exceptions",   "Implements",   "Keywords",     "Parameters",
        "See Also",     "Template Parameters",
                                        "",             "Since "
    };



Docu_Display::Docu_Display( OuputPage_Environment & io_rEnv )
    :   HtmlDisplay_Impl(io_rEnv),
        bUseHtmlInDocuTokens(false),
        pCurClassOverwrite(0)
{
}

Docu_Display::~Docu_Display()
{
}

void
Docu_Display::Assign_Out( csi::xml::Element & o_rOut )
{
    Easy().Enter(o_rOut);
}

void
Docu_Display::Unassign_Out()
{
    Easy().Leave();
}

void
Docu_Display::do_Process( const ary::cpp::Namespace & i_rData )
{
    Process(i_rData.Docu());
}

void
Docu_Display::do_Process( const ary::cpp::Class & i_rData )
{
    pCurClassOverwrite = &i_rData;
    Process(i_rData.Docu());
    pCurClassOverwrite = 0;
}

void
Docu_Display::do_Process( const ary::cpp::Enum & i_rData )
{
    Process(i_rData.Docu());
}

void
Docu_Display::do_Process( const ary::cpp::Typedef & i_rData )
{
    Process(i_rData.Docu());
}

void
Docu_Display::do_Process( const ary::cpp::Function & i_rData )
{
    Process(i_rData.Docu());
}

void
Docu_Display::do_Process( const ary::cpp::Variable & i_rData )
{
    Process(i_rData.Docu());
}



// --------------       Interface ary::info::DocuDisplay  ------------------ //


void
Docu_Display::do_Process(const ary::doc::Documentation & i_rData)
{
    if (i_rData.Data() == 0)
        return;

    const ary::doc::OldCppDocu *
        docdata = dynamic_cast< const ary::doc::OldCppDocu* >(i_rData.Data());
    csv_assert(docdata != 0);

    Start_DocuBlock();

    if ( docdata->IsObsolete() )
    {
        CurOut()
            >> *new html::DefListTerm
                >> *new html::Strong
                    << "D E P R E C A T E D";

    }

    ary::doc::OldCppDocu::TagList::const_iterator
        itEnd = docdata->Tags().end();
    for ( ary::doc::OldCppDocu::TagList::const_iterator it = docdata->Tags().begin();
          it != itEnd;
          ++it )
    {
         (*it)->StoreAt( *this );
    }

    Finish_DocuBlock();
}

void
Docu_Display::Display_StdTag( const StdTag & i_rData )
{
    csv_assert( uintt(i_rData.Std_Id()) < uintt(ary::info::C_eAtTag_NrOfClasses) );

    const ary::info::DocuText::TokenList &
        rText = i_rData.CText().Tokens();
    typedef ary::info::DocuText::TokenList::const_iterator TokenIterator;

    if ( rText.empty() )
        return;
    else if ( rText.size() < 3 )
    {
        bool bIsWhite = true;
        for ( TokenIterator it = rText.begin();
              it != rText.end();
              ++it )
        {
            if (bIsWhite)
                bIsWhite = (*it)->IsWhite();
        }
        if (bIsWhite)
            return;
    }

    Write_TagTitle( C_sTagHeadlines[i_rData.Std_Id()] );
    Write_TagContents( i_rData.CText() );
}

void
Docu_Display::Display_BaseTag( const BaseTag & )
{
}

void
Docu_Display::Display_ExceptionTag( const ExceptionTag & )
{
}

void
Docu_Display::Display_ImplementsTag( const ImplementsTag & )
{
}

void
Docu_Display::Display_KeywordTag( const KeywordTag & )
{
}

void
Docu_Display::Display_ParameterTag( const ParameterTag & i_rData )
{
    Write_TagTitle( "Parameters" );

    adcdisp::ExplanationTable
        aParams( CurOut() >> *new DefListDefinition );

    for ( const ParameterTag * pParam = &i_rData;
          pParam != 0;
          pParam = pParam->GetNext() )   // KORR_FUTURE
    {
        aParams.AddEntry( pParam->ParamName().c_str() );

        Easy().Enter( aParams.Def() );
        Write_Text( pParam->CText() );
        Easy().Leave();
    }   // end for
}

void
Docu_Display::Display_SeeTag( const SeeTag & i_rData )
{
    Write_TagTitle( "See Also" );

    DefListDefinition * dpDef = new DefListDefinition;
    CurOut() << dpDef;
    Easy().Enter(*dpDef);

    for ( std::vector< ary::QualifiedName >::const_iterator
            it = i_rData.References().begin();
          it != i_rData.References().end();
          ++it )
    {
        Write_LinkableText( (*it) );
        CurOut() << new html::LineBreak;
    }

    Easy().Leave();
}

void
Docu_Display::Display_TemplateTag( const TemplateTag & i_rData )
{
    Write_TagTitle( "Template Parameters" );

    adcdisp::ExplanationTable
        aTplParams( CurOut() >> *new DefListDefinition );

    for ( const TemplateTag * pTplParam = &i_rData;
          pTplParam != 0;
          pTplParam = pTplParam->GetNext() )
    {
        aTplParams.AddEntry( pTplParam->TplParamName().c_str() );

        Easy().Enter( aTplParams.Def() );
        Write_Text( pTplParam->CText() );
        Easy().Leave();
    }   // end for
}

void
Docu_Display::Display_LabelTag( const LabelTag & )
{
}

void
Docu_Display::Display_SinceTag( const ary::info::SinceTag & i_rData )
{
    if ( i_rData.Version().empty() )
    {
         return;
    }

    // Transform the value of the @since tag into the text to be displayed.
    String sDisplay;
    if ( autodoc::CommandLine::Get_().DoesTransform_SinceTag() )
    {
        sDisplay = autodoc::CommandLine::Get_()
                        .DisplayOf_SinceTagValue( i_rData.Version() );
    }
    else
    {
        sDisplay = i_rData.Version();
    }

    if (sDisplay.empty())
        return;

    Write_TagTitle( "Since " );

    DefListDefinition * dpDef = new DefListDefinition;
    CurOut() << dpDef;

    Easy().Enter(*dpDef);
    CurOut() << sDisplay;
    Easy().Leave();
}

void
Docu_Display::Display_DT_Text( const DT_Text & i_rData )
{
    Write_TextToken(  i_rData.Text() );
}

void
Docu_Display::Display_DT_MaybeLink( const DT_MaybeLink & i_rData )
{
    // KORR_FUTURE
    Write_TextToken(  i_rData.Text() );
}

void
Docu_Display::Display_DT_Whitespace( const DT_Whitespace & i_rData )
{
    static char sSpace[300] =
        "                                         "
        "                                         "
        "                                         "
        "                                         "
        "                                         "
        "                                        ";
    UINT8 nLength = i_rData.Length();
    sSpace[nLength] = NULCH;
    CurOut() << sSpace;
    sSpace[nLength] = ' ';
}

void
Docu_Display::Display_DT_Eol( const DT_Eol & )
{
    CurOut() << new html::Sbr;
}

void
Docu_Display::Display_DT_Xml( const ary::info::DT_Xml & i_rData )
{
    CurOut() << new xml::XmlCode( i_rData.Text() );
}

const ary::cpp::Gate *
Docu_Display::inq_Get_ReFinder() const
{
    return &Env().Gate();
}

void
Docu_Display::Start_DocuBlock()
{
    DYN DefList * dpDefList = new DefList;
    CurOut() << dpDefList;
    Easy().Enter( *dpDefList );
}

void
Docu_Display::Finish_DocuBlock()
{
    Easy().Leave();
}

void
Docu_Display::Write_TagTitle( const char *        i_sText,
                              const char *        )
{
    if ( strcmp(i_sText,"ATTENTION!") == 0 )
    {
        CurOut()
            >> *new html::DefListTerm
                << new html::ClassAttr("attention")
                << i_sText;
    }
    else
    {
        CurOut()
            >> *new html::DefListTerm
                << i_sText;
    }
}

void
Docu_Display::Write_TagContents( const DocuText & i_rDocuText )
{
    DefListDefinition * dpDef = new DefListDefinition;
    CurOut() << dpDef;

    Easy().Enter(*dpDef);
    Write_Text(i_rDocuText);
    Easy().Leave();
}

void
Docu_Display::Write_Text( const ary::info::DocuText & i_rDocuText )
{
    if ( i_rDocuText.IsNoHtml() )
    {
        CurOut()
            << new xml::XmlCode("<pre>");
        bUseHtmlInDocuTokens = false;
    }
    else
    {
         bUseHtmlInDocuTokens = true;
    }
    i_rDocuText.StoreAt( *this );
    if ( i_rDocuText.IsNoHtml() )
    {
        CurOut()
            << new xml::XmlCode("</pre>");
    }
}

void
Docu_Display::Write_TextToken( const String & i_sText )
{
     if ( bUseHtmlInDocuTokens )
        CurOut() << new xml::XmlCode(i_sText);
    else
        CurOut() << i_sText;
}

void
Docu_Display::Write_LinkableText( const ary::QualifiedName & i_sQuName )
{
    const ary::cpp::CodeEntity *
        pCe = FindUnambiguousCe( Env(), i_sQuName, pCurClassOverwrite );
    if ( pCe != 0 )
    {
        csi::xml::Element *
            pLink = new csi::html::Link( Link2Ce(Env(), *pCe) );
        CurOut() << pLink;
        Easy().Enter(*pLink);
        Write_QualifiedName(i_sQuName);
        Easy().Leave();
    }
    else
    {
        Write_QualifiedName(i_sQuName);
    }
    CurOut() << " ";
}

void
Docu_Display::Write_QualifiedName( const ary::QualifiedName & i_sQuName )
{
    if ( i_sQuName.IsAbsolute() )
        CurOut() << "::";
    for ( ary::QualifiedName::namespace_iterator it = i_sQuName.first_namespace();
          it != i_sQuName.end_namespace();
          ++it )
    {
        CurOut() <<  (*it) << "::";
    }
    CurOut() << i_sQuName.LocalName();
    if ( i_sQuName.IsFunction() )
        CurOut() << "()";
}

