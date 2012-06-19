/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
