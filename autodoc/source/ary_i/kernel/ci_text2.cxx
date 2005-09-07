/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ci_text2.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:22:41 $
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
#include <ary_i/ci_text2.hxx>


// NOT FULLY DEFINED SERVICES
#include <ary_i/disdocum.hxx>
#include <ary_i/d_token.hxx>


namespace ary
{
namespace info
{

DocuTex2::DocuTex2()
{
}

DocuTex2::~DocuTex2()
{
    for ( TokenList::iterator iter = aTokens.begin();
          iter != aTokens.end();
          ++iter )
    {
        delete (*iter);
    }
}

void
DocuTex2::DisplayAt( DocumentationDisplay & o_rDisplay ) const
{
    for ( ary::info::DocuTex2::TokenList::const_iterator
                iter = aTokens.begin();
          iter != aTokens.end();
          ++iter )
    {
        (*iter)->DisplayAt(o_rDisplay);
    }
}

bool
DocuTex2::IsEmpty() const
{
    for ( ary::info::DocuTex2::TokenList::const_iterator
                iter = aTokens.begin();
          iter != aTokens.end();
          ++iter )
    {
        return false;
    }
    return true;
}

using csi::dsapi::DT_TextToken;

const String &
DocuTex2::TextOfFirstToken() const
{
    if (NOT aTokens.empty())
    {
        const DT_TextToken *
            pTok = dynamic_cast< const DT_TextToken* >(*aTokens.begin());

        if (pTok != 0)
            return pTok->GetTextStr();
    }
    return String::Null_();
}

String &
DocuTex2::Access_TextOfFirstToken()
{
    if (NOT aTokens.empty())
    {
        DT_TextToken *
            pTok = dynamic_cast< DT_TextToken* >(*aTokens.begin());

        if (pTok != 0)
            return pTok->Access_Text();
    }

    static String sDummy_;
    return sDummy_;
}



void    DocuText_Display::Display_StdAtTag(
                            const csi::dsapi::DT_StdAtTag & ) {}
void    DocuText_Display::Display_SeeAlsoAtTag(
                            const csi::dsapi::DT_SeeAlsoAtTag & ) {}
void    DocuText_Display::Display_ParameterAtTag(
                            const csi::dsapi::DT_ParameterAtTag & ) {}
void    DocuText_Display::Display_SinceAtTag(
                            const csi::dsapi::DT_SinceAtTag & ) {}



}   // namespace info
}   // namespace ary

