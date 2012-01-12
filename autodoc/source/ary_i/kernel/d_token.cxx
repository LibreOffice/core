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
