/*************************************************************************
 *
 *  $RCSfile: cx_sub.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: np $ $Date: 2002-03-08 14:45:34 $
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
#include <s2_luidl/cx_sub.hxx>



// NOT FULLY DECLARED SERVICES
#include <s2_luidl/tokrecv.hxx>
#include <../../parser/inc/tokens/parseinc.hxx>
#include <x_parse2.hxx>
#include <s2_luidl/tk_const.hxx>



namespace csi
{
namespace uidl
{

bool
Cx_Base::PassNewToken()
{
    if (pNewToken)
    {
        rReceiver.Receive(*pNewToken.Release());
        return true;
    }
    return false;
}

TkpContext &
Cx_Base::FollowUpContext()
{
    csv_assert(pFollowUpContext != 0);
    return *pFollowUpContext;
}

void
Context_MLComment::ReadCharChain( CharacterSource & io_rText )
{
    char cNext = NULCH;

    do {
        if ( jumpTo(io_rText,'*') == NULCH )
            throw X_AutodocParser(X_AutodocParser::x_UnexpectedEOF);

        cNext = jumpOver(io_rText,'*');
        if (cNext == NULCH)
            throw X_AutodocParser(X_AutodocParser::x_UnexpectedEOF);
    } while (cNext != '/');
    io_rText.MoveOn();
    io_rText.CutToken();
    SetToken(0);
}

void
Context_SLComment::ReadCharChain( CharacterSource & io_rText )
{
    jumpToEol(io_rText);
    if (io_rText.CurChar() != NULCH)
        jumpOverEol(io_rText);
    io_rText.CutToken();
    SetToken(0);
}

void
Context_Praeprocessor::ReadCharChain( CharacterSource & io_rText )
{
    jumpToEol(io_rText);
    if (io_rText.CurChar() != NULCH)
        jumpOverEol(io_rText);
    io_rText.CutToken();
    SetToken(0);
}

void
Context_Assignment::ReadCharChain( CharacterSource &    io_rText )
{
    char cNext = NULCH;
    do {
        if ( (cNext = jumpTo(io_rText,';',',','"','}')) == NULCH )
            throw X_AutodocParser(X_AutodocParser::x_UnexpectedEOF);
        if (cNext == '"')
        {
            cNext = io_rText.MoveOn();
            while (cNext != '"')
            {
                if ( (cNext = jumpTo(io_rText,'"','\\')) == NULCH )
                    throw X_AutodocParser(X_AutodocParser::x_UnexpectedEOF);
                if (cNext == '\\')
                    io_rText.MoveOn();
            }
            cNext = io_rText.MoveOn();
        }   // endif (cNext == '"')
    } while (cNext != ';' AND cNext != ',' AND cNext != '}');

    if (cNext == ',' OR cNext == ';')
        io_rText.MoveOn();
    SetToken(new TokAssignment(io_rText.CutToken()));
}



}   // namespace uidl
}   // namespace csi

