/*************************************************************************
 *
 *  $RCSfile: gen_info.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: np $ $Date: 2001-06-11 16:04:51 $
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

