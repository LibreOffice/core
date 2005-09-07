/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ci_attag.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:08:35 $
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
AtTag::do_StoreAt( DocuDisplay &  o_rDisplay ) const
{
    // Dummy
}

}   // namespace info
}   // namespace ary


