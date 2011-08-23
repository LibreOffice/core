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

#include <precomp.h>
#include <ary_i/ci_text2.hxx>


// NOT FULLY DEFINED SERVICES
#include <ary_i/disdocum.hxx>
#include <ary_i/d_token.hxx>


namespace ary
{
namespace inf
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
    for ( ary::inf::DocuTex2::TokenList::const_iterator
                iter = aTokens.begin();
          iter != aTokens.end();
          ++iter )
    {
        (*iter)->DisplayAt(o_rDisplay);
    }
}

void
DocuTex2::AddToken( DYN DocuToken &	let_drToken )
{
    if (aTokens.empty())
    {
        if (let_drToken.IsWhiteOnly())
            return;
    }
    aTokens.push_back(&let_drToken);
}

bool
DocuTex2::IsEmpty() const
{
    for ( ary::inf::DocuTex2::TokenList::const_iterator
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



}   // namespace inf
}   // namespace ary

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
