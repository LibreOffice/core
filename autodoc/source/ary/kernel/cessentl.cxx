/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cessentl.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 16:11:44 $
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
#include <ary/cpp/c_ce.hxx>
#include <ary/doc/d_oldcppdocu.hxx>


namespace ary
{
namespace cpp
{


CeEssentials::CeEssentials()
    :   sLocalName(),
        nOwner(0),
        nLocation(0)
{
}

CeEssentials::CeEssentials( const String  &     i_sLocalName,
                            Cid                 i_nOwner,
                            loc::Le_id          i_nLocation )
    :   sLocalName(i_sLocalName),
        nOwner(i_nOwner),
        nLocation(i_nLocation)
{
}

CeEssentials::~CeEssentials()
{
}



inline bool
IsInternal(const doc::Documentation & i_doc)
{
    const ary::doc::OldCppDocu *
        docu = dynamic_cast< const ary::doc::OldCppDocu* >(i_doc.Data());
    if (docu != 0)
        return docu->IsInternal();
    return false;
}


bool
CodeEntity::IsVisible() const
{
    // KORR_FUTURE:   Improve the whole handling of internal and visibility.
    return bIsVisible && NOT IsInternal(Docu());
}



}   // namespace cpp
}   // namespace ary
