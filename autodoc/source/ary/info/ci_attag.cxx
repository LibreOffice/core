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
#include <ary/info/ci_attag.hxx>


// NOT FULLY DEFINED SERVICES
#include <ary/info/all_dts.hxx>
#include <ary/info/ci_text.hxx>


namespace ary
{
namespace info
{

void
AtTag::Set_HtmlUseInDocuText( bool i_bUseIt )
{
    DocuText * pText = Text();
    if ( pText != 0 )
        pText->Set_HtmlUse(i_bUseIt);
}

void
AtTag::Add_Token( const char * i_sText )
{
    DocuText * pText = Text();
    if (pText != 0)
        pText->Add_Token( *new DT_Text(i_sText) );
}

void
AtTag::Add_PotentialLink( const char *	i_sText,
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
