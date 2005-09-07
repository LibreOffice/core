/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cx_sub.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:45:58 $
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
        do {
            cNext = jumpTo(io_rText,'*','\n');
            if (cNext == '\n')
            {
                Receiver().Increment_CurLine();
                cNext = io_rText.MoveOn();
            }
            else if (cNext == NULCH)
                throw X_AutodocParser(X_AutodocParser::x_UnexpectedEOF);
        } while (cNext != '*');

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

    Receiver().Increment_CurLine();
}

void
Context_Praeprocessor::ReadCharChain( CharacterSource & io_rText )
{
    jumpToEol(io_rText);
    if (io_rText.CurChar() != NULCH)
        jumpOverEol(io_rText);
    io_rText.CutToken();
    SetToken(0);

    Receiver().Increment_CurLine();
}

void
Context_Assignment::ReadCharChain( CharacterSource &    io_rText )
{
    // KORR_FUTURE
    // How to handle new lines within this, so he y get realised by
    //  ParserInfo?

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
