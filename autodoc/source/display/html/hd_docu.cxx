/*************************************************************************
 *
 *  $RCSfile: hd_docu.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 15:23:29 $
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


#include <precomp.h>
#include "hd_docu.hxx"


// NOT FULLY DEFINED SERVICES
#include <ary/cpp/c_namesp.hxx>
#include <ary/cpp/c_class.hxx>
#include <ary/cpp/c_enum.hxx>
#include <ary/cpp/c_tydef.hxx>
#include <ary/cpp/c_funct.hxx>
#include <ary/cpp/c_vari.hxx>
#include <ary/cpp/c_enuval.hxx>
#include <ary/info/codeinfo.hxx>
#include <ary/info/all_tags.hxx>
#include <ary/info/all_dts.hxx>
#include <adc_cl.hxx>
#include "html_kit.hxx"



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
Docu_Display::Display_Namespace( const ary::cpp::Namespace & i_rData )
{
    i_rData.Info().StoreAt( *this );
}

void
Docu_Display::Display_Class( const ary::cpp::Class & i_rData )
{
    pCurClassOverwrite = &i_rData;
    i_rData.Info().StoreAt( *this );
    pCurClassOverwrite = 0;
}

void
Docu_Display::Display_Enum( const ary::cpp::Enum & i_rData )
{
    i_rData.Info().StoreAt( *this );
}

void
Docu_Display::Display_Typedef( const ary::cpp::Typedef & i_rData )
{
    i_rData.Info().StoreAt( *this );
}

void
Docu_Display::Display_Function( const ary::cpp::Function & i_rData )
{
    i_rData.Info().StoreAt( *this );
}

void
Docu_Display::Display_Variable( const ary::cpp::Variable & i_rData )
{
    i_rData.Info().StoreAt( *this );
}


// --------------       Interface ary::info::DocuDisplay  ------------------ //


void
Docu_Display::Display_CodeInfo( const CodeInfo & i_rData )
{
    Start_DocuBlock();

    if ( i_rData.IsObsolete() )
    {
        CurOut()
            >> *new html::DefListTerm
                >> *new html::Strong
                    << "D E P R E C A T E D";

    }

    CodeInfo::TagList::const_iterator itEnd = i_rData.Tags().end();
    for ( CodeInfo::TagList::const_iterator it = i_rData.Tags().begin();
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
Docu_Display::Display_BaseTag( const BaseTag & i_rData )
{

}

void
Docu_Display::Display_ExceptionTag( const ExceptionTag & i_rData )
{
}

void
Docu_Display::Display_ImplementsTag( const ImplementsTag & i_rData )
{
}

void
Docu_Display::Display_KeywordTag( const KeywordTag & i_rData )
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
Docu_Display::Display_LabelTag( const LabelTag & i_rData )
{
}

void
Docu_Display::Display_SinceTag( const ary::info::SinceTag & i_rData )
{
    if ( i_rData.Version().empty()
         OR
         NOT autodoc::CommandLine::Get_().Display_SinceTag() )
    {
         return;
    }
    // Transform the value of the @since tag into the text to be displayed.
    String sDisplay =
        autodoc::CommandLine::Get_().DisplayOf_SinceTagValue(
                                                i_rData.Version() );
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
Docu_Display::Display_DT_Eol( const DT_Eol & i_rData )
{
    CurOut() << new html::Sbr;
}

void
Docu_Display::Display_DT_Xml( const ary::info::DT_Xml & i_rData )
{
    CurOut() << new xml::XmlCode( i_rData.Text() );
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
                              const char *        i_nFontSize )
{
    if ( strcmp(i_sText,"ATTENTION!") == 0 )
    {
        CurOut()
            >> *new html::DefListTerm
                << new html::StyleAttr("color:#dd0000;")
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
Docu_Display::Write_TextToken( const udmstri & i_sText )
{
     if ( bUseHtmlInDocuTokens )
        CurOut() << new xml::XmlCode(i_sText);
    else
        CurOut() << i_sText;
}

void
Docu_Display::Write_LinkableText( const ary::QualifiedName & i_sQuName )
{
    const ary::CodeEntity *
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

