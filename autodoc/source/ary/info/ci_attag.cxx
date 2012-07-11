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
#include <ary/info/ci_attag.hxx>


// NOT FULLY DEFINED SERVICES
#include <ary/info/all_dts.hxx>
#include <ary/info/ci_text.hxx>


namespace ary
{
namespace info
{

void
AtTag::Add_Token( const char * i_sText )
{
    DocuText * pText = Text();
    if (pText != 0)
        pText->Add_Token( *new DT_Text(i_sText) );
}

void
AtTag::Add_PotentialLink( const char *  i_sText,
                          bool          i_bIsGlobal,
                          bool          i_bIsFunction )
{
    DocuText * pText = Text();
    if (pText != 0)
        pText->Add_Token( *new DT_MaybeLink(i_sText, i_bIsGlobal, i_bIsFunction) );
}

void
AtTag::Add_Whitespace( UINT8 i_nLength )
{
    DocuText * pText = Text();
    if (pText != 0)
        pText->Add_Token( *new DT_Whitespace(i_nLength) );
}

void
AtTag::Add_Eol()
{
    DocuText * pText = Text();
    if (pText != 0)
        pText->Add_Token( *new DT_Eol );
}

void
AtTag::do_StoreAt( DocuDisplay &  ) const
{
    // Dummy
}

}   // namespace info
}   // namespace ary


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
