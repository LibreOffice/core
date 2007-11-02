/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: parseenv.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 17:04:12 $
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
#include <semantic/parseenv.hxx>


// NOT FULLY DECLARED SERVICES
#include <ary/doc/d_oldcppdocu.hxx>
#include <x_parse.hxx>


void
ParseEnvironment::Enter( E_EnvStackAction   i_eWayOfEntering )
{
    switch (i_eWayOfEntering)
    {
        case push:
                InitData();
                if ( Parent() != 0 )
                {
                    csv_assert( Parent()->CurSubPeUse() != 0 );
                    Parent()->CurSubPeUse()->InitParse();
                }
                break;
        case pop_success:
                break;
        case pop_failure:
                break;
        default:
            csv_assert(false);
    }   // end switch
}

void
ParseEnvironment::Leave( E_EnvStackAction   i_eWayOfLeaving )
{
    switch (i_eWayOfLeaving)
    {
        case push:
                break;
        case pop_success:
                TransferData();
                if ( Parent() != 0 )
                {
                    csv_assert( Parent()->CurSubPeUse() != 0 );
                    Parent()->CurSubPeUse()->GetResults();
                }
                break;
        case pop_failure:
                break;
        default:
            csv_assert(false);
    }   // end switch
}

ParseEnvironment::ParseEnvironment( ParseEnvironment *  i_pParent )
    :   pParent(i_pParent),
        // pDocu,
        pCurSubPe(0)
{
}

void
ParseEnvironment::GlobalHandlingOfSyntaxError( const char * i_sTokenText,
                                               const char * i_sFileName,
                                               uintt        i_nLineCount )
{
    throw X_Parser( X_Parser::x_UnexpectedToken,
                    i_sTokenText,
                    i_sFileName,
                    i_nLineCount );
}



