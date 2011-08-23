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
#include <ary_i/d_token.hxx>


// NOT FULLY DEFINED SERVICES
#include <ary_i/disdocum.hxx>




namespace csi
{
namespace dsapi
{

bool
DT_Dsapi::IsWhiteOnly() const
{
    return false;
}

DT_TextToken::~DT_TextToken()
{
}

void
DT_TextToken::DisplayAt( DocumentationDisplay & o_rDisplay ) const
{
    o_rDisplay.Display_TextToken( *this );
}

bool
DT_TextToken::IsWhiteOnly() const
{
    for ( const char * it = sText.c_str();
          static_cast<UINT8>(*it) > 32;
          ++it )
    {
        return false;
    }
    return true;
}

DT_White::~DT_White()
{
}

void
DT_White::DisplayAt( DocumentationDisplay & o_rDisplay ) const
{
    o_rDisplay.Display_White();
}

bool
DT_White::IsWhiteOnly() const
{
    return true;
}

DT_MupType::~DT_MupType()
{
}

void
DT_MupType::DisplayAt( DocumentationDisplay & o_rDisplay ) const
{
    o_rDisplay.Display_MupType( *this );
}

DT_MupMember::~DT_MupMember()
{
}

void
DT_MupMember::DisplayAt( DocumentationDisplay & o_rDisplay ) const
{
    o_rDisplay.Display_MupMember( *this );
}

DT_MupConst::~DT_MupConst()
{
}

void
DT_MupConst::DisplayAt( DocumentationDisplay & o_rDisplay ) const
{
    o_rDisplay.Display_MupConst( *this );
}

DT_Style::~DT_Style()
{
}

void
DT_Style::DisplayAt( DocumentationDisplay & o_rDisplay ) const
{
    o_rDisplay.Display_Style( *this );
}

DT_EOL::~DT_EOL()
{
}

void
DT_EOL::DisplayAt( DocumentationDisplay & o_rDisplay ) const
{
    o_rDisplay.Display_EOL();
}

bool
DT_EOL::IsWhiteOnly() const
{
    return true;
}

DT_StdAtTag::~DT_StdAtTag()
{
}

void
DT_StdAtTag::DisplayAt( DocumentationDisplay & o_rDisplay ) const
{
    o_rDisplay.Display_StdAtTag( *this );
}

DT_SeeAlsoAtTag::~DT_SeeAlsoAtTag()
{
}

void
DT_SeeAlsoAtTag::DisplayAt( DocumentationDisplay & o_rDisplay ) const
{
    o_rDisplay.Display_SeeAlsoAtTag( *this );
}

DT_ParameterAtTag::~DT_ParameterAtTag()
{
}

void
DT_ParameterAtTag::DisplayAt( DocumentationDisplay & o_rDisplay ) const
{
    o_rDisplay.Display_ParameterAtTag( *this );
}

DT_SinceAtTag::~DT_SinceAtTag()
{
}

void
DT_SinceAtTag::DisplayAt( DocumentationDisplay & o_rDisplay ) const
{
    o_rDisplay.Display_SinceAtTag( *this );
}




}   // namespace dsapi
}   // namespace csi

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
