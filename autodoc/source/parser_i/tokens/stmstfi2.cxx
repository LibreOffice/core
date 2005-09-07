/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: stmstfi2.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:11:15 $
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
#include <tokens/stmstfi2.hxx>


// NOT FULLY DECLARED SERVICES
#include <tokens/tkpcont2.hxx>


StmBoundsStatu2::StmBoundsStatu2( StateMachineContext &
                                                    o_rOwner,
                                  TkpContext &      i_rFollowUpContext,
                                  uintt             i_nStatusFunctionNr,
                                  bool              i_bIsDefault )
    :   pOwner(&o_rOwner),
        pFollowUpContext(&i_rFollowUpContext),
        nStatusFunctionNr(i_nStatusFunctionNr),
        bIsDefault(i_bIsDefault)
{
}

bool
StmBoundsStatu2::IsADefault() const
{
    return bIsDefault;
}

StmBoundsStatu2 *
StmBoundsStatu2::AsBounds()
{
    return this;
}



