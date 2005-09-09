/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: swdbdata.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 02:12:48 $
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
#ifndef _SWDBDATA_HXX
#define _SWDBDATA_HXX

#ifndef _RTL_USTRING_
#include <rtl/ustring>
#endif

// contains the description of a data source
struct SwDBData
{
    ::rtl::OUString     sDataSource;
    ::rtl::OUString     sCommand;       //table, query or statement
    sal_Int32           nCommandType; //com::sun::star::sdb::CommandType
    SwDBData() :
        nCommandType(0){}

    BOOL operator !=(const SwDBData& rCmp) const
        {return rCmp.sDataSource != sDataSource || rCmp.sCommand != sCommand || rCmp.nCommandType != nCommandType;}
    BOOL operator ==(const SwDBData& rCmp) const
        {return rCmp.sDataSource == sDataSource && rCmp.sCommand == sCommand && rCmp.nCommandType == nCommandType;}
};

#endif

