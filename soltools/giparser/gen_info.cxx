/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: gen_info.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 07:23:49 $
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

#include <gen_info.hxx>
#include <gi_list.hxx>


GenericInfo::GenericInfo( const Simstr &      i_sKey,
                          const Simstr &      i_sValue,
                          const Simstr &      i_sComment )
    :   sKey(i_sKey),
        sValue(i_sValue),
        sComment(i_sComment),
        dpSubList(0)
{
}

GenericInfo::GenericInfo( const GenericInfo & i_rInfo )
    :   sKey(i_rInfo.sKey),
        sValue(i_rInfo.sValue),
        sComment(i_rInfo.sComment),
        dpSubList(0)
{
    if ( i_rInfo.HasSubList() )
    {
        dpSubList = new List_GenericInfo(i_rInfo.SubList());
    }
}

GenericInfo::~GenericInfo()
{
    if ( dpSubList != 0 )
        delete dpSubList;
}

GenericInfo &
GenericInfo::operator=( const GenericInfo & i_rInfo )
{
    sKey = i_rInfo.sKey;
    sValue = i_rInfo.sValue;
    sComment = i_rInfo.sComment;

    if ( dpSubList != 0 )
        delete dpSubList;
    if ( i_rInfo.HasSubList() )
    {
        dpSubList = new List_GenericInfo(i_rInfo.SubList());
    }
    else
        dpSubList = 0;

    return *this;
}

List_GenericInfo &
GenericInfo::CreateMyList() const
{
    return * ( const_cast<GenericInfo&>(*this).dpSubList = new List_GenericInfo);

}

