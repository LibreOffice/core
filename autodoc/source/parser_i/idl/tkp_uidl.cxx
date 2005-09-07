/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tkp_uidl.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:52:42 $
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
#include <s2_luidl/tkp_uidl.hxx>

// NOT FULLY DECLARED SERVICES
#include <s2_luidl/cx_idlco.hxx>



namespace csi
{
namespace uidl
{


TokenParser_Uidl::TokenParser_Uidl( Token_Receiver &        o_rUidlReceiver,
                                    DYN ::TkpDocuContext &  let_drDocuContext )
    :   pBaseContext(new Context_UidlCode(o_rUidlReceiver, let_drDocuContext)),
        pCurContext(0)
{
    SetStartContext();
}

TokenParser_Uidl::~TokenParser_Uidl()
{
}

void
TokenParser_Uidl::SetStartContext()
{
    pCurContext = pBaseContext.Ptr();
}

void
TokenParser_Uidl::SetCurrentContext( TkpContext & io_rContext )
{
    pCurContext = &io_rContext;
}

TkpContext &
TokenParser_Uidl::CurrentContext()
{
    return *pCurContext;
}

}   // namespace uidl
}   // namespace csi

