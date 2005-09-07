/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cessentl.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:11:00 $
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
#include <ary/cessentl.hxx>


// NOT FULLY DEFINED SERVICES
#include <ary/ce.hxx>
#include <ary/info/codeinfo.hxx>


namespace ary {


CeEssentials::CeEssentials()
    :   sLocalName(),
        nId(0),
        nOwner(0),
        nLocation(0),
        pInfo(0)
{
}

CeEssentials::CeEssentials( Cid                 i_nId,
                            const udmstri &     i_sLocalName,
                            Cid                 i_nOwner,
                            Lid                 i_nLocation )
    :   sLocalName(i_sLocalName),
        nId(i_nId),
        nOwner(i_nOwner),
        nLocation(i_nLocation),
        pInfo(0)
{
}

CeEssentials::~CeEssentials()
{
}

bool
CodeEntity::IsVisible() const
{
    // KORR_FUTURE:   Improve the whole handling of internal and visibility.
    return bIsVisible && NOT Info().IsInternal();
}






}   // namespace ary



