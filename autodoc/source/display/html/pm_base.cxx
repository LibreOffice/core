/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pm_base.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:34:34 $
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
#include "pm_base.hxx"


// NOT FULLY DEFINED SERVICES
#include "opageenv.hxx"
#include "pagemake.hxx"


//********************       SpecializedPageMaker        *********************//

SpecializedPageMaker::SpecializedPageMaker( PageDisplay &  io_rPage )
    :   pEnv( &io_rPage.Env() ),
        pCurOut( &io_rPage.CurOut() ),
        pPage( &io_rPage )
{
}

void
SpecializedPageMaker::Write_NavBar()
{
     // Dummy
}

void
SpecializedPageMaker::Write_TopArea()
{
     // Dummy
}

void
SpecializedPageMaker::Write_DocuArea()
{
     // Dummy
}

void
SpecializedPageMaker::Write_ChildList( ary::SlotAccessId   ,
                                       const char *        ,
                                       const char *        )
{
     // Dummy
}

csi::xml::Element &
SpecializedPageMaker::CurOut()
{
    return Page().CurOut();
}

