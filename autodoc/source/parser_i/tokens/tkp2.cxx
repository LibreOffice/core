/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tkp2.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:11:31 $
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
#include <tokens/tkp2.hxx>

// NOT FULLY DECLARED SERVICES
#include <tools/tkpchars.hxx>
#include <tokens/tkpcont2.hxx>

TokenParse2::TokenParse2()
    :   pChars(0)
{
}

void
TokenParse2::Start( CharacterSource & i_rSource )
{
    pChars = &i_rSource;
    SetStartContext();
}

bool
TokenParse2::GetNextToken()
{
    csv_assert(pChars != 0);

    bool bDone = false;
    while ( NOT bDone AND NOT pChars->IsFinished() )
    {
        CurrentContext().ReadCharChain(*pChars);
        bDone = CurrentContext().PassNewToken();
        SetCurrentContext(CurrentContext().FollowUpContext());
    }
    return bDone;
}


