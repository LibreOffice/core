/*************************************************************************
 *
 *  $RCSfile: tk_punct.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: np $ $Date: 2002-03-08 14:45:35 $
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
#include <s2_luidl/tk_punct.hxx>


// NOT FULLY DECLARED SERVICES
#include <s2_luidl/tokintpr.hxx>


using csi::uidl::TokPunctuation;


udm::EnumValueMap           G_aTokPunctuation_EV_TokenId_Values;
TokPunctuation::EV_TokenId  ev_none(TokPunctuation::e_none,"");
TokPunctuation::EV_TokenId  BracketOpen(TokPunctuation::BracketOpen,"(");
TokPunctuation::EV_TokenId  BracketClose(TokPunctuation::BracketClose,")");
TokPunctuation::EV_TokenId  ArrayBracketOpen(TokPunctuation::ArrayBracketOpen,"[");
TokPunctuation::EV_TokenId  ArrayBracketClose(TokPunctuation::ArrayBracketClose,"]");
TokPunctuation::EV_TokenId  CurledBracketOpen(TokPunctuation::CurledBracketOpen,"{");
TokPunctuation::EV_TokenId  CurledBracketClose(TokPunctuation::CurledBracketClose,"}");
TokPunctuation::EV_TokenId  Semicolon(TokPunctuation::Semicolon,";");
TokPunctuation::EV_TokenId  Colon(TokPunctuation::Colon,":");
TokPunctuation::EV_TokenId  DoubleColon(TokPunctuation::DoubleColon,"::");
TokPunctuation::EV_TokenId  Comma(TokPunctuation::Comma,",");
TokPunctuation::EV_TokenId  Minus(TokPunctuation::Minus,"-");
TokPunctuation::EV_TokenId  Fullstop(TokPunctuation::Fullstop,".");




namespace udm
{
EnumValueMap &
TokPunctuation::EV_TokenId::Values_()       { return G_aTokPunctuation_EV_TokenId_Values; }
}




namespace csi
{
namespace uidl
{

void
TokPunctuation::Trigger( TokenInterpreter & io_rInterpreter ) const
{
    io_rInterpreter.Process_Punctuation(*this);
}

const char *
TokPunctuation::Text() const
{
    return eTag.Text();
}

void
Tok_EOL::Trigger( TokenInterpreter &    io_rInterpreter ) const
{
    io_rInterpreter.Process_EOL();
}

const char *
Tok_EOL::Text() const
{
    return "\r\n";
}

void
Tok_EOF::Trigger( TokenInterpreter &    io_rInterpreter ) const
{
    csv_assert(false);
//  io_rInterpreter.Process_EOF();
}

const char *
Tok_EOF::Text() const
{
    return "";
}


}   // namespace uidl
}   // namespace csi

