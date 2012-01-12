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
#include <ary/info/all_dts.hxx>


// NOT FULLY DEFINED SERVICES
#include <ary/info/infodisp.hxx>


namespace ary
{
namespace info
{


void
DT_Text::do_StoreAt( DocuDisplay & o_rDisplay ) const
{
    o_rDisplay.Display_DT_Text(*this);
}

bool
DT_Text::inq_IsWhite() const
{
     return false;
}

void
DT_MaybeLink::do_StoreAt( DocuDisplay & o_rDisplay ) const
{
    o_rDisplay.Display_DT_MaybeLink(*this);
}

bool
DT_MaybeLink::inq_IsWhite() const
{
     return false;
}

void
DT_Whitespace::do_StoreAt( DocuDisplay & o_rDisplay ) const
{
    o_rDisplay.Display_DT_Whitespace(*this);
}

bool
DT_Whitespace::inq_IsWhite() const
{
     return true;
}

void
DT_Eol::do_StoreAt( DocuDisplay & o_rDisplay ) const
{
    o_rDisplay.Display_DT_Eol(*this);
}

bool
DT_Eol::inq_IsWhite() const
{
     return true;
}

void
DT_Xml::do_StoreAt( DocuDisplay & o_rDisplay ) const
{
    o_rDisplay.Display_DT_Xml(*this);
}

bool
DT_Xml::inq_IsWhite() const
{
     return false;
}


}   // namespace info
}   // namespace ary

